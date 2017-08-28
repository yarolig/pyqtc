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

#include "workerclient.h"

using namespace pyqtc;


WorkerClient::WorkerClient(QIODevice* device, QObject* parent)
    : AbstractMessageHandler<pb::Message>(device, parent)
{
}

WorkerClient::ReplyType* WorkerClient::CreateProject(const QString& project_root) {
  pb::Message message;
  pb::CreateProjectRequest* req = message.mutable_create_project_request();

  req->set_project_root(project_root);

  return SendMessageWithReply(&message);
}

WorkerClient::ReplyType* WorkerClient::DestroyProject(const QString& project_root) {
  pb::Message message;
  pb::DestroyProjectRequest* req = message.mutable_destroy_project_request();

  req->set_project_root(project_root);

  return SendMessageWithReply(&message);
}

WorkerClient::ReplyType* WorkerClient::Completion(const QString& file_path,
                                                  const QString& source_text,
                                                  int cursor_position) {
  pb::Message message;
  pb::CompletionRequest* req = message.mutable_completion_request();

  req->mutable_context()->set_file_path(file_path);
  req->mutable_context()->set_source_text(source_text);
  req->mutable_context()->set_cursor_position(cursor_position);

  return SendMessageWithReply(&message);
}

WorkerClient::ReplyType* WorkerClient::Tooltip(const QString& file_path,
                                               const QString& source_text,
                                               int cursor_position) {
  pb::Message message;
  pb::TooltipRequest* req = message.mutable_tooltip_request();

  req->mutable_context()->set_file_path(file_path);
  req->mutable_context()->set_source_text(source_text);
  req->mutable_context()->set_cursor_position(cursor_position);

  return SendMessageWithReply(&message);
}

WorkerClient::ReplyType* WorkerClient::DefinitionLocation(const QString& file_path,
                                                          const QString& source_text,
                                                          int cursor_position) {
  pb::Message message;
  pb::DefinitionLocationRequest* req = message.mutable_definition_location_request();

  req->mutable_context()->set_file_path(file_path);
  req->mutable_context()->set_source_text(source_text);
  req->mutable_context()->set_cursor_position(cursor_position);

  return SendMessageWithReply(&message);
}

WorkerClient::ReplyType* WorkerClient::RebuildSymbolIndex(const QString& project_root) {
  pb::Message message;
  pb::RebuildSymbolIndexRequest* req = message.mutable_rebuild_symbol_index_request();

  req->set_project_root(project_root);

  return SendMessageWithReply(&message);
}

WorkerClient::ReplyType* WorkerClient::UpdateSymbolIndex(const QString& file_path) {
  pb::Message message;
  pb::UpdateSymbolIndexRequest* req = message.mutable_update_symbol_index_request();

  req->set_file_path(file_path);

  return SendMessageWithReply(&message);
}

WorkerClient::ReplyType* WorkerClient::Search(const QString& query,
                                              const QString& file_path,
                                              pb::SymbolType type) {
  pb::Message message;
  pb::SearchRequest* req = message.mutable_search_request();

  req->set_query(query);

  if (!file_path.isEmpty()) {
    req->set_file_path(file_path);
  }

  if (type != pb::ALL) {
    req->set_symbol_type(type);
  }

  return SendMessageWithReply(&message);
}
