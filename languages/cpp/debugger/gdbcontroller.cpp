// *************************************************************************
//                          gdbcontroller.cpp  -  description
//                             -------------------
//    begin                : Sun Aug 8 1999
//    copyright            : (C) 1999 by John Birch
//    email                : jbb@kdevelop.org
// **************************************************************************
//
// **************************************************************************
// *                                                                        *
// *   This program is free software; you can redistribute it and/or modify *
// *   it under the terms of the GNU General Public License as published by *
// *   the Free Software Foundation; either version 2 of the License, or    *
// *   (at your option) any later version.                                  *
// *                                                                        *
// **************************************************************************

#include "gdbcontroller.h"

#include "breakpoint.h"
#include "gdbcommand.h"
#include "stty.h"
#include "mi/miparser.h"
#include "gdbglobal.h"

#include "environmentgrouplist.h"

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kwindowsystem.h>
#include <kshell.h>
#include <KConfigGroup>

#include <QDateTime>
#include <QFileInfo>
#include <QRegExp>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QEventLoop>
#include <QByteArray>

#include <iostream>
#include <ctype.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <assert.h>
#include <typeinfo>
using namespace std;
using namespace GDBMI;

// **************************************************************************
//
// Does all the communication between gdb and the kdevelop's debugger code.
// Significatant classes being used here are
//
// GDBParser  - parses the "variable" data using the vartree and varitems
// VarTree    - where the variable data will end up
// FrameStack - tracks the program frames and allows the user to switch between
//              and therefore view the calling funtions and their data
// Breakpoint - Where and what to do with breakpoints.
// STTY       - the tty that the _application_ will run on.
//
// Significant variables
// state_     - be very careful setting this. The controller is totally
//              dependent on this reflecting the correct state. For instance,
//              if the app is busy but we don't think so, then we lose control
//              of the app. The only way to get out of these situations is to
//              delete (stop) the controller.
// currentFrame_
//            - Holds the frame number where and locals/variable information will
//              go to
//
// Certain commands need to be "wrapped", so that the output gdb produces is
// of the form "\032data_id gdb output \032data_id"
// Then a very simple parse can extract this gdb output and hand it off
// to its' respective parser.
// To do this we set the prompt to be \032data_id before the command and then
// reset to \032i to indicate the "idle".
//
// Note that the following does not work because in certain situations
// gdb can get an error in performing the command and therefore will not
// output the final echo. Hence the data will be thrown away.
// (certain "info locals" will generate this error.
//
//  queueCmd(new GDBCommand(QString().sprintf("define printlocal\n"
//                                            "echo \32%c\ninfo locals\necho \32%c\n"
//                                            "end",
//                                            LOCALS, LOCALS)));
// (although replacing echo with "set prompt" appropriately could work Hmmmm.)
//
// Shared libraries and breakpoints
// ================================
// Shared libraries and breakpoints have a problem that has a reasonable solution.
// The problem is that gdb will not accept breakpoints in source that is in a
// shared library that has _not_ _yet_ been opened but will be opened via a
// dlopen.
//
// The solution is to get gdb to tell us when a shared library has been opened.
// This means that when the user sets a breakpoint, we flag this breakpoint as
// pending, try to set the breakpoint and if gdb says it succeeded then flag it
// as active. If gdb is not successful then we leave the breakpoint as pending.
//
// This is known as "lazy breakpoints"
//
// If the user has selected a file that is really outside the program and tried to
// set a breakpoint then this breakpoint will always be pending. I can't do
// anything about that, because it _might_ be in a shared library. If not they
// are either fools or just misguided...
//
// Now that the breakpoint is pending, we need gdb to tell us when a shared
// library has been loaded. We use "set stop-on 1". This breaks on _any_
// library event, and we just try to set the pending breakpoints. Once we're
// done, we then "continue"
//
// Now here's the problem with all this. If the user "step"s over code that
// contains a library dlopen then it'll just keep running, because we receive a
// break and hence end up doing a continue. In this situation, I do _not_
// do a continue but leave it stopped with the status line reflecting the
// stopped state. The frame stack is in the dl routine that caused the stop.
//
// There isn't any way around this, but I could allievate the problem somewhat
// by only doing a "set stop-on 1" when we have pending breakpoints.
//
// **************************************************************************

namespace GDBDebugger
{

// This is here so we can check for startup /shutdown problems
int debug_controllerExists = false;


GDBController::GDBController(QObject* parent)
        : QObject(parent),
        currentFrame_(0),
        viewedThread_(-1),
        holdingZone_(),
        currentCmd_(0),
        tty_(0),
        state_(s_dbgNotStarted|s_appNotStarted),
        programHasExited_(false),
        config_breakOnLoadingLibrary_(true),
        config_forceBPSet_(true),
        config_displayStaticMembers_(false),
        config_asmDemangle_(true),
        config_dbgTerminal_(false),
        config_gdbPath_(),
        config_outputRadix_(10),
        state_reload_needed(false),
        stateReloadInProgress_(false),
        m_process(0)
{
    configure();

    Q_ASSERT(! debug_controllerExists);
    debug_controllerExists = true;
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
    config_gdbPath_               = config.readEntry("GDB Path");

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
            m_process)
    {
        bool restart = false;
        if (stateIsOn(s_dbgBusy))
        {
            pauseApp();
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
    queueCmd(cmd, queue_at_front);
}

void GDBController::addCommandBeforeRun(GDBCommand* cmd)
{
    queueCmd(cmd, queue_before_run);
}

int  GDBController::currentThread() const
{
    return viewedThread_;
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
void GDBController::queueCmd(GDBCommand *cmd, enum queue_where queue_where)
{
    if (stateIsOn(s_dbgNotStarted))
    {
        KMessageBox::information(
            0,
            i18n("<b>Gdb command sent when debugger is not running</b><br>"
            "The command was:<br> %1").arg(cmd->initialString()),
            i18n("Internal error"), "gdb_error");
        return;
    }

    if (stateReloadInProgress_)
        stateReloadingCommands_.insert(cmd);

    if (queue_where == queue_at_front)
        cmdList_.insert(0, cmd);
    else if (queue_where == queue_at_end)
        cmdList_.append (cmd);
    else if (queue_where == queue_before_run)
    {
        unsigned i;
        for (i = 0; i < cmdList_.count(); ++i)
            if (cmdList_.at(i)->isRun())
                break;

        cmdList_.insert(i, cmd);
    }

    kDebug(9012) << "QUEUE: " << cmd->initialString()
                  << (stateReloadInProgress_ ? " (state reloading)\n" : "\n");

    setStateOn(s_dbgBusy);
    emit dbgStatus("", state_);
    raiseEvent(debugger_busy);

    executeCmd();
}

// **************************************************************************

// If the appliction can accept a command and we've got one waiting
// then send it.
// Commands can be just request for data (or change gdbs state in someway)
// or they can be "run" commands. If a command is sent to gdb our internal
// state will get updated.
void GDBController::executeCmd()
{
    if (stateIsOn(s_dbgNotStarted|s_shuttingDown) || !m_process)
    {
        return;
    }

    if (!currentCmd_)
    {
        if (cmdList_.isEmpty())
            return;

        currentCmd_ = cmdList_.takeAt(0);
    }
    else
    {
        return;
    }

    QString commandText = currentCmd_->cmdToSend();
    bool bad_command = false;
    QString message;

    unsigned length = commandText.length();
    // No i18n for message since it's mainly for debugging.
    if (length == 0)
    {
        // The command might decide it's no longer necessary to send
        // it.
        if (SentinelCommand* sc = dynamic_cast<SentinelCommand*>(currentCmd_))
        {
            kDebug(9012) << "SEND: sentinel command, not sending\n";
            sc->invokeHandler();
        }
        else
        {
            kDebug(9012) << "SEND: command " << currentCmd_->initialString()
                          << " changed its mind, not sending\n";
        }

        destroyCurrentCommand();
        executeCmd();
        commandDone();
        return;
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
        KMessageBox::information(0, i18n("<b>Invalid debugger command</b><br>")
                           + message,
                           i18n("Invalid debugger command"), "gdb_error");
        return;
    }

    kDebug(9012) << "SEND: " << commandText;

    m_process->write(commandText.toLatin1(),
                            commandText.length());

    QString prettyCmd = currentCmd_->cmdToSend();
    prettyCmd.replace( QRegExp("set prompt \032.\n"), "" );
    prettyCmd = "(gdb) " + prettyCmd;

    if (currentCmd_->isUserCommand())
        emit gdbUserCommandStdout( prettyCmd );
    else
        emit gdbInternalCommandStdout( prettyCmd );

    emit dbgStatus ("", state_);
}

// **************************************************************************

void GDBController::destroyCmds()
{
    if (currentCmd_)
    {
        destroyCurrentCommand();
    }

    while (!cmdList_.isEmpty())
        delete cmdList_.takeAt(0);
}

// Pausing an app removes any pending run commands so that the app doesn't
// start again. If we want to be silent then we remove any pending info
// commands as well.
void GDBController::pauseApp()
{
    setStateOn(s_explicitBreakInto);

    /* FIXME: need to decide if we really
       need this, and the consistenly mark
       info commands as such.
    int i = cmdList_.count();
    while (i)
    {
        i--;
        DbgCommand *cmd = cmdList_.seek(i);
        if (cmd->isAnInfoCmd())
            delete cmdList_.takeAt(i);
    }
    */

    if (m_process)
        kill(m_process->pid(), SIGINT);
}

void GDBController::actOnProgramPauseMI(const GDBMI::ResultRecord& r)
{
    // Is this stop on shared library load? Gdb smartly does not
    // print any 'reason' field in this case.
    bool shared_library_load = false;
    if (currentCmd_)
    {
        const QStringList& lines = currentCmd_->allStreamOutput();
        for(unsigned int i = 0; i < lines.count(); ++i)
        {
            if (lines[i].startsWith("Stopped due to shared library event"))
            {
                shared_library_load = true;
                break;
            }
        }
    }

    if (shared_library_load)
    {
        raiseEvent(shared_library_loaded);
        queueCmd(new GDBCommand(GDBMI::ExecContinue));
        return;
    }

    if (!r.hasField("reason"))
    {
        // FIXME: throw an exception, and add the gdb reply in the
        // caller. Show message box in the caller, not here.
        // FIXME: remove this 'bla-bla-bla'.
        KMessageBox::detailedSorry(
            0,
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
            emit dbgStatus("Application interrupted", state_);
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
            KMessageBox::information(0,
                                     i18n("Program received signal %1 (%2)")
                                     .arg(name).arg(user_name),
                                     i18n("Received signal"));
        }
    }

    if (reason == "breakpoint-hit")
    {
        int id = r["bkptno"].literal().toInt();
        emit breakpointHit(id);
    }


}


void GDBController::reloadProgramState()
{
    const GDBMI::ResultRecord& r = *last_stop_result;

    // In gdb 6.3, the *stopped reply does not include full
    // name of the source file. Need to send extra command.
    // Don't send it unless there was 'line' field in last *stopped response.
    // The command has a bug that makes it always returns some file/line,
    // even if we're not in one.
    //
    // FIXME: For gdb 6.4, should not send extra commands.
    // That's for later, so that I verify that this three-command
    // approach works fine.
    if (r.hasField("frame") && r["frame"].hasField("line"))
        queueCmd(new GDBCommand(
                     GDBMI::FileListExecSourceFile,
                     "",
                     this,
                     &GDBController::handleMiFileListExecSourceFile));
    else
    {
        maybeAnnounceWatchpointHit();
    }

    emit dbgStatus ("", state_);

    // We're always at frame zero when the program stops
    // and we must reset the active flag
    if (r.hasField("thread-id"))
        viewedThread_ = r["thread-id"].literal().toInt();
    else
        viewedThread_ = -1;
    currentFrame_ = 0;

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
    viewedThread_ = -1;
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

    raiseEvent(program_exited);

    if (msgBox)
        KMessageBox::information(0, i18n("gdb message:\n")+msg,"Warning", "gdb_error");

    emit dbgStatus (msg, state_);
    /* Also show message in gdb window, so that users who
       prefer to look at gdb window know what's up.  */
    emit gdbUserCommandStdout(msg);
}

void GDBController::parseCliLine(const QString& line)
{
    if (line.startsWith("The program no longer exists")
        || line.startsWith("Program exited")
        || line.startsWith("Program terminated"))
    {
        programNoApp(line, false);
        return;
    }

#if 0
    if (strncmp(buf, "No symbol", 9) == 0 ||                    // watch point failed
            strncmp(buf, "Single", 6) == 0 ||                   // Single stepping
            strncmp(buf, "No source file named", 20) == 0      ||   // breakpoint not set
            strncmp(buf, "[Switching to Thread", 20) == 0      ||    //
            strncmp(buf, "[Thread debugging using", 23) == 0   ||
            strncmp(buf, "Current language:", 17) == 0         ||
            strncmp(buf, "Error while mapping shared library sections:", 44) == 0  ||
            strncmp(buf, "Error while reading shared library symbols:", 43) == 0 ||
            *buf == ':' )
    {
        // We don't change state, because this falls out when a run command
        // starts rather than when a run command stops.
        // Or.... it falls out with other messages that _are_ handled.
        return;
    }
#endif

#if 0

    /// @todo - Only do this at start up
    if (
        strstr(buf, "not in executable format:")                ||
        strstr(buf, "No such file or directory.")               ||      // does this fall out?
        strstr(buf, i18n("No such file or directory.").local8Bit())||   // from system via gdb
        strstr(buf, "is not a core dump:")                      ||
        strncmp(buf, "ptrace: No such process.", 24)==0         ||
        strncmp(buf, "ptrace: Operation not permitted.", 32)==0 ||
        strncmp(buf, "No executable file specified.", 29)==0)
    {
        programNoApp(QString(buf), true);
        kDebug(9012) << "Bad file <"  << buf << ">";
        return;
    }
#endif
}

void GDBController::handleMiFileListExecSourceFile(const GDBMI::ResultRecord& r)
{
    if (r.reason != "done")
    {
        return;

        // FIXME: throw an exception here. Move reporting
        // to the caller, who knows the gdb output.
#if 0
        KMessageBox::information(
            0,
            i18n("Invalid gdb reply\n"
                 "Command was: %1\n"
                 "Response is: %2\n"
                 "Invalid response kind: \"%3\"")
            .arg(currentCmd_->rawDbgCommand())
            .arg(buf)
            .arg(r.reason),
            i18n("Invalid gdb reply"), "gdb_error");
#endif
    }

    QString fullname = "";
    if (r.hasField("fullname"))
        fullname = r["fullname"].literal();

    showStepInSource(fullname,
                     r["line"].literal().toInt(),
                     (*last_stop_result)["frame"]["addr"].literal());

    /* Watchpoint hit is announced only after we've highlighted
       the current line. */
    maybeAnnounceWatchpointHit();

    last_stop_result.reset();
}

void GDBController::maybeAnnounceWatchpointHit()
{
    /* For some cases, for example catchpoints,
       gdb does not report any reason at all. */
    if ((*last_stop_result).hasField("reason"))
    {
        QString last_stop_reason = (*last_stop_result)["reason"].literal();

        if (last_stop_reason == "watchpoint-trigger")
        {
            emit watchpointHit((*last_stop_result)["wpt"]["number"]
                               .literal().toInt(),
                               (*last_stop_result)["value"]["old"].literal(),
                               (*last_stop_result)["value"]["new"].literal());
        }
        else if (last_stop_reason == "read-watchpoint-trigger")
        {
            emit dbgStatus ("Read watchpoint triggered", state_);
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

// **************************************************************************
//                                SLOTS
//                                *****
// For most of these slots data can only be sent to gdb when it
// isn't busy and it is running.

// **************************************************************************

bool GDBController::start(const QString& shell, const KDevelop::IRun& run, int serial)
{
    kDebug(9012) << "Starting debugger controller\n";

    Q_ASSERT (!m_process && !tty_);

    m_process = new KProcess();
    m_process->setOutputChannelMode( KProcess::SeparateChannels );

    connect(m_process, SIGNAL(readyReadStandardOutput()), SLOT(readyReadStandardOutput()));
    connect(m_process, SIGNAL(readyReadStandardError()), SLOT(readyReadStandardError()));
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(processFinished(int, QProcess::ExitStatus)));
    connect(m_process, SIGNAL(error(QProcess::ProcessError)),
            SLOT(processErrored(QProcess::ProcessError)));

    m_process->setProperty("serial", serial);

    application_ = run.executable().path();

    QString gdb = "gdb";
    // Prepend path to gdb, if needed. Using QDir,
    // path can either end with slash, or not.
    if (config_gdbPath_.isValid())
    {
        gdb = config_gdbPath_.path();
    }
    QStringList arguments;
    arguments << run.executable().path() << "--interpreter=mi2" << "-quiet";
    if (!shell.isEmpty())
    {
        arguments.insert(0, gdb );
        arguments.insert(0, shell);
        m_process->setShellCommand( KShell::joinArgs( arguments ) );
    }
    else
    {
        m_process->setProgram( gdb, arguments );
    }
    m_process->start();
    emit gdbUserCommandStdout(
                              shell + " " + gdb
                              + " " + run.executable().path()
                              + " --interpreter=mi2 -quiet\n" );

    setStateOff(s_dbgNotStarted);
    emit dbgStatus ("", state_);

    saw_gdb_prompt_ = false;

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

    // Get gdb to notify us of shared library events. This allows us to
    // set breakpoints in shared libraries that are not loaded yet. On each
    // stop, we'll try to set breakpoints again.
    //
    // Gdb 6.3 and later implement so called 'pending breakpoints' that
    // solve this issue cleanly, but:
    // - This is broken for MI -break-insert command (the breakpoint is
    //   not inserted at all, and no error is produced)
    // - MI does not contains notification that pending breakpoint is resolved.
    queueCmd(new GDBCommand(GDBMI::GdbSet, "stop-on-solib-events 1"));


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

    // Change the "Working directory" to the correct one
    queueCmd(new GDBCommand(EnvironmentCd, QString::fromLatin1(QFile::encodeName( run.workingDirectory().path() ))));

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

    foreach (const QString& envvar, l.createEnvironment(run.environmentKey(), m_process->systemEnvironment()))
        queueCmd(new GDBCommand(GDBMI::GdbSet, "environment " + envvar));

    // Needed so that breakpoint widget has a chance to insert breakpoints.
    // FIXME: a bit hacky, as we're really not ready for new commands.
    setStateOn(s_dbgBusy);
    raiseEvent(debugger_ready);
    raiseEvent(connected_to_program);

    // Now gdb has been started and the application has been loaded,
    // BUT the app hasn't been started yet! A run command is about to be issued
    // by whoever is controlling us. Or we might be asked to load a core, or
    // attach to a running process.

    return true;
}

// **************************************************************************

void GDBController::slotStopDebugger()
{
    kDebug(9012) << "GDBController::slotStopDebugger() called";
    if (stateIsOn(s_shuttingDown) || !m_process)
        return;

    setStateOn(s_shuttingDown);
    kDebug(9012) << "GDBController::slotStopDebugger() executing";

    QTime start;
    QTime now;

    // Get gdb's attention if it's busy. We need gdb to be at the
    // command line so we can stop it.
    if (stateIsOn(s_dbgBusy))
    {
        kDebug(9012) << "gdb busy on shutdown - stopping gdb (SIGINT)";
        //TODO:win32 Porting needed
        kill( m_process->pid(), SIGSTOP );
        start = QTime::currentTime();
        while (-1)
        {
            //kapp->eventLoop()->processEvents( QEventLoop::ExcludeUserInput, 20 );
            now = QTime::currentTime();
            if (!stateIsOn(s_dbgBusy) || start.msecsTo( now ) > 2000)
                break;
        }
    }

    // If the app is attached then we release it here. This doesn't stop
    // the app running.
    if (stateIsOn(s_attached))
    {
        const char *detach="detach\n";
        if (!m_process->write(detach, strlen(detach)))
            kDebug(9012) << "failed to write 'detach' to gdb";
        emit gdbUserCommandStdout("(gdb) detach\n");
        start = QTime::currentTime();
        while (-1)
        {
             //kapp->eventLoop()->processEvents( QEventLoop::ExcludeUserInput, 20 );
             now = QTime::currentTime();
             if (!stateIsOn(s_attached) || start.msecsTo( now ) > 2000)
                 break;
        }
    }

    // Now try to stop gdb running.
    const char *quit="quit\n";
    if (!m_process->write(quit, strlen(quit)))
        kDebug(9012) << "failed to write 'quit' to gdb";

    emit gdbUserCommandStdout("(gdb) quit");
    start = QTime::currentTime();
    while (-1)
    {
         //kapp->eventLoop()->processEvents( QEventLoop::ExcludeUserInput, 20 );
         now = QTime::currentTime();
         if (stateIsOn(s_programExited) || start.msecsTo( now ) > 2000)
             break;
    }

    // We cannot wait forever.
    if (!stateIsOn(s_programExited))
    {
        kDebug(9012) << "gdb not shutdown - killing";
        m_process->kill();
    }

    destroyCmds();
    delete m_process;    m_process = 0;
    delete tty_;           tty_ = 0;

    // The gdb output buffer might contain start marker of some
    // previously issued command that crashed gdb (so there's no end marker)
    // If we don't clear this, then after restart, we'll be trying to search
    // for the end marker of the command issued in previous gdb session,
    // and never succeed.
    gdbOutput_ = "";

    setState(s_dbgNotStarted | s_appNotStarted);
    emit dbgStatus (i18n("Debugger stopped"), state_);

    raiseEvent(debugger_exited);
}

// **************************************************************************

void GDBController::slotCoreFile(const QString &coreFile)
{
    setStateOff(s_programExited|s_appNotStarted);
    setStateOn(s_core);

    queueCmd(new GDBCommand(NonMI, "core " + coreFile));

    raiseEvent(connected_to_program);
    raiseEvent(program_state_changed);
}

// **************************************************************************

void GDBController::slotAttachTo(int pid)
{
    setStateOff(s_appNotStarted|s_programExited);
    setStateOn(s_attached);

    // Currently, we always start debugger with a name of binary,
    // we might be connecting to a different binary completely,
    // so cancel all symbol tables gdb has.
    // We can't omit application name from gdb invocation
    // because for libtool binaries, we have no way to guess
    // real binary name.
    queueCmd(new GDBCommand(FileExecAndSymbols));

    // The MI interface does not implements -target-attach yet,
    // and we don't recognize whatever gibberish 'attach' pours out, so...
    queueCmd(new GDBCommand(NonMI,
        QString().sprintf("attach %d", pid)));

    raiseEvent(connected_to_program);

    // ...emit a separate MI command to step one instruction more. We'll
    // notice the '*stopped' response from it and proceed as usual.
    queueCmd(new GDBCommand(ExecStepInstruction));
}

// **************************************************************************

void GDBController::slotRun()
{
    if (stateIsOn(s_dbgNotStarted|s_shuttingDown))
        return;

    if (stateIsOn(s_appNotStarted)) {

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
            KMessageBox::information(0, i18n("GDB cannot use the tty* or pty* devices.\n"
                                       "Check the settings on /dev/tty* and /dev/pty*\n"
                                       "As root you may need to \"chmod ug+rw\" tty* and pty* devices "
                                       "and/or add the user to the tty group using "
                                       "\"usermod -G tty username\"."), "Warning",  "gdb_error");

            delete tty_;
            tty_ = 0;
            return;
        }

        queueCmd(new GDBCommand(InferiorTtySet, tty));

        if (!config_runShellScript_.isEmpty()) {
            // Special for remote debug...
            QByteArray tty(tty_->getSlave().toLatin1());
            QByteArray options = QByteArray(">") + tty + QByteArray("  2>&1 <") + tty;

            QProcess *proc = new QProcess;
            QStringList arguments;
            arguments << "-c" << config_runShellScript_.path() +
                " " + application_.toLatin1() + options;

            proc->start("sh", arguments);
            //PORTING TODO QProcess::DontCare);
        }

        if (!config_runGdbScript_.isEmpty()) {// gdb script at run is requested

            // Race notice: wait for the remote gdbserver/executable
            // - but that might be an issue for this script to handle...

            // Future: the shell script should be able to pass info (like pid)
            // to the gdb script...

            queueCmd(new GDBCommand(NonMI, "source " + config_runGdbScript_.path()));

            // Note: script could contain "run" or "continue"
        }
        else {

            QFileInfo app(application_);

            if (!app.exists())
            {
                KMessageBox::error(
                    0,
                    i18n("<b>Application does not exist</b>"
                         "<p>The application you're trying to debug,<br>"
                         "    %1\n"
                         "<br>does not exist. Check that you've specified "
                         "the right application in the debugger configuration."
                        ).arg(app.fileName()),
                    i18n("Application does not exist"));

                // FIXME: after this, KDevelop will still show that debugger
                // is running, because DebuggerPart::slotStopDebugger won't be
                // called, and core()->running(this, false) won't be called too.
                slotStopDebugger();
                return;
            }
            if (!app.isExecutable())
            {
                KMessageBox::error(
                    0,
                    i18n("<b>Could not run application '%1'.</b>"
                         "<p>The application does not have the executable bit set. "
                         "Try rebuilding the project, or change permissions "
                         "manually."
                        ).arg(app.fileName()),
                    i18n("Could not run application"));
                slotStopDebugger();
            }
            else
            {
                queueCmd(new GDBCommand(ExecRun));
            }
        }
    }
    else {
        removeStateReloadingCommands();

        queueCmd(new GDBCommand(ExecContinue));
    }
    setStateOff(s_appNotStarted|s_programExited);
}


void GDBController::slotKill()
{
    if (stateIsOn(s_dbgNotStarted|s_shuttingDown))
        return;

    if (stateIsOn(s_dbgBusy))
    {
        pauseApp();
    }

    queueCmd(new GDBCommand(ExecAbort));

    setStateOn(s_appNotStarted);
}

// **************************************************************************

void GDBController::slotRunUntil(const QString &fileName, int lineNum)
{
    if (stateIsOn(s_dbgBusy|s_dbgNotStarted|s_shuttingDown))
        return;

    removeStateReloadingCommands();

    if (fileName.isEmpty())
        queueCmd(new GDBCommand(ExecUntil,
                     QString::number(lineNum)));
    else
        queueCmd(new GDBCommand(ExecUntil,
                QString("%s:%d").arg(fileName).arg(lineNum)));
}

// **************************************************************************

void GDBController::slotJumpTo(const QString &fileName, int lineNum)
{
    if (stateIsOn(s_dbgBusy|s_dbgNotStarted|s_shuttingDown))
        return;

    if (!fileName.isEmpty()) {
        queueCmd(new GDBCommand(NonMI, QString().sprintf("tbreak %s:%d", fileName, lineNum)));
        queueCmd(new GDBCommand(NonMI, QString().sprintf("jump %s:%d", fileName, lineNum)));
    }
}

// **************************************************************************

void GDBController::slotStepInto()
{
    if (stateIsOn(s_dbgBusy|s_appNotStarted|s_shuttingDown))
        return;

    removeStateReloadingCommands();

    queueCmd(new GDBCommand(ExecStep));
}

// **************************************************************************

void GDBController::slotStepIntoIns()
{
    if (stateIsOn(s_dbgBusy|s_appNotStarted|s_shuttingDown))
        return;

    removeStateReloadingCommands();

    queueCmd(new GDBCommand(ExecStepInstruction));
}

// **************************************************************************

void GDBController::slotStepOver()
{
    if (stateIsOn(s_dbgBusy|s_appNotStarted|s_shuttingDown))
        return;

    removeStateReloadingCommands();

    queueCmd(new GDBCommand(ExecNext));
}

// **************************************************************************

void GDBController::slotStepOverIns()
{
    if (stateIsOn(s_dbgBusy|s_appNotStarted|s_shuttingDown))
        return;

    removeStateReloadingCommands();

    queueCmd(new GDBCommand(ExecNextInstruction));
}

// **************************************************************************

void GDBController::slotStepOutOff()
{
    if (stateIsOn(s_dbgBusy|s_appNotStarted|s_shuttingDown))
        return;

    removeStateReloadingCommands();

    queueCmd(new GDBCommand(ExecFinish));
}

// **************************************************************************

// Only interrupt a running program.
void GDBController::slotBreakInto()
{
    pauseApp();
}

// **************************************************************************

void GDBController::selectFrame(int frameNo, int threadNo)
{
    // FIXME: this either should be removed completely, or
    // trigger an error message.
    if (stateIsOn(s_dbgBusy|s_dbgNotStarted|s_shuttingDown))
        return;

    if (threadNo != -1)
    {
        if (viewedThread_ != threadNo)
            queueCmd(new GDBCommand(ThreadSelect,
                         QString::number(threadNo)));
    }

    queueCmd(new GDBCommand(StackSelectFrame, frameNo));

    // Will emit the 'thread_or_frame_changed' event.
    queueCmd(new GDBCommand(StackInfoFrame, "",
                            this, &GDBController::handleMiFrameSwitch));


    // FIXME: the above commands might not be the first in queue, and
    // previous commands might using values of 'viewedThread_' or
    // 'currentFrame_'. Ideally, should change the values only after
    // response from gdb.
    viewedThread_ = threadNo;
    currentFrame_ = frameNo;
}

// **************************************************************************

void GDBController::defaultErrorHandler(const GDBMI::ResultRecord& result)
{
    QString msg = result["msg"].literal();

    if (msg.contains("No such process"))
    {
        setState(s_appNotStarted|s_programExited);
        emit dbgStatus (i18n("Process exited"), state_);
        raiseEvent(program_exited);
        return;
    }

    KMessageBox::information(
        0,
        i18n("<b>Debugger error</b>"
             "<p>Debugger reported the following error:"
             "<p><tt>") + result["msg"].literal(),
        i18n("Debugger error"), "gdb_error");

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
    if (!stateReloadingCommands_.contains(currentCmd_))
        raiseEvent(program_state_changed);
}

void GDBController::processMICommandResponse(const GDBMI::ResultRecord& result)
{
    kDebug(9012) << "MI stop reason " << result.reason << "\n";
    if (result.reason == "stopped")
    {
        actOnProgramPauseMI(result);
    }
    else if (result.reason == "done")
    {
        // At least in one case, for 'detach', debuger write
        // command directly, and 'currentCmd_' will be unset.
        // Checking for currentCmd_ is safer in any case.
        if (currentCmd_)
        {
            // Assume that if this command is part of state reloading,
            // then any further commands issued in command handler
            // are part of state reloading as well.
            if (stateReloadingCommands_.contains(currentCmd_))
            {
                stateReloadInProgress_ = true;
            }
            currentCmd_->invokeHandler(result);
            stateReloadInProgress_ = false;
        }
    }
    else if (result.reason == "error")
    {
        // Some commands want to handle errors themself.
        if (currentCmd_ && currentCmd_->handlesError() &&
            currentCmd_->invokeHandler(result))
        {
            // Done, nothing more needed
        }
        else
        {
            defaultErrorHandler(result);
        }
    }
}

// Data from gdb gets processed here.
void GDBController::readyReadStandardOutput()
{
    m_process->setReadChannel(QProcess::StandardOutput);
    readFromProcess(m_process);
}

void GDBController::readyReadStandardError()
{
    // At the moment, just drop a message out and redirect
    m_process->setReadChannel(QProcess::StandardError);
    readFromProcess(m_process);
}

void GDBController::readFromProcess(QProcess* process)
{
    // Append to the back of the holding zone.
    holdingZone_ +=  process->readAll();

    bool ready_for_next_command = false;

    int i;
    bool got_any_command = false;
    // For each gdb reply. In MI mode, each reply is one string.
    forever
    {
        i = holdingZone_.indexOf('\n');
        if (i == -1) {
          ready_for_next_command = true;
          break;
        }

        got_any_command = true;

        QByteArray reply(holdingZone_.left(i));
        holdingZone_ = holdingZone_.mid(i+1);

        kDebug(9012) << "REPLY: " << reply << "\n";

        FileSymbol file;
        file.contents = reply;

        std::auto_ptr<GDBMI::Record> r(mi_parser_.parse(&file));

        if (r.get() == 0)
        {
            // FIXME: Issue an error!
            kDebug(9012) << "Invalid MI message: " << reply << "\n";
            ready_for_next_command = true;
            continue;
        }

        try {

            switch(r->kind)
            {
            case GDBMI::Record::Result: {

                GDBMI::ResultRecord& result = static_cast<GDBMI::ResultRecord&>(*r);

                if (result.reason != "running")
                {
                    kDebug(9012) << "Command execution time "
                                  << commandExecutionTime.elapsed() << " ms.\n";
                }

                /* The currentCmd_ may be NULL here, because when detaching
                   from debugger, we directly write "detach" to gdb and
                   busy-wait for a reply.  Uisng the commands mechanism
                   won't work there, because command result are
                   communicated asynchronously.
                   This is will be fixed in KDevelop4.  */
                if (currentCmd_ && currentCmd_->isUserCommand())
                    emit gdbUserCommandStdout(QString::fromLocal8Bit(reply));
                else
                    emit gdbInternalCommandStdout(QString::fromLocal8Bit(reply) + "\n");

                if (result.reason == "stopped")
                {
                    // Transfers ownership.
                    // Needed so that in
                    //   handleMiFileListExecSourceFile(GDBMI::ResultRecord& r);
                    // we can use the last stop reason.
                    last_stop_result.reset(static_cast<GDBMI::ResultRecord*>(r.get()));
                    r.release();
                    state_reload_needed = true;
                }
                else if (result.reason == "running")
                {
                    setStateOn(s_appRunning);
                    raiseEvent(program_running);
                }

                // All MI commands have just one response, except for
                // run-like command, which result in
                //
                //   ^running
                //
                // followed by
                //
                //   stopped.

                ready_for_next_command = (result.reason != "running");
                if (ready_for_next_command)
                {
                    // Need to do this before procesing response,
                    // so that when processing response we don't
                    // think that application is running.
                    setStateOff(s_appRunning);
                }

                processMICommandResponse(result);


                if (ready_for_next_command)
                {
                    destroyCurrentCommand();
                }


                break;
            }

            case GDBMI::Record::Stream: {

                GDBMI::StreamRecord& s = dynamic_cast<GDBMI::StreamRecord&>(*r);
                /* The way current code works is that we start gdb,
                   and immediately send commands to it without waiting for
                   a prompt.  As result, when we return back to the event
                   loop and read the first line from GDB, currentCmd_ is
                   already set.  But really, we want to output everything
                   that gdb prints prior to prompt -- it might be
                   output from user's .gdbinit that user cares about.  */
                if (!saw_gdb_prompt_
                    || !currentCmd_ || currentCmd_->isUserCommand())
                    emit gdbUserCommandStdout(s.message);
                else
                    emit gdbInternalCommandStdout(s.message);

                if (currentCmd_)
                    currentCmd_->newOutput(s.message);

                parseCliLine(s.message);

                static QRegExp print_output("^\\$(\\d+) = ");
                if (print_output.search(s.message) != -1)
                {
                    kDebug(9012) << "Found 'print' output: " << s.message << "\n";
                    print_command_result = s.message.ascii();
                }

                /* This is output from the program.  Route it to
                   the Application window.  */
                if (s.reason == '@')
                    emit ttyStderr(s.message.toLocal8Bit());

                break;
            }


            case GDBMI::Record::Prompt:
                saw_gdb_prompt_ = true;
                break;
            }
        }
        catch(const std::exception& e)
        {
            KMessageBox::detailedSorry(
                0,
                i18nc("<b>Internal debugger error</b>",
                     "<p>The debugger component encountered internal error while "
                     "processing reply from gdb. Please submit a bug report."),
                i18n("The exception is: %1\n"
                     "The MI response is: %2", e.what(), QString::fromLatin1(reply)),
                i18n("Internal debugger error"));

            destroyCurrentCommand();
            ready_for_next_command = true;
        }
    }

    // check the queue for any commands to send
    if (ready_for_next_command)
    {
        executeCmd();
    }

    if (got_any_command)
        kDebug(9012) << "COMMANDS: " << cmdList_.count() << " in queue, "
                      << int(bool(currentCmd_)) << " executing\n";

    commandDone();
}

void GDBController::commandDone()
{
    bool no_more_commands = (cmdList_.isEmpty() && !currentCmd_);

    if (no_more_commands && state_reload_needed)
    {
        kDebug(9012) << "Finishing program stop\n";
        // Set to false right now, so that if 'actOnProgramPauseMI_part2'
        // sends some commands, we won't call it again when handling replies
        // from that commands.
        state_reload_needed = false;
        reloadProgramState();
    }

    if (no_more_commands)
    {
        kDebug(9012) << "No more commands\n";
        setStateOff(s_dbgBusy);
        emit dbgStatus("", state_);
        raiseEvent(debugger_ready);
    }
}

void GDBController::destroyCurrentCommand()
{
    stateReloadingCommands_.remove(currentCmd_);
    delete currentCmd_;
    currentCmd_ = 0;
}

void GDBController::removeStateReloadingCommands()
{
    int i = cmdList_.count();
    while (i)
    {
        i--;
        GDBCommand* cmd = cmdList_.at(i);
        if (stateReloadingCommands_.contains(cmd));
        {
            kDebug(9012) << "UNQUEUE: " << cmd->initialString() << "\n";
            delete cmdList_.takeAt(i);
        }
    }

    if (stateReloadingCommands_.contains(currentCmd_))
    {
        // This effectively prevents handler for this command
        // to be ever invoked.
        destroyCurrentCommand();
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

void GDBDebugger::GDBController::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{

    bool abnormal = exitCode != 0 || exitStatus != QProcess::NormalExit;
    delete m_process;
    m_process = 0;
    delete tty_;
    tty_ = 0;

    if (abnormal)
        emit debuggerAbnormalExit();

    raiseEvent(debugger_exited);

    destroyCmds();
    setState(s_dbgNotStarted|s_appNotStarted|s_programExited);
    emit dbgStatus (i18n("Process exited"), state_);

    emit gdbUserCommandStdout("(gdb) Process exited\n");
}

void GDBController::processErrored(QProcess::ProcessError error)
{
    if( error = QProcess::FailedToStart )
    {
        KMessageBox::information(
            0,
            i18n("<b>Could not start debugger.</b>"
                 "<p>Could not run '%1'. "
                 "Make sure that the path name is specified correctly."
                , m_process->program()[0]),
            i18n("Could not start debugger"), "gdb_error");
        delete m_process;
        m_process = 0;
        delete tty_;
        tty_ = 0;
        emit debuggerAbnormalExit();

        raiseEvent(debugger_exited);

        destroyCmds();
        setState(s_dbgNotStarted|s_appNotStarted|s_programExited);
        emit dbgStatus (i18n("Process didn't start"), state_);

        emit gdbUserCommandStdout("(gdb) didn't start\n");
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
    QString information("%1 commands in queue\n"
                        "%2 commands being processed by gdb\n"
                        "Debugger state: %3\n");
    information =
        information.arg(cmdList_.count()).arg(currentCmd_ ? 1 : 0)
        .arg(state_);

    if (currentCmd_)
    {
        QString extra("Current command class: '%1'\n"
                      "Current command text: '%2'\n"
                      "Current command origianl text: '%3'\n");

        extra = extra.arg(
            typeid(*currentCmd_).name()).arg(currentCmd_->cmdToSend()).
            arg(currentCmd_->initialString());
        information += extra;
    }

    KMessageBox::information(0, information, "Debugger status");
}

bool GDBController::stateIsOn(int state)
{
    return state_ & state;
}

void GDBController::setStateOn(int stateOn)
{
    debugStateChange(state_, state_ | stateOn);
    state_ |= stateOn;
}

void GDBController::setStateOff(int stateOff)
{
    debugStateChange(state_, state_ & ~stateOff);
    state_ &= ~stateOff;
}

void GDBController::setState(int newState)
{
    debugStateChange(state_, newState);
    state_ = newState;
}

void GDBController::debugStateChange(int oldState, int newState)
{
    int delta = oldState ^ newState;
    if (delta)
    {
        QString out = "STATE: ";
        for(unsigned i = 1; i < s_lastDbgState; i <<= 1)
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
        kDebug(9012) << out << "\n";
    }
}

int GDBController::qtVersion( ) const
{
  return 4;//DomUtil::readIntEntry( "/kdevcppsupport/qt/version", 3 );
}

void GDBController::demandAttention() const
{
    if ( QWidget * w = qApp->activeWindow() )
    {
        KWindowSystem::demandAttention( w->winId(), true );
    }
}

}

// **************************************************************************
// **************************************************************************
// **************************************************************************

#include "gdbcontroller.moc"
