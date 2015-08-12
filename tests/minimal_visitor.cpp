/*
 * Copyright 2015  Milian Wolff <mail@milianw.de>
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
 */

#include <clang-c/Index.h>

#include <cstdio>

CXChildVisitResult visitCursor(CXCursor cursor, CXCursor /*parent*/, CXClientData /*data*/)
{
    auto range = clang_getCursorExtent(cursor);
    auto start = clang_getRangeStart(range);
    auto end = clang_getRangeEnd(range);
    CXFile file;
    unsigned startLine, startColumn;
    clang_getSpellingLocation(start, &file, &startLine, &startColumn, nullptr);
    unsigned endLine, endColumn;
    clang_getSpellingLocation(end, nullptr, &endLine, &endColumn, nullptr);

    auto str = clang_getCursorSpelling(cursor);
    auto fileStr = clang_getFileName(file);
    auto typeStr = clang_getCursorKindSpelling(cursor.kind);
    printf("\"%s\" [(%d, %d), (%d, %d)] in %s | %s\n", clang_getCString(str),
           startLine, startColumn, endLine, endColumn,
           clang_getCString(fileStr), clang_getCString(typeStr));

    clang_disposeString(str);
    clang_disposeString(fileStr);
    clang_disposeString(typeStr);
    return CXChildVisit_Recurse;
}

int main(int argc, char** argv)
{
    if (argc != 2)
        return 1;

    auto index = clang_createIndex(0, 0);

    CXTranslationUnit unit;
    clang_parseTranslationUnit2(index, argv[1], nullptr, 0, nullptr, 0, 0, &unit);

    auto tuCursor = clang_getTranslationUnitCursor(unit);
    clang_visitChildren(tuCursor, &visitCursor, nullptr);
    return 0;
}