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

#include "projects.h"

#include "closure.h"
#include "messagehandler.h"
#include <projectexplorer/project.h>
//#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/session.h>
#include <utils/qtcassert.h>

#include <QtDebug>

using namespace pyqtc;


Projects::Projects(WorkerPool<WorkerClient>* worker_pool, QObject* parent)
  : QObject(parent),
    worker_pool_(worker_pool)
{
  QObject* session = ProjectExplorer::SessionManager::instance();
  QTC_ASSERT(session, return);

  connect(session, SIGNAL(projectAdded(ProjectExplorer::Project*)),
          SLOT(ProjectAdded(ProjectExplorer::Project*)));
  connect(session, SIGNAL(aboutToRemoveProject(ProjectExplorer::Project *)),
          this, SLOT(AboutToRemoveProject(ProjectExplorer::Project*)));
}

void Projects::ProjectAdded(ProjectExplorer::Project* project) {
  const QString project_root = project->projectDirectory().toString();

  WorkerClient::ReplyType* reply =
      worker_pool_->NextHandler()->CreateProject(project_root);
  NewClosure(reply, SIGNAL(Finished(bool)),
             this, SLOT(CreateProjectFinished(WorkerClient::ReplyType*,QString)),
             reply, project_root);
}

void Projects::CreateProjectFinished(WorkerClient::ReplyType* reply,
                                     const QString& project_root) {
  reply->deleteLater();

  reply = worker_pool_->NextHandler()->RebuildSymbolIndex(project_root);
  connect(reply, SIGNAL(Finished(bool)), reply, SLOT(deleteLater()));
}

void Projects::AboutToRemoveProject(ProjectExplorer::Project* project) {
  WorkerClient::ReplyType* reply =
      worker_pool_->NextHandler()->DestroyProject(project->projectDirectory().toString());

  connect(reply, SIGNAL(Finished(bool)), reply, SLOT(deleteLater()));
}
