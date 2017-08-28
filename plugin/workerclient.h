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

#include "messagehandler.h"
#include "rpc.pb.h"

namespace pyqtc {

class WorkerClient : public AbstractMessageHandler<pb::Message> {
public:
  WorkerClient(QIODevice* device, QObject* parent);

  ReplyType* CreateProject(const QString& project_root);
  ReplyType* DestroyProject(const QString& project_root);

  ReplyType* RebuildSymbolIndex(const QString& project_root);
  ReplyType* UpdateSymbolIndex(const QString& file_path);

  ReplyType* Completion(const QString& file_path,
                        const QString& source_text,
                        int cursor_position);
  ReplyType* Tooltip(const QString& file_path,
                     const QString& source_text,
                     int cursor_position);
  ReplyType* DefinitionLocation(const QString& file_path,
                                const QString& source_text,
                                int cursor_position);

  ReplyType* Search(const QString& query,
                    const QString& file_path = QString(),
                    pb::SymbolType type = pb::ALL);
};

} // namespace
