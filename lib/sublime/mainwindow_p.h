/***************************************************************************
 *   Copyright (C) 2006-2007 by Alexander Dymo  <adymo@kdevelop.org>       *
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

#include "area.h"
#include "sublimedefs.h"

class QMenu;
class QAction;
class QSplitter;
class QDockWidget;

namespace Sublime {

class View;
class Controller;
class AreaIndex;
class MainWindow;

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

    Controller *controller;
    Area *area;
    QList<QDockWidget*> docks;

private slots:
    void switchToArea(QAction *action);
    void updateAreaSwitcher(Sublime::Area *area);

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

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
