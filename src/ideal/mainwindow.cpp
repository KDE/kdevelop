/***************************************************************************
 *   Copyright (C) 2005-2006 by Alexander Dymo                             *
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
#include "mainwindow.h"

#include <QMap>
#include <QLayout>

#include "area.h"
#include "buttonbar.h"
#include "toolview.h"

namespace Ideal {

//==================== MainWindowPrivate =====================

struct MainWindowPrivate {
    MainWindowPrivate(MainWindow *window)
        :w(window)
    {
        area = 0;
    }
    ~MainWindowPrivate()
    {
        delete area;
    }

    MainWindow *w;
    QList<ToolView*> toolViews;
    QMap<QWidget*, ToolView*> toolViewsForWidget;
    QMap<Ideal::Place, ButtonBar*> buttonBars;

    Area *area;

    void initButtonBar(Ideal::Place place);
};

void MainWindowPrivate::initButtonBar(Ideal::Place place)
{
    ButtonBar *bar = w->createButtonBar(place);
    w->addToolBar(bar->toolBarPlace(), bar);
    buttonBars[place] = bar;
}



//======================== MainWindow ========================

MainWindow::MainWindow(QWidget *parent)
    :QMainWindow(parent)
{
    d = new MainWindowPrivate(this);

    d->initButtonBar(Ideal::Left);
    d->initButtonBar(Ideal::Right);
    d->initButtonBar(Ideal::Bottom);
    d->initButtonBar(Ideal::Top);
}

MainWindow::~MainWindow()
{
    delete d;
}

void MainWindow::setArea(int area)
{
    if (d->area && d->area->kind() == area)
        return; //do not switch to the same area
    delete d->area;
    d->area = new Area(area, this);
}

Area *MainWindow::area() const
{
    return d->area;
}

void MainWindow::addToolView(QWidget *view, Ideal::Place defaultPlace, int area)
{
    ToolView *toolView = createToolView(view, defaultPlace, area);
    d->toolViews.append(toolView);
    d->toolViewsForWidget[view] = toolView;
}

void MainWindow::removeToolView(QWidget *view)
{
    ToolView *toolView = d->toolViewsForWidget[view];
    if (!toolView)
        return;

    d->toolViews.removeAll(toolView);
    d->toolViewsForWidget.remove(view);
    d->buttonBars[toolView->place()]->removeToolViewButton(toolView->button());
}

QList<ToolView*> MainWindow::toolViews() const
{
    return d->toolViews;
}

ButtonBar *MainWindow::buttonBar(Ideal::Place place)
{
    return d->buttonBars[place];
}

ToolView *MainWindow::createToolView(QWidget *view, Ideal::Place defaultPlace, int area)
{
    return new ToolView(this, view, defaultPlace, area);
}

ButtonBar *MainWindow::createButtonBar(Ideal::Place place)
{
    return new ButtonBar(place, this);
}

}
