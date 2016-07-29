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

#include "debuggerplugin.h"

#include "debuglog.h"
#include "lldblauncher.h"
#include "widgets/lldbconfigpage.h"
#include "widgets/debuggerconsoleview.h"

#include <execute/iexecuteplugin.h>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/launchconfigurationtype.h>

#include <KPluginFactory>

using namespace KDevMI::LLDB;

K_PLUGIN_FACTORY_WITH_JSON(LldbDebuggerFactory, "kdevlldb.json", registerPlugin<LldbDebuggerPlugin>(); )

LldbDebuggerPlugin::LldbDebuggerPlugin(QObject *parent, const QVariantList &)
    : MIDebuggerPlugin("kdevlldb", parent)
    , m_consoleFactory(nullptr)
    , m_disassembleFactory(nullptr)
{
    setXMLFile("kdevlldbui.rc");

    auto plugins = core()->pluginController()->allPluginsForExtension("org.kdevelop.IExecutePlugin");
    for (auto plugin : plugins) {
        auto iexec = plugin->extension<IExecutePlugin>();
        Q_ASSERT(iexec);

        auto type = core()->runController()->launchConfigurationTypeForId(iexec->nativeAppConfigTypeId());
        Q_ASSERT(type);
        type->addLauncher(new LldbLauncher(this, iexec));
    }
}

void LldbDebuggerPlugin::setupToolviews()
{
    m_consoleFactory = new DebuggerToolFactory<NonInterruptDebuggerConsoleView>(this,
                            "org.kdevelop.debugger.LldbConsole", Qt::BottomDockWidgetArea);
    core()->uiController()->addToolView(i18n("LLDB Console"), m_consoleFactory);
    /*
    m_disassembleFactory = new DebuggerToolFactory<DisassembleWidget>(this,
                            "org.kdevelop.debugger.LldbDisassemble", Qt::BottomDockWidgetArea);
    core()->uiController()->addToolView(i18n("LLDB Disassemble/Register"), m_disassembleFactory);
    */
}

void LldbDebuggerPlugin::unloadToolviews()
{
    if (m_consoleFactory) {
        qCDebug(DEBUGGERLLDB) << "Removing toolview";
        core()->uiController()->removeToolView(m_consoleFactory);
        m_consoleFactory = nullptr;
    }
    /*
    core()->uiController()->removeToolView(m_disassembleFactory);
    core()->uiController()->removeToolView(memoryviewerfactory);
    */
}

LldbDebuggerPlugin::~LldbDebuggerPlugin()
{
}

DebugSession* LldbDebuggerPlugin::createSession()
{
    DebugSession *session = new DebugSession(this);
    core()->debugController()->addSession(session);
    connect(session, &DebugSession::showMessage, this, &LldbDebuggerPlugin::showStatusMessage);
    connect(session, &DebugSession::reset, this, &LldbDebuggerPlugin::reset);
    connect(session, &DebugSession::raiseDebuggerConsoleViews,
            this, &LldbDebuggerPlugin::raiseDebuggerConsoleViews);
    return session;
}

#include "debuggerplugin.moc"
