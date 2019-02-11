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


#include "completionassist.h"
#include "constants.h"
#include "protostring.h"
#include "pythonicons.h"
#include "workerclient.h"
#include "workerpool.h"

#include <coreplugin/id.h>
#include <coreplugin/idocument.h>
#include <texteditor/codeassist/functionhintproposal.h>
#include <texteditor/codeassist/functionhintproposalwidget.h>
#include <texteditor/codeassist/genericproposal.h>
#include <texteditor/codeassist/assistinterface.h>
#include <texteditor/codeassist/assistproposalitem.h>
#include <texteditor/codeassist/assistproposaliteminterface.h>


#include <QApplication>
#include <QStack>
#include <QTextDocument>
#include <QtDebug>

using namespace pyqtc;

CompletionAssistProvider* m_instance = 0;

CompletionAssistProvider::CompletionAssistProvider(WorkerPool<WorkerClient>* worker_pool,
                                                   const PythonIcons* icons)
  : worker_pool_(worker_pool),
    icons_(icons)
{
    m_instance = this;
}

#ifdef QTC_HAS_CORE_ID
bool CompletionAssistProvider::supportsEditor(const Core::Id editorId) const {
  return editorId == Core::Id(constants::kEditorId);
}
#else
bool CompletionAssistProvider::supportsEditor(const QString& editorId) const {
  return editorId == constants::kEditorId;
}
#endif

int CompletionAssistProvider::activationCharSequenceLength() const {
  return 1;
}

bool CompletionAssistProvider::isActivationCharSequence(const QString& sequence) const {
  return sequence == "." || sequence == "(" || sequence == ",";
}

TextEditor::IAssistProcessor* CompletionAssistProvider::createProcessor() const {
  return new CompletionAssistProcessor(worker_pool_, icons_);
}

bool CompletionAssistProvider::isAsynchronous() const {
    return true;
}

bool CompletionAssistProvider::isContinuationChar(const QChar &c) const {
    return false;
}

CompletionAssistProcessor::CompletionAssistProcessor(WorkerPool<WorkerClient>* worker_pool,
      const PythonIcons* icons)
  : worker_pool_(worker_pool),
    icons_(icons)
{
}

TextEditor::IAssistProposal* CompletionAssistProcessor::perform(const TextEditor::AssistInterface *interface) {
  QScopedPointer<const TextEditor::AssistInterface> scoped_interface(interface);

  switch (interface->reason()) {
  case TextEditor::ActivationCharacter:
  case TextEditor::ExplicitlyInvoked:
    break;

  case TextEditor::IdleEditor:
    // Only trigger completions if at least 3 characters of a name have been
    // typed.
    for (int i=1 ; i<=3 ; ++i) {
      QChar c(interface->characterAt(interface->position() - i));
      if (!c.isLetterOrNumber()) {
        return NULL;
      }
    }
    break;
  }

  QScopedPointer<WorkerClient::ReplyType> reply(
      worker_pool_->NextHandler()->Completion(
        interface->fileName(),
        interface->textDocument()->toPlainText(),
        interface->position()));
  reply->WaitForFinished();

  if (!reply->is_successful())
    return NULL;

  const pb::CompletionResponse* response = &reply->message().completion_response();

  if (response->has_calltip()) {
    return CreateCalltipProposal(response->insertion_position(),
                                 ProtoStringToQString(response->calltip()));
  }

  if (response->proposal_size()) {
    return CreateCompletionProposal(response);
  }

  return NULL;
}

TextEditor::IAssistProposal* CompletionAssistProcessor::CreateCompletionProposal(
    const pb::CompletionResponse* response) {
  QList<TextEditor::AssistProposalItemInterface*> items;

  foreach (const pb::CompletionResponse_Proposal& proposal,
           response->proposal()) {
    TextEditor::AssistProposalItem* item = new TextEditor::AssistProposalItem;
    item->setText(ProtoStringToQString(proposal.name()));
    item->setIcon(icons_->IconForCompletionProposal(proposal));

    items << item;
  }

  return new TextEditor::GenericProposal(
        response->insertion_position(),
        QList<TextEditor::AssistProposalItemInterface *>(items));
}

TextEditor::IAssistProposal* CompletionAssistProcessor::CreateCalltipProposal(
    int position, const QString& text) {
  FunctionHintProposalModel* model =
      new FunctionHintProposalModel(text);
  return new TextEditor::FunctionHintProposal(position, model);
}


FunctionHintProposalModel::FunctionHintProposalModel(const QString& text)
  : text_(text),
    current_arg_(0)
{
}

QString FunctionHintProposalModel::text(int index) const {
  const int open_paren    = text_.indexOf('(');
  if (open_paren == -1) {
    return text_;
  }

  const int close_paren   = text_.lastIndexOf(')');
  const int last_dot      = qMax(0, text_.lastIndexOf('.', open_paren));
  const QString args_str  = text_.mid(open_paren + 1,
                                     close_paren - open_paren - 1);
  const QStringList args  = args_str.split(',');

  QStringList rich_args;
  for (int i=0 ; i<args.count() ; ++i) {
    const QString arg_trimmed = args[i].trimmed().toHtmlEscaped();

    if (current_arg_ == i) {
      rich_args << QString("<b>%1</b>").arg(arg_trimmed);
    } else {
      rich_args << arg_trimmed;
    }
  }

  // Pick a color between the tooltip background and foreground for the module
  // name.
  const QPalette& palette = qApp->palette();
  const QColor& foreground = palette.color(QPalette::ToolTipText);

  return QString("<font style=\"color: rgba(%1, %2, %3, 75%)\">%4</font>%5(%6)").arg(
        QString::number(foreground.red()),
        QString::number(foreground.green()),
        QString::number(foreground.blue()),
        QString(text_.left(last_dot).toHtmlEscaped()),
        QString(text_.mid(last_dot, open_paren - last_dot).toHtmlEscaped()),
        rich_args.join(", "));
}

int FunctionHintProposalModel::activeArgument(const QString& prefix) const {
  QStack<QChar> expecting_end_char;
  int arg = 0;

  foreach (const QChar& c, prefix) {
    if (!expecting_end_char.isEmpty()) {
      // We're in some nested scope, waiting to find the end character
      if (c == expecting_end_char.top()) {
        expecting_end_char.pop();
      }
      continue;
    }

    if      (c == ',')  arg ++;
    else if (c == '(')  expecting_end_char.push(')');
    else if (c == '[')  expecting_end_char.push(']');
    else if (c == '{')  expecting_end_char.push('}');
    else if (c == '\'') expecting_end_char.push('\'');
    else if (c == '"')  expecting_end_char.push('"');
    else if (c == ')') {
      return -1;
    }
  }

  current_arg_ = arg;
  return arg;
}



CompletionAssistProvider *CompletionAssistProvider::instance() {
    return m_instance;
}
