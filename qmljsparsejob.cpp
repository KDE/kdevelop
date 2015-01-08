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
#include <custom-definesandincludes/idefinesandincludesmanager.h>

#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/duchain.h>
#include <language/duchain/parsingenvironment.h>
#include <interfaces/ilanguage.h>
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
 * Copyright 2013 Olivier de Gaalon <olivier.jg@gmail.com> and Milian Wolff <mail@milianw.de>
 * Licensed under the GPL v2+
 */
ProjectFileItem* findProjectFileItem(const IndexedString& url)
{
    ProjectFileItem* file = nullptr;

    for (auto project: ICore::self()->projectController()->projects()) {
        auto files = project->filesForPath(url);
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
            IDefinesAndIncludesManager::manager()->includes(file)
        );
    } else {
        QmlJS::Cache::instance().setFileCustomIncludes(
            url,
            IDefinesAndIncludesManager::manager()->includes(url.str())
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
    for (auto dependency : QmlJS::Cache::instance().dependencies(document())) {
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
        QReadLocker parseLock(languageSupport()->parseLock());

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

    if (session.isParsedCorrectly()) {
        qCDebug(KDEV_QMLJS) << "===Success===" << document().str();
    } else {
        qCDebug(KDEV_QMLJS) << "===Failed===" << document().str();
    }
}

