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
