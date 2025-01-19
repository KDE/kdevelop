/*
    SPDX-FileCopyrightText: 2007 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2011 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LicenseRef-MIT-KDevelop-Ideal
*/

#include "idealcontroller.h"

#include <QApplication>
#include <QDebug>
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

#include <debug.h>

#include <algorithm>
#include <array>

namespace {
[[nodiscard]] QString dockWidgetAreaName(const Sublime::IdealDockWidget& dockWidget)
{
    const auto dockWidgetName = dockWidget.objectName();
    const auto lastUnderscorePosition = dockWidgetName.lastIndexOf(QLatin1Char{'_'});
    return lastUnderscorePosition == -1 ? QString() : dockWidgetName.sliced(lastUnderscorePosition + 1);
}

struct PrintDockWidget
{
    explicit PrintDockWidget(const Sublime::IdealDockWidget* dockWidget)
        : PrintDockWidget(dockWidget, dockWidget->view())
    {
    }

    /// Use this overload if dockWidget->view() has not been set up yet.
    explicit PrintDockWidget(const Sublime::IdealDockWidget* dockWidget, const Sublime::View* view)
        : dockWidget(dockWidget)
        , view(view)
    {
        Q_ASSERT(dockWidget);
        Q_ASSERT(view);
    }

    const Sublime::IdealDockWidget* const dockWidget;
    const Sublime::View* const view;
};

QDebug operator<<(QDebug debug, PrintDockWidget p)
{
    const QDebugStateSaver saver(debug);
    debug.noquote().nospace() << dockWidgetAreaName(*p.dockWidget) << ':' << p.view->document()->documentSpecifier()
                              << (p.dockWidget->isVisible() ? " (visible)" : "");
    return debug;
}

[[nodiscard]] Sublime::IdealDockWidget* existentDockWidgetForView(Sublime::View* view)
{
    Q_ASSERT(view);
    Q_ASSERT(view->hasWidget());

    auto* const viewParent = view->widget()->parentWidget();
    auto* dockWidget = qobject_cast<Sublime::IdealDockWidget*>(viewParent);
    if (!dockWidget) {
        // a tool view widget with a toolbar lives in a QMainWindow that lives in a dock widget
        Q_ASSERT(qobject_cast<QMainWindow*>(viewParent));
        dockWidget = qobject_cast<Sublime::IdealDockWidget*>(viewParent->parentWidget());
    }

    Q_ASSERT(dockWidget);
    return dockWidget;
}

} // unnamed namespace

namespace Sublime {

class IdealToolBar : public QToolBar
{
    Q_OBJECT
public:
    explicit IdealToolBar(const QString& title, bool hideWhenEmpty, IdealButtonBarWidget* buttons, QMainWindow* parent)
        : QToolBar(title, parent)
        , m_buttons(hideWhenEmpty ? buttons : nullptr)
    {
        setMovable(false);
        setFloatable(false);
        setObjectName(title);
        layout()->setContentsMargins(0, 0, 0, 0);

        addWidget(buttons);

        // This code determines the initial visibility of the toolbar only if KMainWindow::applyMainWindowSettings()
        // fails to restore the main window state from config. So if the user manually hides or shows a
        // toolbar via the context menu of the main menu, it remains hidden/visible on next KDevelop start.
        if (hideWhenEmpty) {
            updateVisibility();
            connect(m_buttons, &IdealButtonBarWidget::emptyChanged, this, &IdealToolBar::updateVisibility);
        }
    }

    void updateVisibility()
    {
        // if (!m_buttons) do not hide if empty, therefore always show
        setVisible(!m_buttons || !m_buttons->isEmpty());
    }

private:
    IdealButtonBarWidget* const m_buttons;
};

IdealController::IdealController(Sublime::MainWindow* mainWindow)
    : QObject(mainWindow)
    , m_mainWindow(mainWindow)
    , m_leftBarWidget{new IdealButtonBarWidget(Qt::LeftDockWidgetArea, mainWindow)}
    , m_rightBarWidget{new IdealButtonBarWidget(Qt::RightDockWidgetArea, mainWindow)}
    , m_topBarWidget{new IdealButtonBarWidget(Qt::TopDockWidgetArea, mainWindow)}
    , m_bottomBarWidget{new IdealButtonBarWidget(Qt::BottomDockWidgetArea, mainWindow)}
    , m_leftToolBar{new IdealToolBar(i18n("Left Button Bar"), true, m_leftBarWidget, mainWindow)}
    , m_rightToolBar{new IdealToolBar(i18n("Right Button Bar"), true, m_rightBarWidget, mainWindow)}
    // adymo: intentionally do not add a toolbar for top buttonbar
    // this doesn't work well with toolbars added via xmlgui
    // -------------
    // never hide the bottom toolbar, because it contains the status bar
    , m_bottomToolBar{new IdealToolBar(i18n("Bottom Button Bar"), false, m_bottomBarWidget, mainWindow)}
    , m_bottomStatusBarLocation{m_bottomBarWidget->corner()}
{
    forEachButtonBarWidget([this](IdealButtonBarWidget& buttonBarWidget) {
        connect(&buttonBarWidget, &IdealButtonBarWidget::customContextMenuRequested, this,
                &IdealController::slotDockBarContextMenuRequested);
        connect(&buttonBarWidget, &IdealButtonBarWidget::showDockWidget, this, &IdealController::showDockWidget);
    });

    m_mainWindow->addToolBar(Qt::LeftToolBarArea, m_leftToolBar);
    m_mainWindow->addToolBar(Qt::RightToolBarArea, m_rightToolBar);
    m_mainWindow->addToolBar(Qt::BottomToolBarArea, m_bottomToolBar);

    m_docks = qobject_cast<KActionMenu*>(mainWindow->action(QStringLiteral("docks_submenu")));

    m_showLeftDock = m_mainWindow->action(QStringLiteral("show_left_dock"));
    m_showRightDock = m_mainWindow->action(QStringLiteral("show_right_dock"));
    m_showBottomDock = m_mainWindow->action(QStringLiteral("show_bottom_dock"));

    // the 'show top dock' action got removed (IOW, it's never created)
    // (let's keep this code around if we ever want to reintroduce the feature...
    m_showTopDock = m_mainWindow->action(QStringLiteral("show_top_dock"));

    for (auto* const action : {m_showLeftDock, m_showRightDock, m_showBottomDock}) {
        action->setCheckable(true);
    }

    const auto connectToShowDockAction = [this](QAction* action, const IdealButtonBarWidget* buttonBarWidget) {
        Q_ASSERT(action);
        Q_ASSERT(buttonBarWidget);
        connect(action, &QAction::toggled, this, [buttonBarWidget, this](bool checked) {
            showDockActionToggled(sender(), *buttonBarWidget, checked);
        });
    };
    connectToShowDockAction(m_showLeftDock, m_leftBarWidget);
    connectToShowDockAction(m_showRightDock, m_rightBarWidget);
    connectToShowDockAction(m_showBottomDock, m_bottomBarWidget);
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

    // If w->parent() is null, it means that the view->widget(dock) call above returned
    // an existing widget instead of creating a new one with dock as its parent. Either
    // dock or a new QMainWindow toolView will become the widget's parent in the code below.

    // Use the two-argument overload of PrintDockWidget(), because dock->view()
    // will become equal to view only during the addBarWidgetAction() call below.
    qCDebug(SUBLIME) << "creating dock widget" << PrintDockWidget(dock, view) << "in" << area
                     << (w->parent() ? "" : "(reparenting)");

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

    addBarWidgetAction(area, dock, view);

    connect(dock, &IdealDockWidget::dockLocationChanged, this, &IdealController::dockLocationChanged);
    connect(dock, &QDockWidget::topLevelChanged, this, &IdealController::dockTopLevelChanged);

    dock->hide();
}

void IdealController::addBarWidgetAction(Qt::DockWidgetArea area, IdealDockWidget* dock, View* view, bool checked)
{
    auto* const bar = barForDockArea(area);

    auto* const action = bar->addWidget(dock, m_mainWindow->area(), view, checked);
    m_view_to_action[view] = action;

    m_docks->addAction(action);
    connect(dock, &IdealDockWidget::closeRequested, action, &QAction::toggle);
}

void IdealController::dockLocationChanged(Qt::DockWidgetArea area)
{
    auto* const dock = qobject_cast<IdealDockWidget*>(sender());
    Q_ASSERT(dock);

    // Seems since Qt 5.13 the signal QDockWidget::dockLocationChanged is emitted also when the dock changes
    // to be floating, with area = Qt::NoDockWidgetArea.
    if (area == Qt::NoDockWidgetArea) {
        qCDebug(SUBLIME) << "dock widget" << PrintDockWidget{dock} << "location changed to no area";
        return; // this case is handled in dockTopLevelChanged(true), so nothing to do here
    }

    const auto previousArea = dock->dockWidgetArea();

    if (previousArea == area) {
        // this event can happen even when dock changes its location within the same area
        // usecases:
        // 1) user drags to the same area
        // 2) user rearranges tool views inside the same area
        // 3) state restoration shows the dock widget
        qCDebug(SUBLIME) << "dock widget" << PrintDockWidget{dock} << "remains in" << area;
        return;
    }

    qCDebug(SUBLIME) << "dock widget" << PrintDockWidget{dock} << "location changed from" << previousArea << "to"
                     << area;

    auto* const view = dock->view();
    auto* const action = m_view_to_action.value(view);
    const auto isVisible = dock->isVisible();

    barForDockArea(previousArea)->removeAction(action);
    setShowDockStatus(previousArea, false);

    addBarWidgetAction(area, dock, view, isVisible);

    m_mainWindow->area()->setToolViewPosition(view, dockAreaToPosition(area));

    if (!isVisible) {
        return;
    }

    setShowDockStatus(area, true);

    // When the user moves a tool view via the Tool View Position submenu of its context menu, the dock widget
    // never becomes floating between the two dock widget areas. In this case, dockTopLevelChanged()
    // is not invoked and the moved tool view does not receive the focus. So focus it here.
    dock->setFocus(Qt::ShortcutFocusReason);
}

void IdealController::dockTopLevelChanged(bool topLevel)
{
    auto* const dockWidget = qobject_cast<IdealDockWidget*>(sender());
    Q_ASSERT(dockWidget);

    if (topLevel) {
        // Nothing to do, because the newly-floating dock widget's IdealDockWidget::dockWidgetArea() should keep
        // returning its previous area, and the corresponding tool view action and button should stay in the
        // same toolbar. No need to focus the newly-floating dock widget either, because it is a top-level window
        // that becomes active, and consequently the dock widget gets the focus, automatically (unless invisible).
        qCDebug(SUBLIME) << "dock widget" << PrintDockWidget{dockWidget} << "became floating";
        return;
    }

    qCDebug(SUBLIME) << "dock widget" << PrintDockWidget{dockWidget} << "is no longer floating";

    if (dockWidget->isVisible()) {
        // In this case the dock widget should acquire the focus but does not automatically get it.
        // dockLocationChanged() does not always give it the focus either. So focus it here.
        dockWidget->setFocus(Qt::ShortcutFocusReason);
    }
}

IdealButtonBarWidget* IdealController::barForDockArea(Qt::DockWidgetArea area) const
{
    switch (area) {
        case Qt::LeftDockWidgetArea:
            return m_leftBarWidget;
        case Qt::TopDockWidgetArea:
            return m_topBarWidget;
        case Qt::RightDockWidgetArea:
            return m_rightBarWidget;
        case Qt::BottomDockWidgetArea:
            return m_bottomBarWidget;
        default:
            Q_ASSERT(false);
            return nullptr;
    }
}

template<typename ButtonBarWidgetUser>
void IdealController::forEachButtonBarWidget(ButtonBarWidgetUser callback) const
{
    for (auto* const barWidget : {m_leftBarWidget, m_rightBarWidget, m_bottomBarWidget}) {
        callback(*barWidget);
    }
}

template<typename ToolBarUser>
void IdealController::forEachToolBar(ToolBarUser callback) const
{
    for (auto* const toolBar : {m_leftToolBar, m_rightToolBar, m_bottomToolBar}) {
        callback(*toolBar);
    }
}

void IdealController::slotDockBarContextMenuRequested(const QPoint& position)
{
    auto* bar = qobject_cast<IdealButtonBarWidget*>(sender());
    Q_ASSERT(bar);

    emit dockBarContextMenuRequested(bar->area(), bar->mapToGlobal(position));
}

void IdealController::raiseView(View* view)
{
    QAction* action = m_view_to_action.value(view);
    Q_ASSERT(action);

    QWidget *focusWidget = m_mainWindow->focusWidget();
    action->setChecked(true);
    // TODO: adymo: hack: focus needs to stay inside the previously
    // focused widget (setChecked will focus the tool view)
    if (focusWidget)
        focusWidget->setFocus(Qt::ShortcutFocusReason);
}

void IdealController::showDockWidget(IdealDockWidget* dock, bool show)
{
    Qt::DockWidgetArea area = dock->dockWidgetArea();
    qCDebug(SUBLIME) << (show ? "showing" : "hiding") << "dock widget" << PrintDockWidget{dock} << "in" << area;

    if (show) {
        m_mainWindow->addDockWidget(area, dock);
        dock->show();

        dock->setFocus(Qt::ShortcutFocusReason);
    } else {
        // Calling dock->hide() is necessary to make QMainWindow::saveState() store its removed state correctly.
        // Without this call, a previously hidden tool view can become shown on next KDevelop start (see QTBUG-11909).
        dock->hide();
        m_mainWindow->removeDockWidget(dock);

        // Put the focus back on the editor if a dock was hidden
        focusEditor();
    }

    setShowDockStatus(area, show);
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
    if (!checked && barForDockArea(area)->lastCheckedActionsTracker().isAnyChecked()) {
        return; // another checked action remains => nothing to do
    }
    forceSetShowDockStatus(area, checked);
}

void IdealController::forceSetShowDockStatus(Qt::DockWidgetArea area, bool checked)
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
            return m_showLeftDock;
        case Qt::RightDockWidgetArea:
            return m_showRightDock;
        case Qt::TopDockWidgetArea:
            return m_showTopDock;
        case Qt::BottomDockWidgetArea:
            return m_showBottomDock;
        default:
            Q_ASSERT(false);
            return nullptr;
    }
}

void IdealController::removeView(View* view, bool nondestructive)
{
    const auto viewIt = m_view_to_action.constFind(view);
    Q_ASSERT(viewIt != m_view_to_action.cend());

    auto* const action = *viewIt;
    Q_ASSERT(action);
    auto* const dock = existentDockWidgetForView(view);

    /* Hide the view, first.  This is a workaround -- if we
       try to remove IdealDockWidget without this, then eventually
       a call to IdealMainLayout::takeAt will be made, which
       method asserts immediately.  */
    action->setChecked(false);

    qCDebug(SUBLIME) << "destroying dock widget" << PrintDockWidget{dock} << "in" << dock->dockWidgetArea()
                     << (nondestructive ? "(nondestructive)" : "");

    barForDockArea(dock->dockWidgetArea())->removeAction(action);
    m_view_to_action.erase(viewIt);

    if (nondestructive)
        view->widget()->setParent(nullptr);

    delete dock;
}

template<typename ToolViewUser>
void IdealController::forEachShownToolView(ToolViewUser callback) const
{
    for (const auto [view, action] : m_view_to_action.asKeyValueRange()) {
        if (action->isChecked()) {
            callback(view);
        }
    }
}

QList<View*> IdealController::shownViews() const
{
    QList<View*> shown;
    forEachShownToolView([&shown](View* view) {
        shown.push_back(view);
    });
    return shown;
}

QList<QDockWidget*> IdealController::shownButInvisibleFloatingDockWidgets() const
{
    QList<QDockWidget*> ret;
    forEachShownToolView([&ret](View* view) {
        auto* const dockWidget = existentDockWidgetForView(view);
        if (dockWidget->isFloating() && !dockWidget->isVisible()) {
            ret.push_back(dockWidget);
        }
    });
    return ret;
}

void IdealController::adaptToDockWidgetVisibilities()
{
    forEachButtonBarWidget([this](IdealButtonBarWidget& buttonBarWidget) {
        buttonBarWidget.adaptToDockWidgetVisibilities();
        forceSetShowDockStatus(buttonBarWidget.area(), buttonBarWidget.lastCheckedActionsTracker().isAnyChecked());
    });
}

void IdealController::loadButtonOrderSettings(const KConfigGroup& configGroup)
{
    forEachButtonBarWidget([&configGroup](IdealButtonBarWidget& buttonBarWidget) {
        buttonBarWidget.loadOrderSettings(configGroup);
    });
}

void IdealController::saveButtonOrderSettings(KConfigGroup& configGroup)
{
    forEachButtonBarWidget([&configGroup](IdealButtonBarWidget& buttonBarWidget) {
        buttonBarWidget.saveOrderSettings(configGroup);
    });
}

void IdealController::hideToolBars()
{
    forEachToolBar([](IdealToolBar& toolBar) {
        toolBar.hide();
    });
}

void IdealController::updateToolBarVisibility()
{
    forEachToolBar([](IdealToolBar& toolBar) {
        toolBar.updateVisibility();
    });
}

void IdealController::showDockActionToggled(QObject* senderAction, const IdealButtonBarWidget& buttonBarWidget,
                                            bool show)
{
    auto& actionTracker = buttonBarWidget.lastCheckedActionsTracker();

    const auto toggleShowDockActionQuietly = [senderAction, show] {
        auto* const action = qobject_cast<QAction*>(senderAction);
        Q_ASSERT(action);
        Q_ASSERT(action->isChecked() == show);
        const QSignalBlocker blocker(action);
        action->toggle();
    };

    if (!show) {
        if (actionTracker.activateLastShownDockWidget(currentDockWidget())) {
            // we just focused a dock widget instead of hiding it => check the Show Dock action
            toggleShowDockActionQuietly();
        } else {
            actionTracker.uncheckAll();
        }
        return;
    }

    // Open last shown tool view(s) (or the first one).
    // The last of the opened tool views ends up focused.
    if (!actionTracker.checkAllTracked()) {
        const auto barActions = buttonBarWidget.actions();
        if (barActions.empty()) {
            // no tool view to show => uncheck the Show Dock action
            toggleShowDockActionQuietly();
        } else {
            barActions.first()->setChecked(true);
        }
    }
}

IdealDockWidget* IdealController::currentDockWidget() const
{
    if (auto* const activeDockWidget = qobject_cast<IdealDockWidget*>(QApplication::activeWindow())) {
        // a floating dock widget is the active window, so return it as current
        return activeDockWidget;
    }

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

    auto index = bar->actions().indexOf(m_view_to_action.value(currentDock->view()));
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
    auto anyShown = false;
    // use "variable-length" array to prevent allocations
    std::array<ILastCheckedActionsTracker*, 3> trackers;
    auto end = trackers.begin();
    forEachButtonBarWidget([&anyShown, &end](const IdealButtonBarWidget& buttonBarWidget) {
        if (buttonBarWidget.isLocked()) {
            return;
        }
        auto& tracker = buttonBarWidget.lastCheckedActionsTracker();
        anyShown = anyShown || tracker.isAnyChecked();
        *end++ = &tracker;
    });

    // The range [trackers.begin(), end) contains unlocked trackers. Toggle their shared checked state.
    // Saving and retrieving the last saved any-checked state is necessary to restore only the docks that were hidden by
    // the previous call to this function - assuming no tool views were shown in unlocked docks between the two calls.
    std::for_each(trackers.begin(), end, [anyShown](ILastCheckedActionsTracker* tracker) {
        if (anyShown) {
            tracker->saveAnyCheckedState();
            tracker->uncheckAll();
        } else if (tracker->lastSavedAnyCheckedState()) {
            tracker->checkAllTracked();
        }
    });
}

} // namespace Sublime

#include "idealcontroller.moc"
#include "moc_idealcontroller.cpp"
