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
#include <language/backgroundparser/backgroundparser.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/duchain.h>
#include <language/duchain/parsingenvironment.h>
#include <language/interfaces/icodehighlighting.h>
#include <interfaces/ilanguage.h>
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>

#include "duchain/declarationbuilder.h"
#include "duchain/parsesession.h"

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

    kDebug() << "parsing" << document().str();

    ProblemPointer p = readContents();
    if (p) {
        //TODO: associate problem with topducontext
        return;
    }

    ParseSession session(document(), contents().contents);
    // 2) parse
    const bool successfullyParsed = session.isParsedCorrectly();

    if (abortRequested()) {
        return;
    }

    ReferencedTopDUContext toUpdate;
    {
        DUChainReadLocker lock;
        toUpdate = DUChainUtils::standardContextForUrl(document().toUrl());
    }

    if (successfullyParsed) {
        if (abortRequested()) {
            return abortJob();
        }

        QReadLocker parseLock(languageSupport()->language()->parseLock());

        DeclarationBuilder builder(&session);
        ReferencedTopDUContext chain = builder.build(document(), session.ast(), toUpdate);

        if (abortRequested()) {
            return abortJob();
        }

        setDuChain(chain);

        if (abortRequested()) {
            return abortJob();
        }

        {
            DUChainWriteLocker lock(DUChain::lock());

            foreach(const ProblemPointer& problem, session.problems()) {
                chain->addProblem(problem);
            }

            chain->setFeatures(minimumFeatures());
            ParsingEnvironmentFilePointer file = chain->parsingEnvironmentFile();
            file->setModificationRevision(contents().modification);
            DUChain::self()->updateContextEnvironment( chain->topContext(), file.data() );
        }

        highlightDUChain();
    } else {
        DUChainWriteLocker lock;
        if (toUpdate) {
            toUpdate->clearImportedParentContexts();
            toUpdate->parsingEnvironmentFile()->clearModificationRevisions();
            toUpdate->clearProblems();
        } else {
            ParsingEnvironmentFile *file = new ParsingEnvironmentFile(document());
            file->setLanguage(ParseSession::languageString());
            toUpdate = new TopDUContext(document(), RangeInRevision(0, 0, INT_MAX, INT_MAX), file);
            DUChain::self()->addDocumentChain(toUpdate);
        }
        foreach(const ProblemPointer& problem, session.problems()) {
            toUpdate->addProblem(problem);
        }
        setDuChain(toUpdate);
        kDebug() << "===Failed===" << document().str();
    }
}

