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

#include <coreplugin/coreplugin.h>
#include <texteditor/basehoverhandler.h>

#include <QPoint>

namespace pyqtc {

class HoverHandler : public QObject, public TextEditor::BaseHoverHandler {
  Q_OBJECT

public:
  HoverHandler(WorkerPool<WorkerClient>* worker_pool);

private slots:
  void TooltipResponse(WorkerClient::ReplyType* reply);

private:
  void identifyMatch(TextEditor::TextEditorWidget *editorWidget, int pos);
  void operateTooltip(TextEditor::TextEditorWidget* editor, const QPoint& point);

private:
  WorkerPool<WorkerClient>* worker_pool_;

  WorkerClient::ReplyType* current_reply_;
  TextEditor::TextEditorWidget* current_editor_;
  QPoint current_point_;
};

} // namespace

