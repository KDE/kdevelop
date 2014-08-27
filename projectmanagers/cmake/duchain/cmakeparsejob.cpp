/* KDevelop CMake Support
 *
 * Copyright 2014 Aleix Pol <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "cmakeparsejob.h"
#include "declarationbuilder.h"
#include <cmakelistsparser.h>
#include <cmakemanager.h>
#include <language/backgroundparser/urlparselock.h>
#include <language/backgroundparser/backgroundparser.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <interfaces/ilanguage.h>
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <language/duchain/problem.h>
#include <kio/global.h>
#include <QReadWriteLock>

using namespace KDevelop;

CMakeParseJob::CMakeParseJob(const KDevelop::IndexedString& url, KDevelop::ILanguageSupport* languageSupport)
    : ParseJob(url, languageSupport)
{
}

IndexedString parentCMakeFile(const IndexedString& doc)
{
    return IndexedString(QUrl(KIO::upUrl(doc.toUrl().adjusted(QUrl::RemoveFilename)).toString()+"/CMakeLists.txt"));
}

void CMakeParseJob::run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread* thread)
{
    const IndexedString languageName(CMakeManager::languageName());

    UrlParseLock urlLock(document());
    if (abortRequested() || !isUpdateRequired(languageName)) {
        return;
    }

    ProblemPointer p = readContents();
    if (p) {
        //TODO: associate problem with topducontext
        return;
    }

    ReferencedTopDUContext parentCtx;
    if (document().str().endsWith(QStringLiteral("CMakeLists.txt"))) {
        IndexedString parentFile = parentCMakeFile(document());
        if (QFile::exists(parentFile.toUrl().toLocalFile())) {

//         TODO: figure out includes import
            {
                DUChainReadLocker lock;
                parentCtx = DUChain::self()->chainForDocument(parentFile);
            }

            if (!parentCtx && ICore::self()->languageController()->backgroundParser()->parseJobForDocument(parentFile)) {
                qDebug() << "waiting for..." << parentFile << document();
                parentCtx = DUChain::self()->waitForUpdate(parentFile, TopDUContext::AllDeclarationsAndContexts);
            } else if(!parentCtx) {
                qDebug() << "required..." << parentFile;
            }
        }
    }

    ReferencedTopDUContext context;
    {
        DUChainReadLocker lock;
        context = DUChainUtils::standardContextForUrl(document().toUrl());
    }
    if (context) {
        translateDUChainToRevision(context);
        context->setRange(RangeInRevision(0, 0, INT_MAX, INT_MAX));
        context->addImportedParentContext(parentCtx);
    }

    CMakeFileContent package = CMakeListsParser::readCMakeFile(document().toUrl().toLocalFile());
    if (!package.isEmpty()) {
        if (abortRequested()) {
            abortJob();
            return;
        }

        QReadLocker parseLock(languageSupport()->language()->parseLock());
        CMakeContentIterator it(package);
        DeclarationBuilder builder;
        context = builder.build(document(), &it, context);

        if (abortRequested()) {
            abortJob();
            return;
        }

//         session.reparseImporters(context);
//
//         if ( context && minimumFeatures() & TopDUContext::AllDeclarationsContextsAndUses ) {
//             UseBuilder useBuilder(&session);
//             useBuilder.buildUses(session.ast());
//         }
    }

    if (abortRequested()) {
        abortJob();
        return;
    }

    if (!context) {
        DUChainWriteLocker lock;
        ParsingEnvironmentFile *file = new ParsingEnvironmentFile(document());
        file->setLanguage(languageName);
        context = new TopDUContext(document(), RangeInRevision(0, 0, INT_MAX, INT_MAX), file);
        DUChain::self()->addDocumentChain(context);
    }

    setDuChain(context);
    {
        DUChainWriteLocker lock;
//         context->setProblems(session.problems());

        context->setFeatures(minimumFeatures());
        ParsingEnvironmentFilePointer file = context->parsingEnvironmentFile();
        Q_ASSERT(file);
        file->setModificationRevision(contents().modification);
        DUChain::self()->updateContextEnvironment( context->topContext(), file.data() );
    }
    highlightDUChain();

}
