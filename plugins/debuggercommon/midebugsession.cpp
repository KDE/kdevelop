/*
    SPDX-FileCopyrightText: 1999-2001 John Birch <jbb@kdevelop.org>
    SPDX-FileCopyrightText: 2001 Bernd Gehrmann <bernd@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Vladimir Prus <ghost@cs.msu.su>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "midebugsession.h"

#include "debuglog.h"
#include "midebugger.h"
#include "midebuggerplugin.h"
#include "mivariable.h"
#include "mi/mi.h"
#include "mi/micommand.h"
#include "mi/micommandqueue.h"
#include "stty.h"

#include <debugger/interfaces/iframestackmodel.h>
#include <execute/iexecuteplugin.h>
#include <interfaces/icore.h>
#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/ilaunchconfiguration.h>
#include <interfaces/iuicontroller.h>
#include <sublime/message.h>
#include <util/processlinemaker.h>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KShell>

#include <QApplication>
#include <QFileInfo>
#include <QMetaEnum>
#include <QRegularExpression>
#include <QUrl>
#include <QTimer>

using namespace KDevelop;
using namespace KDevMI;
using namespace KDevMI::MI;

namespace {
constexpr DBGStateFlags notStartedDebuggerFlags{s_dbgNotStarted | s_appNotStarted};
}

MIDebugSession::MIDebugSession(MIDebuggerPlugin *plugin)
    : m_procLineMaker(new ProcessLineMaker(this))
    , m_commandQueue(new CommandQueue)
    , m_debuggerState{notStartedDebuggerFlags}
    , m_tty(nullptr)
    , m_plugin(plugin)
{
    qCDebug(DEBUGGERCOMMON) << "Creating" << this;

    // setup signals
    connect(m_procLineMaker, &ProcessLineMaker::receivedStdoutLines,
            this, &MIDebugSession::inferiorStdoutLines);
    connect(m_procLineMaker, &ProcessLineMaker::receivedStderrLines,
            this, &MIDebugSession::inferiorStderrLines);

    // FIXME: see if this still works
    //connect(statusBarIndicator, SIGNAL(doubleClicked()),
    //        controller, SLOT(explainDebuggerStatus()));

    // FIXME: reimplement / re-enable
    //connect(this, SIGNAL(addWatchVariable(QString)), controller->variables(), SLOT(slotAddWatchVariable(QString)));
    //connect(this, SIGNAL(evaluateExpression(QString)), controller->variables(), SLOT(slotEvaluateExpression(QString)));
}

MIDebugSession::~MIDebugSession()
{
    qCDebug(DEBUGGERCOMMON) << "Destroying" << this;
    // Deleting the session involves shutting down gdb nicely.
    // When were attached to a process, we must first detach so that the process
    // can continue running as it was before being attached. gdb is quite slow to
    // detach from a process, so we must process events within here to get a "clean"
    // shutdown.
    if (!debuggerStateIsOn(s_dbgNotStarted)) {
        stopDebugger();
    }
}

IDebugSession::DebuggerState MIDebugSession::state() const
{
    return m_sessionState;
}

QMap<QString, MIVariable*> & MIDebugSession::variableMapping()
{
    return m_allVariables;
}

MIVariable* MIDebugSession::findVariableByVarobjName(const QString &varobjName) const
{
    if (m_allVariables.count(varobjName) == 0)
        return nullptr;
    return m_allVariables.value(varobjName);
}

void MIDebugSession::markAllVariableDead()
{
    for (auto* variable : std::as_const(m_allVariables)) {
        variable->markAsDead();
    }
    m_allVariables.clear();
}

bool MIDebugSession::restartAvaliable() const
{
    if (debuggerStateIsOn(s_attached) || debuggerStateIsOn(s_core)) {
        return false;
    } else {
        return true;
    }
}

bool MIDebugSession::startDebugger(ILaunchConfiguration *cfg)
{
    qCDebug(DEBUGGERCOMMON) << "Starting new debugger instance";
    if (m_debugger) {
        qCWarning(DEBUGGERCOMMON) << "m_debugger object still exists";
        delete m_debugger;
        m_debugger = nullptr;
    }
    m_debugger = createDebugger();
    m_debugger->setParent(this);

    // output signals
    connect(m_debugger, &MIDebugger::applicationOutput, this, [this](const QString& output) {
        auto lines = output.split(QRegularExpression(QStringLiteral("[\r\n]")), Qt::SkipEmptyParts);
        for (auto& line : lines) {
            int p = line.length();
            while (p >= 1 && (line[p - 1] == QLatin1Char('\r') || line[p - 1] == QLatin1Char('\n'))) {
                p--;
            }
            if (p != line.length())
                line.truncate(p);
        }
        emit inferiorStdoutLines(lines);
    });
    connect(m_debugger, &MIDebugger::userCommandOutput, this, &MIDebugSession::debuggerUserCommandOutput);
    connect(m_debugger, &MIDebugger::internalCommandOutput, this, &MIDebugSession::debuggerInternalCommandOutput);
    connect(m_debugger, &MIDebugger::debuggerInternalOutput, this, &MIDebugSession::debuggerInternalOutput);

    // state signals
    connect(m_debugger, &MIDebugger::programStopped, this, &MIDebugSession::inferiorStopped);
    connect(m_debugger, &MIDebugger::programRunning, this, &MIDebugSession::inferiorRunning);

    // internal handlers
    connect(m_debugger, &MIDebugger::ready, this, &MIDebugSession::slotDebuggerReady);
    connect(m_debugger, &MIDebugger::exited, this, &MIDebugSession::slotDebuggerExited);
    connect(m_debugger, &MIDebugger::programStopped, this, &MIDebugSession::slotInferiorStopped);
    connect(m_debugger, &MIDebugger::programRunning, this, &MIDebugSession::slotInferiorRunning);
    connect(m_debugger, &MIDebugger::notification, this, &MIDebugSession::processNotification);


    // start the debugger. Do this after connecting all signals so that initial
    // debugger output, and important events like the debugger died are reported.
    QStringList extraArguments;
    if (!m_sourceInitFile)
        extraArguments << QStringLiteral("--nx");

    auto config = cfg ? cfg->config()
                // FIXME: this is only used when attachToProcess or examineCoreFile.
                // Change to use a global launch configuration when calling
                : KConfigGroup(KSharedConfig::openConfig(), QStringLiteral("GDB Config"));

    if (!m_debugger->start(config, extraArguments)) {
        // debugger failed to start, ensure debugger and session state are correctly updated.
        setDebuggerStateOn(s_dbgFailedStart);
        return false;
    }

    // FIXME: here, we should wait until the debugger is up and waiting for input.
    // Then, clear s_dbgNotStarted
    // It's better to do this right away so that the state bit is always correct.
    setDebuggerStateOff(s_dbgNotStarted);

    // Initialise debugger. At this stage debugger is sitting wondering what to do,
    // and to whom.
    initializeDebugger();

    qCDebug(DEBUGGERCOMMON) << "Debugger instance started";
    return true;
}

bool MIDebugSession::startDebugging(ILaunchConfiguration* cfg, IExecutePlugin* iexec)
{
    qCDebug(DEBUGGERCOMMON) << "Starting new debug session";
    Q_ASSERT(cfg);
    Q_ASSERT(iexec);

    // Ensure debugger is started first
    if (debuggerStateIsOn(s_appNotStarted)) {
        emit showMessage(i18n("Running program"), 1000);
    }

    if (debuggerStateIsOn(s_dbgNotStarted)) {
        if (!startDebugger(cfg))
            return false;
    }

    if (debuggerStateIsOn(s_shuttingDown)) {
        qCDebug(DEBUGGERCOMMON) << "Tried to run when debugger shutting down";
        return false;
    }

    // Only dummy err here, actual errors have been checked already in the job and we don't get here if there were any
    QString err;
    QString executable = iexec->executable(cfg, err).toLocalFile();
    configInferior(cfg, iexec, executable);

    // Set up the tty for the inferior
    bool config_useExternalTerminal = iexec->useTerminal(cfg);
    QString config_ternimalName = iexec->terminal(cfg);
    if (!config_ternimalName.isEmpty()) {
        // the external terminal cmd contains additional arguments, just get the terminal name
        config_ternimalName = KShell::splitArgs(config_ternimalName).first();
    }

    m_tty.reset(new STTY(config_useExternalTerminal, config_ternimalName));
    if (!config_useExternalTerminal) {
        connect(m_tty.get(), &STTY::OutOutput, m_procLineMaker, &ProcessLineMaker::slotReceivedStdout);
    }
    QString tty(m_tty->getSlave());
#ifndef Q_OS_WIN
    if (tty.isEmpty()) {
        auto* const message = new Sublime::Message(m_tty->lastError(), Sublime::Message::Error);
        ICore::self()->uiController()->postMessage(message);

        m_tty.reset(nullptr);

        qCDebug(DEBUGGERCOMMON) << "no TTY slave, stopping debugger";
        // Cannot simply call stopDebugger() here, because for some reason the interruptDebugger() call in
        // stopDebugger() makes the "Debugger Crashed" error dialog appear. KDevelop itself crashes then
        // unless the error dialog is dismissed promptly - before the GDB process is killed at the 5-second
        // timeout set up in stopDebugger(). Duplicate the relevant parts of stopDebugger() definition here.
        m_commandQueue->clear();
        setDebuggerStateOn(s_shuttingDown);
        addGdbExitCommand();
        emit reset();
        return false;
    }
#endif
    addCommand(InferiorTtySet, tty);

    // Change the working directory to the correct one
    QString dir = iexec->workingDirectory(cfg).toLocalFile();
    if (dir.isEmpty()) {
        dir = QFileInfo(executable).absolutePath();
    }
    addCommand(EnvironmentCd, QLatin1Char('"') + dir + QLatin1Char('"'));

    // Set the run arguments
    QStringList arguments = iexec->arguments(cfg, err);
    if (!arguments.isEmpty())
        addCommand(ExecArguments, KShell::joinArgs(arguments));

    // Do other debugger specific config options and actually start the inferior program
    if (!execInferior(cfg, iexec, executable)) {
        return false;
    }

    QString config_startWith = cfg->config().readEntry(Config::StartWithEntry, QStringLiteral("ApplicationOutput"));
    if (config_startWith == QLatin1String("GdbConsole")) {
        emit raiseDebuggerConsoleViews();
    } else if (config_startWith == QLatin1String("FrameStack")) {
        emit raiseFramestackViews();
    } else {
        // ApplicationOutput is raised in DebugJob (by setting job to Verbose/Silent)
    }

    return true;
}

// FIXME: use same configuration process as startDebugging
bool MIDebugSession::attachToProcess(int pid)
{
    qCDebug(DEBUGGERCOMMON) << "Attach to process" << pid;

    emit showMessage(i18n("Attaching to process %1", pid), 1000);

    if (debuggerStateIsOn(s_dbgNotStarted)) {
        // FIXME: use global launch configuration rather than nullptr
        if (!startDebugger(nullptr)) {
            return false;
        }
    }

    setDebuggerStateOn(s_attached);

    //set current state to running, after attaching we will get *stopped response
    setDebuggerStateOn(s_appRunning);

    addCommand(TargetAttach, QString::number(pid),
               this, &MIDebugSession::handleTargetAttach,
               CmdHandlesError);

    addCommand(std::make_unique<SentinelCommand>(breakpointController(), &MIBreakpointController::initSendBreakpoints));

    raiseEvent(connected_to_program);

    emit raiseFramestackViews();

    return true;
}

void MIDebugSession::handleTargetAttach(const MI::ResultRecord& r)
{
    if (r.reason == QLatin1String("error")) {
        const QString messageText =
            i18n("<b>Could not attach debugger:</b><br />")+
                 r[QStringLiteral("msg")].literal();
        auto* message = new Sublime::Message(messageText, Sublime::Message::Error);
        ICore::self()->uiController()->postMessage(message);
        stopDebugger();
    }
}

bool MIDebugSession::examineCoreFile(const QUrl &debugee, const QUrl &coreFile)
{
    emit showMessage(i18n("Examining core file %1", coreFile.toLocalFile()), 1000);

    if (debuggerStateIsOn(s_dbgNotStarted)) {
        // FIXME: use global launch configuration rather than nullptr
        if (!startDebugger(nullptr)) {
            return false;
        }
    }

    // FIXME: support non-local URLs
    if (!loadCoreFile(nullptr, debugee.toLocalFile(), coreFile.toLocalFile())) {
        return false;
    }

    raiseEvent(program_state_changed);

    return true;
}

#define ENUM_NAME(o,e,v) (o::staticMetaObject.enumerator(o::staticMetaObject.indexOfEnumerator(#e)).valueToKey((v)))
void MIDebugSession::setSessionState(DebuggerState state)
{
    qCDebug(DEBUGGERCOMMON) << "Session state changed to"
                            << ENUM_NAME(IDebugSession, DebuggerState, state)
                            << "(" << state << ")";
    if (state != m_sessionState) {
        m_sessionState = state;
        emit stateChanged(state);
    }
}

bool MIDebugSession::debuggerStateIsOn(DBGStateFlags state) const
{
    return m_debuggerState & state;
}

DBGStateFlags MIDebugSession::debuggerState() const
{
    return m_debuggerState;
}

void MIDebugSession::setDebuggerStateOn(DBGStateFlags stateOn)
{
    DBGStateFlags oldState = m_debuggerState;
    m_debuggerState |= stateOn;
    handleDebuggerStateChange(oldState, m_debuggerState);
}

void MIDebugSession::setDebuggerStateOff(DBGStateFlags stateOff)
{
    DBGStateFlags oldState = m_debuggerState;
    m_debuggerState &= ~stateOff;
    handleDebuggerStateChange(oldState, m_debuggerState);
}

void MIDebugSession::setDebuggerState(DBGStateFlags newState)
{
    DBGStateFlags oldState = m_debuggerState;
    m_debuggerState = newState;
    handleDebuggerStateChange(oldState, m_debuggerState);
}

void MIDebugSession::handleDebuggerStateChange(DBGStateFlags oldState, DBGStateFlags newState)
{
    QString message;

    DebuggerState oldSessionState = state();
    DebuggerState newSessionState = oldSessionState;
    DBGStateFlags changedState = oldState ^ newState;

    if (newState & s_dbgNotStarted) {
        if (changedState & s_dbgNotStarted) {
            message = i18n("Debugger stopped");
            emit finished();
        }
        if (oldSessionState != NotStartedState || newState & s_dbgFailedStart) {
            newSessionState = EndedState;
        }
    } else {
        if (newState & s_appNotStarted) {
            if (oldSessionState == NotStartedState || oldSessionState == StartingState) {
                newSessionState = StartingState;
            } else {
                newSessionState = StoppedState;
            }
        } else if (newState & s_programExited) {
            if (changedState & s_programExited) {
                message = i18n("Process exited");
            }
            newSessionState = StoppedState;
        } else if (newState & s_appRunning) {
            if (changedState & s_appRunning) {
                message = i18n("Application is running");
            }
            newSessionState = ActiveState;
        } else {
            if (changedState & s_appRunning) {
                message = i18n("Application is paused");
            }
            newSessionState = PausedState;
        }
    }

    // And now? :-)
    qCDebug(DEBUGGERCOMMON) << "Debugger state changed to:" << newState << message << "- changes:" << changedState;

    if (!message.isEmpty())
        emit showMessage(message, 3000);

    emit debuggerStateChanged(oldState, newState);

    // must be last, since it can lead to deletion of the DebugSession
    if (newSessionState != oldSessionState) {
        setSessionState(newSessionState);
    }
}

void MIDebugSession::restartDebugger()
{
    // We implement restart as kill + slotRun, as opposed as plain "run"
    // command because kill + slotRun allows any special logic in slotRun
    // to apply for restart.
    //
    // That includes:
    // - checking for out-of-date project
    // - special setup for remote debugging.
    //
    // Had we used plain 'run' command, restart for remote debugging simply
    // would not work.
    if (!debuggerStateIsOn(s_dbgNotStarted|s_shuttingDown)) {
        // FIXME: s_dbgBusy or m_debugger->isReady()?
        if (debuggerStateIsOn(s_dbgBusy)) {
            interruptDebugger();
        }
        // The -exec-abort is not implemented in gdb
        // addCommand(ExecAbort);
        addCommand(NonMI, QStringLiteral("kill"));
    }
    run();
}

void MIDebugSession::stopDebugger()
{
    if (debuggerStateIsOn(s_dbgNotStarted)) {
        qCDebug(DEBUGGERCOMMON) << "Stopping debugger when it's not started";
        if (debuggerState() != notStartedDebuggerFlags) {
            setDebuggerState(notStartedDebuggerFlags);
        }
        // Transition into EndedState to let DebugController destroy this session.
        if (state() != EndedState) {
            setSessionState(EndedState);
        }
        return;
    }

    m_commandQueue->clear();

    qCDebug(DEBUGGERCOMMON) << "try stopping debugger";
    if (debuggerStateIsOn(s_shuttingDown) || !m_debugger)
        return;

    setDebuggerStateOn(s_shuttingDown);
    qCDebug(DEBUGGERCOMMON) << "stopping debugger";

    // Get debugger's attention if it's busy. We need debugger to be at the
    // command line so we can stop it.
    if (!m_debugger->isReady()) {
        qCDebug(DEBUGGERCOMMON) << "debugger busy on shutdown - interrupting";
        interruptDebugger();
    }

    // If the app is attached then we release it here. This doesn't stop
    // the app running.
    if (debuggerStateIsOn(s_attached)) {
        addCommand(TargetDetach);
        emit debuggerUserCommandOutput(QStringLiteral("(gdb) detach\n"));
    }

    // Now try to stop debugger running.
    addGdbExitCommand();

    // We cannot wait forever, kill gdb after 5 seconds if it's not yet quit
    QTimer::singleShot(5000, this, [this]() {
        if (!debuggerStateIsOn(s_programExited) && debuggerStateIsOn(s_shuttingDown)) {
            qCDebug(DEBUGGERCOMMON) << "debugger not shutdown - killing";
            killDebuggerImpl();
        }
    });

    emit reset();
}

void MIDebugSession::addGdbExitCommand()
{
    addCommand(GdbExit);
    emit debuggerUserCommandOutput(QStringLiteral("(gdb) quit"));
}

void MIDebugSession::killDebuggerNow()
{
    if (!debuggerStateIsOn(s_dbgNotStarted)) {
        qCDebug(DEBUGGERCOMMON) << "killing debugger now";
        killDebuggerImpl();
    }
}

void MIDebugSession::killDebuggerImpl()
{
    Q_ASSERT(m_debugger);
    m_debugger->kill();
    setDebuggerState(notStartedDebuggerFlags);
    raiseEvent(debugger_exited);
}

void MIDebugSession::interruptDebugger()
{
    Q_ASSERT(m_debugger);

    // Explicitly send the interrupt in case something went wrong with the usual
    // ensureGdbListening logic.
    m_debugger->interrupt();
    addCommand(ExecInterrupt, QString(), CmdInterrupt);
}

void MIDebugSession::run()
{
    if (debuggerStateIsOn(s_appNotStarted|s_dbgNotStarted|s_shuttingDown))
        return;

    addCommand(MI::ExecContinue, QString(), CmdMaybeStartsRunning);
}

void MIDebugSession::runToCursor()
{
    if (IDocument* doc = ICore::self()->documentController()->activeDocument()) {
        KTextEditor::Cursor cursor = doc->cursorPosition();
        if (cursor.isValid())
            runUntil(doc->url(), cursor.line() + 1);
    }
}

void MIDebugSession::jumpToCursor()
{
    if (IDocument* doc = ICore::self()->documentController()->activeDocument()) {
        KTextEditor::Cursor cursor = doc->cursorPosition();
        if (cursor.isValid())
            jumpTo(doc->url(), cursor.line() + 1);
    }
}

void MIDebugSession::stepOver()
{
    if (debuggerStateIsOn(s_appNotStarted|s_shuttingDown))
        return;

    addCommand(ExecNext, QString(), CmdMaybeStartsRunning | CmdTemporaryRun);
}

void MIDebugSession::stepIntoInstruction()
{
    if (debuggerStateIsOn(s_appNotStarted|s_shuttingDown))
        return;

    addCommand(ExecStepInstruction, QString(),
               CmdMaybeStartsRunning | CmdTemporaryRun);
}

void MIDebugSession::stepInto()
{
    if (debuggerStateIsOn(s_appNotStarted|s_shuttingDown))
        return;

    addCommand(ExecStep, QString(), CmdMaybeStartsRunning | CmdTemporaryRun);
}

void MIDebugSession::stepOverInstruction()
{
    if (debuggerStateIsOn(s_appNotStarted|s_shuttingDown))
        return;

    addCommand(ExecNextInstruction, QString(),
               CmdMaybeStartsRunning | CmdTemporaryRun);
}

void MIDebugSession::stepOut()
{
    if (debuggerStateIsOn(s_appNotStarted|s_shuttingDown))
        return;

    addCommand(ExecFinish, QString(), CmdMaybeStartsRunning | CmdTemporaryRun);
}

void MIDebugSession::runUntil(const QUrl& url, int line)
{
    if (debuggerStateIsOn(s_dbgNotStarted|s_shuttingDown))
        return;

    if (!url.isValid()) {
        addCommand(ExecUntil, QString::number(line),
                   CmdMaybeStartsRunning | CmdTemporaryRun);
    } else {
        addCommand(ExecUntil,
                   QStringLiteral("%1:%2").arg(url.toLocalFile()).arg(line),
                   CmdMaybeStartsRunning | CmdTemporaryRun);
    }
}

void MIDebugSession::runUntil(const QString& address)
{
    if (debuggerStateIsOn(s_dbgNotStarted|s_shuttingDown))
        return;

    if (!address.isEmpty()) {
        addCommand(ExecUntil, QStringLiteral("*%1").arg(address),
                   CmdMaybeStartsRunning | CmdTemporaryRun);
    }
}

void MIDebugSession::jumpTo(const QUrl& url, int line)
{
    if (debuggerStateIsOn(s_dbgNotStarted|s_shuttingDown))
        return;

    if (url.isValid()) {
        addCommand(NonMI, QStringLiteral("tbreak %1:%2").arg(url.toLocalFile()).arg(line));
        addCommand(NonMI, QStringLiteral("jump %1:%2").arg(url.toLocalFile()).arg(line));
    }
}

void MIDebugSession::jumpToMemoryAddress(const QString& address)
{
    if (debuggerStateIsOn(s_dbgNotStarted|s_shuttingDown))
        return;

    if (!address.isEmpty()) {
        addCommand(NonMI, QStringLiteral("tbreak *%1").arg(address));
        addCommand(NonMI, QStringLiteral("jump *%1").arg(address));
    }
}

void MIDebugSession::addUserCommand(const QString& cmd)
{
    auto usercmd = createUserCommand(cmd);
    if (!usercmd)
        return;

    queueCmd(std::move(usercmd));
    // User command can theoretically modify absolutely everything,
    // so need to force a reload.

    // We can do it right now, and don't wait for user command to finish
    // since commands used to reload all view will be executed after
    // user command anyway.
    if (!debuggerStateIsOn(s_appNotStarted) && !debuggerStateIsOn(s_programExited))
        raiseEvent(program_state_changed);
}

std::unique_ptr<MICommand> MIDebugSession::createUserCommand(const QString& cmd) const
{
    if (!cmd.isEmpty() && cmd[0].isDigit()) {
        // Add a space to the beginning, so debugger won't get confused if the
        // command starts with a number (won't mix it up with command token added)
        return std::make_unique<UserCommand>(MI::NonMI, QLatin1Char(' ') + cmd);
    } else {
        return std::make_unique<UserCommand>(MI::NonMI, cmd);
    }
}

std::unique_ptr<MICommand> MIDebugSession::createCommand(CommandType type, const QString& arguments,
                                                         CommandFlags flags) const
{
    // uses protected ctor, only accessible to MIDebugSession via friendship - cannot use make_unique :(
    return std::unique_ptr<MICommand>(new MICommand(type, arguments, flags));
}

void MIDebugSession::addCommand(std::unique_ptr<MICommand> cmd)
{
    queueCmd(std::move(cmd));
}

void MIDebugSession::addCommand(MI::CommandType type, const QString& arguments, MI::CommandFlags flags)
{
    queueCmd(createCommand(type, arguments, flags));
}

void MIDebugSession::addCommand(MI::CommandType type, const QString& arguments,
                MI::MICommandHandler *handler,
                MI::CommandFlags flags)
{
    auto cmd = createCommand(type, arguments, flags);
    cmd->setHandler(handler);
    queueCmd(std::move(cmd));
}

void MIDebugSession::addCommand(MI::CommandType type, const QString& arguments,
                const MI::FunctionCommandHandler::Function& callback,
                MI::CommandFlags flags)
{
    auto cmd = createCommand(type, arguments, flags);
    cmd->setHandler(callback);
    queueCmd(std::move(cmd));
}

// Fairly obvious that we'll add whatever command you give me to a queue
// Not quite so obvious though is that if we are going to run again. then any
// information requests become redundant and must be removed.
// We also try and run whatever command happens to be at the head of
// the queue.
void MIDebugSession::queueCmd(std::unique_ptr<MICommand> cmd)
{
    if (debuggerStateIsOn(s_dbgNotStarted)) {
        const QString messageText =
            i18n("<b>Gdb command sent when debugger is not running</b><br>"
                 "The command was:<br> %1", cmd->initialString());
        auto* message = new Sublime::Message(messageText, Sublime::Message::Information);
        ICore::self()->uiController()->postMessage(message);
        return;
    }

    if (m_stateReloadInProgress)
        cmd->setStateReloading(true);

    qCDebug(DEBUGGERCOMMON) << "QUEUE: " << cmd->initialString()
                            << (m_stateReloadInProgress ? "(state reloading)" : "")
                            << m_commandQueue->count() << "pending";

    bool varCommandWithContext= (cmd->type() >= MI::VarAssign
                                 && cmd->type() <= MI::VarUpdate
                                 && cmd->type() != MI::VarDelete);

    bool stackCommandWithContext = (cmd->type() >= MI::StackInfoDepth
                                    && cmd->type() <= MI::StackListLocals);

    if (varCommandWithContext || stackCommandWithContext) {
        if (cmd->thread() == -1)
            qCDebug(DEBUGGERCOMMON) << "\t--thread will be added on execution";

        if (cmd->frame() == -1)
            qCDebug(DEBUGGERCOMMON) << "\t--frame will be added on execution";
    }

    m_commandQueue->enqueue(std::move(cmd));

    setDebuggerStateOn(s_dbgBusy);
    raiseEvent(debugger_busy);

    executeCmd();
}

void MIDebugSession::executeCmd()
{
    Q_ASSERT(m_debugger);

    if (debuggerStateIsOn(s_dbgNotListening) && m_commandQueue->haveImmediateCommand()) {
        // We may have to call this even while a command is currently executing, because
        // debugger can get into a state where a command such as ExecRun does not send a response
        // while the inferior is running.
        ensureDebuggerListening();
    }

    if (!m_debugger->isReady())
        return;

    auto currentCmd = m_commandQueue->nextCommand();
    if (!currentCmd)
        return;

    if (currentCmd->flags() & (CmdMaybeStartsRunning | CmdInterrupt)) {
        setDebuggerStateOff(s_automaticContinue);
    }

    if (currentCmd->flags() & CmdMaybeStartsRunning) {
        // GDB can be in a state where it is listening for commands while the program is running.
        // However, when we send a command such as ExecContinue in this state, GDB may return to
        // the non-listening state without acknowledging that the ExecContinue command has even
        // finished, let alone sending a new notification about the program's running state.
        // So let's be extra cautious about ensuring that we will wake GDB up again if required.
        setDebuggerStateOn(s_dbgNotListening);
    }

    bool varCommandWithContext= (currentCmd->type() >= MI::VarAssign
                                 && currentCmd->type() <= MI::VarUpdate
                                 && currentCmd->type() != MI::VarDelete);

    bool stackCommandWithContext = (currentCmd->type() >= MI::StackInfoDepth
                                    && currentCmd->type() <= MI::StackListLocals);

    if (varCommandWithContext || stackCommandWithContext) {
        // Most var commands should be executed in the context
        // of the selected thread and frame.
        if (currentCmd->thread() == -1)
            currentCmd->setThread(frameStackModel()->currentThread());

        if (currentCmd->frame() == -1)
            currentCmd->setFrame(frameStackModel()->currentFrame());
    }

    QString commandText = currentCmd->cmdToSend();
    bool bad_command = false;
    QString message;

    int length = commandText.length();
    // No i18n for message since it's mainly for debugging.
    if (length == 0) {
        // The command might decide it's no longer necessary to send
        // it.
        if (auto* sc = dynamic_cast<SentinelCommand*>(currentCmd.get())) {
            qCDebug(DEBUGGERCOMMON) << "SEND: sentinel command, not sending";
            sc->invokeHandler();
        } else {
            qCDebug(DEBUGGERCOMMON) << "SEND: command " << currentCmd->initialString()
                          << "changed its mind, not sending";
        }

        executeCmd();
        return;
    } else {
        if (commandText[length-1] != QLatin1Char('\n')) {
            bad_command = true;
            message = QStringLiteral("Debugger command does not end with newline");
        }
    }

    if (bad_command) {
        const QString messageText = i18n("<b>Invalid debugger command</b><br>%1", message);
        auto* message = new Sublime::Message(messageText, Sublime::Message::Information);
        ICore::self()->uiController()->postMessage(message);
        executeCmd();
        return;
    }

    m_debugger->execute(std::move(currentCmd));
}

void MIDebugSession::ensureDebuggerListening()
{
    Q_ASSERT(m_debugger);

    // Note: we don't use interruptDebugger() here since
    // we don't want to queue more commands before queuing a command
    m_debugger->interrupt();

    setDebuggerStateOn(s_interruptSent);
    if (debuggerStateIsOn(s_appRunning))
        setDebuggerStateOn(s_automaticContinue);
    setDebuggerStateOff(s_dbgNotListening);
}

void MIDebugSession::destroyCmds()
{
    m_commandQueue->clear();
}

// FIXME: I don't fully remember what is the business with
// m_stateReloadInProgress and whether we can lift it to the
// generic level.
void MIDebugSession::raiseEvent(event_t e)
{
    if (e == program_exited || e == debugger_exited) {
        m_stateReloadInProgress = false;
    }

    if (e == program_state_changed) {
        m_stateReloadInProgress = true;
        qCDebug(DEBUGGERCOMMON) << "State reload in progress\n";
    }

    IDebugSession::raiseEvent(e);

    if (e == program_state_changed) {
        m_stateReloadInProgress = false;
    }
}

bool KDevMI::MIDebugSession::hasCrashed() const
{
    return m_hasCrashed;
}

void MIDebugSession::slotDebuggerReady()
{
    Q_ASSERT(m_debugger);

    m_stateReloadInProgress = false;

    executeCmd();
    if (m_debugger->isReady()) {
        /* There is nothing in the command queue and no command is currently executing. */
        if (debuggerStateIsOn(s_automaticContinue)) {
            if (!debuggerStateIsOn(s_appRunning)) {
                qCDebug(DEBUGGERCOMMON) << "Posting automatic continue";
                addCommand(ExecContinue, QString(), CmdMaybeStartsRunning);
            }
            setDebuggerStateOff(s_automaticContinue);
            return;
        }

        if (m_stateReloadNeeded && !debuggerStateIsOn(s_appRunning)) {
            qCDebug(DEBUGGERCOMMON) << "Finishing program stop";
            // Set to false right now, so that if 'actOnProgramPauseMI_part2'
            // sends some commands, we won't call it again when handling replies
            // from that commands.
            m_stateReloadNeeded = false;
            reloadProgramState();
        }

        qCDebug(DEBUGGERCOMMON) << "No more commands";
        setDebuggerStateOff(s_dbgBusy);
        raiseEvent(debugger_ready);
    }
}

void MIDebugSession::slotDebuggerExited(bool abnormal, const QString &msg)
{
    /* Technically speaking, GDB is likely not to kill the application, and
       we should have some backup mechanism to make sure the application is
       killed by KDevelop.  But even if application stays around, we no longer
       can control it in any way, so mark it as exited.  */
    setDebuggerStateOn(s_appNotStarted);
    setDebuggerStateOn(s_dbgNotStarted);
    setDebuggerStateOn(s_programExited);
    setDebuggerStateOff(s_shuttingDown);

    if (!msg.isEmpty())
        emit showMessage(msg, 3000);

    if (abnormal) {
        /* The error is reported to user in MIDebugger now.
        KMessageBox::information(
            KDevelop::ICore::self()->uiController()->activeMainWindow(),
            i18n("<b>Debugger exited abnormally</b>"
                "<p>This is likely a bug in GDB. "
                "Examine the gdb output window and then stop the debugger"),
            i18n("Debugger exited abnormally"));
        */
        // FIXME: not sure if the following still applies.
        // Note: we don't stop the debugger here, because that will hide gdb
        // window and prevent the user from finding the exact reason of the
        // problem.
    }

    /* FIXME: raiseEvent is handled across multiple places where we explicitly
     * stop/kill the debugger, a better way is to let the debugger itself report
     * its exited event.
     */
    // raiseEvent(debugger_exited);
}

void MIDebugSession::slotInferiorStopped(const MI::AsyncRecord& r)
{
    /* By default, reload all state on program stop.  */
    m_stateReloadNeeded = true;
    setDebuggerStateOff(s_appRunning);
    setDebuggerStateOff(s_dbgNotListening);

    QString reason;
    if (r.hasField(QStringLiteral("reason"))) reason = r[QStringLiteral("reason")].literal();

    if (reason == QLatin1String("exited-normally") || reason == QLatin1String("exited")) {
        if (r.hasField(QStringLiteral("exit-code"))) {
            programNoApp(i18n("Exited with return code: %1", r[QStringLiteral("exit-code")].literal()));
        } else {
            programNoApp(i18n("Exited normally"));
        }
        m_stateReloadNeeded = false;
        return;
    }

    if (reason == QLatin1String("exited-signalled")) {
        programNoApp(i18n("Exited on signal %1", r[QStringLiteral("signal-name")].literal()));
        m_stateReloadNeeded = false;
        return;
    }

    if (reason == QLatin1String("watchpoint-scope")) {
        // FIXME: should remove this watchpoint
        // But first, we should consider if removing all
        // watchpoints on program exit is the right thing to
        // do.

        addCommand(ExecContinue, QString(), CmdMaybeStartsRunning);

        m_stateReloadNeeded = false;
        return;
    }

    bool wasInterrupt = false;

    if (reason == QLatin1String("signal-received")) {
        QString name = r[QStringLiteral("signal-name")].literal();
        QString user_name = r[QStringLiteral("signal-meaning")].literal();

        // SIGINT is a "break into running program".
        // We do this when the user set/mod/clears a breakpoint but the
        // application is running.
        // And the user does this to stop the program also.
        if (name == QLatin1String("SIGINT") && debuggerStateIsOn(s_interruptSent)) {
            wasInterrupt = true;
        } else {
            // Whenever we have a signal raised then tell the user, but don't
            // end the program as we want to allow the user to look at why the
            // program has a signal that's caused the prog to stop.
            // Continuing from SIG FPE/SEGV will cause a "Cannot ..." and
            // that'll end the program.
            programFinished(i18n("Program received signal %1 (%2)", name, user_name));

            m_hasCrashed = true;
        }
    }

    if (!reason.contains(QLatin1String("exited"))) {
        // FIXME: we should immediately update the current thread and
        // frame in the framestackmodel, so that any user actions
        // are in that thread. However, the way current framestack model
        // is implemented, we can't change thread id until we refresh
        // the entire list of threads -- otherwise we might set a thread
        // id that is not already in the list, and it will be upset.

        //Indicates if program state should be reloaded immediately.
        bool updateState = false;

        if (r.hasField(QStringLiteral("frame"))) {
            const MI::Value& frame = r[QStringLiteral("frame")];
            QString file, line, addr;

            if (frame.hasField(QStringLiteral("fullname"))) file = frame[QStringLiteral("fullname")].literal();
            if (frame.hasField(QStringLiteral("line")))     line = frame[QStringLiteral("line")].literal();
            if (frame.hasField(QStringLiteral("addr")))     addr = frame[QStringLiteral("addr")].literal();

            // gdb counts lines from 1 and we don't
            setCurrentPosition(QUrl::fromLocalFile(file), line.toInt() - 1, addr);

            updateState = true;
        }

        if (updateState) {
            reloadProgramState();
        }
    }

    setDebuggerStateOff(s_interruptSent);
    if (!wasInterrupt)
        setDebuggerStateOff(s_automaticContinue);
}

void MIDebugSession::slotInferiorRunning()
{
    setDebuggerStateOn(s_appRunning);
    raiseEvent(program_running);

    if (m_commandQueue->haveImmediateCommand() ||
        (m_debugger->currentCommand() && (m_debugger->currentCommand()->flags() & (CmdImmediately | CmdInterrupt)))) {
        ensureDebuggerListening();
    } else {
        setDebuggerStateOn(s_dbgNotListening);
    }
}

void MIDebugSession::processNotification(const MI::AsyncRecord & async)
{
    if (async.reason == QLatin1String("thread-group-started")) {
        setDebuggerStateOff(s_appNotStarted | s_programExited);
    } else if (async.reason == QLatin1String("thread-group-exited")) {
        setDebuggerStateOn(s_programExited);
    } else if (async.reason == QLatin1String("library-loaded")) {
        // do nothing
    } else if (async.reason == QLatin1String("breakpoint-created")) {
        breakpointController()->notifyBreakpointCreated(async);
    } else if (async.reason == QLatin1String("breakpoint-modified")) {
        breakpointController()->notifyBreakpointModified(async);
    } else if (async.reason == QLatin1String("breakpoint-deleted")) {
        breakpointController()->notifyBreakpointDeleted(async);
    } else {
        qCDebug(DEBUGGERCOMMON) << "Unhandled notification: " << async.reason;
    }
}

void MIDebugSession::reloadProgramState()
{
    raiseEvent(program_state_changed);
    m_stateReloadNeeded = false;
}

// There is no app anymore. This can be caused by program exiting
// an invalid program specified or ...
// gdb is still running though, but only the run command (may) make sense
// all other commands are disabled.
void MIDebugSession::programNoApp(const QString& msg)
{
    qCDebug(DEBUGGERCOMMON) << msg;

    setDebuggerState(s_appNotStarted | s_programExited | (m_debuggerState & s_shuttingDown));

    destroyCmds();

    // The application has existed, but it's possible that
    // some of application output is still in the pipe. We use
    // different pipes to communicate with gdb and to get application
    // output, so "exited" message from gdb might have arrived before
    // last application output. Get this last bit.

    // Note: this method can be called when we open an invalid
    // core file. In that case, tty_ won't be set.
    if (m_tty){
        m_tty->readRemaining();
        // Tty is no longer usable, delete it. Without this, QSocketNotifier
        // will continuously bomd STTY with signals, so we need to either disable
        // QSocketNotifier, or delete STTY. The latter is simpler, since we can't
        // reuse it for future debug sessions anyway.
        m_tty.reset(nullptr);
    }

    stopDebugger();

    raiseEvent(program_exited);
    raiseEvent(debugger_exited);

    emit showMessage(msg, 3000);

    programFinished(msg);
}

void MIDebugSession::programFinished(const QString& msg)
{
    QString m = QStringLiteral("*** %0 ***").arg(msg.trimmed());
    emit inferiorStderrLines(QStringList(m));

    /* Also show message in gdb window, so that users who
       prefer to look at gdb window know what's up.  */
    emit debuggerUserCommandOutput(m);
}

void MIDebugSession::explainDebuggerStatus()
{
    MICommand* currentCmd_ = m_debugger->currentCommand();
    QString information =
        i18np("1 command in queue\n", "%1 commands in queue\n", m_commandQueue->count()) +
        i18ncp("Only the 0 and 1 cases need to be translated", "1 command being processed by gdb\n", "%1 commands being processed by gdb\n", (currentCmd_ ? 1 : 0)) +
        i18n("Debugger state: %1\n", m_debuggerState);

    if (currentCmd_) {
        QString extra = i18n("Current command class: '%1'\n"
                             "Current command text: '%2'\n"
                             "Current command original text: '%3'\n",
                             QString::fromUtf8(typeid(*currentCmd_).name()),
                             currentCmd_->cmdToSend(),
                             currentCmd_->initialString());

        information += extra;
    }

    auto* message = new Sublime::Message(information, Sublime::Message::Information);
    ICore::self()->uiController()->postMessage(message);
}

// FIXME: connect to debugger's slot.
void MIDebugSession::defaultErrorHandler(const MI::ResultRecord& result)
{
    QString msg = result[QStringLiteral("msg")].literal();

    if (msg.contains(QLatin1String("No such process")))
    {
        setDebuggerState(s_appNotStarted|s_programExited);
        raiseEvent(program_exited);
        return;
    }

    const QString messageText =
        i18n("<b>Debugger error</b>"
             "<p>Debugger reported the following error:"
             "<p><tt>%1", result[QStringLiteral("msg")].literal());
    auto* message = new Sublime::Message(messageText, Sublime::Message::Error);
    ICore::self()->uiController()->postMessage(message);

    // Error most likely means that some change made in GUI
    // was not communicated to the gdb, so GUI is now not
    // in sync with gdb. Resync it.
    //
    // Another approach is to make each widget reload it content
    // on errors from commands that it sent, but that's too complex.
    // Errors are supposed to happen rarely, so full reload on error
    // is not a big deal. Well, maybe except for memory view, but
    // it's no auto-reloaded anyway.
    //
    // Also, don't reload state on errors appeared during state
    // reloading!
    if (!m_debugger->currentCommand()->stateReloading())
        raiseEvent(program_state_changed);
}

void MIDebugSession::setSourceInitFile(bool enable)
{
    m_sourceInitFile = enable;
}

#include "moc_midebugsession.cpp"
