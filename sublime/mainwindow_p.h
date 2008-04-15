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
class Controller;
class AreaIndex;
class IdealMainWidget;

class MainWindowPrivate: public QObject {
    Q_OBJECT
public:
    MainWindowPrivate(MainWindow *w);

    /**Use this to create tool views for an area.*/
    class IdealToolViewCreator {
    public:
        IdealToolViewCreator(MainWindowPrivate *_d): d(_d) {}
        Area::WalkerMode operator() (View *view, Sublime::Position position);
    private:
        MainWindowPrivate *d;
    };

#if 0
    /**Use this to create views for an area.*/
    class ViewCreator {
    public:
        ViewCreator(MainWindowPrivate *_d): d(_d) {}
        Area::WalkerMode operator() (AreaIndex *index);
    private:
        MainWindowPrivate *d;
    };
#endif

    /**Reconstructs the mainwindow according to the current area.*/
    void reconstruct();

    QMenu *areaSwitcherMenu();

    void activateFirstVisibleView();

    /**Updates all QDockWidgets to the TitleBarMode that is set*/
    void applyVerticalTitleBarMode();

    Controller *controller;
    Area *area;
    QList<View*> docks;

    View *activeView;
    View *activeToolView;

    QWidget *centralWidget;

    IdealMainWidget *idealMainWidget;

public slots:
    void raiseToolView(Sublime::View* view);
    void toolViewAdded(Sublime::View *toolView, Sublime::Position position);
    void aboutToRemoveToolView(Sublime::View *toolView, Sublime::Position position);
    void toolViewMoved(Sublime::View *toolView, Sublime::Position position);

private slots:
    void switchToArea(QAction *action);
    void updateAreaSwitcher(Sublime::Area *area);

protected:
    virtual bool eventFilter(QObject *, QEvent *event);

private:
    Qt::DockWidgetArea positionToDockArea(Position position);

    MainWindow *m_mainWindow;
    QMap<AreaIndex*, QSplitter*> m_indexSplitters;
    QMenu *m_areaSwitcherMenu;

    QMap<Area*, QAction*> m_areaActions;
    QMap<QAction*, Area*> m_actionAreas;
};

}

#endif

