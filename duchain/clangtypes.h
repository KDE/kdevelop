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

#ifndef CLANGTYPES_H
#define CLANGTYPES_H

#include <clang-c/Index.h>

#include <QSharedPointer>

#include "duchainexport.h"

namespace KDevelop {
class DocumentRange;
class SimpleRange;
class SimpleCursor;
}

class KDEVCLANGDUCHAIN_EXPORT ClangIndex
{
public:
    ClangIndex();
    ~ClangIndex();

    CXIndex index() const;

private:
    CXIndex m_index;
};

class KDEVCLANGDUCHAIN_EXPORT ClangString
{
public:
    ClangString(CXString string);
    ~ClangString();

    ClangString(const ClangString&) = delete;
    ClangString& operator=(const ClangString&) = delete;

    operator const char*() const;

private:
    CXString string;
};

class KDEVCLANGDUCHAIN_EXPORT ClangLocation
{
public:
    ClangLocation(CXSourceLocation cursor);
    ~ClangLocation();

    operator KDevelop::SimpleCursor() const;

private:
    CXSourceLocation location;
};

class KDEVCLANGDUCHAIN_EXPORT ClangRange
{
public:
    ClangRange(CXSourceRange range);

    ~ClangRange();

    ClangLocation start() const;
    ClangLocation end() const;

    KDevelop::DocumentRange toDocumentRange() const;

    KDevelop::SimpleRange toSimpleRange() const;

private:
    CXSourceRange range;
};

#endif // CLANGTYPES_H
