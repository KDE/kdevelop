/*
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2015 Laszlo Kis-Adam
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#include "problemreportermodel.h"

#include <language/backgroundparser/backgroundparser.h>
#include <language/backgroundparser/parsejob.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/parsingenvironment.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/problem.h>

#include <QThread>
#include <QTimer>

#include <serialization/indexedstring.h>

#include <shell/watcheddocumentset.h>
#include <shell/problemstore.h>

#include <interfaces/idocument.h>

using namespace KDevelop;

const int ProblemReporterModel::MinTimeout = 1000;
const int ProblemReporterModel::MaxTimeout = 5000;

ProblemReporterModel::ProblemReporterModel(QObject *parent)
    : ProblemModel(parent, new ProblemStore())
    , m_showImports(false)
{
    setFeatures(CanDoFullUpdate | CanShowImports | ScopeFilter | SeverityFilter);

    m_minTimer = new QTimer(this);
    m_minTimer->setInterval(MinTimeout);
    m_minTimer->setSingleShot(true);
    connect(m_minTimer, &QTimer::timeout, this, &ProblemReporterModel::timerExpired);
    m_maxTimer = new QTimer(this);
    m_maxTimer->setInterval(MaxTimeout);
    m_maxTimer->setSingleShot(true);
    connect(m_maxTimer, &QTimer::timeout, this, &ProblemReporterModel::timerExpired);
    connect(store(), &ProblemStore::changed, this, &ProblemReporterModel::onProblemsChanged);
}

ProblemReporterModel::~ProblemReporterModel()
{
}


QVector<IProblem::Ptr> ProblemReporterModel::problems(const KDevelop::IndexedString& url, bool showImports) const
{
    QVector<IProblem::Ptr> result;
    QSet<KDevelop::TopDUContext*> visitedContexts;
    KDevelop::DUChainReadLocker lock;
    problemsInternal(KDevelop::DUChain::self()->chainForDocument(url), showImports, visitedContexts, result);
    return result;
}

QVector<IProblem::Ptr> ProblemReporterModel::problems(const QSet< KDevelop::IndexedString > &urls, bool showImports) const
{
    QVector<IProblem::Ptr> result;
    QSet<KDevelop::TopDUContext*> visitedContexts;
    KDevelop::DUChainReadLocker lock;
    foreach (const KDevelop::IndexedString& url, urls) {
        problemsInternal(KDevelop::DUChain::self()->chainForDocument(url), showImports, visitedContexts, result);
    }
    return result;
}


void ProblemReporterModel::forceFullUpdate()
{
    Q_ASSERT(thread() == QThread::currentThread());

    QSet<KDevelop::IndexedString> documents = store()->documents()->get();
    KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
    foreach (const KDevelop::IndexedString& document, documents) {
        if (document.isEmpty())
            continue;

        KDevelop::TopDUContext::Features updateType = KDevelop::TopDUContext::ForceUpdate;
        if (documents.size() == 1)
            updateType = KDevelop::TopDUContext::ForceUpdateRecursive;
        KDevelop::DUChain::self()->updateContextForUrl(document, (KDevelop::TopDUContext::Features)(updateType | KDevelop::TopDUContext::VisibleDeclarationsAndContexts));
    }
}

void ProblemReporterModel::problemsInternal(KDevelop::TopDUContext* context, bool showImports, QSet<KDevelop::TopDUContext*>& visitedContexts, QVector<IProblem::Ptr>& result) const
{
    if (!context || visitedContexts.contains(context)) {
        return;
    }
    foreach (KDevelop::ProblemPointer p, context->problems()) {
        if (p && p->severity() <= store()->severity()) {
            result.append(p);
        }
    }
    visitedContexts.insert(context);
    if (showImports) {
        bool isProxy = context->parsingEnvironmentFile() && context->parsingEnvironmentFile()->isProxyContext();
        foreach (const KDevelop::DUContext::Import &ctx, context->importedParentContexts()) {
            if (!ctx.indexedContext().indexedTopContext().isLoaded())
                continue;
            KDevelop::TopDUContext* topCtx = dynamic_cast<KDevelop::TopDUContext*>(ctx.context(0));
            if (topCtx) {
                //If we are starting at a proxy-context, only recurse into other proxy-contexts,
                //because those contain the problems.
                if (!isProxy || (topCtx->parsingEnvironmentFile() && topCtx->parsingEnvironmentFile()->isProxyContext()))
                    problemsInternal(topCtx, showImports, visitedContexts, result);
            }
        }
    }
}

void ProblemReporterModel::onProblemsChanged()
{
    rebuildProblemList();
}

void ProblemReporterModel::timerExpired()
{
    m_minTimer->stop();
    m_maxTimer->stop();
    rebuildProblemList();
}

void ProblemReporterModel::setCurrentDocument(KDevelop::IDocument *doc)
{
    Q_ASSERT(thread() == QThread::currentThread());

    beginResetModel();
    QUrl currentDocument = doc->url();

    // Will trigger signal changed() if problems change
    store()->setCurrentDocument(KDevelop::IndexedString(currentDocument));
    endResetModel();
}

void ProblemReporterModel::problemsUpdated(const KDevelop::IndexedString& url)
{
    Q_ASSERT(thread() == QThread::currentThread());

    if (store()->documents()->get().contains(url)) {
        // m_minTimer will expire in MinTimeout unless some other parsing job finishes in this period.
        m_minTimer->start();
        // m_maxTimer will expire unconditionally in MaxTimeout
        if (!m_maxTimer->isActive()) {
            m_maxTimer->start();
        }
    }
}

void ProblemReporterModel::setShowImports(bool showImports)
{
    if (m_showImports != showImports) {
        Q_ASSERT(thread() == QThread::currentThread());
        m_showImports = showImports;
        rebuildProblemList();
    }
}

void ProblemReporterModel::rebuildProblemList()
{
    QVector<IProblem::Ptr> problems;
    // No locking here, because it may be called from an already locked context
    beginResetModel();

    problems = this->problems(store()->documents()->get(), m_showImports);
    store()->setProblems(problems);

    endResetModel();
}



