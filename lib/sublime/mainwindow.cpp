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

#include <QMenu>
#include <QSplitter>
#include <QDockWidget>

#include <kdebug.h>
#include <kacceleratormanager.h>

#include "area.h"
#include "view.h"
#include "areaindex.h"
#include "document.h"
#include "container.h"
#include "controller.h"

namespace Sublime {

// struct MainWindowPrivate

struct MainWindowPrivate {
    MainWindowPrivate(MainWindow *w)
        :mainWindow(w), controller(0), area(0), areaSwitcherMenu(0)
    {
    }

    /**Use this to create toolviews for an area.*/
    class ToolViewCreator {
    public:
        ToolViewCreator(MainWindowPrivate *_d): d(_d) {}
        bool operator() (View *view, Sublime::Position position)
        {
            QDockWidget *dock = new QDockWidget(view->document()->title(), d->mainWindow);
            d->docks.append(dock);
            KAcceleratorManager::setNoAccel(dock);

            dock->setWidget(view->widget());
            d->mainWindow->addDockWidget(d->positionToDockArea(position), dock);
            return false;
        }
    private:
        MainWindowPrivate *d;
    };

    /**Use this to create views for an area.*/
    class ViewCreator {
    public:
        ViewCreator(MainWindowPrivate *_d): d(_d) {}
        bool operator() (AreaIndex *index)
        {
            kDebug() << "  reconstructing views for area index " << index << endl;
            QSplitter *parent = 0;
            QSplitter *splitter;
            if (!index->parent())
            {
                kDebug() << "   reconstructing root area" << endl;
                //this is root area
                splitter = new QSplitter(d->mainWindow);
                d->indexSplitters[index] = splitter;
                d->mainWindow->setCentralWidget(splitter);
            }
            else
            {
                parent = d->indexSplitters[index->parent()];
                kDebug() << "adding new splitter to " << parent << endl;
                splitter = new QSplitter(parent);
                d->indexSplitters[index] = splitter;
                parent->addWidget(splitter);
            }

            if (index->first() || index->second()) //this is a splitter with views
                splitter->setOrientation(index->orientation());
            else
            {
                //this is a view container
                Container *container = new Container(splitter);
                splitter->addWidget(container);
                foreach (View *view, index->views())
                    container->addWidget(view->widget());
            }
            return false;
        }
    private:
        MainWindowPrivate *d;
    };

    void reconstruct()
    {
        ToolViewCreator toolViewCreator(this);
        ViewCreator viewCreator(this);
        area->walkToolViews(toolViewCreator, Sublime::AllPositions);
        area->walkViews(viewCreator, area->rootIndex());
    }

    void clearArea()
    {
        foreach (QDockWidget *dock, docks)
        {
            mainWindow->removeDockWidget(dock);
            //@todo adymo: we might need to delete dock
            // delete dock;
        }
        //@todo adymo: we need to prevent view widgets deletion together with central widget
        mainWindow->setCentralWidget(0);
        docks.clear();
        indexSplitters.clear();
        area = 0;
    }

    Qt::DockWidgetArea positionToDockArea(Position position)
    {
        switch (position)
        {
            case Sublime::Left: return Qt::LeftDockWidgetArea;
            case Sublime::Right: return Qt::RightDockWidgetArea;
            case Sublime::Bottom: return Qt::BottomDockWidgetArea;
            case Sublime::Top: return Qt::TopDockWidgetArea;
        }
    }

    MainWindow *mainWindow;
    Area *area;
    Controller *controller;
    QMap<AreaIndex*, QSplitter*> indexSplitters;

    QMap<Area*, QAction*> areaActions;
    QMap<QAction*, Area*> actionAreas;
    QMenu *areaSwitcherMenu;

    QList<QDockWidget*> docks;

    QMap<Document*, QWidget*> widgetCache;
};

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
    {
        emit areaCleared(d->area);
        clearArea();
    }
    d->area = area;
    d->reconstruct();
    emit areaChanged(area);
}

void MainWindow::clearArea()
{
    d->clearArea();
    kDebug() << "area cleared" << endl;
}

QMenu *MainWindow::areaSwitcherMenu()
{
    if (!d->areaSwitcherMenu)
    {
        d->areaSwitcherMenu = new QMenu("Areas", this);
        QActionGroup *group = new QActionGroup(this);
        group->setExclusive(true);
        kDebug() << "preparing area switcher menu" << endl;
        foreach (Area *area, d->controller->areas())
        {
            kDebug() << "   creating action for area " << area->objectName() << endl;
            QAction *action = d->areaSwitcherMenu->addAction(area->objectName());
            action->setCheckable(true);
            action->setActionGroup(group);
            if (area == d->area)
                action->setChecked(true);
            d->areaActions[area] = action;
            d->actionAreas[action] = area;
        }
        connect(d->areaSwitcherMenu, SIGNAL(triggered(QAction*)), this, SLOT(switchToArea(QAction*)));
        connect(this, SIGNAL(areaChanged(Sublime::Area*)), this, SLOT(updateAreaSwitcher(Sublime::Area*)));
    }

    return d->areaSwitcherMenu;
}

void MainWindow::switchToArea(QAction *action)
{
    kDebug() << k_funcinfo << " for " << action << endl;
    d->controller->showArea(d->actionAreas[action], this);
}

void MainWindow::updateAreaSwitcher(Sublime::Area *area)
{
    if (d->areaActions.contains(area))
        d->areaActions[area]->setChecked(true);
}

}

#include "mainwindow.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
