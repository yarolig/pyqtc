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

#include <cplusplus/Icons.h>
#include <texteditor/codeassist/completionassistprovider.h>
#include <texteditor/codeassist/functionhintproposal.h>
#include <texteditor/codeassist/iassistprocessor.h>
#include <texteditor/codeassist/ifunctionhintproposalmodel.h>
#include <texteditor/codeassist/genericproposalmodel.h>

#include "config.h"
#include "rpc.pb.h"
#include "workerclient.h"
#include "workerpool.h"

namespace TextEditor {
  class IAssistInterface;
}

namespace pyqtc {

class PythonIcons;

class CompletionAssistProvider : public TextEditor::CompletionAssistProvider {
public:
  CompletionAssistProvider(WorkerPool<WorkerClient>* worker_pool,
                           const PythonIcons* icons);

#ifdef QTC_HAS_CORE_ID
  bool supportsEditor(const Core::Id editorId) const;
#else
  bool supportsEditor(const QString& editorId) const;
#endif

  int activationCharSequenceLength() const;
  bool isActivationCharSequence(const QString& sequence) const;
  TextEditor::IAssistProcessor* createProcessor() const;

  static CompletionAssistProvider* instance();
private:
  WorkerPool<WorkerClient>* worker_pool_;
  const PythonIcons* icons_;

  // IAssistProvider interface
public:
  bool isAsynchronous() const;

  // CompletionAssistProvider interface
public:
  bool isContinuationChar(const QChar &c) const;
};


class CompletionAssistProcessor : public TextEditor::IAssistProcessor {
public:
  CompletionAssistProcessor(WorkerPool<WorkerClient>* worker_pool,
                            const PythonIcons* icons);

  TextEditor::IAssistProposal* perform(const TextEditor::AssistInterface* interface);
private:
  TextEditor::IAssistProposal* CreateCalltipProposal(
      int position, const QString& text);
  TextEditor::IAssistProposal* CreateCompletionProposal(
      const pb::CompletionResponse* response);

private:
  WorkerPool<WorkerClient>* worker_pool_;
  const PythonIcons* icons_;
};


class FunctionHintProposalModel : public TextEditor::IFunctionHintProposalModel {
public:
  FunctionHintProposalModel(const QString& text);

  void reset() {}
  int size() const { return 1; }
  QString text(int index) const;
  int activeArgument(const QString& prefix) const;

private:
  QString text_;
  mutable int current_arg_;
};

} // namespace pyqtc
