/*
    SPDX-FileCopyrightText: 2015 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
    clang_getFileLocation(start, &file, &startLine, &startColumn, nullptr);
    unsigned endLine, endColumn;
    clang_getFileLocation(end, nullptr, &endLine, &endColumn, nullptr);

    // skip builtin macros
    if (file || cursor.kind != CXCursor_MacroDefinition) {
        auto str = clang_getCursorSpelling(cursor);
        auto fileStr = clang_getFileName(file);
        auto typeStr = clang_getCursorKindSpelling(cursor.kind);

        printf("%s:%u:%u: - %u:%u: \"%s\"  %s %s\n", clang_getCString(fileStr), startLine, startColumn, endLine,
               endColumn, clang_getCString(str), clang_getCString(typeStr),
               clang_isCursorDefinition(cursor) ? "(definition)" : "");
        clang_disposeString(str);
        clang_disposeString(fileStr);
        clang_disposeString(typeStr);
    }
    return CXChildVisit_Recurse;
}

int main(int argc, char** argv)
{
    auto clangVersion = clang_getClangVersion();
    printf("%s\n\n", clang_getCString(clangVersion));
    clang_disposeString(clangVersion);

    if (argc != 2) {
        printf("Usage: clang-minimal-visitor file.cpp\n");
        return 1;
    }

    auto index = clang_createIndex(0, 0);

    unsigned int flags = CXTranslationUnit_DetailedPreprocessingRecord
#if CINDEX_VERSION_MINOR >= 34
        | CXTranslationUnit_KeepGoing;
#endif
    CXTranslationUnit unit;
    clang_parseTranslationUnit2(index, argv[1], nullptr, 0, nullptr, 0, flags, &unit);

    auto tuCursor = clang_getTranslationUnitCursor(unit);
    clang_visitChildren(tuCursor, &visitCursor, nullptr);

    clang_disposeTranslationUnit(unit);
    return 0;
}
