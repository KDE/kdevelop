/***************************************************************************
                          rdbcontroller.h  -  description
                             -------------------
    begin                : Sun Aug 8 1999
    copyright            : (C) 1999 by John Birch
    email                : jbb@kdevelop.org
	
                          Adapted for ruby debugging
                          --------------------------
	
    begin                : Mon Nov 1 2004
    copyright            : (C) 2004 by Richard Dale
    email                : Richard_Dale@tipitina.demon.co.uk
	
	
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _RDBCONTROLLER_H_
#define _RDBCONTROLLER_H_

#include "dbgcontroller.h"

#include <qcstring.h>
#include <qdom.h>
#include <qobject.h>
#include <qptrlist.h>
#include <qstring.h>
#include <qsocketnotifier.h> 

class KProcess;

namespace RDBDebugger
{

class Breakpoint;
class DbgCommand;
class FramestackWidget;
class VarItem;
class VariableTree;
class STTY;

/**
 * A front end implementation to the ruby command line debugger
 * @author jbb
 */

class RDBController : public DbgController
{
    Q_OBJECT

public:
    RDBController(VariableTree *varTree, FramestackWidget *frameStack, QDomDocument &projectDom);
    ~RDBController();

protected:
    void queueCmd(DbgCommand *cmd, bool executeNext=false);

private:
    void parseProgramLocation (char *buf);
    void parseBacktraceList   (char *buf);
    void parseThreadList      (char* buf);
    void parseSwitchThread    (char* buf);
    void parseBreakpointSet   (char *buf);
    void parseDisplay         (char *buf, char * expr);
    void parseUpdateDisplay   (char *buf);
    void parseGlobals         (char *buf);
    void parseLocals          (char type, char *buf);
    void parseRequestedData   (char *buf);
    void parseFrameSelected   (char *buf);

    void parse                (char *buf);

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

    void slotStart(	const QString& shell, const QString& characterCoding, 
					const QString& run_directory, const QString& debuggee_path, 
					const QString &application, const QString& run_arguments );
    //void slotStart(const QString& shell, const QString &application);

    void slotStopDebugger();

    void slotRun();
    void slotRunUntil(const QString &filename, int lineNum);
    void slotStepInto();
    void slotStepOver();
    void slotStepOutOff();

    void slotBreakInto();
    void slotBPState( const Breakpoint& );
    void slotClearAllBreakpoints();

    void slotExpandItem(VarItem *parent, const QCString &userRequest);
    void slotRubyInspect(const QString &inspectText);
    void slotSelectFrame(int frameNo, int threadNo, const QString& frameName);
    void slotFetchGlobals(bool fetch);
    void slotAddWatchExpression(const QString& expr, bool execute);
	void slotRemoveWatchExpression(int displayId);

    void slotUserRDBCmd(const QString&);

protected slots:
    void slotDbgStdout(KProcess *proc, char *buf, int buflen);
    void slotDbgStderr(KProcess *proc, char *buf, int buflen);
    void slotDbgWroteStdin(KProcess *proc);
    void slotDbgProcessExited(KProcess *proc);
	
    void slotAcceptConnection(int masterSocket);
    void slotReadFromSocket(int socket);

signals:
    void acceptPendingBPs     ();
    void unableToSetBPNow     (int BPNo);
    void addWatchExpression     (const QString&);

private:
    FramestackWidget* frameStack_;
    VariableTree*     varTree_;
    int               currentFrame_;
    int               viewedThread_;

    int               stdoutSizeofBuf_;      // size of the buffer for holding stdout piped
	                                         // from the ruby program
    int               stdoutOutputLen_;      // amount of data in the output buffer
    char*             stdoutOutput_;         // buffer for the output from kprocess
    QCString          holdingZone_;
    
	int               rdbSizeofBuf_;         // size of the output buffer from rdb
    int               rdbOutputLen_;         // amount of data in the rdb buffer
    char*             rdbOutput_;            // buffer for the output from rdb via the Unix socket
	
	int               masterSocket_;         // The socket to accept connections
	QSocketNotifier*  acceptNotifier_;
    static const char * unixSocketPath_;     // The name of the Unix Domain socket
	int               socket_;               // The socket to read and write to the debuggee
	QSocketNotifier*  socketNotifier_;

    QPtrList<DbgCommand> cmdList_;
    DbgCommand*       currentCmd_;
	QString           currentPrompt_;

    STTY*             tty_;
	
	// Details for starting the ruby debugger process
	QString          rubyInterpreter_;
	QString          characterCoding_;
	QString          runDirectory_;
	QString          debuggeePath_;
	QString          application_;
	QString          runArguments_;

    // Some state variables
    int               state_;
    bool              programHasExited_;

    // Configuration values
    QDomDocument &dom;
    bool    config_forceBPSet_;
    bool    config_dbgTerminal_;
};

}

#endif
