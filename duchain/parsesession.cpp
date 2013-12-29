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

#include "parsesession.h"
#include "clangindex.h"
#include "debug.h"

using namespace KDevelop;

IndexedString ParseSession::languageString()
{
    static const IndexedString lang("Clang");
    return lang;
}

ParseSession::ParseSession(const IndexedString& url, const QByteArray& contents, ClangIndex* index)
    : m_url(url)
    , m_unit(0)
{
    static const unsigned int flags
        = CXTranslationUnit_CacheCompletionResults
        | CXTranslationUnit_PrecompiledPreamble
        | CXTranslationUnit_Incomplete
        | CXTranslationUnit_CXXChainedPCH
        | CXTranslationUnit_ForSerialization;

    static const int argsSize = 4;
    static const char* args[argsSize] = { "-std=c++11", "-fsyntax-only", "-x", "c++" };

    // TODO: track other open unsaved files and add them here
    CXUnsavedFile file;
    file.Contents = contents.constData();
    file.Length = contents.size();
    file.Filename = url.c_str();

    m_unit = clang_parseTranslationUnit(
        index->index(), file.Filename,
        args, argsSize,
        &file, 1,
        flags
    );
}

ParseSession::~ParseSession()
{
    clang_disposeTranslationUnit(m_unit);
}

IndexedString ParseSession::url() const
{
    return m_url;
}

QList<ProblemPointer> ParseSession::problems() const
{
    return m_problems;
}

CXTranslationUnit ParseSession::unit() const
{
    return m_unit;
}
