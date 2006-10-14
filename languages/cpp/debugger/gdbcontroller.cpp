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
#include "domutil.h"
#include "settings.h"
#include "mi/miparser.h"

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>

#include <qdatetime.h>
#include <qfileinfo.h>
#include <qregexp.h>
#include <qstring.h>
#include <qdir.h>
#include <qvaluevector.h>
#include <qeventloop.h>

#include <iostream>
#include <ctype.h>
#include <stdlib.h>
#include <typeinfo>
using namespace std;

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


GDBController::GDBController(QDomDocument &projectDom)
        : DbgController(),
        currentFrame_(0),
        viewedThread_(-1),
        holdingZone_(),
        currentCmd_(0),
        currentMemoryCallback_(0),
        tty_(0),
        badCore_(QString()),
        state_(s_dbgNotStarted|s_appNotStarted),
        programHasExited_(false),
        dom(projectDom),
        config_breakOnLoadingLibrary_(true),
        config_forceBPSet_(true),
        config_displayStaticMembers_(false),
        config_asmDemangle_(true),
        config_dbgTerminal_(false),
        config_gdbPath_(),
        config_outputRadix_(10),
        state_reload_needed(false),
        stateReloadInProgress_(false)
{
    configure();
    cmdList_.setAutoDelete(true);

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
    // A a configure.gdb script will prevent these from uncontrolled growth...
    config_configGdbScript_       = DomUtil::readEntry(dom, "/kdevdebugger/general/configGdbScript").latin1();
    config_runShellScript_        = DomUtil::readEntry(dom, "/kdevdebugger/general/runShellScript").latin1();
    config_runGdbScript_          = DomUtil::readEntry(dom, "/kdevdebugger/general/runGdbScript").latin1();

//  add macros for reading QStrings? or in configGdbScript?
    config_forceBPSet_            = DomUtil::readBoolEntry(dom, "/kdevdebugger/general/allowforcedbpset", true);
    config_dbgTerminal_           = DomUtil::readBoolEntry(dom, "/kdevdebugger/general/separatetty", false);
    config_gdbPath_               = DomUtil::readEntry(dom, "/kdevdebugger/general/gdbpath");

    bool old_displayStatic        = config_displayStaticMembers_;
    config_displayStaticMembers_  = DomUtil::readBoolEntry(dom, "/kdevdebugger/display/staticmembers",false);

    bool old_asmDemangle  = config_asmDemangle_;
    config_asmDemangle_   = DomUtil::readBoolEntry(dom, "/kdevdebugger/display/demanglenames",true);

    bool old_breakOnLoadingLibrary_ = config_breakOnLoadingLibrary_;
    config_breakOnLoadingLibrary_ = DomUtil::readBoolEntry(dom, "/kdevdebugger/general/breakonloadinglibs",true);

    // FIXME: should move this into debugger part or variable widget.
    int old_outputRadix  = config_outputRadix_;
#if 0
    config_outputRadix_   = DomUtil::readIntEntry(dom, "/kdevdebugger/display/outputradix", 10);
    varTree_->setRadix(config_outputRadix_);
#endif
    

    if (( old_displayStatic             != config_displayStaticMembers_   ||
            old_asmDemangle             != config_asmDemangle_            ||
            old_breakOnLoadingLibrary_  != config_breakOnLoadingLibrary_  ||
            old_outputRadix             != config_outputRadix_)           &&
            dbgProcess_)
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
                queueCmd(new GDBCommand("set print static-members on", NOTRUNCMD,
                                        NOTINFOCMD));
            else
                queueCmd(new GDBCommand("set print static-members off", NOTRUNCMD,
                                        NOTINFOCMD));
        }
        if (old_asmDemangle != config_asmDemangle_)
        {
            if (config_asmDemangle_)
                queueCmd(new GDBCommand("set print asm-demangle on", NOTRUNCMD,
                                        NOTINFOCMD));
            else
                queueCmd(new GDBCommand("set print asm-demangle off", NOTRUNCMD,
                                        NOTINFOCMD));
        }

        // Disabled for MI port.
#if 0
        if (old_breakOnLoadingLibrary_ != config_breakOnLoadingLibrary_)
        {
            if (config_breakOnLoadingLibrary_)
                queueCmd(new GDBCommand("set stop-on 1", NOTRUNCMD, NOTINFOCMD));
            else
                queueCmd(new GDBCommand("set stop-on 0", NOTRUNCMD, NOTINFOCMD));
        }
#endif

        if (old_outputRadix != config_outputRadix_)
        {
            queueCmd(new GDBCommand(QCString().sprintf("set output-radix %d",
                                config_outputRadix_), NOTRUNCMD, NOTINFOCMD));

            // FIXME: should do this in variable widget anyway.
            // After changing output radix, need to refresh variables view.
            raiseEvent(program_state_changed);

        }
        
        if (!config_configGdbScript_.isEmpty())
          queueCmd(new GDBCommand("source " + config_configGdbScript_, NOTINFOCMD, 0));

        if (restart)
            queueCmd(new GDBCommand("-exec-continue", NOTINFOCMD, 0));
    }
}

// **************************************************************************

void GDBController::addCommand(GDBCommand* cmd)
{
    queueCmd(cmd);
}

void GDBController::addCommandToFront(GDBCommand* cmd)
{
    queueCmd(cmd, true);
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
void GDBController::queueCmd(GDBCommand *cmd, bool executeNext)
{
    if (stateIsOn(s_dbgNotStarted))
    {
        KMessageBox::error(
            0, 
            i18n("<b>Gdb command sent when debugger is not running</b><br>"
            "The command was:<br> %1").arg(cmd->rawDbgCommand()),
            i18n("Internal error"));
        return;                    
    }

    if (stateReloadInProgress_)
        stateReloadingCommands_.insert(cmd);

    if (executeNext)
        cmdList_.insert(0, cmd);
    else
        cmdList_.append (cmd);

    kdDebug(9012) << "QUEUE: " << cmd->rawDbgCommand() 
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
    if (stateIsOn(s_dbgNotStarted|s_waitForWrite|s_shuttingDown) || !dbgProcess_)
    {
        return;
    }

    if (!currentCmd_)
    {
        if (cmdList_.isEmpty())
            return;

        currentCmd_ = cmdList_.take(0);
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
            kdDebug(9012) << "SEND: sentinel command, not sending\n";
            sc->invokeHandler();
        }
        else
        {
            kdDebug(9012) << "SEND: command " << currentCmd_->rawDbgCommand()
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
        KMessageBox::error(0, i18n("<b>Invalid debugger command</b><br>")
                           + message,
                           i18n("Invalid debugger command"));
        return;
    }

    kdDebug(9012) << "SEND: " << commandText;

    dbgProcess_->writeStdin(commandText.local8Bit(),
                            commandText.length());
    setStateOn(s_waitForWrite);

    QString prettyCmd = currentCmd_->cmdToSend();
    prettyCmd.replace( QRegExp("set prompt \032.\n"), "" );
    prettyCmd = "(gdb) " + prettyCmd;

    if (currentCmd_->typeMatch(USERCMD))
        emit gdbUserCommandStdout( prettyCmd.latin1() );
    else
        emit gdbInternalCommandStdout( prettyCmd.latin1() );

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
        delete cmdList_.take(0);
}

// Pausing an app removes any pending run commands so that the app doesn't
// start again. If we want to be silent then we remove any pending info
// commands as well.
void GDBController::pauseApp()
{
    setStateOn(s_explicitBreakInto);

    int i = cmdList_.count();
    while (i)
    {
        i--;
        DbgCommand *cmd = cmdList_.at(i);
        if (cmd->isAnInfoCmd())
            delete cmdList_.take(i);
    }

    if (dbgProcess_)
        dbgProcess_->kill(SIGINT);
}

void GDBController::actOnProgramPauseMI(const GDBMI::ResultRecord& r)
{
    // Is this stop on shared library load? Gdb smartly does not
    // print any 'reason' field in this case.
    bool shared_library_load = false;
    if (currentCmd_)
    {
        const QValueVector<QString>& lines = currentCmd_->allStreamOutput();
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
        queueCmd(new GDBCommand("-exec-continue", RUNCMD, NOTINFOCMD));        
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

        queueCmd(new GDBCommand("-exec-continue",
                               RUNCMD, NOTINFOCMD));

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
                     "-file-list-exec-source-file",
                     this,
                     &GDBController::handleMiFileListExecSourceFile));
    else
    {
        announceWatchpointHit();
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
        KMessageBox::error(0, i18n("gdb message:\n")+msg);

    emit dbgStatus (msg, state_);
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
        kdDebug(9012) << "Bad file <"  << buf << ">" << endl;
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
        KMessageBox::error(
            0, 
            i18n("Invalid gdb reply\n"
                 "Command was: %1\n"
                 "Response is: %2\n"
                 "Invalid response kind: \"%3\"")
            .arg(currentCmd_->rawDbgCommand())
            .arg(buf)
            .arg(r.reason), 
            i18n("Invalid gdb reply"));
#endif
    }
    
    QString fullname = "";
    if (r.hasField("fullname"))
        fullname = r["fullname"].literal();
    
    showStepInSource(fullname,
                     r["line"].literal().toInt(),
                     (*last_stop_result)["frame"]["addr"].literal());

    announceWatchpointHit();
 
    last_stop_result.reset();
}

void GDBController::announceWatchpointHit()
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

bool GDBController::start(const QString& shell, const DomUtil::PairList& run_envvars, const QString& run_directory, const QString &application, const QString& run_arguments)
{
    kdDebug(9012) << "Starting debugger controller\n";
    badCore_ = QString();

    Q_ASSERT (!dbgProcess_ && !tty_);

    dbgProcess_ = new KProcess;

    connect( dbgProcess_, SIGNAL(receivedStdout(KProcess *, char *, int)),
             this,        SLOT(slotDbgStdout(KProcess *, char *, int)) );

    connect( dbgProcess_, SIGNAL(receivedStderr(KProcess *, char *, int)),
             this,        SLOT(slotDbgStderr(KProcess *, char *, int)) );

    connect( dbgProcess_, SIGNAL(wroteStdin(KProcess *)),
             this,        SLOT(slotDbgWroteStdin(KProcess *)) );

    connect( dbgProcess_, SIGNAL(processExited(KProcess*)),
             this,        SLOT(slotDbgProcessExited(KProcess*)) );

    application_ = application;

    QString gdb = "gdb";
    // Prepend path to gdb, if needed. Using QDir,
    // path can either end with slash, or not.
    if (!config_gdbPath_.isEmpty())
    {
        gdb = config_gdbPath_;
    }

    if (!shell.isEmpty())
    {
        *dbgProcess_ << "/bin/sh" << "-c" << shell + " " + gdb + 
                      + " " + application + " --interpreter=mi2 -quiet";
        emit gdbUserCommandStdout(
            QString( "/bin/sh -c " + shell + " " + gdb
                     + " " + application 
                     + " --interpreter=mi2 -quiet" ).latin1());
    }
    else
    {
        *dbgProcess_ << gdb << application
                     << "-interpreter=mi2" << "-quiet";
        emit gdbUserCommandStdout(
            QString( gdb + " " + application +
                     " --interpreter=mi2 -quiet" ).latin1());
    }

    if (!dbgProcess_->start( KProcess::NotifyOnExit,
                             KProcess::Communication(KProcess::All)))
    {
        KMessageBox::error(
            0, 
            i18n("<b>Could not start debugger.</b>"
                 "<p>Could not run '%1'. "
                 "Make sure that the path name is specified correctly."
                ).arg(dbgProcess_->args()[0]),
            i18n("Could not start debugger"));

        return false;
    }

    setStateOff(s_dbgNotStarted);
    emit dbgStatus ("", state_);

    // Initialise gdb. At this stage gdb is sitting wondering what to do,
    // and to whom. Organise a few things, then set up the tty for the application,
    // and the application itself

    // The following two are not necessary in MI, and the first one
    // just breaks MI completely.
    // queueCmd(new GDBCommand("set edit off", NOTRUNCMD, NOTINFOCMD, 0));
    // queueCmd(new GDBCommand("set confirm off", NOTRUNCMD, NOTINFOCMD));

    if (config_displayStaticMembers_)
        queueCmd(new GDBCommand("set print static-members on", NOTRUNCMD,
                                    NOTINFOCMD));
    else
        queueCmd(new GDBCommand("set print static-members off", NOTRUNCMD, NOTINFOCMD));

    // This makes gdb pump a variable out on one line.
    queueCmd(new GDBCommand("set width 0", NOTRUNCMD, NOTINFOCMD));
    queueCmd(new GDBCommand("set height 0", NOTRUNCMD, NOTINFOCMD));

    // Get gdb to notify us of shared library events. This allows us to
    // set breakpoints in shared libraries that are not loaded yet. On each
    // stop, we'll try to set breakpoints again.
    //
    // Gdb 6.3 and later implement so called 'pending breakpoints' that
    // solve this issue cleanly, but:
    // - This is broken for MI -break-insert command (the breakpoint is
    //   not inserted at all, and no error is produced)
    // - MI does not contains notification that pending breakpoint is resolved.
    queueCmd(new GDBCommand("set stop-on-solib-events 1", 
                            NOTRUNCMD, NOTINFOCMD));


    queueCmd(new GDBCommand("handle SIG32 pass nostop noprint", NOTRUNCMD,
                            NOTINFOCMD));
    queueCmd(new GDBCommand("handle SIG41 pass nostop noprint", NOTRUNCMD,
                            NOTINFOCMD));
    queueCmd(new GDBCommand("handle SIG42 pass nostop noprint", NOTRUNCMD,
                            NOTINFOCMD));
    queueCmd(new GDBCommand("handle SIG43 pass nostop noprint", NOTRUNCMD,
                            NOTINFOCMD));

    // Print some nicer names in disassembly output. Although for an assembler
    // person this may actually be wrong and the mangled name could be better.
    if (config_asmDemangle_)
        queueCmd(new GDBCommand("set print asm-demangle on", NOTRUNCMD, NOTINFOCMD));
    else
        queueCmd(new GDBCommand("set print asm-demangle off", NOTRUNCMD, NOTINFOCMD));

    // make sure output radix is always set to users view.
    queueCmd(new GDBCommand(QCString().sprintf("set output-radix %d",  config_outputRadix_), NOTRUNCMD, NOTINFOCMD));
       
    // Change the "Working directory" to the correct one
    QCString tmp( "cd " + QFile::encodeName( run_directory ));
    queueCmd(new GDBCommand(tmp, NOTRUNCMD, NOTINFOCMD));

    // Set the run arguments
    if (!run_arguments.isEmpty())
        queueCmd(new GDBCommand(QCString("set args ") + run_arguments.latin1(), NOTRUNCMD, NOTINFOCMD));

    // Get the run environment variables pairs into the environstr string
    // in the form of: "ENV_VARIABLE=ENV_VALUE" and send to gdb using the
    // "set enviroment" command
    // Note that we quote the variable value due to the possibility of
    // embedded spaces
    QString environstr;
    DomUtil::PairList::ConstIterator it;
    for (it = run_envvars.begin(); it != run_envvars.end(); ++it)
    {
        environstr = "set environment ";
        environstr += (*it).first;
        environstr += "=";
        environstr += (*it).second;
        queueCmd(new GDBCommand(environstr.latin1(), NOTRUNCMD, NOTINFOCMD));
    }

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
    kdDebug(9012) << "GDBController::slotStopDebugger() called" << endl;
    if (stateIsOn(s_shuttingDown) || !dbgProcess_)
        return;

    setStateOn(s_shuttingDown);
    kdDebug(9012) << "GDBController::slotStopDebugger() executing" << endl;

    QTime start;
    QTime now;

    // Get gdb's attention if it's busy. We need gdb to be at the
    // command line so we can stop it.
    if (stateIsOn(s_dbgBusy))
    {
        kdDebug(9012) << "gdb busy on shutdown - stopping gdb (SIGINT)" << endl;
        dbgProcess_->kill(SIGINT);
        start = QTime::currentTime();
        while (-1)
        {
            kapp->eventLoop()->processEvents( QEventLoop::ExcludeUserInput, 20 );
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
        if (!dbgProcess_->writeStdin(detach, strlen(detach)))
            kdDebug(9012) << "failed to write 'detach' to gdb" << endl;
        emit gdbUserCommandStdout("(gdb) detach\n");
        start = QTime::currentTime();
        while (-1)
        {
             kapp->eventLoop()->processEvents( QEventLoop::ExcludeUserInput, 20 );
             now = QTime::currentTime();
             if (!stateIsOn(s_attached) || start.msecsTo( now ) > 2000)
                 break;
        }
    }

    // Now try to stop gdb running.
    const char *quit="quit\n";
    if (!dbgProcess_->writeStdin(quit, strlen(quit)))
        kdDebug(9012) << "failed to write 'quit' to gdb" << endl;

    emit gdbUserCommandStdout("(gdb) quit");
    start = QTime::currentTime();
    while (-1)
    {
         kapp->eventLoop()->processEvents( QEventLoop::ExcludeUserInput, 20 );
         now = QTime::currentTime();
         if (stateIsOn(s_programExited) || start.msecsTo( now ) > 2000)
             break;
    }

    // We cannot wait forever.
    if (!stateIsOn(s_programExited))
    {
        kdDebug(9012) << "gdb not shutdown - killing" << endl;
        dbgProcess_->kill(SIGKILL);
    }

    destroyCmds();
    delete dbgProcess_;    dbgProcess_ = 0;
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

    queueCmd(new GDBCommand(QCString("core ") + coreFile.latin1(), 
                                NOTINFOCMD, 0));
   
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
    queueCmd(new GDBCommand(QString("file"), NOTINFOCMD, 0));

    // The MI interface does not implements -target-attach yet,
    // and we don't recognize whatever gibberish 'attach' pours out, so...
    queueCmd(new GDBCommand(
        QCString().sprintf("attach %d", pid), NOTINFOCMD, 0));

    raiseEvent(connected_to_program);
    
    // ...emit a separate MI command to step one instruction more. We'll 
    // notice the '*stopped' response from it and proceed as usual.
    queueCmd(new GDBCommand("-exec-step-instruction", NOTINFOCMD, 0));
}

// **************************************************************************

void GDBController::slotRun()
{
    if (stateIsOn(s_dbgNotStarted|s_shuttingDown))
        return;

    if (stateIsOn(s_appNotStarted)) {

        // Create tty that will be used to get output from the program, as opposed 
        // to output from gdb itself. Need to do this before each run, as when
        // the program ends, we need to delete STTY object -- otherwise, it will
        // be repeatedly reading from pipe, eating 100% CPU.
        // It might be possible to just disable QSockedNotifier inside STTY object,
        // but I'm not sure it's possible to revive the PTY. Creating it fresh
        // is more reliable.

        delete tty_;        
        tty_ = new STTY(config_dbgTerminal_, Settings::terminalEmulatorName( *kapp->config() ));
        if (!config_dbgTerminal_)
        {
            connect( tty_, SIGNAL(OutOutput(const char*)), SIGNAL(ttyStdout(const char*)) );
            connect( tty_, SIGNAL(ErrOutput(const char*)), SIGNAL(ttyStderr(const char*)) );
        }
        
        QString tty(tty_->getSlave());
        if (tty.isEmpty())
        {
            KMessageBox::error(0, i18n("GDB cannot use the tty* or pty* devices.\n"
                                       "Check the settings on /dev/tty* and /dev/pty*\n"
                                       "As root you may need to \"chmod ug+rw\" tty* and pty* devices "
                                       "and/or add the user to the tty group using "
                                       "\"usermod -G tty username\"."));
            
            delete tty_;
            tty_ = 0;
            return;
        }
        
        queueCmd(new GDBCommand(QCString("tty ")+tty.latin1(), NOTINFOCMD));        

        if (!config_runShellScript_.isEmpty()) {
            // Special for remote debug...
            QCString tty(tty_->getSlave().latin1());
            QCString options = QCString(">") + tty + QCString("  2>&1 <") + tty;

            KProcess *proc = new KProcess;

            *proc << "sh" << "-c";
            *proc << config_runShellScript_ +
                " " + application_.latin1() + options;
            proc->start(KProcess::DontCare);
        }

        if (!config_runGdbScript_.isEmpty()) {// gdb script at run is requested

            // Race notice: wait for the remote gdbserver/executable
            // - but that might be an issue for this script to handle...

            // Future: the shell script should be able to pass info (like pid)
            // to the gdb script...

            queueCmd(new GDBCommand("source " + config_runGdbScript_,
                                    NOTINFOCMD, 0));

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
                queueCmd(new GDBCommand("-exec-run", NOTINFOCMD, 0));
            }
        }
    }
    else {
        removeStateReloadingCommands();

        queueCmd(new GDBCommand("-exec-continue", NOTINFOCMD, 0));
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

    queueCmd(new GDBCommand("kill", NOTINFOCMD, 0));

    setStateOn(s_appNotStarted);
}

// **************************************************************************

void GDBController::slotRunUntil(const QString &fileName, int lineNum)
{
    if (stateIsOn(s_dbgBusy|s_dbgNotStarted|s_shuttingDown))
        return;

    removeStateReloadingCommands();

    if (fileName.isEmpty())
        queueCmd(new GDBCommand( QCString().sprintf("-exec-until %d", lineNum),
                                NOTINFOCMD, 0));
    else
        queueCmd(new GDBCommand(
                QCString().sprintf("-exec-until %s:%d", fileName.latin1(), lineNum),
                                NOTINFOCMD, 0));
}

// **************************************************************************

void GDBController::slotJumpTo(const QString &fileName, int lineNum)
{
    if (stateIsOn(s_dbgBusy|s_dbgNotStarted|s_shuttingDown))
        return;

    if (!fileName.isEmpty()) {
        queueCmd(new GDBCommand(QCString().sprintf("tbreak %s:%d", fileName.latin1(), lineNum), NOTINFOCMD, 0));
        queueCmd(new GDBCommand(QCString().sprintf("jump %s:%d", fileName.latin1(), lineNum), NOTINFOCMD, 0));
    }
}

// **************************************************************************

void GDBController::slotStepInto()
{
    if (stateIsOn(s_dbgBusy|s_appNotStarted|s_shuttingDown))
        return;

    removeStateReloadingCommands();

    queueCmd(new GDBCommand("-exec-step", this,
                            &GDBController::handleExecCommandError,
                            true /* handles errors */));
}

// **************************************************************************

void GDBController::slotStepIntoIns()
{
    if (stateIsOn(s_dbgBusy|s_appNotStarted|s_shuttingDown))
        return;

    removeStateReloadingCommands();

    queueCmd(new GDBCommand("-exec-step-instruction", this,
                            &GDBController::handleExecCommandError,
                            true /* handles errors */));
}

// **************************************************************************

void GDBController::slotStepOver()
{
    if (stateIsOn(s_dbgBusy|s_appNotStarted|s_shuttingDown))
        return;

    removeStateReloadingCommands();

    queueCmd(new GDBCommand("-exec-next", this,
                            &GDBController::handleExecCommandError,
                            true /* handles errors */));
}

// **************************************************************************

void GDBController::slotStepOverIns()
{
    if (stateIsOn(s_dbgBusy|s_appNotStarted|s_shuttingDown))
        return;

    removeStateReloadingCommands();

    queueCmd(new GDBCommand("-exec-next-instruction", this,
                            &GDBController::handleExecCommandError,
                            true /* handles errors */));
}

// **************************************************************************

void GDBController::slotStepOutOff()
{
    if (stateIsOn(s_dbgBusy|s_appNotStarted|s_shuttingDown))
        return;

    removeStateReloadingCommands();

    queueCmd(new GDBCommand("-exec-finish", NOTINFOCMD, 0));
}

// **************************************************************************

// Only interrupt a running program.
void GDBController::slotBreakInto()
{
    pauseApp();
}

// **************************************************************************


// **************************************************************************

void GDBController::slotDisassemble(const QString &start, const QString &end)
{
    if (stateIsOn(s_dbgBusy|s_dbgNotStarted|s_shuttingDown))
        return;

    QCString cmd = QCString().sprintf("disassemble %s %s", start.latin1(), end.latin1());
    queueCmd(new GDBCommand(cmd, INFOCMD, DISASSEMBLE));
}

// **************************************************************************

void GDBController::slotMemoryDump(
    MemoryCallback* callback,
    const QString &address, const QString &amount)
{
    if (stateIsOn(s_dbgBusy|s_dbgNotStarted|s_shuttingDown))
        return;

    QCString cmd = QCString().sprintf("x/%sb %s", 
                                      amount.stripWhiteSpace().latin1(),
                                      address.latin1());
    currentMemoryCallback_ = callback;
    queueCmd(new GDBCommand(cmd, INFOCMD, MEMDUMP));
}

// **************************************************************************

void GDBController::slotRegisters()
{
    if (stateIsOn(s_dbgBusy|s_dbgNotStarted|s_shuttingDown))
        return;

    queueCmd(new GDBCommand("info all-registers", INFOCMD, REGISTERS));
}

// **************************************************************************

void GDBController::slotLibraries()
{
    if (stateIsOn(s_dbgBusy|s_dbgNotStarted|s_shuttingDown))
        return;

    queueCmd(new GDBCommand("info sharedlibrary", INFOCMD, LIBRARIES));
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
            queueCmd(new GDBCommand(
                         QString("-thread-select %1").arg(threadNo).ascii(),
                         INFOCMD));
    }

    queueCmd(new GDBCommand(
                 QString("-stack-select-frame %1").arg(frameNo).ascii(),
                 INFOCMD));

    // Will emit the 'thread_or_frame_changed' event.
    queueCmd(new GDBCommand("-stack-info-frame",
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
    KMessageBox::error(
        0, 
        i18n("<b>Debugger error</b>" 
             "<p>Debugger reported the following error:"
             "<p><tt>") + result["msg"].literal(),
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
    if (stateReloadingCommands_.count(currentCmd_) == 0)
        raiseEvent(program_state_changed);    
}

void GDBController::handleExecCommandError(const GDBMI::ResultRecord& result)
{
    QString msg = result["msg"].literal();

    if (msg.contains("No such process"))
    {
        setState(s_appNotStarted|s_programExited);
        emit dbgStatus (i18n("Process exited"), state_);
        raiseEvent(program_exited);
        return;
    }

    defaultErrorHandler(result);
}

void GDBController::processMICommandResponse(const GDBMI::ResultRecord& result)
{
    kdDebug(9012) << "MI stop reason " << result.reason << "\n";
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
            if (stateReloadingCommands_.count(currentCmd_))
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
        if (currentCmd_->handlesError() &&
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
void GDBController::slotDbgStdout(KProcess *, char *buf, int buflen)
{
    static bool parsing = false;

    QCString msg(buf, buflen+1);

    // Copy the data out of the KProcess buffer before it gets overwritten
    // Append to the back of the holding zone.
    holdingZone_ +=  QCString(buf, buflen+1);

    // Already parsing? then get out quick.
    // VP, 2006-01-30. I'm not sure how this could happen, since
    // parsing of gdb reply should not ever execute Qt message loop. Except,
    // maybe, when we pop up a message box. But even in that case,
    // it's likely we won't return to slotDbgStdout again.
    if (parsing)
    {
        kdDebug(9012) << "Already parsing" << endl;
        return;
    }

    bool ready_for_next_command = false;
    
    int i;
    bool got_any_command = false;
    // For each gdb reply. In MI mode, each reply is one string.
    while((i = holdingZone_.find('\n')) != -1)
    {
        got_any_command = true;

        QCString reply(holdingZone_.left(i));
        holdingZone_ = holdingZone_.mid(i+1);

        kdDebug(9012) << "REPLY: " << reply << "\n";

        FileSymbol file;
        file.contents = reply;

        std::auto_ptr<GDBMI::Record> r(mi_parser_.parse(&file));

        if (r.get() == 0)
        {
            // FIXME: Issue an error!
            kdDebug(9012) << "Invalid MI message: " << reply << "\n";
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
                    kdDebug(9012) << "Command execution time " 
                                  << commandExecutionTime.elapsed() << " ms.\n";
                }

                if (!currentCmd_ || currentCmd_->typeMatch(USERCMD))
                    emit gdbUserCommandStdout(reply);
                else
                    emit gdbInternalCommandStdout(reply + "\n");

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
                if (!currentCmd_ || currentCmd_->typeMatch(USERCMD))
                    emit gdbUserCommandStdout(s.message.ascii());
                else
                    emit gdbInternalCommandStdout(s.message.ascii());

                if (currentCmd_)
                    currentCmd_->newOutput(s.message);

                parseCliLine(s.message);


                static QRegExp print_output("^\\$(\\d+) = ");
                if (print_output.search(s.message) != -1)
                {
                    kdDebug(9012) << "Found 'print' output: " << s.message << "\n";
                    print_command_result = s.message.ascii(); 
                }

                break;
            }
            

            case GDBMI::Record::Prompt: 
                break;
            }
        }
        catch(const std::exception& e)
        {
            KMessageBox::detailedSorry(
                0, 
                i18n("<b>Internal debugger error</b>",
                     "<p>The debugger component encountered internal error while "
                     "processing reply from gdb. Please submit a bug report."),
                i18n("The exception is: %1\n"
                     "The MI response is: %2").arg(e.what()).arg(reply),
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
        kdDebug(9012) << "COMMANDS: " << cmdList_.count() << " in queue, "
                      << int(bool(currentCmd_)) << " executing\n";

    commandDone(); 
}

void GDBController::commandDone()
{
    bool no_more_commands = (cmdList_.isEmpty() && !currentCmd_);

    if (no_more_commands && state_reload_needed)
    {
        kdDebug(9012) << "Finishing program stop\n";
        // Set to false right now, so that if 'actOnProgramPauseMI_part2'
        // sends some commands, we won't call it again when handling replies
        // from that commands.
        state_reload_needed = false;
        reloadProgramState();
    }

    if (no_more_commands)
    {
        kdDebug(9012) << "No more commands\n";
        setStateOff(s_dbgBusy);
        emit dbgStatus("", state_);
        raiseEvent(debugger_ready);
    }
}

void GDBController::destroyCurrentCommand()
{
    stateReloadingCommands_.erase(currentCmd_);
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
        if (stateReloadingCommands_.count(cmd));
        {
            kdDebug(9012) << "UNQUEUE: " << cmd->rawDbgCommand() << "\n";
            delete cmdList_.take(i);
        }
    }

    if (stateReloadingCommands_.count(currentCmd_))
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
        kdDebug(9012) << "State reload in progress\n";
    }

    emit event(e);

    if (e == program_state_changed)
    {
        stateReloadInProgress_ = false;
    }
}


void GDBController::slotDbgStderr(KProcess *proc, char *buf, int buflen)
{
    // At the moment, just drop a message out and redirect
    kdDebug(9012) << "STDERR: " << QString::fromLatin1(buf, buflen+1) << endl;
    slotDbgStdout(proc, buf, buflen);
}

// **************************************************************************

void GDBController::slotDbgWroteStdin(KProcess *)
{
    commandExecutionTime.start();

    setStateOff(s_waitForWrite);

    // FIXME: need to remove s_waitForWrite flag completely.
    executeCmd();
}

// **************************************************************************

void GDBController::slotDbgProcessExited(KProcess* process)
{
    Q_ASSERT(process == dbgProcess_);
    bool abnormal = !process->normalExit();
    delete dbgProcess_;
    dbgProcess_ = 0;
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

// **************************************************************************

void GDBController::slotUserGDBCmd(const QString& cmd)
{
    queueCmd(new GDBCommand(cmd.latin1(), INFOCMD, USERCMD));

    // User command can theoreticall modify absolutely everything,
    // so need to force a reload. 
    
    // We can do it right now, and don't wait for user command to finish
    // since commands used to reload all view will be executed after
    // user command anyway.
    if (!stateIsOn(s_appNotStarted) && !stateIsOn(s_programExited))
        raiseEvent(program_state_changed);
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
            arg(currentCmd_->rawDbgCommand());
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
    if (i == name) { out += #name; found = true; } else
                STATE_CHECK(s_dbgNotStarted);
                STATE_CHECK(s_appNotStarted);
                STATE_CHECK(s_waitForWrite);
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
        kdDebug(9012) << out << "\n";
    }
}

int GDBController::qtVersion( ) const
{
  return DomUtil::readIntEntry( dom, "/kdevcppsupport/qt/version", 3 );
}

}

// **************************************************************************
// **************************************************************************
// **************************************************************************
#include "gdbcontroller.moc"
