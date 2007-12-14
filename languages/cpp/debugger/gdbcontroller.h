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

#include <memory>

#include <QObject>
#include <QList>
#include <QString>
#include <QMap>
#include <QSet>
#include <QDateTime>

#include <kprocess.h>

#include <irun.h>

#include "mi/gdbmi.h"
#include "mi/miparser.h"

#include "gdbglobal.h"

namespace GDBDebugger
{

class Breakpoint;
class DbgCommand;
class GDBCommand;
class VarItem;
class STTY;
class CommandQueue;
class VariableCollection;
class StackManager;

/**
 * A front end implementation to the gdb command line debugger
 * @author jbb
 */

class GDBController : public QObject
{
    Q_OBJECT

public:
    GDBController(QObject* parent);
    ~GDBController();

    /**
     * Start the debugger, and execute the program specified by \a run, and remember the provided \a serial number.
     */
    bool startProgram(const KDevelop::IRun& run, int serial);

    /**
     * Run currently executing program to the given \a url and \a line.
     */
    void runUntil(const KUrl& url, int line);

    /**
     * Move the execution point of the currently executing program to the given \a url and \a line.
     */
    void jumpTo(const KUrl& url, int line);

    /**
     * Start the debugger and examine the core file given by \a coreFile.
     */
    void examineCoreFile(const KUrl& coreFile);

    /**
     * Attach to currently running process with the given \a pid.
     */
    void attachToProcess(int pid);

    /**
     * Stop the currently running application.
     */
    void stopDebugger();

    /**
     * Return the serial number for the currently run program, if one was given, else return -1.
     */
    int serial() const;

    /** Adds a command to the end of queue of commands to be executed
        by gdb. The command will be actually sent to gdb only when
        replies from all previous commands are received and full processed.

        The literal command sent to gdb is obtained by calling
        cmd->cmdToSend. The call is made immediately before sending the
        command, so it's possible to use results of prior commands when
        computing the exact command to send.
    */
    void addCommand(GDBCommand* cmd);

    /** Same as above, but internally constructs new GDBCommand
       instance from the string. */
    void addCommand(GDBMI::CommandType type, const QString& cmd);

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

    /* If current command queue has any command
       for which isRun is true, inserts 'cmd'
       before the first such command. Otherwise,
       works the same as addCommand. */
    void addCommandBeforeRun(GDBCommand* cmd);

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

    int qtVersion() const;

    bool stateIsOn(DBGStateFlags state);

    /**
     * Return the shared variable collection cache
     */
    VariableCollection* variables() const;

    /**
     * Return the shared frame stack model
     */
    StackManager* stackManager() const;

    using QObject::event;

public Q_SLOTS:
    void slotPauseApp();

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

    void setStateOn(DBGStateFlags stateOn);
    void setStateOff(DBGStateFlags stateOff);
    void setState(DBGStateFlags newState);

    void debugStateChange(DBGStateFlags oldState, DBGStateFlags newState);
    void commandDone();
    void destroyCurrentCommand();

    /** Raises the specified event. Should be used instead of
        emitting 'event' directly, since this method can perform
        additional book-keeping for events.
    */
    void raiseEvent(event_t e);

    void maybeAnnounceWatchpointHit();

    /** Default handler for errors.
        Tries to guess is the error message is telling that target is
        gone, if so, informs the user.
        Otherwise, shows a dialog box and reloads view state.  */
    void defaultErrorHandler(const GDBMI::ResultRecord& result);

    bool startDebugger();

private Q_SLOTS:
    // All of these slots are entered in the controller's thread, as they use queued connections or are called internally
    void queueCmd(GDBCommand *cmd, QueuePosition queue_where = QueueAtEnd);

    void configure();

    void slotRun();
    void slotKill();
    void slotRestart();
    void slotStepInto();
    void slotStepOver();
    void slotStepIntoInstruction();
    void slotStepOverInstruction();
    void slotStepOut();

    void slotUserGDBCmd(const QString&);

    // Pops up a dialog box with some hopefully
    // detailed information about which state debugger
    // is in, which commands were sent and so on.
    void explainDebuggerStatus();

    void readyReadStandardOutput();
    void readyReadStandardError();
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void processErrored(QProcess::ProcessError);

    void slotKillGdb();

Q_SIGNALS:
    void gotoSourcePosition   (const QString &fileName, int lineNum);
    void rawGDBMemoryDump     (char *buf);
    void rawGDBRegisters      (char *buf);
    void rawGDBLibraries      (char *buf);
    void ttyStdout            (const QByteArray& output);
    void ttyStderr            (const QByteArray& output);
    void gdbInternalCommandStdout (const QString& output);
    void gdbUserCommandStdout (const QString& output);
    void showStepInSource     (const QString &fileName, int lineNum, const QString &address);

    /** This signal is emitted whenever the given event in a program
        happens. See DESIGN.txt for expected handled of each event.

        NOTE: this signal should never be emitted directly. Instead,
        use raiseEvent.
    */
    void event(event_t e);

    void debuggerAbnormalExit();


    /** Emitted immediately after breakpoint is hit, before any commands
        are sent and before current line indicator is shown. */
    void breakpointHit(int id);
    /** Emitted for watchpoint hit, after line indicator is shown. */
    void watchpointHit(int id,
                       const QString& oldValue, const QString& newValue);

    void showMessage(const QString& message, int timeout);
    void stateChanged(DBGStateFlags oldState, DBGStateFlags newState);

private:
    void readFromProcess(QProcess* process);

    // This is the frame designated as "current" by currentFrame() and selectFrame()
    int               viewedFrame_;
    // This is the frame which is actually current in gdb
    int               currentFrame_;
    // This is the thread designated as "current" by currentThread() and selectFrame()
    int               viewedThread_;
    // This is the thread which is actually current in gdb
    int               currentThread_;

    // The output from gdb that arrived where we was
    // parsing the previous output.
    // VP: It's not clear why the previous code was doing
    // this, and holdingZone_ won't be processed until
    // next output arrives, so probably should be just removed.
    QByteArray          holdingZone_;

    CommandQueue*   commandQueue_;
    GDBCommand*       currentCmd_;

    STTY*             tty_;
    QString           badCore_;

    // Gdb command that should be issued when we stop on breakpoint
    // with the given gdb breakpoint id.
    QMap<int, const Breakpoint*> tracedBreakpoints_;

    // Some state variables
    DBGStateFlags     state_;
    bool              programHasExited_;

    // Configuration values
    bool    config_breakOnLoadingLibrary_;
    bool    config_forceBPSet_;
    bool    config_displayStaticMembers_;
    bool    config_asmDemangle_;
    bool    config_dbgTerminal_;
    KUrl config_gdbPath_;
    KUrl config_dbgShell_;
    KUrl config_configGdbScript_;
    KUrl config_runShellScript_;
    KUrl config_runGdbScript_;
    int config_outputRadix_;

    MIParser mi_parser_;

    bool state_reload_needed;

    QTime commandExecutionTime;

    bool stateReloadInProgress_;

    /** Commands issues as result of the 'program_state_changed'
        event. */
    QSet<GDBCommand*> stateReloadingCommands_;

    bool saw_gdb_prompt_;

    KProcess* m_process;

    VariableCollection* m_variableCollection;
    StackManager* m_stackManager;
};

}

#endif
