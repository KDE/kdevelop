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
#include <KLocalizedString>

#include <interfaces/icore.h>
#include <interfaces/idebugcontroller.h>
#include <debugger/breakpoint/breakpointmodel.h>
#include <debugger/breakpoint/breakpoint.h>

#include "gdbcommand.h"
#include "debugsession.h"

using namespace GDBMI;

namespace GDBDebugger {


QString quoteExpression(QString expr)
{
    expr.replace('"', "\\\"");
    expr = expr.prepend('"').append('"');
    return expr;
}
QString unquoteExpression(QString expr)
{
    if (expr.left(1) == QString('"') && expr.right(1) == QString('"')) {
        expr = expr.mid(1, expr.length()-2);
        expr.replace("\\\"", "\"");
    }
    return expr;
}

struct Handler : public GDBCommandHandler
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
    }
    virtual bool handlesError() { return true; }
private:
    KDevelop::Breakpoint::Column m_column;
};

struct InsertedHandler : public Handler
{
    InsertedHandler(BreakpointController *c, KDevelop::Breakpoint *b)
        : Handler(c, b) {}

    virtual void handle(const GDBMI::ResultRecord &r)
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
                controller->m_ids[breakpoint] = r["wpt"]["number"].literal();
            } else if (r.hasField("hw-rwpt")) {
                controller->m_ids[breakpoint] = r["hw-rwpt"]["number"].literal();
            } else if (r.hasField("hw-awpt")) {
                controller->m_ids[breakpoint] = r["hw-awpt"]["number"].literal();
            }
            Q_ASSERT(!controller->m_ids[breakpoint].isEmpty());
            kDebug() << "breakpoint id" << breakpoint << controller->m_ids[breakpoint];
        }
        controller->m_dirty[breakpoint].remove(KDevelop::Breakpoint::LocationColumn);
        controller->breakpointStateChanged(breakpoint);
        controller->sendMaybe(breakpoint);
    }

    virtual bool handlesError() { return true; }
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
            kDebug() << "delete finished, but was not really deleted (it was just modified)";
            controller->sendMaybe(breakpoint);
        } else {
            delete breakpoint;
        }
    }
};


BreakpointController::BreakpointController(DebugSession* parent)
    : KDevelop::IBreakpointController(parent), m_interrupted(false)
{
    Q_ASSERT(parent);
    // FIXME: maybe, all debugger components should derive from
    // a base class that does this connect.
    connect(debugSession(),     SIGNAL(event(IDebugSession::event_t)),
            this,       SLOT(slotEvent(IDebugSession::event_t)));
    connect(parent, SIGNAL(programStopped(GDBMI::ResultRecord)), SLOT(programStopped(GDBMI::ResultRecord)));
}

DebugSession *BreakpointController::debugSession() const
{
    Q_ASSERT(QObject::parent());
    return static_cast<DebugSession*>(const_cast<QObject*>(QObject::parent()));
}


void BreakpointController::slotEvent(IDebugSession::event_t e)
{
    switch(e) {
         case IDebugSession::program_state_changed:
            if (m_interrupted) {
                m_interrupted = false;
            } else {
                debugSession()->addCommand(
                    new GDBCommand(GDBMI::BreakList,
                                "",
                                this,
                                &BreakpointController::handleBreakpointList));
            }
            break;

        case IDebugSession::connected_to_program:
        {
            kDebug() << "connected to program";

            //load breakpoints the user might have added through eg .gdbinit on startup
            //*before* sending, so we avoid getting duplicates
            debugSession()->addCommand(
                new GDBCommand(GDBMI::BreakList,
                            "",
                            this,
                            &BreakpointController::handleBreakpointListInitial));

            break;
        }
        case IDebugSession::debugger_exited:
        {
            break;
        }
        default:
            break;
    }
}

void BreakpointController::handleBreakpointListInitial(const GDBMI::ResultRecord &r)
{
    if (!breakpointModel()) return;

    m_dontSendChanges++;

    const GDBMI::Value& blist = r["BreakpointTable"]["body"];

    for(int i = 0, e = blist.size(); i != e; ++i)
    {
        KDevelop::Breakpoint *updateBreakpoint = 0;
        const GDBMI::Value& mi_b = blist[i];
        QString type = mi_b["type"].literal();
        foreach(KDevelop::Breakpoint *b, breakpointModel()->breakpoints()) {
            if ((type == "watchpoint" || type == "hw watchpoint") && b->kind() == KDevelop::Breakpoint::WriteBreakpoint) {
                if (unquoteExpression(mi_b["original-location"].literal()) == b->expression()) {
                    updateBreakpoint = b;
                }
            } else if (type == "read watchpoint" && b->kind() == KDevelop::Breakpoint::ReadBreakpoint) {
                if (unquoteExpression(mi_b["original-location"].literal()) == b->expression()) {
                    updateBreakpoint = b;
                }
            } else if (type == "acc watchpoint" && b->kind() == KDevelop::Breakpoint::AccessBreakpoint) {
                if (unquoteExpression(mi_b["original-location"].literal()) == b->expression()) {
                    updateBreakpoint = b;
                }
            } else if (b->kind() == KDevelop::Breakpoint::CodeBreakpoint) {
                QString condition;
                if (mi_b.hasField("cond")) {
                    condition = mi_b["cond"].literal();
                }
                if (condition != b->condition())
                    continue;

                QString location;
                if (mi_b.hasField("original-location")) {
                    location = mi_b["original-location"].literal();
                    kDebug() << "location" << location;
                    QRegExp rx("^(.+):(\\d+)$");
                    if (rx.indexIn(location) != -1) {
                        if (unquoteExpression(rx.cap(1)) == b->url().pathOrUrl(KUrl::RemoveTrailingSlash) && rx.cap(2).toInt() - 1 == b->line()) {
                            updateBreakpoint = b;
                        } else {
                            kDebug() << "!=" << b->location();
                        }
                    } else if (location == b->location()) {
                        updateBreakpoint = b;
                    }
                }else if (mi_b.hasField("what") && mi_b["what"].literal() == "exception throw") {
                    if (b->expression() == "catch throw") {
                        updateBreakpoint = b;
                    }
                }else{
                    kWarning() << "That's too bad, breakpoint doesn't contain \"original-location\" field ";
                }
            }
            if (updateBreakpoint) break;
        }
        
        if (updateBreakpoint) {
            update(updateBreakpoint, mi_b);
        } else {
            //ignore, we will load them in the first pause anyway
        }
    }

    m_dontSendChanges--;

    sendMaybeAll();
}

void BreakpointController::sendMaybe(KDevelop::Breakpoint* breakpoint)
{
    if (debugSession()->stateIsOn(s_dbgNotStarted)) {
        return;
    }

    bool addedCommand = false;

    /** See what is dirty, and send the changes.  For simplicity, send
        changes one-by-one and call sendMaybe again in the completion
        handler.
        FIXME: should handle and annotate the errors?
    */
    kDebug() << breakpoint << breakpoint->location();
    if (breakpoint->deleted())
    {
        kDebug() << "deleted";
        m_dirty.remove(breakpoint);
        m_errors.remove(breakpoint);
        if (m_ids.contains(breakpoint)) { //if id is 0 breakpoint insertion is still pending, InsertedHandler will call sendMaybe again and delete it
            kDebug() << "breakpoint id" << m_ids[breakpoint];
            if (!m_ids[breakpoint].isEmpty()) {
                debugSession()->addCommandToFront(
                    new GDBCommand(BreakDelete, m_ids[breakpoint],
                                new DeletedHandler(this, breakpoint)));
                addedCommand = true;
            }
        } else {
            kDebug() << "breakpoint doesn't have yet an id, just delete it";
            delete breakpoint;
        }
    }
    else if (m_dirty[breakpoint].contains(KDevelop::Breakpoint::LocationColumn)) {
        kDebug() << "location changed";
        if (m_ids.contains(breakpoint) && !m_ids[breakpoint].isEmpty()) {
            /* We already have GDB breakpoint for this, so we need to remove
            this one.  */
            kDebug() << "We already have GDB breakpoint for this, so we need to remove this one";
            debugSession()->addCommandToFront(
                new GDBCommand(BreakDelete, m_ids[breakpoint],
                            new DeletedHandler(this, breakpoint)));
            addedCommand = true;
        } else {
            m_ids[breakpoint] = QString(); //add to m_ids so we don't delete it while insert command is still pending
            if (breakpoint->kind() == KDevelop::Breakpoint::CodeBreakpoint) {
                QString location;
                if (breakpoint->line() != -1) {
                    location = quoteExpression(breakpoint->url().pathOrUrl(KUrl::RemoveTrailingSlash)) + ':' + QString::number(breakpoint->line()+1);
                } else {
                    location = breakpoint->location();
                }
                if (breakpoint->expression() == "catch throw") {
                    debugSession()->addCommand(
                    new GDBCommand(GDBMI::NonMI,
                                location));
                    breakpoint->setDeleted();
                }else{
                   debugSession()->addCommandToFront(
                    new GDBCommand(BreakInsert,
                                quoteExpression(location),
                                new InsertedHandler(this, breakpoint)));
                }
                addedCommand = true;
            } else {
                QString opt;
                if (breakpoint->kind() == KDevelop::Breakpoint::ReadBreakpoint)
                    opt = "-r ";
                else if (breakpoint->kind() == KDevelop::Breakpoint::AccessBreakpoint)
                    opt = "-a ";

                debugSession()->addCommandToFront(
                    new GDBCommand(
                        BreakWatch,
                        opt + quoteExpression(breakpoint->location()),
                        new InsertedHandler(this, breakpoint)));
                addedCommand = true;
            }
        }
    } else if (m_dirty[breakpoint].contains(KDevelop::Breakpoint::EnableColumn)) {
        if (m_ids.contains(breakpoint) && !m_ids[breakpoint].isEmpty()) {
            debugSession()->addCommandToFront(
                new GDBCommand(breakpoint->enabled() ? BreakEnable : BreakDisable,
                            m_ids[breakpoint],
                            new UpdateHandler(this, breakpoint, KDevelop::Breakpoint::EnableColumn)));
            addedCommand = true;
        }
    } else if (m_dirty[breakpoint].contains(KDevelop::Breakpoint::IgnoreHitsColumn)) {
        if (m_ids.contains(breakpoint) && !m_ids[breakpoint].isEmpty()) {
            debugSession()->addCommandToFront(
                new GDBCommand(BreakAfter,
                            QString("%0 %1").arg(m_ids[breakpoint]).arg(breakpoint->ignoreHits()),
                            new UpdateHandler(this, breakpoint, KDevelop::Breakpoint::IgnoreHitsColumn)));
            addedCommand = true;
        }
    } else if (m_dirty[breakpoint].contains(KDevelop::Breakpoint::ConditionColumn)) {
        if (m_ids.contains(breakpoint) && !m_ids[breakpoint].isEmpty()) {
            debugSession()->addCommandToFront(
                new GDBCommand(BreakCondition,
                            QString("%0 %1").arg(m_ids[breakpoint]).arg(breakpoint->condition()),
                            new UpdateHandler(this, breakpoint, KDevelop::Breakpoint::ConditionColumn)));
            addedCommand = true;
        }
    }
    if (addedCommand && debugSession()->state() == KDevelop::IDebugSession::ActiveState) {
        if (m_interrupted) {
            kDebug() << "dbg is busy, already interrupting";
        } else {
            kDebug() << "dbg is busy, interrupting";
            m_interrupted = true;
            debugSession()->interruptDebugger();
            debugSession()->addCommand(ExecContinue); //continue right after interrupting, if other breakpoint related commands queue up they get inserted before continue (as addCommandToFront is used)
        }
    }
}

void BreakpointController::handleBreakpointList(const GDBMI::ResultRecord &r)
{
    if (!breakpointModel()) return;

    m_dontSendChanges++;

    const GDBMI::Value& blist = r["BreakpointTable"]["body"];

    /* Remove breakpoints that are gone in GDB.  In future, we might
       want to inform the user that this happened. */
    QSet<QString> present_in_gdb;
    for (int i = 0, e = blist.size(); i != e; ++i)
    {
        present_in_gdb.insert(blist[i]["number"].literal());
    }

    foreach (KDevelop::Breakpoint *b, breakpointModel()->breakpoints()) {
        if (m_ids.contains(b) && !present_in_gdb.contains(m_ids[b])) {
            breakpointModel()->removeRow(breakpointModel()->breakpointIndex(b, 0).row());
        }
    }

    QString previousType;
    for(int i = 0, e = blist.size(); i != e; ++i)
    {
        const GDBMI::Value& mi_b = blist[i];
        QString id = mi_b["number"].literal();
        
        KDevelop::Breakpoint* b = m_ids.key(id);
        if (!b) {
            QString type;
            if (mi_b.hasField("type")) {
                type = mi_b["type"].literal();
            } else {
                //happens for breakpoints with multiple locations (the following ones don't contain a type)
                type = previousType;
            }
            if (type == "watchpoint" || type == "hw watchpoint") {
                b = breakpointModel()->addWatchpoint();
            } else if (type == "read watchpoint") {
                b = breakpointModel()->addReadWatchpoint();
            } else if (type == "acc watchpoint") {
                b = breakpointModel()->addAccessWatchpoint();
            } else {
                //for multiple breakpoints(constructor/destructor... ) show only the parent breakpoint(1, 1.1, 1.2, ... only 1), because all other can be usefull only in dissasembleWidget.
                if(mi_b.hasField("number") && !mi_b["number"].literal().contains(".")){
                    b = breakpointModel()->addCodeBreakpoint();
                }else{
                    continue;
                }
            }
            previousType = type;
        }

        update(b, mi_b);
    }

    m_dontSendChanges--;
}

void BreakpointController::update(KDevelop::Breakpoint *breakpoint, const GDBMI::Value &b)
{
    m_dontSendChanges++;
    
    m_ids[breakpoint] = b["number"].literal();

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
            QString location = b["original-location"].literal();
            kDebug() << "location" << location;
            if (breakpoint->kind() == KDevelop::Breakpoint::CodeBreakpoint) {
                QRegExp rx("^(.+):(\\d+)$");
                if (rx.indexIn(location) != -1) {
                    breakpoint->setLocation(KUrl(unquoteExpression(rx.cap(1))), rx.cap(2).toInt()-1);
                } else {
                    //for regular expression breakpoints and not only...
                    if(b.hasField("fullname") && b.hasField("line")){
                        breakpoint->setLocation(KUrl(unquoteExpression(b["fullname"].literal())), b["line"].toInt()-1);
                    }else{
                        kWarning() << "can't parse location" << location;
                    }
                }
            } else {
                breakpoint->setData(KDevelop::Breakpoint::LocationColumn, unquoteExpression(location));
            }
        }
    } else if (b.hasField("what") && b["what"].literal() == "exception throw") {
        breakpoint->setExpression("catch throw");
    } else {
        kWarning() << "That's too bad, breakpoint doesn't contain \"original-location\" field ";
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

    if (b.hasField("times")) {
        setHitCount(breakpoint, b["times"].toInt());
    } else {
        setHitCount(breakpoint, -1);
    }

    if (b.hasField("ignore")) {
        breakpoint->setIgnoreHits(b["ignore"].toInt());
    } else {
        breakpoint->setIgnoreHits(0);
    }

    m_dontSendChanges--;
}

void BreakpointController::programStopped(const GDBMI::ResultRecord& r)
{
    QString reason;
    if (r.hasField("reason")) {
        reason = r["reason"].literal();
    }
    kDebug() << reason;

    /* This method will not do the right thing if we hit a breakpoint
        that is added in GDB outside kdevelop.  In this case we'll
        first try to find the breakpoint, and fail, and only then
        update the breakpoint table and notice the new one.  */

    QString id;
    if (reason == "breakpoint-hit") {
        id = r["bkptno"].literal();
    } else if (reason == "watchpoint-trigger") {
        id = r["wpt"]["number"].literal();
    } else if (reason == "read-watchpoint-trigger") {
        id = r["hw-rwpt"]["number"].literal();
    } else if (reason == "access-watchpoint-trigger") {
        id = r["hw-awpt"]["number"].literal();
    }
    if (!id.isEmpty()) {
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
