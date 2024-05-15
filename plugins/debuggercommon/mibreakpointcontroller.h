/*
    SPDX-FileCopyrightText: 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2002 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MIBREAKPOINTCONTROLLER_H
#define MIBREAKPOINTCONTROLLER_H

#include "dbgglobal.h"

#include <debugger/interfaces/ibreakpointcontroller.h>
#include <debugger/interfaces/idebugsession.h>

namespace KDevMI {

namespace MI {
struct AsyncRecord;
struct ResultRecord;
struct Value;
}

struct BreakpointData {
    int debuggerId = -1;
    KDevelop::BreakpointModel::ColumnFlags dirty;
    KDevelop::BreakpointModel::ColumnFlags sent;
    KDevelop::BreakpointModel::ColumnFlags errors;
    bool pending = false;

    BreakpointData()
    {}
};

using BreakpointDataPtr = QSharedPointer<BreakpointData>;

class MIDebugSession;
/**
* Handles signals from the editor that relate to breakpoints and the execution
* point of the debugger.
* We may change, add or remove breakpoints in this class.
*/
class MIBreakpointController : public KDevelop::IBreakpointController
{
    Q_OBJECT
public:
    explicit MIBreakpointController(MIDebugSession* parent);

    using IBreakpointController::breakpointModel;

    /**
     * Controls whether when duplicate breakpoints are received via async notification from GDB,
     * one of the duplicates will be deleted.
     */
    void setDeleteDuplicateBreakpoints(bool enable);

    void breakpointAdded(int row) override;
    void breakpointModelChanged(int row, KDevelop::BreakpointModel::ColumnFlags columns) override;
    void breakpointAboutToBeDeleted(int row) override;
    void debuggerStateChanged(KDevelop::IDebugSession::DebuggerState) override;

    void notifyBreakpointCreated(const MI::AsyncRecord& r);
    void notifyBreakpointModified(const MI::AsyncRecord& r);
    void notifyBreakpointDeleted(const MI::AsyncRecord& r);

public Q_SLOTS:
    void initSendBreakpoints();

private Q_SLOTS:
    void programStopped(const MI::AsyncRecord &r);

private:
    virtual QLatin1String pendingBreakpointAddress() const = 0;

    MIDebugSession* debugSession() const;

    int breakpointRow(const BreakpointDataPtr& breakpoint);
    void createBreakpoint(int row);
    void sendUpdates(int row);
    void recalculateState(int row);

    void sendMaybe(KDevelop::Breakpoint *breakpoint) override;

    void createFromDebugger(const MI::Value& miBkpt);
    void updateFromDebugger(int row, const MI::Value& miBkpt,
                            KDevelop::BreakpointModel::ColumnFlags lockedColumns = {});

    int rowFromDebuggerId(int gdbId) const;

    struct Handler;
    struct InsertedHandler;
    struct UpdateHandler;
    struct DeleteHandler;
    struct IgnoreChanges;

    QList<BreakpointDataPtr> m_breakpoints;
    QList<BreakpointDataPtr> m_pendingDeleted;
    int m_ignoreChanges = 0;
    bool m_deleteDuplicateBreakpoints = false;
};

} // end of namespace KDevMI

#endif // MIBREAKPOINTCONTROLLER_H
