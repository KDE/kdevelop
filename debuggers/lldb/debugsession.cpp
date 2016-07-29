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

#include "debugsession.h"

#include "controllers/variable.h"
#include "dbgglobal.h"
#include "debuggerplugin.h"
#include "debuglog.h"
#include "lldbcommand.h"
#include "mi/micommand.h"
#include "stty.h"

#include <debugger/breakpoint/breakpoint.h>
#include <debugger/breakpoint/breakpointmodel.h>
#include <execute/iexecuteplugin.h>
#include <interfaces/icore.h>
#include <interfaces/idebugcontroller.h>
#include <interfaces/ilaunchconfiguration.h>
#include <util/environmentgrouplist.h>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KShell>

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>
#include <QGuiApplication>

using namespace KDevMI::LLDB;
using namespace KDevMI::MI;
using namespace KDevelop;

QString doubleQuoteArg(QString arg)
{
    arg.replace("\"", "\\\"");
    return '"' + arg + '"';
}

DebugSession::DebugSession(LldbDebuggerPlugin *plugin)
    : MIDebugSession(plugin)
    , m_breakpointController(nullptr)
    , m_variableController(nullptr)
    , m_frameStackModel(nullptr)
{
    m_breakpointController = new BreakpointController(this);
    m_variableController = new VariableController(this);
    m_frameStackModel = new LldbFrameStackModel(this);

    if (m_plugin) m_plugin->setupToolviews();
}

DebugSession::~DebugSession()
{
    if (m_plugin) m_plugin->unloadToolviews();
}

BreakpointController *DebugSession::breakpointController() const
{
    return m_breakpointController;
}

VariableController *DebugSession::variableController() const
{
    return m_variableController;
}

LldbFrameStackModel *DebugSession::frameStackModel() const
{
    return m_frameStackModel;
}

LldbDebugger *DebugSession::createDebugger() const
{
    return new LldbDebugger;
}

MICommand *DebugSession::createCommand(MI::CommandType type, const QString& arguments,
                                       MI::CommandFlags flags) const
{
    return new LldbCommand(type, arguments, flags);
}

void DebugSession::initializeDebugger()
{
    //addCommand(MI::EnableTimings, "yes");

    // TODO: lldb data formatter
    /*
    QString fileName = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                              "kdevlldb/printers/lldbinit");
    if (!fileName.isEmpty()) {
        QFileInfo fileInfo(fileName);
        QString quotedPrintersPath = fileInfo.dir().path()
                                             .replace('\\', "\\\\")
                                             .replace('"', "\\\"");
        queueCmd(new MICommand(MI::NonMI,
            QString("python sys.path.insert(0, \"%0\")").arg(quotedPrintersPath)));
        queueCmd(new MICommand(MI::NonMI, "source " + fileName));
    }
    */

    // set a larger term width.
    // TODO: set term-width to exact max column count in console view
    addCommand(MI::NonMI, "settings set term-width 1024");

    qCDebug(DEBUGGERLLDB) << "Initialized LLDB";
}

void DebugSession::configure(ILaunchConfiguration *cfg, IExecutePlugin *)
{
    // Read Configuration values
    KConfigGroup grp = cfg->config();
    QUrl configLldbScript = grp.readEntry(Config::LldbConfigScriptEntry, QUrl());

    // break on start: can't use "-exec-run --start" because in lldb-mi
    // the inferior stops without any notification
    bool breakOnStart = grp.readEntry(KDevMI::Config::BreakOnStartEntry, false);
    if (breakOnStart) {
        BreakpointModel* m = ICore::self()->debugController()->breakpointModel();
        bool found = false;
        foreach (Breakpoint *b, m->breakpoints()) {
            if (b->location() == "main") {
                found = true;
                break;
            }
        }
        if (!found) {
            m->addCodeBreakpoint("main");
        }
    }

    // Needed so that breakpoint widget has a chance to insert breakpoints.
    // FIXME: a bit hacky, as we're really not ready for new commands.
    setDebuggerStateOn(s_dbgBusy);
    raiseEvent(debugger_ready);

    // custom config script
    if (configLldbScript.isValid()) {
        addCommand(MI::NonMI, "command source -s TRUE " + KShell::quoteArg(configLldbScript.toLocalFile()));
    }

    qCDebug(DEBUGGERGDB) << "Per inferior configuration done";
}

bool DebugSession::execInferior(ILaunchConfiguration *cfg, IExecutePlugin *iexec,
                                const QString &executable)
{
    qCDebug(DEBUGGERGDB) << "Executing inferior";

    // debugger specific config
    configure(cfg, iexec);

    // config that can't be placed in configure()
    KConfigGroup grp = cfg->config();

    QString filesymbols = doubleQuoteArg(executable);
    bool remoteDebugging = grp.readEntry(Config::LldbRemoteDebuggingEntry, false);
    if (remoteDebugging) {
        auto connStr = grp.readEntry(Config::LldbRemoteServerEntry, QString());
        auto remoteDir = grp.readEntry(Config::LldbRemotePathEntry, QString());
        auto remoteExe = QDir(remoteDir).filePath(QFileInfo(executable).fileName());

        filesymbols += " -r " + doubleQuoteArg(remoteExe);

        addCommand(MI::FileExecAndSymbols, filesymbols,
                   this, &DebugSession::handleFileExecAndSymbols,
                   CmdHandlesError);

        addCommand(MI::TargetSelect, "remote " + connStr,
                   this, &DebugSession::handleTargetSelect, CmdHandlesError);

        // ensure executable is on remote end
        addCommand(MI::NonMI, QStringLiteral("platform mkdir -v 755 %0").arg(doubleQuoteArg(remoteDir)));
        addCommand(MI::NonMI, QStringLiteral("platform put-file %0 %1")
                              .arg(doubleQuoteArg(executable), doubleQuoteArg(remoteExe)));
    } else {
        addCommand(MI::FileExecAndSymbols, filesymbols,
                   this, &DebugSession::handleFileExecAndSymbols,
                   CmdHandlesError);
    }

    raiseEvent(connected_to_program);

    // Set the environment variables has effect only after target created
    const EnvironmentGroupList l(KSharedConfig::openConfig());
    QString envgrp = iexec->environmentGroup(cfg);
    if (envgrp.isEmpty()) {
        qCWarning(DEBUGGERCOMMON) << i18n("No environment group specified, looks like a broken "
                                          "configuration, please check run configuration '%1'. "
                                          "Using default environment group.", cfg->name());
        envgrp = l.defaultGroup();
    }
    QStringList vars;
    for (auto it = l.variables(envgrp).constBegin(),
              ite = l.variables(envgrp).constEnd();
         it != ite; ++it) {
        vars.append(QStringLiteral("%0=%1").arg(it.key(), doubleQuoteArg(it.value())));
    }
    // actually using lldb command 'settings set target.env-vars' which accepts multiple values
    addCommand(GdbSet, "environment " + vars.join(" "));

    addCommand(new SentinelCommand([this, remoteDebugging]() {
        breakpointController()->initSendBreakpoints();

        if (!remoteDebugging) {
            // FIXME: a hacky way to emulate tty setting on linux. Not sure if this provides all needed
            // functionalities of a pty. Should make this conditional on other platforms.
            // FIXME: 'process launch' doesn't provide thread-group-started notification which MIDebugSession
            // relies on to know the inferior has been started
            QPointer<DebugSession> guarded_this(this);
            addCommand(MI::NonMI,
                       QStringLiteral("process launch --stdin %0 --stdout %0 --stderr %0").arg(m_tty->getSlave()),
                       [guarded_this](const MI::ResultRecord &r) {
                           qCDebug(DEBUGGERLLDB) << "process launched:" << r.reason;
                           if (guarded_this)
                               guarded_this->setDebuggerStateOff(s_appNotStarted | s_programExited);
                       },
                       CmdMaybeStartsRunning);
        } else {
            // what is the expected behavior for using external terminal when doing remote debugging?
            addCommand(MI::ExecRun, QString(), CmdMaybeStartsRunning);
        }
    }, CmdMaybeStartsRunning));
    return true;
}

void DebugSession::interruptDebugger()
{
    // lldb always uses async mode and prompt is always available.
    // no need to interrupt
    return;
}

void DebugSession::ensureDebuggerListening()
{
    // lldb always uses async mode and prompt is always available.
    // no need to interrupt
    // NOTE: there is actually a bug in lldb-mi that, when receiving SIGINT,
    // it would print '^done', which doesn't coresponds to any previous command.
    // This confuses our command queue.
    return;
}

void DebugSession::handleFileExecAndSymbols(const MI::ResultRecord& r)
{
    if (r.reason == "error") {
        KMessageBox::error(
            qApp->activeWindow(),
            i18n("<b>Could not start debugger:</b><br />")+
            r["msg"].literal(),
            i18n("Startup error"));
        stopDebugger();
    }
}

void DebugSession::handleTargetSelect(const MI::ResultRecord& r)
{
    if (r.reason == "error") {
        KMessageBox::error(qApp->activeWindow(),
            i18n("<b>Error connecting to remote target:</b><br />")+
            r["msg"].literal(),
            i18n("Startup error"));
        stopDebugger();
    }
}

void DebugSession::updateAllVariables()
{
    for (auto it = m_allVariables.begin(), ite = m_allVariables.end(); it != ite; ++it) {
        LldbVariable *var = qobject_cast<LldbVariable*>(it.value());
        addCommand(VarUpdate, "--all-values " + it.key(),
                   var, &LldbVariable::handleRawUpdate);
    }
}
