/***************************************************************************
 *   Copyright (C) 2007 by Alexander Dymo  <adymo@kdevelop.org>            *
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
#ifndef UICONTROLLER_H
#define UICONTROLLER_H

#include <kdevexport.h>

#include "iuicontroller.h"
#include <sublime/controller.h>

namespace KDevelop {

class Core;
class MainWindow;

class KDEVPLATFORM_EXPORT UiController: public Sublime::Controller, public IUiController {
public:
    UiController(Core *core);
    virtual ~UiController();

    /** @return area for currently active sublime mainwindow or 0 if
    no sublime mainwindow is active.*/
    virtual Sublime::Area *activeArea();
    /** @return active sublime mainwindow or 0 if no such mainwindow is active.*/
    virtual Sublime::MainWindow *activeMainWindow();

    /** @return default main window - the main window for default area in the shell.
    No guarantee is given that it always exists so this method may return 0.*/
    MainWindow *defaultMainWindow();
    /** @return the default area for this shell.*/
    Sublime::Area *defaultArea();

    virtual void switchToArea(const QString &areaName, SwitchMode switchMode);

    virtual void addToolView(const QString &name, IToolViewFactory *factory);
    virtual void removeToolView(IToolViewFactory *factory);
    virtual void openUrl(const KUrl &url);

    void openEmptyDocument();

    void initialize();
private:
    class UiControllerPrivate *d;
};

}

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
