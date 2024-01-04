/*
    SPDX-FileCopyrightText: 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2002 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2006, 2008 Vladimir Prus <ghost@cs.msu.su>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mibreakpointcontroller.h"

#include "debuglog.h"
#include "midebugsession.h"
#include "mi/micommand.h"
#include "stringhelpers.h"

#include <debugger/breakpoint/breakpoint.h>
#include <debugger/breakpoint/breakpointmodel.h>

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

        if (r.reason == QLatin1String("error")) {
            breakpoint->errors |= columns;

            int row = controller->breakpointRow(breakpoint);
            if (row >= 0) {
                controller->updateErrorText(row, r[QStringLiteral("msg")].literal());
                qCWarning(DEBUGGERCOMMON) << r[QStringLiteral("msg")].literal();
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

        if (r.reason != QLatin1String("error")) {
            QString bkptKind;
            for (auto& kind : {QStringLiteral("bkpt"), QStringLiteral("wpt"), QStringLiteral("hw-rwpt"), QStringLiteral("hw-awpt")}) {
                if (r.hasField(kind)) {
                    bkptKind = kind;
                    break;
                }
            }
            if (bkptKind.isEmpty()) {
                qCWarning(DEBUGGERCOMMON) << "Gdb sent unknown breakpoint kind";
                return;
            }

            const Value& miBkpt = r[bkptKind];

            breakpoint->debuggerId = miBkpt[QStringLiteral("number")].toInt();

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
        : Handler(c, b, BreakpointModel::ColumnFlags()) {}

    void handle(const ResultRecord&) override
    {
        controller->m_pendingDeleted.removeAll(breakpoint);
    }
};

struct MIBreakpointController::IgnoreChanges {
    explicit IgnoreChanges(MIBreakpointController& controller)
        : controller(controller)
    {
        ++controller.m_ignoreChanges;
    }

    ~IgnoreChanges()
    {
        --controller.m_ignoreChanges;
    }

    MIBreakpointController& controller;

private:
    Q_DISABLE_COPY(IgnoreChanges)
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
            location = QStringLiteral("%0:%1")
                .arg(modelBreakpoint->url().url(QUrl::PreferLocalFile | QUrl::StripTrailingSlash))
                .arg(modelBreakpoint->line() + 1);
        } else {
            location = modelBreakpoint->location();
        }

        if (location == QLatin1String("catch throw")) {
            location = QStringLiteral("exception throw");
        }

        // Note: We rely on '-f' to be automatically added by the MICommand logic
        QString arguments;
        if (!modelBreakpoint->enabled())
            arguments += QLatin1String("-d ");
        if (!modelBreakpoint->condition().isEmpty())
            arguments += QStringLiteral("-c %0 ").arg(Utils::quoteExpression(modelBreakpoint->condition()));
        if (modelBreakpoint->ignoreHits() != 0)
            arguments += QStringLiteral("-i %0 ").arg(modelBreakpoint->ignoreHits());
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
            opt = QStringLiteral("-r ");
        else if (modelBreakpoint->kind() == Breakpoint::AccessBreakpoint)
            opt = QStringLiteral("-a ");

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
                                   QStringLiteral("%0 %1").arg(breakpoint->debuggerId)
                                                   .arg(modelBreakpoint->ignoreHits()),
                                   new UpdateHandler(this, breakpoint,
                                                     BreakpointModel::IgnoreHitsColumnFlag),
                                   CmdImmediately);
    }
    if (breakpoint->dirty & BreakpointModel::ConditionColumnFlag) {
        debugSession()->addCommand(BreakCondition,
                                   QStringLiteral("%0 %1").arg(breakpoint->debuggerId)
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
    const Value& miBkpt = r[QStringLiteral("bkpt")];

    // Breakpoints with multiple locations are represented by a parent breakpoint (e.g. 1)
    // and multiple child breakpoints (e.g. 1.1, 1.2, 1.3, ...).
    // We ignore the child breakpoints here in the current implementation; this can lead to dubious
    // results in the UI when breakpoints are marked in document views (e.g. when a breakpoint
    // applies to multiple overloads of a C++ function simultaneously) and in disassembly
    // (e.g. when a breakpoint is set in an inlined functions).
    if (miBkpt[QStringLiteral("number")].literal().contains(QLatin1Char('.')))
        return;

    createFromDebugger(miBkpt);
}

void MIBreakpointController::notifyBreakpointModified(const AsyncRecord& r)
{
    const Value& miBkpt = r[QStringLiteral("bkpt")];
    const int gdbId = miBkpt[QStringLiteral("number")].toInt();
    const int row = rowFromDebuggerId(gdbId);

    if (row < 0) {
        for (const auto& breakpoint : qAsConst(m_pendingDeleted)) {
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
    const int gdbId = r[QStringLiteral("id")].toInt();
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
    const QString type = miBkpt[QStringLiteral("type")].literal();
    Breakpoint::BreakpointKind gdbKind;
    if (type == QLatin1String("breakpoint") || type == QLatin1String("catchpoint")) {
        gdbKind = Breakpoint::CodeBreakpoint;
    } else if (type == QLatin1String("watchpoint") || type == QLatin1String("hw watchpoint")) {
        gdbKind = Breakpoint::WriteBreakpoint;
    } else if (type == QLatin1String("read watchpoint")) {
        gdbKind = Breakpoint::ReadBreakpoint;
    } else if (type == QLatin1String("acc watchpoint")) {
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

            if (miBkpt.hasField(QStringLiteral("fullname")) && miBkpt.hasField(QStringLiteral("line"))) {
                const QString location = Utils::unquoteExpression(miBkpt[QStringLiteral("fullname")].literal());
                const int line = miBkpt[QStringLiteral("line")].toInt() - 1;
                if (location == modelBreakpoint->url().url(QUrl::PreferLocalFile | QUrl::StripTrailingSlash) &&
                    line == modelBreakpoint->line())
                {
                    sameLocation = true;
                }
            }

            if (!sameLocation && miBkpt.hasField(QStringLiteral("original-location"))) {
                const QString location = miBkpt[QStringLiteral("original-location")].literal();
                if (location == modelBreakpoint->location()) {
                    sameLocation = true;
                } else {
                    QRegularExpression rx(QStringLiteral("^(.+):(\\d+)$"));
                    auto match = rx.match(location);
                    if (match.hasMatch() &&
                        Utils::unquoteExpression(match.captured(1)) == modelBreakpoint->url().url(QUrl::PreferLocalFile | QUrl::StripTrailingSlash) &&
                        match.capturedView(2).toInt() - 1 == modelBreakpoint->line()) {
                        sameLocation = true;
                    }
                }
            }

            if (!sameLocation && miBkpt.hasField(QStringLiteral("what")) && miBkpt[QStringLiteral("what")].literal() == QLatin1String("exception throw")) {
                if (modelBreakpoint->expression() == QLatin1String("catch throw") ||
                    modelBreakpoint->expression() == QLatin1String("exception throw")) {
                    sameLocation = true;
                }
            }

            if (!sameLocation)
                continue;
        } else {
            if (Utils::unquoteExpression(miBkpt[QStringLiteral("original-location")].literal()) != modelBreakpoint->expression()) {
                continue;
            }
        }

        QString condition;
        if (miBkpt.hasField(QStringLiteral("cond"))) {
            condition = miBkpt[QStringLiteral("cond")].literal();
        }
        if (condition != modelBreakpoint->condition())
            continue;

        // Breakpoint is equivalent
        if (!breakpointSent) {
            breakpoint->debuggerId = miBkpt[QStringLiteral("number")].toInt();

            // Reasonable people can probably have different opinions about what the "correct" behavior
            // should be for the "enabled" and "ignore hits" column.
            // Here, we let the status in KDevelop's UI take precedence, which we suspect to be
            // marginally more useful. Dirty data will be sent during the initial sending of the
            // breakpoint list.
            const bool gdbEnabled = miBkpt[QStringLiteral("enabled")].literal() == QLatin1String("y");
            if (gdbEnabled != modelBreakpoint->enabled())
                breakpoint->dirty |= BreakpointModel::EnableColumnFlag;

            int gdbIgnoreHits = 0;
            if (miBkpt.hasField(QStringLiteral("ignore")))
                gdbIgnoreHits = miBkpt[QStringLiteral("ignore")].toInt();
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
    breakpoint->debuggerId = miBkpt[QStringLiteral("number")].toInt();

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
    if (miBkpt.hasField(QStringLiteral("fullname")) && miBkpt.hasField(QStringLiteral("line"))) {
        modelBreakpoint->setLocation(
            QUrl::fromLocalFile(Utils::unquoteExpression(miBkpt[QStringLiteral("fullname")].literal())),
            miBkpt[QStringLiteral("line")].toInt() - 1);
    } else if (miBkpt.hasField(QStringLiteral("original-location"))) {
        QRegularExpression rx(QStringLiteral("^(.+):(\\d+)$"));
        QString location = miBkpt[QStringLiteral("original-location")].literal();
        const auto match = rx.match(location);
        if (match.hasMatch()) {
            modelBreakpoint->setLocation(QUrl::fromLocalFile(Utils::unquoteExpression(match.captured(1))),
                                         match.capturedView(2).toInt()-1);
        } else {
            modelBreakpoint->setData(Breakpoint::LocationColumn, Utils::unquoteExpression(location));
        }
    } else if (miBkpt.hasField(QStringLiteral("what"))) {
        modelBreakpoint->setExpression(miBkpt[QStringLiteral("what")].literal());
    } else {
        qCWarning(DEBUGGERCOMMON) << "Breakpoint doesn't contain required location/expression data";
    }

    if (!(lockedColumns & BreakpointModel::EnableColumnFlag)) {
        bool enabled = true;
        if (miBkpt.hasField(QStringLiteral("enabled"))) {
            if (miBkpt[QStringLiteral("enabled")].literal() == QLatin1String("n"))
                enabled = false;
        }
        modelBreakpoint->setData(Breakpoint::EnableColumn, enabled ? Qt::Checked : Qt::Unchecked);
        breakpoint->dirty &= ~BreakpointModel::EnableColumnFlag;
    }

    if (!(lockedColumns & BreakpointModel::ConditionColumnFlag)) {
        QString condition;
        if (miBkpt.hasField(QStringLiteral("cond"))) {
            condition = miBkpt[QStringLiteral("cond")].literal();
        }
        modelBreakpoint->setCondition(condition);
        breakpoint->dirty &= ~BreakpointModel::ConditionColumnFlag;
    }

    if (!(lockedColumns & BreakpointModel::IgnoreHitsColumnFlag)) {
        int ignoreHits = 0;
        if (miBkpt.hasField(QStringLiteral("ignore"))) {
            ignoreHits = miBkpt[QStringLiteral("ignore")].toInt();
        }
        modelBreakpoint->setIgnoreHits(ignoreHits);
        breakpoint->dirty &= ~BreakpointModel::IgnoreHitsColumnFlag;
    }

    breakpoint->pending = false;
    if (miBkpt.hasField(QStringLiteral("addr")) && miBkpt[QStringLiteral("addr")].literal() == QLatin1String("<PENDING>")) {
        breakpoint->pending = true;
    }

    int hitCount = 0;
    if (miBkpt.hasField(QStringLiteral("times"))) {
        hitCount = miBkpt[QStringLiteral("times")].toInt();
    }
    updateHitCount(row, hitCount);

    recalculateState(row);
}

void MIBreakpointController::programStopped(const AsyncRecord& r)
{
    if (!r.hasField(QStringLiteral("reason")))
        return;

    const QString reason = r[QStringLiteral("reason")].literal();

    int debuggerId = -1;
    if (reason == QLatin1String("breakpoint-hit")) {
        debuggerId = r[QStringLiteral("bkptno")].toInt();
    } else if (reason == QLatin1String("watchpoint-trigger")) {
        debuggerId = r[QStringLiteral("wpt")][QStringLiteral("number")].toInt();
    } else if (reason == QLatin1String("read-watchpoint-trigger")) {
        debuggerId = r[QStringLiteral("hw-rwpt")][QStringLiteral("number")].toInt();
    } else if (reason == QLatin1String("access-watchpoint-trigger")) {
        debuggerId = r[QStringLiteral("hw-awpt")][QStringLiteral("number")].toInt();
    }

    if (debuggerId < 0)
        return;

    int row = rowFromDebuggerId(debuggerId);
    if (row < 0)
        return;

    QString msg;
    if (r.hasField(QStringLiteral("value"))) {
        if (r[QStringLiteral("value")].hasField(QStringLiteral("old"))) {
            msg += i18n("<br>Old value: %1", r[QStringLiteral("value")][QStringLiteral("old")].literal());
        }
        if (r[QStringLiteral("value")].hasField(QStringLiteral("new"))) {
            msg += i18n("<br>New value: %1", r[QStringLiteral("value")][QStringLiteral("new")].literal());
        }
    }

    notifyHit(row, msg);
}

#include "moc_mibreakpointcontroller.cpp"
