/*
    Copyright David Nolden  <david.nolden.kdevelop@art-master.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "workingsetwidget.h"

#include <QHBoxLayout>
#include <QToolButton>

#include <KDebug>

#include <sublime/area.h>

#include "mainwindow.h"

#include "workingsetcontroller.h"
#include "workingset.h"
#include "workingsettoolbutton.h"

using namespace KDevelop;

WorkingSetWidget::WorkingSetWidget(MainWindow* parent, WorkingSetController* controller, bool mini,
                                   Sublime::Area* fixedArea)
    : QWidget(0), m_fixedArea(fixedArea), m_mini(mini), m_mainWindow(parent)
{
    m_layout = new QHBoxLayout(this);
    m_layout->setMargin(0);
    if(!m_fixedArea)
        connect(parent, SIGNAL(areaChanged(Sublime::Area*)), this, SLOT(areaChanged(Sublime::Area*)));

    connect(controller, SIGNAL(workingSetAdded(QString)), this, SLOT(workingSetsChanged()));
    connect(controller, SIGNAL(workingSetRemoved(QString)), this, SLOT(workingSetsChanged()));

    Sublime::Area* area = parent->area();
    if(m_fixedArea)
        area = m_fixedArea;
    if(area)
        areaChanged(area);

    workingSetsChanged();
}

void WorkingSetWidget::areaChanged(Sublime::Area* area)
{
    if(m_connectedArea) {
        disconnect(m_connectedArea, SIGNAL(changingWorkingSet(Sublime::Area*, QString, QString)),
                   this, SLOT(changingWorkingSet(Sublime::Area*, QString, QString)));
    }

    //Queued connect so the change is already applied to the area when we start processing
    connect(area, SIGNAL(changingWorkingSet(Sublime::Area*, QString, QString)), this,
            SLOT(changingWorkingSet(Sublime::Area*, QString, QString)), Qt::QueuedConnection);

    m_connectedArea = area;

    changingWorkingSet(area, QString(), area->workingSet());
}

void WorkingSetWidget::changingWorkingSet( Sublime::Area* /*area*/, const QString& /*from*/, const QString& /*to*/)
{
    workingSetsChanged();
}

void WorkingSetWidget::workingSetsChanged()
{
    kDebug() << "re-creating widget" << m_connectedArea << m_fixedArea << m_mini;
    foreach(QToolButton* button, m_buttons.keys())
        delete button;
    m_buttons.clear();

    foreach(WorkingSet* set, Core::self()->workingSetControllerInternal()->allWorkingSets()) {

        disconnect(set, SIGNAL(setChangedSignificantly()), this, SLOT(workingSetsChanged()));
        connect(set, SIGNAL(setChangedSignificantly()), this, SLOT(workingSetsChanged()));

        if(m_mini && set->id() != m_connectedArea->workingSet()) {
//             kDebug() << "skipping" << set->id() << ", searching" << m_connectedArea->workingSet();
            continue; //In "mini" mode, show only the current working set
        }
        if(set->isEmpty()) {
//             kDebug() << "skipping" << set->id() << "because empty";
            continue;
        }

        // Don't show working-sets that are active in an area belong to this main-window, as those
        // can be activated directly through the icons in the tabs
        if(!m_mini && set->hasConnectedAreas(m_mainWindow->areas()))
             continue;
        
//         kDebug() << "adding button for" << set->id();
        QToolButton* butt = new WorkingSetToolButton(this, set, m_mainWindow);
        butt->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored));

        m_layout->addWidget(butt);
        m_buttons[butt] = set;
    }
    update();
}

#include "workingsetwidget.moc"
