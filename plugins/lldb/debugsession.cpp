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
#include "stringhelpers.h"

#include <debugger/breakpoint/breakpoint.h>
#include <debugger/breakpoint/breakpointmodel.h>
#include <execute/iexecuteplugin.h>
#include <interfaces/icore.h>
#include <interfaces/idebugcontroller.h>
#include <interfaces/ilaunchconfiguration.h>
#include <util/environmentprofilelist.h>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KShell>

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QGuiApplication>

using namespace KDevMI::LLDB;
using namespace KDevMI::MI;
using namespace KDevelop;

struct ExecRunHandler : public MICommandHandler
{
    explicit ExecRunHandler(DebugSession *session, int maxRetry = 5)
        : m_session(session)
        , m_remainRetry(maxRetry)
        , m_activeCommands(1)
    {
    }

    void handle(const ResultRecord& r) override
    {
        --m_activeCommands;
        if (r.reason == QLatin1String("error")) {
            if (r.hasField(QStringLiteral("msg"))
                && r[QStringLiteral("msg")].literal().contains(QLatin1String("Invalid process during debug session"))) {
                // for some unknown reason, lldb-mi sometimes fails to start process
                if (m_remainRetry && m_session) {
                    qCDebug(DEBUGGERLLDB) << "Retry starting";
                    --m_remainRetry;
                    // resend the command again.
                    ++m_activeCommands;
                    m_session->addCommand(ExecRun, QStringLiteral(""),
                                          this, // use *this as handler, so we can track error times
                                          CmdMaybeStartsRunning | CmdHandlesError);
                    return;
                }
            }
            qCDebug(DEBUGGERLLDB) << "Failed to start inferior:"
                                  << "exceeded retry times or session become invalid";
            m_session->stopDebugger();
        }
        if (m_activeCommands == 0)
            delete this;
    }

    bool handlesError() override { return true; }
    bool autoDelete() override { return false; }

    QPointer<DebugSession> m_session;
    int m_remainRetry;
    int m_activeCommands;
};

DebugSession::DebugSession(LldbDebuggerPlugin *plugin)
    : MIDebugSession(plugin)
    , m_breakpointController(nullptr)
    , m_variableController(nullptr)
    , m_frameStackModel(nullptr)
    , m_formatterPath()
    , m_hasCorrectCLIOutput(false)
{
    m_breakpointController = new BreakpointController(this);
    m_variableController = new VariableController(this);
    m_frameStackModel = new LldbFrameStackModel(this);

    if (m_plugin) m_plugin->setupToolviews();

    connect(this, &DebugSession::stateChanged, this, &DebugSession::handleSessionStateChange);
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

MICommand *DebugSession::createUserCommand(const QString& cmd) const
{
    if (m_hasCorrectCLIOutput)
        return MIDebugSession::createUserCommand(cmd);
    auto msg = i18n("Attempting to execute user command on unsupported LLDB version");
    emit debuggerInternalOutput(msg);
    qCDebug(DEBUGGERLLDB) << "Attempting user command on unsupported LLDB version";
    return nullptr;
}

void DebugSession::setFormatterPath(const QString &path)
{
    m_formatterPath = path;
}

void DebugSession::initializeDebugger()
{
    //addCommand(MI::EnableTimings, "yes");

    // Check version
    addCommand(new CliCommand(MI::NonMI, QStringLiteral("version"), this, &DebugSession::handleVersion));

    // load data formatter
    auto formatterPath = m_formatterPath;
    if (!QFileInfo(formatterPath).isFile()) {
        formatterPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                               QStringLiteral("kdevlldb/formatters/all.py"));
    }
    if (!formatterPath.isEmpty()) {
        addCommand(MI::NonMI, "command script import " + KShell::quoteArg(formatterPath));
    }


    // Treat char array as string
    addCommand(MI::GdbSet, QStringLiteral("print char-array-as-string on"));

    // set a larger term width.
    // TODO: set term-width to exact max column count in console view
    addCommand(MI::NonMI, QStringLiteral("settings set term-width 1024"));

    qCDebug(DEBUGGERLLDB) << "Initialized LLDB";
}

void DebugSession::configInferior(ILaunchConfiguration *cfg, IExecutePlugin *iexec, const QString &executable)
{
    // Read Configuration values
    KConfigGroup grp = cfg->config();

    // Create target as early as possible, so we can do target specific configuration later
    QString filesymbols = Utils::quote(executable);
    bool remoteDebugging = grp.readEntry(Config::LldbRemoteDebuggingEntry, false);
    if (remoteDebugging) {
        auto connStr = grp.readEntry(Config::LldbRemoteServerEntry, QString());
        auto remoteDir = grp.readEntry(Config::LldbRemotePathEntry, QString());
        auto remoteExe = QDir(remoteDir).filePath(QFileInfo(executable).fileName());

        filesymbols += " -r " + Utils::quote(remoteExe);

        addCommand(MI::FileExecAndSymbols, filesymbols,
                   this, &DebugSession::handleFileExecAndSymbols,
                   CmdHandlesError);

        addCommand(MI::TargetSelect, "remote " + connStr,
                   this, &DebugSession::handleTargetSelect, CmdHandlesError);

        // ensure executable is on remote end
        addCommand(MI::NonMI, QStringLiteral("platform mkdir -v 755 %0").arg(Utils::quote(remoteDir)));
        addCommand(MI::NonMI, QStringLiteral("platform put-file %0 %1")
                              .arg(Utils::quote(executable), Utils::quote(remoteExe)));
    } else {
        addCommand(MI::FileExecAndSymbols, filesymbols,
                   this, &DebugSession::handleFileExecAndSymbols,
                   CmdHandlesError);
    }

    raiseEvent(connected_to_program);

    // Set the environment variables has effect only after target created
    const EnvironmentProfileList environmentProfiles(KSharedConfig::openConfig());
    QString envProfileName = iexec->environmentProfileName(cfg);
    if (envProfileName.isEmpty()) {
        envProfileName = environmentProfiles.defaultProfileName();
    }
    const auto &envVariables = environmentProfiles.variables(envProfileName);
    if (!envVariables.isEmpty()) {
        QStringList vars;
        for (auto it = envVariables.constBegin(), ite = envVariables.constEnd(); it != ite; ++it) {
            vars.append(QStringLiteral("%0=%1").arg(it.key(), Utils::quote(it.value())));
        }
        // actually using lldb command 'settings set target.env-vars' which accepts multiple values
        addCommand(GdbSet, "environment " + vars.join(QStringLiteral(" ")));
    }

    // Break on start: can't use "-exec-run --start" because in lldb-mi
    // the inferior stops without any notification
    bool breakOnStart = grp.readEntry(KDevMI::Config::BreakOnStartEntry, false);
    if (breakOnStart) {
        BreakpointModel* m = ICore::self()->debugController()->breakpointModel();
        bool found = false;
        foreach (Breakpoint *b, m->breakpoints()) {
            if (b->location() == QLatin1String("main")) {
                found = true;
                break;
            }
        }
        if (!found) {
            m->addCodeBreakpoint(QStringLiteral("main"));
        }
    }

    // Needed so that breakpoint widget has a chance to insert breakpoints.
    // FIXME: a bit hacky, as we're really not ready for new commands.
    setDebuggerStateOn(s_dbgBusy);
    raiseEvent(debugger_ready);

    qCDebug(DEBUGGERLLDB) << "Per inferior configuration done";
}

bool DebugSession::execInferior(ILaunchConfiguration *cfg, IExecutePlugin *, const QString &)
{
    qCDebug(DEBUGGERLLDB) << "Executing inferior";

    KConfigGroup grp = cfg->config();

    // Start inferior
    bool remoteDebugging = grp.readEntry(Config::LldbRemoteDebuggingEntry, false);
    QUrl configLldbScript = grp.readEntry(Config::LldbConfigScriptEntry, QUrl());
    addCommand(new SentinelCommand([this, remoteDebugging, configLldbScript]() {
        // setup inferior I/O redirection
        if (!remoteDebugging) {
            // FIXME: a hacky way to emulate tty setting on linux. Not sure if this provides all needed
            // functionalities of a pty. Should make this conditional on other platforms.

            // no need to quote, settings set takes 'raw' input
            addCommand(MI::NonMI, QStringLiteral("settings set target.input-path %0").arg(m_tty->getSlave()));
            addCommand(MI::NonMI, QStringLiteral("settings set target.output-path %0").arg(m_tty->getSlave()));
            addCommand(MI::NonMI, QStringLiteral("settings set target.error-path %0").arg(m_tty->getSlave()));
        } else {
            // what is the expected behavior for using external terminal when doing remote debugging?
        }

        // send breakpoints already in our breakpoint model to lldb
        auto bc = breakpointController();
        bc->initSendBreakpoints();

        qCDebug(DEBUGGERLLDB) << "Turn on delete duplicate mode";
        // turn on delete duplicate breakpoints model, so that breakpoints created by user command in
        // the script and returned as a =breakpoint-created notification won't get duplicated with the
        // one already in our model.
        // we will turn this model off once we first reach a paused state, and from that time on,
        // the user can create duplicated breakpoints using normal command.
        bc->setDeleteDuplicateBreakpoints(true);
        // run custom config script right before we starting the inferior,
        // so the user has the freedom to change everything.
        if (configLldbScript.isValid()) {
            addCommand(MI::NonMI, "command source -s 0 " + KShell::quoteArg(configLldbScript.toLocalFile()));
        }

        addCommand(MI::ExecRun, QString(), new ExecRunHandler(this),
                   CmdMaybeStartsRunning | CmdHandlesError);
    }, CmdMaybeStartsRunning));
    return true;
}

bool DebugSession::loadCoreFile(ILaunchConfiguration *,
                                const QString &debugee, const QString &corefile)
{
    addCommand(MI::FileExecAndSymbols, debugee,
               this, &DebugSession::handleFileExecAndSymbols,
               CmdHandlesError);
    raiseEvent(connected_to_program);

    addCommand(new CliCommand(NonMI, "target create -c " + Utils::quote(corefile),
                              this, &DebugSession::handleCoreFile,
                              CmdHandlesError));
    return true;
}

void DebugSession::interruptDebugger()
{
    if (debuggerStateIsOn(s_dbgNotStarted|s_shuttingDown))
        return;

    addCommand(ExecInterrupt, QString(), CmdInterrupt);
    return;
}

void DebugSession::ensureDebuggerListening()
{
    // lldb always uses async mode and prompt is always available.
    // no need to interrupt
    setDebuggerStateOff(s_dbgNotListening);
    // NOTE: there is actually a bug in lldb-mi that, when receiving SIGINT,
    // it would print '^done', which doesn't coresponds to any previous command.
    // This confuses our command queue.
}

void DebugSession::handleFileExecAndSymbols(const MI::ResultRecord& r)
{
    if (r.reason == QLatin1String("error")) {
        KMessageBox::error(
            qApp->activeWindow(),
            i18n("<b>Could not start debugger:</b><br />")+
            r[QStringLiteral("msg")].literal(),
            i18n("Startup error"));
        stopDebugger();
    }
}

void DebugSession::handleTargetSelect(const MI::ResultRecord& r)
{
    if (r.reason == QLatin1String("error")) {
        KMessageBox::error(qApp->activeWindow(),
            i18n("<b>Error connecting to remote target:</b><br />")+
            r[QStringLiteral("msg")].literal(),
            i18n("Startup error"));
        stopDebugger();
    }
}

void DebugSession::handleCoreFile(const QStringList &s)
{
    qCDebug(DEBUGGERLLDB) << s;
    for (const auto &line : s) {
        if (line.startsWith(QLatin1String("error:"))) {
            KMessageBox::error(
                qApp->activeWindow(),
                i18n("<b>Failed to load core file</b>"
                "<p>Debugger reported the following error:"
                "<p><tt>%1",
                s.join('\n')),
                i18n("Startup error"));
            stopDebugger();
            return;
        }
    }
    // There's no "thread-group-started" notification from lldb-mi, so pretend we have received one.
    // see MIDebugSession::processNotification(const MI::AsyncRecord & async)
    setDebuggerStateOff(s_appNotStarted | s_programExited);

    setDebuggerStateOn(s_programExited | s_core);
}

void DebugSession::handleVersion(const QStringList& s)
{
    m_hasCorrectCLIOutput = !s.isEmpty();
    if (!m_hasCorrectCLIOutput) {
        // No output from 'version' command. It's likely that
        // the lldb used is not patched for the CLI output

        if (!qobject_cast<QGuiApplication*>(qApp))  {
            //for unittest
            qFatal("You need a graphical application.");
        }

        auto ans = KMessageBox::warningYesNo(
            qApp->activeWindow(),
            i18n("<b>Your lldb-mi version is unsupported, as it lacks an essential patch.</b><br/>"
                 "See https://llvm.org/bugs/show_bug.cgi?id=28026 for more information.<br/>"
                 "Debugger console will be disabled to prevent crash.<br/>"
                 "Do you want to continue?"),
            i18n("LLDB Version Unsupported"),
            KStandardGuiItem::yes(),
            KStandardGuiItem::no(),
            QStringLiteral("unsupported-lldb-debugger"));
        if (ans == KMessageBox::ButtonCode::No) {
            programFinished(QStringLiteral("Stopped because of unsupported LLDB version"));
            stopDebugger();
        }
        return;
    }

    qCDebug(DEBUGGERLLDB) << s.first();

// minimal version is 3.8.1
#ifdef Q_OS_OSX
    QRegularExpression rx("^lldb-(\\d+).(\\d+).(\\d+)\\b", QRegularExpression::MultilineOption);
    // lldb 3.8.1 reports version 350.99.0 on OS X
    const int min_ver[] = {350, 99, 0};
#else
    QRegularExpression rx(QStringLiteral("^lldb version (\\d+).(\\d+).(\\d+)\\b"), QRegularExpression::MultilineOption);
    const int min_ver[] = {3, 8, 1};
#endif

    auto match = rx.match(s.first());
    int version[] = {0, 0, 0};
    if (match.hasMatch()) {
        for (int i = 0; i != 3; ++i) {
            version[i] = match.captured(i+1).toInt();
        }
    }

    bool ok = true;
    for (int i = 0; i < 3; ++i) {
        if (version[i] < min_ver[i]) {
            ok = false;
            break;
        } else if (version[i] > min_ver[i]) {
            ok = true;
            break;
        }
    }

    if (!ok) {
        if (!qobject_cast<QGuiApplication*>(qApp))  {
            //for unittest
            qFatal("You need a graphical application.");
        }

        KMessageBox::error(
            qApp->activeWindow(),
            i18n("<b>You need lldb-mi from LLDB 3.8.1 or higher.</b><br />"
            "You are using: %1", s.first()),
            i18n("LLDB Error"));
        stopDebugger();
    }
}

void DebugSession::updateAllVariables()
{
    // FIXME: this is only a workaround for lldb-mi doesn't provide -var-update changelist
    // for variables that have a python synthetic provider. Remove this after this is fixed
    // in the upstream.

    // re-fetch all toplevel variables, as -var-update doesn't work with data formatter
    // we have to pick out top level variables first, as refetching will delete child
    // variables.
    QList<LldbVariable*> toplevels;
    for (auto it = m_allVariables.begin(), ite = m_allVariables.end(); it != ite; ++it) {
        LldbVariable *var = qobject_cast<LldbVariable*>(it.value());
        if (var->topLevel()) {
            toplevels << var;
        }
    }

    for (auto var : toplevels) {
        var->refetch();
    }
}

void DebugSession::handleSessionStateChange(IDebugSession::DebuggerState state)
{
    if (state == IDebugSession::PausedState) {
        // session is paused, the user can input any commands now.
        // Turn off delete duplicate breakpoints mode, as the user
        // may intentionaly want to do this.
        qCDebug(DEBUGGERLLDB) << "Turn off delete duplicate mode";
        breakpointController()->setDeleteDuplicateBreakpoints(false);
    }
}
