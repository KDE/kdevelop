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
    : QObject(parent), m_dontSendChanges(0)
{
    connect(breakpointModel(),
            SIGNAL(breakpointChanged(KDevelop::Breakpoint*,KDevelop::Breakpoint::Column)),
            SLOT(breakpointChanged(KDevelop::Breakpoint*,KDevelop::Breakpoint::Column)));
    connect(breakpointModel(), SIGNAL(breakpointDeleted(KDevelop::Breakpoint*)),
            SLOT(breakpointDeleted(KDevelop::Breakpoint*)));
    
    foreach (Breakpoint *breakpoint, breakpointModel()->breakpoints()) {
        m_dirty[breakpoint].insert(Breakpoint::LocationColumn);
        if (!breakpoint->condition().isEmpty()) {
            m_dirty[breakpoint].insert(Breakpoint::ConditionColumn);
        }
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
    foreach (Breakpoint *breakpoint, breakpointModel()->breakpoints()) {
        sendMaybe(breakpoint);
    }
}

void IBreakpointController::breakpointChanged(KDevelop::Breakpoint* breakpoint, KDevelop::Breakpoint::Column column)
{
    if (m_dontSendChanges) return;

    if (column != Breakpoint::StateColumn) {
        m_dirty[breakpoint].insert(column);
        if (m_errors.contains(breakpoint)) m_errors[breakpoint].remove(column);
        breakpointStateChanged(breakpoint);
        if (debugSession()->isRunning()) {
            sendMaybe(breakpoint);
        }
    }

    kDebug() << column << m_dirty;
}

void IBreakpointController::breakpointDeleted(KDevelop::Breakpoint* breakpoint)
{
    kDebug() << breakpoint;
    sendMaybe(breakpoint);
}

Breakpoint::BreakpointState IBreakpointController::breakpointState(const Breakpoint* breakpoint) const
{
    if (!m_dirty.contains(breakpoint) || m_dirty[breakpoint].isEmpty()) {
        if (m_pending.contains(breakpoint)) {
            return Breakpoint::PendingState;
        }
        return Breakpoint::CleanState;
    }
    return Breakpoint::DirtyState;
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
    if (breakpoint->deleted()) return;
    m_dontSendChanges++;
    breakpoint->reportChange(Breakpoint::StateColumn);
    m_dontSendChanges--;
}

void IBreakpointController::setHitCount(Breakpoint* breakpoint, int count)
{
    m_hitCount[breakpoint] = count;
    m_dontSendChanges++;
    breakpoint->reportChange(Breakpoint::HitCountColumn);
    m_dontSendChanges--;
}

void IBreakpointController::error(Breakpoint* breakpoint, const QString &msg, Breakpoint::Column column)
{
    m_dontSendChanges++;
    m_errorText.insert(breakpoint, msg);
    m_errors[breakpoint].insert(column);
    breakpoint->reportChange(column);
    breakpointModel()->errorEmit(breakpoint, msg, Breakpoint::LocationColumn);
    m_dontSendChanges--;
}

void IBreakpointController::hit(KDevelop::Breakpoint* breakpoint)
{
    kDebug() << breakpoint;
    breakpointModel()->hitEmit(breakpoint);
    debugSession()->demandAttention();
}

QSet<Breakpoint::Column> IBreakpointController::breakpointErrors(const Breakpoint* breakpoint) const
{
    if (!m_errors.contains(breakpoint)) return QSet<Breakpoint::Column>();
    return m_errors[breakpoint];
}

QString IBreakpointController::breakpointErrorText(const KDevelop::Breakpoint* breakpoint) const
{
    if (!m_errorText.contains(breakpoint)) return QString();
    return m_errorText[breakpoint];
}



}

#include "ibreakpointcontroller.moc"
