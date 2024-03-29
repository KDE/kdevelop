/*
    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "codecompletionmodel.h"

#include <QThread>
#include <KTextEditor/View>
#include <KTextEditor/Document>

#include "../duchain/declaration.h"
#include "../duchain/classfunctiondeclaration.h"
#include "../duchain/ducontext.h"
#include "../duchain/duchain.h"
#include "../duchain/namespacealiasdeclaration.h"
#include "../duchain/parsingenvironment.h"
#include "../duchain/duchainlock.h"
#include "../duchain/duchainbase.h"
#include "../duchain/topducontext.h"
#include "../duchain/duchainutils.h"
#include "../interfaces/quickopendataprovider.h"
#include "../interfaces/icore.h"
#include "../interfaces/ilanguagecontroller.h"
#include "../interfaces/icompletionsettings.h"
#include <debug.h>

#include "codecompletionworker.h"
#include "codecompletioncontext.h"
#include <duchain/specializationstore.h>

using namespace KTextEditor;

//Multi-threaded completion creates some multi-threading related crashes, and sometimes shows the completions in the wrong position if the cursor was moved
// #define SINGLE_THREADED_COMPLETION

namespace KDevelop {
class CompletionWorkerThread
    : public QThread
{
    Q_OBJECT

public:

    explicit CompletionWorkerThread(CodeCompletionModel* model)
        : QThread(model)
        , m_model(model)
        , m_worker(m_model->createCompletionWorker())
    {
        Q_ASSERT(m_worker->parent() == nullptr); // Must be null, else we cannot change the thread affinity!
        m_worker->moveToThread(this);
        Q_ASSERT(m_worker->thread() == this);
    }

    ~CompletionWorkerThread() override
    {
        delete m_worker;
    }

    void run() override
    {
        //We connect directly, so we can do the pre-grouping within the background thread
        connect(m_worker, &CodeCompletionWorker::foundDeclarationsReal, m_model,
                &CodeCompletionModel::foundDeclarations, Qt::QueuedConnection);

        connect(m_model, &CodeCompletionModel::completionsNeeded, m_worker,
                QOverload<const DUChainPointer<KDevelop::DUContext>&, const Cursor&, View*>::of(&CodeCompletionWorker::computeCompletions),
                Qt::QueuedConnection);
        connect(m_model, &CodeCompletionModel::doSpecialProcessingInBackground, m_worker,
                &CodeCompletionWorker::doSpecialProcessing);
        exec();
    }

    CodeCompletionModel* m_model;
    CodeCompletionWorker* m_worker;
};

bool CodeCompletionModel::forceWaitForModel()
{
    return m_forceWaitForModel;
}

void CodeCompletionModel::setForceWaitForModel(bool wait)
{
    m_forceWaitForModel = wait;
}

CodeCompletionModel::CodeCompletionModel(QObject* parent)
    : KTextEditor::CodeCompletionModel(parent)
    , m_forceWaitForModel(false)
    , m_fullCompletion(true)
    , m_mutex(new QMutex)
    , m_thread(nullptr)
{
    qRegisterMetaType<KTextEditor::Cursor>();
}

void CodeCompletionModel::initialize()
{
    if (!m_thread) {
        m_thread = new CompletionWorkerThread(this);
#ifdef SINGLE_THREADED_COMPLETION
        m_thread->m_worker = createCompletionWorker();
#endif
        m_thread->start();
    }
}

CodeCompletionModel::~CodeCompletionModel()
{
    if (m_thread->m_worker)
        m_thread->m_worker->abortCurrentCompletion();
    m_thread->quit();
    m_thread->wait();

    delete m_thread;
    delete m_mutex;
}

bool CodeCompletionModel::fullCompletion() const
{
    return m_fullCompletion;
}

KDevelop::CodeCompletionWorker* CodeCompletionModel::worker() const
{
    return m_thread->m_worker;
}

void CodeCompletionModel::clear()
{
    beginResetModel();
    m_completionItems.clear();
    m_completionContext.reset();
    endResetModel();
}

void CodeCompletionModel::completionInvokedInternal(KTextEditor::View* view, const KTextEditor::Range& range,
                                                    InvocationType invocationType, const QUrl& url)
{
    Q_ASSERT(m_thread == worker()->thread());
    Q_UNUSED(invocationType)

    DUChainReadLocker lock(DUChain::lock(), 400);
    if (!lock.locked()) {
        qCDebug(LANGUAGE) << "could not lock du-chain in time";
        return;
    }

    TopDUContext* top = DUChainUtils::standardContextForUrl(url);
    if (!top) {
        qCDebug(LANGUAGE) << "================== NO CONTEXT FOUND =======================";
        beginResetModel();
        m_completionItems.clear();
        endResetModel();

        qCDebug(LANGUAGE) << "Completion invoked for unknown context. Document:" << url << ", Known documents:" <<
            DUChain::self()->documents();

        return;
    }

    setCurrentTopContext(TopDUContextPointer(top));

    RangeInRevision rangeInRevision = top->transformToLocalRevision(KTextEditor::Range(range));

    qCDebug(LANGUAGE) << "completion invoked for context" << ( DUContext* )top;

    if (top->parsingEnvironmentFile() &&
        top->parsingEnvironmentFile()->modificationRevision() !=
        ModificationRevision::revisionForFile(IndexedString(url.toString()))) {
        qCDebug(LANGUAGE) << "Found context is not current.";
    }

    DUContextPointer thisContext;
    {
        qCDebug(LANGUAGE) << "apply specialization:" << range.start();
        thisContext = SpecializationStore::self().applySpecialization(top->findContextAt(rangeInRevision.start), top);

        if (thisContext) {
            qCDebug(LANGUAGE) << "after specialization:" << thisContext->localScopeIdentifier().toString() <<
                thisContext->rangeInCurrentRevision();
        } else {
            thisContext = top;
        }

        qCDebug(LANGUAGE) << "context is set to" << thisContext.data();
    }

    lock.unlock();

    if (m_forceWaitForModel)
        emit waitForReset();

    emit completionsNeeded(thisContext, range.start(), view);
}

void CodeCompletionModel::completionInvoked(KTextEditor::View* view, const KTextEditor::Range& range,
                                            InvocationType invocationType)
{
    //If this triggers, initialize() has not been called after creation.
    Q_ASSERT(m_thread);

    KDevelop::ICompletionSettings::CompletionLevel level =
        KDevelop::ICore::self()->languageController()->completionSettings()->completionLevel();
    if (level == KDevelop::ICompletionSettings::AlwaysFull ||
        (invocationType != AutomaticInvocation && level == KDevelop::ICompletionSettings::MinimalWhenAutomatic))
        m_fullCompletion = true;
    else
        m_fullCompletion = false;

    //Only use grouping in full completion mode
    setHasGroups(m_fullCompletion);

    Q_UNUSED(invocationType)

    if (!worker()) {
        qCWarning(LANGUAGE) << "Completion invoked on a completion model which has no code completion worker assigned!";
    }

    beginResetModel();
    m_completionItems.clear();
    endResetModel();

    worker()->abortCurrentCompletion();
    worker()->setFullCompletion(m_fullCompletion);

    QUrl url = view->document()->url();

    completionInvokedInternal(view, range, invocationType, url);
}

void CodeCompletionModel::foundDeclarations(const QList<QExplicitlySharedDataPointer<CompletionTreeElement>>& items,
                                            const QExplicitlySharedDataPointer<CodeCompletionContext>& completionContext)
{
    m_completionContext = completionContext;

    if (m_completionItems.isEmpty() && items.isEmpty()) {
        if (m_forceWaitForModel) {
            // TODO KF5: Check if this actually works
            beginResetModel();
            endResetModel(); //If we need to reset the model, reset it
        }
        return; //We don't need to reset, which is bad for target model
    }

    beginResetModel();
    m_completionItems = items;
    endResetModel();

    if (m_completionContext) {
        qCDebug(LANGUAGE) << "got completion-context with " << m_completionContext->ungroupedElements().size() <<
            "ungrouped elements";
    }
}

KTextEditor::CodeCompletionModelControllerInterface::MatchReaction CodeCompletionModel::matchingItem(const QModelIndex& /*matched*/)
{
    return None;
}

void CodeCompletionModel::setCompletionContext(
    const QExplicitlySharedDataPointer<CodeCompletionContext>& completionContext)
{
    QMutexLocker lock(m_mutex);
    m_completionContext = completionContext;

    if (m_completionContext) {
        qCDebug(LANGUAGE) << "got completion-context with " << m_completionContext->ungroupedElements().size() <<
            "ungrouped elements";
    }
}

QExplicitlySharedDataPointer<CodeCompletionContext> CodeCompletionModel::completionContext() const
{
    QMutexLocker lock(m_mutex);
    return m_completionContext;
}

void CodeCompletionModel::executeCompletionItem(View* view, const KTextEditor::Range& word,
                                                const QModelIndex& index) const
{
    //We must not lock the duchain at this place, because the items might rely on that
    auto* element = static_cast<CompletionTreeElement*>(index.internalPointer());
    if (!element || !element->asItem())
        return;

    element->asItem()->execute(view, word);
}

QExplicitlySharedDataPointer<KDevelop::CompletionTreeElement> CodeCompletionModel::itemForIndex(const QModelIndex& index)
const
{
    auto* element = static_cast<CompletionTreeElement*>(index.internalPointer());
    return QExplicitlySharedDataPointer<KDevelop::CompletionTreeElement>(element);
}

QVariant CodeCompletionModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::TextAlignmentRole && index.column() == 0) {
        return Qt::AlignRight;
    }
    auto element = static_cast<const CompletionTreeElement*>(index.internalPointer());
    if (!element)
        return QVariant();

    if (role == CodeCompletionModel::GroupRole) {
        if (element->asNode()) {
            return QVariant(element->asNode()->role);
        } else {
            qCDebug(LANGUAGE) << "Requested group-role from leaf tree element";
            return QVariant();
        }
    } else {
        if (element->asNode()) {
            if (role == CodeCompletionModel::InheritanceDepth) {
                auto customGroupNode = dynamic_cast<const CompletionCustomGroupNode*>(element);
                if (customGroupNode)
                    return QVariant(customGroupNode->inheritanceDepth);
            }
            if (role == element->asNode()->role) {
                return element->asNode()->roleValue;
            } else {
                return QVariant();
            }
        }
    }

    if (!element->asItem()) {
        qCWarning(LANGUAGE) << "Error in completion model";
        return QVariant();
    }

    //Navigation widget interaction is done here, the other stuff is done within the tree-elements
    switch (role) {
    case CodeCompletionModel::InheritanceDepth:
        return element->asItem()->inheritanceDepth();
    case CodeCompletionModel::ArgumentHintDepth:
        return element->asItem()->argumentHintDepth();

    case CodeCompletionModel::ItemSelected: {
        DeclarationPointer decl = element->asItem()->declaration();
        if (decl) {
            DUChain::self()->emitDeclarationSelected(decl);
        }
        break;
    }
    }

    //In minimal completion mode, hide all columns except the "name" one
    if (!m_fullCompletion && role == Qt::DisplayRole && index.column() != Name &&
        (element->asItem()->argumentHintDepth() == 0 || index.column() == Prefix))
        return QVariant();

    //In reduced completion mode, don't show information text with the selected items
    if (role == ItemSelected &&
        (!m_fullCompletion ||
         !ICore::self()->languageController()->completionSettings()->showMultiLineSelectionInformation()))
        return QVariant();

    return element->asItem()->data(index, role, this);
}

KDevelop::TopDUContextPointer CodeCompletionModel::currentTopContext() const
{
    return m_currentTopContext;
}

void CodeCompletionModel::setCurrentTopContext(const KDevelop::TopDUContextPointer& topContext)
{
    m_currentTopContext = topContext;
}

QModelIndex CodeCompletionModel::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid()) {
        auto* element = static_cast<CompletionTreeElement*>(parent.internalPointer());

        CompletionTreeNode* node = element->asNode();

        if (!node) {
            qCDebug(LANGUAGE) << "Requested sub-index of leaf node";
            return QModelIndex();
        }

        if (row < 0 || row >= node->children.count() || column < 0 || column >= ColumnCount)
            return QModelIndex();

        return createIndex(row, column, node->children[row].data());
    } else {
        if (row < 0 || row >= m_completionItems.count() || column < 0 || column >= ColumnCount)
            return QModelIndex();

        return createIndex(row, column, const_cast<CompletionTreeElement*>(m_completionItems[row].data()));
    }
}

QModelIndex CodeCompletionModel::parent(const QModelIndex& index) const
{
    if (rowCount() == 0)
        return QModelIndex();

    if (index.isValid()) {
        auto* element = static_cast<CompletionTreeElement*>(index.internalPointer());

        if (element->parent())
            return createIndex(element->rowInParent(), element->columnInParent(), element->parent());
    }

    return QModelIndex();
}

int CodeCompletionModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        auto* element = static_cast<CompletionTreeElement*>(parent.internalPointer());

        CompletionTreeNode* node = element->asNode();

        if (!node)
            return 0;

        return node->children.count();
    } else {
        return m_completionItems.count();
    }
}

QString CodeCompletionModel::filterString(KTextEditor::View* view, const KTextEditor::Range& range,
                                          const KTextEditor::Cursor& position)
{
    m_filterString = KTextEditor::CodeCompletionModelControllerInterface::filterString(view, range, position);
    return m_filterString;
}
}

#include "moc_codecompletionmodel.cpp"
#include "codecompletionmodel.moc"
