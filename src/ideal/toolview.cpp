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

#include <QTimer>
#include <QInputDialog>

#include <kmenu.h>
#include <klocale.h>
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
        toolView->connect(button, SIGNAL(customContextMenuRequested(const QPoint&)),
            toolView, SLOT(buttonContextMenu(const QPoint&)));

        mainWindow->buttonBar(place)->addToolViewButton(button);

        dockWidget = 0;
        isVisible = false;
        isEnabled = false;
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
    bool isVisible;
    bool isEnabled;
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

Ideal::Place ToolView::idealPlace(Qt::DockWidgetArea dockPlace)
{
    Ideal::Place place = Ideal::Left;
    if (dockPlace == Qt::RightDockWidgetArea) place = Ideal::Right;
    else if (dockPlace == Qt::BottomDockWidgetArea) place = Ideal::Bottom;
    else if (dockPlace == Qt::TopDockWidgetArea) place = Ideal::Top;
    return place;
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
    if (d->isVisible == visible)
        return;
    d->isVisible = visible;
    ButtonBar *bar = d->mainWindow->buttonBar(d->place);
    if (visible)
    {
        bar->setVisible(true);
        foreach (ToolView *view,
            d->mainWindow->toolViews(d->place, ToolView::Enabled | ToolView::Visible))
        {
            if ((view->place() == d->place) and (view != this))
                view->hideView();
        }
    }
    d->button->setChecked(visible);
    dockWidget()->setVisible(visible);
}

void ToolView::setViewEnabled(bool enabled)
{
    if (d->isEnabled == enabled)
        return;
    d->isEnabled = enabled;

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
        if (d->mainWindow->toolViews(d->place, ToolView::Visible).count() == 0)
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
//     dockWidget->setAllowedAreas(dockPlace());
    dockWidget->setWidget(d->contents);
    connect(dockWidget, SIGNAL(visibilityChanged(bool)), d->button, SLOT(setChecked(bool)));
    connect(dockWidget, SIGNAL(topLevelChanged(bool)), this, SLOT(adjustPlacement(bool)));
    d->mainWindow->addDockWidget(dockPlace(), dockWidget);
}

Button *ToolView::createToolViewButton(Ideal::Place place, const QString &title, const QIcon &icon)
{
    return new Button(0, place, title, icon);
}

bool ToolView::isVisible() const
{
    return d->isVisible;
}

bool ToolView::isEnabled() const
{
    return d->isEnabled;
}

int ToolView::mode() const
{
    int mode = 0;
    if (d->isVisible)
        mode |= ToolView::Visible;
    if (d->isEnabled)
        mode |= ToolView::Enabled;
    if (!d->isVisible && !d->isEnabled)
        mode = ToolView::None;
    return mode;
}

void ToolView::adjustPlacement(bool toplevel)
{
    if (toplevel)
        return;

    QTimer::singleShot(1000, this, SLOT(setDockPlace()));
}

void ToolView::setDockPlace(Qt::DockWidgetArea dockPlace)
{
    kDebug() << "set dock place from " << ToolView::dockPlace()<< " to " << dockPlace << endl;
    if (dockPlace == ToolView::dockPlace())
        return; //nothing to do here - already in the place
    if (d->dockWidget)
    {
        if (d->mainWindow->dockWidgetArea(d->dockWidget) != dockPlace)
        {
            //we need to move the dock widget first
            d->mainWindow->removeDockWidget(d->dockWidget);
            d->mainWindow->addDockWidget(dockPlace, d->dockWidget);
        }
    }
    ButtonBar *origBar = d->mainWindow->buttonBar(d->place);
    d->place = idealPlace(dockPlace);
    ButtonBar *newBar = d->mainWindow->buttonBar(d->place);
    origBar->removeToolViewButton(d->button);
    newBar->addToolViewButton(d->button);

    d->button->setPlace(d->place);
    if (d->isVisible)
        d->button->show();
    if (d->isEnabled)
        newBar->show();
}

void ToolView::setDockPlace()
{
    kDebug() << "TEST: dock area is: " << d->mainWindow->dockWidgetArea(d->dockWidget) << endl;
    setDockPlace(d->mainWindow->dockWidgetArea(d->dockWidget));
}

void ToolView::buttonContextMenu(const QPoint &p)
{
    kDebug() << k_funcinfo << endl;

    Button *button = qobject_cast<Ideal::Button*>(sender());
    if (!button)
        return;

    KMenu menu;
    QAction *renumber = menu.addAction("Assign Number...");
    QAction *result = menu.exec(button->mapToGlobal(p));

    if (result == renumber)
    {
        bool ok;
        int newNumber = QInputDialog::getInteger(d->mainWindow, i18n("Toolview renumbering"),
            i18n("Enter toolview number to be used as accelerator:"), 0, 0, 1000, 1, &ok);
        if (ok)
            d->mainWindow->renumberToolViews(this, newNumber);
    }
}

}

#include "toolview.moc"
