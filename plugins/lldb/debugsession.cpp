/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "debugsession.h"

#include "controllers/variable.h"
#include "lldbcommand.h"

#include <debuglog.h>

#include "dbgglobal.h"
#include "mi/micommand.h"
#include "stringhelpers.h"
#include "stty.h"

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
using namespace KDevMI;
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
        if (r.isReasonError()) {
            if (r.hasField(QStringLiteral("msg"))
                && r.errorMessage().contains(QLatin1String("Invalid process during debug session"))) {
                // for some unknown reason, lldb-mi sometimes fails to start process
                if (m_remainRetry && m_session) {
                    qCDebug(DEBUGGERLLDB) << "Retry starting";
                    --m_remainRetry;
                    // resend the command again.
                    ++m_activeCommands;
                    m_session->addCommand(ExecRun, QString(),
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

DebugSession::DebugSession()
{
    m_breakpointController = new BreakpointController(this);
    m_variableController = new VariableController(this);
    m_frameStackModel = new LldbFrameStackModel(this);

    connect(this, &DebugSession::stateChanged, this, &DebugSession::handleSessionStateChange);
}

DebugSession::~DebugSession() = default;

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

std::unique_ptr<MICommand> DebugSession::createCommand(MI::CommandType type, const QString& arguments,
                                                       MI::CommandFlags flags) const
{
    // using protected ctor, cannot use make_unique
    return std::unique_ptr<MICommand>(new LldbCommand(type, arguments, flags));
}

std::unique_ptr<MICommand> DebugSession::createUserCommand(const QString& cmd) const
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
    addCommand(std::make_unique<CliCommand>(MI::NonMI, QStringLiteral("version"), this, &DebugSession::handleVersion));

    // load data formatter
    auto formatterPath = m_formatterPath;
    if (!QFileInfo(formatterPath).isFile()) {
        formatterPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                               QStringLiteral("kdevlldb/formatters/all.py"));
    }
    if (!formatterPath.isEmpty()) {
        addCommand(MI::NonMI, QLatin1String("command script import ") + KShell::quoteArg(formatterPath));
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

        filesymbols += QLatin1String(" -r ") + Utils::quote(remoteExe);

        addFileExecAndSymbolsCommand(filesymbols);

        addCommand(MI::TargetSelect, QLatin1String("remote ") + connStr,
                   this, &DebugSession::handleTargetSelect, CmdHandlesError);

        // ensure executable is on remote end
        addCommand(MI::NonMI, QStringLiteral("platform mkdir -v 755 %0").arg(Utils::quote(remoteDir)));
        addCommand(MI::NonMI, QStringLiteral("platform put-file %0 %1")
                              .arg(Utils::quote(executable), Utils::quote(remoteExe)));
    } else {
        addFileExecAndSymbolsCommand(filesymbols);
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
        vars.reserve(envVariables.size());
        for (auto it = envVariables.constBegin(), ite = envVariables.constEnd(); it != ite; ++it) {
            vars.append(QStringLiteral("%0=%1").arg(it.key(), Utils::quote(it.value())));
        }
        // actually using lldb command 'settings set target.env-vars' which accepts multiple values
        addCommand(GdbSet, QLatin1String("environment ") + vars.join(QLatin1Char(' ')));
    }

    // Break on start: can't use "-exec-run --start" because in lldb-mi
    // the inferior stops without any notification
    bool breakOnStart = grp.readEntry(KDevMI::Config::BreakOnStartEntry, false);
    if (breakOnStart) {
        BreakpointModel* m = ICore::self()->debugController()->breakpointModel();
        bool found = false;
        const auto breakpoints = m->breakpoints();
        for (Breakpoint* b : breakpoints) {
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
    addCommand(std::make_unique<SentinelCommand>(
        [this, remoteDebugging, configLldbScript]() {
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
                addCommand(MI::NonMI,
                           QLatin1String("command source -s 0 ") + KShell::quoteArg(configLldbScript.toLocalFile()));
            }

            addCommand(MI::ExecRun, QString(), new ExecRunHandler(this), CmdMaybeStartsRunning | CmdHandlesError);
        },
        CmdMaybeStartsRunning));
    return true;
}

void DebugSession::loadCoreFile(const QString& coreFile)
{
    addCommand(std::make_unique<CliCommand>(NonMI, QLatin1String("target create -c ") + Utils::quote(coreFile), this,
                                            &DebugSession::handleCoreFile, CmdHandlesError));
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
    // it would print '^done', which doesn't corresponds to any previous command.
    // This confuses our command queue.
}

void DebugSession::handleTargetSelect(const MI::ResultRecord& r)
{
    if (r.isReasonError()) {
        stopDebuggerOnError(i18n("<b>Error connecting to remote target:</b><br />") + r.errorMessage());
    }
}

void DebugSession::handleCoreFile(const QStringList &s)
{
    qCDebug(DEBUGGERLLDB) << s;
    for (const auto &line : s) {
        if (line.startsWith(QLatin1String("error:"))) {
            failedToLoadCoreFile(s.join(QLatin1Char{'\n'}));
            return;
        }
    }
    // There's no "thread-group-started" notification from lldb-mi, so pretend we have received one.
    // see MIDebugSession::processNotification(const MI::AsyncRecord & async)
    setDebuggerStateOff(s_appNotStarted | s_programExited);

    coreFileLoaded();
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

        auto ans = KMessageBox::warningTwoActions(
            qApp->activeWindow(),
            i18n("<b>Your lldb-mi version is unsupported, as it lacks an essential patch.</b><br/>"
                 "See https://llvm.org/bugs/show_bug.cgi?id=28026 for more information.<br/>"
                 "Debugger console will be disabled to prevent crash.<br/>"
                 "Do you want to continue?"),
            i18n("LLDB Version Unsupported"), KStandardGuiItem::cont(), KStandardGuiItem::cancel(),
            QStringLiteral("unsupported-lldb-debugger"));
        if (ans == KMessageBox::SecondaryAction) {
            programFinished(QStringLiteral("Stopped because of unsupported LLDB version"));
            stopDebugger();
        }
        return;
    }

    qCDebug(DEBUGGERLLDB) << s.first();

// minimal version is 3.8.1
#ifdef Q_OS_OSX
    QRegularExpression rx(QStringLiteral("^lldb-(\\d+).(\\d+).(\\d+)\\b"), QRegularExpression::MultilineOption);
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
            version[i] = match.capturedView(i + 1).toInt();
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

        const QString messageText = i18n("<b>You need lldb-mi from LLDB 3.8.1 or higher.</b><br />"
            "You are using: %1", s.first());
        stopDebuggerOnError(messageText);
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
    for (auto* variable : std::as_const(m_allVariables)) {
        auto *var = qobject_cast<LldbVariable*>(variable);
        if (var->topLevel()) {
            toplevels << var;
        }
    }

    for (auto var : std::as_const(toplevels)) {
        var->refetch();
    }
}

void DebugSession::handleSessionStateChange(IDebugSession::DebuggerState state)
{
    if (state == IDebugSession::PausedState) {
        // session is paused, the user can input any commands now.
        // Turn off delete duplicate breakpoints mode, as the user
        // may intentionally want to do this.
        qCDebug(DEBUGGERLLDB) << "Turn off delete duplicate mode";
        breakpointController()->setDeleteDuplicateBreakpoints(false);
    }
}

#include "moc_debugsession.cpp"
