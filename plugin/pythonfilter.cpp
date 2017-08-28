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
#include "pythonfilter.h"
#include "pythonicons.h"

#include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/icore.h>
#include <texteditor/texteditor.h>

using namespace pyqtc;

PythonFilterBase::PythonFilterBase(WorkerPool<WorkerClient>* worker_pool,
                                   const PythonIcons* icons)
  : Core::ILocatorFilter(NULL),
    worker_pool_(worker_pool),
    icons_(icons),
    symbol_type_(pb::ALL),
    file_path_(QString())
{
}

QList<Core::LocatorFilterEntry> PythonFilterBase::matchesFor(
    QFutureInterface<Core::LocatorFilterEntry>& future, const QString& entry) {
  QScopedPointer<WorkerClient::ReplyType> reply(
        worker_pool_->NextHandler()->Search(entry, file_path_, symbol_type_));
  reply->WaitForFinished();

  QList<Core::LocatorFilterEntry> ret;
  if (!reply->is_successful() || future.isCanceled()) {
    return ret;
  }

  const pb::SearchResponse* response = &reply->message().search_response();

  for (int i=0 ; i<response->result_size() ; ++i) {
    const pb::SearchResponse_Result* result = &response->result(i);

    EntryInternalData internal_data(result->file_path(), result->line_number());

    Core::LocatorFilterEntry entry(this, result->symbol_name(),
                               QVariant::fromValue(internal_data));
    entry.extraInfo = result->module_name();
    entry.displayIcon = icons_->IconForSearchResult(*result);

    ret << entry;
  }

  return ret;
}

void PythonFilterBase::accept(Core::LocatorFilterEntry selection) const {
  const EntryInternalData& data =
      selection.internalData.value<pyqtc::PythonFilterBase::EntryInternalData>();

#ifdef QTC_HAS_CORE_ID
  Core::Id editor_kind;
#else
  QString editor_kind;
#endif

  Core::EditorManager::openEditorAt(
        data.file_path_, data.line_number_, 0,
        editor_kind);
}

void PythonFilterBase::refresh(QFutureInterface<void>& future) {
}


PythonClassFilter::PythonClassFilter(
    WorkerPool<WorkerClient>* worker_pool, const PythonIcons* icons)
  : PythonFilterBase(worker_pool, icons)
{
  set_symbol_type(pb::CLASS);
  setShortcutString("c");
}


PythonFunctionFilter::PythonFunctionFilter(
    WorkerPool<WorkerClient>* worker_pool, const PythonIcons* icons)
  : PythonFilterBase(worker_pool, icons)
{
  set_symbol_type(pb::FUNCTION);
  setShortcutString("m");
}


PythonCurrentDocumentFilter::PythonCurrentDocumentFilter(
    WorkerPool<WorkerClient>* worker_pool, const PythonIcons* icons)
  : PythonFilterBase(worker_pool, icons)
{
  Core::EditorManager* editor_manager = Core::EditorManager::instance();

  connect(editor_manager, SIGNAL(currentEditorChanged(Core::IEditor*)),
          SLOT(CurrentEditorChanged(Core::IEditor*)));

  setShortcutString(".");
}

void PythonCurrentDocumentFilter::CurrentEditorChanged(Core::IEditor* editor) {
  if (editor) {
    set_file_path(editor->document()->filePath());
  }
}
