/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2010 Milian Wolff <mail@milianw.de>                         *
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


#include "closedworkingsetswidget.h"

#include <sublime/area.h>

#include <shell/core.h>

#include "mainwindow.h"
#include "workingsetcontroller.h"

#include "workingset.h"
#include "workingsettoolbutton.h"
#include "../debug.h"

using namespace KDevelop;

WorkingSet* getWorkingSet(const QString& id)
{
    return Core::self()->workingSetControllerInternal()->getWorkingSet(id);
}

ClosedWorkingSetsWidget::ClosedWorkingSetsWidget( MainWindow* window )
    : QWidget(0), m_mainWindow(window)
{
    connect(window, SIGNAL(areaChanged(Sublime::Area*)),
            this, SLOT(areaChanged(Sublime::Area*)));

    m_layout = new QHBoxLayout(this);
    m_layout->setMargin(0);

    if (window->area()) {
        areaChanged(window->area());
    }

    connect(Core::self()->workingSetControllerInternal(), SIGNAL(aboutToRemoveWorkingSet(WorkingSet*)),
            this, SLOT(removeWorkingSet(WorkingSet*)));

    connect(Core::self()->workingSetControllerInternal(), SIGNAL(workingSetAdded(WorkingSet*)),
            this, SLOT(addWorkingSet(WorkingSet*)));
}

void ClosedWorkingSetsWidget::areaChanged( Sublime::Area* area )
{
    if (m_connectedArea) {
        disconnect(area, SIGNAL(changedWorkingSet(Sublime::Area*,QString,QString)),
                   this, SLOT(changedWorkingSet(Sublime::Area*,QString,QString)));
    }

    m_connectedArea = area;
    connect(m_connectedArea, SIGNAL(changedWorkingSet(Sublime::Area*,QString,QString)),
            this, SLOT(changedWorkingSet(Sublime::Area*,QString,QString)));

    // clear layout
    qDeleteAll(m_buttons.values());
    m_buttons.clear();

    // add sets from new area
    foreach(WorkingSet* set, Core::self()->workingSetControllerInternal()->allWorkingSets()) {
        addWorkingSet(set);
    }
}

void ClosedWorkingSetsWidget::changedWorkingSet( Sublime::Area* area, const QString& from, const QString& to )
{
    Q_ASSERT(area == m_connectedArea);
    Q_UNUSED(area);

    if (!from.isEmpty()) {
        WorkingSet* oldSet = getWorkingSet(from);
        addWorkingSet(oldSet);
    }

    if (!to.isEmpty()) {
        WorkingSet* newSet = getWorkingSet(to);
        removeWorkingSet(newSet);
    }
}

void ClosedWorkingSetsWidget::removeWorkingSet( WorkingSet* set )
{
    delete m_buttons.take(set);
    Q_ASSERT(m_buttons.size() == m_layout->count());
    setVisible(!m_buttons.isEmpty());
}

void ClosedWorkingSetsWidget::addWorkingSet( WorkingSet* set )
{
    if (m_buttons.contains(set)) {
        return;
    }

    // Don't show working-sets that are active in an area belong to this main-window, as those
    // can be activated directly through the icons in the tabs
    if (set->hasConnectedAreas(m_mainWindow->areas())) {
        return;
    }

    if (set->isEmpty()) {
//             qCDebug(SHELL) << "skipping" << set->id() << "because empty";
        return;
    }

//     qCDebug(SHELL) << "adding button for" << set->id();
    WorkingSetToolButton* button = new WorkingSetToolButton(this, set);
    button->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored));

    m_layout->addWidget(button);
    m_buttons[set] = button;
    setVisible(!m_buttons.isEmpty());
}

