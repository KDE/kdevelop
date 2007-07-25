/***************************************************************************
 *   Copyright 2006-2007 by Alexander Dymo  <adymo@kdevelop.org>       *
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
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>

#include <kdebug.h>

#include "area.h"
#include "view.h"
#include "document.h"
#include "mainwindow.h"

namespace Sublime {

struct WidgetFinder {
    WidgetFinder(QWidget *_w) :w(_w), view(0) {}
    Area::WalkerMode operator()(AreaIndex *index)
    {
        foreach (View *v, index->views())
        {
            if (v->hasWidget() && (v->widget() == w))
            {
                view = v;
                return Area::StopWalker;
            }
        }
        return Area::ContinueWalker;
    }

    QWidget *w;
    View *view;
};

struct ToolWidgetFinder {
    ToolWidgetFinder(QWidget *_w) :w(_w), view(0) {}
    Area::WalkerMode operator()(View *v, Sublime::Position /*position*/)
    {
        if (v->hasWidget() && (v->widget() == w))
        {
            view = v;
            return Area::StopWalker;
        }
        return Area::ContinueWalker;
    }

    QWidget *w;
    View *view;
};


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
    QList<MainWindow*> controlledWindows;
};



// class Controller

Controller::Controller(QObject *parent)
    :QObject(parent), MainWindowOperator(), d( new ControllerPrivate() )
{
    init();
}

void Controller::init()
{

    qApp->installEventFilter(this);
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
    d->controlledWindows << mainWindow;
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
    d->controlledWindows.removeAll(d->shownAreas[area]);
    d->shownAreas.remove(area);
    d->namedAreas.remove(area->objectName());
}

Area *Controller::area(const QString &areaName)
{
    return d->namedAreas[areaName];
}

/*We need this to catch activation of views and toolviews
so that we can always tell what view and toolview is active.
"Active" doesn't mean focused. It means that it is focused now
or was focused before and no other view/toolview wasn't focused
after that."*/
//implementation is based upon KParts::PartManager::eventFilter
bool Controller::eventFilter(QObject *obj, QEvent *ev)
{

    if (ev->type() != QEvent::MouseButtonPress &&
        ev->type() != QEvent::MouseButtonDblClick &&
        ev->type() != QEvent::FocusIn)
        return false;

    //not a widget? - return
    if (!obj->isWidgetType())
        return false;

    //is dialog or popup? - return
    QWidget *w = static_cast<QWidget*>(obj);
    if (((w->windowFlags().testFlag(Qt::Dialog)) && w->isModal()) ||
            (w->windowFlags().testFlag(Qt::Popup)) || (w->windowFlags().testFlag(Qt::Tool)))
        return false;

    //not a mouse button that should activate the widget? - return
    QMouseEvent *mev = 0;
    if (ev->type() == QEvent::MouseButtonPress || ev->type() == QEvent::MouseButtonDblClick)
    {
        mev = static_cast<QMouseEvent*>(ev);
        int activationButtonMask = Qt::LeftButton | Qt::MidButton | Qt::RightButton;
        if ((mev->button() & activationButtonMask) == 0)
            return false;
    }

    while (w)
    {
        //not inside sublime mainwindow
        MainWindow *mw = qobject_cast<Sublime::MainWindow*>(w->topLevelWidget());
        if (!mw || !d->controlledWindows.contains(mw))
            return false;

        Area *area = mw->area();

        ///@todo adymo: this is extra slow - optimize
        //find this widget in views
        WidgetFinder widgetFinder(w);
        area->walkViews(widgetFinder, area->rootIndex());
        if (widgetFinder.view && widgetFinder.view != mw->activeView())
        {
            setActiveView(mw, widgetFinder.view);
            ///@todo adymo: shall we filter out the event?
            return false;
        }

        //find this widget in toolviews
        ToolWidgetFinder toolFinder(w);
        area->walkToolViews(toolFinder, Sublime::AllPositions);
        if (toolFinder.view && toolFinder.view != mw->activeToolView())
        {
            setActiveToolView(mw, toolFinder.view);
            ///@todo adymo: shall we filter out the event?
            return false;
        }

        w = w->parentWidget();
    }

    return false;
}

}

#include "controller.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
