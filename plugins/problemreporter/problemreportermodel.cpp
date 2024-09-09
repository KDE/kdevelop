/*
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2015 Laszlo Kis-Adam <laszlo.kis-adam@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "problemreportermodel.h"

#include <language/backgroundparser/backgroundparser.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/parsingenvironment.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/problem.h>
#include <language/duchain/duchainutils.h>
#include <language/assistant/staticassistantsmanager.h>

#include <QThread>
#include <QTimer>

#include <serialization/indexedstring.h>

#include <shell/watcheddocumentset.h>
#include <shell/filteredproblemstore.h>

#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/idocument.h>

using namespace KDevelop;

ProblemReporterModel::ProblemReporterModel(QObject* parent)
    : ProblemModel(parent, new FilteredProblemStore())
{
    setFeatures(CanDoFullUpdate | CanShowImports | ScopeFilter | SeverityFilter | ShowSource);

    connect(store(), &FilteredProblemStore::changed, this, &ProblemReporterModel::onProblemsChanged);
    connect(ICore::self()->languageController()->staticAssistantsManager(), &StaticAssistantsManager::problemsChanged,
            this, &ProblemReporterModel::onProblemsChanged);
}

ProblemReporterModel::~ProblemReporterModel()
{
}

QVector<KDevelop::IProblem::Ptr> ProblemReporterModel::problems(const QSet<KDevelop::IndexedString>& docs) const
{
    QVector<IProblem::Ptr> result;
    DUChainReadLocker lock;

    for (const IndexedString& doc : docs) {
        if (doc.isEmpty())
            continue;

        TopDUContext* ctx = DUChain::self()->chainForDocument(doc);
        if (!ctx)
            continue;

        const auto allProblems = DUChainUtils::allProblemsForContext(ctx);
        result.reserve(result.size() + allProblems.size());
        for (const ProblemPointer& p : allProblems) {
            result.append(p);
        }
    }

    return result;
}

void ProblemReporterModel::forceFullUpdate()
{
    Q_ASSERT(thread() == QThread::currentThread());

    QSet<IndexedString> documents = store()->documents()->get();
    if (showImports())
        documents += store()->documents()->imports();

    DUChainReadLocker lock(DUChain::lock());
    for (const IndexedString& document : std::as_const(documents)) {
        if (document.isEmpty())
            continue;

        TopDUContext::Features updateType = TopDUContext::ForceUpdate;
        if (documents.size() == 1)
            updateType = TopDUContext::ForceUpdateRecursive;
        DUChain::self()->updateContextForUrl(
            document, updateType | TopDUContext::VisibleDeclarationsAndContexts);
    }
}

void ProblemReporterModel::onProblemsChanged()
{
    rebuildProblemList();
}

void ProblemReporterModel::setCurrentDocument(KDevelop::IDocument* doc)
{
    Q_ASSERT(thread() == QThread::currentThread());

    beginResetModel();

    /// Will trigger signal changed() if problems change
    store()->setCurrentDocument(IndexedString(doc->url()));
    endResetModel();
}

void ProblemReporterModel::problemsUpdated(const KDevelop::IndexedString& url)
{
    Q_ASSERT(thread() == QThread::currentThread());

    // skip update for urls outside current scope
    if (!store()->documents()->get().contains(url) &&
        !(showImports() && store()->documents()->imports().contains(url)))
        return;

    rebuildProblemList();
}

void ProblemReporterModel::rebuildProblemList()
{
    /// No locking here, because it may be called from an already locked context
    beginResetModel();

    QVector<IProblem::Ptr> allProblems = problems(store()->documents()->get());

    if (showImports())
        allProblems += problems(store()->documents()->imports());

    store()->setProblems(allProblems);

    endResetModel();
}

#include "moc_problemreportermodel.cpp"
