/*
    SPDX-FileCopyrightText: 2007 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2011 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LicenseRef-MIT-KDevelop-Ideal
*/

#ifndef SUBLIME_IDEALCONTROLLER_H
#define SUBLIME_IDEALCONTROLLER_H

#include <QAction>

#include "sublimedefs.h"

class KActionMenu;

namespace Sublime {

class Area;
class View;
class MainWindow;
class IdealButtonBarWidget;
class IdealDockWidget;

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

    void moveView(View *view, Qt::DockWidgetArea area);

    void showLeftDock(bool show);
    void showRightDock(bool show);
    void showBottomDock(bool show);
    void toggleDocksShown();

    enum Direction { NextDock, PrevDock };
    void goPrevNextDock(IdealController::Direction direction);

    IdealButtonBarWidget *leftBarWidget;
    IdealButtonBarWidget *rightBarWidget;
    IdealButtonBarWidget *bottomBarWidget;
    IdealButtonBarWidget *topBarWidget;

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
     *
     * @return whether the action was added successfully
     */
    bool addBarWidgetAction(Qt::DockWidgetArea area, IdealDockWidget* dock, View* view, bool checked = false);

    [[nodiscard]] IdealButtonBarWidget* barForDockArea(Qt::DockWidgetArea area) const;
    void focusEditor();
    void setShowDockStatus(Qt::DockWidgetArea area, bool checked);
    [[nodiscard]] QAction* actionForArea(Qt::DockWidgetArea area) const;

    void showDock(Qt::DockWidgetArea area, bool show);
    [[nodiscard]] IdealDockWidget* currentDockWidget() const;

    Sublime::MainWindow* const m_mainWindow;

    QWidget* m_bottomStatusBarLocation;

    /** Map from View to an action that shows/hides
        the IdealDockWidget containing that view.  */
    QMap<View*, QAction*> m_view_to_action;

    KActionMenu* m_docks;

    QAction* m_showLeftDock;
    QAction* m_showRightDock;
    QAction* m_showBottomDock;
    QAction* m_showTopDock;

};

}

#endif
