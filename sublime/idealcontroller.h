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

#ifndef SUBLIME_IDEAL_H
#define SUBLIME_IDEAL_H

#include <QAction>
#include <QActionEvent>
#include <QToolButton>
#include <QDockWidget>
#include <QStyleOption>

#include "sublimedefs.h"

class QAction;
class KActionMenu;

namespace Sublime {

class Area;
class View;
class MainWindow;
class IdealButtonBarWidget;
class IdealDockWidget;
class View;

class IdealController: public QObject
{
    Q_OBJECT
public:
    IdealController(Sublime::MainWindow *mainWindow);

    void addView(Qt::DockWidgetArea area, View* view);

    enum RaiseMode { HideOtherViews, GroupWithOtherViews };
    void raiseView(View* view, RaiseMode mode = HideOtherViews);
    void showDockWidget(IdealDockWidget* dock, bool show);
    void focusEditor();
    QWidget *statusBarLocation() const;
    QAction* actionForView(View* view) const;
    void setShowDockStatus(Qt::DockWidgetArea area, bool checked);

    /** Remove view.  If nondestructive true, view->widget()
        is not deleted, as is left with NULL parent.
        Otherwise, it's deleted.  */
    void removeView(View* view, bool nondestructive = false);

    void moveView(View *view, Qt::DockWidgetArea area);

    void showLeftDock(bool show);
    void showRightDock(bool show);
    void showBottomDock(bool show);
    void toggleDocksShown();

    IdealButtonBarWidget* barForDockArea(Qt::DockWidgetArea area) const;
    QAction* actionForArea(Qt::DockWidgetArea area) const;

    enum Direction { NextDock, PrevDock };
    void goPrevNextDock(IdealController::Direction direction);

    void setWidthForArea(Qt::DockWidgetArea, int);

    IdealButtonBarWidget *leftBarWidget;
    IdealButtonBarWidget *rightBarWidget;
    IdealButtonBarWidget *bottomBarWidget;
    IdealButtonBarWidget *topBarWidget;
    QWidget *bottomStatusBarLocation;

    IdealDockWidget* currentDockWidget();
    QMap<Qt::DockWidgetArea, QWeakPointer<IdealDockWidget> > lastDockWidget;

    void emitWidgetResized(Qt::DockWidgetArea dockArea, int thickness);

    QList<IdealDockWidget*> allDockWidgets();

Q_SIGNALS:
        /// Emitted, when a context menu is requested on one of the dock bars.
    /// When no actions gets associated to the KMenu, it won't be shown.
    void dockBarContextMenuRequested(Qt::DockWidgetArea area, const QPoint& position);
    void dockShown(Sublime::View*, Sublime::Position pos, bool shown);
    void widgetResized(Qt::DockWidgetArea dockArea, int thickness);

private Q_SLOTS:
    void slotDockBarContextMenuRequested(QPoint position);
    void dockLocationChanged(Qt::DockWidgetArea);
    void loadSettings();

private:
    void showDock(Qt::DockWidgetArea area, bool show);
    void toggleDocksShown(IdealButtonBarWidget *bar, bool show);

    Sublime::MainWindow *m_mainWindow;

    QSet<IdealDockWidget*> docks;

    /** Map from View to an action that shows/hides
        the IdealDockWidget containing that view.  */
    QMap<View*, QAction*> m_view_to_action;
    /** Map from IdealDockWidget  to an action that shows/hides
        that IdealDockWidget.  */
    QMap<IdealDockWidget*, QAction*> m_dockwidget_to_action;
    QMap<Qt::DockWidgetArea, int> m_widthsForAreas;

    KActionMenu* m_docks;

    QAction* m_showLeftDock;
    QAction* m_showRightDock;
    QAction* m_showBottomDock;
    QAction* m_showTopDock;

};

}

#endif
