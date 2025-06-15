/*
    SPDX-FileCopyrightText: 2006 Vladimir Prus <ghost@cs.msu.su>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2025 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "toolviewfactoryholder.h"

#include "debuggerplugin.h"
#include "gdboutputwidget.h"
#include "memviewdlg.h"

#include <config-gdb-plugin.h>
#include <debuglog.h>

#include "widgets/disassemblewidget.h"

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>

#include <KLocalizedString>

using namespace KDevMI::GDB;

ToolViewFactoryHolder::ToolViewFactoryHolder(CppDebuggerPlugin* plugin)
{
    qCDebug(DEBUGGERGDB) << "registering GDB tool views for" << plugin;
    auto* const uiController = KDevelop::ICore::self()->uiController();

    m_disassembleFactory = new Factory<DisassembleWidget>(
        plugin, QStringLiteral("org.kdevelop.debugger.DisassemblerView"), Qt::BottomDockWidgetArea);
    uiController->addToolView(i18nc("@title:window", "Disassemble/Registers"), m_disassembleFactory);

    m_gdbOutputFactory = new Factory<GDBOutputWidget, CppDebuggerPlugin>(
        plugin, QStringLiteral("org.kdevelop.debugger.ConsoleView"), Qt::BottomDockWidgetArea);
    uiController->addToolView(i18nc("@title:window", "GDB"), m_gdbOutputFactory);

#ifdef KDEV_WITH_MEMVIEW
    m_memoryViewerFactory = new Factory<MemoryViewerWidget, CppDebuggerPlugin>(
        plugin, QStringLiteral("org.kdevelop.debugger.MemoryView"), Qt::BottomDockWidgetArea);
    uiController->addToolView(i18nc("@title:window", "Memory"), m_memoryViewerFactory);
#endif
}

ToolViewFactoryHolder::~ToolViewFactoryHolder()
{
    auto* const core = KDevelop::ICore::self();
    if (!core || core->shuttingDown()) {
        // NOTE: cannot print plugin() because it may return a dangling pointer if the factories are already destroyed.
        qCDebug(DEBUGGERGDB) << "not unregistering GDB tool views, because KDevelop is exiting";
        return;
    }
    qCDebug(DEBUGGERGDB) << "unregistering GDB tool views for" << plugin();
    auto* const uiController = core->uiController();

    uiController->removeToolView(m_disassembleFactory);
    uiController->removeToolView(m_gdbOutputFactory);
#ifdef KDEV_WITH_MEMVIEW
    uiController->removeToolView(m_memoryViewerFactory);
#endif
}

const KDevMI::MIDebuggerPlugin* ToolViewFactoryHolder::plugin() const
{
    return m_gdbOutputFactory->data();
}
