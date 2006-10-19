/***************************************************************************
                          gdbcontroller.h  -  description
                             -------------------
    begin                : Sun Aug 8 1999
    copyright            : (C) 1999 by John Birch
    email                : jbb@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _GDBCONTROLLER_H_
#define _GDBCONTROLLER_H_

#include "dbgcontroller.h"
#include "mi/gdbmi.h"
#include "mi/miparser.h"

#include <qcstring.h>
#include <qdom.h>
#include <qobject.h>
#include <qptrlist.h>
#include <qstring.h>
#include <qmap.h>
#include <qdatetime.h>

#include <memory>
#include <set>

class KProcess;

namespace GDBDebugger
{

class Breakpoint;
class DbgCommand;
class GDBCommand;
class VarItem;
class STTY;
class MemoryCallback;
class ValueCallback;

/**
 * A front end implementation to the gdb command line debugger
 * @author jbb
 */

class GDBController : public DbgController
{
    Q_OBJECT

public:
    GDBController(QDomDocument &projectDom);
    ~GDBController();
    
    enum event_t { program_state_changed = 1, program_exited, debugger_exited,
                   thread_or_frame_changed, debugger_busy, debugger_ready,
                   shared_library_loaded,
                   // Raised when debugger believe that program start running.
                   // Can be used to hide current line indicator.
                   // Don't count on this being raise in all cases where
                   // program is running.
                   program_running,
                   connected_to_program
    };


    /** Adds a command to the end of queue of commands to be executed
        by gdb. The command will be actually sent to gdb only when
        replies from all previous commands are received and full processed.

        The literal command sent to gdb is obtained by calling 
        cmd->cmdToSend. The call is made immediately before sending the
        command, so it's possible to use results of prior commands when
        computing the exact command to send.
    */
    void addCommand(GDBCommand* cmd);

    /** Adds command to the front of the commands queue. It will be executed
        next.

        This is usefull to implement 'atomic' command sequences. For example,
        if one wants to switch to each thread in turn, asking gdb where that
        thread stand, this should never be interrupted by other command, since
        other commands might not expect that thread magically changes.

        In this specific case, first -thread-list-ids commands is issued. The
        handler for reply will add a number of command to front, and it will
        guarantee that no other command will get in between.

        Note that if you call:

            addCommandToFront(cmd1);
            addCommandToFront(cmd2);

        The execution order will be 'cmd2', then 'cmd1'.

        FIXME: is not used for now, maybe remove.
    */
    void addCommandToFront(GDBCommand* cmd);

    /** Selects the specified thread/frame. Immediately emits
        thread_or_frame_changed event.
    */
    void selectFrame(int frameNo, int threadNo);


    /** Returns the numerical identfier of the current thread,
        or -1 if the program is not threaded (i.e. there's just
        one thread.
    */
    int currentThread() const;

    int currentFrame() const;

    bool start(const QString& shell, 
               const DomUtil::PairList& run_envvars, 
               const QString& run_directory, 
               const QString &application, 
               const QString& run_arguments);

    int qtVersion() const;

    void pauseApp();

protected:
    void queueCmd(GDBCommand *cmd, bool executeNext=false);

private:
    void parseLocals          (char type, char *buf);
    /** Parses the CLI output line, and catches interesting messages
        like "Program exited". This is intended to allow using console
        commands in the gdb window despite the fact that GDB does not
        produce right MI notification for CLI commands. I.e. if you
        run "continue" there will be no MI message if the application has
        exited.
    */
    void parseCliLine            (const QString&);

    /** Handles a result response from a MI command -- that is
        all MI responses except for Stream and Prompt responses.
        Uses currentCmd to decide what to do with response and
        calls appropriate method.
    */
    void processMICommandResponse(const GDBMI::ResultRecord& r);

    void handleMiFileListExecSourceFile(const GDBMI::ResultRecord& r);
    /** Handles reply from -stack-info-frame command issues
        after switching the stack frame.
    */
    void handleMiFrameSwitch(const GDBMI::ResultRecord& r);

    void executeCmd ();
    void destroyCmds();
    void removeInfoRequests();
    void actOnProgramPauseMI(const GDBMI::ResultRecord& mi_record);
    /** Called when there are no pending commands and 'state_reload_needed'
        is true.
        Issues commands to completely reload all program state shown
        to the user.
    */
    void reloadProgramState();

    void programNoApp(const QString &msg, bool msgBox);

    void setStateOn(int stateOn);
    void setStateOff(int stateOff);
    void setState(int newState);

    void debugStateChange(int oldState, int newState);
    void commandDone();
    void destroyCurrentCommand();

    /** Removes all 'stateReloading' commands from the queue.
     */
    void removeStateReloadingCommands();

    /** Raises the specified event. Should be used instead of
        emitting 'event' directly, since this method can perform
        additional book-keeping for events. 
    */
    void raiseEvent(event_t e);

    void announceWatchpointHit();

    /** Default handler for errors -- shows a dialog box and reloads
        view state.  */
    void defaultErrorHandler(const GDBMI::ResultRecord& result);

    /** Called on errors in commands that execute target -- line next.
        Tries to guess is the error message is telling that target is
        gone, if so, informs the user.  
        If unrecognized error message, calls defaultErrorHandler. */
    void handleExecCommandError(const GDBMI::ResultRecord& result);

public:
    bool stateIsOn(int state);

public slots:
    void configure();


    //void slotStart(const QString& shell, const QString &application);
    void slotCoreFile(const QString &coreFile);
    void slotAttachTo(int pid);

    void slotStopDebugger();

    void slotRun();
    void slotKill();
    void slotRunUntil(const QString &filename, int lineNum);
    void slotJumpTo(const QString &filename, int lineNum);
    void slotStepInto();
    void slotStepOver();
    void slotStepIntoIns();
    void slotStepOverIns();
    void slotStepOutOff();

    void slotBreakInto();

    void slotDisassemble(const QString &start, const QString &end);
    void slotMemoryDump(MemoryCallback* callback,
                        const QString &start, const QString &amount);
    void slotRegisters();
    void slotLibraries();

    void slotUserGDBCmd(const QString&);

    // Pops up a dialog box with some hopefully
    // detailed information about which state debugger
    // is in, which commands were sent and so on.
    void explainDebuggerStatus();


protected slots:
    void slotDbgStdout(KProcess *proc, char *buf, int buflen);
    void slotDbgStderr(KProcess *proc, char *buf, int buflen);
    void slotDbgWroteStdin(KProcess *proc);
    void slotDbgProcessExited(KProcess *proc);

signals:

    /** This signal is emitted whenever the given event in a program
        happens. See DESIGN.txt for expected handled of each event.

        NOTE: this signal should never be emitted directly. Instead,
        use raiseEvent.
    */
    void event(GDBController::event_t e);

    void debuggerAbnormalExit();


    /** Emitted immediately after breakpoint is hit, before any commands
        are sent and before current line indicator is shown. */
    void breakpointHit(int id);
    /** Emitted for watchpoint hit, after line indicator is shown. */
    void watchpointHit(int id,
                       const QString& oldValue, const QString& newValue);

private:
    int               currentFrame_;
    int               viewedThread_;

    // The output from gdb that was not parsed yet
    QCString          gdbOutput_;
    // The output from gdb that arrived where we was
    // parsing the previous output. To avoid messing
    // things up, it's not directly added to
    // gdbOutput_ but stored for future use.
    // VP: It's not clear why the previous code was doing
    // this, and holdingZone_ won't be processed until
    // next output arrives, so probably should be just removed.
    QCString          holdingZone_;

    QPtrList<GDBCommand> cmdList_;
    GDBCommand*       currentCmd_;
    MemoryCallback*   currentMemoryCallback_;

    STTY*             tty_;
    QString           badCore_;
    QString           application_;

    // Gdb command that should be issued when we stop on breakpoint
    // with the given gdb breakpoint id.
    QMap<int, const Breakpoint*> tracedBreakpoints_;

    // Some state variables
    int               state_;
    bool              programHasExited_;

    // Configuration values
    QDomDocument &dom;
    bool    config_breakOnLoadingLibrary_;
    bool    config_forceBPSet_;
    bool    config_displayStaticMembers_;
    bool    config_asmDemangle_;
    bool    config_dbgTerminal_;
    QString config_gdbPath_;
    QString config_dbgShell_;
    QCString config_configGdbScript_;
    QCString config_runShellScript_;
    QCString config_runGdbScript_;
    int config_outputRadix_;

    MIParser mi_parser_;
    // As of gdb 6.3, the *stopped packet does not contain
    // full file name. So we need to send another command to
    // fetch that, to highlight current line.
    // After highting current line we need to do something more,
    // like announcing write watchpoints, and so need to have
    // access to the stop packet. So store it here.
    std::auto_ptr<GDBMI::ResultRecord> last_stop_result;

    // Gdb 6.4 (and 6.3) does not support "character" format with MI,
    // so the only way it can work is via the "print" command. As gdb
    // outputs things, we'll grep for lines that look like output from
    // print, and store such lines in this variable, so later use.
    QCString print_command_result;

    bool state_reload_needed;

    QTime commandExecutionTime;

    bool stateReloadInProgress_;

    /** Commands issues as result of the 'program_state_changed'
        event. */
    std::set<GDBCommand*> stateReloadingCommands_;
};

}

#endif
