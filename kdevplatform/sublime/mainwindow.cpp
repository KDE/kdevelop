/*
    SPDX-FileCopyrightText: 2006-2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mainwindow.h"
#include "mainwindow_p.h"

#include <QApplication>
#include <QDockWidget>
#include <QMenuBar>
#include <QStatusBar>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KToolBar>

#include "area.h"
#include "view.h"
#include "controller.h"
#include "container.h"
#include "idealcontroller.h"
#include "holdupdates.h"
#include <debug.h>

namespace {
/**
 * @return the main window UI config group for a given area
 *
 * @note Each area has its own separate main window UI config group. This allows to customize
 *       each area independently from others. For example, each area can have unique sets
 *       and locations of visible toolbars, enabled tool views and visible tool views.
 */
[[nodiscard]] QString configGroupName(const Sublime::Area* area)
{
    auto groupName = QStringLiteral("MainWindow");
    if (area) {
        groupName += QLatin1Char('_') + area->objectName();
    }
    return groupName;
}

} // unnamed namespace

namespace Sublime {

MainWindow::MainWindow(Controller *controller, Qt::WindowFlags flags)
    : KParts::MainWindow(nullptr, flags)
    , d_ptr(new MainWindowPrivate(this, controller))
{
    qCDebug(SUBLIME) << "creating mainwindow";
    connect(this, &MainWindow::destroyed, controller, QOverload<>::of(&Controller::areaReleased));

    // don't allow AllowTabbedDocks - that doesn't make sense for "ideal" UI
    setDockOptions(QMainWindow::AnimatedDocks);
}

bool MainWindow::containsView(View* view) const
{
    const auto areas = this->areas();

    return std::any_of(areas.begin(), areas.end(), [view](Area* area) {
        return area->views().contains(view);
    });
}

QList< Area* > MainWindow::areas() const
{
    QList< Area* > areas = controller()->areas(const_cast<MainWindow*>(this));
    if(areas.isEmpty())
        areas = controller()->defaultAreas();

    return areas;
}

MainWindow::~MainWindow()
{
    qCDebug(SUBLIME) << "destroying mainwindow";
}

void MainWindow::reconstructViews(const QList<View*>& topViews)
{
    Q_D(MainWindow);

    d->reconstructViews(topViews);
}

QList<View*> MainWindow::topViews() const
{
    Q_D(const MainWindow);

    QList<View*> topViews;
    const auto views = d->area->views();
    for (View* view : views) {
        if(view->hasWidget())
        {
            QWidget* widget = view->widget();
            if(widget->parent() && widget->parent()->parent())
            {
                auto* container = qobject_cast<Container*>(widget->parent()->parent());
                if(container->currentWidget() == widget)
                    topViews << view;
            }
        }
    }
    return topViews;
}

QSet<Container*> MainWindow::containers() const
{
    Q_D(const MainWindow);

    return QSet<Container*>(d->viewContainers.cbegin(), d->viewContainers.cend());
}

void MainWindow::setArea(Area *area)
{
    Q_D(MainWindow);

    if (d->area) {
        // Disconnect the previous area. We really do not want to mess with
        // the main window if an area not visible now is modified. Further,
        // if setArea() is called with the same area as is current
        // now, we do not want to connect to the same signals twice.
        disconnect(d->area, nullptr, d, nullptr);
    }

    bool differentArea = (area != d->area);
    if (d->autoAreaSettingsSave && differentArea)
        saveSettings();

    HoldUpdates hu(this);
    if (d->area)
        clearArea();
    d->area = area;
    d->reconstruct();

    if(d->area->activeView())
        activateView(d->area->activeView());
    else
        d->activateFirstVisibleView();

    initializeStatusBar();
    emit areaChanged(area);

    hu.stop();

    loadSettings();
    connect(area, &Area::viewAdded, d, &MainWindowPrivate::viewAdded);
    connect(area, &Area::viewRemoved, d, &MainWindowPrivate::viewRemovedInternal);
    connect(area, &Area::requestToolViewRaise, d, &MainWindowPrivate::raiseToolView);
    connect(area, &Area::aboutToRemoveView, d, &MainWindowPrivate::aboutToRemoveView);
    connect(area, &Area::toolViewAdded, d, &MainWindowPrivate::toolViewAdded);
    connect(area, &Area::aboutToRemoveToolView, d, &MainWindowPrivate::aboutToRemoveToolView);
}

void MainWindow::initializeStatusBar()
{
    //nothing here, reimplement in the subclasses if you want to have status bar
    //inside the bottom tool view buttons row
}

void MainWindow::clearArea()
{
    Q_D(MainWindow);

    emit areaCleared(d->area);
    d->clearArea();
}

QList<View*> MainWindow::toolDocks() const
{
    Q_D(const MainWindow);

    return d->docks;
}

Area *Sublime::MainWindow::area() const
{
    Q_D(const MainWindow);

    return d->area;
}

Controller *MainWindow::controller() const
{
    Q_D(const MainWindow);

    return d->controller;
}

View *MainWindow::activeView() const
{
    Q_D(const MainWindow);

    return d->activeView;
}

View *MainWindow::activeToolView() const
{
    Q_D(const MainWindow);

    return d->activeToolView;
}

void MainWindow::activateView(Sublime::View* view, bool focus)
{
    Q_D(MainWindow);

    const auto containerIt = d->viewContainers.constFind(view);
    if (containerIt == d->viewContainers.constEnd())
        return;

    if (d->activeView == view)
    {
        if (focus && view && !view->widget()->hasFocus())
            view->widget()->setFocus();
        return;
    }

    (*containerIt)->setCurrentWidget(view->widget());

    setActiveView(view, focus);
    d->area->setActiveView(view);
}

void MainWindow::setActiveView(View *view, bool focus)
{
    Q_D(MainWindow);

    View* oldActiveView = d->activeView;

    d->activeView = view;

    if (focus && view && !view->widget()->hasFocus())
        view->widget()->setFocus();

    if(d->activeView != oldActiveView)
        emit activeViewChanged(view);
}

void Sublime::MainWindow::setActiveToolView(View *view)
{
    Q_D(MainWindow);

    d->activeToolView = view;
    emit activeToolViewChanged(view);
}

void MainWindow::saveSettings()
{
    Q_D(MainWindow);

    d->disableConcentrationMode();

    KConfigGroup cg(KSharedConfig::openConfig(), configGroupName(d->area));

    // When the current area changes, all shown dock widgets are visible at this point.
    //
    // On KDevelop exit, all non-floating dock widgets are always invisible at this point, because hiding
    // the main window also hides its every child widget that is not a top-level widget (not a window).
    // Such a desynchronization of visibility of non-floating dock widgets does not affect their visibility
    // when the main window state is restored, that is, shown tool views remain shown after restarting KDevelop.
    //
    // Exiting KDevelop via the Quit action or via the terminating signal handler closes all
    // top-level windows, including floating dock widgets. After the closing, all floating dock
    // widgets also become invisible by this time. As a result, none of the floating dock widgets
    // becomes visible (and consequently shown) when the main window state is restored. Detect this
    // bug-producing situation here and make the shown floating dock widgets visible while calling
    // saveMainWindowSettings(), which saves the main window state. This transient change of
    // visibility does not cause flickering, because no event loop is entered in saveMainWindowSettings().

    const auto shownButInvisibleFloatingDockWidgets = d->idealController->shownButInvisibleFloatingDockWidgets();

    qCDebug(SUBLIME) << "saving settings for" << (d->area ? d->area->objectName() : QString())
                     << (shownButInvisibleFloatingDockWidgets.empty()
                             ? ""
                             : qUtf8Printable(QStringLiteral("(temporarily making %1 floating dock widget(s) visible)")
                                                  .arg(shownButInvisibleFloatingDockWidgets.size())));

    for (auto* const dockWidget : shownButInvisibleFloatingDockWidgets) {
        dockWidget->setVisible(true);
    }
    saveMainWindowSettings(cg);
    for (auto* const dockWidget : shownButInvisibleFloatingDockWidgets) {
        dockWidget->setVisible(false);
    }

    //debugToolBar visibility is stored separately to allow a area dependent default value
    const auto toolBars = this->toolBars();
    for (KToolBar* toolbar : toolBars) {
        if (toolbar->objectName() == QLatin1String("debugToolBar")) {
            cg.writeEntry("debugToolBarVisibility", toolbar->isVisibleTo(this));
        }
    }

    d->idealController->saveButtonOrderSettings(cg);

    cg.sync();
}

void MainWindow::loadSettings()
{
    Q_D(MainWindow);

    qCDebug(SUBLIME) << "loading settings for" << (area() ? area()->objectName() : QString());

    const auto groupName = configGroupName(d->area);
    setStateConfigGroup(groupName);
    const KConfigGroup cg(KSharedConfig::openConfig(), groupName);

    applyMainWindowSettings(cg);

    //debugToolBar visibility is stored separately to allow a area dependent default value
    const auto toolBars = this->toolBars();
    for (KToolBar* toolbar : toolBars) {
        if (toolbar->objectName() == QLatin1String("debugToolBar")) {
            bool visibility = cg.readEntry("debugToolBarVisibility", area()->objectName() == QLatin1String("debug"));
            toolbar->setVisible(visibility);
        }
    }

    d->idealController->loadButtonOrderSettings(cg);

    // applyMainWindowSettings() restores the main window state, including the visibility of ideal toolbars,
    // from config. So the visibility of ideal toolbars may no longer match their emptiness. This can happen if
    // the main window state is out of sync with Sublime::Area's "desired views" config or if some plugin with a
    // tool view is loaded now but was not loaded during the previous KDevelop run (or vice versa) for some reason.
    // Update ideal toolbar visibility to avoid showing an empty toolbar or not showing a nonempty one.
    d->idealController->updateToolBarVisibility();

    if (isVisible()) {
        d->adaptToDockWidgetVisibilities();
    } else {
        d->waitingToAdaptToDockWidgetVisibilities = true;
    }

    d->disableConcentrationMode();
}

void MainWindow::showEvent(QShowEvent* event)
{
    Q_D(MainWindow);

    KParts::MainWindow::showEvent(event);

    if (d->waitingToAdaptToDockWidgetVisibilities) {
        d->adaptToDockWidgetVisibilities();
    }
}

void MainWindow::postMessage(Message* message)
{
    Q_D(MainWindow);

    d->postMessage(message);
}

void MainWindow::enableAreaSettingsSave()
{
    Q_D(MainWindow);

    d->autoAreaSettingsSave = true;
}

QWidget *MainWindow::statusBarLocation() const
{
    Q_D(const MainWindow);

    return d->idealController->statusBarLocation();
}

ViewBarContainer *MainWindow::viewBarContainer() const
{
    Q_D(const MainWindow);

    return d->viewBarContainer;
}

void MainWindow::loadUiPreferences()
{
    Q_D(const MainWindow);

    const auto tabBarHidden = !Container::configTabBarVisible();
    const auto closeButtonsOnTabs = Container::configCloseButtonsOnTabs();
    for (auto* const container : std::as_const(d->viewContainers)) {
        container->setTabBarHidden(tabBarHidden);
        container->setCloseButtonsOnTabs(closeButtonsOnTabs);
    }
}

void MainWindow::tabDoubleClicked(View* view)
{
    Q_UNUSED(view);

    Q_D(MainWindow);

    d->toggleDocksShown();
}

void MainWindow::tabContextMenuRequested(View* , QMenu* )
{
    // do nothing
}

void MainWindow::tabToolTipRequested(View*, Container*, int)
{
    // do nothing
}

void MainWindow::newTabRequested()
{
}

void MainWindow::dockBarContextMenuRequested(Qt::DockWidgetArea , const QPoint& )
{
    // do nothing
}

View* MainWindow::viewForPosition(const QPoint& globalPos) const
{
    Q_D(const MainWindow);

    for (Container* container : std::as_const(d->viewContainers)) {
        QRect globalGeom = QRect(container->mapToGlobal(QPoint(0,0)), container->mapToGlobal(QPoint(container->width(), container->height())));
       if(globalGeom.contains(globalPos))
       {
           return d->widgetToView[container->currentWidget()];
       }
    }

    return nullptr;
}

void MainWindow::setBackgroundCentralWidget(QWidget* w)
{
    Q_D(MainWindow);

    d->setBackgroundCentralWidget(w);
}

}

#include "moc_mainwindow.cpp"
