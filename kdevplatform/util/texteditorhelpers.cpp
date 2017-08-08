/* This file is part of the KDE project
   Copyright 2015 Maciej Cencora

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <cstdlib>

#include "texteditorhelpers.h"

#include <KTextEditor/View>

#include <QRegularExpression>

namespace KDevelop {

namespace {

// TODO: this is a hack, but Kate does not provide interface for this
int getLineHeight(const KTextEditor::View* view, int curLine)
{
  KTextEditor::Cursor c(curLine, 0);
  int currentHeight = view->cursorToCoordinate(c).y();
  c.setLine(curLine + 1);
  if (view->cursorToCoordinate(c).y() < 0) {
    c.setLine(curLine - 1);
  }
  return std::abs(view->cursorToCoordinate(c).y() - currentHeight);
}

}

QRect KTextEditorHelpers::getItemBoundingRect(const KTextEditor::View* view, const KTextEditor::Range& itemRange)
{
  QPoint startPoint = view->mapToGlobal(view->cursorToCoordinate(itemRange.start()));
  QPoint endPoint = view->mapToGlobal(view->cursorToCoordinate(itemRange.end()));
  endPoint.ry() += getLineHeight(view, itemRange.start().line());
  return QRect(startPoint, endPoint);
}

KTextEditor::Cursor KTextEditorHelpers::extractCursor(const QString& input, int* pathLength)
{
    // ":ll:cc", ":ll"
    static const QRegularExpression pattern(QStringLiteral(":(\\d+)(?::(\\d+))?$"));
    // "#Lll", "#nll", "#ll" as e.g. seen with repo web links
    static const QRegularExpression pattern2(QStringLiteral("#(?:n|L|)(\\d+)$"));

    auto match = pattern.match(input);

    if (!match.hasMatch()) {
        match = pattern2.match(input);
    }

    if (!match.hasMatch()) {
        if (pathLength)
            *pathLength = input.length();
        return KTextEditor::Cursor::invalid();
    }

    int line = match.capturedRef(1).toInt() - 1;
    // captured(2) for pattern2 will yield null QString, toInt() thus 0, so no need for if-else
    // don't use an invalid column when the line is valid
    int column = qMax(0, match.captured(2).toInt() - 1);

    if (pathLength)
        *pathLength = match.capturedStart(0);
    return {line, column};
}

}
