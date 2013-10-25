/*
 * GDB Debugger Support
 *
 * Copyright 1999-2001 John Birch <jbb@kdevelop.org>
 * Copyright 2001 by Bernd Gehrmann <bernd@kdevelop.org>
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2009 Niko Sams <niko.sams@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#ifndef GDB_DEBUGSESSION_H
#define GDB_DEBUGSESSION_H

#include <QWeakPointer>
#include <QScopedPointer>

#include <KConfigGroup>

#include <debugger/interfaces/idebugsession.h>

#include "gdbglobal.h"
#include "mi/gdbmi.h"

class IExecutePlugin;
class KToolBar;

namespace KTextEditor {
class Cursor;
}
namespace KDevelop {
class ProcessLineMaker;
class ILaunchConfiguration;
}

namespace GDBDebugger {

class STTY;
class CommandQueue;
class GDBCommand;
class GDB;
class BreakpointController;


static QString gdbPathEntry = "GDB Path";
static QString debuggerShellEntry = "Debugger Shell";
static QString remoteGdbConfigEntry = "Remote GDB Config Script";
static QString remoteGdbShellEntry = "Remote GDB Shell Script";
static QString remoteGdbRunEntry = "Remote GDB Run Script";
static QString staticMembersEntry = "Display Static Members";
static QString demangleNamesEntry = "Display Demangle Names";
//static QString separateTerminalEntry = "Separate Terminal For Application IO";
static QString allowForcedBPEntry = "Allow Forced Breakpoint Set";
static QString startWithEntry = "Start With";

class DebugSession : public KDevelop::IDebugSession
{
    Q_OBJECT
public:
    DebugSession();
    ~DebugSession();

    virtual DebuggerState state() const;

    virtual bool restartAvaliable() const;

Q_SIGNALS:
    void applicationStandardOutputLines(const QStringList& lines);
    void applicationStandardErrorLines(const QStringList& lines);
    void showMessage(const QString& message, int timeout);
    void reset();
    void programStopped(const GDBMI::ResultRecord& mi_record);

public Q_SLOTS:
    /**
     * Start the debugger, and execute the program specified by \a run.
     */
    bool startProgram(KDevelop::ILaunchConfiguration* run, IExecutePlugin* execute);
    virtual void restartDebugger();
    virtual void stopDebugger();
    virtual void interruptDebugger();
    virtual void run();
    virtual void runToCursor();
    virtual void jumpToCursor();
    virtual void stepOver();
    virtual void stepIntoInstruction();
    virtual void stepInto();
    virtual void stepOverInstruction();
    virtual void stepOut();

    /**
     * Start the debugger and examine the core file given by \a coreFile.
     */
    void examineCoreFile(const KUrl& debugee, const KUrl& coreFile);

    /**
     * Attach to currently running process with the given \a pid.
     */
    void attachToProcess(int pid);

Q_SIGNALS:
    void raiseGdbConsoleViews();

private Q_SLOTS:
    void slotDebuggerAbnormalExit();

private:
    void _gdbStateChanged(DBGStateFlags oldState, DBGStateFlags newState);
   
    KDevelop::IFrameStackModel* createFrameStackModel();
    void setupController();
    void setSessionState(KDevelop::IDebugSession::DebuggerState state);

    KDevelop::ProcessLineMaker *m_procLineMaker;
    KDevelop::ProcessLineMaker *m_gdbLineMaker;
    DebuggerState m_sessionState;
    bool justRestarted_;
    KConfigGroup m_config;
    QWeakPointer<GDB> m_gdb;




public:
    /**
     * Run currently executing program to the given \a url and \a line.
     */
    void runUntil(const KUrl& url, int line);

    /**
     * Run currently executing program to the given \a address
     */
    void runUntil(QString& address);
    /**
     * Move the execution point of the currently executing program to the given \a url and \a line.
     */
    void jumpTo(const KUrl& url, int line);

    /**
     * Move the execution point of the currently executing program to the given \a address.
     *Note: It can be really very dangerous, so use jumpTo instead.
     */
    void jumpToMemoryAddress(QString& address);

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
    void addCommand(GDBMI::CommandType type, const QString& cmd = QString());

    /** Adds command to the front of the commands queue. It will be executed
        next.

        This is useful to implement 'atomic' command sequences. For example,
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

    bool stateIsOn(DBGStateFlags state) const;
    DBGStateFlags debuggerState() const;

    using QObject::event;

private:

    /** Handles a result response from a MI command -- that is
        all MI responses except for Stream and Prompt responses.
        Uses currentCmd to decide what to do with response and
        calls appropriate method.
    */
    void processMICommandResponse(const GDBMI::ResultRecord& r);

    /** Try to execute next command in the queue.  If GDB is not
        busy with previous command, and there's a command in the
        queue, sends it and returns true.
        Otherwise, returns false.  */
    bool executeCmd ();
    void destroyCmds();
    void removeInfoRequests();
    /** Called when there are no pending commands and 'state_reload_needed'
        is true. Also can be used to immediately reload program state.
        Issues commands to completely reload all program state shown
        to the user.
    */
    void reloadProgramState();

    void programNoApp(const QString &msg);
    void programFinished(const QString &msg);

    void setStateOn(DBGStateFlags stateOn);
    void setStateOff(DBGStateFlags stateOff);
    void setState(DBGStateFlags newState);

    void debugStateChange(DBGStateFlags oldState, DBGStateFlags newState);
    void commandDone();

    void raiseEvent(event_t e);

    void maybeAnnounceWatchpointHit();

    bool startDebugger(KDevelop::ILaunchConfiguration* cfg);

    ///Checks if exited inferior is the last one, if so ends the debug session.
    void lastInferiorHandler(const QStringList& l);

private Q_SLOTS:

    void gdbReady();

    void gdbExited();

    void slotProgramStopped(const GDBMI::ResultRecord& mi_record);

    /** Parses the CLI output line, and catches interesting messages
        like "Program exited". This is intended to allow using console
        commands in the gdb window despite the fact that GDB does not
        produce right MI notification for CLI commands. I.e. if you
        run "continue" there will be no MI message if the application has
        exited.
    */
    void parseStreamRecord(const GDBMI::StreamRecord& s);

    /** Default handler for errors.
        Tries to guess is the error message is telling that target is
        gone, if so, informs the user.
        Otherwise, shows a dialog box and reloads view state.  */
    void defaultErrorHandler(const GDBMI::ResultRecord& result);

    /**Triggered every time program begins/continues it's execution.*/
    void programRunning();

    // All of these slots are entered in the controller's thread, as they use queued connections or are called internally
    void queueCmd(GDBCommand *cmd, QueuePosition queue_where = QueueAtEnd);

    void configure();

    void slotUserGDBCmd(const QString&);

    // Pops up a dialog box with some hopefully
    // detailed information about which state debugger
    // is in, which commands were sent and so on.
    void explainDebuggerStatus();

    void slotKillGdb();

    void handleVersion(const QStringList& s);
    void handleFileExecAndSymbols(const GDBMI::ResultRecord& r);
    void handleTargetAttach(const GDBMI::ResultRecord& r);

public Q_SLOTS:
    void slotKill();

Q_SIGNALS:
    void rawGDBMemoryDump     (char *buf);
    void rawGDBRegisters      (char *buf);
    void rawGDBLibraries      (char *buf);
    void ttyStdout            (const QByteArray& output);
    void ttyStderr            (const QByteArray& output);
    void gdbInternalCommandStdout (const QString& output);
    void gdbUserCommandStdout (const QString& output);
    void gdbStateChanged(DBGStateFlags oldState, DBGStateFlags newState);

    void debuggerAbnormalExit();


    /** Emitted immediately after breakpoint is hit, before any commands
        are sent and before current line indicator is shown. */
    void breakpointHit(int id);
    /** Emitted for watchpoint hit, after line indicator is shown. */
    void watchpointHit(int id,
                       const QString& oldValue, const QString& newValue);

private:
    friend class GdbTest;

    CommandQueue*   commandQueue_;

    QScopedPointer<STTY> m_tty;
    QString           badCore_;

    // Some state variables
    DBGStateFlags     state_;

    /**When program stops and all commands from queue are executed and this variable is true, program state shown to the user is updated.*/
    bool state_reload_needed;

    QTime commandExecutionTime;

    /**True if program has stopped and all stuff like breakpoints is being updated.*/
    bool stateReloadInProgress_;

    ///Exit code of the last inferior(in format: exit normally, with code "number" e.t.c)
    QString m_inferiorExitCode;
};

}

#endif
