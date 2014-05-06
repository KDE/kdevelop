/*
    This file is part of KDevelop

    Copyright 2013 Olivier de Gaalon <olivier.jg@gmail.com>
    Copyright 2013 Milian Wolff <mail@milianw.de>

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

#include "clangtypes.h"

#include <QFileInfo>
#include <QReadLocker>

#include <language/editor/documentcursor.h>
#include <language/editor/documentrange.h>
#include <language/backgroundparser/urlparselock.h>
#include <util/path.h>

#include "clangpch.h"

using namespace KDevelop;

namespace {

template<typename T>
inline T cursorForCXSrcLoc(CXSourceLocation loc)
{
    uint line = 0;
    uint column = 0;
    clang_getFileLocation(loc, 0, &line, &column, 0);
    return {static_cast<int>(line-1), static_cast<int>(column-1)};
}

}

ClangIndex::ClangIndex()
    : m_index(clang_createIndex(1 /*Exclude PCH Decls*/, qgetenv("KDEV_CLANG_DISPLAY_DIAGS") == "1" /*Display diags*/))
{
}

CXIndex ClangIndex::index() const
{
    return m_index;
}

QSharedPointer<const ClangPCH> ClangIndex::pch(const Path& pchInclude, const Path::List& includePaths, const QHash<QString, QString>& defines)
{
    if (!pchInclude.isValid()) {
        return {};
    }

    UrlParseLock pchLock(pchInclude.toIndexed());

    static const QString pchExt = QString::fromLatin1(".pch");

    if (QFile::exists(pchInclude.toLocalFile() + pchExt)) {
        QReadLocker lock(&m_pchLock);
        auto pch = m_pch.constFind(pchInclude);
        if (pch != m_pch.constEnd()) {
            return pch.value();
        }
    }

    QSharedPointer<const ClangPCH> pch(new ClangPCH(pchInclude, includePaths, defines, this));
    QWriteLocker lock(&m_pchLock);
    m_pch.insert(pchInclude, pch);
    return pch;
}

ClangIndex::~ClangIndex()
{
    clang_disposeIndex(m_index);
}

ClangString::ClangString(CXString string)
    : string(string)
{
}

ClangString::~ClangString()
{
    clang_disposeString(string);
}

const char* ClangString::c_str() const
{
    return clang_getCString(string);
}

ClangString::operator const char*() const
{
    const char *data = clang_getCString(string);
    return data ? data : "";
}

QString ClangString::toString() const
{
    return QString::fromUtf8(clang_getCString(string));
}

ClangLocation::ClangLocation(CXSourceLocation location)
    : location(location)
{

}

ClangLocation::operator DocumentCursor() const
{
    uint line = 0;
    uint column = 0;
    CXFile file;
    clang_getFileLocation(location, &file, &line, &column, 0);
    ClangString fileName(clang_getFileName(file));
    return {IndexedString(fileName), {static_cast<int>(line-1), static_cast<int>(column-1)}};
}

ClangLocation::operator SimpleCursor() const
{
    return cursorForCXSrcLoc<SimpleCursor>(location);
}

ClangLocation::operator CursorInRevision() const
{
    return cursorForCXSrcLoc<CursorInRevision>(location);
}

ClangLocation::operator CXSourceLocation() const
{
    return location;
}

ClangLocation::~ClangLocation()
{
}

ClangRange::ClangRange(CXSourceRange range)
    : m_range(range)
{

}

ClangLocation ClangRange::start() const
{
    return {clang_getRangeStart(m_range)};
}

ClangLocation ClangRange::end() const
{
    return {clang_getRangeEnd(m_range)};
}

CXSourceRange ClangRange::range() const
{
    return m_range;
}

DocumentRange ClangRange::toDocumentRange() const
{
    auto start = clang_getRangeStart(m_range);
    CXFile file;
    clang_getFileLocation(start, &file, 0, 0, 0);
    ClangString fileName(clang_getFileName(file));
    return {IndexedString(fileName), toSimpleRange()};
}

SimpleRange ClangRange::toSimpleRange() const
{
    return {start(), end()};
}

RangeInRevision ClangRange::toRangeInRevision() const
{
    return {start(), end()};
}

ClangRange::~ClangRange()
{
}
