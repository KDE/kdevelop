/*
    SPDX-FileCopyrightText: 2006-2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_SUBLIMEMAINWINDOWOPERATOR_H
#define KDEVPLATFORM_SUBLIMEMAINWINDOWOPERATOR_H

//krazy:excludeall=dpointer

#include "sublimeexport.h"

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
    /**Sets the active tool view for the mainwindow and focuses it.*/
    void setActiveToolView(MainWindow *w, View *view);
};

}

#endif

