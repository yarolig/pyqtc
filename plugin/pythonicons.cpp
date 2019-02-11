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

#include "pythonicons.h"
#include "protostring.h"

#include <utils/utilsicons.h>


using namespace pyqtc;

QIcon PythonIcons::IconForCompletionProposal(
      const pb::CompletionResponse_Proposal& proposal) const {
  Utils::CodeModelIcon::Type type = Utils::CodeModelIcon::Unknown;

  const bool is_private = ProtoStringToQString(proposal.name()).startsWith("_");

  // Keywords are treated differently
  switch (proposal.scope()) {
  case pb::CompletionResponse_Proposal_Scope_KEYWORD:
    type = Utils::CodeModelIcon::Keyword;
    break;

  default:
    // Continue to look at the type
    switch (proposal.type()) {
    case pb::CompletionResponse_Proposal_Type_INSTANCE:
      type = is_private ?
            Utils::CodeModelIcon::VarPrivate:
            Utils::CodeModelIcon::VarPublic;
      break;

    case pb::CompletionResponse_Proposal_Type_CLASS:
      type = Utils::CodeModelIcon::Class;
      break;

    case pb::CompletionResponse_Proposal_Type_FUNCTION:
      type = is_private ?
            Utils::CodeModelIcon::FuncPrivate:
            Utils::CodeModelIcon::FuncPublic;
      break;

    case pb::CompletionResponse_Proposal_Type_MODULE:
      type = Utils::CodeModelIcon::Namespace;
      break;
    }
    break;
  }

  return Utils::CodeModelIcon::iconForType(type);
}

QIcon PythonIcons::IconForSearchResult(const pb::SearchResponse_Result& result) const {
  Utils::CodeModelIcon::Type type = Utils::CodeModelIcon::Unknown;

  const bool is_private = ProtoStringToQString(result.symbol_name()).startsWith("_");

  switch (result.symbol_type()) {
  case pb::VARIABLE:
    type = is_private ?
          Utils::CodeModelIcon::VarPrivate:
          Utils::CodeModelIcon::VarPublic;
    break;

  case pb::CLASS:
    type = Utils::CodeModelIcon::Class;
    break;

  case pb::FUNCTION:
    type = is_private ?
          Utils::CodeModelIcon::FuncPrivate:
          Utils::CodeModelIcon::FuncPublic;
    break;

  case pb::MODULE:
    type = Utils::CodeModelIcon::Namespace;
    break;
  }

  return Utils::CodeModelIcon::iconForType(type);
}
