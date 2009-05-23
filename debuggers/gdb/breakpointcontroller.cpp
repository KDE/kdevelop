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

#include "breakpointcontroller.h"

#include <KDebug>

#include <interfaces/icore.h>
#include <interfaces/idebugcontroller.h>
#include <debugger/breakpoint/breakpointmodel.h>
#include <debugger/breakpoint/breakpoint.h>

#include "gdbcontroller.h"
#include "gdbcommand.h"
#include "debugsession.h"
#include <KLocalizedString>

using namespace GDBMI;

namespace GDBDebugger {

struct Handler : public QObject
{
    Handler(BreakpointController *c, KDevelop::Breakpoint *b)
        : controller(c), breakpoint(b) {}

    BreakpointController *controller;
    KDevelop::Breakpoint *breakpoint;
};

struct UpdateHandler : public Handler
{
    UpdateHandler(BreakpointController *c, KDevelop::Breakpoint *b, KDevelop::Breakpoint::Column col)
        : Handler(c, b), m_column(col) {}

    void handle(const GDBMI::ResultRecord &r)
    {
        if (r.reason == "error") {
            controller->error(breakpoint, r["msg"].literal(), m_column);
            kWarning() << r["msg"].literal();
        } else {
            controller->m_errors[breakpoint].remove(m_column);
        }
        controller->m_dirty[breakpoint].remove(m_column);
        controller->breakpointStateChanged(breakpoint);
        controller->sendMaybe(breakpoint);
        delete this;
    }
private:
    KDevelop::Breakpoint::Column m_column;
};

struct InsertedHandler : public Handler
{
    InsertedHandler(BreakpointController *c, KDevelop::Breakpoint *b)
        : Handler(c, b) {}

    void handle(const GDBMI::ResultRecord &r)
    {
        kDebug() << controller->m_dirty[breakpoint];

        if (r.reason == "error") {
            controller->error(breakpoint, r["msg"].literal(), KDevelop::Breakpoint::LocationColumn);
            kWarning() << r["msg"].literal();
        } else {
            controller->m_errors[breakpoint].remove(KDevelop::Breakpoint::LocationColumn);
            if (r.hasField("bkpt")) {
                controller->update(breakpoint, r["bkpt"]);
            } else if (r.hasField("wpt")) {
                // For watchpoint creation, GDB basically does not say
                // anything.  Just record id.
                controller->m_ids[breakpoint] = r["wpt"]["number"].toInt();
            } else if (r.hasField("hw-rwpt")) {
                controller->m_ids[breakpoint] = r["hw-rwpt"]["number"].toInt();
            } else if (r.hasField("hw-awpt")) {
                controller->m_ids[breakpoint] = r["hw-awpt"]["number"].toInt();
            }
        }
        controller->m_dirty[breakpoint].remove(KDevelop::Breakpoint::LocationColumn);
        controller->breakpointStateChanged(breakpoint);
        controller->sendMaybe(breakpoint);
        delete this;
    }
};

struct DeletedHandler : public Handler
{
    DeletedHandler(BreakpointController *c, KDevelop::Breakpoint *b)
        : Handler(c, b) {}

    void handle(const GDBMI::ResultRecord &r)
    {
        Q_UNUSED(r);
        controller->m_ids.remove(breakpoint);
        if (!breakpoint->deleted()) {
            controller->sendMaybe(breakpoint);
        } else {
            delete breakpoint;
        }
        delete this;
    }
};


BreakpointController::BreakpointController(DebugSession* parent)
    : KDevelop::IBreakpointController(parent), m_interrupted(false)
{
    Q_ASSERT(parent);
    // FIXME: maybe, all debugger components should derive from
    // a base class that does this connect.
    connect(controller(),     SIGNAL(event(event_t)),
            this,       SLOT(slotEvent(event_t)));
    connect(parent, SIGNAL(programStopped(GDBMI::ResultRecord)), SLOT(programStopped(GDBMI::ResultRecord)));
}

DebugSession *BreakpointController::debugSession() const
{
    Q_ASSERT(QObject::parent());
    return static_cast<DebugSession*>(const_cast<QObject*>(QObject::parent()));
}

GDBController * BreakpointController::controller() const
{
    Q_ASSERT(debugSession());
    return debugSession()->controller();
}

void BreakpointController::slotEvent(event_t e)
{
    switch(e) {
        case program_state_changed:
            if (m_interrupted) {
                m_interrupted = false;
                controller()->addCommand(ExecContinue);
            } else {
                controller()->addCommand(
                    new GDBCommand(GDBMI::BreakList,
                                "",
                                this,
                                &BreakpointController::handleBreakpointList));
            }
            break;

        case connected_to_program:
        {
            kDebug() << "connected to program";
            sendMaybeAll();
            break;
        }
        case debugger_exited:
        {
            break;
        }
        default:
            break;
    }
}

void BreakpointController::sendMaybe(KDevelop::Breakpoint* breakpoint)
{
    if (controller()->stateIsOn(s_dbgNotStarted)) {
        return;
    }

    bool addedCommand = false;

    /** See what is dirty, and send the changes.  For simplicity, send
        changes one-by-one and call sendToGDB again in the completion
        handler.
        FIXME: should handle and annotate the errors?
    */
    if (breakpoint->deleted())
    {
        m_dirty.remove(breakpoint);
        m_errors.remove(breakpoint);
        if (m_ids.contains(breakpoint)) {
            DeletedHandler *handler = new DeletedHandler(this, breakpoint);
            controller()->addCommand(
                new GDBCommand(BreakDelete, QString::number(m_ids[breakpoint]),
                               handler, &DeletedHandler::handle));
            addedCommand = true;
        } else {
            delete breakpoint;
        }
    }
    else if (m_dirty[breakpoint].contains(KDevelop::Breakpoint::LocationColumn)) {
        if (!breakpoint->enabled()) {
            m_dirty[breakpoint].clear();
            breakpointStateChanged(breakpoint);
        } else {
            if (m_ids.contains(breakpoint)) {
                /* We already have GDB breakpoint for this, so we need to remove
                this one.  */
                DeletedHandler *handler = new DeletedHandler(this, breakpoint);
                controller()->addCommand(
                    new GDBCommand(BreakDelete, QString::number(m_ids[breakpoint]),
                                handler, &DeletedHandler::handle));
                addedCommand = true;
            } else {
                if (breakpoint->kind() == KDevelop::Breakpoint::CodeBreakpoint) {
                    InsertedHandler *handler = new InsertedHandler(this, breakpoint);
                    controller()->addCommand(
                        new GDBCommand(BreakInsert,
                                    breakpoint->location(),
                                    handler, &InsertedHandler::handle, true));
                    addedCommand = true;
                } else {
                    InsertedHandler *handler = new InsertedHandler(this, breakpoint);
                    QString opt;
                    if (breakpoint->kind() == KDevelop::Breakpoint::ReadBreakpoint)
                        opt = "-r ";
                    else if (breakpoint->kind() == KDevelop::Breakpoint::AccessBreakpoint)
                        opt = "-a ";

                    controller()->addCommand(
                        new GDBCommand(
                            BreakWatch,
                            opt + breakpoint->location(),
                            handler, &InsertedHandler::handle, true));
                    addedCommand = true;
                }
            }
        }
    } else if (m_dirty[breakpoint].contains(KDevelop::Breakpoint::EnableColumn)) {
        Q_ASSERT(m_ids.contains(breakpoint));
        UpdateHandler *handler = new UpdateHandler(this, breakpoint, KDevelop::Breakpoint::EnableColumn);
        controller()->addCommandToFront(
            new GDBCommand(breakpoint->enabled() ? BreakEnable : BreakDisable,
                           QString::number(m_ids[breakpoint]),
                           handler, &UpdateHandler::handle,
                           true));
        addedCommand = true;
    } else if (m_dirty[breakpoint].contains(KDevelop::Breakpoint::IgnoreHitsColumn)) {
        Q_ASSERT(m_ids.contains(breakpoint));
        UpdateHandler *handler = new UpdateHandler(this, breakpoint, KDevelop::Breakpoint::IgnoreHitsColumn);
        controller()->addCommandToFront(
            new GDBCommand(BreakAfter,
                           QString("%0 %1").arg(m_ids[breakpoint]).arg(breakpoint->ignoreHits()),
                           handler, &UpdateHandler::handle,
                           true));
        addedCommand = true;
    } else if (m_dirty[breakpoint].contains(KDevelop::Breakpoint::ConditionColumn)) {
        Q_ASSERT(m_ids.contains(breakpoint));
        UpdateHandler *handler = new UpdateHandler(this, breakpoint, KDevelop::Breakpoint::ConditionColumn);
        controller()->addCommandToFront(
            new GDBCommand(BreakCondition,
                           QString("%0 %1").arg(m_ids[breakpoint]).arg(breakpoint->condition()),
                           handler, &UpdateHandler::handle,
                           true));
        addedCommand = true;
    }
    if (addedCommand && debugSession()->state() == KDevelop::IDebugSession::ActiveState) {
        if (m_interrupted) {
            kDebug() << "dbg is busy, already interrupting";
        } else {
            kDebug() << "dbg is busy, interrupting";
            m_interrupted = true;
            controller()->slotPauseApp();
        }
    }
}

void BreakpointController::handleBreakpointList(const GDBMI::ResultRecord &r)
{
    m_dontSendChanges++;

    const GDBMI::Value& blist = r["BreakpointTable"]["body"];

    /* Remove breakpoints that are gone in GDB.  In future, we might
       want to inform the user that this happened. */
    QSet<int> present_in_gdb;
    for (int i = 0, e = blist.size(); i != e; ++i)
    {
        present_in_gdb.insert(blist[i]["number"].toInt());
    }

    foreach (KDevelop::Breakpoint *b, breakpointModel()->breakpoints()) {
        if (m_ids.contains(b) && !present_in_gdb.contains(m_ids[b])) {
            breakpointModel()->removeRow(breakpointModel()->breakpointIndex(b, 0).row());
        }
    }

    for(int i = 0, e = blist.size(); i != e; ++i)
    {
        const GDBMI::Value& mi_b = blist[i];
        int id = mi_b["number"].toInt();
        
        KDevelop::Breakpoint* b = m_ids.key(id);
        if (!b) {
            QString type = mi_b["type"].literal();
            if (type == "watchpoint" || type == "hw watchpoint") {
                b = breakpointModel()->addWatchpoint();
            } else if (type == "read watchpoint") {
                b = breakpointModel()->addReadWatchpoint();
            } else if (type == "acc watchpoint") {
                b = breakpointModel()->addAccessWatchpoint();
            } else {
                b = breakpointModel()->addCodeBreakpoint();
            }
        }

        update(b, mi_b);
    }

    m_dontSendChanges--;
}

void BreakpointController::update(KDevelop::Breakpoint *breakpoint, const GDBMI::Value &b)
{
    m_dontSendChanges++;
    
    if (breakpoint->deleted()) return;

    m_ids[breakpoint] = b["number"].toInt();

    QString type = b["type"].literal();
    if (b.hasField("original-location")) {
        if (breakpoint->address().isEmpty()) {
            /* If the address is not empty, it means that the breakpoint
               is set by KDevelop, not by the user, and that we want to
               show the original expression, not the address, in the table.
               TODO: this also means that if used added a watchpoint in gdb
               like "watch foo", then we'll show it in the breakpoint table
               just fine, but after KDevelop restart, we'll try to add the
               breakpoint using basically "watch *&(foo)".  I'm not sure if
               that's a problem or not.  */
            breakpoint->setData(KDevelop::Breakpoint::LocationColumn, b["original-location"].literal());
        }
    } else {
        breakpoint->setData(KDevelop::Breakpoint::LocationColumn, "Your GDB is too old");
    }


    if (!m_dirty[breakpoint].contains(KDevelop::Breakpoint::ConditionColumn)
        && !m_errors[breakpoint].contains(KDevelop::Breakpoint::ConditionColumn))
    {
        if (b.hasField("cond")) {
            breakpoint->setData(KDevelop::Breakpoint::ConditionColumn, b["cond"].literal());
        }
    }

    if (b.hasField("addr") && b["addr"].literal() == "<PENDING>") {
        m_pending.insert(breakpoint);
    } else {
        m_pending.remove(breakpoint);
    }

    setHitCount(breakpoint, b["times"].toInt());

    if (b.hasField("ignore")) {
        breakpoint->setIgnoreHits(b["ignore"].toInt());
    } else {
        breakpoint->setIgnoreHits(0);
    }

    m_dontSendChanges--;
}

void BreakpointController::programStopped(const GDBMI::ResultRecord& r)
{
    QString reason = r["reason"].literal();
    kDebug() << reason;

    /* This method will not do the right thing if we hit a breakpoint
        that is added in GDB outside kdevelop.  In this case we'll
        first try to find the breakpoint, and fail, and only then
        update the breakpoint table and notice the new one.  */

    int id = 0;
    if (reason == "breakpoint-hit") {
        id = r["bkptno"].literal().toInt();
    } else if (reason == "watchpoint-trigger") {
        id = r["wpt"]["number"].literal().toInt();
    } else if (reason == "read-watchpoint-trigger") {
        id = r["hw-rwpt"]["number"].literal().toInt();
    } else if (reason == "access-watchpoint-trigger") {
        id = r["hw-awpt"]["number"].literal().toInt();
    }
    if (id) {
        QString msg;
        if (r.hasField("value")) {
            if (r["value"].hasField("old")) {
                msg += i18n("<br>Old value: %1", r["value"]["old"].literal());
            }
            if (r["value"].hasField("new")) {
                msg += i18n("<br>New value: %1", r["value"]["new"].literal());
            }
        }
        KDevelop::Breakpoint* b = m_ids.key(id);
        if (b) {
            hit(b, msg);
        }
    }
}


}

#include "breakpointcontroller.moc"
