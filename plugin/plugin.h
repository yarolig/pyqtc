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

#include "workerclient.h"
#include "workerpool.h"

#include <extensionsystem/iplugin.h>

namespace pyqtc {

class PythonIcons;

class Plugin : public ExtensionSystem::IPlugin {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "pyqtc.json")


public:
  Plugin();
  virtual ~Plugin();

  bool initialize(const QStringList& arguments, QString* errorString);
  void extensionsInitialized();
  ShutdownFlag aboutToShutdown();

private slots:
  void JumpToDefinition();
  void JumpToDefinitionFinished(WorkerClient::ReplyType* reply);

private:
  static const char* kJumpToDefinition;

  WorkerPool<WorkerClient>* worker_pool_;
  PythonIcons* icons_;
};

} // namespace pyqtc
