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

#include "clangparsejob.h"

#include <language/backgroundparser/urlparselock.h>

#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/duchain.h>
#include <language/duchain/parsingenvironment.h>
#include <interfaces/ilanguage.h>

#include "duchain/parsesession.h"
#include "duchain/buildduchainvisitor.h"

#include "debug.h"
#include "clanglanguagesupport.h"

#include <QReadLocker>

using namespace KDevelop;

ClangParseJob::ClangParseJob(const IndexedString& url, ILanguageSupport* languageSupport)
: ParseJob(url, languageSupport)
{
}

ClangLanguageSupport* ClangParseJob::clang() const
{
    return static_cast<ClangLanguageSupport*>(languageSupport());
}

void ClangParseJob::run()
{
    UrlParseLock urlLock(document());
    if (abortRequested() || !isUpdateRequired(ParseSession::languageString())) {
        return;
    }

    ProblemPointer p = readContents();
    if (p) {
        //TODO: associate problem with topducontext
        return;
    }

    ParseSession session(document(), contents().contents, clang()->index());

    if (abortRequested()) {
        return;
    }

    ReferencedTopDUContext context;
    {
        DUChainReadLocker lock;
        context = DUChainUtils::standardContextForUrl(document().toUrl());
    }
    if (!context) {
        DUChainWriteLocker lock;
        ParsingEnvironmentFile *file = new ParsingEnvironmentFile(document());
        file->setLanguage(ParseSession::languageString());
        context = new TopDUContext(document(), RangeInRevision(0, 0, INT_MAX, INT_MAX), file);
        DUChain::self()->addDocumentChain(context);
    } else {
        //TODO: update existing contexts
        DUChainWriteLocker lock;
        context->cleanIfNotEncountered({});
    }
    setDuChain(context);
    {
        QReadLocker parseLock(languageSupport()->language()->parseLock());

        if (abortRequested()) {
            return abortJob();
        }

        BuildDUChainVisitor visitor;
        visitor.visit(session.unit(), context);
    }

    if (abortRequested()) {
        return abortJob();
    }

    {
        DUChainWriteLocker lock;
        context->setProblems(session.problems());

        context->setFeatures(minimumFeatures());
        ParsingEnvironmentFilePointer file = context->parsingEnvironmentFile();
        Q_ASSERT(file);
        file->setModificationRevision(contents().modification);
        DUChain::self()->updateContextEnvironment( context->topContext(), file.data() );
    }
    highlightDUChain();
}

