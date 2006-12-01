
/***************************************************************************
*   Copyright (C) 2006 by Andras Mantia                                   *
*   amantia@kde.org                                                       *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/


#ifndef BLOCKINGKPROCESS_H
#define BLOCKINGKPROCESS_H

#include <kprocess.h>

/**
 *  Blocking version of KProcess, which stores the stdout.
 *  Differences between start(KProcess::Block, KProcess::StdOut) and this
 *  class are:
 *  - the GUI update is not blocked why the external process is running
 *  - in case of problems there is a timeout (defaults to 60 seconds), after which the 
 *    process is terminated.
 *  - the stdout is caught - it the caller request it - and can be read by the caller
 * @author Andras Mantia <amantia@kde.org>
*/

class QTimer;
class BlockingKProcess : public KProcess
{
  Q_OBJECT
  
public:
    BlockingKProcess(QObject *parent, const char *name=0);
    BlockingKProcess();

    virtual ~BlockingKProcess();
    
    /**
     * Start the process. It waits until the process exits or the timeout is hit.
     * @param runmode @see KProcess, use KProcess::NotifyOnExit to get proper behaviour,
     * not KProcess::Block
     * @param comm if Stdout is passed, it catches the output. For the rest @see KProcess
     * @return true in case of success, false if there are problems to start the process
     *        or it was killed because of the timeout.
     */
    virtual bool start(RunMode runmode=NotifyOnExit, Communication comm=NoCommunication);
  
    /**
     * Get the output of the run process
     * @return the output
     */
    QString stdOut() { return m_stdOut;}
    /**
     * Clear the internal stdout buffer. Useful in case the class is reused.
     */
    void clearStdOut() { m_stdOut = "";}
    /**
     * Get the error output of the run process
     * @return the output
     */
    QString stdErr() { return m_stdErr;}
    /**
     * Clear the internal stderr buffer. Useful in case the class is reused.
     */
    void clearStdErr() { m_stdErr = "";}

    /**
     * Sets the timeout
     * @param timeout seconds after which the process is considered hung and killed. 0 disables the timeout.
     */
    void setTimeOut(int timeout) { m_timeoutValue = timeout; }
    
private slots:
    void slotReceivedStdOut(KProcess *proc, char *buffer, int buflen);
    void slotReceivedStdErr(KProcess *proc, char *buffer, int buflen);
    void slotProcessExited(KProcess *proc);
    void slotTimeOut();
            
private:
    void enter_loop();
        
    QString m_stdOut;   
    QString m_stdErr;   
    bool m_timeout;     
    int m_timeoutValue;
    QTimer *m_timer;
};

#endif
