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

    enum DefaultArgumentsMode
    {
        FixedSize, ///< The vector will have length equal to the number of arguments to the function
                   /// and any arguments without a default parameter will be represented with an empty string.
        MinimumSize ///< The vector will have a length equal to the number of default values
    };

    /**
     * Given a cursor representing a function, returns a vector containing the string
     * representations of the default arguments of the function which are defined at
     * the occurance of the cursor. Note that this is not necessarily all of the default
     * arguments of the function.
     *
     * @param cursor The cursor to examine
     * @return a vector of QStrings representing the default arguments, or an empty
     *         vector if cursor does not represent a function
     */
    QVector<QString> getDefaultArguments(CXCursor cursor, DefaultArgumentsMode mode = FixedSize);

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
     * @param scope The scope of the cursor (e.g. "SomeNS::SomeClass")
     * @return A QString of the function's signature
     */
    QString getCursorSignature(CXCursor cursor, const QString& scope, QVector<QString> defaultArgs = QVector<QString>());

    /**
     * Extract the raw contents of the range @p range
     *
     * @note This will return the exact textual representation of the code,
     *   no whitespace stripped, etc.
     *
     * TODO: It would better if we'd be able to just memcpy parts of the file buffer
     * that's stored inside Clang (cf. llvm::MemoryBuffer for files), but libclang
     * doesn't offer API for that. This implementation here is a lot more expensive.
     *
     * @param unit Translation unit this range is part of
     */
    QByteArray getRawContents(CXTranslationUnit unit, CXSourceRange range);

    /**
     * TODO remove once we depend on Clang 3.5
     * Given a cursor, returns true if and only if the function is a
     * const qualified class method.
     */
    bool isConstMethod(CXCursor cursor);
};

#endif // CLANGUTILS_H
