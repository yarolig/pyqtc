#!/usr/bin/env python

# Ignore access to protected members pylint: disable=W0212

"""
Entry point for the pyqtc worker.
"""

import logging
import os
import sys
import rope.base.project
from rope.base import worder
from rope.contrib import codeassist

import messagehandler
import rpc_pb2
import symbolindex


class ProjectNotFoundError(Exception):
  """
  An action was requested on a file that was not found in a project.
  """


class Project(object):
  """
  Helper object that contains a rope project and an associated symbol index.
  """

  def __init__(self, rope_project):
    self.rope_project = rope_project
    self.symbol_index = symbolindex.SymbolIndex(rope_project)


class Handler(messagehandler.MessageHandler):
  """
  Handles rpc requests.
  """

  PROPOSAL_TYPES = {
    "instance": rpc_pb2.CompletionResponse.Proposal.INSTANCE,
    "class":    rpc_pb2.CompletionResponse.Proposal.CLASS,
    "function": rpc_pb2.CompletionResponse.Proposal.FUNCTION,
    "module":   rpc_pb2.CompletionResponse.Proposal.MODULE,
  }

  PROPOSAL_SCOPES = {
    "local":             rpc_pb2.CompletionResponse.Proposal.LOCAL,
    "global":            rpc_pb2.CompletionResponse.Proposal.GLOBAL,
    "builtin":           rpc_pb2.CompletionResponse.Proposal.BUILTIN,
    "attribute":         rpc_pb2.CompletionResponse.Proposal.ATTRIBUTE,
    "imported":          rpc_pb2.CompletionResponse.Proposal.IMPORTED,
    "keyword":           rpc_pb2.CompletionResponse.Proposal.KEYWORD,
    "parameter_keyword": rpc_pb2.CompletionResponse.Proposal.PARAMETER_KEYWORD,
  }

  MAXFIXES = 10

  def __init__(self):
    super(Handler, self).__init__(rpc_pb2.Message)

    self.projects = {}

  def CreateProjectRequest(self, request, _response):
    """
    Creates a new rope project and stores it away for later.
    """

    root = os.path.normpath(request.project_root)
    project = rope.base.project.Project(root)

    self.projects[root] = Project(project)

  def DestroyProjectRequest(self, request, _response):
    """
    Cleans up a rope project when it is closed by the user in Qt Creator.
    """

    root = os.path.normpath(request.project_root)
    project = self.projects[root]

    project.rope_project.close()
    del self.projects[root]

  def _Context(self, context):
    """
    Returns a (project, resource, source, offset) tuple for the context.
    """

    project       = self._ProjectForFile(context.file_path).rope_project
    relative_path = os.path.relpath(context.file_path, project.address)
    resource      = project.get_resource(relative_path)

    return (
      project,
      resource,
      context.source_text + "\n",
      context.cursor_position,
    )

  def _ProjectForFile(self, file_path):
    """
    Tries to find the project that contains the given file.
    """

    project_root = file_path
    while project_root:
      try:
        return self.projects[project_root]
      except KeyError:
        pass

      project_root = os.path.dirname(project_root)

    raise ProjectNotFoundError(file_path)

  def CompletionRequest(self, request, response):
    """
    Finds completion proposals for the given location in the given source file.
    """

    # Get information out of the request
    project, resource, source, offset = self._Context(request.context)

    # If the cursor is immediately after a comma or open paren, we should look
    # for a calltip first.
    word_finder = worder.Worder(source)
    non_space_offset = word_finder.code_finder._find_last_non_space_char(offset)

    if word_finder.code_finder.code[non_space_offset] in "(,":
      paren_start = word_finder.find_parens_start_from_inside(offset)

      # Get a calltip now
      calltip = codeassist.get_calltip(project, source, paren_start-1,
                                       maxfixes=self.MAXFIXES,
                                       resource=resource,
                                       remove_self=True)

      if calltip is not None:
        response.insertion_position = paren_start + 1
        response.calltip = calltip
        return

    # Do normal completion if a calltip couldn't be found
    proposals = codeassist.code_assist(project, source, offset,
                                       maxfixes=self.MAXFIXES,
                                       resource=resource)
    proposals = codeassist.sorted_proposals(proposals)

    # Get the position that this completion will start from.
    starting_offset = codeassist.starting_offset(source, offset)
    response.insertion_position = starting_offset

    # Construct the response protobuf
    for proposal in proposals:
      proposal_pb = response.proposal.add()
      proposal_pb.name = proposal.name

      docstring = proposal.get_doc()

      if proposal.type in self.PROPOSAL_TYPES:
        proposal_pb.type = self.PROPOSAL_TYPES[proposal.type]

      if proposal.scope in self.PROPOSAL_SCOPES:
        proposal_pb.scope = self.PROPOSAL_SCOPES[proposal.scope]

      if docstring is not None:
        proposal_pb.docstring = docstring

  def TooltipRequest(self, request, response):
    """
    Finds and returns a tooltip for the given location in the given source file.
    """

    project, resource, source, offset = self._Context(request.context)
    docstring = codeassist.get_doc(project, source, offset,
        maxfixes=self.MAXFIXES, resource=resource)

    if docstring is not None:
      response.rich_text = docstring

  def DefinitionLocationRequest(self, request, response):
    """
    Finds the definition location of the current symbol.
    """

    project, resource, source, offset = self._Context(request.context)
    resource, offset = codeassist.get_definition_location(
        project, source, offset,
        maxfixes=self.MAXFIXES, resource=resource)

    if resource is not None:
      response.file_path = resource.real_path

    if offset is not None:
      response.line = offset

  def RebuildSymbolIndexRequest(self, request, _response):
    """
    Parses all the files in the project and rebuilds the symbol index.
    """

    project = self.projects[request.project_root]
    project.symbol_index.Rebuild()

  def UpdateSymbolIndexRequest(self, request, _response):
    """
    Parses just one file in the project and updates the symbol index.
    """

    project = self._ProjectForFile(request.file_path)

    # Make the file_path relative to the project
    relative_path = os.path.relpath(
        request.file_path, project.rope_project.address)

    project.symbol_index.UpdateFile(relative_path)

  def SearchRequest(self, request, response):
    """
    Searches the symbol index.
    """

    # If a file_path was provided, only search in the project that owns it.
    # Otherwise search all the projects.
    if request.HasField("file_path"):
      projects = [self._ProjectForFile(request.file_path)]
    else:
      projects = self.projects.values()

    for project in projects:
      project_dir = project.rope_project.address

      file_path = None
      symbol_type = None

      if request.HasField("file_path"):
        # Make the file_path relative to the project
        file_path = os.path.relpath(
            request.file_path, project_dir)

      if request.HasField("symbol_type") and request.symbol_type != rpc_pb2.ALL:
        symbol_type = request.symbol_type

      # Do the query
      results = project.symbol_index.Search(request.query,
          file_path=file_path, symbol_type=symbol_type)

      # Create the response
      for module_name, file_path, line_number, symbol_name, symbol_type in results:
        result_pb = response.result.add()

        result_pb.module_name = module_name
        result_pb.file_path   = os.path.join(project_dir, file_path)
        result_pb.line_number = line_number
        result_pb.symbol_name = symbol_name
        result_pb.symbol_type = symbol_type


def Main(args):
  """
  Connects to the socket passed on the commandline and listens for requests.
  """

  logging.basicConfig()

  handler = Handler()
  handler.ServeForever(args[0])


if __name__ == "__main__":
  Main(sys.argv[1:])
