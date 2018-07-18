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


#include "clangdebug.h"

#include "clangtypes.h"

#include <language/editor/documentcursor.h>
#include <language/editor/documentrange.h>

const QtMsgType defaultMsgType = QtInfoMsg;
Q_LOGGING_CATEGORY(KDEV_CLANG, "kdevelop.languages.clang", defaultMsgType)

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
