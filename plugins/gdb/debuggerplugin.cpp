/*
    SPDX-FileCopyrightText: 1999-2001 John Birch <jbb@kdevelop.org>
    SPDX-FileCopyrightText: 2001 Bernd Gehrmann <bernd@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Vladimir Prus <ghost@cs.msu.su>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "debuggerplugin.h"

#include "config-gdb-plugin.h"

#include "widgets/disassemblewidget.h"
#include "memviewdlg.h"
#include "gdboutputwidget.h"

#include "gdbconfigpage.h"
#include "debugsession.h"

#include <execute/iexecuteplugin.h>
#include <interfaces/icore.h>
#include <interfaces/idebugcontroller.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/launchconfigurationtype.h>

#include <KPluginFactory>

// explicit init of resources needed, because all files
// are first compiled into a static library which is also used for unit testing
// for some reason the respective resource init methods are not triggered or registered then
inline void initMyResource() { Q_INIT_RESOURCE(kdevgdb); }

using namespace KDevMI::GDB;

K_PLUGIN_FACTORY_WITH_JSON(CppDebuggerFactory, "kdevgdb.json", registerPlugin<CppDebuggerPlugin>(); )

CppDebuggerPlugin::CppDebuggerPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList&)
    : MIDebuggerPlugin(QStringLiteral("kdevgdb"), i18n("GDB"), parent, metaData)
    , disassemblefactory(nullptr)
    , gdbfactory(nullptr)
    , memoryviewerfactory(nullptr)
{
    initMyResource();

    setXMLFile(QStringLiteral("kdevgdbui.rc"));


    auto pluginController = core()->pluginController();
    const auto plugins = pluginController->allPluginsForExtension(QStringLiteral("org.kdevelop.IExecutePlugin"));
    for (auto plugin : plugins) {
        setupExecutePlugin(plugin, true);
    }

    connect(pluginController, &KDevelop::IPluginController::pluginLoaded,
            this, [this](KDevelop::IPlugin* plugin) {
                setupExecutePlugin(plugin, true);
            });

    connect(pluginController, &KDevelop::IPluginController::unloadingPlugin,
            this, [this](KDevelop::IPlugin* plugin) {
                setupExecutePlugin(plugin, false);
            });
}

void CppDebuggerPlugin::unload()
{
    const auto plugins = core()->pluginController()->allPluginsForExtension(QStringLiteral("org.kdevelop.IExecutePlugin"));
    for (auto plugin : plugins) {
        setupExecutePlugin(plugin, false);
    }
    Q_ASSERT(m_launchers.isEmpty());
}

void CppDebuggerPlugin::setupExecutePlugin(KDevelop::IPlugin* plugin, bool load)
{
    if (plugin == this) {
        return;
    }

    auto iface = plugin->extension<IExecutePlugin>();
    if (!iface) {
        return;
    }

    auto type = core()->runController()->launchConfigurationTypeForId(iface->nativeAppConfigTypeId());
    Q_ASSERT(type);

    if (load) {
        auto launcher = new GdbLauncher(this, iface);
        m_launchers.insert(plugin, launcher);
        type->addLauncher(launcher);
    } else {
        auto launcher = m_launchers.take(plugin);
        Q_ASSERT(launcher);

        type->removeLauncher(launcher);
        delete launcher;
    }
}

void CppDebuggerPlugin::setupToolViews()
{
    disassemblefactory = new DebuggerToolFactory<DisassembleWidget>(
    this, QStringLiteral("org.kdevelop.debugger.DisassemblerView"), Qt::BottomDockWidgetArea);

    gdbfactory = new DebuggerToolFactory<GDBOutputWidget, CppDebuggerPlugin>(
    this, QStringLiteral("org.kdevelop.debugger.ConsoleView"),Qt::BottomDockWidgetArea);

    core()->uiController()->addToolView(
        i18nc("@title:window", "Disassemble/Registers"),
        disassemblefactory);

    core()->uiController()->addToolView(
        i18nc("@title:window", "GDB"),
        gdbfactory);

#ifndef KDEV_WITH_MEMVIEW
    memoryviewerfactory = nullptr;
#else
    memoryviewerfactory = new DebuggerToolFactory<MemoryViewerWidget, CppDebuggerPlugin>(
    this, QStringLiteral("org.kdevelop.debugger.MemoryView"), Qt::BottomDockWidgetArea);
    core()->uiController()->addToolView(
        i18nc("@title:window", "Memory"),
        memoryviewerfactory);
#endif
}

void CppDebuggerPlugin::unloadToolViews()
{
    if (disassemblefactory) {
        core()->uiController()->removeToolView(disassemblefactory);
        disassemblefactory = nullptr;
    }
    if (gdbfactory) {
        core()->uiController()->removeToolView(gdbfactory);
        gdbfactory = nullptr;
    }
    if (memoryviewerfactory) {
        core()->uiController()->removeToolView(memoryviewerfactory);
        memoryviewerfactory = nullptr;
    }
}

CppDebuggerPlugin::~CppDebuggerPlugin()
{
}

DebugSession* CppDebuggerPlugin::createSession()
{
    auto *session = new DebugSession(this);
    KDevelop::ICore::self()->debugController()->addSession(session);
    connect(session, &DebugSession::showMessage, this, &CppDebuggerPlugin::showStatusMessage);
    connect(session, &DebugSession::reset, this, &CppDebuggerPlugin::reset);
    connect(session, &DebugSession::raiseDebuggerConsoleViews,
            this, &CppDebuggerPlugin::raiseDebuggerConsoleViews);
    return session;
}

#include "debuggerplugin.moc"
#include "moc_debuggerplugin.cpp"
