/* This file is part of the KDE project
   Copyright (C) 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2002 John Firebaugh <jfirebaugh@kde.org>
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
#ifndef MIBREAKPOINTCONTROLLER_H
#define MIBREAKPOINTCONTROLLER_H

#include "dbgglobal.h"

#include <debugger/interfaces/ibreakpointcontroller.h>
#include <debugger/interfaces/idebugsession.h>

#include <QObject>

class QModelIndex;

namespace KDevMI {

namespace MI {
struct AsyncRecord;
struct ResultRecord;
struct Value;
}

struct BreakpointData {
    int debuggerId;
    KDevelop::BreakpointModel::ColumnFlags dirty;
    KDevelop::BreakpointModel::ColumnFlags sent;
    KDevelop::BreakpointModel::ColumnFlags errors;
    bool pending;

    BreakpointData()
        : debuggerId(-1)
        , pending(false)
    {}
};

typedef QSharedPointer<BreakpointData> BreakpointDataPtr;

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
    MIBreakpointController(MIDebugSession* parent);

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
    MIDebugSession* debugSession() const;

    int breakpointRow(const BreakpointDataPtr& breakpoint);
    void createBreakpoint(int row);
    void sendUpdates(int row);
    void recalculateState(int row);

    void sendMaybe(KDevelop::Breakpoint *breakpoint) override;

    void createFromDebugger(const MI::Value& miBkpt);
    void updateFromDebugger(int row, const MI::Value& miBkpt,
                       KDevelop::BreakpointModel::ColumnFlags lockedColumns = 0);

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
