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

class KActionMenu;
class KConfigGroup;

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

    enum RaiseMode { HideOtherViews, GroupWithOtherViews };
    void raiseView(View* view, RaiseMode mode = HideOtherViews);
    void showDockWidget(IdealDockWidget* dock, bool show);
    QWidget *statusBarLocation() const;
    QAction* actionForView(View* view) const;

    /** Remove view.  If nondestructive true, view->widget()
        is not deleted, as is left with NULL parent.
        Otherwise, it's deleted.  */
    void removeView(View* view, bool nondestructive = false);

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
    void dockShown(Sublime::View*, Sublime::Position pos, bool shown);

private Q_SLOTS:
    void slotDockBarContextMenuRequested(const QPoint& position);
    void dockLocationChanged(Qt::DockWidgetArea);

private:
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

    void focusEditor();
    void setShowDockStatus(Qt::DockWidgetArea area, bool checked);
    [[nodiscard]] QAction* actionForArea(Qt::DockWidgetArea area) const;

    void showDockActionToggled(QObject* senderAction, const IdealButtonBarWidget& buttonBarWidget, bool show);
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
