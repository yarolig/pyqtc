Pyqtc is a plugin for Qt Creator that adds Python code-completion, navigation
and documentation.

How to install
==============

Dependencies
------------

You need:

  * Protobuf 2.4.0 or greater.
  * Qt Creator 2.8.
  * The Qt Creator source code.  You don't have to compile Qt Creator yourself,
    but you do need to have the source available.  You can download it from
    https://github.com/qtproject/qt-creator
  * Python and Pylint

Quick build
-----------

For Ubuntu 16.04 launch `sudo apt-get install build-essential libprotoc-dev libprotobuf-dev python-protobuf qt4-qmake cmake pylint`

Compiling pyqtc
---------------

You need to tell cmake the locations of the Qt Creator source and binaries.
These might be in the same place if you've built Qt Creator yourself, otherwise
they will be different.  Pyqtc will be installed into the binary directory, the
source directory will be used to find header files.

    mkdir bin
    cd bin
    cmake .. -DQTC_SOURCE=/path/to/qtcreator/source \
             -DQTC_BINARY=/path/to/qtcreator/installation
    make
    make install

You may need to copy `python.xml` to `~/.config/QtProject/qtcreator/generic-highlighter` if highlihgting not working.

Features
========

Code completion
---------------

![Code completion](https://raw.github.com/davidsansome/pyqtc/master/images/completion.png)

Pyqtc statically analyses your Python code as you type and offers code
completion suggestions automatically after a period (.), or after pressing
Ctrl+Space.

Function calltips
-----------------

![Function calltips](https://raw.github.com/davidsansome/pyqtc/master/images/calltip.png)

After an opening bracket, Pyqtc will show a tooltip containing information about
the arguments in the function you're about to call.

Hover tooltips
--------------

![Tooltips](https://raw.github.com/davidsansome/pyqtc/master/images/tooltip.png)

Pyqtc will show docstrings for functions, modules and classes if you hover over
them using the mouse.

Python indentation
------------------

![Indentation](https://raw.github.com/davidsansome/pyqtc/master/images/indentation.png)

Automatically indents after a colon (:) and unindents after a continue, break,
pass, return or raise.

Navigation
----------

Pressing the F2 key will jump to the definition of the Python symbol under the
cursor.
Pressing Ctrl+K lets you jump straight to any Python class and function
definitions in the current project, or browse the Python symbols in the current
file.

Documentation
-------------

The documentation for Python 2.7.2 is available for browsing using Qt Creator's
Help tab.

![Documentation](https://raw.github.com/davidsansome/pyqtc/master/images/help.png)
