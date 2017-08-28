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

#include "config.h"
#include "constants.h"
#include "completionassist.h"
#include "hoverhandler.h"
#include "plugin.h"
#include "projects.h"
#include "pythoneditorfactory.h"
#include "pythonfilter.h"
#include "pythonicons.h"
#include "workerpool.h"

#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/helpmanager.h>
#include <texteditor/textdocument.h>
#include <coreplugin/icore.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/icontext.h>
#include <utils/mimetypes/mimedatabase.h>

#include <QAction>
#include <QMessageBox>
#include <QMainWindow>
#include <QMenu>
#include <QtHelp/QHelpEngineCore>

#include <QtDebug>
#include <QtPlugin>

using namespace pyqtc;

const char* Plugin::kJumpToDefinition = "pyqtc.JumpToDefinition";


inline void InitResources() {
  Q_INIT_RESOURCE(pyqtc);
}


Plugin::Plugin()
  : worker_pool_(new WorkerPool<WorkerClient>(this)),
    icons_(new PythonIcons)
{
  InitResources();

  worker_pool_->SetExecutableName("python");
  worker_pool_->SetExecutableArguments(QStringList() << config::kWorkerZipPath);
  worker_pool_->SetWorkerCount(1);
  worker_pool_->SetLocalServerName("pyqtc");
  worker_pool_->Start();
}

Plugin::~Plugin() {
  delete icons_;
}

bool Plugin::initialize(const QStringList& arguments, QString* errorString) {
  Q_UNUSED(arguments)
  Q_UNUSED(errorString)

  qDebug() << "pyqtc Plugin::initialize";

  // Utils::addMimeTypes(QLatin1String(":/pythoneditor/PythonEditor.mimetypes.xml"));

  addAutoReleasedObject(new Projects(worker_pool_));
  addAutoReleasedObject(new CompletionAssistProvider(worker_pool_, icons_));
  addAutoReleasedObject(new PythonEditorFactory(0, worker_pool_));
  addAutoReleasedObject(new PythonClassFilter(worker_pool_, icons_));
  addAutoReleasedObject(new PythonFunctionFilter(worker_pool_, icons_));
  addAutoReleasedObject(new PythonCurrentDocumentFilter(worker_pool_, icons_));

  Core::Context context(constants::kEditorId);
  Core::ActionContainer* menu = Core::ActionManager::createMenu(constants::kMenuContext);

  QAction* action = new QAction(tr("Follow Symbol Under Cursor"), this);
  Core::Command* cmd = Core::ActionManager::registerAction(
        action, constants::kJumpToDefinitionId, context);
  cmd->setDefaultKeySequence(QKeySequence(Qt::Key_F2));
  connect(action, SIGNAL(triggered()), this, SLOT(JumpToDefinition()));
  menu->addAction(cmd);

  return true;
}

void Plugin::extensionsInitialized() {
  Core::HelpManager::registerDocumentation(QStringList() << config::kDocumentationPath);
}

ExtensionSystem::IPlugin::ShutdownFlag Plugin::aboutToShutdown() {
  return SynchronousShutdown;
}

void Plugin::JumpToDefinition() {
  Core::EditorManager* em = Core::EditorManager::instance();

  TextEditor::TextEditorWidget* editor = qobject_cast<TextEditor::TextEditorWidget*>(
        em->currentEditor()->widget());
  if (!editor) {
    return;
  }

  WorkerClient::ReplyType* reply =
      worker_pool_->NextHandler()->DefinitionLocation(
        editor->textDocument()->filePath().toString(),
        editor->textDocument()->plainText(),
        editor->position());

  NewClosure(reply, SIGNAL(Finished(bool)),
             this, SLOT(JumpToDefinitionFinished(WorkerClient::ReplyType*)),
             reply);
}

void Plugin::JumpToDefinitionFinished(WorkerClient::ReplyType* reply) {
  reply->deleteLater();

  if (!reply->is_successful()) {
    return;
  }

  Core::EditorManager* em = Core::EditorManager::instance();
  TextEditor::TextEditorWidget* editor = qobject_cast<TextEditor::TextEditorWidget*>(
        em->currentEditor()->widget());
  if (!editor) {
    return;
  }

  const pb::DefinitionLocationResponse& response =
      reply->message().definition_location_response();

  if (response.has_line()) {
    if (response.has_file_path()) {
      Core::EditorManager::openEditorAt(response.file_path(), response.line());
    } else {
      editor->gotoLine(response.line());
    }
  }
}

