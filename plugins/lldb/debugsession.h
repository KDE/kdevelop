/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

class DebugSession : public MIDebugSession
{
    Q_OBJECT
public:
    explicit DebugSession();
    ~DebugSession() override;

    BreakpointController * breakpointController() const override;
    VariableController * variableController() const override;
    LldbFrameStackModel * frameStackModel() const override;

    std::unique_ptr<MI::MICommand> createCommand(MI::CommandType type, const QString& arguments,
                                                 MI::CommandFlags flags) const override;

    void updateAllVariables();

    void setFormatterPath(const QString &path);

public Q_SLOTS:
    void interruptDebugger() override;

protected:
    LldbDebugger *createDebugger() const override;
    void initializeDebugger() override;

    void configInferior(KDevelop::ILaunchConfiguration *cfg, IExecutePlugin *iexec,
                        const QString &executable) override;

    bool execInferior(KDevelop::ILaunchConfiguration *cfg, IExecutePlugin *iexec,
                      const QString &executable) override;
    void loadCoreFile(const QString& coreFile) override;

    void ensureDebuggerListening() override;

    void setupToolViews();
    void unloadToolViews();

private Q_SLOTS:
    void handleTargetSelect(const MI::ResultRecord &r);
    void handleCoreFile(const QStringList &s);
    void handleVersion(const QStringList& s);

    void handleSessionStateChange(IDebugSession::DebuggerState state);

private:
    BreakpointController *m_breakpointController = nullptr;
    VariableController *m_variableController = nullptr;
    LldbFrameStackModel *m_frameStackModel = nullptr;

    QString m_formatterPath;
};

} // end of namespace GDB
} // end of namespace KDevMI

#endif // LLDB_DEBUGSESSION_H
