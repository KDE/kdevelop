/***************************************************************************
 *   Copyright 2006-2007 Alexander Dymo  <adymo@kdevelop.org>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#ifndef SUBLIMEMAINWINDOW_P_H
#define SUBLIMEMAINWINDOW_P_H

#include <QMap>
#include <QObject>
#include <QSet>

#include "area.h"
#include "sublimedefs.h"

#include "mainwindow.h"

class QMenu;
class QAction;
class QSplitter;
class QDockWidget;

namespace Sublime {

class View;
class Container;
class Controller;
class AreaIndex;
class IdealMainWidget;

class MainWindowPrivate: public QObject {
    Q_OBJECT
public:
    MainWindowPrivate(MainWindow *w);

    /**Use this to create toolviews for an area.*/
    class ToolViewCreator {
    public:
        ToolViewCreator(MainWindowPrivate *_d): d(_d) {}
        Area::WalkerMode operator() (View *view, Sublime::Position position);
    private:
        MainWindowPrivate *d;
    };

    /**Use this to create ideal tool views for an area.*/
    class IdealToolViewCreator {
    public:
        IdealToolViewCreator(MainWindowPrivate *_d): d(_d) {}
        Area::WalkerMode operator() (View *view, Sublime::Position position);
    private:
        MainWindowPrivate *d;
    };

    /**Use this to create views for an area.*/
    class ViewCreator {
    public:
        ViewCreator(MainWindowPrivate *_d): d(_d) {}
        Area::WalkerMode operator() (AreaIndex *index);
    private:
        MainWindowPrivate *d;
    };

    /**Reconstructs the mainwindow according to the current area.*/
    void reconstruct();
    /**Clears the area leaving mainwindow empty.*/
    void clearArea();
    QMenu *areaSwitcherMenu();

    void activateFirstVisibleView();

    /**Updates all QDockWidgets to the TitleBarMode that is set*/
    void applyVerticalTitleBarMode();

    Controller *controller;
    Area *area;
    QList<QDockWidget*> docks;
    QMap<View*, QDockWidget*> viewDocks;
    QSet<View*> idealViews;
    QMap<View*, Container*> viewContainers;

    View *activeView;
    View *activeToolView;

    QWidget *centralWidget;

    Sublime::MainWindow::VerticalTabsMode m_verticalTabsMode;
    Sublime::MainWindow::VerticalTitleBarMode m_verticalTitleBarMode;
    Sublime::MainWindow::UserInterfaceStyle m_uistyle;

    IdealMainWidget *idealMainWidget;

public slots:
    void viewAdded(Sublime::AreaIndex *index, Sublime::View *view);
    void raiseToolView(Sublime::View* view);
    void aboutToRemoveView(Sublime::AreaIndex *index, Sublime::View *view);
    void toolViewAdded(Sublime::View *toolView, Sublime::Position position);
    void aboutToRemoveToolView(Sublime::View *toolView, Sublime::Position position);

private slots:
    void switchToArea(QAction *action);
    void updateAreaSwitcher(Sublime::Area *area);

protected:
    virtual bool eventFilter(QObject *, QEvent *event);
    
private:
    Qt::DockWidgetArea positionToDockArea(Position position);
    void recreateCentralWidget();

    MainWindow *m_mainWindow;
    QMap<AreaIndex*, QSplitter*> m_indexSplitters;
    QMenu *m_areaSwitcherMenu;

    QMap<Area*, QAction*> m_areaActions;
    QMap<QAction*, Area*> m_actionAreas;
};

}

#endif

