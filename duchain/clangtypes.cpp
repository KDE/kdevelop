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

#include <language/editor/documentrange.h>

using namespace KDevelop;

ClangIndex::ClangIndex()
    : m_index(clang_createIndex(1 /*Exclude PCH Decls*/, 1 /*Display diags*/))
{
}

CXIndex ClangIndex::index() const
{
    return m_index;
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

ClangString::operator const char*() const
{
    return clang_getCString(string);
}

ClangLocation::ClangLocation(CXSourceLocation location)
    : location(location)
{

}

ClangLocation::operator SimpleCursor() const
{
    uint line = 0;
    uint column = 0;
    clang_getFileLocation(location, 0, &line, &column, 0);
    return {static_cast<int>(line), static_cast<int>(column)};
}

ClangLocation::~ClangLocation()
{
}

ClangRange::ClangRange(CXSourceRange range)
    : range(range)
{

}

ClangLocation ClangRange::start() const
{
    return {clang_getRangeStart(range)};
}

ClangLocation ClangRange::end() const
{
    return {clang_getRangeEnd(range)};
}

DocumentRange ClangRange::toDocumentRange() const
{
    auto start = clang_getRangeStart(range);
    CXFile file;
    clang_getFileLocation(start, &file, 0, 0, 0);
    ClangString fileName(clang_getFileName(file));
    return {IndexedString(fileName), toSimpleRange()};
}

SimpleRange ClangRange::toSimpleRange() const
{
    return {start(), end()};
}

ClangRange::~ClangRange()
{
}
