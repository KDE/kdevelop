/*
 * GDB Debugger Support
 *
 * Copyright 1999-2001 John Birch <jbb@kdevelop.org>
 * Copyright 2001 by Bernd Gehrmann <bernd@kdevelop.org>
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
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


#ifndef GDB_DEBUGSESSION_H
#define GDB_DEBUGSESSION_H

#include "midebugsession.h"

#include "dbgglobal.h"
#include "gdb.h"
#include "gdbbreakpointcontroller.h"
#include "gdbframestackmodel.h"
#include "variablecontroller.h"
#include "mi/mi.h"


#include <KConfigGroup>

#include <QPointer>
#include <QScopedPointer>


class IExecutePlugin;
class KToolBar;

namespace KDevelop {
class ProcessLineMaker;
class ILaunchConfiguration;
}

namespace KDevMI {

class STTY;

namespace MI {
class MICommand;
class CommandQueue;
}

namespace GDB {

class CppDebuggerPlugin;
class DebugSession : public MIDebugSession
{
    Q_OBJECT
public:
    explicit DebugSession(CppDebuggerPlugin *plugin = nullptr);
    ~DebugSession() override;

    BreakpointController * breakpointController() const override;
    VariableController * variableController() const override;
    GdbFrameStackModel * frameStackModel() const override;

    /// FIXME: only used in unit test currently, potentially could
    /// be made a user configurable option.
    /// Whether turn off auto-disable ASLR when starting inferiors
    void setAutoDisableASLR(bool enable);

protected:
    GdbDebugger *createDebugger() const override;
    void initializeDebugger() override;
    bool execInferior(KDevelop::ILaunchConfiguration *cfg, IExecutePlugin *iexec,
                      const QString &executable) override;

    void configure(KDevelop::ILaunchConfiguration *cfg, IExecutePlugin *iexec);

private Q_SLOTS:
    void handleVersion(const QStringList& s);
    void handleFileExecAndSymbols(const MI::ResultRecord& r);

private:
    friend class GdbTest;

    BreakpointController *m_breakpointController;
    VariableController *m_variableController;
    GdbFrameStackModel *m_frameStackModel;

    bool m_autoDisableASLR;
};

} // end of namespace GDB
} // end of namespace KDevMI

#endif
