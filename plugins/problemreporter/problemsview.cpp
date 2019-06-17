/*
 * Copyright 2015 Laszlo Kis-Adam <laszlo.kis-adam@kdemail.net>
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

#include "problemsview.h"

#include <KActionMenu>
#include <KLocalizedString>

#include <QAction>
#include <QLineEdit>
#include <QMenu>
#include <QTabWidget>
#include <QTimer>
#include <QVBoxLayout>

#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <shell/problemconstants.h>
#include <shell/problemmodelset.h>
#include <util/expandablelineedit.h>
#include "problemtreeview.h"
#include "problemmodel.h"

namespace KDevelop
{

void ProblemsView::setupActions()
{
    {
        m_fullUpdateAction = new QAction(this);
        m_fullUpdateAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        m_fullUpdateAction->setText(i18n("Force Full Update"));
        m_fullUpdateAction->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));
        connect(m_fullUpdateAction, &QAction::triggered, this, [this]() {
            currentView()->model()->forceFullUpdate();
        });
        addAction(m_fullUpdateAction);
    }

    {
        m_scopeMenu = new KActionMenu(this);
        m_scopeMenu->setDelayed(false);
        m_scopeMenu->setToolTip(i18nc("@info:tooltip", "Which files to display the problems for"));
        m_scopeMenu->setObjectName(QStringLiteral("scopeMenu"));

        auto* scopeActions = new QActionGroup(this);

        m_currentDocumentAction = new QAction(this);
        m_currentDocumentAction->setText(i18n("Current Document"));
        m_currentDocumentAction->setToolTip(i18nc("@info:tooltip", "Display problems in current document"));

        auto* openDocumentsAction = new QAction(this);
        openDocumentsAction->setText(i18n("Open Documents"));
        openDocumentsAction->setToolTip(i18nc("@info:tooltip", "Display problems in all open documents"));

        auto* currentProjectAction = new QAction(this);
        currentProjectAction->setText(i18n("Current Project"));
        currentProjectAction->setToolTip(i18nc("@info:tooltip", "Display problems in current project"));

        auto* allProjectAction = new QAction(this);
        allProjectAction->setText(i18n("All Projects"));
        allProjectAction->setToolTip(i18nc("@info:tooltip", "Display problems in all projects"));

        QVector<QAction*> actions;
        actions.push_back(m_currentDocumentAction);
        actions.push_back(openDocumentsAction);
        actions.push_back(currentProjectAction);
        actions.push_back(allProjectAction);

        m_showAllAction = new QAction(this);
        m_showAllAction->setText(i18n("Show All"));
        m_showAllAction->setToolTip(i18nc("@info:tooltip", "Display ALL problems"));
        actions.push_back(m_showAllAction);

        for (QAction* action : qAsConst(actions)) {
            action->setCheckable(true);
            scopeActions->addAction(action);
            m_scopeMenu->addAction(action);
        }
        addAction(m_scopeMenu);

        connect(m_currentDocumentAction, &QAction::triggered, this, [this](){ setScope(CurrentDocument); });
        connect(openDocumentsAction, &QAction::triggered, this, [this](){ setScope(OpenDocuments); });
        connect(currentProjectAction, &QAction::triggered, this, [this](){ setScope(CurrentProject); });
        connect(allProjectAction, &QAction::triggered, this, [this](){ setScope(AllProjects); });
        connect(actions.last(), &QAction::triggered, this, [this](){ setScope(BypassScopeFilter); });
    }

    {
        m_showImportsAction = new QAction(this);
        addAction(m_showImportsAction);
        m_showImportsAction->setCheckable(true);
        m_showImportsAction->setChecked(false);
        m_showImportsAction->setText(i18n("Show Imports"));
        m_showImportsAction->setToolTip(i18nc("@info:tooltip", "Display problems in imported files"));
        connect(m_showImportsAction, &QAction::triggered, this, [this](bool checked) {
            currentView()->model()->setShowImports(checked);
        });
    }

    {
        m_severityActions = new QActionGroup(this);

        m_errorSeverityAction = new QAction(this);
        m_errorSeverityAction->setToolTip(i18nc("@info:tooltip", "Display errors"));
        m_errorSeverityAction->setIcon(QIcon::fromTheme(QStringLiteral("dialog-error")));
        m_errorSeverityAction->setIconText(i18n("Show Errors"));

        m_warningSeverityAction = new QAction(this);
        m_warningSeverityAction->setToolTip(i18nc("@info:tooltip", "Display warnings"));
        m_warningSeverityAction->setIcon(QIcon::fromTheme(QStringLiteral("dialog-warning")));
        m_warningSeverityAction->setIconText(i18n("Show Warnings"));

        m_hintSeverityAction = new QAction(this);
        m_hintSeverityAction->setToolTip(i18nc("@info:tooltip", "Display hints"));
        m_hintSeverityAction->setIcon(QIcon::fromTheme(QStringLiteral("dialog-information")));
        m_hintSeverityAction->setIconText(i18n("Show Hints"));

        QAction* severityActionArray[] = { m_errorSeverityAction, m_warningSeverityAction, m_hintSeverityAction };
        for (int i = 0; i < 3; ++i) {
            severityActionArray[i]->setCheckable(true);
            m_severityActions->addAction(severityActionArray[i]);
            addAction(severityActionArray[i]);
        }
        m_severityActions->setExclusive(false);

        m_hintSeverityAction->setChecked(true);
        m_warningSeverityAction->setChecked(true);
        m_errorSeverityAction->setChecked(true);

        connect(m_errorSeverityAction, &QAction::toggled, this, &ProblemsView::handleSeverityActionToggled);
        connect(m_warningSeverityAction, &QAction::toggled, this, &ProblemsView::handleSeverityActionToggled);
        connect(m_hintSeverityAction, &QAction::toggled, this, &ProblemsView::handleSeverityActionToggled);
    }

    {
        m_groupingMenu = new KActionMenu(i18n("Grouping"), this);
        m_groupingMenu->setDelayed(false);

        auto* groupingActions = new QActionGroup(this);

        QAction* noGroupingAction = new QAction(i18n("None"), this);
        QAction* pathGroupingAction = new QAction(i18n("Path"), this);
        QAction* severityGroupingAction = new QAction(i18n("Severity"), this);

        QAction* groupingActionArray[] = { noGroupingAction, pathGroupingAction, severityGroupingAction };
        for (unsigned i = 0; i < sizeof(groupingActionArray) / sizeof(QAction*); ++i) {
            QAction* action = groupingActionArray[i];
            action->setCheckable(true);
            groupingActions->addAction(action);
            m_groupingMenu->addAction(action);
        }
        addAction(m_groupingMenu);

        noGroupingAction->setChecked(true);

        connect(noGroupingAction, &QAction::triggered, this, [this](){ currentView()->model()->setGrouping(NoGrouping); });
        connect(pathGroupingAction, &QAction::triggered, this, [this](){ currentView()->model()->setGrouping(PathGrouping); });
        connect(severityGroupingAction, &QAction::triggered, this, [this](){ currentView()->model()->setGrouping(SeverityGrouping); });
    }

    {
        auto* filterTimer = new QTimer(this);
        filterTimer->setSingleShot(true);
        filterTimer->setInterval(500);

        connect(filterTimer, &QTimer::timeout, this, [this]() {
            setFilter(m_filterEdit->text());
        });

        m_filterEdit = new KExpandableLineEdit(this);
        m_filterEdit->setClearButtonEnabled(true);
        m_filterEdit->setPlaceholderText(i18n("Search..."));

        connect(m_filterEdit, &QLineEdit::textChanged,
                filterTimer, static_cast<void (QTimer::*)()>(&QTimer::start));

        auto* filterAction = new QWidgetAction(this);
        filterAction->setDefaultWidget(m_filterEdit);
        addAction(filterAction);

        m_prevTabIdx = -1;
        setFocusProxy(m_filterEdit);
    }
}

void ProblemsView::updateActions()
{
    auto problemModel = currentView()->model();
    Q_ASSERT(problemModel);

    m_fullUpdateAction->setVisible(problemModel->features().testFlag(ProblemModel::CanDoFullUpdate));
    m_fullUpdateAction->setToolTip(problemModel->fullUpdateTooltip());
    m_showImportsAction->setVisible(problemModel->features().testFlag(ProblemModel::CanShowImports));
    m_scopeMenu->setVisible(problemModel->features().testFlag(ProblemModel::ScopeFilter));
    m_severityActions->setVisible(problemModel->features().testFlag(ProblemModel::SeverityFilter));
    m_groupingMenu->setVisible(problemModel->features().testFlag(ProblemModel::Grouping));

    m_showAllAction->setVisible(problemModel->features().testFlag(ProblemModel::CanByPassScopeFilter));

    m_showImportsAction->setChecked(false);
    problemModel->setShowImports(false);

    // Show All should be default if it's supported. It helps with error messages that are otherwise invisible
    if (problemModel->features().testFlag(ProblemModel::CanByPassScopeFilter)) {
        //actions.last()->setChecked(true);
        setScope(BypassScopeFilter);
    } else {
        m_currentDocumentAction->setChecked(true);
        setScope(CurrentDocument);
    }

    problemModel->setSeverities(IProblem::Error | IProblem::Warning | IProblem::Hint);

    setFocus(); // set focus to default widget (filterEdit)
}

/// TODO: Move to util?
/// Note: Support for recursing into child indices would be nice
class ItemViewWalker
{
public:
    explicit ItemViewWalker(QItemSelectionModel* itemView);

    void selectNextIndex();
    void selectPreviousIndex();

    enum Direction { NextIndex, PreviousIndex };
    void selectIndex(Direction direction);

private:
    QItemSelectionModel* m_selectionModel;
};

ItemViewWalker::ItemViewWalker(QItemSelectionModel* itemView)
    : m_selectionModel(itemView)
{
}

void ItemViewWalker::selectNextIndex()
{
    selectIndex(NextIndex);
}

void ItemViewWalker::selectPreviousIndex()
{
    selectIndex(PreviousIndex);
}

void ItemViewWalker::selectIndex(Direction direction)
{
    if (!m_selectionModel) {
        return;
    }

    const QModelIndexList list = m_selectionModel->selectedRows();

    const QModelIndex currentIndex = list.value(0);
    if (!currentIndex.isValid()) {
        /// no selection yet, just select the first
        const QModelIndex firstIndex = m_selectionModel->model()->index(0, 0);
        m_selectionModel->setCurrentIndex(firstIndex, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
        return;
    }

    const int nextRow = currentIndex.row() + (direction == NextIndex ? 1 : -1);
    const QModelIndex nextIndex = currentIndex.sibling(nextRow, 0);
    if (!nextIndex.isValid()) {
        return; /// never invalidate the selection
    }

    m_selectionModel->setCurrentIndex(nextIndex, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
}

ProblemsView::ProblemsView(QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle(i18n("Problems"));
    setWindowIcon(QIcon::fromTheme(QStringLiteral("script-error"), windowIcon()));

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabPosition(QTabWidget::South);
    m_tabWidget->setDocumentMode(true);
    layout->addWidget(m_tabWidget);

    setupActions();
}

ProblemsView::~ProblemsView()
{
}

void ProblemsView::load()
{
    m_tabWidget->clear();

    KDevelop::ProblemModelSet* pms = KDevelop::ICore::self()->languageController()->problemModelSet();
    QVector<KDevelop::ModelData> v = pms->models();

    QVectorIterator<KDevelop::ModelData> itr(v);
    while (itr.hasNext()) {
        const KDevelop::ModelData& data = itr.next();
        addModel(data);
    }

    connect(pms, &ProblemModelSet::added, this, &ProblemsView::onModelAdded);
    connect(pms, &ProblemModelSet::removed, this, &ProblemsView::onModelRemoved);
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &ProblemsView::onCurrentChanged);

    if (m_tabWidget->currentIndex() == 0) {
        updateActions();
        return;
    }

    m_tabWidget->setCurrentIndex(0);
}

void ProblemsView::onModelAdded(const ModelData& data)
{
    addModel(data);
}

void ProblemsView::showModel(const QString& id)
{
    for (int i = 0; i < m_models.size(); ++i) {
        if (m_models[i].id == id) {
            m_tabWidget->setCurrentIndex(i);
            return;
        }
    }
}

void ProblemsView::onModelRemoved(const QString& id)
{
    for (int i = 0; i < m_models.size(); ++i) {
        if (m_models[i].id == id) {
            m_models.remove(i);
            QWidget* w = m_tabWidget->widget(i);
            m_tabWidget->removeTab(i);
            delete w;
            return;
        }
    }
}

void ProblemsView::onCurrentChanged(int idx)
{
    if (idx == -1)
        return;

    setFilter(QString(), m_prevTabIdx);
    setFilter(QString());
    m_prevTabIdx = idx;

    updateActions();
}

void ProblemsView::onViewChanged()
{
    auto* view = static_cast<ProblemTreeView*>(sender());
    int idx = m_tabWidget->indexOf(view);
    int rows = view->model()->rowCount();

    updateTab(idx, rows);
}

void ProblemsView::addModel(const ModelData& newData)
{
    // We implement follows tabs order:
    //
    // 1) First tab always used by "Parser" model due to it's the most important
    //    problem listing, it should be at the front (K.Funk idea at #kdevelop IRC channel).
    //
    // 2) Other tabs are alphabetically ordered.

    static const QString parserId = QStringLiteral("Parser");

    auto model = newData.model;
    auto view = new ProblemTreeView(nullptr, model);
    connect(view, &ProblemTreeView::changed, this, &ProblemsView::onViewChanged);
    connect(model, &ProblemModel::fullUpdateTooltipChanged,
            this, [this, model]() {
                if (currentView()->model() == model) {
                    m_fullUpdateAction->setToolTip(model->fullUpdateTooltip());
                }
            });

    int insertIdx = 0;
    if (newData.id != parserId) {
        for (insertIdx = 0; insertIdx < m_models.size(); ++insertIdx) {
            const ModelData& currentData = m_models[insertIdx];

            // Skip first element if it's already occupied by "Parser" model
            if (insertIdx == 0 && currentData.id == parserId)
                continue;

            if (currentData.name.localeAwareCompare(newData.name) > 0)
                break;
        }
    }
    m_tabWidget->insertTab(insertIdx, view, newData.name);
    m_models.insert(insertIdx, newData);

    updateTab(insertIdx, model->rowCount());
}

void ProblemsView::updateTab(int idx, int rows)
{
    if (idx < 0 || idx >= m_models.size())
        return;

    const QString name = m_models[idx].name;
    const QString tabText = i18nc("%1: tab name, %2: number of problems", "%1 (%2)", name, rows);
    m_tabWidget->setTabText(idx, tabText);
}

ProblemTreeView* ProblemsView::currentView() const
{
    return qobject_cast<ProblemTreeView*>(m_tabWidget->currentWidget());
}

void ProblemsView::selectNextItem()
{
    auto view = currentView();
    if (view) {
        ItemViewWalker walker(view->selectionModel());
        walker.selectNextIndex();
        view->openDocumentForCurrentProblem();
    }
}

void ProblemsView::selectPreviousItem()
{
    auto view = currentView();
    if (view) {
        ItemViewWalker walker(view->selectionModel());
        walker.selectPreviousIndex();
        view->openDocumentForCurrentProblem();
    }
}

void ProblemsView::handleSeverityActionToggled()
{
    currentView()->model()->setSeverities( (m_errorSeverityAction->isChecked() ? IProblem::Error : IProblem::Severities()) |
                            (m_warningSeverityAction->isChecked() ? IProblem::Warning : IProblem::Severities()) |
                            (m_hintSeverityAction->isChecked() ? IProblem::Hint : IProblem::Severities()) );
}

void ProblemsView::setScope(int scope)
{
    m_scopeMenu->setText(i18n("Scope: %1", m_scopeMenu->menu()->actions().at(scope)->text()));

    currentView()->model()->setScope(scope);
}

void ProblemsView::setFilter(const QString& filterText)
{
    setFilter(filterText, m_tabWidget->currentIndex());
}

void ProblemsView::setFilter(const QString& filterText, int tabIdx)
{
    if (tabIdx < 0 || tabIdx >= m_tabWidget->count())
        return;

    auto* view = static_cast<ProblemTreeView*>(m_tabWidget->widget(tabIdx));
    int rows = view->setFilter(filterText);

    updateTab(tabIdx, rows);

    if (tabIdx == m_tabWidget->currentIndex()) {
        QSignalBlocker blocker(m_filterEdit);
        m_filterEdit->setText(filterText);
    }
}

}
