/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>
    SPDX-FileCopyrightText: 2025 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "toolviewfactoryholder.h"

#include "midebuggerplugin.h"
#include "widgets/debuggerconsoleview.h"

#include <debuglog.h>

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>

#include <KLocalizedString>

using namespace KDevMI::LLDB;

class KDevMI::LLDB::NonInterruptDebuggerConsoleView : public DebuggerConsoleView
{
    Q_OBJECT
public:
    explicit NonInterruptDebuggerConsoleView(MIDebuggerPlugin* plugin, QWidget* parent = nullptr)
        : DebuggerConsoleView(plugin, parent)
    {
        setShowInterrupt(false);
        setReplacePrompt(QStringLiteral("(lldb)"));
    }
};

ToolViewFactoryHolder::ToolViewFactoryHolder(MIDebuggerPlugin* plugin)
{
    qCDebug(DEBUGGERLLDB) << "registering LLDB tool views for" << plugin;
    auto* const uiController = KDevelop::ICore::self()->uiController();

    m_debuggerConsoleFactory = new Factory<NonInterruptDebuggerConsoleView>(
        plugin, QStringLiteral("org.kdevelop.debugger.LldbConsole"), Qt::BottomDockWidgetArea);
    uiController->addToolView(i18nc("@title:window", "LLDB Console"), m_debuggerConsoleFactory);
}

ToolViewFactoryHolder::~ToolViewFactoryHolder()
{
    auto* const core = KDevelop::ICore::self();
    if (!core || core->shuttingDown()) {
        // NOTE: cannot print plugin() because it may return a dangling pointer if the factories are already destroyed.
        qCDebug(DEBUGGERLLDB) << "not unregistering LLDB tool views, because KDevelop is exiting";
        return;
    }
    qCDebug(DEBUGGERLLDB) << "unregistering LLDB tool views for" << plugin();
    auto* const uiController = core->uiController();

    uiController->removeToolView(m_debuggerConsoleFactory);
}

const KDevMI::MIDebuggerPlugin* ToolViewFactoryHolder::plugin() const
{
    return m_debuggerConsoleFactory->data();
}

#include "toolviewfactoryholder.moc"
