/*
    Copyright David Nolden  <david.nolden.kdevelop@art-master.de>
    Copyright 2010 Milian Wolff <mail@milianw.de>

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

#include <KDebug>

#include <sublime/area.h>

#include "workingsetcontroller.h"
#include "workingset.h"
#include "workingsettoolbutton.h"

#include <core.h>

using namespace KDevelop;

WorkingSet* getSet(const QString& id)
{
    if (id.isEmpty()) {
        return 0;
    }

    return Core::self()->workingSetControllerInternal()->getWorkingSet(id);
}

WorkingSetWidget::WorkingSetWidget(Sublime::Area* area, QWidget* parent)
    : WorkingSetToolButton(parent, 0)
    , m_area(area)
{
    //Queued connect so the change is already applied to the area when we start processing
    connect(m_area, SIGNAL(changingWorkingSet(Sublime::Area*,QString,QString)), this,
            SLOT(changingWorkingSet(Sublime::Area*,QString,QString)), Qt::QueuedConnection);

    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored));

    changingWorkingSet(m_area, QString(), area->workingSet());
}

void WorkingSetWidget::setVisible( bool visible )
{
    // never show empty working sets
    // TODO: I overloaded this only because hide() in the ctor does not work, other ideas?
    QWidget::setVisible( visible && (workingSet() && !workingSet()->isEmpty()) );
}

void WorkingSetWidget::changingWorkingSet( Sublime::Area* area, const QString& /*from*/, const QString& newSet)
{
    kDebug() << "re-creating widget" << m_area;

    Q_ASSERT(area == m_area);
    Q_UNUSED(area);

    if (workingSet()) {
        disconnect(workingSet(), SIGNAL(setChangedSignificantly()),
                   this, SLOT(setChangedSignificantly()));
    }

    if (newSet.isEmpty()) {
        setWorkingSet(0);
        setVisible(false);
    } else {
        setWorkingSet(getSet(newSet));
        connect(workingSet(), SIGNAL(setChangedSignificantly()),
                   this, SLOT(setChangedSignificantly()));
        setVisible(!workingSet()->isEmpty());
    }
}

void WorkingSetWidget::setChangedSignificantly()
{
    if (workingSet()->isEmpty()) {
        setVisible(false);
    }
}

#include "workingsetwidget.moc"
