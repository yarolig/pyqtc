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
#pragma once

#include <QIcon>
#include <QMultiMap>
#include <QObject>

#include <cplusplus/Icons.h>

#include "workerclient.h"
#include "workerpool.h"


namespace ProjectExplorer {
  class Project;
}

namespace pyqtc {

class WorkerClient;

class Projects : public QObject {
  Q_OBJECT

public:
  Projects(WorkerPool<WorkerClient>* worker_pool, QObject* parent = 0);

private slots:
  void ProjectAdded(ProjectExplorer::Project* project);
  void AboutToRemoveProject(ProjectExplorer::Project* project);

  void CreateProjectFinished(WorkerClient::ReplyType* reply,
                             const QString& project_root);

private:
  WorkerPool<WorkerClient>* worker_pool_;
};

} // namespace pyqtc
