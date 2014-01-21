/*
  Copyright 2007 Roberto Raggi <roberto@kdevelop.org>
  Copyright 2007 Hamish Rodda <rodda@kde.org>
  Copyright 2011 Alexander Dymo <adymo@kdevelop.org>

  Permission to use, copy, modify, distribute, and sell this software and its
  documentation for any purpose is hereby granted without fee, provided that
  the above copyright notice appear in all copies and that both that
  copyright notice and this permission notice appear in supporting
  documentation.

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  KDEVELOP TEAM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
  AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "idealcontroller.h"

#include <QApplication>
#include <QIcon>
#include <QMainWindow>
#include <QStylePainter>

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kshortcutwidget.h>
#include <KActionCollection>
#include <KActionMenu>
#include <KAcceleratorManager>
#include <KMenu>
#include <KToolBar>
#include <KDialog>

#include "area.h"
#include "view.h"
#include "document.h"
#include "mainwindow.h"
#include "ideallayout.h"
#include "idealtoolbutton.h"
#include "idealdockwidget.h"
#include "idealbuttonbarwidget.h"
#include <QVBoxLayout>
#include <QLabel>

using namespace Sublime;

IdealController::IdealController(Sublime::MainWindow* mainWindow):
    QObject(mainWindow), m_mainWindow(mainWindow)
{
    leftBarWidget = new IdealButtonBarWidget(Qt::LeftDockWidgetArea, this, m_mainWindow);
    connect(leftBarWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotDockBarContextMenuRequested(QPoint)));

    rightBarWidget = new IdealButtonBarWidget(Qt::RightDockWidgetArea, this, m_mainWindow);
    connect(rightBarWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotDockBarContextMenuRequested(QPoint)));

    bottomBarWidget = new IdealButtonBarWidget(Qt::BottomDockWidgetArea, this, m_mainWindow);
    bottomStatusBarLocation = bottomBarWidget->corner();
    connect(bottomBarWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotDockBarContextMenuRequested(QPoint)));

    topBarWidget = new IdealButtonBarWidget(Qt::TopDockWidgetArea, this, m_mainWindow);
    connect(topBarWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotDockBarContextMenuRequested(QPoint)));

    m_docks = qobject_cast<KActionMenu*>(mainWindow->action("docks_submenu"));

    m_showLeftDock = qobject_cast<QAction*>(m_mainWindow->action("show_left_dock"));
    m_showRightDock = qobject_cast<QAction*>(m_mainWindow->action("show_right_dock"));
    m_showBottomDock = qobject_cast<QAction*>(m_mainWindow->action("show_bottom_dock"));
    m_showTopDock = qobject_cast<QAction*>(m_mainWindow->action("show_top_dock"));

    connect(m_mainWindow, SIGNAL(settingsLoaded()), this, SLOT(loadSettings()));

}

void IdealController::addView(Qt::DockWidgetArea area, View* view)
{
    IdealDockWidget *dock = new IdealDockWidget(this, m_mainWindow);
    // dock object name is used to store toolview settings
    QString dockObjectName = view->document()->title();
    // support different configuration for same docks opened in different areas
    if (m_mainWindow->area())
        dockObjectName += '_' + m_mainWindow->area()->objectName();

    dock->setObjectName(dockObjectName);

    KAcceleratorManager::setNoAccel(dock);
    QWidget *w = view->widget(dock);
    if (w->parent() == 0)
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
      QMainWindow *toolView = new QMainWindow();
      KToolBar *toolBar = new KToolBar(toolView);
      int iconSize = m_mainWindow->style()->pixelMetric(QStyle::PM_SmallIconSize);
      toolBar->setIconSize(QSize(iconSize, iconSize));
      toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
      toolBar->setWindowTitle(i18n("%1 Tool Bar", w->windowTitle()));
      toolBar->setFloatable(false);
      toolBar->setMovable(false);
      toolBar->addActions(toolBarActions);
      toolView->setCentralWidget(w);
      toolView->addToolBar(toolBar);
      dock->setWidget(toolView);
    }

    dock->setWindowTitle(view->widget()->windowTitle());
    dock->setWindowIcon(view->widget()->windowIcon());
    dock->setFocusProxy(dock->widget());

    if (IdealButtonBarWidget* bar = barForDockArea(area)) {
        QAction* action = bar->addWidget(
            view->document()->title(), dock,
            static_cast<MainWindow*>(parent())->area(), view);
        m_dockwidget_to_action[dock] = m_view_to_action[view] = action;

        m_docks->addAction(action);
        connect(dock, SIGNAL(closeRequested()), action, SLOT(toggle()));
    }

    connect(dock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), this, SLOT(dockLocationChanged(Qt::DockWidgetArea)));

    dock->hide();

    docks.insert(dock);
}

void IdealController::dockLocationChanged(Qt::DockWidgetArea area)
{
    IdealDockWidget *dock = qobject_cast<IdealDockWidget*>(sender());
    View *view = dock->view();
    QAction* action = m_view_to_action.value(view);

    if (dock->dockWidgetArea() == area) {
        // this event can happen even when dock changes its location within the same area
        // usecases:
        // 1) user drags to the same area
        // 2) user rearranges toolviews inside the same area
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

    docks.insert(dock);

    if (IdealButtonBarWidget* bar = barForDockArea(area)) {
        QAction* action = bar->addWidget(
            view->document()->title(), dock,
            static_cast<MainWindow*>(parent())->area(), view);
        m_dockwidget_to_action[dock] = m_view_to_action[view] = action;

        // at this point the dockwidget is visible (user dragged it)
        // properly set up UI state
        bar->showWidget(action, true);

        // the dock should now be the "last" opened in a new area, not in the old area
        for (QMap<Qt::DockWidgetArea, QWeakPointer<IdealDockWidget> >::iterator it = lastDockWidget.begin(); it != lastDockWidget.end(); ++it) {
            if (it->data() == dock)
                it->clear();
        }
        lastDockWidget[area] = dock;

        // after drag, the toolview loses focus, so focus it again
        dock->setFocus(Qt::ShortcutFocusReason);

        m_docks->addAction(action);
    }

    if (area == Qt::BottomDockWidgetArea || area == Qt::TopDockWidgetArea)
        dock->setFeatures( QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | IdealDockWidget::DockWidgetVerticalTitleBar );
    else
        dock->setFeatures( QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable );
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
            return 0;
    }
}

void IdealController::slotDockBarContextMenuRequested(QPoint position)
{
    IdealButtonBarWidget* bar = qobject_cast<IdealButtonBarWidget*>(sender());
    Q_ASSERT(bar);

    emit dockBarContextMenuRequested(bar->area(), bar->mapToGlobal(position));
}

void IdealController::raiseView(View* view, RaiseMode mode)
{
    /// @todo GroupWithOtherViews is disabled for now by forcing "mode = HideOtherViews".
    ///       for the release of KDevelop 4.3.
    ///       Reason: Inherent bugs which need significant changes to be fixed.
    ///       Example: Open two equal toolviews (for example 2x konsole),
    ///                activate one, switch area, switch back, -> Both are active instead of one.
    ///       The problem is that views are identified purely by their factory-id, which is equal
    ///       for toolviews of the same type.
    mode = HideOtherViews;
   
    QAction* action = m_view_to_action.value(view);
    Q_ASSERT(action);

    QWidget *focusWidget = m_mainWindow->focusWidget();

    action->setProperty("raise", mode);
    action->setChecked(true);
    // TODO: adymo: hack: focus needs to stay inside the previously
    // focused widget (setChecked will focus the toolview)
    if (focusWidget)
        focusWidget->setFocus(Qt::ShortcutFocusReason);
}

QList< IdealDockWidget* > IdealController::allDockWidgets()
{
    return docks.toList();
}

void IdealController::showDockWidget(IdealDockWidget* dock, bool show)
{
    Q_ASSERT(docks.contains(dock));

    Qt::DockWidgetArea area = dock->dockWidgetArea();

    if (show) {
        m_mainWindow->addDockWidget(area, dock);
        dock->show();
    } else {
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
    return bottomStatusBarLocation;
}

QAction* IdealController::actionForView(View* view) const
{
    return m_view_to_action.value(view);
}

void IdealController::setShowDockStatus(Qt::DockWidgetArea area, bool checked)
{
    QAction* action = actionForArea(area);
    if (action->isChecked() != checked) {
        bool blocked = action->blockSignals(true);
        action->setChecked(checked);
        action->blockSignals(blocked);
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
    IdealDockWidget *dock = qobject_cast<IdealDockWidget *>(viewParent);
    if (!dock) { // toolviews with a toolbar live in a QMainWindow which lives in a Dock
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
        view->widget()->setParent(0);

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
            action->blockSignals(true);
            action->setChecked(true);
            action->blockSignals(false);
        }
        return;
    }

    if (!show) {
        // close all toolviews
        foreach (QAction *action, bar->actions()) {
            if (action->isChecked())
                action->setChecked(false);
        }
        focusEditor();
    } else {
        // open the last opened toolview (or the first one) and focus it
        if (lastDock) {
            if (QAction *action = m_dockwidget_to_action.value(lastDock))
                action->setChecked(show);

            lastDock->setFocus(Qt::ShortcutFocusReason);
            return;
        }

        if (barForDockArea(area)->actions().count())
            barForDockArea(area)->actions().first()->setChecked(show);
    }
}

// returns currently focused dock widget (if any)
IdealDockWidget* IdealController::currentDockWidget()
{
    QWidget *w = m_mainWindow->focusWidget();
    while (true) {
        if (!w) break;
        IdealDockWidget *dockCandidate = qobject_cast<IdealDockWidget*>(w);
        if (dockCandidate)
            return dockCandidate;

        w = w->parentWidget();
    }
    return 0;
}

void IdealController::goPrevNextDock(IdealController::Direction direction)
{
    IdealDockWidget *currentDock = currentDockWidget();
    if (!currentDock)
        return;
    IdealButtonBarWidget *bar = barForDockArea(currentDock->dockWidgetArea());

    int index = bar->actions().indexOf(m_dockwidget_to_action.value(currentDock));

    if (direction == NextDock) {
        if (index < 1)
            index = bar->actions().count() - 1;
        else
            --index;
    } else {
        if (index == -1 || index == bar->actions().count() - 1)
            index = 0;
        else
            ++index;
    }

    if (index < bar->actions().count()) {
        QAction* action = bar->actions().at(index);
        action->setChecked(true);
    }
}

void IdealController::toggleDocksShown()
{
    QList<QAction*> allActions;
    allActions += leftBarWidget->actions();
    allActions += bottomBarWidget->actions();
    allActions += rightBarWidget->actions();

    bool show = true;
    foreach (QAction *action, allActions) {
        if (action->isChecked()) {
            show = false;
            break;
        }
    }

    toggleDocksShown(leftBarWidget, show);
    toggleDocksShown(bottomBarWidget, show);
    toggleDocksShown(rightBarWidget, show);
}

void IdealController::toggleDocksShown(IdealButtonBarWidget* bar, bool show)
{
    if (!show) {
        foreach (QAction *action, bar->actions()) {
            if (action->isChecked())
                action->setChecked(false);
        }
        focusEditor();
    } else {
        IdealDockWidget *lastDock = lastDockWidget[bar->area()].data();
        if (lastDock)
            m_dockwidget_to_action[lastDock]->setChecked(true);
    }
}

void IdealController::loadSettings()
{
    KConfigGroup cg(KSharedConfig::openConfig(), "UiSettings");

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

void IdealController::setWidthForArea(Qt::DockWidgetArea area, int width)
{
    m_widthsForAreas[area] = width;
}

void IdealController::emitWidgetResized(Qt::DockWidgetArea dockArea, int thickness)
{
    emit widgetResized(dockArea, thickness);
}
