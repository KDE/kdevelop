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

#include <qcstring.h>
#include <qdom.h>
#include <qobject.h>
#include <qptrlist.h>
#include <qstring.h>

class KProcess;

namespace GDBDebugger
{

class Breakpoint;
class DbgCommand;
class FramestackWidget;
class VarItem;
class VariableTree;
class STTY;

/**
 * A front end implementation to the gdb command line debugger
 * @author jbb
 */

class GDBController : public DbgController
{
    Q_OBJECT

public:
    GDBController(VariableTree *varTree, FramestackWidget *frameStack, QDomDocument &projectDom);
    ~GDBController();

protected:
    void queueCmd(DbgCommand *cmd, bool executeNext=false);

private:
    void parseProgramLocation (char *buf);
    void parseBacktraceList   (char *buf);
    void parseThreadList      (char* buf);
    void parseBreakpointSet   (char *buf);
    void parseLocals          (char type, char *buf);
    void parseRequestedData   (char *buf);
    void parseWhatis          (char *buf);
    void parseLine            (char *buf);
    void parseFrameSelected   (char *buf);
    //  void parseFileStart       (char *buf);

    char *parse               (char *buf);
    char *parseOther          (char *buf);
    char *parseCmdBlock       (char *buf);

    void pauseApp();
    void executeCmd ();
    void destroyCmds();
    void removeInfoRequests();
    void actOnProgramPause(const QString &msg);
    void programNoApp(const QString &msg, bool msgBox);

    void setBreakpoint(const QCString &BPSetCmd, int key);
    void clearBreakpoint(const QCString &BPClearCmd);
    void modifyBreakpoint(const Breakpoint&);

    void setStateOn(int stateOn)    { state_ |= stateOn; }
    void setStateOff(int stateOff)  { state_ &= ~stateOff; }
    bool stateIsOn(int state)       { return state_  &state; }

public slots:
    void configure();

    void slotStart(const QString& shell, const DomUtil::PairList& run_envvars, const QString& run_directory, const QString &application, const QString& run_arguments);
    //void slotStart(const QString& shell, const QString &application);
    void slotCoreFile(const QString &coreFile);
    void slotAttachTo(int pid);

    void slotStopDebugger();

    void slotRun();
    void slotRunUntil(const QString &filename, int lineNum);
    void slotStepInto();
    void slotStepOver();
    void slotStepIntoIns();
    void slotStepOverIns();
    void slotStepOutOff();

    void slotBreakInto();
    void slotBPState( const Breakpoint& );
    void slotClearAllBreakpoints();

    void slotDisassemble(const QString &start, const QString &end);
    void slotMemoryDump(const QString &start, const QString &amount);
    void slotRegisters();
    void slotLibraries();

    void slotExpandItem(TrimmableItem *parent);
    void slotExpandUserItem(VarItem *parent, const QCString &userRequest);
    void slotSelectFrame(int frameNo, int threadNo, bool needFrames);
    void slotSetLocalViewState(bool onOff);

    // jw - type determination requires a var object, so we do it here
    void slotVarItemConstructed(VarItem *item);

    void slotUserGDBCmd(const QString&);

protected slots:
    void slotDbgStdout(KProcess *proc, char *buf, int buflen);
    void slotDbgStderr(KProcess *proc, char *buf, int buflen);
    void slotDbgWroteStdin(KProcess *proc);
    void slotDbgProcessExited(KProcess *proc);

signals:
    void acceptPendingBPs     ();
    void unableToSetBPNow     (int BPNo);
    void debuggerRunError(int errorCode);

private:
    FramestackWidget* frameStack_;
    VariableTree*     varTree_;
    int               currentFrame_;
    int               viewedThread_;

    int               gdbSizeofBuf_;          // size of the output buffer
    int               gdbOutputLen_;          // amount of data in the output buffer
    char*             gdbOutput_;             // buffer for the output from kprocess
    QCString          holdingZone_;

    QPtrList<DbgCommand> cmdList_;
    DbgCommand*       currentCmd_;

    STTY*             tty_;
    QString           badCore_;
    QString           application_;

    // Some state variables
    int               state_;
    bool              programHasExited_;
    bool              backtraceDueToProgramStop_;

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
};

}

#endif
