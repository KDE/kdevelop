/***************************************************************************
                          jdbcontroller.h  -  description
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

#ifndef _JDBCONTROLLER_H_
#define _JDBCONTROLLER_H_

#include "dbgcontroller.h"
#include <qobject.h>
#include <q3ptrlist.h>
#include <q3dict.h>
#include <qstringlist.h>
//Added by qt3to4:
#include <Q3CString>


class KProcess;
class QString;

namespace JAVADebugger
{

class Breakpoint;
class DbgCommand;
class FramestackWidget;
class VarItem;
class VariableTree;
class STTY;

class JDBVarItem : QObject {
    Q_OBJECT
public:
    JDBVarItem();
    QString toString();

    QString value;
    QString name;
    Q3PtrList<JDBVarItem> siblings;

};


/**
 * A front end implementation to the jdb command line debugger
 * @author jbb
 */

class JDBController : public DbgController
{
    Q_OBJECT

public:
    JDBController(VariableTree *varTree, FramestackWidget *frameStack, QString projectDirectory, QString mainProgram);
    ~JDBController();
    void reConfig();

protected:
    void queueCmd(DbgCommand *cmd, bool executeNext=false);

private:
    QString getFile(QString className);
    void varUpdateDone();
    void parseBacktraceList   (char *buf);
    void parseLocals          (char *buf);
    void analyzeDump          (QString data);
    char* parseLine           (char *buf);
    char* parseInfo           (char *buf);
    char* parseBacktrace      (char *buf);
    char* parseLocalVars      (char *buf);
    char* parseDump           (char *buf);
    void parseLocals();
    void parseFrameSelected   (char *buf);

    char *parse               (char *buf);

    void pauseApp();
    void executeCmd ();
    void destroyCmds();
    void removeInfoRequests();
    void actOnProgramPause(const QString &msg);
    void programNoApp(const QString &msg, bool msgBox);

    void setBreakpoint(const Q3CString &BPSetCmd, int key);
    void clearBreakpoint(const Q3CString &BPClearCmd);
    void modifyBreakpoint(Breakpoint *BP);

    void setStateOn(int stateOn)    { state_ |= stateOn; }
    void setStateOff(int stateOff)  { state_ &= ~stateOff; }
    bool stateIsOn(int state)       { return state_  &state; }

    QString classpath_;
    QString mainclass_;
    QString projectDirectory_;
    QString curLine;
    QString curMethod, locals;
    Q3Dict<JDBVarItem> localData;
    QStringList nameQueue;
    int s_command;
    int stackLineCount, varLineCount;
    bool parsedThis;
public slots:
    void slotStart(const QString &application, const QString &args, const QString &sDbgShell=QString());
    void slotCoreFile(const QString &coreFile);
    void slotAttachTo(int pid);

    void slotRun();
    void slotRunUntil(const QString &filename, int lineNum);
    void slotStepInto();
    void slotStepOver();
    void slotStepIntoIns();
    void slotStepOverIns();
    void slotStepOutOff();

    void slotBreakInto();
    void slotBPState(Breakpoint *BP);
    void slotClearAllBreakpoints();

    void slotDisassemble(const QString &start, const QString &end);
    void slotMemoryDump(const QString &start, const QString &amount);
    void slotRegisters();
    void slotLibraries();

    void slotExpandItem(VarItem *parent);
    void slotExpandUserItem(VarItem *parent, const Q3CString &userRequest);
    void slotSelectFrame(int frame);
    void slotSetLocalViewState(bool onOff);

protected slots:
    void slotDbgStdout(KProcess *proc, char *buf, int buflen);
    void slotDbgStderr(KProcess *proc, char *buf, int buflen);
    void slotDbgWroteStdin(KProcess *proc);
    void slotDbgProcessExited(KProcess *proc);
    void slotStepInSource(const QString &fileName, int lineNum);
    void slotDbgStatus(const QString &status, int state);
    void slotDebuggerStarted();

private slots:
    void slotAbortTimedEvent();

signals:
    void rawData              (const QString &rawData);
    void showStepInSource     (const QString &fileName, int lineNum, const QString &address);
    void rawJDBBreakpointList (char *buf);
    void rawJDBBreakpointSet  (char *buf, int key);
    void rawJDBDisassemble    (char *buf);
    void rawJDBMemoryDump     (char *buf);
    void rawJDBRegisters      (char *buf);
    void rawJDBLibraries      (char *buf);
    void ttyStdout            (const char *output);
    void ttyStderr            (const char *output);
    void dbgStatus            (const QString &status, int statusFlag);
    void acceptPendingBPs     ();
    void unableToSetBPNow     (int BPNo);
    void debuggerStarted      ();

private:
    FramestackWidget* frameStack_;
    VariableTree*     varTree_;
    int               currentFrame_;

    int               state_;
    int               jdbSizeofBuf_;          // size of the output buffer
    int               jdbOutputLen_;          // amount of data in the output buffer
    char*             jdbOutput_;             // buffer for the output from kprocess

    Q3PtrList<DbgCommand> cmdList_;
    DbgCommand*       currentCmd_;

    STTY*             tty_;
    bool              programHasExited_;

	// Configuration values
    bool    config_breakOnLoadingLibrary_;
    bool    config_forceBPSet_;
    bool    config_displayStaticMembers_;
    bool    config_dbgTerminal_;
    QString config_jdbPath_;
};

}

#endif
