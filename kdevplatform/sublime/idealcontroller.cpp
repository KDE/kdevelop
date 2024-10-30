/*
    SPDX-FileCopyrightText: 2007 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2011 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LicenseRef-MIT-KDevelop-Ideal
*/

#include "idealcontroller.h"

#include <QMainWindow>
#include <QToolBar>
#include <QStyle>

#include <KAcceleratorManager>
#include <KActionMenu>
#include <KLocalizedString>
#include <KSharedConfig>

#include "area.h"
#include "view.h"
#include "document.h"
#include "mainwindow.h"
#include "ideallayout.h"
#include "idealdockwidget.h"
#include "idealbuttonbarwidget.h"

using namespace Sublime;

IdealController::IdealController(Sublime::MainWindow* mainWindow):
    QObject(mainWindow), m_mainWindow(mainWindow)
{
    leftBarWidget = new IdealButtonBarWidget(Qt::LeftDockWidgetArea, this, m_mainWindow);
    connect(leftBarWidget, &IdealButtonBarWidget::customContextMenuRequested,
            this, &IdealController::slotDockBarContextMenuRequested);

    rightBarWidget = new IdealButtonBarWidget(Qt::RightDockWidgetArea, this, m_mainWindow);
    connect(rightBarWidget, &IdealButtonBarWidget::customContextMenuRequested,
            this, &IdealController::slotDockBarContextMenuRequested);

    bottomBarWidget = new IdealButtonBarWidget(Qt::BottomDockWidgetArea, this, m_mainWindow);
    m_bottomStatusBarLocation = bottomBarWidget->corner();
    connect(bottomBarWidget, &IdealButtonBarWidget::customContextMenuRequested,
            this, &IdealController::slotDockBarContextMenuRequested);

    topBarWidget = new IdealButtonBarWidget(Qt::TopDockWidgetArea, this, m_mainWindow);
    connect(topBarWidget, &IdealButtonBarWidget::customContextMenuRequested,
            this, &IdealController::slotDockBarContextMenuRequested);

    m_docks = qobject_cast<KActionMenu*>(mainWindow->action(QStringLiteral("docks_submenu")));

    m_showLeftDock = m_mainWindow->action(QStringLiteral("show_left_dock"));
    m_showRightDock = m_mainWindow->action(QStringLiteral("show_right_dock"));
    m_showBottomDock = m_mainWindow->action(QStringLiteral("show_bottom_dock"));

    // the 'show top dock' action got removed (IOW, it's never created)
    // (let's keep this code around if we ever want to reintroduce the feature...
    m_showTopDock = m_mainWindow->action(QStringLiteral("show_top_dock"));

    connect(m_mainWindow, &MainWindow::settingsLoaded, this, &IdealController::loadSettings);

}

void IdealController::addView(Qt::DockWidgetArea area, View* view)
{
    auto *dock = new IdealDockWidget(this, m_mainWindow);
    // dock object name is used to store tool view settings
    QString dockObjectName = view->document()->title();
    // support different configuration for same docks opened in different areas
    if (m_mainWindow->area())
        dockObjectName += QLatin1Char('_') + m_mainWindow->area()->objectName();

    dock->setObjectName(dockObjectName);

    KAcceleratorManager::setNoAccel(dock);
    QWidget *w = view->widget(dock);
    if (w->parent() == nullptr)
    {
        /* Could happen when we're moving the widget from
           one IdealDockWidget to another.  See moveView below.
           In this case, we need to reparent the widget. */
        w->setParent(dock);
    }

    QList<QAction *> toolBarActions = view->toolBarActions();
    if (toolBarActions.isEmpty()) {
      dock->setWidget(w);
    } else {
      auto* toolView = new QMainWindow();
      auto *toolBar = new QToolBar(toolView);
      int iconSize = m_mainWindow->style()->pixelMetric(QStyle::PM_SmallIconSize);
      toolBar->setIconSize(QSize(iconSize, iconSize));
      toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
      toolBar->setWindowTitle(i18nc("@title:window", "%1 Toolbar", w->windowTitle()));
      toolBar->setFloatable(false);
      toolBar->setMovable(false);
      toolBar->addActions(toolBarActions);
      toolView->setCentralWidget(w);
      toolView->setFocusProxy(w);
      toolView->addToolBar(toolBar);
      dock->setWidget(toolView);

      KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("UiSettings/Docks/ToolbarEnabled"));
      toolBar->setVisible(cg.readEntry(dockObjectName, true));
      connect(toolBar->toggleViewAction(), &QAction::toggled,
            this, [toolBar, dockObjectName](){
                KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("UiSettings/Docks/ToolbarEnabled"));
                cg.writeEntry(dockObjectName, toolBar->toggleViewAction()->isChecked());
            });
    }

    dock->setWindowTitle(view->widget()->windowTitle());
    dock->setWindowIcon(view->widget()->windowIcon());
    dock->setFocusProxy(dock->widget());

    if (IdealButtonBarWidget* bar = barForDockArea(area)) {
        QAction* action = bar->addWidget(dock, m_mainWindow->area(), view);
        m_dockwidget_to_action[dock] = m_view_to_action[view] = action;

        m_docks->addAction(action);
        connect(dock, &IdealDockWidget::closeRequested, action, &QAction::toggle);
    }

    connect(dock, &IdealDockWidget::dockLocationChanged, this, &IdealController::dockLocationChanged);

    dock->hide();
}

void IdealController::dockLocationChanged(Qt::DockWidgetArea area)
{
    // Seems since Qt 5.13 the signal QDockWidget::dockLocationChanged is emitted also when the dock changes
    // to be floating, with area = Qt::NoDockWidgetArea. The current code is not designed for this,
    // so just ignore the signal in that case for now
    if (area == Qt::NoDockWidgetArea) {
        return;
    }

    auto *dock = qobject_cast<IdealDockWidget*>(sender());
    View *view = dock->view();
    QAction* action = m_view_to_action.value(view);

    if (dock->dockWidgetArea() == area) {
        // this event can happen even when dock changes its location within the same area
        // usecases:
        // 1) user drags to the same area
        // 2) user rearranges tool views inside the same area
        // 3) state restoration shows the dock widget

        // in 3rd case we need to show dock if we don't want it to be shown
        // TODO: adymo: invent a better solution for the restoration problem
        if (!action->isChecked() && dock->isVisible()) {
            dock->hide();
        }

        return;
    }

    if (IdealButtonBarWidget* bar = barForDockArea(dock->dockWidgetArea()))
        bar->removeAction(action);

    if (IdealButtonBarWidget* bar = barForDockArea(area)) {
        QAction* action = bar->addWidget(dock, m_mainWindow->area(), view);
        m_dockwidget_to_action[dock] = m_view_to_action[view] = action;

        // at this point the dockwidget is visible (user dragged it)
        // properly set up UI state
        bar->showWidget(action, true);

        // the dock should now be the "last" opened in a new area, not in the old area
        for (auto& dockWidgetPtr : lastDockWidget) {
            if (dockWidgetPtr.data() == dock) {
                dockWidgetPtr.clear();
            }
        }
        lastDockWidget[area] = dock;

        // after drag, the tool view loses focus, so focus it again
        dock->setFocus(Qt::ShortcutFocusReason);

        m_docks->addAction(action);
    }

    if (area == Qt::BottomDockWidgetArea || area == Qt::TopDockWidgetArea)
        dock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetVerticalTitleBar);
    else
        dock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
}

IdealButtonBarWidget* IdealController::barForDockArea(Qt::DockWidgetArea area) const
{
    switch (area) {
        case Qt::LeftDockWidgetArea:
            return leftBarWidget;

        case Qt::TopDockWidgetArea:
            return topBarWidget;

        case Qt::RightDockWidgetArea:
            return rightBarWidget;

        case Qt::BottomDockWidgetArea:
            return bottomBarWidget;

        default:
            Q_ASSERT(false);
            return nullptr;
    }
}

void IdealController::slotDockBarContextMenuRequested(const QPoint& position)
{
    auto* bar = qobject_cast<IdealButtonBarWidget*>(sender());
    Q_ASSERT(bar);

    emit dockBarContextMenuRequested(bar->area(), bar->mapToGlobal(position));
}

void IdealController::raiseView(View* view, RaiseMode mode)
{
    /// @todo GroupWithOtherViews is disabled for now by forcing "mode = HideOtherViews".
    ///       for the release of KDevelop 4.3.
    ///       Reason: Inherent bugs which need significant changes to be fixed.
    ///       Example: Open two equal tool views (for example 2x konsole),
    ///                activate one, switch area, switch back, -> Both are active instead of one.
    ///       The problem is that views are identified purely by their factory-id, which is equal
    ///       for tool views of the same type.
    mode = HideOtherViews;

    QAction* action = m_view_to_action.value(view);
    Q_ASSERT(action);

    QWidget *focusWidget = m_mainWindow->focusWidget();

    action->setProperty("raise", mode);
    action->setChecked(true);
    // TODO: adymo: hack: focus needs to stay inside the previously
    // focused widget (setChecked will focus the tool view)
    if (focusWidget)
        focusWidget->setFocus(Qt::ShortcutFocusReason);
}

void IdealController::showDockWidget(IdealDockWidget* dock, bool show)
{
    Qt::DockWidgetArea area = dock->dockWidgetArea();

    if (show) {
        m_mainWindow->addDockWidget(area, dock);
        dock->show();
    } else {
        // Calling dock->hide() is necessary to make QMainWindow::saveState() store its removed state correctly.
        // Without this call, a previously hidden dock widget can become visible on next KDevelop start, while
        // its associated tool view action (button) correctly but inconsistently remains unchecked (see QTBUG-11909).
        dock->hide();
        m_mainWindow->removeDockWidget(dock);
    }

    setShowDockStatus(area, show);
    emit dockShown(dock->view(), Sublime::dockAreaToPosition(area), show);

    if (!show)
        // Put the focus back on the editor if a dock was hidden
        focusEditor();
    else {
        // focus the dock
        dock->setFocus(Qt::ShortcutFocusReason);
    }
}

void IdealController::focusEditor()
{
    if (View* view = m_mainWindow->activeView())
        if (view->hasWidget())
            view->widget()->setFocus(Qt::ShortcutFocusReason);
}

QWidget* IdealController::statusBarLocation() const
{
    return m_bottomStatusBarLocation;
}

QAction* IdealController::actionForView(View* view) const
{
    return m_view_to_action.value(view);
}

void IdealController::setShowDockStatus(Qt::DockWidgetArea area, bool checked)
{
    QAction* action = actionForArea(area);
    if (action->isChecked() != checked) {
        QSignalBlocker blocker(action);
        action->setChecked(checked);
    }
}

QAction* IdealController::actionForArea(Qt::DockWidgetArea area) const
{
    switch (area) {
        case Qt::LeftDockWidgetArea:
        default:
            return m_showLeftDock;
        case Qt::RightDockWidgetArea:
            return m_showRightDock;
        case Qt::TopDockWidgetArea:
            return m_showTopDock;
        case Qt::BottomDockWidgetArea:
            return m_showBottomDock;
    }
}

void IdealController::removeView(View* view, bool nondestructive)
{
    Q_ASSERT(m_view_to_action.contains(view));
    QAction* action = m_view_to_action.value(view);

    QWidget *viewParent = view->widget()->parentWidget();
    auto *dock = qobject_cast<IdealDockWidget *>(viewParent);
    if (!dock) { // tool views with a toolbar live in a QMainWindow which lives in a Dock
        Q_ASSERT(qobject_cast<QMainWindow*>(viewParent));
        viewParent = viewParent->parentWidget();
        dock = qobject_cast<IdealDockWidget*>(viewParent);
    }
    Q_ASSERT(dock);

    /* Hide the view, first.  This is a workaround -- if we
       try to remove IdealDockWidget without this, then eventually
       a call to IdealMainLayout::takeAt will be made, which
       method asserts immediately.  */
    action->setChecked(false);

    if (IdealButtonBarWidget* bar = barForDockArea(dock->dockWidgetArea()))
        bar->removeAction(action);

    m_view_to_action.remove(view);
    m_dockwidget_to_action.remove(dock);

    if (nondestructive)
        view->widget()->setParent(nullptr);

    delete dock;
}

void IdealController::moveView(View *view, Qt::DockWidgetArea area)
{
    removeView(view);
    addView(area, view);
}

void IdealController::showBottomDock(bool show)
{
    showDock(Qt::BottomDockWidgetArea, show);
}

void IdealController::showLeftDock(bool show)
{
    showDock(Qt::LeftDockWidgetArea, show);
}

void IdealController::showRightDock(bool show)
{
    showDock(Qt::RightDockWidgetArea, show);
}

void IdealController::hideDocks(IdealButtonBarWidget *bar)
{
    const auto barActions = bar->actions();
    for (QAction* action : barActions) {
        if (action->isChecked())
            action->setChecked(false);
    }
    focusEditor();
}

void IdealController::showDock(Qt::DockWidgetArea area, bool show)
{
    IdealButtonBarWidget *bar = barForDockArea(area);
    if (!bar) return;
    IdealDockWidget *lastDock = lastDockWidget[area].data();

    if (lastDock && lastDock->isVisible() && !lastDock->hasFocus()) {
        lastDock->setFocus(Qt::ShortcutFocusReason);
        // re-sync action state given we may have asked for the dock to be hidden
        QAction* action = actionForArea(area);
        if (!action->isChecked()) {
            QSignalBlocker blocker(action);
            action->setChecked(true);
        }
        return;
    }

    if (!show) {
        hideDocks(bar);
    } else {
        // open the last opened tool view (or the first one) and focus it
        if (lastDock) {
            if (QAction *action = m_dockwidget_to_action.value(lastDock))
                action->setChecked(true);

            lastDock->setFocus(Qt::ShortcutFocusReason);
            return;
        }

        const auto barActions = bar->actions();
        if (!barActions.isEmpty())
            barActions.first()->setChecked(true);
    }
}

// returns currently focused dock widget (if any)
IdealDockWidget* IdealController::currentDockWidget() const
{
    QWidget *w = m_mainWindow->focusWidget();
    while (true) {
        if (!w) break;
        auto *dockCandidate = qobject_cast<IdealDockWidget*>(w);
        if (dockCandidate)
            return dockCandidate;

        w = w->parentWidget();
    }
    return nullptr;
}

void IdealController::goPrevNextDock(IdealController::Direction direction)
{
    IdealDockWidget *currentDock = currentDockWidget();
    if (!currentDock)
        return;
    IdealButtonBarWidget *bar = barForDockArea(currentDock->dockWidgetArea());

    int index = bar->actions().indexOf(m_dockwidget_to_action.value(currentDock));
    int step = (direction == NextDock) ? 1 : -1;

    if (bar->area() == Qt::BottomDockWidgetArea)
        step = -step;

    index += step;

    if (index < 0)
        index = bar->actions().count() - 1;

    if (index > bar->actions().count() - 1)
        index = 0;

    bar->actions().at(index)->setChecked(true);
}

void IdealController::toggleDocksShown()
{
    bool anyBarShown =
        (leftBarWidget->isShown() && !leftBarWidget->isLocked()) ||
        (bottomBarWidget->isShown() && !bottomBarWidget->isLocked()) ||
        (rightBarWidget->isShown() && !rightBarWidget->isLocked());

    if (anyBarShown) {
        leftBarWidget->saveShowState();
        bottomBarWidget->saveShowState();
        rightBarWidget->saveShowState();
    }

    if (!leftBarWidget->isLocked())
        toggleDocksShown(leftBarWidget, !anyBarShown && leftBarWidget->lastShowState());

    if (!bottomBarWidget->isLocked())
        toggleDocksShown(bottomBarWidget, !anyBarShown && bottomBarWidget->lastShowState());

    if (!rightBarWidget->isLocked())
        toggleDocksShown(rightBarWidget, !anyBarShown && rightBarWidget->lastShowState());
}

void IdealController::toggleDocksShown(IdealButtonBarWidget* bar, bool show)
{
    if (!show) {
        hideDocks(bar);
    } else {
        IdealDockWidget *lastDock = lastDockWidget[bar->area()].data();
        if (lastDock)
            m_dockwidget_to_action[lastDock]->setChecked(true);
    }
}

void IdealController::loadSettings()
{
    KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("UiSettings"));

    int bottomOwnsBottomLeft = cg.readEntry("BottomLeftCornerOwner", 0);
    if (bottomOwnsBottomLeft)
        m_mainWindow->setCorner(Qt::BottomLeftCorner, Qt::BottomDockWidgetArea);
    else
        m_mainWindow->setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);

    int bottomOwnsBottomRight = cg.readEntry("BottomRightCornerOwner", 0);
    if (bottomOwnsBottomRight)
        m_mainWindow->setCorner(Qt::BottomRightCorner, Qt::BottomDockWidgetArea);
    else
        m_mainWindow->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
}

#include "moc_idealcontroller.cpp"
