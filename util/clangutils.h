/*
 * Copyright 2014 Kevin Funk <kfunk@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef CLANGUTILS_H
#define CLANGUTILS_H

#include <util/path.h>

#include <clang-c/Index.h>

namespace KDevelop {
    class IndexedString;
}

namespace ClangUtils
{
    /**
     * Finds the most specific CXCursor which applies to the the specified line and column
     * in the given translation unit and file.
     *
     * @param line The 0-indexed line number at which to search.
     * @param column The 0-indexed column number at which to search.
     * @param unit The translation unit to examine.
     * @param file The file in the translation unit to examine.
     *
     * @return The cursor at the specified location
     */
    CXCursor getCXCursor(int line, int column, const CXTranslationUnit& unit, const CXFile& file);

    /**
     * Given a cursor representing a function, returns a vector containing the string
     * representations of the default arguments of the function which are defined at
     * the occurance of the cursor. Note that this is not necessarily all of the default
     * arguments of the function. The vector will have length equal to the number of
     * arguments to the function, and any arguments without a default parameter will be
     * represented with an empty string.
     *
     * @param cursor The cursor to examine
     * @return a vector of QStrings representing the default arguments, or an empty
     *         vector if cursor does not represent a function
     */
    QVector<QString> getDefaultArguments(CXCursor cursor);

    /**
     * Given a cursor and destination context, returns the string representing the
     * cursor's scope at its current location.
     *
     * @param cursor The cursor to examine
     * @return the cursor's scope as a string
     */
    QString getScope(CXCursor cursor);

    /**
     * Given a cursor representing some sort of function, returns its signature. The
     * effect of this function when passed a non-function cursor is undefined.
     *
     * @param cursor The cursor to work with
     * @param prefix The scope prefix of the cursor
     * @return A QString of the function's signature
     */
    QString getCursorSignature(CXCursor cursor, QString& prefix, QVector<QString> defaultArgs = QVector<QString>());

    /**
     * TODO remove once we depend on Clang 3.5
     * Given a cursor, returns true if and only if the function is a
     * const qualified class method.
     */
    bool isConstMethod(CXCursor cursor);
};

#endif // CLANGUTILS_H
