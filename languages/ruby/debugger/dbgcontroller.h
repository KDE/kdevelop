/***************************************************************************
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

#ifndef _DBGCONTROLLER_H_
#define _DBGCONTROLLER_H_

#include <qobject.h>
#include <domutil.h>

class KProcess;
class QString;
class QStrList;

namespace RDBDebugger
{

class Breakpoint;
class DbgCommand;
class LazyFetchItem;
class VarItem;

/***************************************************************************/
/**
 * @author jbb
 */
/***************************************************************************/
// sigh - namespace's don't work on some of the older compilers
enum DBGStateFlags
{
  s_dbgNotStarted     = 1 << 0,
  s_appNotStarted     = 1 << 1,
  s_appBusy           = 1 << 2,
  s_waitForWrite      = 1 << 3,
  s_programExited     = 1 << 4,
  s_silent            = 1 << 5,
  s_fetchLocals       = 1 << 6,
  s_viewBT            = 1 << 7,
  s_viewBP            = 1 << 8,
  s_attached          = 1 << 9,
  s_fetchGlobals      = 1 << 10,
  s_waitTimer         = 1 << 11,
  s_shuttingDown      = 1 << 12,
  s_viewThreads       = 1 << 13
};

				
enum RttiValues { 
	RTTI_WATCH_ROOT			= 1001, 
	RTTI_GLOBAL_ROOT		= 1002, 
	RTTI_VAR_FRAME_ROOT		= 1003, 
	RTTI_LAZY_FETCH_ITEM 	= 1004,
	RTTI_VAR_ITEM			= 1005,
	RTTI_WATCH_VAR_ITEM		= 1006,
	RTTI_THREAD_STACK_ITEM	= 1007,
	RTTI_FRAME_STACK_ITEM	= 1008
}; 


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
class DbgController : public QObject
{
    Q_OBJECT

public:

    DbgController();
    virtual ~DbgController();

    virtual bool stateIsOn( int state )                                     = 0;

protected:
    virtual void queueCmd(DbgCommand *cmd, bool executeNext)                = 0;
    virtual void parse(char *str)                                           = 0;

public slots:
    virtual void configure()                                                = 0;

    /**
     * Start the debugger
     * \param ruby_interpreter shell
	 * \param character_coding -K option
     * \param run_directory Directory from where the program should be run
     * \param debuggee_path Absolute path to debuggee.rb debugger script
     * \param application Absolute path to application
     * \param run_arguments Command line arguments to be passed to the application
     */
    virtual void slotStart(const QString& ruby_interpreter,
	                       const QString& character_coding,
                           const QString& run_directory,
						   const QString& debuggee_path,
                           const QString& application,
                           const QString& run_arguments)                    = 0;

    virtual void slotStopDebugger()                                         = 0;

    virtual void slotRun()                                                  = 0;
    virtual void slotRunUntil(const QString &fileName, int lineNum)         = 0;
    virtual void slotStepInto()                                             = 0;
    virtual void slotStepOver()                                             = 0;
    virtual void slotStepOutOff()                                           = 0;

    virtual void slotBreakInto()                                            = 0;
    virtual void slotBPState(const Breakpoint&)                             = 0;


    virtual void slotExpandItem(VarItem *parent,
                                    const QCString &userRequest)            = 0;
    virtual void slotSelectFrame(int frame, int thread, 
	                                const QString& frameName)               = 0;
    virtual void slotFetchGlobals(bool fetch)                               = 0;

protected slots:
    virtual void slotDbgStdout(KProcess *proc, char *buf, int buflen)       = 0;
    virtual void slotDbgStderr(KProcess*, char*, int) {} ;
    virtual void slotDbgWroteStdin(KProcess *proc)                          = 0;
    virtual void slotDbgProcessExited(KProcess *proc)                       = 0;
	
    virtual void slotAcceptConnection(int passive_socket)                   = 0;
    virtual void slotReadFromSocket(int socket)                             = 0;

signals:
    void gotoSourcePosition   (const QString &fileName, int lineNum);
    void rawRDBBreakpointList (char *buf);
    void rawRDBBreakpointSet  (char *buf, int key);
    void ttyStdout            (const char *output);
    void ttyStderr            (const char *output);
    void rdbStdout            (const char *output);
    void rdbStderr            (const char *output);
    void showStepInSource     (const QString &fileName, int lineNum, const QString &address);
    void dbgStatus            (const QString &status, int statusFlag);

protected:
    KProcess *dbgProcess_;
};

}

#endif
