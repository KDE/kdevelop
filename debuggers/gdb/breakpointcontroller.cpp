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

#include <KLocalizedString>

#include <interfaces/icore.h>
#include <interfaces/idebugcontroller.h>
#include <debugger/breakpoint/breakpointmodel.h>
#include <debugger/breakpoint/breakpoint.h>

#include "gdbcommand.h"
#include "debugsession.h"
#include "debug.h"

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

struct BreakpointData {
    int gdbId;
    BreakpointModel::ColumnFlags dirty;
    BreakpointModel::ColumnFlags sent;
    BreakpointModel::ColumnFlags errors;
    bool pending;

    BreakpointData()
        : gdbId(-1)
        , pending(false)
    {}
};

struct BreakpointController::Handler : public GDBCommandHandler
{
    Handler(BreakpointController* controller, const BreakpointDataPtr& b, BreakpointModel::ColumnFlags columns)
        : controller(controller)
        , breakpoint(b)
        , columns(columns)
    {
        breakpoint->sent |= columns;
        breakpoint->dirty &= ~columns;
    }

    virtual void handle(const ResultRecord& r) override
    {
        breakpoint->sent &= ~columns;

        if (r.reason == "error") {
            breakpoint->errors |= columns;

            int row = controller->breakpointRow(breakpoint);
            if (row >= 0) {
                controller->updateErrorText(row, r["msg"].literal());
                qWarning() << r["msg"].literal();
            }
        } else {
            if (breakpoint->errors & columns) {
                breakpoint->errors &= ~columns;

                if (breakpoint->errors) {
                    // Since at least one error column cleared, it's possible that any remaining
                    // error bits were collateral damage; try resending the corresponding columns
                    // to see whether errors remain.
                    breakpoint->dirty |= (breakpoint->errors & ~breakpoint->sent);
                }
            }
        }
    }

    virtual bool handlesError() override
    {
        return true;
    }

    BreakpointController* controller;
    BreakpointDataPtr breakpoint;
    BreakpointModel::ColumnFlags columns;
};

struct BreakpointController::UpdateHandler : public BreakpointController::Handler
{
    UpdateHandler(BreakpointController* c, const BreakpointDataPtr& b, BreakpointModel::ColumnFlags columns)
        : Handler(c, b, columns) {}

    void handle(const GDBMI::ResultRecord &r)
    {
        Handler::handle(r);

        int row = controller->breakpointRow(breakpoint);
        if (row >= 0) {
            // Note: send further updates even if we got an error; who knows: perhaps
            // these additional updates will "unstuck" the error condition.
            if (breakpoint->sent == 0 && breakpoint->dirty != 0) {
                controller->sendUpdates(row);
            }
            controller->recalculateState(row);
        }
    }
};

struct BreakpointController::InsertedHandler : public BreakpointController::Handler
{
    InsertedHandler(BreakpointController* c, const BreakpointDataPtr& b, BreakpointModel::ColumnFlags columns)
        : Handler(c, b, columns) {}

    virtual void handle(const GDBMI::ResultRecord &r)
    {
        Handler::handle(r);

        int row = controller->breakpointRow(breakpoint);

        if (r.reason != "error") {
            QString bkptKind;
            for (auto kind : {"bkpt", "wpt", "hw-rwpt", "hw-awpt"}) {
                if (r.hasField(kind)) {
                    bkptKind = kind;
                    break;
                }
            }
            if (bkptKind.isEmpty()) {
                qWarning() << "Gdb sent unknown breakpoint kind";
                return;
            }

            const Value& miBkpt = r[bkptKind];

            breakpoint->gdbId = miBkpt["number"].toInt();

            if (row >= 0) {
                controller->updateFromGdb(row, miBkpt);
                if (breakpoint->dirty != 0)
                    controller->sendUpdates(row);
            } else {
                // breakpoint was deleted while insertion was in flight
                controller->debugSession()->addCommand(
                    new GDBCommand(BreakDelete, QString::number(breakpoint->gdbId),
                                   CmdImmediately));
            }
        }

        controller->recalculateState(row);
    }
};

struct BreakpointController::DeleteHandler : BreakpointController::Handler {
    DeleteHandler(BreakpointController* c, const BreakpointDataPtr& b)
        : Handler(c, b, 0) {}

    virtual void handle(const ResultRecord&)
    {
        controller->m_pendingDeleted.removeAll(breakpoint);
    }
};

struct BreakpointController::IgnoreChanges {
    IgnoreChanges(BreakpointController& controller)
        : controller(controller)
    {
        ++controller.m_ignoreChanges;
    }

    ~IgnoreChanges()
    {
        --controller.m_ignoreChanges;
    }

    BreakpointController& controller;
};

BreakpointController::BreakpointController(DebugSession* parent)
    : IBreakpointController(parent)
    , m_ignoreChanges(0)
{
    Q_ASSERT(parent);
    connect(parent, &DebugSession::programStopped, this, &BreakpointController::programStopped);

    int numBreakpoints = breakpointModel()->breakpoints().size();
    for (int row = 0; row < numBreakpoints; ++row)
        breakpointAdded(row);
}

DebugSession *BreakpointController::debugSession() const
{
    Q_ASSERT(QObject::parent());
    return static_cast<DebugSession*>(const_cast<QObject*>(QObject::parent()));
}

int BreakpointController::breakpointRow(const BreakpointDataPtr& breakpoint)
{
    return m_breakpoints.indexOf(breakpoint);
}

void BreakpointController::initSendBreakpoints()
{
    for (int row = 0; row < m_breakpoints.size(); ++row) {
        BreakpointDataPtr breakpoint = m_breakpoints[row];
        if (breakpoint->gdbId < 0 && breakpoint->sent == 0) {
            createGdbBreakpoint(row);
        }
    }
}

void BreakpointController::breakpointAdded(int row)
{
    if (m_ignoreChanges > 0)
        return;

    auto breakpoint = BreakpointDataPtr::create();
    m_breakpoints.insert(row, breakpoint);

    const Breakpoint* modelBreakpoint = breakpointModel()->breakpoint(row);
    if (!modelBreakpoint->enabled())
        breakpoint->dirty |= BreakpointModel::EnableColumnFlag;
    if (!modelBreakpoint->condition().isEmpty())
        breakpoint->dirty |= BreakpointModel::ConditionColumnFlag;
    if (modelBreakpoint->ignoreHits() != 0)
        breakpoint->dirty |= BreakpointModel::IgnoreHitsColumnFlag;
    if (!modelBreakpoint->address().isEmpty())
        breakpoint->dirty |= BreakpointModel::LocationColumnFlag;

    createGdbBreakpoint(row);
}

void BreakpointController::breakpointModelChanged(int row, BreakpointModel::ColumnFlags columns)
{
    if (m_ignoreChanges > 0)
        return;

    BreakpointDataPtr breakpoint = m_breakpoints.at(row);
    breakpoint->dirty |= columns &
        (BreakpointModel::EnableColumnFlag | BreakpointModel::LocationColumnFlag |
         BreakpointModel::ConditionColumnFlag | BreakpointModel::IgnoreHitsColumnFlag);

    if (breakpoint->sent != 0) {
        // Throttle the amount of commands we send to GDB; the response handler of currently
        // in-flight commands will take care of sending the update.
        // This also prevents us from sending updates while a break-create command is in-flight.
        return;
    }

    if (breakpoint->gdbId < 0) {
        createGdbBreakpoint(row);
    } else {
        sendUpdates(row);
    }
}

void BreakpointController::breakpointAboutToBeDeleted(int row)
{
    if (m_ignoreChanges > 0)
        return;

    BreakpointDataPtr breakpoint = m_breakpoints.at(row);
    m_breakpoints.removeAt(row);

    if (breakpoint->gdbId < 0) {
        // Two possibilities:
        //  (1) Breakpoint has never been sent to GDB, so we're done
        //  (2) Breakpoint has been sent to GDB, but we haven't received
        //      the response yet; the response handler will delete the
        //      breakpoint.
        return;
    }

    if (debugSession()->stateIsOn(s_dbgNotStarted))
        return;

    debugSession()->addCommand(
        new GDBCommand(
            BreakDelete, QString::number(breakpoint->gdbId),
            new DeleteHandler(this, breakpoint), CmdImmediately));
    m_pendingDeleted << breakpoint;
}

void BreakpointController::debuggerStateChanged(IDebugSession::DebuggerState)
{
}

void BreakpointController::createGdbBreakpoint(int row)
{
    if (debugSession()->stateIsOn(s_dbgNotStarted))
        return;

    BreakpointDataPtr breakpoint = m_breakpoints.at(row);
    Breakpoint* modelBreakpoint = breakpointModel()->breakpoint(row);

    Q_ASSERT(breakpoint->gdbId < 0 && breakpoint->sent == 0);

    if (modelBreakpoint->location().isEmpty())
        return;

    if (modelBreakpoint->kind() == Breakpoint::CodeBreakpoint) {
        QString location;
        if (modelBreakpoint->line() != -1) {
            location = QString("%0:%1")
                .arg(modelBreakpoint->url().url(QUrl::PreferLocalFile | QUrl::StripTrailingSlash))
                .arg(modelBreakpoint->line() + 1);
        } else {
            location = modelBreakpoint->location();
        }

        if (location == "catch throw") {
            location = "exception throw";
        }

        // Note: We rely on '-f' to be automatically added by the GDBCommand logic
        QString arguments;
        if (!modelBreakpoint->enabled())
            arguments += "-d ";
        if (!modelBreakpoint->condition().isEmpty())
            arguments += QString("-c %0 ").arg(quoteExpression(modelBreakpoint->condition()));
        if (modelBreakpoint->ignoreHits() != 0)
            arguments += QString("-i %0 ").arg(modelBreakpoint->ignoreHits());
        arguments += quoteExpression(location);

        BreakpointModel::ColumnFlags sent =
            BreakpointModel::EnableColumnFlag |
            BreakpointModel::ConditionColumnFlag |
            BreakpointModel::IgnoreHitsColumnFlag |
            BreakpointModel::LocationColumnFlag;
        debugSession()->addCommand(
            new GDBCommand(BreakInsert, arguments,
                new InsertedHandler(this, breakpoint, sent),
                CmdImmediately));
    } else {
        QString opt;
        if (modelBreakpoint->kind() == Breakpoint::ReadBreakpoint)
            opt = "-r ";
        else if (modelBreakpoint->kind() == Breakpoint::AccessBreakpoint)
            opt = "-a ";

        debugSession()->addCommand(
            new GDBCommand(
                BreakWatch,
                opt + quoteExpression(modelBreakpoint->location()),
                new InsertedHandler(this, breakpoint, BreakpointModel::LocationColumnFlag),
                CmdImmediately));
    }

    recalculateState(row);
}

void BreakpointController::sendUpdates(int row)
{
    if (debugSession()->stateIsOn(s_dbgNotStarted))
        return;

    BreakpointDataPtr breakpoint = m_breakpoints.at(row);
    Breakpoint* modelBreakpoint = breakpointModel()->breakpoint(row);

    Q_ASSERT(breakpoint->gdbId >= 0 && breakpoint->sent == 0);

    if (breakpoint->dirty & BreakpointModel::LocationColumnFlag) {
        // Gdb considers locations as fixed, so delete and re-create the breakpoint
        debugSession()->addCommand(
            new GDBCommand(BreakDelete, QString::number(breakpoint->gdbId), CmdImmediately));
        breakpoint->gdbId = -1;
        createGdbBreakpoint(row);
        return;
    }

    if (breakpoint->dirty & BreakpointModel::EnableColumnFlag) {
        debugSession()->addCommand(
            new GDBCommand(modelBreakpoint->enabled() ? BreakEnable : BreakDisable,
                QString::number(breakpoint->gdbId),
                new UpdateHandler(this, breakpoint, BreakpointModel::EnableColumnFlag),
                CmdImmediately));
    }
    if (breakpoint->dirty & BreakpointModel::IgnoreHitsColumnFlag) {
        debugSession()->addCommand(
            new GDBCommand(BreakAfter,
                QString("%0 %1").arg(breakpoint->gdbId).arg(modelBreakpoint->ignoreHits()),
                new UpdateHandler(this, breakpoint, BreakpointModel::IgnoreHitsColumnFlag),
                CmdImmediately));
    }
    if (breakpoint->dirty & BreakpointModel::ConditionColumnFlag) {
        debugSession()->addCommand(
            new GDBCommand(BreakCondition,
                QString("%0 %1").arg(breakpoint->gdbId).arg(modelBreakpoint->condition()),
                new UpdateHandler(this, breakpoint, BreakpointModel::ConditionColumnFlag),
                CmdImmediately));
    }

    recalculateState(row);
}

void BreakpointController::recalculateState(int row)
{
    BreakpointDataPtr breakpoint = m_breakpoints.at(row);

    if (breakpoint->errors == 0)
        updateErrorText(row, QString());

    Breakpoint::BreakpointState newState = Breakpoint::NotStartedState;
    if (!debugSession()->stateIsOn(s_dbgNotStarted)) {
        if (breakpoint->dirty == 0 && breakpoint->sent == 0) {
            if (breakpoint->pending) {
                newState = Breakpoint::PendingState;
            } else {
                newState = Breakpoint::CleanState;
            }
        } else {
            newState = Breakpoint::DirtyState;
        }
    }

    updateState(row, newState);
}

int BreakpointController::rowFromGdbId(int gdbId) const
{
    for (int row = 0; row < m_breakpoints.size(); ++row) {
        if (gdbId == m_breakpoints[row]->gdbId)
            return row;
    }
    return -1;
}

void BreakpointController::notifyBreakpointCreated(const AsyncRecord& r)
{
    const Value& miBkpt = r["bkpt"];

    // Breakpoints with multiple locations are represented by a parent breakpoint (e.g. 1)
    // and multiple child breakpoints (e.g. 1.1, 1.2, 1.3, ...).
    // We ignore the child breakpoints here in the current implementation; this can lead to dubious
    // results in the UI when breakpoints are marked in document views (e.g. when a breakpoint
    // applies to multiple overloads of a C++ function simultaneously) and in disassembly
    // (e.g. when a breakpoint is set in an inlined functions).
    if (miBkpt["number"].literal().contains('.'))
        return;

    createFromGdb(miBkpt);
}

void BreakpointController::notifyBreakpointModified(const AsyncRecord& r)
{
    const Value& miBkpt = r["bkpt"];
    const int gdbId = miBkpt["number"].toInt();
    const int row = rowFromGdbId(gdbId);

    if (row < 0) {
        for (const auto& breakpoint : m_pendingDeleted) {
            if (breakpoint->gdbId == gdbId) {
                // Received a modification of a breakpoint whose deletion is currently
                // in-flight; simply ignore it.
                return;
            }
        }

        qWarning() << "Received a modification of an unknown breakpoint";
        createFromGdb(miBkpt);
    } else {
        updateFromGdb(row, miBkpt);
    }
}

void BreakpointController::notifyBreakpointDeleted(const AsyncRecord& r)
{
    const int gdbId = r["id"].toInt();
    const int row = rowFromGdbId(gdbId);

    if (row < 0) {
        // The user may also have deleted the breakpoint via the UI simultaneously
        return;
    }

    IgnoreChanges ignoreChanges(*this);
    breakpointModel()->removeRow(row);
}

void BreakpointController::createFromGdb(const Value& miBkpt)
{
    IgnoreChanges ignoreChanges(*this);

    const QString type = miBkpt["type"].literal();
    Breakpoint::BreakpointKind gdbKind;
    if (type == "breakpoint") {
        gdbKind = Breakpoint::CodeBreakpoint;
    } else if (type == "watchpoint" || type == "hw watchpoint") {
        gdbKind = Breakpoint::WriteBreakpoint;
    } else if (type == "read watchpoint") {
        gdbKind = Breakpoint::ReadBreakpoint;
    } else if (type == "acc watchpoint") {
        gdbKind = Breakpoint::AccessBreakpoint;
    } else {
        qWarning() << "Unknown gdb breakpoint type " << type;
        return;
    }

    // During gdb startup, we want to avoid creating duplicate breakpoints when the same breakpoint
    // appears both in our model and in a .gdbinit file
    BreakpointModel* model = breakpointModel();
    int numRows = model->rowCount();
    int row;
    for (row = 0; row < numRows; ++row) {
        BreakpointDataPtr breakpoint = m_breakpoints.at(row);
        if (breakpoint->gdbId >= 0 || breakpoint->sent != 0)
            continue;

        Breakpoint* modelBreakpoint = model->breakpoint(row);
        if (modelBreakpoint->kind() != gdbKind)
            continue;

        if (gdbKind == Breakpoint::CodeBreakpoint) {
            bool sameLocation = false;

            if (miBkpt.hasField("fullname") && miBkpt.hasField("line")) {
                const QString location = unquoteExpression(miBkpt["fullname"].literal());
                const int line = miBkpt["line"].toInt() - 1;
                if (location == modelBreakpoint->url().url(QUrl::PreferLocalFile | QUrl::StripTrailingSlash) &&
                    line == modelBreakpoint->line())
                {
                    sameLocation = true;
                }
            }

            if (!sameLocation && miBkpt.hasField("original-location")) {
                const QString location = miBkpt["original-location"].literal();
                if (location == modelBreakpoint->location()) {
                    sameLocation = true;
                } else {
                    QRegExp rx("^(.+):(\\d+)$");
                    if (rx.indexIn(location) != -1 &&
                        unquoteExpression(rx.cap(1)) == modelBreakpoint->url().url(QUrl::PreferLocalFile | QUrl::StripTrailingSlash) &&
                        rx.cap(2).toInt() - 1 == modelBreakpoint->line()) {
                        sameLocation = true;
                    }
                }
            }

            if (!sameLocation && miBkpt.hasField("what") && miBkpt["what"].literal() == "exception throw") {
                if (modelBreakpoint->expression() == "catch throw" ||
                    modelBreakpoint->expression() == "exception throw") {
                    sameLocation = true;
                }
            }

            if (!sameLocation)
                continue;
        } else {
            if (unquoteExpression(miBkpt["original-location"].literal()) != modelBreakpoint->expression()) {
                continue;
            }
        }

        QString condition;
        if (miBkpt.hasField("cond")) {
            condition = miBkpt["cond"].literal();
        }
        if (condition != modelBreakpoint->condition())
            continue;

        break;
    }

    if (row < numRows) {
        BreakpointDataPtr breakpoint = m_breakpoints.at(row);
        Breakpoint* modelBreakpoint = model->breakpoint(row);
        breakpoint->gdbId = miBkpt["number"].toInt();

        // Reasonable people can probably have different opinions about what the "correct" behavior
        // should be for the "enabled" and "ignore hits" column.
        // Here, we let the status in KDevelop's UI take precedence, which we suspect to be
        // marginally more useful. Dirty data will be sent during the initial sending of the
        // breakpoint list.
        const bool gdbEnabled = miBkpt["enabled"].literal() == "y";
        if (gdbEnabled != modelBreakpoint->enabled())
            breakpoint->dirty |= BreakpointModel::EnableColumnFlag;

        int gdbIgnoreHits = 0;
        if (miBkpt.hasField("ignore"))
            gdbIgnoreHits = miBkpt["ignore"].toInt();
        if (gdbIgnoreHits != modelBreakpoint->ignoreHits())
            breakpoint->dirty |= BreakpointModel::IgnoreHitsColumnFlag;

        updateFromGdb(row, miBkpt, BreakpointModel::EnableColumnFlag | BreakpointModel::IgnoreHitsColumnFlag);
    } else {
        switch (gdbKind) {
        case Breakpoint::WriteBreakpoint: model->addWatchpoint(); break;
        case Breakpoint::ReadBreakpoint: model->addReadWatchpoint(); break;
        case Breakpoint::AccessBreakpoint: model->addAccessWatchpoint(); break;
        case Breakpoint::CodeBreakpoint: model->addCodeBreakpoint(); break;
        default: Q_ASSERT(false); return;
        }

        // Since we are in ignore-changes mode, we have to add the BreakpointData manually.
        auto breakpoint = BreakpointDataPtr::create();
        m_breakpoints << breakpoint;
        breakpoint->gdbId = miBkpt["number"].toInt();

        updateFromGdb(row, miBkpt);
    }
}

// This method is required for the legacy interface which will be removed
void BreakpointController::sendMaybe(KDevelop::Breakpoint*)
{
    Q_ASSERT(false);
}

void BreakpointController::updateFromGdb(int row, const Value& miBkpt, BreakpointModel::ColumnFlags lockedColumns)
{
    IgnoreChanges ignoreChanges(*this);
    BreakpointDataPtr breakpoint = m_breakpoints[row];
    Breakpoint* modelBreakpoint = breakpointModel()->breakpoint(row);

    // Commands that are currently in flight will overwrite the modification we have received,
    // so do not update the corresponding data
    lockedColumns |= breakpoint->sent | breakpoint->dirty;

    // TODO:
    // Gdb has a notion of "original-location", which is the "expression" or "location" used
    // to set the breakpoint, and notions of the actual location of the breakpoint (function name,
    // address, source file and line). The breakpoint model currently does not map well to this
    // (though it arguably should), and does not support multi-location breakpoints at all.
    // We try to do the best we can until the breakpoint model gets cleaned up.
    if (miBkpt.hasField("fullname") && miBkpt.hasField("line")) {
        modelBreakpoint->setLocation(
            QUrl::fromLocalFile(unquoteExpression(miBkpt["fullname"].literal())),
            miBkpt["line"].toInt() - 1);
    } else if (miBkpt.hasField("original-location")) {
        QRegExp rx("^(.+):(\\d+)$");
        QString location = miBkpt["original-location"].literal();
        if (rx.indexIn(location) != -1) {
            modelBreakpoint->setLocation(QUrl::fromLocalFile(unquoteExpression(rx.cap(1))), rx.cap(2).toInt()-1);
        } else {
            modelBreakpoint->setData(KDevelop::Breakpoint::LocationColumn, unquoteExpression(location));
        }
    } else if (miBkpt.hasField("what")) {
        modelBreakpoint->setExpression(miBkpt["what"].literal());
    } else {
        qWarning() << "Breakpoint doesn't contain required location/expression data";
    }

    if (!(lockedColumns & BreakpointModel::ConditionColumnFlag)) {
        QString condition;
        if (miBkpt.hasField("cond")) {
            condition = miBkpt["cond"].literal();
        }
        modelBreakpoint->setCondition(condition);
        breakpoint->dirty &= ~BreakpointModel::ConditionColumnFlag;
    }

    if (!(lockedColumns & BreakpointModel::IgnoreHitsColumnFlag)) {
        int ignoreHits = 0;
        if (miBkpt.hasField("ignore")) {
            ignoreHits = miBkpt["ignore"].toInt();
        }
        modelBreakpoint->setIgnoreHits(ignoreHits);
        breakpoint->dirty &= ~BreakpointModel::IgnoreHitsColumnFlag;
    }

    breakpoint->pending = false;
    if (miBkpt.hasField("addr") && miBkpt["addr"].literal() == "<PENDING>") {
        breakpoint->pending = true;
    }

    int hitCount = 0;
    if (miBkpt.hasField("times")) {
        hitCount = miBkpt["times"].toInt();
    }
    updateHitCount(row, hitCount);

    recalculateState(row);
}

void BreakpointController::programStopped(const GDBMI::AsyncRecord& r)
{
    if (!r.hasField("reason"))
        return;

    const QString reason = r["reason"].literal();

    int gdbId = -1;
    if (reason == "breakpoint-hit") {
        gdbId = r["bkptno"].toInt();
    } else if (reason == "watchpoint-trigger") {
        gdbId = r["wpt"]["number"].toInt();
    } else if (reason == "read-watchpoint-trigger") {
        gdbId = r["hw-rwpt"]["number"].toInt();
    } else if (reason == "access-watchpoint-trigger") {
        gdbId = r["hw-awpt"]["number"].toInt();
    }

    if (gdbId < 0)
        return;

    int row = rowFromGdbId(gdbId);
    if (row < 0)
        return;

    QString msg;
    if (r.hasField("value")) {
        if (r["value"].hasField("old")) {
            msg += i18n("<br>Old value: %1", r["value"]["old"].literal());
        }
        if (r["value"].hasField("new")) {
            msg += i18n("<br>New value: %1", r["value"]["new"].literal());
        }
    }

    notifyHit(row, msg);
}

}
