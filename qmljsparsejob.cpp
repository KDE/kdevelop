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
#include <qmljs/qmljsdocument.h>
#include "kdevqmljsplugin.h"
#include "duchain/declarationbuilder.h"

#include <QReadLocker>

using namespace KDevelop;

QmlJsParseJob::QmlJsParseJob(const IndexedString& url, ILanguageSupport* languageSupport)
: ParseJob(url, languageSupport)
{}

SimpleRange locationToSimpleRange(QmlJS::AST::SourceLocation location)
{
    return SimpleRange(location.startLine, location.startColumn, location.startLine, location.length);
}

void QmlJsParseJob::run()
{
    UrlParseLock urlLock(document());
    static const IndexedString langString("qml/js");
    if (abortRequested() || !isUpdateRequired(langString)) {
        return;
    }

    kDebug() << "parsing" << document().str();

    ProblemPointer p = readContents();
    if (p) {
        //TODO: associate problem with topducontext
        return abortJob();
    }

    const QString url = document().str();
    QmlJS::Document::MutablePtr doc = QmlJS::Document::create(url, QmlJS::Document::guessLanguageFromSuffix(url));
    doc->setSource(contents().contents);
    // 2) parse
    const bool successfullyParsed = doc->parse();

    if (abortRequested() || ICore::self()->shuttingDown()) {
        return abortJob();
    }

    ReferencedTopDUContext toUpdate;
    {
        DUChainReadLocker duchainlock(DUChain::lock());
        toUpdate = DUChainUtils::standardContextForUrl(document().toUrl());
    }

    if (successfullyParsed) {
        if (abortRequested()) {
            return abortJob();
        }

        QReadLocker parseLock(languageSupport()->language()->parseLock());

        DeclarationBuilder builder(doc);
        ReferencedTopDUContext chain = builder.build(document(), doc->ast(), toUpdate);

        if (abortRequested()) {
            return abortJob();
        }

        setDuChain(chain);

        if (abortRequested()) {
            return abortJob();
        }

        {
            DUChainWriteLocker lock(DUChain::lock());

            foreach(const QmlJS::DiagnosticMessage& msg, doc->diagnosticMessages()) {
                ProblemPointer p(new Problem);
                p->setDescription(msg.message);
                p->setFinalLocation(DocumentRange(IndexedString(doc->fileName()), locationToSimpleRange(msg.loc)));
                chain->addProblem(p);
            }

            chain->setFeatures(minimumFeatures());
            ParsingEnvironmentFilePointer file = chain->parsingEnvironmentFile();
            file->setModificationRevision(contents().modification);
            DUChain::self()->updateContextEnvironment( chain->topContext(), file.data() );
        }

        highlightDUChain();
    } else {
        ReferencedTopDUContext top;
        DUChainWriteLocker lock;
        {
            top = DUChain::self()->chainForDocument(document());
        }
        if (top) {
            ///NOTE: if we clear the imported parent contexts, autocompletion of built-in PHP stuff won't work!
            //top->clearImportedParentContexts();
            top->parsingEnvironmentFile()->clearModificationRevisions();
            top->clearProblems();
        } else {
            ParsingEnvironmentFile *file = new ParsingEnvironmentFile(document());
            /// Indexed string for 'Php', identifies environment files from this language plugin
            static const IndexedString qmljsLangString("qml/js");
            file->setLanguage(qmljsLangString);
            top = new TopDUContext(document(), RangeInRevision(0, 0, INT_MAX, INT_MAX), file);
            DUChain::self()->addDocumentChain(top);
        }
        foreach(const QmlJS::DiagnosticMessage& msg, doc->diagnosticMessages()) {
            ProblemPointer p(new Problem);
            p->setDescription(msg.message);
            p->setFinalLocation(DocumentRange(IndexedString(doc->fileName()), locationToSimpleRange(msg.loc)));
            top->addProblem(p);
        }
        setDuChain(top);
        kDebug() << "===Failed===" << document().str();
    }
}

