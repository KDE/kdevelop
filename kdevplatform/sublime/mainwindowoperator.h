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
#ifndef KDEVPLATFORM_SUBLIMEMAINWINDOWOPERATOR_H
#define KDEVPLATFORM_SUBLIMEMAINWINDOWOPERATOR_H

//krazy:excludeall=dpointer

#include "sublimeexport.h"

#include <QIcon>

namespace Sublime {

class Area;
class View;
class MainWindow;

/**Proxy class to give descendants access to MainWindow::setArea
and MainWindow::hideArea methods. Just inherit it and use the methods.*/
class KDEVPLATFORMSUBLIME_EXPORT MainWindowOperator {
protected:
    /**Sets the area of main window and fills it with views.*/
    void setArea(MainWindow *w, Area *area);
    /**Unsets the area clearing main window.*/
    void clearArea(MainWindow *w);

    /**Sets the active view for the mainwindow and focuses it.*/
    void setActiveView(MainWindow *w, View *view);
    /**Sets the active toolview for the mainwindow and focuses it.*/
    void setActiveToolView(MainWindow *w, View *view);
};

}

#endif

