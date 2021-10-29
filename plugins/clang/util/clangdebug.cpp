/*
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "clangdebug.h"

#include "clangtypes.h"

#include <language/editor/documentcursor.h>
#include <language/editor/documentrange.h>

// Keep in sync with declare_qt_logging_category call
const QtMsgType defaultMsgType = QtInfoMsg;
Q_LOGGING_CATEGORY(KDEV_CLANG, "kdevelop.plugins.clang", defaultMsgType)

using namespace KDevelop;

QDebug operator<<(QDebug dbg, CXString string)
{
    dbg << ClangString(string).c_str();
    return dbg;
}

QDebug operator<<(QDebug dbg, CXSourceLocation location)
{
    dbg << DocumentCursor(ClangLocation(location));
    return dbg;
}

QDebug operator<<(QDebug dbg, CXSourceRange range)
{
    dbg << ClangRange(range).toDocumentRange();
    return dbg;
}

QDebug operator<<(QDebug dbg, CXCursor cursor)
{
    return dbg << clang_getCursorKind(cursor) << clang_getCursorDisplayName(cursor) << clang_getCursorType(cursor) << clang_getCursorLocation(cursor);
}

QDebug operator<<(QDebug dbg, CXCursorKind kind)
{
    return dbg << clang_getCursorKindSpelling(kind);
}

QDebug operator<<(QDebug dbg, CXType type)
{
    return dbg << type.kind << clang_getTypeSpelling(type);
}

QDebug operator<<(QDebug dbg, CXTypeKind typeKind)
{
    return dbg << clang_getTypeKindSpelling(typeKind);
}
