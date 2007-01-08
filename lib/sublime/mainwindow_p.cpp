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
#include "mainwindow_p.h"

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
#include "mainwindow.h"

namespace Sublime {

MainWindowPrivate::MainWindowPrivate(MainWindow *w)
    :m_mainWindow(w), controller(0), area(0), m_areaSwitcherMenu(0)
{
}

Area::WalkerMode MainWindowPrivate::ToolViewCreator::operator() (View *view, Sublime::Position position)
{
    QDockWidget *dock = new QDockWidget(view->document()->title(), d->m_mainWindow);
    d->docks.append(dock);
    KAcceleratorManager::setNoAccel(dock);

    dock->setWidget(view->widget());
    d->m_mainWindow->addDockWidget(d->positionToDockArea(position), dock);
    return Area::ContinueWalker;
}

Area::WalkerMode MainWindowPrivate::ViewCreator::operator() (AreaIndex *index)
{
    kDebug() << "  reconstructing views for area index " << index << endl;
    QSplitter *parent = 0;
    QSplitter *splitter;
    if (!index->parent())
    {
        kDebug() << "   reconstructing root area" << endl;
        //this is root area
        splitter = new QSplitter(d->m_mainWindow);
        d->m_indexSplitters[index] = splitter;
        d->m_mainWindow->setCentralWidget(splitter);
    }
    else
    {
        parent = d->m_indexSplitters[index->parent()];
        kDebug() << "adding new splitter to " << parent << endl;
        splitter = new QSplitter(parent);
        d->m_indexSplitters[index] = splitter;
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
    return Area::ContinueWalker;
}

void MainWindowPrivate::reconstruct()
{
    ToolViewCreator toolViewCreator(this);
    ViewCreator viewCreator(this);
    area->walkToolViews(toolViewCreator, Sublime::AllPositions);
    area->walkViews(viewCreator, area->rootIndex());
}

void MainWindowPrivate::clearArea()
{
    //reparent toolview widgets to 0 to prevent their deletion together with dockwidgets
    foreach (View *view, area->toolViews())
    {
        if (view->hasWidget())
            view->widget()->setParent(0);
    }
    foreach (QDockWidget *dock, docks)
    {
        m_mainWindow->removeDockWidget(dock);
        delete dock;
    }
    //reparent all view widgets to 0 to prevent their deletion together with central
    //widget. this reparenting is necessary when switching areas inside the same mainwindow
    foreach (View *view, area->views())
    {
        if (view->hasWidget())
            view->widget()->setParent(0);
    }
    m_mainWindow->setCentralWidget(0);
    docks.clear();
    m_indexSplitters.clear();
    area = 0;
}

Qt::DockWidgetArea MainWindowPrivate::positionToDockArea(Position position)
{
    switch (position)
    {
        case Sublime::Left: return Qt::LeftDockWidgetArea;
        case Sublime::Right: return Qt::RightDockWidgetArea;
        case Sublime::Bottom: return Qt::BottomDockWidgetArea;
        case Sublime::Top: return Qt::TopDockWidgetArea;
        default: return Qt::LeftDockWidgetArea;
    }
}

void MainWindowPrivate::switchToArea(QAction *action)
{
    kDebug() << k_funcinfo << " for " << action << endl;
    controller->showArea(m_actionAreas[action], m_mainWindow);
}

void MainWindowPrivate::updateAreaSwitcher(Sublime::Area *area)
{
    if (m_areaActions.contains(area))
        m_areaActions[area]->setChecked(true);
}

QMenu *MainWindowPrivate::areaSwitcherMenu()
{
    if (!m_areaSwitcherMenu)
    {
        m_areaSwitcherMenu = new QMenu("Areas", m_mainWindow);
        QActionGroup *group = new QActionGroup(m_mainWindow);
        group->setExclusive(true);
        kDebug() << "preparing area switcher menu" << endl;
        foreach (Area *a, controller->areas())
        {
            kDebug() << "   creating action for area " << a->objectName() << endl;
            QAction *action = m_areaSwitcherMenu->addAction(a->objectName());
            action->setCheckable(true);
            action->setActionGroup(group);
            if (a == area)
                action->setChecked(true);
            m_areaActions[a] = action;
            m_actionAreas[action] = a;
        }
        connect(m_areaSwitcherMenu, SIGNAL(triggered(QAction*)), this, SLOT(switchToArea(QAction*)));
        connect(this, SIGNAL(areaChanged(Sublime::Area*)), this, SLOT(updateAreaSwitcher(Sublime::Area*)));
    }

    return m_areaSwitcherMenu;
}

}

#include "mainwindow_p.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
