// *************************************************************************
//                          gdbcontroller.cpp  -  description
//                             -------------------
//    begin                : Sun Aug 8 1999
//    copyright            : (C) 1999 by John Birch
//    email                : jb.nz@writeme.com
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
#include "framestackwidget.h"
#include "gdbcommand.h"
#include "stty.h"
#include "variablewidget.h"
#include "domutil.h"

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>

#include <qregexp.h>
#include <qstring.h>
#include <qtimer.h>

#include <iostream>
#include <ctype.h>
#include <stdlib.h>
using namespace std;

#ifndef _NDEBUG
#define DBG_MONITOR
#endif

#if defined(DBG_MONITOR)
  #define GDB_MONITOR
  #define DBG_DISPLAY(X)          {kdDebug(9012) << QString(X) << endl;}
#else
  #define DBG_DISPLAY(X)          {;}
#endif

#if defined(GDB_MONITOR)
  #define GDB_DISPLAY(X)          {emit rawData(X);}
#else
  #define GDB_DISPLAY(X)          {;}
#endif

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
//                                              "echo \32%c\ninfo locals\necho \32%c\n"
//                                              "end",
//                                              LOCALS, LOCALS)));
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


GDBController::GDBController(VariableTree *varTree, FramestackWidget *frameStack, QDomDocument &projectDom)
    : DbgController(),
      frameStack_(frameStack),
      varTree_(varTree),
      currentFrame_(0),
      state_(s_dbgNotStarted|s_appNotStarted|s_silent),
      gdbSizeofBuf_(2048),
      gdbOutputLen_(0),
      gdbOutput_(new char[2048]),
      currentCmd_(0),
      tty_(0),
      programHasExited_(false),
      badCore_(QString()),
      dom(projectDom),
      config_breakOnLoadingLibrary_(true),
      config_forceBPSet_(true),
      config_displayStaticMembers_(false),
      config_asmDemangle_(true),
      config_dbgTerminal_(false),
      config_gdbPath_(),
      config_dbgShell_(),
      config_programArgs_()
{
    config_displayStaticMembers_  = DomUtil::readEntry(projectDom, "/kdevdebugger/display/staticmembers");
    config_asmDemangle_           = !DomUtil::readEntry(projectDom, "/kdevdebugger/display/manglednames");
    config_breakOnLoadingLibrary_ = DomUtil::readEntry(projectDom, "/kdevdebugger/general/breakonloadinglibs");
    config_forceBPSet_            = DomUtil::readEntry(projectDom, "/kdevdebugger/general/allowforcedbpset");
    config_dbgTerminal_           = DomUtil::readEntry(projectDom, "/kdevdebugger/general/separatetty");
    config_gdbPath_               = DomUtil::readEntry(projectDom, "/kdevdebugger/general/gdbpath");
    config_dbgShell_              = DomUtil::readEntry(projectDom, "/kdevdebugger/general/dbgshell");
    config_programArgs_           = DomUtil::readEntry(projectDom, "/kdevdebugger/general/programargs");
    
#if defined (GDB_MONITOR)
    connect(  this,   SIGNAL(dbgStatus(const QString&, int)),
              SLOT(slotDbgStatus(const QString&, int)));
#endif
#if defined (DBG_MONITOR)
    connect(  this,   SIGNAL(showStepInSource(const QString&, int, const QString&)),
              SLOT(slotStepInSource(const QString&,int)));
#endif
    
    cmdList_.setAutoDelete(true);
}

// **************************************************************************

// Deleting the controller involves shutting down gdb nicely.
// When were attached to a process, we must first detach so that the process
// can continue running as it was before being attached. gdb is quite slow to
// detach from a process, so we must process events within here to get a "clean"
// shutdown.
GDBController::~GDBController()
{
    setStateOn(s_shuttingDown);
    destroyCmds();
    
    if (dbgProcess_) {
        setStateOn(s_silent);
        pauseApp();
        setStateOn(s_waitTimer);
        
        QTimer *timer;
        
        timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(slotAbortTimedEvent()) );
        
        if (stateIsOn(s_attached)) { 
            queueCmd(new GDBCommand("detach", NOTRUNCMD, NOTINFOCMD, DETACH));
            timer->start(3000, TRUE);
            DBG_DISPLAY("<attached wait>\n");
            while (stateIsOn(s_waitTimer)) {
                if (!stateIsOn(s_attached))
                    break;
                kapp->processEvents(20);
            }
        }
        
        setStateOn(s_waitTimer|s_appBusy);
        const char *quit="quit\n";
        dbgProcess_->writeStdin(quit, strlen(quit));
        GDB_DISPLAY(quit)
        timer->start(3000, TRUE);
        DBG_DISPLAY("<quit wait>\n");
        while (stateIsOn(s_waitTimer)) {
            if (stateIsOn(s_programExited))
                break;
            kapp->processEvents(20);
        }
        
        // We cannot wait forever.
        if (stateIsOn(s_shuttingDown))
            dbgProcess_->kill(SIGKILL);
    }
    
    delete tty_; tty_ = 0;
    delete[] gdbOutput_;
    
    emit dbgStatus (i18n("Debugger stopped"), state_);
}

// **************************************************************************

void GDBController::reConfig()
{
    bool old_displayStatic        = config_displayStaticMembers_;
    config_displayStaticMembers_  = DomUtil::readEntry(dom, "/kdevdebugger/display/staticmembers");
    
    bool old_asmDemangle  = config_asmDemangle_;
    config_asmDemangle_   = !DomUtil::readEntry(dom, "/kdevdebugger/display/manglednames");
    
    bool old_breakOnLoadingLibrary_ = config_breakOnLoadingLibrary_;
    config_breakOnLoadingLibrary_ = DomUtil::readEntry(dom, "/kdevdebugger/general/breakonloadinglibs");
    
    if (( old_displayStatic           != config_displayStaticMembers_   ||
          old_asmDemangle             != config_asmDemangle_            ||
          old_breakOnLoadingLibrary_  != config_breakOnLoadingLibrary_ )&&
        dbgProcess_) {
        bool restart = false;
        if (stateIsOn(s_appBusy)) {
            setStateOn(s_silent);
            pauseApp();
            restart = true;
        }
        
        if (old_displayStatic != config_displayStaticMembers_) {
            if (config_displayStaticMembers_)
                queueCmd(new GDBCommand("set print static-members on", NOTRUNCMD, NOTINFOCMD));
            else
                queueCmd(new GDBCommand("set print static-members off", NOTRUNCMD, NOTINFOCMD));
        }
        if (old_asmDemangle != config_asmDemangle_) {
            if (config_asmDemangle_)
                queueCmd(new GDBCommand("set print asm-demangle on", NOTRUNCMD, NOTINFOCMD));
            else
                queueCmd(new GDBCommand("set print asm-demangle off", NOTRUNCMD, NOTINFOCMD));
        }
        
        if (old_breakOnLoadingLibrary_ != config_breakOnLoadingLibrary_) {
            if (config_breakOnLoadingLibrary_)
                queueCmd(new GDBCommand("set stop-on 1", NOTRUNCMD, NOTINFOCMD));
            else
                queueCmd(new GDBCommand("set stop-on 0", NOTRUNCMD, NOTINFOCMD));
        }
        
        if (restart)
            queueCmd(new GDBCommand("continue", RUNCMD, NOTINFOCMD, 0));
    }
}

// **************************************************************************

// Fairly obvious that we'll add whatever command you give me to a queue
// If you tell me to, I'll put it at the head of the queue so it'll run ASAP
// Not quite so obvious though is that if we are going to run again. then any
// information requests become redundent and must be removed.
// We also try and run whatever command happens to be at the head of
// the queue.
void GDBController::queueCmd(DbgCommand *cmd, bool executeNext)
{
    // We remove any info command or _run_ command if we are about to
    // add a run command.
    if (cmd->isARunCmd())
        removeInfoRequests();
    
    if (executeNext)
        cmdList_.insert(0, cmd);
    else
        cmdList_.append (cmd);
    
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
    if (stateIsOn(s_dbgNotStarted|s_waitForWrite|s_appBusy))
        return;
    
    if (!currentCmd_) {
        if (cmdList_.isEmpty())
            return;
        
        currentCmd_ = cmdList_.take(0);
    }
    
    if (!currentCmd_->moreToSend()) {
        if (currentCmd_->expectReply())
            return;
        
        delete currentCmd_;
        if (cmdList_.isEmpty()) {
            currentCmd_ = 0;
            return;
        }
        
        currentCmd_ = cmdList_.take(0);
    }
    
    ASSERT(currentCmd_ && currentCmd_->moreToSend());
    
    dbgProcess_->writeStdin(currentCmd_->cmdToSend().data(), currentCmd_->cmdLength());
    setStateOn(s_waitForWrite);
    
    if (currentCmd_->isARunCmd()) {
        setStateOn(s_appBusy);
        setStateOff(s_appNotStarted|s_programExited|s_silent);
    }
    
    GDB_DISPLAY(currentCmd_->cmdToSend());
    if (!stateIsOn(s_silent))
        emit dbgStatus ("", state_);
}

// **************************************************************************

void GDBController::destroyCmds()
{
    if (currentCmd_) {
        delete currentCmd_;
        currentCmd_ = 0;
    }
    
    while (!cmdList_.isEmpty())
        delete cmdList_.take(0);
}

// **********************************************************************

void GDBController::removeInfoRequests()
{
    int i = cmdList_.count();
    while (i) {
        i--;
        DbgCommand *cmd = cmdList_.at(i);
        if (cmd->isAnInfoCmd() || cmd->isARunCmd())
            delete cmdList_.take(i);
    }
}

// **********************************************************************

// Pausing an app removes any pending run commands so that the app doesn't
// start again. If we want to be silent then we remove any pending info
// commands as well.
void GDBController::pauseApp()
{
    int i = cmdList_.count();
    while (i) {
        i--;
        DbgCommand *cmd = cmdList_.at(i);
        if ((stateIsOn(s_silent) && cmd->isAnInfoCmd()) || cmd->isARunCmd())
            delete cmdList_.take(i);
    }
    
    if (dbgProcess_ && stateIsOn(s_appBusy))
        dbgProcess_->kill(SIGINT);
}

// **********************************************************************

// Whenever the program pauses we need to refresh the data visible to
// the user. The reason we've stooped may be passed in  to be emitted.
void GDBController::actOnProgramPause(const QString &msg)
{
    // We're only stopping if we were running, of course.
    if (stateIsOn(s_appBusy)) {
        DBG_DISPLAY("Acting on program paused");
        setStateOff(s_appBusy);
        if (stateIsOn(s_silent))
            return;
        
        emit dbgStatus (msg, state_);
        
        // We're always at frame zero when the program stops
        // and we must reset the active flag
        currentFrame_ = 0;
        varTree_->setActiveFlag();
        
        // These two need to be actioned immediately. The order _is_ important
        queueCmd(new GDBCommand("backtrace", NOTRUNCMD, INFOCMD, BACKTRACE), true);
        if (stateIsOn(s_viewLocals))
            queueCmd(new GDBCommand("info local", NOTRUNCMD, INFOCMD, LOCALS));
        
        varTree_->findWatch()->requestWatchVars();
        varTree_->findWatch()->setActive();
        emit acceptPendingBPs();
    }
}

// **************************************************************************

// There is no app anymore. This can be caused by program exiting
// an invalid program specified or ...
// gdb is still running though, but only the run command (may) make sense
// all other commands are disabled.
void GDBController::programNoApp(const QString &msg, bool msgBox)
{
    state_ = (s_appNotStarted|s_programExited|(state_&s_viewLocals));
    destroyCmds();
    emit dbgStatus (msg, state_);
    
    // We're always at frame zero when the program stops
    // and we must reset the active flag
    currentFrame_ = 0;
    varTree_->setActiveFlag();
    
    // Now wipe the tree out
    varTree_->viewport()->setUpdatesEnabled(false);
    varTree_->trim();
    varTree_->viewport()->setUpdatesEnabled(true);
    varTree_->repaint();
    
    frameStack_->clear();
    
    if (msgBox)
        KMessageBox::error(0, i18n("gdb message:\n")+msg);
}

// **************************************************************************

// Any data that isn't "wrapped", arrives here.
void GDBController::parseLine(char* buf)
{
  ASSERT(*buf != (char)BLOCK_START);

  // Don't process blank lines
  if (!*buf)
    return;

  if (strncmp(buf, "Prog", 4) == 0)
  {
    if ((strncmp(buf, "Program exited", 14) == 0) ||
        (strncmp(buf, "Program terminated", 18) == 0))
    {
      DBG_DISPLAY("Parsed (exit) <" + QString(buf) + ">");
      programNoApp(QString(buf), false);
      programHasExited_ = true;   // TODO - a nasty switch - this needs fixing
      return;
    }

    if (strncmp(buf, "Program received signal", 23) == 0)
    {
      // SIGINT is a "break into running program".
      // We do this when the user set/mod/clears a breakpoint but the
      // application is running.
      // And the user does this to stop the program for their own
      // nefarious purposes.
      if (strstr(buf+23, "SIGINT") && stateIsOn(s_silent))
        return;

      if (strstr(buf+23, "SIGSEGV"))
      {
        // Oh, shame, shame. The app has died a horrible death
        // Lets remove the pending commands and get the current
        // state organised for the user to figure out what went
        // wrong.
        // Note we're not quite dead yet...
        DBG_DISPLAY("Parsed (SIGSEGV) <" + QString(buf) + ">");
        destroyCmds();
        actOnProgramPause(QString(buf));
        programHasExited_ = true;   // TODO - a nasty switch - this needs fixing
        return;
      }
    }

    // All "Program" strings cause a refresh of the program state
    DBG_DISPLAY("Unparsed (START_Prog)<" + QString(buf) + ">");
    actOnProgramPause(QString(buf));
    return;
  }

  if (strncmp(buf, "Cann", 4) == 0)
  {
    // If you end the app and then restart when you have breakpoints set
    // in a dynamically loaded library, gdb will halt because the set
    // breakpoint is trying to access memory no longer used. The breakpoint
    // must first be deleted, however, we want to retain the breakpoint for
    // when the library gets loaded again.
    // TODO  programHasExited_ isn't always set correctly,
    // but it (almost) doesn't matter.
    if ( strncmp(buf, "Cannot insert breakpoint", 24)==0)
    {
      if (programHasExited_)
      {
        setStateOn(s_silent);
        actOnProgramPause(QString());
        int BPNo = atoi(buf+25);
        if (BPNo)
        {
          emit unableToSetBPNow(BPNo);
          queueCmd(new GDBCommand(QCString().sprintf("delete %d", BPNo), NOTRUNCMD, NOTINFOCMD));
          queueCmd(new GDBCommand("info breakpoints", NOTRUNCMD, NOTINFOCMD, BPLIST));
          queueCmd(new GDBCommand("continue", RUNCMD, NOTINFOCMD, 0));
        }
        DBG_DISPLAY("Parsed (START_cann)<" + QString(buf) + ">");
        return;
      }

      DBG_DISPLAY("Ignore (START_cann)<" + QString(buf) + ">");
//        actOnProgramPause(QString());
      return;
    }

    DBG_DISPLAY("Unparsed (START_cann)<" + QString(buf) + ">");
    actOnProgramPause(QString(buf));
    return;
  }

  if ( strncmp(buf, "[New Thread", 11)==0)
  {
    DBG_DISPLAY("Parsed (START_[New)<ignored><" + QString(buf) + ">");
    return;
  }

  if ( strncmp(buf, "[Switching to Thread", 20)==0)
  {
    DBG_DISPLAY("Parsed (START_[Swi)<ignored><" + QString(buf) + ">");
    return;
  }

  if ( strncmp(buf, "Current language:", 17)==0)
  {
    DBG_DISPLAY("Parsed (START_Curr)<ignored><" + QString(buf) + ">");
    return;
  }

    // When the watchpoint variable goes out of scope the program stops
    // and tells you. (sometimes)
  if (strncmp(buf, "Watc", 4) == 0)
  {
    if ((strncmp(buf, "Watchpoint", 10)==0) &&
        (strstr(buf, "deleted because the program has left the block")))
    {
      int BPNo = atoi(buf+11);
      if (BPNo)
      {
        queueCmd(new GDBCommand(QCString().sprintf("delete %d",BPNo), NOTRUNCMD, NOTINFOCMD));
        queueCmd(new GDBCommand("info breakpoints", NOTRUNCMD, NOTINFOCMD, BPLIST));
      }
      else
        ASSERT(false);

      actOnProgramPause(QString(buf));
      return;
    }
    actOnProgramPause(QString(buf));
    DBG_DISPLAY("Unparsed (START_Watc)<" + QString(buf) + ">");
    return;
  }

  if (strncmp(buf, "Temp", 4) == 0)
  {
    if (strncmp(buf, "Temporarily disabling shared library breakpoints:", 49) == 0)
    {
      DBG_DISPLAY("Parsed (START_Temp)<" + QString(buf) + ">");
      return;
    }

    actOnProgramPause(QString(buf));
    DBG_DISPLAY("Unparsed (START_Temp)<" + QString(buf) + ">");
    return;
  }

  if (strncmp(buf, "Stop", 4) == 0)
  {
    if (strncmp(buf, "Stopped due to shared library event", 35) == 0)
    {
      // When it's a library event, we try and set any pending
      // breakpoints, and that done, just continue onwards.
      // HOWEVER, this only applies when we did a "run" or a
      // "continue" otherwise the program will just keep going
      // on a "step" type command, in this situation and that's
      // REALLY wrong.
//        DBG_DISPLAY("Parsed (sh.lib) <" + QString(buf) + ">");
      if (currentCmd_ && (currentCmd_->rawDbgCommand() == "run" ||
                          currentCmd_->rawDbgCommand() == "continue"))
      {
        setStateOn(s_silent);     // be quiet, children!!
        setStateOff(s_appBusy);   // and stop that fiddling.
        emit acceptPendingBPs();  // now go clean your rooms!
        queueCmd(new GDBCommand("continue", RUNCMD, NOTINFOCMD, 0));
      }
      else
        actOnProgramPause(QString(buf));

      return;
    }

    // A stop line means we've stopped. We're not really expecting one
    // of these unless it's a library event so just call actOnPause
    actOnProgramPause(QString(buf));
    DBG_DISPLAY("Unparsed (START_Stop)<" + QString(buf) + ">");
    return;
  }

  if (strncmp(buf, "Brea", 4) == 0)
  {
    // Starts with "Brea" so assume "Breakpoint" and just get a full
    // breakpoint list. Note that the state is unchanged.
    // Much later: I forget why I did it like this :-o
    queueCmd(new GDBCommand("info breakpoints", NOTRUNCMD, NOTINFOCMD, BPLIST));

    DBG_DISPLAY("Parsed (BP) <" + QString(buf) + ">");
    return;
  }

  if (strncmp(buf, "No s", 4) == 0 ||      // "No symbols loaded"
      strncmp(buf, "Sing", 4) == 0)        // Single stepping
  {
    // We don't change state, because this falls out when a run command starts
    // rather than when a run command stops.
    // We do let the user know what is happening though.
    emit dbgStatus (QString(buf), state_);
    return;
  }

  if (strncmp(buf, "warn", 4) == 0)
  {
    if (strncmp(buf, "warning: core file may not match", 32) == 0 ||
        strncmp(buf, "warning: exec file is newer", 27) == 0)
    {
      badCore_ = QString(buf);
    }
    actOnProgramPause(QString());
    return;
  }

  if (strncmp(buf, "Core", 4) == 0)
  {
    DBG_DISPLAY("Parsed (Core)<" + QString(buf) + ">");
    actOnProgramPause(buf);
    if (!badCore_.isEmpty() && strncmp(buf, "Core was generated by", 21) == 0)
      KMessageBox::error( 0,
                        i18n("gdb message:\n")+badCore_ + "\n" + QString(buf)+"\n\n"+
                              i18n("Any symbols gdb resolves are suspect"),
                        i18n("Mismatched core file"));

    return;
  }

  // The first "step into" into a source file that is missing
  // prints on stderr with a message that there's no source. Subsequent
  // "step into"s just print line number at filename. Both start with a
  // numeric char.
  // Also a 0x message arrives everytime the program stops
  // In the case where there is no source available and you were
  // then this message should appear. Otherwise a program location
  // message will arrive immediately after this and overwrite it.
  if (isdigit(*buf))
  {
    DBG_DISPLAY("Parsed (digit)<" + QString(buf) + ">");
    parseProgramLocation(buf);
//    actOnProgramPause(QString(buf));
    return;
  }

  // TODO - Only do this at start up
  if (//strncmp(buf, "No executable file specified.", 29) ==0   ||
      strstr(buf, "not in executable format:")                ||
      strstr(buf, "No such file or directory.")               ||  // does this fall out?
      strstr(buf, i18n("No such file or directory.").latin1())||  // from system via gdb
      strstr(buf, "is not a core dump:")                      ||
      strncmp(buf, "ptrace: No such process.", 24)==0         ||
      strncmp(buf, "ptrace: Operation not permitted.", 32)==0)
  {
    programNoApp(QString(buf), true);
    DBG_DISPLAY("Bad file <" + QString(buf) + ">");
    return;
  }

  // Any other line that falls out when we are busy is a stop. We
  // might blank a previous message or display this message
  if (stateIsOn(s_appBusy))
  {
    if ((strncmp(buf, "No ", 3)==0) && strstr(buf, "not meaningful"))
    {
      DBG_DISPLAY("Parsed (not meaningful)<" + QString(buf) + ">");
      actOnProgramPause(QString(buf));
      return;
    }

    DBG_DISPLAY("Unparsed (default - busy)<" + QString(buf) + ">");
    actOnProgramPause(QString(" "));
    return;
  }

  // All other lines are ignored
  DBG_DISPLAY("Unparsed (default - not busy)<" + QString(buf) + ">");
  return;
}

// **************************************************************************

// The program location falls out of gdb, preceeded by \032\032. We treat
// it as a wrapped command (even though it doesn't have a trailing \032\032.
// The data gets parsed here and emitted in its component parts.
void GDBController::parseProgramLocation(char *buf)
{
    if (stateIsOn(s_silent)) {
        // It's a silent stop. This means that the queue will have a "continue"
        // in it somewhere. The only action needed is to reset the state so that
        // queue'd items can be sent to gdb
        DBG_DISPLAY("Program location (but silent) <" + QString(buf) + ">");
        setStateOff(s_appBusy);
        return;
    }
    
    //  "/opt/qt/src/widgets/qlistview.cpp:1558:42771:beg:0x401b22f2"
    // This is soooo easy in perl...
    QRegExp regExp1(":[0-9]+:[0-9]+:[a-z]+:0x[abcdef0-9]+$");
    QRegExp regExp2(":0x[abcdef0-9]+$");
    int linePos=0;
    int addressPos=0;
    if (((linePos     = regExp1.match(buf, 0)) >= 0) &&
        ((addressPos  = regExp2.match(buf, 0)) >= 0)) {
        actOnProgramPause(QString(" "));
        emit showStepInSource(QCString(buf, linePos+1),
                              atoi(buf+linePos+1),
                              QString(buf+addressPos+1));
        return;
    }
    
    if (stateIsOn(s_appBusy))
        actOnProgramPause(i18n("No source: %1").arg(QString(buf)));
    else
        emit dbgStatus (i18n("No source: %1").arg(QString(buf)), state_);
    
    QRegExp regExp3("^0x[abcdef0-9]+ ");
    int start;
    if ((start = regExp3.match(buf, 0)) >= 0)
        emit showStepInSource(QString(), -1,
                              QCString(buf, (strchr(buf, ' ')-buf)+1));
    else
        emit showStepInSource("", -1, "");
    
}

// **************************************************************************

// parsing the backtrace list will cause the vartree to be refreshed
void GDBController::parseBacktraceList(char *buf)
{
    frameStack_->parseGDBBacktraceList(buf);
    
    varTree_->viewport()->setUpdatesEnabled(false);
    varTree_->setCurrentThread(viewedThread_);

    VarFrameRoot *varFrame;
    // The locals are always attached to the currentFrame
    // so make sure we have one of those.
    if (!(varFrame = varTree_->findFrame(currentFrame_, viewedThread_)))
        varFrame = new VarFrameRoot(varTree_, currentFrame_, viewedThread_);
    
    ASSERT(varFrame);
    
    // Make a nice descriptive name for this item.
    // This item gets all the variables attached to it.
    QString frameName = frameStack_->getFrameName(currentFrame_, viewedThread_);
    varFrame->setFrameName(frameName);

    // Add the frame params to the variable list
    varFrame->setParams(frameStack_->getFrameParams(currentFrame_, viewedThread_));
    
    if (currentFrame_ == 0 && viewedThread_ != -1)
        varTree_->trimExcessFrames();
    
    varTree_->viewport()->setUpdatesEnabled(true);
    varTree_->repaint();
}

// **************************************************************************

void GDBController::parseThreadList(char *buf)
{
    frameStack_->parseGDBThreadList(buf);
    viewedThread_ = frameStack_->viewedThread();
}

// **************************************************************************

// When a breakpoint has been set, gdb responds with some data about the
// new breakpoint. We just inform the breakpoint system about this.
void GDBController::parseBreakpointSet(char *buf)
{
    if (GDBSetBreakpointCommand *BPCmd = dynamic_cast<GDBSetBreakpointCommand*>(currentCmd_)) {
        // ... except in this case :-) A -1 key tells us that this is
        // a special internal breakpoint, and we shouldn't do anything with it.
        // Currently there are _no_ internal breakpoints.
        if (BPCmd->getKey() != -1)
            emit rawGDBBreakpointSet(buf, BPCmd->getKey());
    }
}

// **************************************************************************

// Extra data needed by an item was requested. Here's the result.
void GDBController::parseRequestedData(char *buf)
{
    if (GDBItemCommand *gdbItemCommand = dynamic_cast<GDBItemCommand*> (currentCmd_)) {
        // Fish out the item from the command and let it deal with the data
        VarItem *item = gdbItemCommand->getItem();
        varTree_->viewport()->setUpdatesEnabled(false);
        item->updateValue(buf);
        item->trim();
        varTree_->viewport()->setUpdatesEnabled(true);
        //    varTree_->repaint();
    }
}

// **************************************************************************

// If the user gives us a bad program, catch that here.
//void GDBController::parseFileStart(char *buf)
//{
//  if (strstr(buf, "not in executable format:") ||
//      strstr(buf, "No such file or directory."))
//  {
//    programNoApp(QString(buf), true);
//    DBG_DISPLAY("Bad file start <" + QString(buf) + ">");
//  }
//}

// **************************************************************************

// Select a different frame to view. We need to get and (maybe) display
// where we are in the program source.
void GDBController::parseFrameSelected(char *buf)
{
    char lookup[3] = {BLOCK_START, SRC_POSITION, 0};
    if (char *start = strstr(buf, lookup)) {
        //    if (char *end = strchr(start, '\n'))  // 21/11/2000 this has already been removed
        //    {
        //      *end = 0;      // clobber the new line
        parseProgramLocation(start+2);
        return;
        //    }
    }
    
    if (!stateIsOn(s_silent))
        {
            //    if (char *end = strchr(buf, '\n'))    // 21/11/2000 this has already been removed
            //      *end = 0;      // clobber the new line
            emit showStepInSource("", -1, "");
            emit dbgStatus (i18n("No source: %1").arg(QString(buf)), state_);
        }
}

// **************************************************************************

// This is called when a completely new set of local data arrives. This data
// is always attached to (and completely updates) the current frame
// _All_ inactive items in the tree are trimmed here.
void GDBController::parseLocals(char *buf)
{
    varTree_->viewport()->setUpdatesEnabled(false);
    
    VarFrameRoot *frame;
    // The locals are always attached to the currentFrame
    // so make sure we have one of those.
    if (!(frame = varTree_->findFrame(currentFrame_, viewedThread_)))
        frame = new VarFrameRoot(varTree_, currentFrame_, viewedThread_);
    
    ASSERT(frame);
    
    frame->setFrameName(frameStack_->getFrameName(currentFrame_, viewedThread_));
    
    // Frame data consists of the parameters of the calling function
    // and the local data.
    frame->setLocals(buf);
    
    // This is tricky - trim the whole tree when we're on the top most
    // frame so that they always see onlu "frame 0" on a program stop.
    // User selects frame 1, will show both frame 0 and frame 1.
    // Reselecting a frame 0 regenerates the data and therefore trims
    // the whole tree _but_ all the items in every frame will be active
    // so nothing will be deleted.
    if (currentFrame_ == 0)
        varTree_->trim();
    else
        frame->trim();
    
    varTree_->viewport()->setUpdatesEnabled(true);
    varTree_->repaint();
}

// **************************************************************************

// We are given a block of data that starts with \032. We now try to find a
// matching end block and if we can we shoot the data of to the appropriate
// parser for that type of data.
char *GDBController::parseCmdBlock(char *buf)
{
    ASSERT(*buf == (char)BLOCK_START);
    
    char *end = 0;
    switch (*(buf+1)) {
    case IDLE:
        // remove the idle tag because they often don't come in pairs
        return buf+1;
        
    case SRC_POSITION:
        // file and line number info that gdb just drops out starts with a
        // \32 but ends with a \n. Could treat this as a line rather than
        // a block. Ah well!
        if((end = strchr(buf, '\n')))
            *end = 0;      // Make a null terminated c-string
        break;
        
    default:
        {
            // match the start block with the end block if we can.
            char lookup[3] = {BLOCK_START, *(buf+1), 0};
            if ((end = strstr(buf+2, lookup))) {
                if (*(end-1) == '\n')
                    *(end-1) = 0;   // fix this by clobbering the new line
                *end = 0;         // Make a null terminated c-string
                end++;            // The real end!
            }
            break;
        }
    }
    
    if (end) {
        char cmdType = *(buf+1);
        buf +=2;
        switch (cmdType) {
        case FRAME:           parseFrameSelected        (buf);      break;
        case SET_BREAKPT:     parseBreakpointSet        (buf);      break;
        case SRC_POSITION:    parseProgramLocation      (buf);      break;
        case LOCALS:          parseLocals               (buf);      break;
        case DATAREQUEST:     parseRequestedData        (buf);      break;
        case BPLIST:          emit rawGDBBreakpointList (buf);      break;
        case BACKTRACE:       parseBacktraceList        (buf);      break;
        case DISASSEMBLE:     emit rawGDBDisassemble    (buf);      break;
        case MEMDUMP:         emit rawGDBMemoryDump     (buf);      break;
        case REGISTERS:       emit rawGDBRegisters      (buf);      break;
        case LIBRARIES:       emit rawGDBLibraries      (buf);      break;
        case DETACH:          setStateOff(s_attached);              break;
            //      case FILE_START:      parseFileStart            (buf);      break;
        default:                                                    break;
        }
        
        // Once we've dealt with the data, we can remove the current command if
        // it is a match for this data.
        if (currentCmd_ && currentCmd_->typeMatch(cmdType)) {
            delete currentCmd_;
            currentCmd_ = 0;
        }
    }
    
    return end;
}

// **************************************************************************

// Deals with data that just falls out of gdb. Basically waits for a line
// terminator to arrive and then gives it to the line parser.
char *GDBController::parseOther(char *buf)
{
    // Could be the start of a block that isn't terminated yet
    ASSERT (*buf != (char)BLOCK_START);
    
    char *end = buf;
    while (*end) {
        if (*end=='(') {   // quick test before a big test
            // This falls out of gdb without a \n terminator. Sometimes
            // a "Stopped due" message will fall out imediately behind this
            // creating a "line". Soemtimes it doesn'y. So we need to check
            // for and remove them first then continue as if it wasn't there.
            // And there can be more that one in a row!!!!!
            // Isn't this bloody awful...
            if (strncmp(end, "(no debugging symbols found)...", 31) == 0) {
                emit dbgStatus (QCString(end, 32), state_);
                return end+30;    // The last char parsed
            }
        }
        
        if (*end=='\n') {
            // Join continuation lines together by removing the '\n'
            if ((end-buf > 2) && (*(end-1) == ' ' && *(end-2) == ',') || (*(end-1) == ':'))
                *end = ' ';
            else {
                *end = 0;        // make a null terminated c-string
                parseLine(buf);
                return end;
            }
        }
        
        // Remove stuff like "junk\32i".
        // This only removes "junk" and leaves "\32i"
        if (*end == (char)BLOCK_START)
            return end-1;
        
        end++;
    }
    
    return 0;
}

// **************************************************************************

char *GDBController::parse(char *buf)
{
    char *unparsed = buf;
    while (*unparsed) {
        char *parsed;
        if (*unparsed == (char)BLOCK_START)
            parsed = parseCmdBlock(unparsed);
        else
            parsed = parseOther(unparsed);
        
        if (!parsed)
            break;
        
        // Move one beyond the end of the parsed data
        unparsed = parsed+1;
    }
    
    return (unparsed==buf) ? 0 : unparsed;
}

// **************************************************************************

void GDBController::setBreakpoint(const QCString &BPSetCmd, int key)
{
    queueCmd(new GDBSetBreakpointCommand(BPSetCmd, key));
}

// **************************************************************************

void GDBController::clearBreakpoint(const QCString &BPClearCmd)
{
    queueCmd(new GDBCommand(BPClearCmd, NOTRUNCMD, NOTINFOCMD));
    // Note: this is NOT an info command, because gdb doesn't explictly tell
    // us that the breakpoint has been deleted, so if we don't have it the
    // BP list doesn't get updated.
    queueCmd(new GDBCommand("info breakpoints", NOTRUNCMD, NOTINFOCMD, BPLIST));
}

// **************************************************************************

void GDBController::modifyBreakpoint( const Breakpoint& BP )
{
    ASSERT(BP.isActionModify());
    if (BP.dbgId()) {
        if (BP.changedCondition())
            queueCmd(new GDBCommand(QCString().sprintf("condition %d %s", BP.dbgId(), BP.conditional().latin1()),
                                    NOTRUNCMD, NOTINFOCMD));
        
        if (BP.changedIgnoreCount())
            queueCmd(new GDBCommand(QCString().sprintf("ignore %d %d", BP.dbgId(), BP.ignoreCount()),
                                    NOTRUNCMD, NOTINFOCMD));
        
        if (BP.changedEnable())
            queueCmd(new GDBCommand(QCString().sprintf("%s %d",
                                                       BP.isEnabled() ? "enable" : "disable", BP.dbgId()), NOTRUNCMD, NOTINFOCMD));
        
//        BP.setDbgProcessing(true);
        // Note: this is NOT an info command, because gdb doesn't explictly tell
        // us that the breakpoint has been deleted, so if we don't have it the
        // BP list doesn't get updated.
        queueCmd(new GDBCommand("info breakpoints", NOTRUNCMD, NOTINFOCMD, BPLIST));
    }
}

// **************************************************************************
//                                SLOTS
//                                *****
// For most of these slots data can only be sent to gdb when it
// isn't busy and it is running.

// **************************************************************************

void GDBController::slotStart(const QString &application)
{
    badCore_ = QString();
    
    ASSERT (!dbgProcess_ && !tty_);
    
    tty_ = new STTY(config_dbgTerminal_, "konsole");
    if (!config_dbgTerminal_) {
        connect( tty_, SIGNAL(OutOutput(const char*)), SIGNAL(ttyStdout(const char*)) );
        connect( tty_, SIGNAL(ErrOutput(const char*)), SIGNAL(ttyStderr(const char*)) );
    }
    
    QString tty(tty_->getSlave());
    if (tty.isEmpty()) {
        KMessageBox::error(0, i18n("gdb cannot use the tty* or pty* devices.\n"
                                   "Check the settings on /dev/tty* and /dev/pty*\n"
                                   "As root you may need to \"chmod ug+rw\" tty* and pty* devices "
                                   "and/or add the user to the tty group using "
                                   "\"usermod -G tty username\""));
        
        delete tty_;
        tty_ = 0;
        return;
    }
    
    GDB_DISPLAY("\nStarting GDB - app:["+application+"] shell:["+config_dbgShell_+"] path:["+config_gdbPath_+"]\n");
    dbgProcess_ = new KProcess;
    
    connect( dbgProcess_, SIGNAL(receivedStdout(KProcess *, char *, int)),
             this,        SLOT(slotDbgStdout(KProcess *, char *, int)) );
    
    connect( dbgProcess_, SIGNAL(receivedStderr(KProcess *, char *, int)),
             this,        SLOT(slotDbgStderr(KProcess *, char *, int)) );
    
    connect( dbgProcess_, SIGNAL(wroteStdin(KProcess *)),
             this,        SLOT(slotDbgWroteStdin(KProcess *)) );
    
    connect( dbgProcess_, SIGNAL(processExited(KProcess*)),
             this,        SLOT(slotDbgProcessExited(KProcess*)) );
    
    if (!config_dbgShell_.isEmpty())
        *dbgProcess_ << "/bin/sh" << "-c"
                     << config_dbgShell_ + " " +config_gdbPath_ + "gdb " + application + " -fullname -nx -quiet";
    else
        *dbgProcess_ << config_gdbPath_ + "gdb" << application << "-fullname" << "-nx" << "-quiet";
    
    dbgProcess_->start( KProcess::NotifyOnExit,
                        KProcess::Communication(KProcess::All));
    
    setStateOff(s_dbgNotStarted);
    emit dbgStatus ("", state_);
    
    // Initialise gdb. At this stage gdb is sitting wondering what to do,
    // and to whom. Organise a few things, then set up the tty for the application,
    // and the application itself
    
    queueCmd(new GDBCommand("set edit off", NOTRUNCMD, NOTINFOCMD, 0));
    queueCmd(new GDBCommand("set confirm off", NOTRUNCMD, NOTINFOCMD));
    
    if (config_displayStaticMembers_)
        queueCmd(new GDBCommand("set print static-members on", NOTRUNCMD, NOTINFOCMD));
    else
        queueCmd(new GDBCommand("set print static-members off", NOTRUNCMD, NOTINFOCMD));
    
    queueCmd(new GDBCommand(QCString("tty ")+tty.latin1(), NOTRUNCMD, NOTINFOCMD));
    
    if (!config_programArgs_.isEmpty())
        queueCmd(new GDBCommand(QCString("set args ") + config_programArgs_.latin1(), NOTRUNCMD, NOTINFOCMD));
    
    // This makes gdb pump a variable out on one line.
    queueCmd(new GDBCommand("set width 0", NOTRUNCMD, NOTINFOCMD));
    queueCmd(new GDBCommand("set height 0", NOTRUNCMD, NOTINFOCMD));
    
    // Get gdb to notify us of shared library events. This allows us to
    // set breakpoints in shared libraries, that the user has set previously.
    // The 1 doesn't mean anything specific, just any non-zero value to
    // satisfy gdb!
    // An alternative to this would be catch load, catch unload, but they don't work!
    if (config_breakOnLoadingLibrary_)
        queueCmd(new GDBCommand("set stop-on 1", NOTRUNCMD, NOTINFOCMD));
    else
        queueCmd(new GDBCommand("set stop-on 0", NOTRUNCMD, NOTINFOCMD));
    
    // Print some nicer names in disassembly output. Although for an assembler
    // person this may actually be wrong and the mangled name could be better.
    if (config_asmDemangle_)
        queueCmd(new GDBCommand("set print asm-demangle on", NOTRUNCMD, NOTINFOCMD));
    else
        queueCmd(new GDBCommand("set print asm-demangle off", NOTRUNCMD, NOTINFOCMD));
    
    // Load the file into gdb
    /*if (sDbgShell.isEmpty())
      {
      QString fileCmd = "file " + application;
      queueCmd(new GDBCommand(fileCmd, NOTRUNCMD, NOTINFOCMD));
      }
    */
    // Organise any breakpoints.
    emit acceptPendingBPs();
    
    // Now gdb has been started and the application has been loaded,
    // BUT the app hasn't been started yet! A run command is about to be issued
    // by whoever is controlling us. Or we might be asked to load a core, or
    // attach to a running process.
}

// **************************************************************************

void GDBController::slotCoreFile(const QString &coreFile)
{
    setStateOff(s_silent);
    queueCmd(new GDBCommand(QCString("core ") + coreFile.latin1(), NOTRUNCMD, NOTINFOCMD, 0));
    queueCmd(new GDBCommand("backtrace", NOTRUNCMD, INFOCMD, BACKTRACE));
    if (stateIsOn(s_viewLocals))
        queueCmd(new GDBCommand("info local", NOTRUNCMD, INFOCMD, LOCALS));
}

// **************************************************************************

void GDBController::slotAttachTo(int pid)
{
    setStateOff(s_appNotStarted|s_programExited|s_silent);
    setStateOn(s_attached);
    queueCmd(new GDBCommand(QCString().sprintf("attach %d", pid), NOTRUNCMD, NOTINFOCMD, 0));
    queueCmd(new GDBCommand("backtrace", NOTRUNCMD, INFOCMD, BACKTRACE));
    if (stateIsOn(s_viewLocals))
        queueCmd(new GDBCommand("info local", NOTRUNCMD, INFOCMD, LOCALS));
}

// **************************************************************************

void GDBController::slotRun()
{
    if (stateIsOn(s_appBusy|s_dbgNotStarted|s_shuttingDown))
        return;
    
    queueCmd(new GDBCommand(stateIsOn(s_appNotStarted) ? "run" : "continue", RUNCMD, NOTINFOCMD, 0));
}

// **************************************************************************

void GDBController::slotRunUntil(const QString &fileName, int lineNum)
{
    if (stateIsOn(s_appBusy|s_dbgNotStarted|s_shuttingDown))
        return;
    
    if (fileName == "")
        queueCmd(new GDBCommand(QCString().sprintf("until %d", lineNum), RUNCMD, NOTINFOCMD, 0));
    else
        queueCmd(new GDBCommand(QCString().sprintf("until %s:%d", fileName.latin1(), lineNum),
                                RUNCMD, NOTINFOCMD, 0));
}

// **************************************************************************

void GDBController::slotStepInto()
{
    if (stateIsOn(s_appBusy|s_appNotStarted|s_shuttingDown))
        return;
    
    queueCmd(new GDBCommand("step", RUNCMD, NOTINFOCMD, 0));
}

// **************************************************************************

void GDBController::slotStepIntoIns()
{
    if (stateIsOn(s_appBusy|s_appNotStarted|s_shuttingDown))
        return;
    
    queueCmd(new GDBCommand("stepi", RUNCMD, NOTINFOCMD, 0));
}

// **************************************************************************

void GDBController::slotStepOver()
{
    if (stateIsOn(s_appBusy|s_appNotStarted|s_shuttingDown))
        return;
    
    queueCmd(new GDBCommand("next", RUNCMD, NOTINFOCMD, 0));
}

// **************************************************************************

void GDBController::slotStepOverIns()
{
    if (stateIsOn(s_appBusy|s_appNotStarted|s_shuttingDown))
        return;
    
    queueCmd(new GDBCommand("nexti", RUNCMD, NOTINFOCMD, 0));
}

// **************************************************************************

void GDBController::slotStepOutOff()
{
    if (stateIsOn(s_appBusy|s_appNotStarted|s_shuttingDown))
        return;
    
    queueCmd(new GDBCommand("finish", RUNCMD, NOTINFOCMD, 0));
}

// **************************************************************************

// Only interrupt a running program.
void GDBController::slotBreakInto()
{
    pauseApp();
}

// **************************************************************************

// See what, if anything needs doing to this breakpoint.
void GDBController::slotBPState( const Breakpoint& BP )
{
    // Are we in a position to do anything to this breakpoint?
    if (stateIsOn(s_dbgNotStarted|s_shuttingDown) || !BP.isPending() || BP.isActionDie())
        return;
    
    // We need this flag so that we can continue execution. I did use
    // the s_silent state flag but it can be set prior to this method being
    // called, hence is invalid.
    bool restart = false;
    if (stateIsOn(s_appBusy)) {
        if (!config_forceBPSet_)
            return;
        
        // When forcing breakpoints to be set/unset, interrupt a running app
        // and change the state.
        setStateOn(s_silent);
        pauseApp();
        restart = true;
    }
    
    if (BP.isActionAdd()) {
        setBreakpoint(BP.dbgSetCommand().latin1(), BP.key());
//        BP.setDbgProcessing(true);
    } else {
        if (BP.isActionClear()) {
            clearBreakpoint(BP.dbgRemoveCommand().latin1());
//            BP.setDbgProcessing(true);
        } else if (BP.isActionModify()) {
            modifyBreakpoint(BP); // Note: DbgProcessing gets set in modify fn
        }
    }
    
    if (restart)
        queueCmd(new GDBCommand("continue", RUNCMD, NOTINFOCMD, 0));
}

// **************************************************************************

void GDBController::slotClearAllBreakpoints()
{
    // Are we in a position to do anything to this breakpoint?
    if (stateIsOn(s_dbgNotStarted|s_shuttingDown))
        return;
    
    bool restart = false;
    if (stateIsOn(s_appBusy)) {
        if (!config_forceBPSet_)
            return;
        
        // When forcing breakpoints to be set/unset, interrupt a running app
        // and change the state.
        setStateOn(s_silent);
        pauseApp();
        restart = true;
    }
    
    queueCmd(new GDBCommand("delete", NOTRUNCMD, NOTINFOCMD));
    // Note: this is NOT an info command, because gdb doesn't explictly tell
    // us that the breakpoint has been deleted, so if we don't have it the
    // BP list doesn't get updated.
    queueCmd(new GDBCommand("info breakpoints", NOTRUNCMD, NOTINFOCMD, BPLIST));
    
    if (restart)
        queueCmd(new GDBCommand("continue", RUNCMD, NOTINFOCMD, 0));
}

// **************************************************************************

void GDBController::slotDisassemble(const QString &start, const QString &end)
{
    if (stateIsOn(s_appBusy|s_dbgNotStarted|s_shuttingDown))
        return;
    
    QCString cmd = QCString().sprintf("disassemble %s %s", start.latin1(), end.latin1());
    queueCmd(new GDBCommand(cmd, NOTRUNCMD, INFOCMD, DISASSEMBLE));
}

// **************************************************************************

void GDBController::slotMemoryDump(const QString &address, const QString &amount)
{
    if (stateIsOn(s_appBusy|s_dbgNotStarted|s_shuttingDown))
        return;
    
    QCString cmd = QCString().sprintf("x/%sb %s", amount.latin1(), address.latin1());
    queueCmd(new GDBCommand(cmd, NOTRUNCMD, INFOCMD, MEMDUMP));
}

// **************************************************************************

void GDBController::slotRegisters()
{
    if (stateIsOn(s_appBusy|s_dbgNotStarted|s_shuttingDown))
        return;
    
    queueCmd(new GDBCommand("info all-registers", NOTRUNCMD, INFOCMD, REGISTERS));
}

// **************************************************************************

void GDBController::slotLibraries()
{
    if (stateIsOn(s_appBusy|s_dbgNotStarted|s_shuttingDown))
        return;
    
    queueCmd(new GDBCommand("info sharedlibrary", NOTRUNCMD, INFOCMD, LIBRARIES));
}

// **************************************************************************

void GDBController::slotSelectFrame(int frameNo, int threadNo, bool needFrames)
{
    if (stateIsOn(s_appBusy|s_dbgNotStarted|s_shuttingDown))
        return;
    
    // Get gdb to switch the frame stack on a frame change.
    // This is an info command because _any_ run command will set the system back
    // to frame 0 regardless, so being removed with a run command is the best
    // thing that could happen here.
    // _Always_ switch frames (even if we're the same frame so that a program
    // position will be generated by gdb
    //  if (frameNo != currentFrame_)
    if (threadNo != -1) {
        // We don't switch threads if we on this thread. The -1 check is because the first time
        // after a stop we're actually on this thread but the thread no had been reset to -1.
        if (viewedThread_ != -1) {
            if (viewedThread_ != threadNo)
                queueCmd(new GDBCommand(QCString().sprintf("thread %d", threadNo), NOTRUNCMD, INFOCMD, FRAME));
            
            if (needFrames)
                queueCmd(new GDBCommand("backtrace", NOTRUNCMD, INFOCMD, BACKTRACE));
            
            if (needFrames || (viewedThread_ != threadNo) || (currentFrame_ != frameNo))
                queueCmd(new GDBCommand(QCString().sprintf("frame %d", frameNo), NOTRUNCMD, INFOCMD, FRAME));
        }
    } else {
        if (currentFrame_ != frameNo)
            queueCmd(new GDBCommand(QCString().sprintf("frame %d", frameNo), NOTRUNCMD, INFOCMD, FRAME));
    }
    
    // Hold on to  this thread/frame so that we know where to put the local variables if generated.
    viewedThread_ = threadNo;
    currentFrame_ = frameNo;
    
    // Find or add the frame details. hold onto whether it existed because we're
    // about to create one if it didn't.
    VarFrameRoot *frame = varTree_->findFrame(frameNo, viewedThread_);
    bool haveFrame = (bool)frame;
    if (!haveFrame)
        frame = new VarFrameRoot(varTree_, currentFrame_, viewedThread_);
    
    ASSERT(frame);
    
    // Make vartree display a pretty frame description
    frame->setFrameName(frameStack_->getFrameName(currentFrame_, viewedThread_));
    
    if (stateIsOn(s_viewLocals)) {
        // Have we already got these details?
        if (frame->needLocals()) {
            // Add the frame params to the variable list
            frame->setParams(frameStack_->getFrameParams(currentFrame_, viewedThread_));
            // and ask for the locals
            queueCmd(new GDBCommand("info local", NOTRUNCMD, INFOCMD, LOCALS));
        }
    }
}

// **************************************************************************

// This is called when the user desires to see the details of an item, by
// clicking open an varItem on the varTree.
void GDBController::slotExpandItem(VarItem *item)
{
    if (stateIsOn(s_appBusy|s_dbgNotStarted|s_shuttingDown))
        return;
    
    switch (item->getDataType()) {
    case typePointer:
        queueCmd(new GDBPointerCommand(item));
        break;
        
    default:
        queueCmd(new GDBItemCommand(item, QCString("print ") + item->fullName().latin1()));
        break;
    }
}

// **************************************************************************

// This is called when an item needs special processing to show a value.
// Example = QStrings. We want to display the QString string against the var name
// so the user doesn't have to open the qstring to find it. Here's where that happens
void GDBController::slotExpandUserItem(VarItem *item, const QCString &userRequest)
{
    if (stateIsOn(s_appBusy|s_dbgNotStarted|s_shuttingDown))
        return;
    
    ASSERT(item);
    
    // Bad user data!!
    if (userRequest.isEmpty())
        return;
    
    queueCmd(new GDBItemCommand(item, QCString("print ")+userRequest.data(), false, DATAREQUEST));
}

// **************************************************************************

// The user will only get locals if one of the branches to the local tree
// is open. This speeds up stepping through code a great deal.
void GDBController::slotSetLocalViewState(bool onOff)
{
    if (onOff)
        setStateOn(s_viewLocals);
    else
        setStateOff(s_viewLocals);
    
    DBG_DISPLAY(onOff ? "<Locals ON>": "<Locals OFF>");
}

// **************************************************************************

// Data from gdb gets processed here.
void GDBController::slotDbgStdout(KProcess *, char *buf, int buflen)
{
#ifdef GDB_MONITOR
    QCString msg(buf, buflen+1);
    msg.replace(QRegExp("\032."),"\n(gdb) ");
    GDB_DISPLAY(msg);
#endif
    
    // Allocate some buffer space, if adding to this buffer will exceed it
    if (gdbOutputLen_+buflen+1 > gdbSizeofBuf_) {
        gdbSizeofBuf_ = gdbOutputLen_+buflen+1;
        char *newBuf = new char[gdbSizeofBuf_];     // ??? shoudn't this be malloc ???
        if (gdbOutputLen_)
            memcpy(newBuf, gdbOutput_, gdbOutputLen_+1);
        delete[] gdbOutput_;                        // ??? and free ???
        gdbOutput_ = newBuf;
    }
    
    // Copy the data out of the KProcess buffer before it gets overwritten
    // and fake a string so we can use the string fns on this buffer
    memcpy(gdbOutput_+gdbOutputLen_, buf, buflen);
    gdbOutputLen_ += buflen;
    *(gdbOutput_+gdbOutputLen_) = 0;
    
    if (char *nowAt = parse(gdbOutput_)) {
        ASSERT(nowAt <= gdbOutput_+gdbOutputLen_+1);
        gdbOutputLen_ = strlen(nowAt);
        // Some bytes that wern't parsed need to be move to the head of the buffer
        if (gdbOutputLen_)
            memmove(gdbOutput_, nowAt, gdbOutputLen_);     // Overlapping data
    }
    
    // check the queue for any commands to send
    executeCmd();
}

// **************************************************************************

void GDBController::slotDbgStderr(KProcess *proc, char *buf, int buflen)
{
    // At the moment, just drop a message out and redirect
    DBG_DISPLAY(QString("\nSTDERR: ")+QString::fromLatin1(buf, buflen+1));
    slotDbgStdout(proc, buf, buflen);
    
    //  QString bufData(buf, buflen+1);
    //  char *found;
    //  if ((found = strstr(buf, "No symbol table is loaded")))
    //    emit dbgStatus (QString("No symbol table is loaded"), state_);
    
    // If you end the app and then restart when you have breakpoints set
    // in a dynamically loaded library, gdb will halt because the set
    // breakpoint is trying to access memory no longer used. The breakpoint
    // must first be deleted, however, we want to retain the breakpoint for
    // when the library gets loaded again.
    // TODO  programHasExited_ isn't always set correctly,
    // but it (almost) doesn't matter.
    //  if (programHasExited_ && (found = strstr(bufData.data(), "Cannot insert breakpoint")))
    //  {
    //    setStateOff(s_appBusy);
    //    int BPNo = atoi(found+25);
    //    if (BPNo)
    
    //    {
    //      queueCmd(new GDBCommand(QString().sprintf("delete %d", BPNo), NOTRUNCMD, NOTINFOCMD));
    //      queueCmd(new GDBCommand("info breakpoints", NOTRUNCMD, NOTINFOCMD, BPLIST));
    //      queueCmd(new GDBCommand("continue", RUNCMD, NOTINFOCMD, 0));
    //      emit unableToSetBPNow(BPNo);
    //    }
    //    return;
    //  }
    //
    //  parse(bufData.data());
}

// **************************************************************************

void GDBController::slotDbgWroteStdin(KProcess *)
{
    setStateOff(s_waitForWrite);
    //  if (!stateIsOn(s_silent))
    //    emit dbgStatus ("", state_);
    executeCmd();
}

// **************************************************************************

void GDBController::slotDbgProcessExited(KProcess*)
{
    destroyCmds();
    state_ = s_appNotStarted|s_programExited|(state_&s_viewLocals);
    emit dbgStatus (i18n("Process exited"), state_);
    
    GDB_DISPLAY(QString("\n(gdb) Process exited"));
}

// **************************************************************************

// The time limit has expired so set the state off.
void GDBController::slotAbortTimedEvent()
{
    setStateOff(s_waitTimer);
    DBG_DISPLAY(QString("Timer aborted\n"));
}

// **************************************************************************

// These are here for debug display. I wanted them to be removed if DBG_MONITOR
// wasn't defined but qt's moc has problems with that.
#if defined(DBG_MONITOR)
void GDBController::slotStepInSource(const QString &fileName, int lineNum)
{
    DBG_DISPLAY((QString("(Show step in source) ")+fileName+QString(":")+QString().setNum(lineNum)).data());
}

// **************************************************************************

void GDBController::slotDbgStatus(const QString &status, int state)
{
    QString s("(status) ");
    if (!state)
        s += QString("<program paused>");
    if (state & s_dbgNotStarted)
        s += QString("<dbg not started>");
    if (state & s_appNotStarted)
        s += QString("<app not started>");
    if (state & s_appBusy)
        s += QString("<app busy>");
    if (state & s_waitForWrite)
        s += QString("<wait for write>");
    if (state & s_programExited)
        s += QString("<program exited>");
    if (state & s_silent)
        s += QString("<silent>");
    if (state & s_viewLocals)
        s += QString("<viewing locals>");
    
    DBG_DISPLAY((s+status).data());
}
#else

void GDBController::slotStepInSource(const QString&, int)
{}

// **************************************************************************

void GDBController::slotDbgStatus(const QString&, int)
{}

#endif

// **************************************************************************
// **************************************************************************
// **************************************************************************
#include "gdbcontroller.moc"
