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
#ifndef BREAKPOINTCONTROLLER_H
#define BREAKPOINTCONTROLLER_H

#include <QObject>

#include <debugger/interfaces/ibreakpointcontroller.h>
#include <debugger/interfaces/idebugsession.h>

#include "gdbglobal.h"

class QModelIndex;

namespace GDBMI {
struct AsyncRecord;
struct ResultRecord;
struct Value;
}

namespace GDBDebugger
{
using namespace KDevelop;

class DebugSession;
struct BreakpointData;

typedef QSharedPointer<BreakpointData> BreakpointDataPtr;

/**
* Handles signals from the editor that relate to breakpoints and the execution
* point of the debugger.
* We may change, add or remove breakpoints in this class.
*/
class BreakpointController : public IBreakpointController
{
    Q_OBJECT
public:
    BreakpointController(DebugSession* parent);

    using IBreakpointController::breakpointModel;

    void initSendBreakpoints();

    virtual void breakpointAdded(int row) override;
    virtual void breakpointModelChanged(int row, BreakpointModel::ColumnFlags columns) override;
    virtual void breakpointAboutToBeDeleted(int row) override;
    virtual void debuggerStateChanged(IDebugSession::DebuggerState) override;

    void notifyBreakpointCreated(const GDBMI::AsyncRecord& r);
    void notifyBreakpointModified(const GDBMI::AsyncRecord& r);
    void notifyBreakpointDeleted(const GDBMI::AsyncRecord& r);

private Q_SLOTS:
    void programStopped(const GDBMI::AsyncRecord &r);

private:
    DebugSession* debugSession() const;

    int breakpointRow(const BreakpointDataPtr& breakpoint);
    void createGdbBreakpoint(int row);
    void sendUpdates(int row);
    void recalculateState(int row);

    virtual void sendMaybe(KDevelop::Breakpoint *breakpoint);

    void createFromGdb(const GDBMI::Value& miBkpt);
    void updateFromGdb(int row, const GDBMI::Value& miBkpt, BreakpointModel::ColumnFlags lockedColumns = 0);

    int rowFromGdbId(int gdbId) const;

    struct Handler;
    struct InsertedHandler;
    struct UpdateHandler;
    struct DeleteHandler;
    struct IgnoreChanges;

    QList<BreakpointDataPtr> m_breakpoints;
    QList<BreakpointDataPtr> m_pendingDeleted;
    int m_ignoreChanges;
};

}

#endif
