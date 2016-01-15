/*
 * KDevelop Problem Reporter
 *
 * Copyright (c) 2006-2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2006 Adam Treat <treat@kde.org>
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

#include "problemtreeview.h"

#include <QAction>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QIcon>
#include <QItemDelegate>
#include <QMenu>
#include <QSignalMapper>
#include <QSortFilterProxyModel>

#include <KActionMenu>
#include <KLocalizedString>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iassistant.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <util/kdevstringhandler.h>

#include "problemreporterplugin.h"
#include <shell/problemmodel.h>
#include <shell/problem.h>
#include <shell/problemconstants.h>

//#include "modeltest.h"

using namespace KDevelop;

namespace KDevelop
{

class ProblemTreeViewItemDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    explicit ProblemTreeViewItemDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};
}

ProblemTreeViewItemDelegate::ProblemTreeViewItemDelegate(QObject* parent)
    : QItemDelegate(parent)
{
}

void ProblemTreeViewItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                        const QModelIndex& index) const
{
    QStyleOptionViewItem newOption(option);
    newOption.textElideMode = index.column() == ProblemModel::File ? Qt::ElideMiddle : Qt::ElideRight;

    QItemDelegate::paint(painter, newOption, index);
}

ProblemTreeView::ProblemTreeView(QWidget* parent, QAbstractItemModel* itemModel)
    : QTreeView(parent)
    , m_proxy(new QSortFilterProxyModel(this))
{

    setObjectName("Problem Reporter Tree");
    setWhatsThis(i18n("Problems"));
    setItemDelegate(new ProblemTreeViewItemDelegate);
    setSelectionBehavior(QAbstractItemView::SelectRows);

    m_proxy->setSortRole(ProblemModel::SeverityRole);
    m_proxy->setDynamicSortFilter(true);
    m_proxy->sort(0, Qt::AscendingOrder);

    ProblemModel* problemModel = dynamic_cast<ProblemModel*>(itemModel);
    Q_ASSERT(problemModel);
    setModel(problemModel);

    header()->setStretchLastSection(false);

    if (problemModel->features().testFlag(ProblemModel::CanDoFullUpdate)) {
        QAction* fullUpdateAction = new QAction(this);
        fullUpdateAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        fullUpdateAction->setText(i18n("Force Full Update"));
        fullUpdateAction->setToolTip(i18nc("@info:tooltip", "Re-parse all watched documents"));
        fullUpdateAction->setIcon(QIcon::fromTheme("view-refresh"));
        connect(fullUpdateAction, &QAction::triggered, model(), &ProblemModel::forceFullUpdate);
        addAction(fullUpdateAction);
    }

    if (problemModel->features().testFlag(ProblemModel::CanShowImports)) {
        QAction* showImportsAction = new QAction(this);
        addAction(showImportsAction);
        showImportsAction->setCheckable(true);
        showImportsAction->setChecked(false);
        showImportsAction->setText(i18n("Show Imports"));
        showImportsAction->setToolTip(i18nc("@info:tooltip", "Display problems in imported files"));
        this->model()->setShowImports(false);
        connect(showImportsAction, &QAction::triggered, model(), &ProblemModel::setShowImports);
    }

    if (problemModel->features().testFlag(ProblemModel::ScopeFilter)) {
        KActionMenu* scopeMenu = new KActionMenu(this);
        scopeMenu->setDelayed(false);
        scopeMenu->setToolTip(i18nc("@info:tooltip", "Which files to display the problems for"));
        scopeMenu->setObjectName(QStringLiteral("scopeMenu"));

        QActionGroup* scopeActions = new QActionGroup(this);

        QAction* currentDocumentAction = new QAction(this);
        currentDocumentAction->setText(i18n("Current Document"));
        currentDocumentAction->setToolTip(i18nc("@info:tooltip", "Display problems in current document"));

        QAction* openDocumentsAction = new QAction(this);
        openDocumentsAction->setText(i18n("Open Documents"));
        openDocumentsAction->setToolTip(i18nc("@info:tooltip", "Display problems in all open documents"));

        QAction* currentProjectAction = new QAction(this);
        currentProjectAction->setText(i18n("Current Project"));
        currentProjectAction->setToolTip(i18nc("@info:tooltip", "Display problems in current project"));

        QAction* allProjectAction = new QAction(this);
        allProjectAction->setText(i18n("All Projects"));
        allProjectAction->setToolTip(i18nc("@info:tooltip", "Display problems in all projects"));

        QVector<QAction*> actions;
        actions.push_back(currentDocumentAction);
        actions.push_back(openDocumentsAction);
        actions.push_back(currentProjectAction);
        actions.push_back(allProjectAction);

        if (problemModel->features().testFlag(ProblemModel::CanByPassScopeFilter)) {
            QAction* showAllAction = new QAction(this);
            showAllAction->setText(i18n("Show All"));
            showAllAction->setToolTip(i18nc("@info:tooltip", "Display ALL problems"));
            actions.push_back(showAllAction);
        }

        foreach (QAction* action, actions) {
            action->setCheckable(true);
            scopeActions->addAction(action);
            scopeMenu->addAction(action);
        }
        addAction(scopeMenu);
        setScope(CurrentDocument);

        // Show All should be default if it's supported. It helps with error messages that are otherwise invisible
        if (problemModel->features().testFlag(ProblemModel::CanByPassScopeFilter)) {
            actions.last()->setChecked(true);
            model()->setScope(BypassScopeFilter);
        } else {
            currentDocumentAction->setChecked(true);
            model()->setScope(CurrentDocument);
        }

        QSignalMapper* scopeMapper = new QSignalMapper(this);
        scopeMapper->setMapping(currentDocumentAction, CurrentDocument);
        scopeMapper->setMapping(openDocumentsAction, OpenDocuments);
        scopeMapper->setMapping(currentProjectAction, CurrentProject);
        scopeMapper->setMapping(allProjectAction, AllProjects);
        connect(currentDocumentAction, &QAction::triggered, scopeMapper,
                static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
        connect(openDocumentsAction, &QAction::triggered, scopeMapper,
                static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
        connect(currentProjectAction, &QAction::triggered, scopeMapper,
                static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
        connect(allProjectAction, &QAction::triggered, scopeMapper,
                static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));

        if (problemModel->features().testFlag(ProblemModel::CanByPassScopeFilter)) {
            scopeMapper->setMapping(actions.last(), BypassScopeFilter);
            connect(actions.last(), &QAction::triggered, scopeMapper,
                    static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
        }

        connect(scopeMapper, static_cast<void (QSignalMapper::*)(int)>(&QSignalMapper::mapped), this,
                &ProblemTreeView::setScope);
    }

    if (problemModel->features().testFlag(ProblemModel::SeverityFilter)) {
        QActionGroup* severityActions = new QActionGroup(this);
        auto errorSeverityAction = new QAction(this);
        errorSeverityAction->setToolTip(i18nc("@info:tooltip", "Display only errors"));
        errorSeverityAction->setIcon(QIcon::fromTheme("dialog-error"));

        auto warningSeverityAction = new QAction(this);
        warningSeverityAction->setToolTip(i18nc("@info:tooltip", "Display errors and warnings"));
        warningSeverityAction->setIcon(QIcon::fromTheme("dialog-warning"));

        auto hintSeverityAction = new QAction(this);
        hintSeverityAction->setToolTip(i18nc("@info:tooltip", "Display errors, warnings and hints"));
        hintSeverityAction->setIcon(QIcon::fromTheme("dialog-information"));

        QAction* severityActionArray[] = { errorSeverityAction, warningSeverityAction, hintSeverityAction };
        for (int i = 0; i < 3; ++i) {
            severityActionArray[i]->setCheckable(true);
            severityActions->addAction(severityActionArray[i]);
            addAction(severityActionArray[i]);
        }

        hintSeverityAction->setChecked(true);
        model()->setSeverity(IProblem::Hint);
        QSignalMapper* severityMapper = new QSignalMapper(this);
        severityMapper->setMapping(errorSeverityAction, IProblem::Error);
        severityMapper->setMapping(warningSeverityAction, IProblem::Warning);
        severityMapper->setMapping(hintSeverityAction, IProblem::Hint);
        connect(errorSeverityAction, &QAction::triggered, severityMapper,
                static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
        connect(warningSeverityAction, &QAction::triggered, severityMapper,
                static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
        connect(hintSeverityAction, &QAction::triggered, severityMapper,
                static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
        connect(severityMapper, static_cast<void (QSignalMapper::*)(int)>(&QSignalMapper::mapped), model(),
                &ProblemModel::setSeverity);
    }

    if (problemModel->features().testFlag(ProblemModel::Grouping)) {
        KActionMenu* groupingMenu = new KActionMenu(i18n("Grouping"), this);
        groupingMenu->setDelayed(false);

        QActionGroup* groupingActions = new QActionGroup(this);

        QAction* noGroupingAction = new QAction(i18n("None"), this);
        QAction* pathGroupingAction = new QAction(i18n("Path"), this);
        QAction* severityGroupingAction = new QAction(i18n("Severity"), this);

        QAction* groupingActionArray[] = { noGroupingAction, pathGroupingAction, severityGroupingAction };
        for (unsigned i = 0; i < sizeof(groupingActionArray) / sizeof(QAction*); ++i) {
            QAction* action = groupingActionArray[i];
            action->setCheckable(true);
            groupingActions->addAction(action);
            groupingMenu->addAction(action);
        }
        addAction(groupingMenu);

        noGroupingAction->setChecked(true);
        QSignalMapper* groupingMapper = new QSignalMapper(this);
        groupingMapper->setMapping(noGroupingAction, NoGrouping);
        groupingMapper->setMapping(pathGroupingAction, PathGrouping);
        groupingMapper->setMapping(severityGroupingAction, SeverityGrouping);

        connect(noGroupingAction, &QAction::triggered, groupingMapper,
                static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
        connect(pathGroupingAction, &QAction::triggered, groupingMapper,
                static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
        connect(severityGroupingAction, &QAction::triggered, groupingMapper,
                static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));

        connect(groupingMapper, static_cast<void (QSignalMapper::*)(int)>(&QSignalMapper::mapped), model(),
                &ProblemModel::setGrouping);
    }

    connect(this, &ProblemTreeView::clicked, this, &ProblemTreeView::itemActivated);

    connect(model(), &QAbstractItemModel::rowsInserted, this, &ProblemTreeView::changed);
    connect(model(), &QAbstractItemModel::rowsRemoved, this, &ProblemTreeView::changed);
    connect(model(), &QAbstractItemModel::modelReset, this, &ProblemTreeView::changed);
}

ProblemTreeView::~ProblemTreeView()
{
}

void ProblemTreeView::openDocumentForCurrentProblem()
{
    itemActivated(currentIndex());
}

void ProblemTreeView::itemActivated(const QModelIndex& index)
{
    if (!index.isValid())
        return;

    KTextEditor::Cursor start;
    QUrl url;

    {
        // TODO: is this really necessary?
        DUChainReadLocker lock(DUChain::lock());
        const auto problem = index.data(ProblemModel::ProblemRole).value<IProblem::Ptr>();
        if (!problem)
            return;

        url = problem->finalLocation().document.toUrl();
        start = problem->finalLocation().start();
    }

    ICore::self()->documentController()->openDocument(url, start);
}

void ProblemTreeView::setScope(int scope)
{
    foreach (auto action, actions()) {
        if (action->objectName() == QLatin1String("scopeMenu")) {
            action->setText(i18n("Scope: %1", action->menu()->actions().at(scope)->text()));
        }
    }

    model()->setScope(scope);
}

void ProblemTreeView::resizeColumns()
{
    for (int i = 0; i < model()->columnCount(); ++i)
        resizeColumnToContents(i);
}

void ProblemTreeView::dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles)
{
    QTreeView::dataChanged(topLeft, bottomRight, roles);
    resizeColumns();
}

void ProblemTreeView::reset()
{
    QTreeView::reset();
    resizeColumns();
}

ProblemModel* ProblemTreeView::model() const
{
    return static_cast<ProblemModel*>(m_proxy->sourceModel());
}

void ProblemTreeView::setModel(QAbstractItemModel* model)
{
    Q_ASSERT(qobject_cast<ProblemModel*>(model));
    m_proxy->setSourceModel(model);
    QTreeView::setModel(m_proxy);
}

void ProblemTreeView::contextMenuEvent(QContextMenuEvent* event)
{
    QModelIndex index = indexAt(event->pos());
    if (index.isValid()) {
        const auto problem = index.data(ProblemModel::ProblemRole).value<IProblem::Ptr>();
        if (!problem) {
            return;
        }
        QExplicitlySharedDataPointer<KDevelop::IAssistant> solution = problem->solutionAssistant();
        if (!solution) {
            return;
        }
        QList<QAction*> actions;
        foreach (KDevelop::IAssistantAction::Ptr action, solution->actions()) {
            actions << action->toKAction();
        }
        if (!actions.isEmpty()) {
            QString title = solution->title();
            title = KDevelop::htmlToPlainText(title);
            title.replace("&apos;", "\'");

            QPointer<QMenu> m = new QMenu(this);
            m->addSection(title);
            m->addActions(actions);
            m->exec(event->globalPos());
            delete m;
        }
    }
}

void ProblemTreeView::showEvent(QShowEvent* event)
{
    Q_UNUSED(event)
    resizeColumns();
}

#include "problemtreeview.moc"
