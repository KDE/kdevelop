/*
    SPDX-FileCopyrightText: 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2002 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2006, 2008 Vladimir Prus <ghost@cs.msu.su>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/


#include "ibreakpointcontroller.h"

#include <KNotification>
#include <KLocalizedString>
#include <KParts/MainWindow>

#include "idebugsession.h"
#include "../../interfaces/icore.h"
#include "../breakpoint/breakpointmodel.h"
#include "../../interfaces/idebugcontroller.h"
#include "../breakpoint/breakpoint.h"
#include "../../interfaces/iuicontroller.h"
#include <debug.h>

#include <QApplication>

namespace KDevelop {

IBreakpointController::IBreakpointController(KDevelop::IDebugSession* parent)
    : QObject(parent), m_dontSendChanges(0)
{
    connect(parent, &IDebugSession::stateChanged,
             this, &IBreakpointController::debuggerStateChanged);
}

IDebugSession* IBreakpointController::debugSession() const
{
    return static_cast<IDebugSession*>(const_cast<QObject*>(QObject::parent()));
}

BreakpointModel* IBreakpointController::breakpointModel() const
{
    if (!ICore::self()) return nullptr;
    return ICore::self()->debugController()->breakpointModel();
}

void IBreakpointController::updateState(int row, Breakpoint::BreakpointState state)
{
    auto* const modelBreakpoint = breakpointModel()->breakpoint(row);
    Q_ASSERT(modelBreakpoint);
    modelBreakpoint->setState(state);
}

void IBreakpointController::updateHitCount(int row, int hitCount)
{
    auto* const modelBreakpoint = breakpointModel()->breakpoint(row);
    Q_ASSERT(modelBreakpoint);
    modelBreakpoint->setHitCount(hitCount);
}

void IBreakpointController::updateErrorText(int row, const QString& errorText)
{
    breakpointModel()->updateErrorText(row, errorText);

    if (errorText.isEmpty()) {
        return;
    }

    auto* const errorNotification = new KNotification(QStringLiteral("BreakpointError"));
    errorNotification->setText(errorText);
    errorNotification->sendEvent();
}

void IBreakpointController::notifyHit(int row, const QString& msg)
{
    BreakpointModel* model = breakpointModel();
    model->notifyHit(row);

    // This is a slightly odd place to issue this notification,
    // but then again it's not clear which place would be more natural
    Breakpoint* breakpoint = model->breakpoint(row);
    KNotification* ev = nullptr;
    switch(breakpoint->kind()) {
        case Breakpoint::CodeBreakpoint:
            ev = new KNotification(QStringLiteral("BreakpointHit"));
            ev->setText(i18n("Breakpoint hit: %1", breakpoint->location()) + msg);
            break;
        case Breakpoint::WriteBreakpoint:
        case Breakpoint::ReadBreakpoint:
        case Breakpoint::AccessBreakpoint:
            ev = new KNotification(QStringLiteral("WatchpointHit"));
            ev->setText(i18n("Watchpoint hit: %1", breakpoint->location()) + msg);
            break;
        default:
            Q_ASSERT(0);
            break;
    }
    if (ev) {
        ev->setPixmap(QIcon::fromTheme(QStringLiteral("breakpoint")).pixmap(QSize(22,22)));
        // TODO: Port
        //ev->setComponentName(ICore::self()->aboutData().componentName());
        ev->sendEvent();
    }

    QApplication::alert(ICore::self()->uiController()->activeMainWindow());
}

// Temporary: empty default implementation
void IBreakpointController::breakpointAdded(int row)
{
    Q_UNUSED(row);
}

// Temporary: implement old-style behavior to ease transition through API changes
void IBreakpointController::breakpointModelChanged(int row, BreakpointModel::ColumnFlags columns)
{
    if (m_dontSendChanges)
        return;

    if ((columns & ~BreakpointModel::StateColumnFlag) != 0) {
        Breakpoint * breakpoint = breakpointModel()->breakpoint(row);
        for (int column = 0; column < BreakpointModel::NumColumns; ++column) {
            if (columns & (1 << column)) {
                m_dirty[breakpoint].insert(Breakpoint::Column(column));
                auto errorIt = m_errors.find(breakpoint);
                if (errorIt != m_errors.end()) {
                    errorIt->remove(Breakpoint::Column(column));
                }
            }
        }
        breakpointStateChanged(breakpoint);
        if (debugSession()->isRunning()) {
            sendMaybe(breakpoint);
        }
    }
}

void IBreakpointController::debuggerStateChanged(IDebugSession::DebuggerState state)
{
    BreakpointModel* model = breakpointModel();
    if (!model)
        return;

    //breakpoint state changes when session started or stopped
    const auto breakpoints = model->breakpoints();
    for (Breakpoint* breakpoint : breakpoints) {
        if (state == IDebugSession::StartingState) {
            auto& dirty = m_dirty[breakpoint];

            //when starting everything is dirty
            dirty.insert(Breakpoint::LocationColumn);
            if (!breakpoint->condition().isEmpty()) {
                dirty.insert(Breakpoint::ConditionColumn);
            }
            if (!breakpoint->enabled()) {
                dirty.insert(KDevelop::Breakpoint::EnableColumn);
            }
        }
        breakpointStateChanged(breakpoint);
    }
}

void IBreakpointController::sendMaybeAll()
{
    BreakpointModel* model = breakpointModel();
    if (!model)
        return;

    const auto breakpoints = model->breakpoints();
    for (Breakpoint* breakpoint : breakpoints) {
        sendMaybe(breakpoint);
    }
}

// Temporary implementation to ease the API transition
void IBreakpointController::breakpointAboutToBeDeleted(int row)
{
    Breakpoint* breakpoint = breakpointModel()->breakpoint(row);
    qCDebug(DEBUGGER) << "breakpointAboutToBeDeleted(" << row << "): " << breakpoint;
    sendMaybe(breakpoint);
}

void IBreakpointController::breakpointStateChanged(Breakpoint* breakpoint)
{
    if (breakpoint->deleted()) return;

    Breakpoint::BreakpointState newState = Breakpoint::NotStartedState;
    if (debugSession()->state() != IDebugSession::EndedState &&
        debugSession()->state() != IDebugSession::NotStartedState)
    {
        if (m_dirty.value(breakpoint).isEmpty()) {
            if (m_pending.contains(breakpoint)) {
                newState = Breakpoint::PendingState;
            } else {
                newState = Breakpoint::CleanState;
            }
        } else {
            newState = Breakpoint::DirtyState;
        }
    }

    m_dontSendChanges++;
    updateState(breakpointModel()->breakpointIndex(breakpoint, 0).row(), newState);
    m_dontSendChanges--;
}

void IBreakpointController::setHitCount(Breakpoint* breakpoint, int count)
{
    m_dontSendChanges++;
    updateHitCount(breakpointModel()->breakpointIndex(breakpoint, 0).row(), count);
    m_dontSendChanges--;
}

void IBreakpointController::error(Breakpoint* breakpoint, const QString &msg, Breakpoint::Column column)
{
    BreakpointModel* model = breakpointModel();
    int row = model->breakpointIndex(breakpoint, 0).row();

    m_dontSendChanges++;
    m_errors[breakpoint].insert(column);
    updateErrorText(row, msg);
    m_dontSendChanges--;
}

void IBreakpointController::hit(KDevelop::Breakpoint* breakpoint, const QString &msg)
{
    int row = breakpointModel()->breakpointIndex(breakpoint, 0).row();
    notifyHit(row, msg);
}

}

#include "moc_ibreakpointcontroller.cpp"
