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
#include <QScopedValueRollback>
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
#include <utility>

namespace {
[[nodiscard]] QString dockWidgetAreaName(const Sublime::IdealDockWidget& dockWidget)
{
    const auto dockWidgetName = dockWidget.objectName();
    const auto lastUnderscorePosition = dockWidgetName.lastIndexOf(QLatin1Char{'_'});
    return lastUnderscorePosition == -1 ? QString() : dockWidgetName.sliced(lastUnderscorePosition + 1);
}

struct PrintDockWidget
{
    const Sublime::IdealDockWidget* const dockWidget;
};

QDebug operator<<(QDebug debug, PrintDockWidget p)
{
    const QDebugStateSaver saver(debug);
    Q_ASSERT(p.dockWidget);
    debug.noquote().nospace() << dockWidgetAreaName(*p.dockWidget) << ':'
                              << p.dockWidget->view()->document()->documentSpecifier()
                              << (p.dockWidget->isVisible() ? " (visible)" : "");
    return debug;
}

/**
 * If a given tool view has toolbar actions, create a new QMainWindow with a
 * toolbar that displays the actions and with @p viewWidget as the central widget.
 *
 * @param mainWindow the main window ancestor of @p viewWidget
 * @param view the tool view, for which a dock widget is being set up
 * @param viewWidget just-created non-null @p view->widget()
 * @param toolViewTitle the title of the tool view
 *
 * @return a widget that should become the dock widget's widget (@p viewWidget itself or its new QMainWindow parent)
 */
[[nodiscard]] QWidget* setUpWidgetForDockWidget(const Sublime::MainWindow& mainWindow, const Sublime::View& view,
                                                QWidget* viewWidget, const QString& toolViewTitle)
{
    Q_ASSERT(viewWidget);
    Q_ASSERT(view.widget() == viewWidget);

    const auto toolBarActions = view.toolBarActions();
    if (toolBarActions.empty()) {
        return viewWidget;
    }

    auto* const toolView = new QMainWindow();
    auto* const toolBar = new QToolBar(toolView);

    const auto iconSize = mainWindow.style()->pixelMetric(QStyle::PM_SmallIconSize);
    toolBar->setIconSize(QSize(iconSize, iconSize));
    toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolBar->setWindowTitle(i18nc("@title:window", "%1 Toolbar", toolViewTitle));
    toolBar->setFloatable(false);
    toolBar->setMovable(false);
    toolBar->addActions(toolBarActions);

    toolView->setCentralWidget(viewWidget);
    toolView->setFocusProxy(viewWidget);
    toolView->addToolBar(toolBar);

    static const auto configGroupName = QStringLiteral("UiSettings/Docks/ToolbarEnabled");
    const auto configEntryKey = view.document()->documentSpecifier();

    const KConfigGroup cg(KSharedConfig::openConfig(), configGroupName);
    toolBar->setVisible(cg.readEntry(configEntryKey, true));

    const auto* const toggleViewAction = toolBar->toggleViewAction();
    QObject::connect(toggleViewAction, &QAction::toggled, toggleViewAction, [toggleViewAction, configEntryKey] {
        KConfigGroup cg(KSharedConfig::openConfig(), configGroupName);
        cg.writeEntry(configEntryKey, toggleViewAction->isChecked());
    });

    return toolView;
}

/**
 * Return a widget that used to be and can again become a dock widget's widget
 * (@p viewWidget itself or its QMainWindow parent).
 *
 * @param mainWindow the main window ancestor of @p viewWidget
 * @param viewWidget non-null disowned tool view widget
 */
[[nodiscard]] QWidget* existentWidgetForDockWidget(const Sublime::MainWindow& mainWindow, QWidget* viewWidget)
{
    Q_ASSERT(viewWidget);

    // The previous dock widget (grand)parent of the disowned view widget must have been destroyed by now, and
    // the view widget itself must have been saved from destruction by setting its (grand)parent to the main window.
    // If not, the caller of this function would be stealing the view widget from its current (grand)parent dock widget.

    auto* const viewParent = viewWidget->parentWidget();
    Q_ASSERT(viewParent);
    if (viewParent == &mainWindow) {
        return viewWidget;
    }

    // a tool view widget with a toolbar lives in a QMainWindow that lives in a dock widget
    Q_ASSERT(qobject_cast<QMainWindow*>(viewParent));
    Q_ASSERT(viewParent->parentWidget() == &mainWindow);
    return viewParent;
}

struct DockWidgetInfo
{
    Sublime::IdealDockWidget* dockWidget;
    QMainWindow* toolView;
    QWidget* viewWidget;
};

/**
 * @return the dock widget (grand)parent of @p view->widget(), the QMainWindow parent
 *         of @p view->widget() if it has a toolbar, and @p view->widget() itself
 *
 * @pre @p view->widget() belongs to a dock widget
 */
[[nodiscard]] DockWidgetInfo existentDockWidgetForView(const Sublime::View* view)
{
    Q_ASSERT(view);
    auto* const viewWidget = view->widget();
    Q_ASSERT(viewWidget);

    auto* const viewParent = viewWidget->parentWidget();
    Q_ASSERT(viewParent);
    auto* dockWidget = qobject_cast<Sublime::IdealDockWidget*>(viewParent);
    if (dockWidget) {
        return {dockWidget, nullptr, viewWidget};
    }

    // a tool view widget with a toolbar lives in a QMainWindow that lives in a dock widget
    Q_ASSERT(qobject_cast<QMainWindow*>(viewParent) == viewParent);
    auto* const toolView = static_cast<QMainWindow*>(viewParent);
    dockWidget = qobject_cast<Sublime::IdealDockWidget*>(toolView->parentWidget());
    Q_ASSERT(dockWidget);
    return {dockWidget, toolView, viewWidget};
}

} // unnamed namespace

namespace Sublime {

/**
 * A cache of tool view widgets reused for all sublime areas.
 *
 * This class keeps track of tool view widget use counts in order to reuse a single tool view widget
 * for all sublime areas rather than create a new one for each area. This reuse is possible
 * because the previous area disowns all tool view widgets before the next area adopts them.
 *
 * @todo (if/when IdealController starts supporting multiple view widgets per tool document):
 *       remove the enableMultipleToolViewWidgets workarounds from TestAreaOperation and TestViewActivation.
 */
class ToolViewWidgetCache
{
public:
    /**
     * Make sure that the widget for a given view is initialized.
     *
     * If the widget for @p view is uninitialized, either set it to the cached widget for @p view.document() and add one
     * more cache use or, if the cache has no such widget, initialize with a given parent and insert it into the cache.
     *
     * @return {non-null @p view.widget(), \<whether the widget was just created\>}
     */
    std::pair<QWidget*, bool> prepareViewWidget(View& view, QWidget* parent);

    /**
     * Remove one cache use of the widget for a given view.
     *
     * @pre @p view.widget() is not null and is present in the cache
     *
     * @return whether the widget is now unused and should be destroyed by the caller
     */
    [[nodiscard]] bool disuse(const View& view);

    /**
     * Remove the cache entry for a given document if present.
     *
     * @param document a non-null tool document
     *
     * @return the widget of the removed cache entry that should be destroyed
     *         by the caller or @c nullptr if no such cache entry
     */
    [[nodiscard]] QWidget* remove(const Document* document);

private:
    class ToolViewWidget
    {
    public:
        [[nodiscard]] QWidget* widget() const
        {
            assertConsistent();
            return m_widget;
        }
        [[nodiscard]] int useCount() const
        {
            assertConsistent();
            return m_useCount;
        }

        void initializeWidget(QWidget* widget)
        {
            Q_ASSERT(m_useCount == 0);
            Q_ASSERT(!m_widget);
            Q_ASSERT(widget);
            m_widget = widget;
            m_useCount = 1;
        }

        void addUse()
        {
            assertHasWidget();
            ++m_useCount;
        }
        void removeUse()
        {
            --m_useCount;
            assertHasWidget();
        }

    private:
        void assertConsistent() const
        {
            Q_ASSERT(m_useCount >= 0);
            Q_ASSERT((m_widget == nullptr) == (m_useCount == 0));
        }
        void assertHasWidget() const
        {
            Q_ASSERT(m_useCount > 0);
            Q_ASSERT(m_widget);
        }

        QWidget* m_widget = nullptr;
        int m_useCount = 0;
    };

    QHash<const Document*, ToolViewWidget> m_widgets;
};

std::pair<QWidget*, bool> ToolViewWidgetCache::prepareViewWidget(View& view, QWidget* parent)
{
    const auto* const document = view.document();
    Q_ASSERT(document);

    if (auto* const widget = view.widget()) {
        Q_ASSERT(m_widgets.value(document).widget() == widget);
        qCDebug(SUBLIME) << "the view already has a tool view widget" << document->documentSpecifier();
        return {widget, false};
    }

    auto& cachedWidget = m_widgets[document];

    if (auto* const widget = cachedWidget.widget()) {
        cachedWidget.addUse();
        view.setSharedWidget(widget);
        qCDebug(SUBLIME).nospace() << "reusing an existing tool view widget " << document->documentSpecifier()
                                   << ", use count: " << cachedWidget.useCount();
        return {widget, false};
    }

    auto* const widget = view.initializeWidget(parent);
    Q_ASSERT(widget);
    cachedWidget.initializeWidget(widget);
    qCDebug(SUBLIME) << "created a new tool view widget" << document->documentSpecifier();
    return {widget, true};
}

bool ToolViewWidgetCache::disuse(const View& view)
{
    Q_ASSERT(view.widget());
    const auto* const document = view.document();
    Q_ASSERT(document);

    const auto it = m_widgets.find(document);
    Q_ASSERT(it != m_widgets.end());
    Q_ASSERT(it->widget() == view.widget());

    if (it->useCount() == 1) {
        m_widgets.erase(it);
        qCDebug(SUBLIME) << "destroying a no longer used tool view widget" << document->documentSpecifier();
        return true;
    }

    it->removeUse();
    qCDebug(SUBLIME).nospace() << "keeping a still used tool view widget " << document->documentSpecifier()
                               << ", use count: " << it->useCount();
    return false;
}

QWidget* ToolViewWidgetCache::remove(const Document* document)
{
    Q_ASSERT(document);

    const auto it = m_widgets.constFind(document);
    if (it == m_widgets.cend()) {
        qCDebug(SUBLIME) << "no tool view widget to destroy for the removed tool view" << document->documentSpecifier();
        return nullptr;
    }
    qCDebug(SUBLIME).nospace() << "destroying the tool view widget for the removed tool view "
                               << document->documentSpecifier() << ", use count: " << it->useCount();

    auto* const widget = it->widget();
    Q_ASSERT(widget);
    m_widgets.erase(it);
    return widget;
}

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
        // fails to restore the main window state from config.
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
    , m_leftBarWidget{new IdealButtonBarWidget(Qt::LeftDockWidgetArea, m_dockWidgetToGroupWith, mainWindow)}
    , m_rightBarWidget{new IdealButtonBarWidget(Qt::RightDockWidgetArea, m_dockWidgetToGroupWith, mainWindow)}
    , m_topBarWidget{new IdealButtonBarWidget(Qt::TopDockWidgetArea, m_dockWidgetToGroupWith, mainWindow)}
    , m_bottomBarWidget{new IdealButtonBarWidget(Qt::BottomDockWidgetArea, m_dockWidgetToGroupWith, mainWindow)}
    , m_leftToolBar{new IdealToolBar(i18n("Left Button Bar"), true, m_leftBarWidget, mainWindow)}
    , m_rightToolBar{new IdealToolBar(i18n("Right Button Bar"), true, m_rightBarWidget, mainWindow)}
    // adymo: intentionally do not add a toolbar for top buttonbar
    // this doesn't work well with toolbars added via xmlgui
    // -------------
    // never hide the bottom toolbar, because it contains the status bar
    , m_bottomToolBar{new IdealToolBar(i18n("Bottom Button Bar"), false, m_bottomBarWidget, mainWindow)}
    , m_bottomStatusBarLocation{m_bottomBarWidget->corner()}
    , m_toolViewWidgetCache(new ToolViewWidgetCache)
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
    dock->setView(view);

    const auto documentTitle = view->document()->title();
    dock->setWindowTitle(documentTitle);

    // QMainWindow::saveState() and QMainWindow::restoreState() use the objectName property to
    // identify a QDockWidget. Make the name of the dock widget's sublime area part of the object name
    // in order to support differing locations and visibilities of dock widgets in different areas.
    auto* const sublimeArea = m_mainWindow->area();
    Q_ASSERT(sublimeArea);
    dock->setArea(sublimeArea);
    dock->setObjectName(documentTitle + QLatin1Char('_') + sublimeArea->objectName());

    qCDebug(SUBLIME) << "creating dock widget" << PrintDockWidget{dock} << "in" << area;

    KAcceleratorManager::setNoAccel(dock);

    const auto [viewWidget, justCreatedWidget] = m_toolViewWidgetCache->prepareViewWidget(*view, dock);
    auto* const widgetForDockWidget = justCreatedWidget
        ? setUpWidgetForDockWidget(*m_mainWindow, *view, viewWidget, documentTitle)
        : existentWidgetForDockWidget(*m_mainWindow, viewWidget);

    dock->setWidget(widgetForDockWidget);
    dock->setWindowIcon(viewWidget->windowIcon());
    dock->setFocusProxy(dock->widget());

    addBarWidgetAction(area, dock, view);

    connect(dock, &IdealDockWidget::dockLocationChanged, this, &IdealController::dockLocationChanged);
    connect(dock, &QDockWidget::topLevelChanged, this, &IdealController::dockTopLevelChanged);

    dock->hide();
}

void IdealController::addBarWidgetAction(Qt::DockWidgetArea area, IdealDockWidget* dock, View* view, bool checked)
{
    auto* const bar = barForDockArea(area);

    auto* const action = bar->addWidget(dock, checked);
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

    // This function is invoked programmatically and shows the tool view in the background.
    // If another tool view is currently active, a background event should not hide it,
    // because that would contradict the notion of the "background". Furthermore, after such
    // a hiding, the attempt to reactivate the previously active widget in the code below would
    // either activate an invisible tool view or fail, neither of which is correct behavior.
    // Set m_dockWidgetToGroupWith to the currently active dock widget (if any) to make
    // the IdealButtonBarWidget of the tool view action checked below group with the active
    // dock widget if it happens to be in the same dock widget area as the raised tool view.
    Q_ASSERT_X(!m_dockWidgetToGroupWith, Q_FUNC_INFO, "Recursive raising and grouping is unsupported");
    const QScopedValueRollback<const IdealDockWidget*> guard(m_dockWidgetToGroupWith, currentDockWidget());

    auto* const activeWindow = QApplication::activeWindow();
    QWidget *focusWidget = m_mainWindow->focusWidget();

    action->setChecked(true);

    // Reactivate and raise the previously active window, and refocus the previously focused widget, because
    // checking the action activates its tool view but the action's tool view must be shown in the background. The
    // raising is needed if both the previously active window and the action's tool view are floating dock widgets.
    // If the temporary window activations or the focus transfers cause bugs, they can be partially
    // prevented by introducing a new ToggleOnlyBool data member to inhibit activating the main window,
    // focusing the shown dock widget and the editor view in showDockWidget(). But such an inhibition
    // cannot prevent the automatic activation of a floating dock widget that becomes visible.
    if (activeWindow) {
        activeWindow->activateWindow();
        activeWindow->raise();
    }
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

        // A floating dock widget is a top-level window. When a floating dock widget becomes visible,
        // its window is activated, and consequently the dock widget gets the focus, automatically.
        // Do not activate or focus a floating dock widget redundantly here, because that breaks
        // activating and raising a previously active floating dock widget in raiseView().
        if (!dock->isFloating()) {
            // Activate the newly shown non-floating dock widget's window (i.e. the main window) in order
            // to transfer the focus to it in case some other floating dock widget is currently active.
            dock->activateWindow();
            dock->setFocus(Qt::ShortcutFocusReason);
        }
    } else {
        // Calling dock->hide() is necessary to make QMainWindow::saveState() store its removed state correctly.
        // Without this call, a previously hidden tool view can become shown on next KDevelop start (see QTBUG-11909).
        dock->hide();
        m_mainWindow->removeDockWidget(dock);

        // Activate the main window that contains the editor view in case some other floating
        // dock widget is the active window, and focus the editor view after hiding a tool view.
        m_mainWindow->activateWindow();
        m_mainWindow->focusEditor();
    }

    setShowDockStatus(area, show);
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
    const auto [dock, toolView, viewWidget] = existentDockWidgetForView(view);

    /* Hide the view, first.  This is a workaround -- if we
       try to remove IdealDockWidget without this, then eventually
       a call to IdealMainLayout::takeAt will be made, which
       method asserts immediately.  */
    action->setChecked(false);

    qCDebug(SUBLIME) << "destroying dock widget" << PrintDockWidget{dock} << "in" << dock->dockWidgetArea()
                     << (nondestructive ? "(nondestructive)" : "");

    barForDockArea(dock->dockWidgetArea())->removeAction(action);
    m_view_to_action.erase(viewIt);

    if (nondestructive || !m_toolViewWidgetCache->disuse(*view)) {
        auto* const widgetToSave = toolView ? static_cast<QWidget*>(toolView) : viewWidget;
        // Prevent destroying viewWidget and toolView along with their (grand)parent dock widget, and
        // make sure that they are destroyed on KDevelop exit along with their new (grand)parent m_mainWindow.
        widgetToSave->setParent(m_mainWindow);
    }

    delete dock;
}

void IdealController::toolViewRemoved(const Document* document)
{
    // Removing a tool view from all areas (e.g. when a plugin is unloaded possibly on KDevelop exit)
    // normally destroys the tool view widget, but not if it is marked as used by a non-current sublime area
    // because its cache use count stays positive in this case. The tool document is about to be destroyed, so
    // remove the no longer usable cache entry and destroy the tool view widget in order to avoid leaking them.
    // NOTE: the QMainWindow parent of a tool view widget with a toolbar is always created,
    //       reused, and destroyed along with the tool view widget itself, including here.

    auto* const viewWidget = m_toolViewWidgetCache->remove(document);
    if (viewWidget) {
        delete existentWidgetForDockWidget(*m_mainWindow, viewWidget);
    }
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
        auto* const dockWidget = existentDockWidgetForView(view).dockWidget;
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
