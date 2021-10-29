/*
    SPDX-FileCopyrightText: 2015 Maciej Cencora

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEXTEDITORHELPERS_H
#define KDEVPLATFORM_TEXTEDITORHELPERS_H

#include <QRect>
#include <KTextEditor/Range>

#include "utilexport.h"

namespace KTextEditor {
class View;
}

namespace KDevelop {

namespace KTextEditorHelpers {

/// @return Item's bounding rect in global screen coordinates
QRect KDEVPLATFORMUTIL_EXPORT itemBoundingRect(const KTextEditor::View* view, const KTextEditor::Range& itemRange);

/**
 * @brief Try parsing a string such as "path_to_file:line_num:column_num".
 *
 * Both line_num and column_num may be empty. Thus, other valid inputs are: "path_to_file", "path_to_file:line_num"
 * @param input Source string
 * @param pathLength Set to the length of the "path_to_file" string, you can use this to extract the raw path from the input string
 * @return Cursor representing line_num and column_num
 * */
KTextEditor::Cursor KDEVPLATFORMUTIL_EXPORT extractCursor(const QString& input, int* pathLength = nullptr);

}

}

#endif // KDEVPLATFORM_TEXTEDITORHELPERS_H
