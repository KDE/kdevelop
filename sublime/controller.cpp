/***************************************************************************
 *   Copyright 2006-2007 Alexander Dymo  <adymo@kdevelop.org>       *
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
    QList<Area*> allAreas;
    QMap<QString, Area*> namedAreas;
    // FIXME: remove this.
    QMap<Area*, MainWindow*> shownAreas;
    QList<MainWindow*> controlledWindows;
    QVector< QList<Area*> > mainWindowAreas;
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
    // FIXME:
//    foreach (MainWindow *w, d->controlledWindows)
//        delete w;
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

    showAreaInternal(areaToShow, mainWindow);
}

void Controller::showAreaInternal(Area* area, MainWindow *mainWindow)
{
    /* Disconnect the previous area.  We really don't want to mess with
       main window if an area not visible now is modified.  Further,
       if showAreaInternal is called with the same area as is current
       now, we don't want to connect the same signals twice.  */
    MainWindowOperator::setArea(mainWindow, area);
}

void Controller::showArea(const QString& areaTypeId, MainWindow *mainWindow)
{
    int index = d->controlledWindows.indexOf(mainWindow);
    Q_ASSERT(index != -1);

    Area* area = NULL;
    foreach (Area* a, d->mainWindowAreas[index])
    {
        kDebug() << "Object name: " << a->objectName() << " id "
                     << areaTypeId;
        if (a->objectName() == areaTypeId)
        {
            area = a;
            break;
        }
    }
    Q_ASSERT (area);

    showAreaInternal(area, mainWindow);
}

void Controller::resetCurrentArea(MainWindow *mainWindow)
{
    QString id = mainWindow->area()->objectName();

    int areaIndex = 0;
    Area* def = NULL;
    foreach (Area* a, d->areas) {
        if (a->objectName() == id)
        {
            def = a;
            break;
        }
        ++areaIndex;
    }
    Q_ASSERT(def);

    int index = d->controlledWindows.indexOf(mainWindow);
    Q_ASSERT(index != -1);

    Area* prev = d->mainWindowAreas[index][areaIndex];
    d->mainWindowAreas[index][areaIndex] = new Area(*def);
    showAreaInternal(d->mainWindowAreas[index][areaIndex], mainWindow);
    delete prev;
}

const QList<Area*> &Controller::defaultAreas() const
{
    return d->areas;
}


const QList< Area* >& Controller::areas(MainWindow* mainWindow) const
{
    int index = d->controlledWindows.indexOf(mainWindow);
    Q_ASSERT(index != -1);
    return areas(index);
}

const QList<Area*> &Controller::areas(int mainWindow) const
{
    return d->mainWindowAreas[mainWindow];
}

const QList<Area*> &Controller::allAreas() const
{
    return d->allAreas;
}

const QList<Document*> &Controller::documents() const
{
    return d->documents;
}

void Controller::addDefaultArea(Area *area)
{
    d->areas.append(area);
    d->allAreas.append(area);
    d->namedAreas[area->objectName()] = area;
    emit areaCreated(area);
}

void Controller::addMainWindow(MainWindow* mainWindow)
{
    Q_ASSERT (!d->controlledWindows.contains(mainWindow));
    d->controlledWindows << mainWindow;
    d->mainWindowAreas.resize(d->controlledWindows.size());
    int index = d->controlledWindows.size()-1;

    foreach (Area* area, defaultAreas())
    {
        Area *na = new Area(*area);
        d->allAreas.append(na);
        d->mainWindowAreas[index].push_back(na);
        emit areaCreated(na);
    }
    showAreaInternal(d->mainWindowAreas[index][0], mainWindow);
    emit mainWindowAdded( mainWindow );
}

void Controller::addDocument(Document *document)
{
    d->documents.append(document);
}

void Controller::areaReleased()
{
    MainWindow *w = reinterpret_cast<Sublime::MainWindow*>(sender());
    kDebug() << "marking areas as mainwindow-free" << w << d->controlledWindows.contains(w) << d->shownAreas.keys(w);
    foreach (Area *area, d->shownAreas.keys(w))
    {
        kDebug() << "" << area->objectName();
        areaReleased(area);
        disconnect(area, 0, w, 0);
    }

    d->controlledWindows.removeAll(w);
}

void Controller::areaReleased(Sublime::Area *area)
{
    d->shownAreas.remove(area);
    d->namedAreas.remove(area->objectName());
}

Area *Controller::defaultArea(const QString &id)
{
    return d->namedAreas[id];
}

Area *Controller::area(int mainWindow, const QString& id)
{
    foreach (Area* area, areas(mainWindow))
    {
        if (area->objectName() == id)
            return area;
    }
    return 0;
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

const QList< MainWindow * > & Controller::mainWindows() const
{
    return d->controlledWindows;
}


void Controller::notifyToolViewRemoved(Sublime::View *view, Sublime::Position)
{
    emit aboutToRemoveToolView(view);
}

void Controller::notifyToolViewAdded(Sublime::View *view, Sublime::Position)
{
    emit toolViewAdded(view);
}

void Controller::notifyViewRemoved(Sublime::AreaIndex*, Sublime::View *view)
{
    emit aboutToRemoveView(view);
}

void Controller::notifyViewAdded(Sublime::AreaIndex*, Sublime::View *view)
{
    emit viewAdded(view);
}

void Controller::setStatusIcon(Document * document, const QIcon & icon)
{
    document->setStatusIcon(icon);
}

}

#include "controller.moc"
