/*************************************************************************************
 *  Copyright (C) 2012 by Aleix Pol <aleixpol@kde.org>                               *
 *  Copyright (C) 2012 by Milian Wolff <mail@milianw.de>                             *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "qmljsparsejob.h"

#include <language/backgroundparser/urlparselock.h>

#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/duchain.h>
#include <language/duchain/parsingenvironment.h>
#include <interfaces/ilanguage.h>

#include "duchain/declarationbuilder.h"
#include "duchain/parsesession.h"
#include "duchain/usebuilder.h"

#include "debug.h"

#include <QReadLocker>

using namespace KDevelop;

QmlJsParseJob::QmlJsParseJob(const IndexedString& url, ILanguageSupport* languageSupport)
: ParseJob(url, languageSupport)
{}

void QmlJsParseJob::run()
{
    UrlParseLock urlLock(document());
    if (abortRequested() || !isUpdateRequired(ParseSession::languageString())) {
        return;
    }

    debug() << "parsing" << document().str();

    ProblemPointer p = readContents();
    if (p) {
        //TODO: associate problem with topducontext
        return;
    }

    ParseSession session(document(), contents().contents, priority());

    if (abortRequested()) {
        return;
    }

    ReferencedTopDUContext context;
    {
        DUChainReadLocker lock;
        context = DUChainUtils::standardContextForUrl(document().toUrl());
    }
    if (context) {
        translateDUChainToRevision(context);
        context->setRange(RangeInRevision(0, 0, INT_MAX, INT_MAX));
    }

    if (session.ast()) {
        QReadLocker parseLock(languageSupport()->language()->parseLock());

        if (abortRequested()) {
            return abortJob();
        }

        DeclarationBuilder builder(&session);
        context = builder.build(document(), session.ast(), context);

        if (abortRequested()) {
            abortJob();
            return;
        }

        if ( context && minimumFeatures() & TopDUContext::AllDeclarationsContextsAndUses ) {
            UseBuilder useBuilder(&session);
            useBuilder.buildUses(session.ast());
        }
    }

    if (abortRequested()) {
        return abortJob();
    }

    if (!context) {
        DUChainWriteLocker lock;
        ParsingEnvironmentFile *file = new ParsingEnvironmentFile(document());
        file->setLanguage(ParseSession::languageString());
        context = new TopDUContext(document(), RangeInRevision(0, 0, INT_MAX, INT_MAX), file);
        DUChain::self()->addDocumentChain(context);
    }

    setDuChain(context);
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

    if (session.isParsedCorrectly()) {
        debug() << "===Success===" << document().str();
    } else {
        debug() << "===Failed===" << document().str();
    }
}

