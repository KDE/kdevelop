/***************************************************************************
 *   Copyright (C) 2006 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
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
#include "area.h"

#include <QDebug>

#include "mainwindow.h"
#include "buttonbar.h"
#include "toolview.h"
#include "toolviewwidget.h"

namespace Ideal {

struct AreaPrivate {
    int kind;
    MainWindow *mainWindow;
    QMap<ToolView*, ToolViewWidget*> toolDocks;
};

Area::Area(int kind, MainWindow *mainWindow)
{
    d = new AreaPrivate;
    d->kind = kind;
    d->mainWindow = mainWindow;

    initArea();
}

Area::~Area()
{
    delete d;
}

void Area::initArea()
{
    QList<ToolView*> toolViews = d->mainWindow->toolViews();
    foreach (ToolView *view, toolViews)
        addToolView(view);
}

void Area::addToolView(ToolView *view)
{
    if (!allowed(view))
        return;

    ToolViewWidget *dockWidget = new ToolViewWidget(view->contents()->windowTitle(), d->mainWindow);
    dockWidget->setWidget(view->contents());
    d->toolDocks[view] = dockWidget;
    d->mainWindow->addDockWidget(view->dockPlace(), dockWidget);
    d->mainWindow->buttonBar(view->place())->addToolViewButton(dockWidget);
}

void Area::removeToolView(ToolView *view)
{
    if (!allowed(view))
        return;
    ToolViewWidget *dockWidget = d->toolDocks[view];
    d->mainWindow->removeDockWidget(dockWidget);
    d->mainWindow->buttonBar(view->place())->removeToolViewButton(dockWidget);
    d->toolDocks.remove(view);
    delete dockWidget;
}

void Area::showToolView(ToolView *view)
{
    if (!allowed(view))
        return;
    ToolViewWidget *dockWidget = d->toolDocks[view];
    dockWidget->show();
    d->mainWindow->buttonBar(view->place())->showToolViewButton(dockWidget);
}

void Area::hideToolView(ToolView *view)
{
    if (!allowed(view))
        return;
    ToolViewWidget *dockWidget = d->toolDocks[view];
    dockWidget->hide();
    d->mainWindow->buttonBar(view->place())->hideToolViewButton(dockWidget);
}

int Area::kind() const
{
    return d->kind;
}

bool Area::allowed(ToolView *view)
{
    return view->area() & d->kind;
}

}
