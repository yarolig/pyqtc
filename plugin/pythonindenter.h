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

#include <texteditor/indenter.h>
#include <texteditor/textindenter.h>
#include <texteditor/tabsettings.h>

#include <QRegExp>

namespace pyqtc {

class PythonIndenter : public TextEditor::TextIndenter {
public:
  PythonIndenter(QTextDocument *doc);

  int indentFor(const QTextBlock &block,
                const TextEditor::TabSettings &tab_settings,
                int cursorPositionInEditor = -1) override;
};

} // namespace pyqtc

