/*************************************************************************************
 *  Copyright (C) 2012 by Aleix Pol <aleixpol@kde.org>                               *
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

#include "parseqmljsjob.h"
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

ParseQmlJsJob::ParseQmlJsJob(const KUrl& url, KDevQmlJsPlugin* lang)
    : ParseJob(url)
    , m_lang(lang)
{}

KDevelop::SimpleRange locationToSimpleRange(QmlJS::AST::SourceLocation location)
{
    return SimpleRange(location.startLine, location.startColumn, location.startLine, location.length);
}

void ParseQmlJsJob::run()
{
    KDevelop::UrlParseLock urlLock(document());
    if ( !(minimumFeatures() & TopDUContext::ForceUpdate) ) {
        DUChainReadLocker lock;
        static const IndexedString langString("qml/js");
        foreach(const ParsingEnvironmentFilePointer &file, DUChain::self()->allEnvironmentFiles(document())) {
            if (file->language() != langString) {
                continue;
            }
            if (!file->needsUpdate() && file->featuresSatisfied(minimumFeatures())) {
                kDebug() << "Already up to date" << document().str();
                setDuChain(file->topContext());
                if (qmljs() && qmljs()->codeHighlighting()
                    && ICore::self()->languageController()->backgroundParser()->trackerForUrl(document()))
                {
                    lock.unlock();
                    qmljs()->codeHighlighting()->highlightDUChain(duChain());
                }
                return;
            }
            break;
        }
    }

    kDebug() << "parsing" << document().str();

    KDevelop::ProblemPointer p = readContents();
    if (p) {
        //TODO: associate problem with topducontext
        return abortJob();
    }

    KUrl url = document().toUrl();
    Q_ASSERT(url.isLocalFile() && "TODO: pull them if they're remote");
    QString path(url.toLocalFile());

    QmlJS::Document::MutablePtr doc = QmlJS::Document::create(path, QmlJS::Document::guessLanguageFromSuffix(path));
    // 2) parse
    bool matched = doc->isParsedCorrectly();

    if (abortRequested() || ICore::self()->shuttingDown()) {
        return abortJob();
    }

    KDevelop::ReferencedTopDUContext toUpdate;
    {
        KDevelop::DUChainReadLocker duchainlock(KDevelop::DUChain::lock());
        toUpdate = KDevelop::DUChainUtils::standardContextForUrl(document().toUrl());
    }

    KDevelop::TopDUContext::Features newFeatures = minimumFeatures();
    if (toUpdate)
        newFeatures = (KDevelop::TopDUContext::Features)(newFeatures | toUpdate->features());

    //Remove update-flags like 'Recursive' or 'ForceUpdate'
    newFeatures = static_cast<KDevelop::TopDUContext::Features>(newFeatures & KDevelop::TopDUContext::AllDeclarationsContextsUsesAndAST);

    if (matched) {
        if (abortRequested() || !qmljs() || !qmljs()->language()) {
            return abortJob();
        }

        QReadLocker parseLock(qmljs()->language()->parseLock());

        DeclarationBuilder builder(doc);
        KDevelop::ReferencedTopDUContext chain = builder.build(document(), doc->ast(), toUpdate);

        if (abortRequested()) {
            return abortJob();
        }

        setDuChain(chain);

//         if ( newFeatures & TopDUContext::AllDeclarationsContextsAndUses )
//         {
//             UseBuilder useBuilder(doc);
//             useBuilder.buildUses(ast);
//         }

        if (abortRequested()) {
            return abortJob();
        }

        {
            DUChainWriteLocker lock(DUChain::lock());

            foreach(const QmlJS::DiagnosticMessage& msg, doc->diagnosticMessages()) {
                KDevelop::ProblemPointer p(new KDevelop::Problem);
                p->setDescription(msg.message);
                p->setFinalLocation(DocumentRange(IndexedString(doc->fileName()), locationToSimpleRange(msg.loc)));
                chain->addProblem(p);
            }

            chain->setFeatures(newFeatures);
            ParsingEnvironmentFilePointer file = chain->parsingEnvironmentFile();
            file->setModificationRevision(contents().modification);
            DUChain::self()->updateContextEnvironment( chain->topContext(), file.data() );
        }

        if (qmljs() && qmljs()->codeHighlighting()
            && ICore::self()->languageController()->backgroundParser()->trackerForUrl(document()))
        {
            qmljs()->codeHighlighting()->highlightDUChain(chain);
        }
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
            KDevelop::ProblemPointer p(new KDevelop::Problem);
            p->setDescription(msg.message);
            p->setFinalLocation(DocumentRange(IndexedString(doc->fileName()), locationToSimpleRange(msg.loc)));
            top->addProblem(p);
        }
        setDuChain(top);
        kDebug() << "===Failed===" << document().str();
    }
}

