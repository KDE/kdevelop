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
#include "controller.h"

#include <QMap>
#include <QList>

#include <kdebug.h>

#include "area.h"
#include "document.h"
#include "mainwindow.h"

namespace Sublime {

// struct ControllerPrivate

struct ControllerPrivate {
    ControllerPrivate()
    {
    }

    void removeArea(QObject *obj)
    {
        areas.removeAll(reinterpret_cast<Area*>(obj));
    }

    void removeDocument(QObject *obj)
    {
        documents.removeAll(reinterpret_cast<Document*>(obj));
    }

    QList<Document*> documents;
    QList<Area*> areas;
    QMap<QString, Area*> namedAreas;
    QMap<Area*, MainWindow*> shownAreas;
};



// class Controller

Controller::Controller(QObject *parent)
    :QObject(parent), MainWindowOperator()
{
    init();
}

void Controller::init()
{
    d = new ControllerPrivate();
}

Controller::~Controller()
{
    delete d;
}

void Controller::showArea(Area *area, MainWindow *mainWindow)
{
    Area *areaToShow = 0;
    //if the area is already shown in another mainwindow then we need to clone it
    if (d->shownAreas.contains(area) && (mainWindow != d->shownAreas[area]))
        areaToShow = new Area(*area);
    else
        areaToShow = area;
    d->shownAreas[areaToShow] = mainWindow;
    MainWindowOperator::setArea(mainWindow, areaToShow);
    connect(areaToShow, SIGNAL(viewAdded(Sublime::AreaIndex*, Sublime::View*)),
        mainWindow, SLOT(viewAdded(Sublime::AreaIndex*, Sublime::View*)));
    connect(areaToShow, SIGNAL(aboutToRemoveView(Sublime::AreaIndex*, Sublime::View*)),
        mainWindow, SLOT(aboutToRemoveView(Sublime::AreaIndex*, Sublime::View*)));
    connect(areaToShow, SIGNAL(toolViewAdded(Sublime::View*, Sublime::Position)),
        mainWindow, SLOT(toolViewAdded(Sublime::View*, Sublime::Position)));
    connect(areaToShow, SIGNAL(aboutToRemoveToolView(Sublime::View*, Sublime::Position)),
        mainWindow, SLOT(aboutToRemoveToolView(Sublime::View*, Sublime::Position)));
}

QList<Area*> &Controller::areas() const
{
    return d->areas;
}

QList<Document*> &Controller::documents() const
{
    return d->documents;
}

void Controller::addArea(Area *area)
{
    d->areas.append(area);
    d->namedAreas[area->objectName()] = area;
}

void Controller::addDocument(Document *document)
{
    d->documents.append(document);
}

void Controller::areaReleased()
{
    MainWindow *w = reinterpret_cast<Sublime::MainWindow*>(sender());
    kDebug(9037) << "marking areas as mainwindow-free" << endl;
    foreach (Area *area, d->shownAreas.keys(w))
    {
        kDebug(9037) << "   " << area->objectName() << endl;
        areaReleased(area);
        disconnect(area, 0, w, 0);
    }
}

void Controller::areaReleased(Sublime::Area *area)
{
    d->shownAreas.remove(area);
    d->namedAreas.remove(area->objectName());
}

Area *Controller::area(const QString &areaName)
{
    return d->namedAreas[areaName];
}

}

#include "controller.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
