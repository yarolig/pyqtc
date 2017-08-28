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

#include "closure.h"
#include "hoverhandler.h"
#include "rpc.pb.h"

#include <coreplugin/idocument.h>
#include <texteditor/texteditor.h>
#include <utils/fileutils.h>
#include <texteditor/textdocument.h>
//#include <utils/tooltip/tipcontents.h>
#include <utils/tooltip/tooltip.h>

using namespace pyqtc;

HoverHandler::HoverHandler(WorkerPool<WorkerClient>* worker_pool)
    : worker_pool_(worker_pool),
      current_reply_(NULL),
      current_editor_(NULL)
{
}

bool HoverHandler::acceptEditor(Core::IEditor* editor) {
    return true;
}

void HoverHandler::identifyMatch(TextEditor::TextEditorWidget *editorWidget, int pos) {
    current_reply_ = worker_pool_->NextHandler()->Tooltip(
                editorWidget->textDocument()->filePath().toString(),
                editorWidget->textDocument()->plainText(),
                pos);

    NewClosure(current_reply_, SIGNAL(Finished(bool)),
               this, SLOT(TooltipResponse(WorkerClient::ReplyType*)),
               current_reply_);
}

void HoverHandler::TooltipResponse(WorkerClient::ReplyType* reply) {
    reply->deleteLater();

    if (!reply->is_successful() || reply != current_reply_)
        return;

    const QString& text = reply->message().tooltip_response().rich_text();
    if (current_editor_) {
        if (text.isEmpty()) {
            Utils::ToolTip::instance()->hide();
        } else {
            Utils::ToolTip::instance()->show(
                        current_point_,
                        text, // !!! Was: Utils::TextContent(text),
                        current_editor_);
        }
    }

    current_reply_ = NULL;
}

void HoverHandler::operateTooltip(TextEditor::TextEditorWidget *editor,
                                  const QPoint& point) {
    current_editor_ = editor;
    current_point_ = point;
}
