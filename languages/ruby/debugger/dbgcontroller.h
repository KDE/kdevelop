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
  s_dbgNotStarted     = 1,
  s_appNotStarted     = 2,
  s_appBusy           = 4,
  s_waitForWrite      = 8,
  s_programExited     = 16,
  s_silent            = 32,
  s_viewLocals        = 64,
  s_viewBT            = 128,
  s_viewBP            = 256,
  s_attached          = 512,
  s_viewGlobals       = 1024,
  s_waitTimer         = 2048,
  s_shuttingDown      = 4096,
  s_viewThreads       = 8192
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
    //virtual void slotStart(const QString& shell, const QString &application)= 0;

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
    virtual void slotSelectFrame(int frame, int thread, bool needFrames)    = 0;
    virtual void slotSetLocalViewState(bool onOff)                          = 0;

    // for optional additional commands and initialization
    virtual void slotVarItemConstructed(VarItem */*item*/) {}

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
