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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


#include "ibreakpointcontroller.h"

#include <KDE/KDebug>          // remove later
#include <KNotification>
#include <KLocalizedString>
#include <KGlobal>
#include <KParts/MainWindow>

#include "idebugsession.h"
#include "../../interfaces/icore.h"
#include "../breakpoint/breakpointmodel.h"
#include "../../interfaces/idebugcontroller.h"
#include "../breakpoint/breakpoint.h"
#include "../../interfaces/iuicontroller.h"
#include <KComponentData>

namespace KDevelop {

IBreakpointController::IBreakpointController(KDevelop::IDebugSession* parent)
    : QObject(parent), m_dontSendChanges(0)
{
    connect(breakpointModel(),
            SIGNAL(breakpointChanged(KDevelop::Breakpoint*,KDevelop::Breakpoint::Column)),
            SLOT(breakpointChanged(KDevelop::Breakpoint*,KDevelop::Breakpoint::Column)));
    connect(breakpointModel(), SIGNAL(breakpointDeleted(KDevelop::Breakpoint*)),
            SLOT(breakpointDeleted(KDevelop::Breakpoint*)));

    connect(parent, SIGNAL(stateChanged(KDevelop::IDebugSession::DebuggerState)),
             SLOT(debuggerStateChanged(KDevelop::IDebugSession::DebuggerState)));
}

IDebugSession* IBreakpointController::debugSession() const
{
    return static_cast<IDebugSession*>(const_cast<QObject*>(QObject::parent()));
}

BreakpointModel* IBreakpointController::breakpointModel() const
{
    if (!ICore::self()) return 0;
    return ICore::self()->debugController()->breakpointModel();
}

void IBreakpointController::debuggerStateChanged(IDebugSession::DebuggerState state)
{
    BreakpointModel* model = breakpointModel();
    if (!model)
        return;

    if (state == IDebugSession::StartingState || state == IDebugSession::EndedState) {
        //breakpoint state changes when session started or stopped
        foreach (Breakpoint *breakpoint, model->breakpoints()) {
            if (state == IDebugSession::StartingState) {
                //when starting everything is dirty
                m_dirty[breakpoint].insert(Breakpoint::LocationColumn);
                if (!breakpoint->condition().isEmpty()) {
                    m_dirty[breakpoint].insert(Breakpoint::ConditionColumn);
                }
                if (!breakpoint->enabled()) {
                	m_dirty[breakpoint].insert(KDevelop::Breakpoint::EnableColumn);
                }
            }
            breakpointStateChanged(breakpoint);
        }
    }
}

void IBreakpointController::sendMaybeAll()
{
    BreakpointModel* model = breakpointModel();
    if (!model)
        return;

    foreach (Breakpoint *breakpoint, model->breakpoints()) {
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
    if (debugSession()->state() != IDebugSession::ActiveState
        && debugSession()->state() != IDebugSession::PausedState
    ) {
        return Breakpoint::NotStartedState;
    }
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
    breakpoint->reportChange(Breakpoint::StateColumn);
    breakpointModel()->errorEmit(breakpoint, msg, Breakpoint::LocationColumn);
    m_dontSendChanges--;
}

void IBreakpointController::hit(KDevelop::Breakpoint* breakpoint, const QString &msg)
{
    kDebug() << breakpoint;
    breakpointModel()->hitEmit(breakpoint);

    KNotification* ev = 0;
    switch(breakpoint->kind()) {
        case Breakpoint::CodeBreakpoint:
            ev = new KNotification("BreakpointHit", ICore::self()->uiController()->activeMainWindow());
            ev->setText(i18n("Breakpoint hit: %1", breakpoint->location()) + msg);
            break;
        case Breakpoint::WriteBreakpoint:
        case Breakpoint::ReadBreakpoint:
        case Breakpoint::AccessBreakpoint:
            ev = new KNotification("WatchpointHit", ICore::self()->uiController()->activeMainWindow());
            ev->setText(i18n("Watchpoint hit: %1", breakpoint->location()) + msg);
            break;
        default:
            Q_ASSERT(0);
            break;
    }
    if (ev) {
        ev->setPixmap(QIcon::fromTheme("script-error").pixmap(QSize(22,22)));
        ev->setComponentName(ICore::self()->componentData().componentName());
        ev->sendEvent();
    }
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

