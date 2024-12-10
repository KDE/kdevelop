/*
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "problemmodel.h"
#include <QThread>
#include <QIcon>
#include <KLocalizedString>
#include <interfaces/icore.h>
#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/icompletionsettings.h>
#include <interfaces/iassistant.h>
#include <language/editor/documentrange.h>

#include <shell/problem.h>
#include <shell/problemstore.h>
#include <shell/problemstorenode.h>
#include <shell/filteredproblemstore.h>
#include <shell/problemconstants.h>
#include <shell/watcheddocumentset.h>
#include <util/scopedincrementor.h>

namespace KDevelop
{

class ProblemModelPrivate
{
public:
    explicit ProblemModelPrivate(KDevelop::ProblemStore *store)
        : m_problems(store)
        , m_features(KDevelop::ProblemModel::NoFeatures)
        , m_fullUpdateTooltip(i18nc("@info:tooltip", "Re-parse all watched documents"))
        , m_isPlaceholderShown(false)
    {
    }

    KDevelop::IProblem::Ptr createPlaceholdreProblem() const
    {
        Q_ASSERT(!m_placeholderText.isEmpty());

        KDevelop::IProblem::Ptr problem(new KDevelop::DetectedProblem(m_placeholderSource));
        problem->setDescription(m_placeholderText);
        problem->setFinalLocation(m_placeholderLocation);
        problem->setSeverity(KDevelop::IProblem::Hint);

        return problem;
    }

    QScopedPointer<KDevelop::ProblemStore> m_problems;
    KDevelop::ProblemModel::Features m_features;
    QString m_fullUpdateTooltip;
    QString m_placeholderText;
    QString m_placeholderSource;
    KDevelop::DocumentRange m_placeholderLocation;
    bool m_isPlaceholderShown;
    NonNegative<signed char> m_resettingModel{}; // signed char is sufficient and does not affect the class size
};


ProblemModel::ProblemModel(QObject * parent, ProblemStore *store)
  : QAbstractItemModel(parent)
  , d_ptr(new ProblemModelPrivate(store))
{
    Q_D(ProblemModel);

    if (!d->m_problems) {
        d->m_problems.reset(new FilteredProblemStore());
        d->m_features = ScopeFilter | SeverityFilter | Grouping | CanByPassScopeFilter;
    }

    setScope(CurrentDocument);

    connect(ICore::self()->documentController(), &IDocumentController::documentActivated, this, &ProblemModel::setCurrentDocument);
    connect(ICore::self()->documentController(), &IDocumentController::documentClosed, this, &ProblemModel::closedDocument);
    connect(ICore::self()->documentController(), &IDocumentController::documentUrlChanged, this,
            &ProblemModel::documentUrlChanged);
    /// CompletionSettings include a list of todo markers we care for, so need to update
    connect(ICore::self()->languageController()->completionSettings(), &ICompletionSettings::settingsChanged, this, &ProblemModel::forceFullUpdate);

    if (ICore::self()->documentController()->activeDocument()) {
        setCurrentDocument(ICore::self()->documentController()->activeDocument());
    }

    connect(d->m_problems.data(), &ProblemStore::beginRebuild, this, &ProblemModel::beginResetModel);
    connect(d->m_problems.data(), &ProblemStore::endRebuild, this, &ProblemModel::endResetModel);

    connect(d->m_problems.data(), &ProblemStore::problemsChanged, this, &ProblemModel::problemsChanged);
}

ProblemModel::~ ProblemModel()
{
}

int ProblemModel::rowCount(const QModelIndex& parent) const
{
    Q_D(const ProblemModel);

    if (!parent.isValid()) {
        return d->m_problems->count();
    } else {
        return d->m_problems->count(reinterpret_cast<ProblemStoreNode*>(parent.internalPointer()));
    }
}

static QString displayUrl(const QUrl &url, const QUrl &base)
{
    if (base.isParentOf(url)) {
        return url.toDisplayString(QUrl::PreferLocalFile).mid(base.toDisplayString(QUrl::PreferLocalFile).length());
    } else {
        return ICore::self()->projectController()->prettyFileName(url, IProjectController::FormatPlain);
    }
}

QVariant ProblemModel::data(const QModelIndex& index, int role) const
{
    Q_D(const ProblemModel);

    if (!index.isValid())
        return QVariant();

    QUrl baseDirectory = d->m_problems->currentDocument().toUrl().adjusted(QUrl::RemoveFilename);
    IProblem::Ptr p = problemForIndex(index);
    if (!p.constData()) {
        if (role == Qt::DisplayRole && index.column() == Error) {
            auto *node = reinterpret_cast<ProblemStoreNode*>(index.internalPointer());
            if (node) {
                return node->label();
            }
        }
        return {};
    }

    if (role == SeverityRole) {
        return p->severity();
    } else if (role == ProblemRole) {
        return QVariant::fromValue(p);
    }

    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case Source:
            return p->sourceString();
        case Error:
            return p->description();
        case File:
            return displayUrl(p->finalLocation().document.toUrl().adjusted(QUrl::NormalizePathSegments), baseDirectory);
        case Line:
            if (p->finalLocation().isValid()) {
                return QString::number(p->finalLocation().start().line() + 1);
            }
            break;
        case Column:
            if (p->finalLocation().isValid()) {
                return QString::number(p->finalLocation().start().column() + 1);
            }
            break;
        }
        break;

    case Qt::DecorationRole:
        if (index.column() == Error) {
            return IProblem::iconForSeverity(p->severity());
        }
        break;
    case Qt::ToolTipRole:
        return p->explanation();

    default:
        break;
    }

    return {};
}

QModelIndex ProblemModel::parent(const QModelIndex& index) const
{
    auto *node = reinterpret_cast<ProblemStoreNode*>(index.internalPointer());
    if (!node) {
        return {};
    }

    ProblemStoreNode *parent = node->parent();
    if (!parent || parent->isRoot()) {
        return {};
    }

    int idx = parent->index();
    return createIndex(idx, 0, parent);
}

QModelIndex ProblemModel::index(int row, int column, const QModelIndex& parent) const
{
    Q_D(const ProblemModel);

    if (row < 0 || row >= rowCount(parent) || column < 0 || column >= LastColumn) {
        return QModelIndex();
    }

    auto *parentNode = reinterpret_cast<ProblemStoreNode*>(parent.internalPointer());
    const ProblemStoreNode *node = d->m_problems->findNode(row, parentNode);
    return createIndex(row, column, (void*)node);
}

int ProblemModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return LastColumn;
}

IProblem::Ptr ProblemModel::problemForIndex(const QModelIndex& index) const
{
    auto *node = reinterpret_cast<ProblemStoreNode*>(index.internalPointer());
    if (!node) {
        return {};
    } else {
        return node->problem();
    }
}

ProblemModel::Features ProblemModel::features() const
{
    Q_D(const ProblemModel);

    return d->m_features;
}

void ProblemModel::setFeatures(Features features)
{
    Q_D(ProblemModel);

    d->m_features = features;
}

QString ProblemModel::fullUpdateTooltip() const
{
    Q_D(const ProblemModel);

    return d->m_fullUpdateTooltip;
}

void ProblemModel::setFullUpdateTooltip(const QString& tooltip)
{
    Q_D(ProblemModel);

    if (d->m_fullUpdateTooltip == tooltip) {
        return;
    }

    d->m_fullUpdateTooltip = tooltip;
    emit fullUpdateTooltipChanged();
}

void ProblemModel::setPlaceholderText(const QString& text, const KDevelop::DocumentRange& location, const QString& source)
{
    Q_D(ProblemModel);

    d->m_placeholderText = text;
    d->m_placeholderLocation = location;
    d->m_placeholderSource = source;

    if (d->m_isPlaceholderShown || d->m_problems->count() == 0) {
        // clearing will show/update the new placeholder
        clearProblems();
    }
}

void ProblemModel::addProblem(const IProblem::Ptr &problem)
{
    Q_D(ProblemModel);

    if (d->m_isPlaceholderShown) {
        setProblems({ problem });
    } else {
        int c = d->m_problems->count();
        beginInsertRows(QModelIndex(), c, c);
        d->m_problems->addProblem(problem);
        endInsertRows();
    }
}

void ProblemModel::setProblems(const QVector<IProblem::Ptr> &problems)
{
    Q_D(ProblemModel);

    beginResetModel();
    if (problems.isEmpty() && !d->m_placeholderText.isEmpty()) {
        d->m_problems->setProblems({ d->createPlaceholdreProblem() });
        d->m_isPlaceholderShown = true;
    } else {
        d->m_problems->setProblems(problems);
        d->m_isPlaceholderShown = false;
    }
    endResetModel();
}

void ProblemModel::clearProblems()
{
    setProblems({});
}

QVector<IProblem::Ptr> ProblemModel::problems(const KDevelop::IndexedString& document) const
{
    Q_D(const ProblemModel);

    return d->m_problems->problems(document);
}

QVariant ProblemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation);

    if (role != Qt::DisplayRole)
        return {};

    switch (section) {
        case Source:
            return i18nc("@title:column source of problem", "Source");
        case Error:
            return i18nc("@title:column problem description", "Problem");
        case File:
            return i18nc("@title:column file where problem was found", "File");
        case Line:
            return i18nc("@title:column line number with problem", "Line");
        case Column:
            return i18nc("@title:column column number with problem", "Column");
    }

    return {};
}

void ProblemModel::setCurrentDocument(IDocument* document)
{
    Q_D(ProblemModel);

    Q_ASSERT(thread() == QThread::currentThread());

    QUrl currentDocument = document->url();
    /// Will trigger signals beginRebuild(), endRebuild() if problems change and are rebuilt
    d->m_problems->setCurrentDocument(IndexedString(currentDocument));
}

void ProblemModel::closedDocument(IDocument* document)
{
    Q_D(ProblemModel);

    if (IndexedString(document->url()) == d->m_problems->currentDocument())
    {   // reset current document
        d->m_problems->setCurrentDocument(IndexedString());
    }
}

void ProblemModel::documentUrlChanged(IDocument* document, const QUrl& previousUrl)
{
    Q_D(ProblemModel);

    Q_ASSERT(thread() == QThread::currentThread());

    const auto currentDocument = d->m_problems->currentDocument();
    // If currentDocument.isEmpty(), the renamed document must have been closed already in
    // DocumentControllerPrivate::changeDocumentUrl() because of a conflict with another open modified document at its
    // new URL; another document at document->url() should be active now. So set the active document's URL as current.
    if (currentDocument.isEmpty() || currentDocument == IndexedString{previousUrl}) {
        setCurrentDocument(document);
    }
}

void ProblemModel::setShowImports(bool showImports)
{
    Q_D(ProblemModel);

    Q_ASSERT(thread() == QThread::currentThread());

    d->m_problems->setShowImports(showImports);
}

bool ProblemModel::showImports() const
{
    Q_D(const ProblemModel);

    return d->m_problems->showImports();
}

void ProblemModel::setScope(ProblemScope scope)
{
    Q_D(ProblemModel);

    Q_ASSERT(thread() == QThread::currentThread());

    if (!features().testFlag(ScopeFilter))
        scope = ProblemScope::BypassScopeFilter;

    /// Will trigger signals beginRebuild(), endRebuild() if problems change and are rebuilt
    d->m_problems->setScope(scope);
}

void ProblemModel::setPathForDocumentsInPathScope(const QString& path)
{
    Q_D(ProblemModel);

    d->m_problems->setPathForDocumentsInPathScope(path);
}

void ProblemModel::setSeverity(int severity)
{
    switch (severity)
    {
        case KDevelop::IProblem::Error:
            setSeverities(KDevelop::IProblem::Error);
            break;
        case KDevelop::IProblem::Warning:
            setSeverities(KDevelop::IProblem::Error | KDevelop::IProblem::Warning);
            break;
        case KDevelop::IProblem::Hint:
            setSeverities(KDevelop::IProblem::Error | KDevelop::IProblem::Warning | KDevelop::IProblem::Hint);
            break;
    }
}

void ProblemModel::setSeverities(KDevelop::IProblem::Severities severities)
{
    Q_D(ProblemModel);

    Q_ASSERT(thread() == QThread::currentThread());
    /// Will trigger signals beginRebuild(), endRebuild() if problems change and are rebuilt
    d->m_problems->setSeverities(severities);
}

void ProblemModel::setGrouping(int grouping)
{
    Q_D(ProblemModel);

    /// Will trigger signals beginRebuild(), endRebuild() if problems change and are rebuilt
    d->m_problems->setGrouping(grouping);
}

ProblemStore* ProblemModel::store() const
{
    Q_D(const ProblemModel);

    return d->m_problems.data();
}

void ProblemModel::beginResetModel()
{
    Q_D(ProblemModel);

    const bool alreadyResetting = d->m_resettingModel;
    ++d->m_resettingModel;
    if (!alreadyResetting) {
        QAbstractItemModel::beginResetModel();
    }
}

void ProblemModel::endResetModel()
{
    Q_D(ProblemModel);

    --d->m_resettingModel;
    const bool stillResetting = d->m_resettingModel;
    if (!stillResetting) {
        QAbstractItemModel::endResetModel();
    }
}
}

#include "moc_problemmodel.cpp"
