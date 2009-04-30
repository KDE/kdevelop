/* This file is part of the KDE project
   Copyright (C) 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2002 John Firebaugh <jfirebaugh@kde.org>
   Copyright (C) 2006, 2008 Vladimir Prus <ghost@cs.msu.su>
   Copyright (C) 2007 Hamish Rodda <rodda@kde.org>
   Copyright (C) 2009 Niko Sams <niko.sams@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/


#include "ibreakpointcontroller.h"

#include <KDE/KDebug>          // remove later

#include "idebugsession.h"
#include "../../interfaces/icore.h"
#include "../breakpoint/breakpointmodel.h"
#include "../../interfaces/idebugcontroller.h"
#include "../breakpoint/breakpoint.h"

namespace KDevelop {

IBreakpointController::IBreakpointController(KDevelop::IDebugSession* parent)
    : QObject(parent), m_dontSendChanges(false)
{
    connect(breakpointModel(),
            SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(dataChanged(QModelIndex,QModelIndex)));
    connect(breakpointModel(), SIGNAL(breakpointDeleted(KDevelop::Breakpoint*)), SLOT(breakpointDeleted(KDevelop::Breakpoint*)));
    
    Breakpoints *breakpoints = breakpointModel()->breakpointsItem();
    for (int i=0; i < breakpoints->breakpointCount(); ++i) {
        Breakpoint *breakpoint = breakpoints->breakpoint(i);
        m_dirty[breakpoint].insert(Breakpoint::LocationColumn);
        m_dirty[breakpoint].insert(Breakpoint::ConditionColumn);
        breakpointStateChanged(breakpoint);
    }
}

IDebugSession* IBreakpointController::debugSession() const
{
    return static_cast<IDebugSession*>(const_cast<QObject*>(QObject::parent()));
}

BreakpointModel* IBreakpointController::breakpointModel() const
{
    return ICore::self()->debugController()->breakpointModel();
}

void IBreakpointController::sendMaybeAll()
{
    Breakpoints *breakpoints = breakpointModel()->breakpointsItem();
    kDebug() << breakpoints->breakpointCount();
    for (int i=0; i < breakpoints->breakpointCount(); ++i) {
        Breakpoint *breakpoint = breakpoints->breakpoint(i);
        if (breakpoint->pleaseEnterLocation()) continue;
        sendMaybe(breakpoint);
    }
}

void IBreakpointController::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (m_dontSendChanges) return;

    kDebug() << debugSession()->state();

    Q_ASSERT(topLeft.parent() == bottomRight.parent());
    Q_ASSERT(topLeft.row() == bottomRight.row());
    Q_ASSERT(topLeft.column() == bottomRight.column());
    KDevelop::Breakpoint *b = breakpointModel()->breakpointsItem()->breakpoint(topLeft.row());
    if (topLeft.column() != Breakpoint::StateColumn) {
        m_dirty[b].insert(topLeft.column());
        breakpointStateChanged(b);
        if (debugSession()->isRunning()) {
            sendMaybe(b);
        }
    }
    kDebug() << topLeft.column() << m_dirty;
}

void IBreakpointController::breakpointDeleted(KDevelop::Breakpoint* breakpoint)
{
    kDebug() << breakpoint;
    sendMaybe(breakpoint);
}

IBreakpointController::BreakpointState IBreakpointController::breakpointState(const Breakpoint* breakpoint) const
{
    if (!m_dirty.contains(breakpoint) || m_dirty[breakpoint].isEmpty()) {
        if (m_pending.contains(breakpoint)) {
            return PendingState;
        }
        return CleanState;
    }
    return DirtyState;
}

int IBreakpointController::breakpointHitCount(const KDevelop::Breakpoint* breakpoint) const
{
    if (m_hitCount.contains(breakpoint)) {
        return m_hitCount[breakpoint];
    }
    return 0;
}

void IBreakpointController::breakpointStateChanged(Breakpoint* breakpoint)
{
    if (breakpoint->pleaseEnterLocation()) return;
    if (breakpoint->deleted()) return;
    m_dontSendChanges = true;
    breakpoint->reportChange(Breakpoint::StateColumn);
    m_dontSendChanges = false;
}

void IBreakpointController::setHitCount(Breakpoint* breakpoint, int count)
{
    m_hitCount[breakpoint] = count;
    m_dontSendChanges = true;
    breakpoint->reportChange();
    m_dontSendChanges = false;
}

}

#include "ibreakpointcontroller.moc"
