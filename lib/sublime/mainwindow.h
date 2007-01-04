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
#ifndef SUBLIMEMAINWINDOW_H
#define SUBLIMEMAINWINDOW_H

#include <kparts/mainwindow.h>

namespace Sublime {

class Area;
class Controller;
class MainWindowOperator;

class MainWindow: public KParts::MainWindow {
Q_OBJECT
public:
    MainWindow(Controller *controller, Qt::WindowFlags flags = KDE_DEFAULT_WINDOWFLAGS);
    ~MainWindow();

    QMenu *areaSwitcherMenu();

signals:
    void areaCleared(Sublime::Area*);
    void areaChanged(Sublime::Area*);

private slots:
    void switchToArea(QAction *action);
    void updateAreaSwitcher(Sublime::Area *area);

private:
    void init();

    //Inherit MainWindowOperator to access two methods below
    /**Sets the area of main window and fills it with views.*/
    void setArea(Area *area);
    /**Unsets the area clearing main window.*/
    void clearArea();

    struct MainWindowPrivate *d;
    friend class MainWindowOperator;
};

}

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
