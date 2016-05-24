/*
 * Common code for debugger support
 *
 * Copyright 1999-2001 John Birch <jbb@kdevelop.org>
 * Copyright 2001 by Bernd Gehrmann <bernd@kdevelop.org>
 * Copyright 2006 Vladimir Prus <ghost@cs.msu.su>
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2009 Niko Sams <niko.sams@gmail.com>
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

#include "debugsessionbase.h"

#include "debuggerbase.h"
#include "debuglog.h"
#include "mi/mi.h"
#include "mi/micommand.h"
#include "mi/micommandqueue.h"
#include "stty.h"

#include <debugger/breakpoint/breakpointmodel.h>
#include <debugger/interfaces/ivariablecontroller.h>
#include <debugger/interfaces/iframestackmodel.h>
#include <execute/iexecuteplugin.h>
#include <interfaces/icore.h>
#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/ilaunchconfiguration.h>
#include <util/environmentgrouplist.h>
#include <util/processlinemaker.h>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>
#include <KShell>

#include <QApplication>
#include <QFileInfo>
#include <QMetaEnum>
#include <QPointer>
#include <QRegularExpression>
#include <QUrl>

using namespace KDevelop;
using namespace KDevDebugger;
using namespace KDevDebugger::MI;

DebugSessionBase::DebugSessionBase()
    : m_procLineMaker(new ProcessLineMaker(this))
    , m_commandQueue(new CommandQueue)
    , m_sessionState(NotStartedState)
    , m_debugger(nullptr)
    , m_debuggerState(s_dbgNotStarted | s_appNotStarted)
    , m_stateReloadInProgress(false)
    , m_stateReloadNeeded(false)
    , m_tty(nullptr)
    , m_hasCrashed(false)
    , m_sourceInitFile(true)
{
    // setup signals
    connect(m_procLineMaker, &ProcessLineMaker::receivedStdoutLines,
            this, &DebugSessionBase::inferiorStdoutLines);
    connect(m_procLineMaker, &ProcessLineMaker::receivedStderrLines,
            this, &DebugSessionBase::inferiorStderrLines);

    // forward tty output to process line maker
    connect(this, &DebugSessionBase::inferiorTtyStdout,
            m_procLineMaker, &ProcessLineMaker::slotReceivedStdout);
    connect(this, &DebugSessionBase::inferiorTtyStderr,
            m_procLineMaker, &ProcessLineMaker::slotReceivedStderr);

    // FIXME: see if this still works
    //connect(statusBarIndicator, SIGNAL(doubleClicked()),
    //        controller, SLOT(explainDebuggerStatus()));

    // FIXME: reimplement / re-enable
    //connect(this, SIGNAL(addWatchVariable(QString)), controller->variables(), SLOT(slotAddWatchVariable(QString)));
    //connect(this, SIGNAL(evaluateExpression(QString)), controller->variables(), SLOT(slotEvaluateExpression(QString)));
}

DebugSessionBase::~DebugSessionBase()
{
    qCDebug(DEBUGGERCOMMON) << "Destroying DebugSessionBase";
    // Deleting the session involves shutting down gdb nicely.
    // When were attached to a process, we must first detach so that the process
    // can continue running as it was before being attached. gdb is quite slow to
    // detach from a process, so we must process events within here to get a "clean"
    // shutdown.
    if (!debuggerStateIsOn(s_dbgNotStarted)) {
        stopDebugger();
    }
}

IDebugSession::DebuggerState DebugSessionBase::state() const
{
    return m_sessionState;
}

bool DebugSessionBase::restartAvaliable() const
{
    if (debuggerStateIsOn(s_attached) || debuggerStateIsOn(s_core)) {
        return false;
    } else {
        return true;
    }
}

bool DebugSessionBase::startDebugger(ILaunchConfiguration *cfg)
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
    connect(m_debugger, &DebuggerBase::applicationOutput,
            this, [this](const QString &output) {
                emit inferiorStdoutLines(output.split(QRegularExpression("[\r\n]"), QString::SkipEmptyParts));
            });
    connect(m_debugger, &DebuggerBase::userCommandOutput, this, &DebugSessionBase::debuggerUserCommandOutput);
    connect(m_debugger, &DebuggerBase::internalCommandOutput, this, &DebugSessionBase::debuggerInternalCommandOutput);

    // state signals
    connect(m_debugger, &DebuggerBase::programStopped, this, &DebugSessionBase::inferiorStopped);
    connect(m_debugger, &DebuggerBase::programRunning, this, &DebugSessionBase::inferiorRunning);

    // internal handlers
    connect(m_debugger, &DebuggerBase::ready, this, &DebugSessionBase::slotDebuggerReady);
    connect(m_debugger, &DebuggerBase::exited, this, &DebugSessionBase::slotDebuggerExited);
    connect(m_debugger, &DebuggerBase::programStopped, this, &DebugSessionBase::slotInferiorStopped);
    connect(m_debugger, &DebuggerBase::programRunning, this, &DebugSessionBase::slotInferiorRunning);
    connect(m_debugger, &DebuggerBase::notification, this, &DebugSessionBase::processNotification);


    // start the debugger. Do this after connecting all signals so that initial
    // debugger output, and important events like the debugger died are reported.
    QStringList extraArguments;
    if (!m_sourceInitFile)
        extraArguments << "--nx";

    auto config = cfg ? cfg->config()
                // FIXME: this is only used when attachToProcess or examineCoreFile.
                // Change to use a global launch configuration when calling
                : KConfigGroup(KSharedConfig::openConfig(), "GDB Config");

    m_debugger->start(config, extraArguments);

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

bool DebugSessionBase::startDebugging(ILaunchConfiguration* cfg, IExecutePlugin* iexec)
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

    // Set up the tty for the inferior
    bool config_useExternalTerminal = iexec->useTerminal(cfg);
    QString config_ternimalName = iexec->terminal(cfg);
    if (!config_ternimalName.isEmpty()) {
        // the external terminal cmd contains additional arguments, just get the terminal name
        config_ternimalName = KShell::splitArgs(config_ternimalName).first();
    }

    m_tty.reset(new STTY(config_useExternalTerminal, config_ternimalName));
    if (!config_useExternalTerminal) {
        connect(m_tty.get(), &STTY::OutOutput, this, &DebugSessionBase::inferiorTtyStdout);
        connect(m_tty.get(), &STTY::ErrOutput, this, &DebugSessionBase::inferiorTtyStderr);
    }
    QString tty(m_tty->getSlave());
    if (tty.isEmpty()) {
        KMessageBox::information(qApp->activeWindow(), m_tty->lastError(), i18n("warning"));

        m_tty.reset(nullptr);
        return false;
    }
    queueCmd(new MICommand(InferiorTtySet, tty));

    // Only dummy err here, actual erros have been checked already in the job and we don't get here if there were any
    QString err;
    QString executable = iexec->executable(cfg, err).toLocalFile();
    QStringList arguments = iexec->arguments(cfg, err);
    // Change the working directory to the correct one
    QString dir = iexec->workingDirectory(cfg).toLocalFile();
    if (dir.isEmpty()) {
        dir = QFileInfo(executable).absolutePath();
    }
    queueCmd(new MICommand(MI::EnvironmentCd, '"' + dir + '"'));

    // Set the environment variables
    EnvironmentGroupList l(KSharedConfig::openConfig());
    QString envgrp = iexec->environmentGroup(cfg);
    if (envgrp.isEmpty()) {
        qCWarning(DEBUGGERCOMMON) << i18n("No environment group specified, looks like a broken "
                                          "configuration, please check run configuration '%1'. "
                                          "Using default environment group.", cfg->name());
        envgrp = l.defaultGroup();
    }
    for (const auto &envvar : l.createEnvironment(envgrp, {})) {
        queueCmd(new MICommand(MI::GdbSet, "environment " + envvar));
    }

    // Set the run arguments
    if (!arguments.isEmpty())
        queueCmd(new MICommand(MI::ExecArguments, KShell::joinArgs(arguments)));

    // Do other debugger specific config options and actually start the inferior program
    if (!execInferior(cfg, executable)) {
        return false;
    }

    QString config_startWith = cfg->config().readEntry(startWithEntry, QStringLiteral("ApplicationOutput"));
    if (config_startWith == "GdbConsole") {
        emit raiseDebuggerConsoleViews();
    } else if (config_startWith == "FrameStack") {
        emit raiseFramestackViews();
    } else {
        // ApplicationOutput is raised in DebugJob (by setting job to Verbose/Silent)
    }

    return true;
}

// FIXME: use same configuration process as startDebugging
bool DebugSessionBase::attachToProcess(int pid)
{
    qCDebug(DEBUGGERCOMMON) << "Attach to process" << pid;

    if (debuggerStateIsOn(s_dbgNotStarted)) {
        // FIXME: use global launch configuration rather than nullptr
        if (!startDebugger(nullptr)) {
            return false;
        }
    }

    setDebuggerStateOn(s_attached);

    //set current state to running, after attaching we will get *stopped response
    setDebuggerStateOn(s_appRunning);

    // Currently, we always start debugger with a name of binary,
    // we might be connecting to a different binary completely,
    // so cancel all symbol tables gdb has.
    // We can't omit application name from gdb invocation
    // because for libtool binaries, we have no way to guess
    // real binary name.
    queueCmd(new MICommand(MI::FileExecAndSymbols));

    queueCmd(new MICommand(MI::TargetAttach, QString::number(pid),
                           this, &DebugSessionBase::handleTargetAttach,
                           CmdHandlesError));

    queueCmd(new SentinelCommand(breakpointController(),
                                 &BreakpointControllerBase::initSendBreakpoints));

    raiseEvent(connected_to_program);

    emit raiseFramestackViews();

    return true;
}

void DebugSessionBase::handleTargetAttach(const MI::ResultRecord& r)
{
    if (r.reason == "error") {
        KMessageBox::error(
            qApp->activeWindow(),
            i18n("<b>Could not attach debugger:</b><br />")+
            r["msg"].literal(),
            i18n("Startup error"));
        stopDebugger();
    }
}

bool DebugSessionBase::examineCoreFile(const QUrl &debugee, const QUrl &coreFile)
{
    if (debuggerStateIsOn(s_dbgNotStarted)) {
        // FIXME: use global launch configuration rather than nullptr
        if (!startDebugger(nullptr)) {
            return false;
        }
    }

    // FIXME: support non-local URLs
    queueCmd(new MICommand(MI::FileExecAndSymbols, debugee.toLocalFile()));
    queueCmd(new MICommand(MI::NonMI, "core " + coreFile.toLocalFile(),
                           this, &DebugSessionBase::handleCoreFile, CmdHandlesError));

    raiseEvent(connected_to_program);
    raiseEvent(program_state_changed);

    return true;
}

void DebugSessionBase::handleCoreFile(const MI::ResultRecord& r)
{
    if (r.reason != "error") {
        setDebuggerStateOn(s_programExited|s_core);
    } else {
        KMessageBox::information(
            qApp->activeWindow(),
            i18n("<b>Failed to load core file</b>"
                "<p>Debugger reported the following error:"
                "<p><tt>%1", r["msg"].literal()),
            i18n("Debugger error"));

        // FIXME: How should we proceed at this point? Stop the debugger?
    }
}

#define ENUM_NAME(o,e,v) (o::staticMetaObject.enumerator(o::staticMetaObject.indexOfEnumerator(#e)).valueToKey((v)))
void DebugSessionBase::setSessionState(DebuggerState state)
{
    qCDebug(DEBUGGERCOMMON) << "Session state changed to"
                            << ENUM_NAME(IDebugSession, DebuggerState, state)
                            << "(" << state << ")";
    if (state != m_sessionState) {
        m_sessionState = state;
        emit stateChanged(state);
    }
}

bool DebugSessionBase::debuggerStateIsOn(DBGStateFlags state) const
{
    return m_debuggerState & state;
}

DBGStateFlags DebugSessionBase::debuggerState() const
{
    return m_debuggerState;
}

void DebugSessionBase::setDebuggerStateOn(DBGStateFlags stateOn)
{
    DBGStateFlags oldState = m_debuggerState;

    debuggerStateChange(m_debuggerState, m_debuggerState | stateOn);
    m_debuggerState |= stateOn;

    handleDebuggerStateChange(oldState, m_debuggerState);
}

void DebugSessionBase::setDebuggerStateOff(DBGStateFlags stateOff)
{
    DBGStateFlags oldState = m_debuggerState;

    debuggerStateChange(m_debuggerState, m_debuggerState & ~stateOff);
    m_debuggerState &= ~stateOff;

    handleDebuggerStateChange(oldState, m_debuggerState);
}

void DebugSessionBase::setDebuggerState(DBGStateFlags newState)
{
    DBGStateFlags oldState = m_debuggerState;

    debuggerStateChange(m_debuggerState, newState);
    m_debuggerState = newState;

    handleDebuggerStateChange(oldState, m_debuggerState);
}

void DebugSessionBase::debuggerStateChange(DBGStateFlags oldState, DBGStateFlags newState)
{
    int delta = oldState ^ newState;
    if (delta)
    {
        QString out;
#define STATE_CHECK(name) \
    do { \
        if (delta & name) { \
            out += ((newState & name) ? " +" : " -"); \
            out += #name; \
            delta &= ~name; \
        } \
    } while (0)
        STATE_CHECK(s_dbgNotStarted);
        STATE_CHECK(s_appNotStarted);
        STATE_CHECK(s_programExited);
        STATE_CHECK(s_attached);
        STATE_CHECK(s_core);
        STATE_CHECK(s_shuttingDown);
        STATE_CHECK(s_dbgBusy);
        STATE_CHECK(s_appRunning);
        STATE_CHECK(s_dbgNotListening);
        STATE_CHECK(s_automaticContinue);
#undef STATE_CHECK

        for (unsigned int i = 0; delta != 0 && i < 32; ++i) {
            if (delta & (1 << i))  {
                delta &= ~(1 << i);
                out += ((1 << i) & newState) ? " +" : " -";
                out += QString::number(i);
            }
        }
        qCDebug(DEBUGGERCOMMON) << "Debugger state change:" << out;
    }
}

void DebugSessionBase::handleDebuggerStateChange(DBGStateFlags oldState, DBGStateFlags newState)
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
        if (oldSessionState != NotStartedState) {
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
    qCDebug(DEBUGGERCOMMON) << "Debugger state changed to: " << newState << message;

    if (!message.isEmpty())
        emit showMessage(message, 3000);

    emit debuggerStateChanged(oldState, newState);

    // must be last, since it can lead to deletion of the DebugSession
    if (newSessionState != oldSessionState) {
        setSessionState(newSessionState);
    }
}

void DebugSessionBase::restartDebugger()
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
        // queueCmd(new MICommand(MI::ExecAbort));
        queueCmd(new MICommand(MI::NonMI, "kill"));
    }
    run();
}

void DebugSessionBase::stopDebugger()
{
    m_commandQueue->clear();

    qCDebug(DEBUGGERCOMMON) << "try stopping debugger";
    if (debuggerStateIsOn(s_shuttingDown) || !m_debugger)
        return;

    setDebuggerStateOn(s_shuttingDown);
    qCDebug(DEBUGGERCOMMON) << "stopping debugger";

    // Get debugger's attention if it's busy. We need debugger to be at the
    // command line so we can stop it.
    if (!m_debugger->isReady()) {
        qCDebug(DEBUGGERCOMMON) << "debugger busy on shutdown - interruping";
        interruptDebugger();
    }

    // If the app is attached then we release it here. This doesn't stop
    // the app running.
    if (debuggerStateIsOn(s_attached)) {
        queueCmd(new MICommand(MI::TargetDetach));
        emit debuggerUserCommandOutput("(gdb) detach\n");
    }

    // Now try to stop debugger running.
    queueCmd(new MICommand(MI::GdbExit));
    emit debuggerUserCommandOutput("(gdb) quit");

    // We cannot wait forever, kill gdb after 5 seconds if it's not yet quit
    QPointer<DebugSessionBase> guarded_this(this);
    QTimer::singleShot(5000, [guarded_this](){
        if (guarded_this) {
            if (!guarded_this->debuggerStateIsOn(s_programExited)
                && guarded_this->debuggerStateIsOn(s_shuttingDown)) {
                qCDebug(DEBUGGERCOMMON) << "debugger not shutdown - killing";
                guarded_this->m_debugger->kill();
                guarded_this->setDebuggerState(s_dbgNotStarted | s_appNotStarted);
                guarded_this->raiseEvent(debugger_exited);
            }
        }
    });

    emit reset();
}

void DebugSessionBase::interruptDebugger()
{
    Q_ASSERT(m_debugger);

    // Explicitly send the interrupt in case something went wrong with the usual
    // ensureGdbListening logic.
    m_debugger->interrupt();
    queueCmd(new MICommand(MI::ExecInterrupt, QString(), CmdInterrupt));
}

void DebugSessionBase::run()
{
    if (debuggerStateIsOn(s_appNotStarted|s_dbgNotStarted|s_shuttingDown))
        return;

    queueCmd(new MICommand(MI::ExecContinue, QString(), CmdMaybeStartsRunning));
}

void DebugSessionBase::runToCursor()
{
    if (IDocument* doc = ICore::self()->documentController()->activeDocument()) {
        KTextEditor::Cursor cursor = doc->cursorPosition();
        if (cursor.isValid())
            runUntil(doc->url(), cursor.line() + 1);
    }
}

void DebugSessionBase::jumpToCursor()
{
    if (IDocument* doc = ICore::self()->documentController()->activeDocument()) {
        KTextEditor::Cursor cursor = doc->cursorPosition();
        if (cursor.isValid())
            jumpTo(doc->url(), cursor.line() + 1);
    }
}

void DebugSessionBase::stepOver()
{
    if (debuggerStateIsOn(s_appNotStarted|s_shuttingDown))
        return;

    queueCmd(new MICommand(MI::ExecNext, QString(), CmdMaybeStartsRunning | CmdTemporaryRun));
}

void DebugSessionBase::stepIntoInstruction()
{
    if (debuggerStateIsOn(s_appNotStarted|s_shuttingDown))
        return;

    queueCmd(new MICommand(MI::ExecStepInstruction, QString(),
                           CmdMaybeStartsRunning | CmdTemporaryRun));
}

void DebugSessionBase::stepInto()
{
    if (debuggerStateIsOn(s_appNotStarted|s_shuttingDown))
        return;

    queueCmd(new MICommand(MI::ExecStep, QString(), CmdMaybeStartsRunning | CmdTemporaryRun));
}

void DebugSessionBase::stepOverInstruction()
{
    if (debuggerStateIsOn(s_appNotStarted|s_shuttingDown))
        return;

    queueCmd(new MICommand(MI::ExecNextInstruction, QString(),
                           CmdMaybeStartsRunning | CmdTemporaryRun));
}

void DebugSessionBase::stepOut()
{
    if (debuggerStateIsOn(s_appNotStarted|s_shuttingDown))
        return;

    queueCmd(new MICommand(MI::ExecFinish, QString(), CmdMaybeStartsRunning | CmdTemporaryRun));
}

void DebugSessionBase::runUntil(const QUrl& url, int line)
{
    if (debuggerStateIsOn(s_dbgNotStarted|s_shuttingDown))
        return;

    if (!url.isValid()) {
        queueCmd(new MICommand(MI::ExecUntil, QString::number(line),
                               CmdMaybeStartsRunning | CmdTemporaryRun));
    } else {
        queueCmd(new MICommand(MI::ExecUntil,
                 QString("%1:%2").arg(url.toLocalFile()).arg(line),
                 CmdMaybeStartsRunning | CmdTemporaryRun));
    }
}

void DebugSessionBase::runUntil(const QString& address)
{
    if (debuggerStateIsOn(s_dbgNotStarted|s_shuttingDown))
        return;

    if (!address.isEmpty()) {
        queueCmd(new MICommand(MI::ExecUntil, QString("*%1").arg(address),
                               CmdMaybeStartsRunning | CmdTemporaryRun));
    }
}

void DebugSessionBase::jumpTo(const QUrl& url, int line)
{
    if (debuggerStateIsOn(s_dbgNotStarted|s_shuttingDown))
        return;

    if (url.isValid()) {
        queueCmd(new MICommand(MI::NonMI,
                               QString("tbreak %1:%2").arg(url.toLocalFile()).arg(line)));
        queueCmd(new MICommand(MI::NonMI,
                               QString("jump %1:%2").arg(url.toLocalFile()).arg(line)));
    }
}

void DebugSessionBase::jumpToMemoryAddress(const QString& address)
{
    if (debuggerStateIsOn(s_dbgNotStarted|s_shuttingDown))
        return;

    if (!address.isEmpty()) {
        queueCmd(new MICommand(MI::NonMI, QString("tbreak *%1").arg(address)));
        queueCmd(new MICommand(MI::NonMI, QString("jump *%1").arg(address)));
    }
}

void DebugSessionBase::addUserCommand(const QString& cmd)
{
    queueCmd(new UserCommand(MI::NonMI, cmd));

    // User command can theoreticall modify absolutely everything,
    // so need to force a reload.

    // We can do it right now, and don't wait for user command to finish
    // since commands used to reload all view will be executed after
    // user command anyway.
    if (!debuggerStateIsOn(s_appNotStarted) && !debuggerStateIsOn(s_programExited))
        raiseEvent(program_state_changed);
}

void DebugSessionBase::addCommand(MICommand* cmd)
{
    queueCmd(cmd);
}

void DebugSessionBase::addCommand(MI::CommandType type, const QString& str)
{
    queueCmd(new MICommand(type, str));
}

// Fairly obvious that we'll add whatever command you give me to a queue
// Not quite so obvious though is that if we are going to run again. then any
// information requests become redundent and must be removed.
// We also try and run whatever command happens to be at the head of
// the queue.
void DebugSessionBase::queueCmd(MICommand *cmd)
{
    if (debuggerStateIsOn(s_dbgNotStarted)) {
        KMessageBox::information(
            qApp->activeWindow(),
            i18n("<b>Gdb command sent when debugger is not running</b><br>"
                 "The command was:<br> %1", cmd->initialString()),
            i18n("Internal error"));
        return;
    }

    if (m_stateReloadInProgress)
        cmd->setStateReloading(true);

    m_commandQueue->enqueue(cmd);

    qCDebug(DEBUGGERCOMMON) << "QUEUE: " << cmd->initialString()
                            << (m_stateReloadInProgress ? "(state reloading)" : "");

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

    setDebuggerStateOn(s_dbgBusy);
    raiseEvent(debugger_busy);

    executeCmd();
}

void DebugSessionBase::executeCmd()
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

    MICommand* currentCmd = m_commandQueue->nextCommand();
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
        if (SentinelCommand* sc = dynamic_cast<SentinelCommand*>(currentCmd))
        {
            qCDebug(DEBUGGERCOMMON) << "SEND: sentinel command, not sending";
            sc->invokeHandler();
        }
        else
        {
            qCDebug(DEBUGGERCOMMON) << "SEND: command " << currentCmd->initialString()
                          << "changed its mind, not sending";
        }

        delete currentCmd;
        executeCmd();
        return;
    } else {
        if (commandText[length-1] != '\n') {
            bad_command = true;
            message = "Debugger command does not end with newline";
        }
    }

    if (bad_command) {
        KMessageBox::information(qApp->activeWindow(),
                                 i18n("<b>Invalid debugger command</b><br>%1", message),
                                 i18n("Invalid debugger command"));
        executeCmd();
        return;
    }

    m_debugger->execute(currentCmd);
}

void DebugSessionBase::ensureDebuggerListening()
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

void DebugSessionBase::destroyCmds()
{
    m_commandQueue->clear();
}

// FIXME: I don't fully remember what is the business with
// m_stateReloadInProgress and whether we can lift it to the
// generic level.
void DebugSessionBase::raiseEvent(event_t e)
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

bool KDevDebugger::DebugSessionBase::hasCrashed() const
{
    return m_hasCrashed;
}

void DebugSessionBase::slotDebuggerReady()
{
    Q_ASSERT(m_debugger);

    m_stateReloadInProgress = false;

    executeCmd();
    if (m_debugger->isReady()) {
        /* There is nothing in the command queue and no command is currently executing. */
        if (debuggerStateIsOn(s_automaticContinue)) {
            if (!debuggerStateIsOn(s_appRunning)) {
                qCDebug(DEBUGGERCOMMON) << "Posting automatic continue";
                queueCmd(new MICommand(MI::ExecContinue, QString(), CmdMaybeStartsRunning));
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

void DebugSessionBase::slotDebuggerExited(bool abnormal, const QString &msg)
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
        /* The error is reported to user in DebuggerBase now.
        KMessageBox::information(
            KDevelop::ICore::self()->uiController()->activeMainWindow(),
            i18n("<b>Debugger exited abnormally</b>"
                "<p>This is likely a bug in GDB. "
                "Examine the gdb output window and then stop the debugger"),
            i18n("Debugger exited abnormally"));
        */
        // FIXME: not sure if the following still applies.
        // Note: we don't stop the debugger here, becuse that will hide gdb
        // window and prevent the user from finding the exact reason of the
        // problem.
    }

    /* FIXME: raiseEvent is handled across multiple places where we explicitly
     * stop/kill the debugger, a better way is to let the debugger itself report
     * its exited event.
     */
    // raiseEvent(debugger_exited);
}

void DebugSessionBase::slotInferiorStopped(const MI::AsyncRecord& r)
{
    /* By default, reload all state on program stop.  */
    m_stateReloadNeeded = true;
    setDebuggerStateOff(s_appRunning);
    setDebuggerStateOff(s_dbgNotListening);

    QString reason;
    if (r.hasField("reason")) reason = r["reason"].literal();

    if (reason == "exited-normally" || reason == "exited") {
        if (r.hasField("exit-code")) {
            programNoApp(i18n("Exited with return code: %1", r["exit-code"].literal()));
        } else {
            programNoApp(i18n("Exited normally"));
        }
        m_stateReloadNeeded = false;
        return;
    }

    if (reason == "exited-signalled") {
        programNoApp(i18n("Exited on signal %1", r["signal-name"].literal()));
        m_stateReloadNeeded = false;
        return;
    }

    if (reason == "watchpoint-scope") {
        QString number = r["wpnum"].literal();

        // FIXME: shuld remove this watchpoint
        // But first, we should consider if removing all
        // watchpoinst on program exit is the right thing to
        // do.

        queueCmd(new MICommand(MI::ExecContinue, QString(), CmdMaybeStartsRunning));

        m_stateReloadNeeded = false;
        return;
    }

    bool wasInterrupt = false;

    if (reason == "signal-received") {
        QString name = r["signal-name"].literal();
        QString user_name = r["signal-meaning"].literal();

        // SIGINT is a "break into running program".
        // We do this when the user set/mod/clears a breakpoint but the
        // application is running.
        // And the user does this to stop the program also.
        if (name == "SIGINT" && debuggerStateIsOn(s_interruptSent)) {
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

    if (!reason.contains("exited")) {
        // FIXME: we should immediately update the current thread and
        // frame in the framestackmodel, so that any user actions
        // are in that thread. However, the way current framestack model
        // is implemented, we can't change thread id until we refresh
        // the entire list of threads -- otherwise we might set a thread
        // id that is not already in the list, and it will be upset.

        //Indicates if program state should be reloaded immediately.
        bool updateState = false;

        if (r.hasField("frame")) {
            const MI::Value& frame = r["frame"];
            QString file, line, addr;

            if (frame.hasField("fullname")) file = frame["fullname"].literal();;
            if (frame.hasField("line"))     line = frame["line"].literal();
            if (frame.hasField("addr"))     addr = frame["addr"].literal();

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

void DebugSessionBase::slotInferiorRunning()
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

void DebugSessionBase::processNotification(const MI::AsyncRecord & async)
{
    if (async.reason == "thread-group-started") {
        setDebuggerStateOff(s_appNotStarted | s_programExited);
    } else if (async.reason == "thread-group-exited") {
        setDebuggerStateOn(s_programExited);
    } else if (async.reason == "library-loaded") {
        // do nothing
    } else if (async.reason == "breakpoint-created") {
        breakpointController()->notifyBreakpointCreated(async);
    } else if (async.reason == "breakpoint-modified") {
        breakpointController()->notifyBreakpointModified(async);
    } else if (async.reason == "breakpoint-deleted") {
        breakpointController()->notifyBreakpointDeleted(async);
    } else {
        qCDebug(DEBUGGERCOMMON) << "Unhandled notification: " << async.reason;
    }
}

void DebugSessionBase::reloadProgramState()
{
    raiseEvent(program_state_changed);
    m_stateReloadNeeded = false;
}

// There is no app anymore. This can be caused by program exiting
// an invalid program specified or ...
// gdb is still running though, but only the run command (may) make sense
// all other commands are disabled.
void DebugSessionBase::programNoApp(const QString& msg)
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
        // will continiously bomd STTY with signals, so we need to either disable
        // QSocketNotifier, or delete STTY. The latter is simpler, since we can't
        // reuse it for future debug sessions anyway.
        m_tty.reset(nullptr);
    }

    stopDebugger();

    raiseEvent(program_exited);
    raiseEvent(debugger_exited);

    emit showMessage(msg, 0);

    programFinished(msg);
}

void DebugSessionBase::programFinished(const QString& msg)
{
    QString m = QString("*** %0 ***").arg(msg.trimmed());
    emit inferiorStderrLines(QStringList(m));

    /* Also show message in gdb window, so that users who
       prefer to look at gdb window know what's up.  */
    emit debuggerUserCommandOutput(m);
}

void DebugSessionBase::explainDebuggerStatus()
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
                             typeid(*currentCmd_).name(),
                             currentCmd_->cmdToSend(),
                             currentCmd_->initialString());

        information += extra;
    }

    KMessageBox::information(qApp->activeWindow(), information,
                             i18n("Debugger status"));
}

// There is no app anymore. This can be caused by program exiting
// an invalid program specified or ...
// gdb is still running though, but only the run command (may) make sense
// all other commands are disabled.
void DebugSessionBase::handleNoInferior(const QString& msg)
{
    qCDebug(DEBUGGERCOMMON) << msg;

    setDebuggerState(s_appNotStarted | s_programExited | (debuggerState() & s_shuttingDown));

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
        // will continiously bomd STTY with signals, so we need to either disable
        // QSocketNotifier, or delete STTY. The latter is simpler, since we can't
        // reuse it for future debug sessions anyway.
        m_tty.reset(0);
    }

    stopDebugger();

    raiseEvent(program_exited);
    raiseEvent(debugger_exited);

    emit showMessage(msg, 0);

    handleInferiorFinished(msg);
}

void DebugSessionBase::handleInferiorFinished(const QString& msg)
{
    QString m = QStringLiteral("*** %0 ***").arg(msg.trimmed());
    emit inferiorStderrLines(QStringList(m));

    /* Also show message in gdb window, so that users who
       prefer to look at gdb window know what's up.  */
    emit debuggerUserCommandOutput(m);
}

// FIXME: connect to debugger's slot.
void DebugSessionBase::defaultErrorHandler(const MI::ResultRecord& result)
{
    QString msg = result["msg"].literal();

    if (msg.contains("No such process"))
    {
        setDebuggerState(s_appNotStarted|s_programExited);
        raiseEvent(program_exited);
        return;
    }

    KMessageBox::information(
        qApp->activeWindow(),
        i18n("<b>Debugger error</b>"
             "<p>Debugger reported the following error:"
             "<p><tt>%1", result["msg"].literal()),
        i18n("Debugger error"));

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

void DebugSessionBase::setSourceInitFile(bool enable)
{
    m_sourceInitFile = enable;
}
