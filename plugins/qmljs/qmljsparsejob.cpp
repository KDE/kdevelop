/*
    SPDX-FileCopyrightText: 2012 Aleix Pol <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qmljsparsejob.h"

#include <language/backgroundparser/urlparselock.h>
#include <custom-definesandincludes/idefinesandincludesmanager.h>

#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/duchain.h>
#include <language/duchain/parsingenvironment.h>
#include <language/interfaces/ilanguagesupport.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <project/projectmodel.h>

#include "duchain/cache.h"
#include "duchain/declarationbuilder.h"
#include "duchain/parsesession.h"
#include "duchain/usebuilder.h"

#include "debug.h"

#include <QReadLocker>

using namespace KDevelop;

/*
 * This function has been copied from kdev-clang
 *
 * SPDX-FileCopyrightText: 2013 Olivier de Gaalon <olivier.jg@gmail.com> and Milian Wolff <mail@milianw.de>
 * Licensed under the GPL v2+
 */
ProjectFileItem* findProjectFileItem(const IndexedString& url)
{
    ProjectFileItem* file = nullptr;

    const auto& projects = ICore::self()->projectController()->projects();
    for (auto project: projects) {
        const auto files = project->filesForPath(url);
        if (files.isEmpty()) {
            continue;
        }

        file = files.last();

        // A file might be defined in different targets.
        // Prefer file items defined inside a target with non-empty includes.
        for (auto f: files) {
            if (!dynamic_cast<ProjectTargetItem*>(f->parent())) {
                continue;
            }
            file = f;
            if (!IDefinesAndIncludesManager::manager()->includes(f, IDefinesAndIncludesManager::ProjectSpecific).isEmpty()) {
                break;
            }
        }
    }
    return file;
}

QmlJsParseJob::QmlJsParseJob(const IndexedString& url, ILanguageSupport* languageSupport)
: ParseJob(url, languageSupport)
{
    // Tell the cache that this file has custom include directories
    if (auto file = findProjectFileItem(url)) {
        QmlJS::Cache::instance().setFileCustomIncludes(
            url,
            IDefinesAndIncludesManager::manager()->includes(file,
                IDefinesAndIncludesManager::Type(
                    IDefinesAndIncludesManager::ProjectSpecific | IDefinesAndIncludesManager::UserDefined))
        );
    } else {
        QmlJS::Cache::instance().setFileCustomIncludes(
            url,
            IDefinesAndIncludesManager::manager()->includes(url.str(),
                IDefinesAndIncludesManager::ProjectSpecific)
        );
    }
}

void QmlJsParseJob::run(ThreadWeaver::JobPointer pointer, ThreadWeaver::Thread* thread)
{
    Q_UNUSED(pointer)
    Q_UNUSED(thread)

    UrlParseLock urlLock(document());
    if (abortRequested() || !isUpdateRequired(ParseSession::languageString())) {
        return;
    }

    // Don't parse this file if one of its dependencies is not up to date
    const auto& dependencies = QmlJS::Cache::instance().dependencies(document());
    for (auto& dependency : dependencies) {
        if (!QmlJS::Cache::instance().isUpToDate(dependency)) {
            QmlJS::Cache::instance().setUpToDate(document(), false);
            return;
        }
    }

    qCDebug(KDEV_QMLJS) << "parsing" << document().str();

    ProblemPointer p = readContents();
    if (p) {
        //TODO: associate problem with topducontext
        return;
    }

    ParseSession session(document(), QString::fromUtf8(contents().contents), priority());

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
        QReadLocker parseLock(languageSupport()->parseLock());

        if (abortRequested()) {
            abortJob();
            return;
        }

        DeclarationBuilder builder(&session);
        context = builder.build(document(), session.ast(), context);

        if (abortRequested()) {
            abortJob();
            return;
        }

        if ( context && (minimumFeatures() & TopDUContext::AllDeclarationsContextsAndUses) ) {
            UseBuilder useBuilder(&session);
            useBuilder.buildUses(session.ast());
        }
    }

    if (abortRequested()) {
        abortJob();
        return;
    }

    if (!context) {
        DUChainWriteLocker lock;
        auto* file = new ParsingEnvironmentFile(document());
        file->setLanguage(ParseSession::languageString());
        context = new TopDUContext(document(), RangeInRevision(0, 0, INT_MAX, INT_MAX), file);
        DUChain::self()->addDocumentChain(context);
    }

    setDuChain(context);

    // If the file has become up to date, reparse its importers
    bool dependenciesOk = session.allDependenciesSatisfied();

    QmlJS::Cache::instance().setUpToDate(document(), dependenciesOk);

    if (dependenciesOk) {
        session.reparseImporters();
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

    DUChain::self()->emitUpdateReady(document(), duChain());

    if (session.isParsedCorrectly()) {
        qCDebug(KDEV_QMLJS) << "===Success===" << document().str();
    } else {
        qCDebug(KDEV_QMLJS) << "===Failed===" << document().str() << session.problems();
    }
}

