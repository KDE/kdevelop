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

#ifndef CLANGLANG_CLANGPARSEJOB_H
#define CLANGLANG_CLANGPARSEJOB_H

#include <language/backgroundparser/parsejob.h>
#include <project/path.h>
#include <clang-c/Index.h>

#include "duchain/parsesession.h"
#include "duchain/includedfilecontexts.h"

class ClangLanguageSupport;

struct Import
{
    CXFile file;
    KDevelop::CursorInRevision location;
};

using Imports = QMultiHash<CXFile, Import>;

class ClangParseJob : public KDevelop::ParseJob
{
public:
    ClangParseJob(const KDevelop::IndexedString& url,
                  KDevelop::ILanguageSupport* languageSupport);

    ClangLanguageSupport* clang() const;

protected:
    virtual void run();

private:
    KDevelop::ReferencedTopDUContext buildDUChain(CXFile file, IncludeFileContexts &includedFiles);

    KDevelop::Path::List m_includes;
    QHash<QString, QString> m_defines;
    KSharedPtr<ParseSession> m_session;
    Imports m_imports;
};

#endif // CLANGLANG_CLANGPARSEJOB_H
