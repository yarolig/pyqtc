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

#include "pythonindenter.h"

#include <QTextBlock>
#include <QTextDocument>

using namespace pyqtc;


PythonIndenter::PythonIndenter(QTextDocument *doc)
    : TextEditor::TextIndenter(doc)
{
}

int PythonIndenter::indentFor(const QTextBlock &block,
                              const TextEditor::TabSettings &tab_settings,
                              int /*cursorPositionInEditor*/)
{
    if (!block.isValid())
        return 0;

    // Find the last non-empty block
    QTextBlock previous = block.previous();
    QString previous_text;

    forever {
      if (!previous.isValid()) {
        // We reached the beginning of the document - do nothing
        return 0;
      }

      // Get the text of this block, if it's not empty then we're done.
      previous_text = previous.text();
      if (!previous_text.isEmpty() && !previous_text.trimmed().isEmpty()) {
        break;
      }

      // Try another block
      previous = previous.previous();
    }

    // Start with the indentation of the previous line
    int indentation_amount = -1;
    int i = 0;
    while (i < previous_text.size()) {
      if (!previous_text.at(i).isSpace()) {
        indentation_amount = tab_settings.columnAt(previous_text, i);
        break;
      }
      ++i;
    }

    if (indentation_amount == -1) {
      return 0;
    }

    // If the previous line ended with a colon, increase the indentation one level
    if (previous_text.endsWith(':')) {
      indentation_amount += tab_settings.m_indentSize;
    } else {
      const QString previous_trimmed = previous_text.trimmed();

      if (previous_trimmed == "continue" ||
          previous_trimmed == "break" ||
          previous_trimmed == "pass" ||
          previous_trimmed == "return" ||
          previous_trimmed == "raise" ||
          previous_trimmed.startsWith("raise ") ||
          previous_trimmed.startsWith("return ")) {
        indentation_amount -= tab_settings.m_indentSize;
      }
    }

    // tab_settings.indentLine(block, indentation_amount);
    return indentation_amount;
}
