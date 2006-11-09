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

#include "mi/gdbmi.h"

#include <qobject.h>
#include <domutil.h>



class KProcess;
class QString;
class QStrList;

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
  s_waitForWrite      = 8,
  s_programExited     = 16,
  s_viewBT            = 128,
  s_viewBP            = 256,
  s_attached          = 512,
  s_core              = 1024,
  s_waitTimer         = 2048,
  // Set when 'slotStopDebugger' started executing, to avoid
  // entering that function several times.
  s_shuttingDown      = 4096,
  s_explicitBreakInto = (s_shuttingDown << 1),
  s_dbgBusy           = (s_explicitBreakInto << 1),
  s_appRunning        = (s_dbgBusy << 1),
  s_lastDbgState      = (s_appRunning << 1)

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

public slots:
    virtual void configure()                                                = 0;

    virtual void slotCoreFile(const QString &coreFile)                      = 0;
    virtual void slotAttachTo(int pid)                                      = 0;

    virtual void slotStopDebugger()                                         = 0;

    virtual void slotRun()                                                  = 0;
    // Kills the application but does not stop the debugger itself.
    virtual void slotKill()                                                 = 0;
    virtual void slotRunUntil(const QString &fileName, int lineNum)         = 0;
    virtual void slotJumpTo(const QString &fileName, int lineNum)           = 0;
    virtual void slotStepInto()                                             = 0;
    virtual void slotStepOver()                                             = 0;
    virtual void slotStepIntoIns()                                          = 0;
    virtual void slotStepOverIns()                                          = 0;
    virtual void slotStepOutOff()                                           = 0;

    virtual void slotBreakInto()                                            = 0;

    virtual void slotRegisters()                                            = 0;
    virtual void slotLibraries()                                            = 0;

    // jw - for optional additional commands and initialization
    virtual void slotVarItemConstructed(VarItem */*item*/) {}

protected slots:
    virtual void slotDbgStdout(KProcess *proc, char *buf, int buflen)       = 0;
    virtual void slotDbgStderr(KProcess*, char*, int) {} ;
    virtual void slotDbgWroteStdin(KProcess *proc)                          = 0;
    virtual void slotDbgProcessExited(KProcess *proc)                       = 0;

signals:
    void gotoSourcePosition   (const QString &fileName, int lineNum);
    void rawGDBMemoryDump     (char *buf);
    void rawGDBRegisters      (char *buf);
    void rawGDBLibraries      (char *buf);
    void ttyStdout            (const char *output);
    void ttyStderr            (const char *output);
    void gdbInternalCommandStdout (const char *output);
    void gdbUserCommandStdout (const char *output);
    void gdbStderr            (const char *output);
    void showStepInSource     (const QString &fileName, int lineNum, const QString &address);
    void dbgStatus            (const QString &status, int statusFlag);

protected:
    KProcess *dbgProcess_;
};

}

#endif
