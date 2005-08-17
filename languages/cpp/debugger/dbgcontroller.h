/***************************************************************************
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

#ifndef _DBGCONTROLLER_H_
#define _DBGCONTROLLER_H_

#include <qobject.h>
//Added by qt3to4:
#include <Q3StrList>
#include <Q3CString>
#include <domutil.h>

class KProcess;
class QString;
class Q3StrList;

namespace GDBDebugger
{

class Breakpoint;
class DbgCommand;
class TrimmableItem;
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
  s_core              = 1024,
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
    virtual char *parse(char *str)                                          = 0;

public slots:
    virtual void configure()                                                = 0;

    /**
     * Start the debugger
     * \param shell shell
     * \param run_envvars List with the environment variables
     * \param run_directory Directory from where the program should be run
     * \param application Absolute path to application
     * \param run_arguments Command line arguments to be passed to the application
     */
    virtual void slotStart(const QString& shell,
                           const DomUtil::PairList& run_envvars,
                           const QString& run_directory,
                           const QString &application,
                           const QString& run_arguments)                    = 0;
    //virtual void slotStart(const QString& shell, const QString &application)= 0;
    virtual void slotCoreFile(const QString &coreFile)                      = 0;
    virtual void slotAttachTo(int pid)                                      = 0;

    virtual void slotStopDebugger()                                         = 0;

    virtual void slotRun()                                                  = 0;
    virtual void slotRunUntil(const QString &fileName, int lineNum)         = 0;
    virtual void slotStepInto()                                             = 0;
    virtual void slotStepOver()                                             = 0;
    virtual void slotStepIntoIns()                                          = 0;
    virtual void slotStepOverIns()                                          = 0;
    virtual void slotStepOutOff()                                           = 0;

    virtual void slotBreakInto()                                            = 0;
    virtual void slotBPState(const Breakpoint&)                             = 0;

    virtual void slotDisassemble(const QString &start, const QString &end)  = 0;
    virtual void slotMemoryDump(const QString &start, const QString &amount)= 0;
    virtual void slotRegisters()                                            = 0;
    virtual void slotLibraries()                                            = 0;

    virtual void slotExpandItem(TrimmableItem *parent)                     = 0;
    virtual void slotExpandUserItem(VarItem *parent,
                                    const Q3CString &userRequest)            = 0;
    virtual void slotSelectFrame(int frame, int thread, bool needFrames)    = 0;
    virtual void slotSetLocalViewState(bool onOff)                          = 0;

    // jw - for optional additional commands and initialization
    virtual void slotVarItemConstructed(VarItem */*item*/) {}

protected slots:
    virtual void slotDbgStdout(KProcess *proc, char *buf, int buflen)       = 0;
    virtual void slotDbgStderr(KProcess*, char*, int) {} ;
    virtual void slotDbgWroteStdin(KProcess *proc)                          = 0;
    virtual void slotDbgProcessExited(KProcess *proc)                       = 0;

signals:
    void gotoSourcePosition   (const QString &fileName, int lineNum);
    void rawGDBBreakpointList (char *buf);
    void rawGDBBreakpointSet  (char *buf, int key);
    void rawGDBDisassemble    (char *buf);
    void rawGDBMemoryDump     (char *buf);
    void rawGDBRegisters      (char *buf);
    void rawGDBLibraries      (char *buf);
    void ttyStdout            (const char *output);
    void ttyStderr            (const char *output);
    void gdbStdout            (const char *output);
    void gdbStderr            (const char *output);
    void showStepInSource     (const QString &fileName, int lineNum, const QString &address);
    void dbgStatus            (const QString &status, int statusFlag);

protected:
    KProcess *dbgProcess_;
};

}

#endif
