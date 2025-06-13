/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "debuggerplugin.h"

#include "debuglog.h"
#include "debugsession.h"
#include "lldblauncher.h"
#include "widgets/debuggerconsoleview.h"

#include <execute/iexecuteplugin.h>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/launchconfigurationtype.h>

#include <KLocalizedString>
#include <KPluginFactory>

using namespace KDevMI::LLDB;

inline void initMyResource() { Q_INIT_RESOURCE(kdevlldb); }

K_PLUGIN_FACTORY_WITH_JSON(LldbDebuggerFactory, "kdevlldb.json", registerPlugin<LldbDebuggerPlugin>(); )

LldbDebuggerPlugin::LldbDebuggerPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList&)
    : MIDebuggerPlugin(QStringLiteral("kdevlldb"), i18n("LLDB"), parent, metaData)
    , m_consoleFactory(nullptr)
{
    initMyResource();

    setXMLFile(QStringLiteral("kdevlldbui.rc"));

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

void LldbDebuggerPlugin::unload()
{
    const auto plugins = core()->pluginController()->allPluginsForExtension(QStringLiteral("org.kdevelop.IExecutePlugin"));
    for (auto plugin : plugins) {
        setupExecutePlugin(plugin, false);
    }
    Q_ASSERT(m_launchers.isEmpty());
}

void LldbDebuggerPlugin::setupExecutePlugin(KDevelop::IPlugin* plugin, bool load)
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
        auto launcher = new LldbLauncher(this, iface);
        m_launchers.insert(plugin, launcher);
        type->addLauncher(launcher);
    } else {
        auto launcher = m_launchers.take(plugin);
        Q_ASSERT(launcher);

        type->removeLauncher(launcher);
        delete launcher;
    }
}

void LldbDebuggerPlugin::setupToolViews()
{
    m_consoleFactory = new DebuggerToolFactory<NonInterruptDebuggerConsoleView>(this,
                            QStringLiteral("org.kdevelop.debugger.LldbConsole"), Qt::BottomDockWidgetArea);
    core()->uiController()->addToolView(i18nc("@title:window", "LLDB Console"), m_consoleFactory);
}

void LldbDebuggerPlugin::unloadToolViews()
{
    if (m_consoleFactory) {
        qCDebug(DEBUGGERLLDB) << "Removing tool view";
        core()->uiController()->removeToolView(m_consoleFactory);
        m_consoleFactory = nullptr;
    }
}

LldbDebuggerPlugin::~LldbDebuggerPlugin()
{
}

KDevMI::MIDebugSession* LldbDebuggerPlugin::createSessionObject()
{
    return new DebugSession(this);
}

#include "debuggerplugin.moc"
#include "moc_debuggerplugin.cpp"
