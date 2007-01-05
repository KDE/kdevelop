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
#include "mainwindow.h"

#include <kdebug.h>

#include "area.h"
#include "controller.h"
#include "mainwindow_p.h"

namespace Sublime {

MainWindow::MainWindow(Controller *controller, Qt::WindowFlags flags)
    :KParts::MainWindow(0, flags)
{
    init();
    d->controller = controller;
    connect(this, SIGNAL(destroyed()), controller, SLOT(areaReleased()));
    connect(this, SIGNAL(areaCleared(Sublime::Area*)), controller, SLOT(areaReleased(Sublime::Area*)));
}

void MainWindow::init()
{
    d = new MainWindowPrivate(this);
}

MainWindow::~MainWindow()
{
    kDebug() << "destroying mainwindow" << endl;
    delete d;
}

void MainWindow::setArea(Area *area)
{
    if (d->area)
        clearArea();
    d->area = area;
    d->reconstruct();
    emit areaChanged(area);
}

void MainWindow::clearArea()
{
    emit areaCleared(d->area);
    d->clearArea();
    kDebug() << "area cleared" << endl;
}

QMenu *MainWindow::areaSwitcherMenu()
{
    return d->areaSwitcherMenu();
}

QList<QDockWidget*> MainWindow::toolDocks() const
{
    return d->docks;
}

Area *Sublime::MainWindow::area() const
{
    return d->area;
}

}

#include "mainwindow.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
