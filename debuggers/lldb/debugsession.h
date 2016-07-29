/*
 * LLDB Debugger Support
 * Copyright 2016  Aetf <aetf@unlimitedcodeworks.xyz>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef LLDB_DEBUGSESSION_H
#define LLDB_DEBUGSESSION_H

#include "midebugsession.h"

#include "controllers/breakpointcontroller.h"
#include "controllers/framestackmodel.h"
#include "controllers/variablecontroller.h"
#include "lldbdebugger.h"

namespace KDevelop {
class ILaunchConfiguration;
}

namespace KDevMI {

namespace MI {
struct ResultRecord;
}

namespace LLDB {

class LldbDebuggerPlugin;
class DebugSession : public MIDebugSession
{
    Q_OBJECT
public:
    explicit DebugSession(LldbDebuggerPlugin *plugin = nullptr);
    ~DebugSession() override;

    BreakpointController * breakpointController() const override;
    VariableController * variableController() const override;
    LldbFrameStackModel * frameStackModel() const override;

    MI::MICommand *createCommand(MI::CommandType type, const QString &arguments,
                                 MI::CommandFlags flags) const override;

    void updateAllVariables();

public Q_SLOTS:
    void interruptDebugger() override;

protected:
    LldbDebugger *createDebugger() const override;
    void initializeDebugger() override;
    bool execInferior(KDevelop::ILaunchConfiguration *cfg, IExecutePlugin *iexec,
                      const QString &executable) override;

    void configure(KDevelop::ILaunchConfiguration *cfg, IExecutePlugin *iexec);

    void ensureDebuggerListening() override;

    void setupToolviews();
    void unloadToolviews();

private Q_SLOTS:
    void handleFileExecAndSymbols(const MI::ResultRecord& r);
    void handleTargetSelect(const MI::ResultRecord& r);

private:
    BreakpointController *m_breakpointController;
    VariableController *m_variableController;
    LldbFrameStackModel *m_frameStackModel;
};

} // end of namespace GDB
} // end of namespace KDevMI

#endif // LLDB_DEBUGSESSION_H
