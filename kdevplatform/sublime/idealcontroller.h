/*
    SPDX-FileCopyrightText: 2007 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2011 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LicenseRef-MIT-KDevelop-Ideal
*/

#ifndef SUBLIME_IDEALCONTROLLER_H
#define SUBLIME_IDEALCONTROLLER_H

#include "sublimedefs.h"

#include <QAction>
#include <QHash>
#include <QList>

class KActionMenu;
class KConfigGroup;

class QDockWidget;

namespace Sublime {

class Area;
class View;
class MainWindow;
class IdealButtonBarWidget;
class IdealDockWidget;
class IdealToolBar;

class IdealController: public QObject
{
    Q_OBJECT
public:
    explicit IdealController(Sublime::MainWindow *mainWindow);

    void addView(Qt::DockWidgetArea area, View* view);

    void raiseView(View* view);
    QWidget *statusBarLocation() const;
    QAction* actionForView(View* view) const;

    /** Remove view.  If nondestructive true, view->widget()
        is not deleted, as is left with NULL parent.
        Otherwise, it's deleted.  */
    void removeView(View* view, bool nondestructive = false);

    /**
     * @return the list of currently shown tool views
     *
     * @note This function determines whether a tool view is shown by testing the checked state of its action.
     *       So if called before adaptToDockWidgetVisibilities(), it returns the list of previously shown views.
     */
    [[nodiscard]] QList<View*> shownViews() const;

    /**
     * @return the list of currently shown (the tool view action is checked) yet invisible floating dock widgets
     *
     * @note By definition, if the returned list is nonempty, the visibility of the floating
     *       dock widgets is out of sync with the checked state of their tool view actions.
     *       Such a desynchronization occurs if all top-level windows are closed on KDevelop exit.
     */
    [[nodiscard]] QList<QDockWidget*> shownButInvisibleFloatingDockWidgets() const;

    /**
     * Set the checked state of each tool view action to the visibility of its dock widget
     * and update the checked state of each Show Dock action accordingly.
     *
     * @note Call this function when restoring the main window state makes some dock widgets visible
     *       without checking their tool view actions. This function ensures tool view UI consistency.
     */
    void adaptToDockWidgetVisibilities();

    void loadButtonOrderSettings(const KConfigGroup& configGroup);
    void saveButtonOrderSettings(KConfigGroup& configGroup);

    /**
     * Hide all ideal toolbars.
     */
    void hideToolBars();
    /**
     * Show ideal toolbars that are nonempty.
     */
    void updateToolBarVisibility();

    void toggleDocksShown();

    enum Direction { NextDock, PrevDock };
    void goPrevNextDock(IdealController::Direction direction);

Q_SIGNALS:
        /// Emitted, when a context menu is requested on one of the dock bars.
    /// When no actions gets associated to the QMenu, it won't be shown.
    void dockBarContextMenuRequested(Qt::DockWidgetArea area, const QPoint& position);

private Q_SLOTS:
    void slotDockBarContextMenuRequested(const QPoint& position);
    void dockLocationChanged(Qt::DockWidgetArea);

private:
    void dockTopLevelChanged(bool topLevel);

    /**
     * Add a tool view action for a given area, dock widget and tool view.
     *
     * @param checked whether the action should be initially checked
     */
    void addBarWidgetAction(Qt::DockWidgetArea area, IdealDockWidget* dock, View* view, bool checked = false);

    [[nodiscard]] IdealButtonBarWidget* barForDockArea(Qt::DockWidgetArea area) const;

    template<typename ButtonBarWidgetUser>
    void forEachButtonBarWidget(ButtonBarWidgetUser callback) const;
    template<typename ToolBarUser>
    void forEachToolBar(ToolBarUser callback) const;

    template<typename ToolViewUser>
    void forEachShownToolView(ToolViewUser callback) const;

    void showDockWidget(IdealDockWidget* dock, bool show);

    void focusEditor();

    /**
     * Update the checked state of the Show Dock action for a given area when
     * the checked state of a tool view action in the same area becomes @p checked.
     */
    void setShowDockStatus(Qt::DockWidgetArea area, bool checked);
    /**
     * Set the checked state of the Show Dock action for a given area to @p checked.
     */
    void forceSetShowDockStatus(Qt::DockWidgetArea area, bool checked);

    /**
     * @return the Show Dock action for a given area
     */
    [[nodiscard]] QAction* actionForArea(Qt::DockWidgetArea area) const;

    void showDockActionToggled(QObject* senderAction, const IdealButtonBarWidget& buttonBarWidget, bool show);

    /**
     * @return the dock widget that currently has the focus or the dock widget one of whose
     *         descendant widgets has the focus or @c nullptr if there is no such dock widget
     */
    [[nodiscard]] IdealDockWidget* currentDockWidget() const;

    Sublime::MainWindow* const m_mainWindow;

    IdealButtonBarWidget* const m_leftBarWidget;
    IdealButtonBarWidget* const m_rightBarWidget;
    IdealButtonBarWidget* const m_topBarWidget;
    IdealButtonBarWidget* const m_bottomBarWidget;

    IdealToolBar* const m_leftToolBar;
    IdealToolBar* const m_rightToolBar;
    IdealToolBar* const m_bottomToolBar;

    QWidget* const m_bottomStatusBarLocation;

    /** Map from View to an action that shows/hides
        the IdealDockWidget containing that view.  */
    QHash<View*, QAction*> m_view_to_action;

    KActionMenu* m_docks;

    QAction* m_showLeftDock;
    QAction* m_showRightDock;
    QAction* m_showBottomDock;
    QAction* m_showTopDock;

};

}

#endif
