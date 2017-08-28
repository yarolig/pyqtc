/*  pyqtc - QtCreator plugin with code completion using rope.
    Copyright 2011 David Sansome <me@davidsansome.com>
    Copyright 2017 Alexander Izmailov <yarolig@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "constants.h"
#include "hoverhandler.h"
#include "pythonindenter.h"
#include "pythoneditorfactory.h"
#include "completionassist.h"

#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/ieditor.h>
#include <texteditor/texteditoractionhandler.h>
#include <texteditor/texteditorsettings.h>
#include <texteditor/texteditor.h>
#include <texteditor/textdocument.h>

#include <QtDebug>

//using namespace pyqtc;
using pyqtc::PythonEditorFactory;

PythonEditorFactory::PythonEditorFactory(QObject* parent, WorkerPool<WorkerClient> *worker_pool)
  : TextEditor::TextEditorFactory(parent)
{
    setId(pyqtc::constants::kEditorId);
    setDisplayName(tr(pyqtc::constants::kEditorDisplayName));
    addMimeType("text/plain");
    addMimeType("text/python");
    addMimeType("text/x-python");
    addMimeType("text/x-python-gui");
    addMimeType("application/python");
    addMimeType("application/x-python");

    setIndenterCreator([]() { return new pyqtc::PythonIndenter; });
    setDocumentCreator([]() {
        return new TextEditor::TextDocument(pyqtc::constants::kEditorId);
    });

    setCompletionAssistProvider(pyqtc::CompletionAssistProvider::instance());
    setUseGenericHighlighter(true);
    //setCommentStyle(Utils::CommentDefinition::HashStyle);
    setParenthesesMatchingEnabled(true);
    setMarksVisible(true);
    setCodeFoldingSupported(true);
    setEditorActionHandlers(
          TextEditor::TextEditorActionHandler::Format
        | TextEditor::TextEditorActionHandler::UnCommentSelection
        | TextEditor::TextEditorActionHandler::UnCollapseAll);

    addHoverHandler(new pyqtc::HoverHandler(worker_pool));
}

PythonEditorFactory::~PythonEditorFactory() {
}

