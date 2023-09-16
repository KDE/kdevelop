/*
    SPDX-FileCopyrightText: 2015 Maciej Cencora

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "texteditorhelpers.h"

#include <KTextEditor/View>

#include <QRegularExpression>

#include <cstdlib>

namespace KDevelop {

namespace {

// TODO: this is a hack, but Kate does not provide interface for this
int lineHeight(const KTextEditor::View* view, int curLine)
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

QRect KTextEditorHelpers::itemBoundingRect(const KTextEditor::View* view, const KTextEditor::Range& itemRange)
{
    QPoint startPoint = view->mapToGlobal(view->cursorToCoordinate(itemRange.start()));
    QPoint endPoint = view->mapToGlobal(view->cursorToCoordinate(itemRange.end()));
    endPoint.ry() += lineHeight(view, itemRange.start().line());
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

    int line = match.capturedView(1).toInt() - 1;
    // captured(2) for pattern2 will yield null QString, toInt() thus 0, so no need for if-else
    // don't use an invalid column when the line is valid
    int column = qMax(0, match.capturedView(2).toInt() - 1);

    if (pathLength)
        *pathLength = match.capturedStart(0);
    return {
               line, column
    };
}

}
