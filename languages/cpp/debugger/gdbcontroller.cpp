// *************************************************************************
//                          gdbcontroller.cpp  -  description
//    begin                : Sun Aug 8 1999
//    copyright            : (C) 1999 by John Birch
//    email                : jbb@kdevelop.org
//
// Copyright 2006 Vladimir Prus <ghost@cs.msu.su>
// **************************************************************************
// *                                                                        *
// *   This program is free software; you can redistribute it and/or modify *
// *   it under the terms of the GNU General Public License as published by *
// *   the Free Software Foundation; either version 2 of the License, or    *
// *   (at your option) any later version.                                  *
// *                                                                        *
// **************************************************************************

#include "gdbcontroller.h"

#include <iostream>
#include <ctype.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <assert.h>
#include <typeinfo>

#include <QDateTime>
#include <QFileInfo>
#include <QRegExp>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QEventLoop>
#include <QByteArray>
#include <QTimer>


#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kshell.h>
#include <KConfigGroup>

#include <environmentgrouplist.h>

#include "breakpoint.h"
#include "gdbcommand.h"
#include "stty.h"
#include "mi/miparser.h"
#include "gdbcommandqueue.h"
#include "variablecollection.h"
#include "stackmanager.h"
#include "breakpointcontroller.h"
#include "gdb.h"

using namespace std;
using namespace GDBMI;

namespace GDBDebugger
{

// This is here so we can check for startup /shutdown problems
int debug_controllerExists = false;

GDBController::GDBController(QObject* parent)
        : QObject(parent),
        currentFrame_(0),
        currentThread_(-1),
        commandQueue_(new CommandQueue),
        tty_(0),
        state_(s_dbgNotStarted|s_appNotStarted),
        programHasExited_(false),
        config_breakOnLoadingLibrary_(true),
        config_forceBPSet_(true),
        config_displayStaticMembers_(false),
        config_asmDemangle_(true),
        config_dbgTerminal_(false),
        config_outputRadix_(10),
        state_reload_needed(false),
        stateReloadInProgress_(false),
        gdb_(0),
        m_variableCollection(new VariableCollection(this)),
        m_stackManager(new StackManager(this)),
        m_breakpointController(new BreakpointController(this))
{
    configure();
    kDebug(9012) << "GDB script" << config_configGdbScript_ << "\n";

    Q_ASSERT(! debug_controllerExists);
    debug_controllerExists = true;

    connect(this, SIGNAL(event(event_t)), 
            m_variableCollection, SLOT(slotEvent(event_t)));

    connect( this, SIGNAL(showStepInSource(const QString&, int, const QString&)),
             this, SLOT(slotShowStep(const QString&, int)));
}

// **************************************************************************

// Deleting the controller involves shutting down gdb nicely.
// When were attached to a process, we must first detach so that the process
// can continue running as it was before being attached. gdb is quite slow to
// detach from a process, so we must process events within here to get a "clean"
// shutdown.
GDBController::~GDBController()
{
    debug_controllerExists = false;

    if (!stateIsOn(s_dbgNotStarted)) {
        stopDebugger();
        // This currently isn't working, so comment out until it can be resolved - at the moment it just causes a delay on stopping kdevelop
        //m_process->waitForFinished();
    }

    delete commandQueue_;
}

// **************************************************************************

void GDBController::configure()
{
    KConfigGroup config(KGlobal::config(), "GDB Debugger");

    // A a configure.gdb script will prevent these from uncontrolled growth...
    config_configGdbScript_       = config.readEntry("Remote GDB Configure Script", "");
    config_runShellScript_        = config.readEntry("Remote GDB Shell Script", "");
    config_runGdbScript_          = config.readEntry("Remote GDB Run Script", "");

    // PORTING TODO: where is this in the ui?
    config_forceBPSet_            = config.readEntry("Allow Forced Breakpoint Set", true);

    config_dbgTerminal_           = config.readEntry("Separate Terminal For Application IO", false);

    bool old_displayStatic        = config_displayStaticMembers_;
    config_displayStaticMembers_  = config.readEntry("Display Static Members",false);

    bool old_asmDemangle  = config_asmDemangle_;
    config_asmDemangle_   = config.readEntry("Display Demangle Names",true);

    bool old_breakOnLoadingLibrary_ = config_breakOnLoadingLibrary_;
    config_breakOnLoadingLibrary_ = config.readEntry("Try Setting Breakpoints On Loading Libraries",true);

    // FIXME: should move this into debugger part or variable widget.
    int old_outputRadix  = config_outputRadix_;
#if 0
    config_outputRadix_   = DomUtil::readIntEntry("Output Radix", 10);
    varTree_->setRadix(config_outputRadix_);
#endif


    if (( old_displayStatic             != config_displayStaticMembers_   ||
            old_asmDemangle             != config_asmDemangle_            ||
            old_breakOnLoadingLibrary_  != config_breakOnLoadingLibrary_  ||
            old_outputRadix             != config_outputRadix_)           &&
            gdb_)
    {
        bool restart = false;
        if (stateIsOn(s_dbgBusy))
        {
            slotPauseApp();
            restart = true;
        }

        if (old_displayStatic != config_displayStaticMembers_)
        {
            if (config_displayStaticMembers_)
                queueCmd(new GDBCommand(GDBMI::GdbSet, "print static-members on"));
            else
                queueCmd(new GDBCommand(GDBMI::GdbSet, "print static-members off"));
        }
        if (old_asmDemangle != config_asmDemangle_)
        {
            if (config_asmDemangle_)
                queueCmd(new GDBCommand(GDBMI::GdbSet, "print asm-demangle on"));
            else
                queueCmd(new GDBCommand(GDBMI::GdbSet, "print asm-demangle off"));
        }

        // Disabled for MI port.
        if (old_outputRadix != config_outputRadix_)
        {
            queueCmd(new GDBCommand(GDBMI::GdbSet, QString().sprintf("output-radix %d",
                                config_outputRadix_)));

            // FIXME: should do this in variable widget anyway.
            // After changing output radix, need to refresh variables view.
            raiseEvent(program_state_changed);

        }

        if (config_configGdbScript_.isValid())
          queueCmd(new GDBCommand(GDBMI::NonMI, "source " + config_configGdbScript_.path()));


        if (restart)
            queueCmd(new GDBCommand(GDBMI::ExecContinue));
    }
}

// **************************************************************************

void GDBController::addCommand(GDBCommand* cmd)
{
    queueCmd(cmd);
}

void GDBController::addCommand(GDBMI::CommandType type, const QString& str)
{
    queueCmd(new GDBCommand(type, str));
}

void GDBController::addCommandToFront(GDBCommand* cmd)
{
    queueCmd(cmd, QueueAtFront);
}

void GDBController::addCommandBeforeRun(GDBCommand* cmd)
{
    queueCmd(cmd, QueueWhileInterrupted);
}

int  GDBController::currentThread() const
{
    return currentThread_;
}

int GDBController::currentFrame() const
{
    return currentFrame_;
}

// Fairly obvious that we'll add whatever command you give me to a queue
// If you tell me to, I'll put it at the head of the queue so it'll run ASAP
// Not quite so obvious though is that if we are going to run again. then any
// information requests become redundent and must be removed.
// We also try and run whatever command happens to be at the head of
// the queue.
void GDBController::queueCmd(GDBCommand *cmd, QueuePosition queue_where)
{
    if (stateIsOn(s_dbgNotStarted))
    {
        KMessageBox::information(
            qApp->activeWindow(),
            i18n("<b>Gdb command sent when debugger is not running</b><br>"
            "The command was:<br> %1", cmd->initialString()),
            i18n("Internal error"));
        return;
    }

    if (stateReloadInProgress_)
        cmd->setStateReloading(true);

    commandQueue_->enqueue(cmd, queue_where);
    
    kDebug(9012) << "QUEUE: " << cmd->initialString()
                  << (stateReloadInProgress_ ? "(state reloading)" : "");

    setStateOn(s_dbgBusy);
    raiseEvent(debugger_busy);

    executeCmd();
}

bool GDBController::executeCmd()
{
    Q_ASSERT(gdb_);

    if (!gdb_->isReady())
        return false;

    GDBCommand* currentCmd = commandQueue_->nextCommand();
    if (!currentCmd)
        return false;

    if (currentCmd->thread() != -1 && currentCmd->thread() != currentThread_) {
        // put current command back for now
        commandQueue_->enqueue(currentCmd, QueueAtFront);

        // Switching threads will auto-select frame 0, maybe we want a different frame
        if (currentCmd->frame() > 0)
            commandQueue_->enqueue(new GDBCommand(StackSelectFrame, currentCmd->frame()), QueueAtFront);

        currentCmd = new GDBCommand(ThreadSelect, currentCmd->thread());

    } else if (currentCmd->frame() != -1 && currentCmd->frame() != currentFrame_) {
        // put current command back for now
        commandQueue_->enqueue(currentCmd, QueueAtFront);

        currentCmd = new GDBCommand(StackSelectFrame, currentCmd->frame());
    }

    QString commandText = currentCmd->cmdToSend();
    bool bad_command = false;
    QString message;

    int length = commandText.length();
    // No i18n for message since it's mainly for debugging.
    if (length == 0)
    {
        // The command might decide it's no longer necessary to send
        // it.
        if (SentinelCommand* sc = dynamic_cast<SentinelCommand*>(currentCmd))
        {
            kDebug(9012) << "SEND: sentinel command, not sending";
            sc->invokeHandler();
        }
        else
        {
            kDebug(9012) << "SEND: command " << currentCmd->initialString()
                          << "changed its mind, not sending";
        }

        delete currentCmd;
        return executeCmd();
    }
    else
    {
        if (commandText[length-1] != '\n')
        {
            bad_command = true;
            message = "Debugger command does not end with newline";
        }
    }
    if (bad_command)
    {
        KMessageBox::information(qApp->activeWindow(),
                                 i18n("<b>Invalid debugger command</b><br>%1", message),
                                 i18n("Invalid debugger command"));
        return executeCmd();
    }

    gdb_->execute(currentCmd);
    return true;
}

// **************************************************************************

void GDBController::destroyCmds()
{
    commandQueue_->clear();
}

// Pausing an app removes any pending run commands so that the app doesn't
// start again. If we want to be silent then we remove any pending info
// commands as well.
void GDBController::slotPauseApp()
{
    Q_ASSERT(gdb_);

    setStateOn(s_explicitBreakInto);

    gdb_->interrupt();
}

void GDBController::programStopped(const GDBMI::ResultRecord& r)
{
    /* By default, reload all state on program stop.  */
    state_reload_needed = true;
    setStateOff(s_appRunning);

    if (!r.hasField("reason"))
    {
        // FIXME: throw an exception, and add the gdb reply in the
        // caller. Show message box in the caller, not here.
        // FIXME: remove this 'bla-bla-bla'.
        KMessageBox::detailedSorry(
            qApp->activeWindow(),
            i18n("<b>Invalid gdb reply</b>"
                 "<p>The 'stopped' packet does not include the 'reason' field'."),
            i18n("The gdb reply is: bla-bla-bla"),
            i18n("Invalid gdb reply"));
        return;
    }

    QString reason = r["reason"].literal();
    if (reason == "exited-normally" || reason == "exited")
    {
        programNoApp("Exited normally", false);
        programHasExited_ = true;
        state_reload_needed = false;
        return;
    }

    if (reason == "exited-signalled")
    {
        programNoApp(i18n("Exited on signal %1")
                     .arg(r["signal-name"].literal()), false);
        // FIXME: figure out why this variable is needed.
        programHasExited_ = true;
        state_reload_needed = false;
        return;
    }

    if (reason == "watchpoint-scope")
    {
        QString number = r["wpnum"].literal();

        // FIXME: shuld remove this watchpoint
        // But first, we should consider if removing all
        // watchpoinst on program exit is the right thing to
        // do.

        queueCmd(new GDBCommand(GDBMI::ExecContinue));

        state_reload_needed = false;
        return;
    }

    if (reason == "signal-received")
    {
        QString name = r["signal-name"].literal();
        QString user_name = r["signal-meaning"].literal();

        // SIGINT is a "break into running program".
        // We do this when the user set/mod/clears a breakpoint but the
        // application is running.
        // And the user does this to stop the program also.
        bool suppress_reporting = false;
        if (name == "SIGINT" && stateIsOn(s_explicitBreakInto))
        {
            suppress_reporting = true;
            // TODO: check that we do something reasonable on
            // implicit break into program (for setting breakpoints,
            // or whatever).

            setStateOff(s_explicitBreakInto);
            // Will show the source line in the code
            // handling non-special stop kinds, below.
        }

        if (!suppress_reporting)
        {
            // Whenever we have a signal raised then tell the user, but don't
            // end the program as we want to allow the user to look at why the
            // program has a signal that's caused the prog to stop.
            // Continuing from SIG FPE/SEGV will cause a "Cannot ..." and
            // that'll end the program.
            KMessageBox::information(qApp->activeWindow(),
                                     i18n("Program received signal %1 (%2)", name, user_name),
                                     i18n("Received signal"));
        }
    }

    if (!reason.contains("exited"))
    {
        // Update information
        if (r.hasField("thread-id"))
            currentThread_ = r["thread-id"].toInt();

        if (r.hasField("frame")) {
            const GDBMI::Value& frame = r["frame"];
            if (frame.hasField("fullname") 
                && frame.hasField("line") 
                && frame.hasField("addr")) {
                showStepInSource(frame["fullname"].literal(),
                     frame["line"].literal().toInt(),
                     frame["addr"].literal());

                raiseEvent(program_state_changed);
                state_reload_needed = false;
            }
        }
    }

    if (reason == "breakpoint-hit")
    {
        int id = r["bkptno"].literal().toInt();
        emit breakpointHit(id);
    }

    if (reason.contains("watchpoint-trigger"))
    {
        if (reason == "watchpoint-trigger")
        {
            emit watchpointHit(r["wpt"]["number"]
                               .literal().toInt(),
                               r["value"]["old"].literal(),
                               r["value"]["new"].literal());
        }
        else if (reason == "read-watchpoint-trigger")
        {
            emit showMessage("Read watchpoint triggered", 3000);
        }
        else if (reason == "access-watchpoint-trigger")
        {
            emit showMessage("Access watchpoint triggered", 3000);
        }
    }

    if (reason == "function-finished" && r.hasField("gdb-result-var"))
    {
        variables()->watches()->addFinishResult(r["gdb-result-var"].literal());
    }
    else
    {
        variables()->watches()->removeFinishResult();
    }
}


void GDBController::reloadProgramState()
{
    raiseEvent(program_state_changed);
    state_reload_needed = false;
}


// **************************************************************************

// There is no app anymore. This can be caused by program exiting
// an invalid program specified or ...
// gdb is still running though, but only the run command (may) make sense
// all other commands are disabled.
void GDBController::programNoApp(const QString &msg, bool msgBox)
{
    setState(s_appNotStarted|s_programExited|(state_&s_shuttingDown));

    destroyCmds();

    // We're always at frame zero when the program stops
    // and we must reset the active flag
    currentThread_ = -1;
    currentFrame_ = 0;

    // The application has existed, but it's possible that
    // some of application output is still in the pipe. We use
    // different pipes to communicate with gdb and to get application
    // output, so "exited" message from gdb might have arrived before
    // last application output. Get this last bit.

    // Note: this method can be called when we open an invalid
    // core file. In that case, tty_ won't be set.
    if (tty_)
        tty_->readRemaining();

    // Tty is no longer usable, delete it. Without this, QSocketNotifier
    // will continiously bomd STTY with signals, so we need to either disable
    // QSocketNotifier, or delete STTY. The latter is simpler, since we can't
    // reuse it for future debug sessions anyway.

    delete tty_;
    tty_ = 0;

    gdb_->kill();
    gdb_->deleteLater();

    setStateOn(s_dbgNotStarted);

    raiseEvent(program_exited);

    raiseEvent(debugger_exited);

    if (msgBox)
        KMessageBox::information(qApp->activeWindow(), i18n("gdb message:\n%1", msg), i18n("Warning"));

    emit showMessage(msg, 0);
    /* Also show message in gdb window, so that users who
       prefer to look at gdb window know what's up.  */
    emit gdbUserCommandStdout(msg);
}

void GDBController::parseStreamRecord(const GDBMI::StreamRecord& s)
{
    if (s.reason == '~')
    {
        QString line = s.message;
        if (line.startsWith("The program no longer exists")
            || line.startsWith("Program exited")
            || line.startsWith("Program terminated"))
        {
            programNoApp(line, false);
        }
    }
}

void GDBController::handleMiFrameSwitch(const GDBMI::ResultRecord& r)
{
    raiseEvent(thread_or_frame_changed);

    const GDBMI::Value& frame = r["frame"];

    QString file;
    if (frame.hasField("fullname"))
        file = frame["fullname"].literal();
    else if (frame.hasField("file"))
        file = frame["file"].literal();

    int line = -1;
    if (frame.hasField("line"))
        line = frame["line"].literal().toInt();

    showStepInSource(file,
                     line,
                     frame["addr"].literal());
}

bool GDBController::startDebugger()
{
    kDebug(9012) << "Starting debugger controller";

    Q_ASSERT (!gdb_);
    gdb_ = new GDB();

    /** FIXME: connect ttyStdout. It takes QByteArray, so
        I'm not sure what to do.  */
#if 0
    connect(gdb_, SIGNAL(applicationOutput(const QString&)),
            this, SIGNAL(ttyStdout(const QString &)));
#endif
    connect(gdb_, SIGNAL(userCommandOutput(const QString&)), this, 
            SIGNAL(gdbUserCommandStdout(const QString&)));
    connect(gdb_, SIGNAL(internalCommandOutput(const QString&)), this, 
            SIGNAL(gdbInternalCommandStdout(const QString&)));

    connect(gdb_, SIGNAL(ready()), this, SLOT(gdbReady()));
    connect(gdb_, SIGNAL(programStopped(const GDBMI::ResultRecord&)),
            this, SLOT(programStopped(const GDBMI::ResultRecord&)));
    connect(gdb_, SIGNAL(programRunning()),
            this, SLOT(programRunning()));

    connect(gdb_, SIGNAL(streamRecord(const GDBMI::StreamRecord&)),
            this, SLOT(parseStreamRecord(const GDBMI::StreamRecord&)));
    connect(gdb_, SIGNAL(resultRecord(const GDBMI::ResultRecord&)),
            this, SLOT(resultRecord(const GDBMI::ResultRecord&)));
    
    setStateOff(s_dbgNotStarted);

    // Initialise gdb. At this stage gdb is sitting wondering what to do,
    // and to whom. Organise a few things, then set up the tty for the application,
    // and the application itself

    if (config_displayStaticMembers_)
        queueCmd(new GDBCommand(GDBMI::GdbSet, "print static-members on"));
    else
        queueCmd(new GDBCommand(GDBMI::GdbSet, "print static-members off"));

    // This makes gdb pump a variable out on one line.
    queueCmd(new GDBCommand(GDBMI::GdbSet, "width 0"));
    queueCmd(new GDBCommand(GDBMI::GdbSet, "height 0"));

    queueCmd(new GDBCommand(SignalHandle, "SIG32 pass nostop noprint"));
    queueCmd(new GDBCommand(SignalHandle, "SIG41 pass nostop noprint"));
    queueCmd(new GDBCommand(SignalHandle, "SIG42 pass nostop noprint"));
    queueCmd(new GDBCommand(SignalHandle, "SIG43 pass nostop noprint"));

    // Print some nicer names in disassembly output. Although for an assembler
    // person this may actually be wrong and the mangled name could be better.
    if (config_asmDemangle_)
        queueCmd(new GDBCommand(GDBMI::GdbSet, "print asm-demangle on"));
    else
        queueCmd(new GDBCommand(GDBMI::GdbSet, "print asm-demangle off"));

    // make sure output radix is always set to users view.
    queueCmd(new GDBCommand(GDBMI::GdbSet, QString().sprintf("output-radix %d",  config_outputRadix_)));

    if (config_configGdbScript_.isValid())
        queueCmd(new GDBCommand(GDBMI::NonMI, "source " + config_configGdbScript_.path()));

    return true;
}

bool GDBController::startProgram(const KDevelop::IRun& run, int serial)
{
    if (stateIsOn(s_dbgNotStarted))
    {
        // User has already run the debugger, but it's not running.
        // Most likely, the debugger has crashed, and the debuggerpart
        // was left in 'running' state so that the user can examine
        // gdb output or something. But now, need to fully shut down
        // previous debug session.
        stopDebugger();
    }

    if (stateIsOn( s_appNotStarted ) )
    {
        emit showMessage(i18n("Running program"), 1000);
    }

    if (stateIsOn(s_dbgNotStarted))
        if (!startDebugger())
            return false;

    if (stateIsOn(s_shuttingDown)) {
        kDebug() << "Tried to run when debugger shutting down";
        return false;
    }

    // FIXME: what's this?
#if 0
    m_process->setProperty("serial", serial);
#endif

    // Need to set up a new TTY for each run...
    if (tty_)
        delete tty_;

    tty_ = new STTY(config_dbgTerminal_);//, Settings::terminalEmulatorName( *KGlobal::config() ));
    if (!config_dbgTerminal_)
    {
        connect( tty_, SIGNAL(OutOutput(const QByteArray&)), SIGNAL(ttyStdout(const QByteArray&)) );
        connect( tty_, SIGNAL(ErrOutput(const QByteArray&)), SIGNAL(ttyStderr(const QByteArray&)) );
    }

    QString tty(tty_->getSlave());
    if (tty.isEmpty())
    {
        KMessageBox::information(qApp->activeWindow(), i18n("GDB cannot use the tty* or pty* devices.\n"
                                    "Check the settings on /dev/tty* and /dev/pty*\n"
                                    "As root you may need to \"chmod ug+rw\" tty* and pty* devices "
                                    "and/or add the user to the tty group using "
                                    "\"usermod -G tty username\"."), i18n("Warning"));

        delete tty_;
        tty_ = 0;
        return false;
    }

    queueCmd(new GDBCommand(InferiorTtySet, tty));

    // Change the "Working directory" to the correct one
    QString dir = QString::fromLatin1(QFile::encodeName( run.workingDirectory().path() ));
    if (!dir.isEmpty())
        queueCmd(new GDBCommand(EnvironmentCd, dir));

    // Set the run arguments
    if (!run.arguments().isEmpty())
        queueCmd(
            new GDBCommand(GDBMI::ExecArguments, KShell::joinArgs( run.arguments() )));

    // Get the run environment variables pairs into the environstr string
    // in the form of: "ENV_VARIABLE=ENV_VALUE" and send to gdb using the
    // GDBMI::GdbSet, "enviroment" command
    // Note that we quote the variable value due to the possibility of
    // embedded spaces
    QString environstr;
    typedef QPair<QString, QString> QStringPair;

    KDevelop::EnvironmentGroupList l(KGlobal::config());

// FIXME: not what it does, and how to get access to GDB's KProcess
#if 0
    foreach (const QString& envvar, l.createEnvironment(run.environmentKey(), 
                                                        m_process->systemEnvironment()))
        queueCmd(new GDBCommand(GDBMI::GdbSet, "environment " + envvar));
#endif

    // Needed so that breakpoint widget has a chance to insert breakpoints.
    // FIXME: a bit hacky, as we're really not ready for new commands.
    setStateOn(s_dbgBusy);
    raiseEvent(debugger_ready);

    if (!config_runShellScript_.isEmpty()) {
        // Special for remote debug...
        QByteArray tty(tty_->getSlave().toLatin1());
        QByteArray options = QByteArray(">") + tty + QByteArray("  2>&1 <") + tty;

        QProcess *proc = new QProcess;
        QStringList arguments;
        arguments << "-c" << config_runShellScript_.path() +
            " " + run.executable().path().toLatin1() + options;

        proc->start("sh", arguments);
        //PORTING TODO QProcess::DontCare);
    }

    if (!config_runGdbScript_.isEmpty()) {// gdb script at run is requested

        // Race notice: wait for the remote gdbserver/executable
        // - but that might be an issue for this script to handle...

        // Future: the shell script should be able to pass info (like pid)
        // to the gdb script...

        kDebug(9012) << "Running gdb script " << config_runGdbScript_.path();
        queueCmd(new GDBCommand(NonMI, "source " + config_runGdbScript_.path()));

        // Note: script could contain "run" or "continue"
    }
    else
    {
        QFileInfo app(run.executable().path());

        if (!app.exists())
        {
            KMessageBox::error(
                qApp->activeWindow(),
                i18n("<b>Application does not exist</b>"
                      "<p>The application you're trying to debug,<br>"
                      "    %1\n"
                      "<br>does not exist. Check that you've specified "
                      "the right application in the debugger configuration.",
                      app.fileName()),
                i18n("Application does not exist"));

            // FIXME: after this, KDevelop will still show that debugger
            // is running, because DebuggerPart::slotStopDebugger won't be
            // called, and core()->running(this, false) won't be called too.
            stopDebugger();
            return false;
        }

        if (!app.isExecutable())
        {
            KMessageBox::error(
                qApp->activeWindow(),
                i18n("<b>Could not run application '%1'.</b>"
                      "<p>The application does not have the executable bit set. "
                      "Try rebuilding the project, or change permissions "
                      "manually.",
                      app.fileName()),
                i18n("Could not run application"));
            stopDebugger();
            return false;
        }
        else
        {
            queueCmd(new GDBCommand(GDBMI::FileExecAndSymbols, run.executable().path()));
            raiseEvent(connected_to_program);
            queueCmd(new GDBCommand(ExecRun));
        }
    }

    setStateOff(s_appNotStarted|s_programExited);

    return true;
}

// **************************************************************************
//                                SLOTS
//                                *****
// For most of these slots data can only be sent to gdb when it
// isn't busy and it is running.

// **************************************************************************

void GDBController::stopDebugger()
{
    kDebug(9012) << "GDBController::slotStopDebugger() called";
    if (stateIsOn(s_shuttingDown) || !gdb_)
        return;

    setStateOn(s_shuttingDown);
    kDebug(9012) << "GDBController::slotStopDebugger() executing";

    // Get gdb's attention if it's busy. We need gdb to be at the
    // command line so we can stop it.
    if (!gdb_->isReady())
    {
        kDebug(9012) << "gdb busy on shutdown - interruping";
        gdb_->interrupt();
    }

    // If the app is attached then we release it here. This doesn't stop
    // the app running.
    if (stateIsOn(s_attached))
    {
        queueCmd(new GDBCommand(TargetDetach));
        emit gdbUserCommandStdout("(gdb) detach\n");
    }

    // Now try to stop gdb running.
    queueCmd(new GDBCommand(GdbExit));
    emit gdbUserCommandStdout("(gdb) quit");

    // We cannot wait forever, kill gdb after 5 seconds if it's not yet quit
    QTimer::singleShot(5000, this, SLOT(slotKillGdb()));

    breakpoints()->clearExecutionPoint();
}

void GDBController::slotKillGdb()
{
    if (!stateIsOn(s_programExited) && stateIsOn(s_shuttingDown))
    {
        kDebug(9012) << "gdb not shutdown - killing";
        gdb_->kill();

        setState(s_dbgNotStarted | s_appNotStarted);

        raiseEvent(debugger_exited);
    }
}

// **************************************************************************

void GDBController::examineCoreFile(const KUrl& coreFile)
{
    setStateOff(s_programExited|s_appNotStarted);
    setStateOn(s_core);

    if (stateIsOn(s_dbgNotStarted))
      startDebugger();

    // TODO support non-local URLs
    queueCmd(new GDBCommand(NonMI, "core " + coreFile.path()));

    raiseEvent(connected_to_program);
    raiseEvent(program_state_changed);
}

// **************************************************************************

void GDBController::attachToProcess(int pid)
{
    setStateOff(s_appNotStarted|s_programExited);
    setStateOn(s_attached);

    if (stateIsOn(s_dbgNotStarted))
      startDebugger();

    // Currently, we always start debugger with a name of binary,
    // we might be connecting to a different binary completely,
    // so cancel all symbol tables gdb has.
    // We can't omit application name from gdb invocation
    // because for libtool binaries, we have no way to guess
    // real binary name.
    queueCmd(new GDBCommand(FileExecAndSymbols));

    queueCmd(new GDBCommand(TargetAttach, pid));

    raiseEvent(connected_to_program);

    // ...emit a separate MI command to step one instruction more. We'll
    // notice the '*stopped' response from it and proceed as usual.
    queueCmd(new GDBCommand(ExecStepInstruction));
}

// **************************************************************************

void GDBController::slotRun()
{
    if (stateIsOn(s_appNotStarted|s_dbgNotStarted|s_shuttingDown))
        return;

    queueCmd(new GDBCommand(ExecContinue));
}

// **************************************************************************

void GDBController::slotKill()
{
    if (stateIsOn(s_dbgNotStarted|s_shuttingDown))
        return;

    if (stateIsOn(s_dbgBusy))
    {
        slotPauseApp();
    }

    // The -exec-abort is not implemented in gdb
    // queueCmd(new GDBCommand(ExecAbort));
    queueCmd(new GDBCommand(NonMI, "kill"));

    setStateOn(s_appNotStarted);
}

// **************************************************************************

void GDBController::runUntil(const KUrl& url, int line)
{
    if (stateIsOn(s_dbgNotStarted|s_shuttingDown))
        return;

    if (!url.isValid())
        queueCmd(new GDBCommand(ExecUntil, line));
    else
        queueCmd(new GDBCommand(ExecUntil,
                QString("%1:%2").arg(url.path()).arg(line)));
}

// **************************************************************************

void GDBController::jumpTo(const KUrl& url, int line)
{
    if (stateIsOn(s_dbgNotStarted|s_shuttingDown))
        return;

    if (url.isValid()) {
        queueCmd(new GDBCommand(NonMI, QString("tbreak %1:%2").arg(url.path()).arg(line)));
        queueCmd(new GDBCommand(NonMI, QString("jump %1:%2").arg(url.path()).arg(line)));
    }
}

// **************************************************************************

void GDBController::slotStepInto()
{
    if (stateIsOn(s_appNotStarted|s_shuttingDown))
        return;

    queueCmd(new GDBCommand(ExecStep));
}

// **************************************************************************

void GDBController::slotStepIntoInstruction()
{
    if (stateIsOn(s_appNotStarted|s_shuttingDown))
        return;

    queueCmd(new GDBCommand(ExecStepInstruction));
}

// **************************************************************************

void GDBController::slotStepOver()
{
    if (stateIsOn(s_appNotStarted|s_shuttingDown))
        return;

    queueCmd(new GDBCommand(ExecNext));
}

// **************************************************************************

void GDBController::slotStepOverInstruction()
{
    if (stateIsOn(s_appNotStarted|s_shuttingDown))
        return;

    queueCmd(new GDBCommand(ExecNextInstruction));
}

// **************************************************************************

void GDBController::slotStepOut()
{
    if (stateIsOn(s_appNotStarted|s_shuttingDown))
        return;

    queueCmd(new GDBCommand(ExecFinish));
}

// **************************************************************************

void GDBController::selectFrame(int frameNo, int threadNo)
{
    // FIXME: this either should be removed completely, or
    // trigger an error message.
    if (stateIsOn(s_dbgNotStarted|s_shuttingDown))
        return;

    // Will emit the 'thread_or_frame_changed' event.
    GDBCommand* stackInfoFrame = new GDBCommand(StackInfoFrame);
    stackInfoFrame->setThread(threadNo);
    stackInfoFrame->setFrame(frameNo);
    stackInfoFrame->setHandler(this, &GDBController::handleMiFrameSwitch);
    queueCmd(stackInfoFrame);
}

// **************************************************************************

// FIXME: connect to GDB's slot.
void GDBController::defaultErrorHandler(const GDBMI::ResultRecord& result)
{
    QString msg = result["msg"].literal();

    if (msg.contains("No such process"))
    {
        setState(s_appNotStarted|s_programExited);
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
    if (!gdb_->currentCommand()->stateReloading())
        raiseEvent(program_state_changed);
}

void GDBController::resultRecord(const GDBMI::ResultRecord& result)
{
    GDBCommand* cmd = gdb_->currentCommand();
    switch (cmd->type()) {
    case GDBMI::ThreadSelect:
        if (result.hasField("new-thread-id"))
            currentThread_ = result["new-thread-id"].toInt();
        currentFrame_ = 0;
        break;
    case GDBMI::StackSelectFrame:
        currentFrame_ = cmd->command().toInt();
        break;
    default:
        break;
    }       
}

void GDBController::gdbReady()
{
    stateReloadInProgress_ = false;

    if (!executeCmd())
    {
        /* We know that gdb is ready, so if executeCmd returns false
           it means there's nothing in command queue.  */

        if (state_reload_needed)
        {
            kDebug(9012) << "Finishing program stop\n";
            // Set to false right now, so that if 'actOnProgramPauseMI_part2'
            // sends some commands, we won't call it again when handling replies
            // from that commands.
            state_reload_needed = false;
            reloadProgramState();
        }

        kDebug(9012) << "No more commands\n";
        setStateOff(s_dbgBusy);
        raiseEvent(debugger_ready);
    }
}

void GDBController::raiseEvent(event_t e)
{
    if (e == program_exited || e == debugger_exited)
    {
        stateReloadInProgress_ = false;
    }

    if (e == program_state_changed)
    {
        stateReloadInProgress_ = true;
        kDebug(9012) << "State reload in progress\n";
    }

    emit event(e);

    if (e == program_state_changed)
    {
        stateReloadInProgress_ = false;
    }
}

// **************************************************************************

void GDBController::slotUserGDBCmd(const QString& cmd)
{
    queueCmd(new UserCommand(NonMI, cmd));

    // User command can theoreticall modify absolutely everything,
    // so need to force a reload.

    // We can do it right now, and don't wait for user command to finish
    // since commands used to reload all view will be executed after
    // user command anyway.
    //if (!stateIsOn(s_appNotStarted) && !stateIsOn(s_programExited))
    //    raiseEvent(program_state_changed);
}

void GDBController::explainDebuggerStatus()
{
    GDBCommand* currentCmd_ = gdb_->currentCommand();
    QString information = i18n(
        "%1 commands in queue\n"
        "%2 commands being processed by gdb\n"
        "Debugger state: %3\n", 
        commandQueue_->count(), (currentCmd_ ? 1 : 0), state_);

    if (currentCmd_)
    {
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

bool GDBController::stateIsOn(DBGStateFlags state)
{
    return state_ & state;
}

void GDBController::setStateOn(DBGStateFlags stateOn)
{
    DBGStateFlags oldState = state_;

    debugStateChange(state_, state_ | stateOn);
    state_ |= stateOn;

    emit stateChanged(oldState, state_);
}

void GDBController::setStateOff(DBGStateFlags stateOff)
{
    DBGStateFlags oldState = state_;
  
    debugStateChange(state_, state_ & ~stateOff);
    state_ &= ~stateOff;

    emit stateChanged(oldState, state_);
}

void GDBController::setState(DBGStateFlags newState)
{
    DBGStateFlags oldState = state_;
  
    debugStateChange(state_, newState);
    state_ = newState;

    emit stateChanged(oldState, state_);
}

void GDBController::debugStateChange(DBGStateFlags oldState, DBGStateFlags newState)
{
    int delta = oldState ^ newState;
    if (delta)
    {
        QString out = "STATE: ";
        for(int i = 1; i < s_lastDbgState; i <<= 1)
        {
            if (delta & i)
            {
                if (i & newState)
                    out += "+";
                else
                    out += "-";

                bool found = false;
#define STATE_CHECK(name)\
    if (i == name) { out += #name; found = true; }
                STATE_CHECK(s_dbgNotStarted);
                STATE_CHECK(s_appNotStarted);
                STATE_CHECK(s_programExited);
                STATE_CHECK(s_viewBT);
                STATE_CHECK(s_viewBP);
                STATE_CHECK(s_attached);
                STATE_CHECK(s_core);
                STATE_CHECK(s_waitTimer);
                STATE_CHECK(s_shuttingDown);
                STATE_CHECK(s_explicitBreakInto);
                STATE_CHECK(s_dbgBusy);
                STATE_CHECK(s_appRunning);
#undef STATE_CHECK

                if (!found)
                    out += QString::number(i);
                out += " ";

            }
        }
        kDebug(9012) << out;
    }
}

void GDBController::slotRestart()
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
    slotKill();
    slotRun();
}

int GDBController::serial() const
{
    QVariant var;
    // FIXME: no idea what this used to do.
#if 0
    if (m_process)
        var = m_process->property("serial");

    if (var.canConvert(QVariant::Int))
        return var.toInt();
#endif
    return -1;
}

VariableCollection * GDBController::variables() const
{
    return m_variableCollection;
}

StackManager * GDBController::stackManager() const
{
    return m_stackManager;
}

BreakpointController* GDBController::breakpoints() const
{
    return m_breakpointController;
}

void GDBController::slotShowStep(const QString &fileName, int lineNum)
{
    if ( ! fileName.isEmpty() )
    {
        // Debugger counts lines from 1
        breakpoints()->gotoExecutionPoint(KUrl( fileName ), lineNum-1);
    }
    else
    {
        breakpoints()->clearExecutionPoint();
    }
}

void GDBController::programRunning()
{
    setStateOn(s_appRunning);
    raiseEvent(program_running);
}

}

// **************************************************************************
// **************************************************************************
// **************************************************************************

#include "gdbcontroller.moc"
