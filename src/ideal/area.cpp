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
#include "toolview.h"

namespace Ideal {

struct AreaPrivate {
    int kind;
    MainWindow *mainWindow;
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
        placeToolView(view);
}

void Area::placeToolView(ToolView *toolView)
{
    qDebug() << "1";
    if (toolView->areaKind() & d->kind)
    {
        qDebug() << "2";
        //TODO place the view here
        d->mainWindow->addDockWidget(toolView->dockPlace(), toolView);
    }
}

}

#include "area.moc"
