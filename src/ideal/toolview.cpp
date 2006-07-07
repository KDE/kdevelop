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
#include "toolview.h"

#include <kdebug.h>

#include "button.h"
#include "buttonbar.h"
#include "mainwindow.h"
#include "toolviewwidget.h"

namespace Ideal {

//==================== ToolViewPrivate =====================

struct ToolViewPrivate {

    ToolViewPrivate(ToolView *toolView, MainWindow *_mainWindow, QWidget *_contents,
            Ideal::Place _place, int _area)
        :toolView(toolView), mainWindow(_mainWindow), contents(_contents),
            place(_place), area(_area)
    {
        button = toolView->createToolViewButton(place, contents->windowTitle(), contents->windowIcon());
        toolView->connect(button, SIGNAL(toggled(bool)), toolView, SLOT(setViewVisible(bool)));

        mainWindow->buttonBar(place)->addToolViewButton(button);

        dockWidget = 0;
    }
    ~ToolViewPrivate()
    {
    }

    ToolView *toolView;
    MainWindow *mainWindow;
    QWidget *contents;
    Ideal::Place place;
    int area;
    //toolview button
    Button *button;
    //toolview dock
    ToolViewWidget *dockWidget;
};



//==================== ToolView =====================

ToolView::ToolView(MainWindow *parent, QWidget *contents, Ideal::Place place, int area)
    :QObject(parent)
{
    d = new ToolViewPrivate(this, parent, contents, place, area);
}

ToolView::~ToolView()
{
    delete d;
}

Qt::DockWidgetArea ToolView::dockPlace() const
{
    return dockPlace(d->place);
}

Qt::DockWidgetArea ToolView::dockPlace(Ideal::Place place)
{
    Qt::DockWidgetArea dockArea = Qt::LeftDockWidgetArea;
    if (place == Ideal::Right) dockArea = Qt::RightDockWidgetArea;
    else if (place == Ideal::Bottom) dockArea = Qt::BottomDockWidgetArea;
    else if (place == Ideal::Top) dockArea = Qt::TopDockWidgetArea;
    return dockArea;
}

Ideal::Place ToolView::place() const
{
    return d->place;
}

int Ideal::ToolView::area() const
{
    return d->area;
}

QWidget *ToolView::contents() const
{
    return d->contents;
}

Button *ToolView::button() const
{
    return d->button;
}

ToolViewWidget *ToolView::dockWidget()
{
    if (!d->dockWidget)
    {
        d->dockWidget = createDockWidget();
        setupDockWidget(d->dockWidget);
    }
    return d->dockWidget;
}

void ToolView::setViewVisible(bool visible)
{
    ButtonBar *bar = d->mainWindow->buttonBar(d->place);
    if (visible)
        bar->setVisible(true);
    d->button->setChecked(visible);
    dockWidget()->setVisible(visible);
}

void ToolView::setViewEnabled(bool enabled)
{
    ButtonBar *bar = d->mainWindow->buttonBar(d->place);
    d->button->setVisible(enabled);
    if (enabled)
        bar->setVisible(true);
    else
    {
        kDebug() << "disable view" << endl;
        //if we have the dock widget and it is visible we should hide it
        //to make the view disabled
        if (d->dockWidget)
           d->dockWidget->hide();
        //also hide the empty button bar
        if (bar->isEmpty())
            bar->setVisible(false);
    }
}

void ToolView::showView()
{
    setViewVisible(true);
}

void ToolView::hideView()
{
    setViewVisible(false);
}

void ToolView::enableView()
{
    setViewEnabled(true);
}

void ToolView::disableView()
{
    setViewEnabled(false);
}

ToolViewWidget *ToolView::createDockWidget()
{
    return new ToolViewWidget(d->contents->windowTitle(), d->mainWindow);
}

void ToolView::setupDockWidget(ToolViewWidget *dockWidget)
{
    connect(dockWidget, SIGNAL(visibilityChanged(bool)), d->button, SLOT(setChecked(bool)));
    d->mainWindow->addDockWidget(dockPlace(), dockWidget);
    dockWidget->setWidget(d->contents);
}

Button *ToolView::createToolViewButton(Ideal::Place place, const QString &title, const QIcon &icon)
{
    return new Button(0, place, title, icon);
}

}
#include "toolview.moc"
