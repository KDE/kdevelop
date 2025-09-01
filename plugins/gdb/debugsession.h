/*
    SPDX-FileCopyrightText: 1999-2001 John Birch <jbb@kdevelop.org>
    SPDX-FileCopyrightText: 2001 Bernd Gehrmann <bernd@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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

class IExecutePlugin;

namespace KDevelop {
class ILaunchConfiguration;
}

namespace KDevMI {
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

    void configInferior(KDevelop::ILaunchConfiguration* cfg, IExecutePlugin* iexec, const QString&) override;

    bool execInferior(KDevelop::ILaunchConfiguration* cfg, IExecutePlugin*,
                      const QString& executable) override;

    bool loadCoreFile(KDevelop::ILaunchConfiguration *cfg,
                      const QString &debugee, const QString &corefile) override;

private Q_SLOTS:
    void handleVersion(const QStringList& s);
    void handleFileExecAndSymbols(const MI::ResultRecord& r);
    void handleCoreFile(const MI::ResultRecord& r);

private:
    BreakpointController *m_breakpointController = nullptr;
    VariableController *m_variableController = nullptr;
    GdbFrameStackModel *m_frameStackModel = nullptr;

    bool m_autoDisableASLR = false;
};

} // end of namespace GDB
} // end of namespace KDevMI

#endif
