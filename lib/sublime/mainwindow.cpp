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
#include <kglobal.h>
#include <kconfig.h>
#include <ksharedconfig.h>

#include "area.h"
#include "view.h"
#include "controller.h"
#include "container.h"
#include "mainwindow_p.h"

namespace Sublime {

MainWindow::MainWindow(Controller *controller, Qt::WindowFlags flags)
    :KParts::MainWindow(0, flags), d(new MainWindowPrivate(this))
{
    d->controller = controller;
    connect(this, SIGNAL(destroyed()), controller, SLOT(areaReleased()));
    connect(this, SIGNAL(areaCleared(Sublime::Area*)), controller, SLOT(areaReleased(Sublime::Area*)));
}

MainWindow::~MainWindow()
{
    kDebug(9037) << "destroying mainwindow" << endl;
    delete d;
}

void MainWindow::setArea(Area *area)
{
    if (d->area)
        clearArea();
    d->area = area;
    d->reconstruct();
    d->activateFirstVisibleView();
    emit areaChanged(area);

    loadSettings();
}

void MainWindow::clearArea()
{
    emit areaCleared(d->area);
    d->clearArea();

    saveSettings();
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

Controller *MainWindow::controller() const
{
    return d->controller;
}

View *MainWindow::activeView()
{
    return d->activeView;
}

View *MainWindow::activeToolView()
{
    return d->activeToolView;
}

void MainWindow::activateView(View *view)
{
    if (!d->viewContainers.contains(view))
        return;
    d->viewContainers[view]->setCurrentWidget(view->widget());
    setActiveView(view);
}

void MainWindow::setActiveView(View *view)
{
    d->activeView = view;
    if (!view->widget()->hasFocus())
        view->widget()->setFocus();
    emit activeViewChanged(view);
}

void Sublime::MainWindow::setActiveToolView(View *view)
{
    d->activeToolView = view;
    emit activeToolViewChanged(view);
}

void MainWindow::saveSettings()
{
    QString group = "MainWindow";
    if (area())
        group += "_" + area()->objectName();
    KConfigGroup cg = KGlobal::config()->group(group);
    saveMainWindowSettings(cg);
}

void MainWindow::loadSettings()
{
    QString group = "MainWindow";
    if (area())
        group += "_" + area()->objectName();
    KConfigGroup cg = KGlobal::config()->group(group);
    applyMainWindowSettings(cg);
}

bool MainWindow::queryClose()
{
    saveSettings();
    return KParts::MainWindow::queryClose();
}

}

#include "mainwindow.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
