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
#include "mainwindow_p.h"

#include <QMenu>
#include <QLayout>
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
#include "ideal.h"
#include "idealcentralwidget.h"

namespace Sublime {

MainWindowPrivate::MainWindowPrivate(MainWindow *w)
    :controller(0), area(0), activeView(0), activeToolView(0), centralWidget(0),
    idealMainWidget(0), m_mainWindow(w), m_areaSwitcherMenu(0)
{
    idealMainWidget = new IdealMainWidget(m_mainWindow, m_mainWindow->actionCollection());
    m_mainWindow->setCentralWidget(idealMainWidget);
}

Area::WalkerMode MainWindowPrivate::IdealToolViewCreator::operator() (View *view, Sublime::Position position)
{
    if (!d->docks.contains(view))
    {
        d->docks << view;
        d->idealMainWidget->addView(d->positionToDockArea(position), view);
    }
    return Area::ContinueWalker;
}

void MainWindowPrivate::reconstruct()
{
    IdealToolViewCreator toolViewCreator(this);
    area->walkToolViews(toolViewCreator, Sublime::AllPositions);

    idealMainWidget->internalCentralWidget()->setArea(area);
}

void Sublime::MainWindowPrivate::raiseToolView(Sublime::View * view)
{
    idealMainWidget->raiseView(view);
}

void MainWindowPrivate::toolViewAdded(Sublime::View */*toolView*/, Sublime::Position position)
{
    IdealToolViewCreator toolViewCreator(this);
    area->walkToolViews(toolViewCreator, position);
}

void MainWindowPrivate::aboutToRemoveToolView(Sublime::View *toolView, Sublime::Position /*position*/)
{
    if (!docks.contains(toolView))
        return;

    idealMainWidget->removeView(toolView);
    // TODO are Views unique?
    docks.removeAll(toolView);
}

void MainWindowPrivate::toolViewMoved(
    Sublime::View *toolView, Sublime::Position position)
{
    if (!docks.contains(toolView))
        return;

    idealMainWidget->moveView(toolView, positionToDockArea(position));
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
    kDebug(9504) << "for" << action;
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
        kDebug(9504) << "preparing area switcher menu";
        foreach (Area *a, controller->areas())
        {
            kDebug(9504) << "creating action for area" << a->objectName();
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

void MainWindowPrivate::activateFirstVisibleView()
{
    if (area->views().count() > 0)
        m_mainWindow->activateView(area->views().first());
}

bool MainWindowPrivate::eventFilter(QObject *, QEvent *event)
{
    if (event->type() == QEvent::FocusIn)
        idealMainWidget->centralWidgetFocused();

    return false;
}

}

#include "mainwindow_p.moc"

