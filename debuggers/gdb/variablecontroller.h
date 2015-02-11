/*
 * GDB Debugger Support
 *
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2008 Vladimir Prus <ghost@cs.msu.su>
 * Copyright 2009 Niko Sams <niko.sams@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef GDBDEBUGGER_VARIABLECONTROLLER_H
#define GDBDEBUGGER_VARIABLECONTROLLER_H

#include <debugger/interfaces/ivariablecontroller.h>

#include "gdbglobal.h"

using namespace KDevelop;

namespace GDBMI {
struct AsyncRecord;
struct ResultRecord;
struct Value;
}

namespace GDBDebugger {

class GDBController;
class DebugSession;

class VariableController : public KDevelop::IVariableController
{
    Q_OBJECT

public:
    VariableController(DebugSession* parent);

    virtual Variable* createVariable(TreeModel* model, TreeItem* parent, 
                                     const QString& expression,
                                     const QString& display = "") override;
    virtual QString expressionUnderCursor(KTextEditor::Document* doc, const KTextEditor::Cursor& cursor) override;
    virtual void addWatch(KDevelop::Variable* variable) override;
    virtual void addWatchpoint(KDevelop::Variable* variable) override;
    virtual void update() override;

private slots:
    void programStopped(const GDBMI::AsyncRecord &r);
    void stateChanged(KDevelop::IDebugSession::DebuggerState);

private:
    DebugSession* debugSession() const;

    void updateLocals();

    void handleVarUpdate(const GDBMI::ResultRecord& r);
    void addWatch(const GDBMI::ResultRecord& r);
    void addWatchpoint(const GDBMI::ResultRecord& r);

    void handleEvent(IDebugSession::event_t event) override;
};

}

#endif // GDBDEBUGGER_VARIABLECONTROLLER_H
