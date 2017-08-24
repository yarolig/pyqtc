"""
Builds, maintains and searches an index of symbols in the project.
"""

import os.path
import re
import sqlite3

import rope.base.exceptions
import rope.base.pynames
import rope.base.pyobjects

import rpc_pb2


class SymbolIndex(object):
  """
  Creates an index of all the symbols in all the files in the project.

  Note: file paths passed to and returned from this class are all relative to
  the project's root directory.
  """

  DATABASE_FILENAME = "symbol_index.db"
  SCHEMA = [
    """
    CREATE TABLE files (
      file_path TEXT,
      module_name TEXT
    );
    CREATE TABLE symbols (
      fileid INTEGER,
      line_number INTEGER,
      symbol_name TEXT,
      symbol_type INTEGER
    );
    CREATE VIRTUAL TABLE symbol_index USING fts3();

    CREATE TABLE schema_version (version INTEGER);
    INSERT INTO schema_version(version) VALUES (0);
    """
  ]

  def __init__(self, project):
    self.project = project

    # Open the database
    db_filename = os.path.join(project.ropefolder.real_path,
                               self.DATABASE_FILENAME)
    self.conn = sqlite3.connect(db_filename)

    with self.conn:
      # Get the current schema version
      try:
        cursor = self.conn.execute("SELECT version FROM schema_version")
      except sqlite3.OperationalError:
        current_version = -1
      else:
        current_version = cursor.fetchone()[0]

      for version in xrange(current_version + 1, len(self.SCHEMA)):
        # Apply this schema update
        self.conn.executescript(self.SCHEMA[version])

  def Rebuild(self):
    """
    Completely rebuilds the index by removing everything from the database and
    parsing all the python files.
    """

    with self.conn:
      self.conn.execute("DELETE FROM files")
      self.conn.execute("DELETE FROM symbols")
      self.conn.execute("DELETE FROM symbol_index")

      map(self._AddFile, self.project.pycore.get_python_files())

  def UpdateFile(self, file_path):
    """
    Updates a single file in the index.
    """

    # Find this file in the project
    resource = self.project.get_resource(file_path)

    with self.conn:
      # Did this file exist already?
      row = self.conn.execute(
        "SELECT rowid FROM files WHERE file_path = ?",
        (file_path, )).fetchone()

      if row is not None:
        # Remove the existing data for this file
        self._RemoveFile(row[0])

      # Re-parse the file
      self._AddFile(resource)

  def Search(self, query, file_path=None, symbol_type=None, limit=1000):
    """
    Searches for the given query string in the index and returns an iterator
    over (module_name, file_path, line_number, symbol_name, symbol_type) tuples.
    If file_path is not None, only symbols in that file are returned.
    If symbol_type is not None, only symbols of that given type are returned.
    """

    # Remove special FTS characters from the user's query.  The .lower() removes
    # NEAR/n instructions as well.
    fts_query = re.sub(r'\W+', ' ', query.lower())

    # Stick * on the end of each search term
    fts_query = " ".join("%s*" % x for x in fts_query.split(" "))

    # Build the WHERE section of the query.
    where_clauses = [
      "i.content MATCH ?",
      "i.rowid = s.rowid",
      "s.fileid = f.rowid",
    ]
    where_parameters = [
      fts_query,
    ]

    if file_path is not None:
      where_clauses.append("f.file_path = ?")
      where_parameters.append(file_path)

    if symbol_type is not None:
      where_clauses.append("s.symbol_type = ?")
      where_parameters.append(symbol_type)

    # Build the query
    sql = """
      SELECT f.module_name,
             f.file_path,
             s.line_number,
             s.symbol_name,
             s.symbol_type
      FROM files AS f,
           symbols AS s,
           symbol_index AS i
      WHERE %s
      LIMIT ?
    """ % " AND ".join(where_clauses)

    # Execute the query
    return self.conn.execute(sql, tuple(where_parameters + [limit]))

  def _AddFile(self, resource):
    """
    Parses the resource and adds all its symbols to the database.
    The database connection MUST already be in a transaction.
    """

    # Open this file
    try:
      pyobject  = self.project.pycore.resource_to_pyobject(resource)
    except rope.base.exceptions.RopeError:
      # If the file couldn't be loaded, ignore it
      return

    module_name = self.project.pycore.modname(resource)
    file_path   = resource.path

    # Get the list of symbols in the module
    symbols = []
    self._WalkPyObject(pyobject, None, symbols)

    # Add the file to the database
    fileid = self.conn.execute(
      "INSERT INTO files (module_name, file_path) VALUES (?, ?)",
      (module_name, file_path)).lastrowid

    # Add each symbol to the database
    for symbol_name, line_number, symbol_type in symbols:
      self._AddSymbol(fileid, line_number, symbol_name, symbol_type)

  def _AddSymbol(self, fileid, line_number, symbol_name, symbol_type):
    """
    Adds the Python symbol to the database.
    The database connection MUST already be in a transaction.
    """

    cursor = self.conn.execute("""
      INSERT INTO symbols (fileid, line_number, symbol_name, symbol_type)
      VALUES (?, ?, ?, ?)
    """, (fileid, line_number, symbol_name, symbol_type))

    rowid = cursor.lastrowid

    self.conn.execute("""
      INSERT INTO symbol_index (rowid, content)
      VALUES (?, ?)
    """, (rowid, symbol_name))

  def _WalkPyObject(self, pyobject, dotted_name, ret):
    """
    Walks pyobject and all its children, adding a tuple for each one to ret.
    """

    if dotted_name is not None:
      line_number = pyobject.get_ast().lineno
      symbol_type = rpc_pb2.VARIABLE

      if isinstance(pyobject, rope.base.pyobjects.AbstractFunction):
        symbol_type = rpc_pb2.FUNCTION
      elif isinstance(pyobject, rope.base.pyobjects.AbstractClass):
        symbol_type = rpc_pb2.CLASS
      elif isinstance(pyobject, rope.base.pyobjects.AbstractModule):
        symbol_type = rpc_pb2.MODULE

      # Add this object to the result list
      ret.append((dotted_name, line_number, symbol_type))

    # Walk the child objects
    try:
      children = pyobject.get_attributes().items()
    except Exception:
      # Ignore any errors from rope
      return

    for name, pyname in children:
      if isinstance(pyname, rope.base.pynames.DefinedName):
        if dotted_name is not None:
          name = "%s.%s" % (dotted_name, name)

        self._WalkPyObject(pyname.get_object(), name, ret)

  def _RemoveFile(self, fileid):
    """
    Removes the file with the give rowid from the database.
    """

    # Remove the file itself
    self.conn.execute("DELETE FROM files WHERE rowid = ?", (fileid,))

    # Remove any symbols in the file
    for row in self.conn.execute("SELECT rowid FROM symbols WHERE fileid = ?",
                                 (fileid,)):
      self._RemoveSymbol(row[0])

  def _RemoveSymbol(self, rowid):
    """
    Removes the symbol with the given rowid from the database.
    """

    self.conn.execute("DELETE FROM symbols WHERE rowid = ?", (rowid,))
    self.conn.execute("DELETE FROM symbol_index WHERE rowid = ?", (rowid,))
