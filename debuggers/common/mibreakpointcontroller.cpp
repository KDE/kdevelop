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

#include "mibreakpointcontroller.h"

#include "debuglog.h"
#include "midebugsession.h"
#include "mi/micommand.h"
#include "stringhelpers.h"

#include <debugger/breakpoint/breakpoint.h>
#include <debugger/breakpoint/breakpointmodel.h>
#include <interfaces/icore.h>
#include <interfaces/idebugcontroller.h>

#include <KLocalizedString>

using namespace KDevMI;
using namespace KDevMI::MI;
using namespace KDevelop;

struct MIBreakpointController::Handler : public MICommandHandler
{
    Handler(MIBreakpointController* controller, const BreakpointDataPtr& b,
            BreakpointModel::ColumnFlags columns)
        : controller(controller)
        , breakpoint(b)
        , columns(columns)
    {
        breakpoint->sent |= columns;
        breakpoint->dirty &= ~columns;
    }

    void handle(const ResultRecord& r) override
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

    bool handlesError() override
    {
        return true;
    }

    MIBreakpointController* controller;
    BreakpointDataPtr breakpoint;
    BreakpointModel::ColumnFlags columns;
};

struct MIBreakpointController::UpdateHandler : public MIBreakpointController::Handler
{
    UpdateHandler(MIBreakpointController* c, const BreakpointDataPtr& b,
                  BreakpointModel::ColumnFlags columns)
        : Handler(c, b, columns) {}

    void handle(const ResultRecord &r) override
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

struct MIBreakpointController::InsertedHandler : public MIBreakpointController::Handler
{
    InsertedHandler(MIBreakpointController* c, const BreakpointDataPtr& b,
                    BreakpointModel::ColumnFlags columns)
        : Handler(c, b, columns) {}

    void handle(const ResultRecord &r) override
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

            breakpoint->debuggerId = miBkpt["number"].toInt();

            if (row >= 0) {
                controller->updateFromDebugger(row, miBkpt);
                if (breakpoint->dirty != 0)
                    controller->sendUpdates(row);
            } else {
                // breakpoint was deleted while insertion was in flight
                controller->debugSession()->addCommand(BreakDelete,
                                                       QString::number(breakpoint->debuggerId),
                                                       CmdImmediately);
            }
        }

        if (row >= 0) {
            controller->recalculateState(row);
        }
    }
};

struct MIBreakpointController::DeleteHandler : MIBreakpointController::Handler {
    DeleteHandler(MIBreakpointController* c, const BreakpointDataPtr& b)
        : Handler(c, b, 0) {}

    void handle(const ResultRecord&) override
    {
        controller->m_pendingDeleted.removeAll(breakpoint);
    }
};

struct MIBreakpointController::IgnoreChanges {
    IgnoreChanges(MIBreakpointController& controller)
        : controller(controller)
    {
        ++controller.m_ignoreChanges;
    }

    ~IgnoreChanges()
    {
        --controller.m_ignoreChanges;
    }

    MIBreakpointController& controller;
};

MIBreakpointController::MIBreakpointController(MIDebugSession * parent)
    : IBreakpointController(parent)
{
    Q_ASSERT(parent);
    connect(parent, &MIDebugSession::inferiorStopped,
            this, &MIBreakpointController::programStopped);

    int numBreakpoints = breakpointModel()->breakpoints().size();
    for (int row = 0; row < numBreakpoints; ++row)
        breakpointAdded(row);
}

MIDebugSession *MIBreakpointController::debugSession() const
{
    Q_ASSERT(QObject::parent());
    return static_cast<MIDebugSession *>(const_cast<QObject*>(QObject::parent()));
}

int MIBreakpointController::breakpointRow(const BreakpointDataPtr& breakpoint)
{
    return m_breakpoints.indexOf(breakpoint);
}

void MIBreakpointController::setDeleteDuplicateBreakpoints(bool enable)
{
    m_deleteDuplicateBreakpoints = enable;
}

void MIBreakpointController::initSendBreakpoints()
{
    for (int row = 0; row < m_breakpoints.size(); ++row) {
        BreakpointDataPtr breakpoint = m_breakpoints[row];
        if (breakpoint->debuggerId < 0 && breakpoint->sent == 0) {
            createBreakpoint(row);
        }
    }
}

void MIBreakpointController::breakpointAdded(int row)
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

    createBreakpoint(row);
}

void MIBreakpointController::breakpointModelChanged(int row, BreakpointModel::ColumnFlags columns)
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

    if (breakpoint->debuggerId < 0) {
        createBreakpoint(row);
    } else {
        sendUpdates(row);
    }
}

void MIBreakpointController::breakpointAboutToBeDeleted(int row)
{
    if (m_ignoreChanges > 0)
        return;

    BreakpointDataPtr breakpoint = m_breakpoints.at(row);
    m_breakpoints.removeAt(row);

    if (breakpoint->debuggerId < 0) {
        // Two possibilities:
        //  (1) Breakpoint has never been sent to GDB, so we're done
        //  (2) Breakpoint has been sent to GDB, but we haven't received
        //      the response yet; the response handler will delete the
        //      breakpoint.
        return;
    }

    if (debugSession()->debuggerStateIsOn(s_dbgNotStarted))
        return;

    debugSession()->addCommand(
            BreakDelete, QString::number(breakpoint->debuggerId),
            new DeleteHandler(this, breakpoint), CmdImmediately);
    m_pendingDeleted << breakpoint;
}

// Note: despite the name, this is in fact session state changed.
void MIBreakpointController::debuggerStateChanged(IDebugSession::DebuggerState state)
{
    IgnoreChanges ignoreChanges(*this);
    if (state == IDebugSession::EndedState ||
        state == IDebugSession::NotStartedState) {
        for (int row = 0; row < m_breakpoints.size(); ++row) {
            updateState(row, Breakpoint::NotStartedState);
        }
    } else if (state == IDebugSession::StartingState) {
        for (int row = 0; row < m_breakpoints.size(); ++row) {
            updateState(row, Breakpoint::DirtyState);
        }
    }
}

void MIBreakpointController::createBreakpoint(int row)
{
    if (debugSession()->debuggerStateIsOn(s_dbgNotStarted))
        return;

    BreakpointDataPtr breakpoint = m_breakpoints.at(row);
    Breakpoint* modelBreakpoint = breakpointModel()->breakpoint(row);

    Q_ASSERT(breakpoint->debuggerId < 0 && breakpoint->sent == 0);

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

        // Note: We rely on '-f' to be automatically added by the MICommand logic
        QString arguments;
        if (!modelBreakpoint->enabled())
            arguments += "-d ";
        if (!modelBreakpoint->condition().isEmpty())
            arguments += QString("-c %0 ").arg(Utils::quoteExpression(modelBreakpoint->condition()));
        if (modelBreakpoint->ignoreHits() != 0)
            arguments += QString("-i %0 ").arg(modelBreakpoint->ignoreHits());
        arguments += Utils::quoteExpression(location);

        BreakpointModel::ColumnFlags sent =
            BreakpointModel::EnableColumnFlag |
            BreakpointModel::ConditionColumnFlag |
            BreakpointModel::IgnoreHitsColumnFlag |
            BreakpointModel::LocationColumnFlag;
        debugSession()->addCommand(BreakInsert, arguments,
                                   new InsertedHandler(this, breakpoint, sent),
                                   CmdImmediately);
    } else {
        QString opt;
        if (modelBreakpoint->kind() == Breakpoint::ReadBreakpoint)
            opt = "-r ";
        else if (modelBreakpoint->kind() == Breakpoint::AccessBreakpoint)
            opt = "-a ";

        debugSession()->addCommand(BreakWatch,
                                   opt + Utils::quoteExpression(modelBreakpoint->location()),
                                   new InsertedHandler(this, breakpoint,
                                                       BreakpointModel::LocationColumnFlag),
                                   CmdImmediately);
    }

    recalculateState(row);
}

void MIBreakpointController::sendUpdates(int row)
{
    if (debugSession()->debuggerStateIsOn(s_dbgNotStarted))
        return;

    BreakpointDataPtr breakpoint = m_breakpoints.at(row);
    Breakpoint* modelBreakpoint = breakpointModel()->breakpoint(row);

    Q_ASSERT(breakpoint->debuggerId >= 0 && breakpoint->sent == 0);

    if (breakpoint->dirty & BreakpointModel::LocationColumnFlag) {
        // Gdb considers locations as fixed, so delete and re-create the breakpoint
        debugSession()->addCommand(BreakDelete,
                                   QString::number(breakpoint->debuggerId), CmdImmediately);
        breakpoint->debuggerId = -1;
        createBreakpoint(row);
        return;
    }

    if (breakpoint->dirty & BreakpointModel::EnableColumnFlag) {
        debugSession()->addCommand(modelBreakpoint->enabled() ? BreakEnable : BreakDisable,
                                   QString::number(breakpoint->debuggerId),
                                   new UpdateHandler(this, breakpoint,
                                                     BreakpointModel::EnableColumnFlag),
                                   CmdImmediately);
    }
    if (breakpoint->dirty & BreakpointModel::IgnoreHitsColumnFlag) {
        debugSession()->addCommand(BreakAfter,
                                   QString("%0 %1").arg(breakpoint->debuggerId)
                                                   .arg(modelBreakpoint->ignoreHits()),
                                   new UpdateHandler(this, breakpoint,
                                                     BreakpointModel::IgnoreHitsColumnFlag),
                                   CmdImmediately);
    }
    if (breakpoint->dirty & BreakpointModel::ConditionColumnFlag) {
        debugSession()->addCommand(BreakCondition,
                                   QString("%0 %1").arg(breakpoint->debuggerId)
                                                   .arg(modelBreakpoint->condition()),
                                   new UpdateHandler(this, breakpoint,
                                                     BreakpointModel::ConditionColumnFlag),
                                   CmdImmediately);
    }

    recalculateState(row);
}

void MIBreakpointController::recalculateState(int row)
{
    BreakpointDataPtr breakpoint = m_breakpoints.at(row);

    if (breakpoint->errors == 0)
        updateErrorText(row, QString());

    Breakpoint::BreakpointState newState = Breakpoint::NotStartedState;
    if (debugSession()->state() != IDebugSession::EndedState &&
        debugSession()->state() != IDebugSession::NotStartedState) {
        if (!debugSession()->debuggerStateIsOn(s_dbgNotStarted)) {
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
    }

    updateState(row, newState);
}

int MIBreakpointController::rowFromDebuggerId(int gdbId) const
{
    for (int row = 0; row < m_breakpoints.size(); ++row) {
        if (gdbId == m_breakpoints[row]->debuggerId)
            return row;
    }
    return -1;
}

void MIBreakpointController::notifyBreakpointCreated(const AsyncRecord& r)
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

    createFromDebugger(miBkpt);
}

void MIBreakpointController::notifyBreakpointModified(const AsyncRecord& r)
{
    const Value& miBkpt = r["bkpt"];
    const int gdbId = miBkpt["number"].toInt();
    const int row = rowFromDebuggerId(gdbId);

    if (row < 0) {
        for (const auto& breakpoint : m_pendingDeleted) {
            if (breakpoint->debuggerId == gdbId) {
                // Received a modification of a breakpoint whose deletion is currently
                // in-flight; simply ignore it.
                return;
            }
        }

        qCWarning(DEBUGGERCOMMON) << "Received a modification of an unknown breakpoint";
        createFromDebugger(miBkpt);
    } else {
        updateFromDebugger(row, miBkpt);
    }
}

void MIBreakpointController::notifyBreakpointDeleted(const AsyncRecord& r)
{
    const int gdbId = r["id"].toInt();
    const int row = rowFromDebuggerId(gdbId);

    if (row < 0) {
        // The user may also have deleted the breakpoint via the UI simultaneously
        return;
    }

    IgnoreChanges ignoreChanges(*this);
    breakpointModel()->removeRow(row);
    m_breakpoints.removeAt(row);
}

void MIBreakpointController::createFromDebugger(const Value& miBkpt)
{
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
        qCWarning(DEBUGGERCOMMON) << "Unknown breakpoint type " << type;
        return;
    }

    // During debugger startup, we want to avoid creating duplicate breakpoints when the same breakpoint
    // appears both in our model and in a init file e.g. .gdbinit
    BreakpointModel* model = breakpointModel();
    const int numRows = model->rowCount();
    for (int row = 0; row < numRows; ++row) {
        BreakpointDataPtr breakpoint = m_breakpoints.at(row);
        const bool breakpointSent = breakpoint->debuggerId >= 0 || breakpoint->sent != 0;
        if (breakpointSent && !m_deleteDuplicateBreakpoints)
            continue;

        Breakpoint* modelBreakpoint = model->breakpoint(row);
        if (modelBreakpoint->kind() != gdbKind)
            continue;

        if (gdbKind == Breakpoint::CodeBreakpoint) {
            bool sameLocation = false;

            if (miBkpt.hasField("fullname") && miBkpt.hasField("line")) {
                const QString location = Utils::unquoteExpression(miBkpt["fullname"].literal());
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
                        Utils::unquoteExpression(rx.cap(1)) == modelBreakpoint->url().url(QUrl::PreferLocalFile | QUrl::StripTrailingSlash) &&
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
            if (Utils::unquoteExpression(miBkpt["original-location"].literal()) != modelBreakpoint->expression()) {
                continue;
            }
        }

        QString condition;
        if (miBkpt.hasField("cond")) {
            condition = miBkpt["cond"].literal();
        }
        if (condition != modelBreakpoint->condition())
            continue;

        // Breakpoint is equivalent
        if (!breakpointSent) {
            breakpoint->debuggerId = miBkpt["number"].toInt();

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

            updateFromDebugger(row, miBkpt, BreakpointModel::EnableColumnFlag | BreakpointModel::IgnoreHitsColumnFlag);
            return;
        }

        // Breakpoint from the model has already been sent, but we want to delete duplicates
        // It is not entirely clear _which_ breakpoint ought to be deleted, and reasonable people
        // may have different opinions.
        // We suspect that it is marginally more useful to delete the existing model breakpoint;
        // after all, this only happens when a user command creates a breakpoint, and perhaps the
        // user intends to modify the breakpoint they created manually. In any case,
        // this situation should only happen rarely (in particular, when a user sets a breakpoint
        // inside the remote run script).
        model->removeRows(row, 1);
        break; // fall through to pick up the manually created breakpoint in the model
    }

    // No equivalent breakpoint found, or we have one but want to be consistent with GDB's
    // behavior of allowing multiple equivalent breakpoint.
    IgnoreChanges ignoreChanges(*this);
    const int row = m_breakpoints.size();
    Q_ASSERT(row == model->rowCount());

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
    breakpoint->debuggerId = miBkpt["number"].toInt();

    updateFromDebugger(row, miBkpt);
}

// This method is required for the legacy interface which will be removed
void MIBreakpointController::sendMaybe(KDevelop::Breakpoint*)
{
    Q_ASSERT(false);
}

void MIBreakpointController::updateFromDebugger(int row, const Value& miBkpt, BreakpointModel::ColumnFlags lockedColumns)
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
            QUrl::fromLocalFile(Utils::unquoteExpression(miBkpt["fullname"].literal())),
            miBkpt["line"].toInt() - 1);
    } else if (miBkpt.hasField("original-location")) {
        QRegExp rx("^(.+):(\\d+)$");
        QString location = miBkpt["original-location"].literal();
        if (rx.indexIn(location) != -1) {
            modelBreakpoint->setLocation(QUrl::fromLocalFile(Utils::unquoteExpression(rx.cap(1))),
                                         rx.cap(2).toInt()-1);
        } else {
            modelBreakpoint->setData(Breakpoint::LocationColumn, Utils::unquoteExpression(location));
        }
    } else if (miBkpt.hasField("what")) {
        modelBreakpoint->setExpression(miBkpt["what"].literal());
    } else {
        qWarning() << "Breakpoint doesn't contain required location/expression data";
    }

    if (!(lockedColumns & BreakpointModel::EnableColumnFlag)) {
        bool enabled = true;
        if (miBkpt.hasField("enabled")) {
            if (miBkpt["enabled"].literal() == "n")
                enabled = false;
        }
        modelBreakpoint->setData(Breakpoint::EnableColumn, enabled ? Qt::Checked : Qt::Unchecked);
        breakpoint->dirty &= ~BreakpointModel::EnableColumnFlag;
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

void MIBreakpointController::programStopped(const AsyncRecord& r)
{
    if (!r.hasField("reason"))
        return;

    const QString reason = r["reason"].literal();

    int debuggerId = -1;
    if (reason == "breakpoint-hit") {
        debuggerId = r["bkptno"].toInt();
    } else if (reason == "watchpoint-trigger") {
        debuggerId = r["wpt"]["number"].toInt();
    } else if (reason == "read-watchpoint-trigger") {
        debuggerId = r["hw-rwpt"]["number"].toInt();
    } else if (reason == "access-watchpoint-trigger") {
        debuggerId = r["hw-awpt"]["number"].toInt();
    }

    if (debuggerId < 0)
        return;

    int row = rowFromDebuggerId(debuggerId);
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
