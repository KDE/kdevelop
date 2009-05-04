/*  This file is part of the KDE project
    
    Copyright (C) 2007 John Tapsell <tapsell@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef PROCESSES_BASE_P_H
#define PROCESSES_BASE_P_H

#include <QSet>
#include <QObject>

namespace KSysGuard
{
    class Process;
    /**
     * This class contains the specific code to get the processes from the given host.
     *
     * To port this to other operating systems you need to make a processes_(osname).cpp  file
     * which implements all of the function below.  If you need private functions/variables etc put them in
     * the Private class.
     *
     * @author John Tapsell <tapsell@kde.org>
     */
    class AbstractProcesses : public QObject
    {
      Q_OBJECT

      public:

	AbstractProcesses() {}
	virtual ~AbstractProcesses() {}
	/**
	 *  To get information about processes, this will be the first function called.
	 */
	virtual QSet<long> getAllPids() = 0;
	/**
	 *  For each of the pids that getAllPids() returns, getParentPid will be called.  This is used to setup the tree structure.
	 *  For a particular pid, this is guaranteed to be called before updateProcessInfo for that pid.
	 *  However this may be called several times in a row before the updateProcessInfo is called, so be careful
	 *  if you want to try to preserve state in Private.
	 */
        virtual long getParentPid(long pid) = 0;
	/**
	 *  This will be called for every pid, after getParentPid() has been called for the same parameter.
	 *
	 *  The process->pid process->ppid and process->parent  are all guaranteed to be filled in correctly and process->parent
	 *  will be non null.  
	 */ 
        virtual bool updateProcessInfo(long pid, Process *process) = 0;
        /**
	 *  Send the specified named POSIX signal to the process given.
	 *
	 *  For example, to indicate for process 324 to STOP do:
	 *  \code
	 *    #include <signals.h>
	 *     ...
	 *
	 *    KSysGuard::Processes::sendSignal(23, SIGSTOP);
	 *  \endcode
	 *
	 */
	virtual bool sendSignal(long pid, int sig) = 0;

	/**
	 *  Set the priority for a process.  For the normal scheduler, this is usually from 19 
	 *  (very nice, lowest priority) to -20 (highest priority).  The default value for a process is 0.
	 *
	 *  This has no effect if the scheduler is not the normal one (SCHED_OTHER)
	 *  
	 *  @return false if you do not have permission to set the priority
	 */
	virtual bool setNiceness(long pid, int priority) = 0;

	/**
	 *  Set the scheduler for a process.  This is defined according to POSIX.1-2001 
	 *  See "man sched_setscheduler" for more information.
	 *
	 *  @p priorityClass One of SCHED_FIFO, SCHED_RR, SCHED_OTHER, and SCHED_BATCH
	 *  @p priority Set to 0 for SCHED_OTHER and SCHED_BATCH.  Between 1 and 99 for SCHED_FIFO and SCHED_RR
	 *  @return false if you do not have permission to set the priority
	 */
	virtual bool setScheduler(long pid, int priorityClass, int priority) = 0;
	
	/**
	 *  Return the total amount of physical memory in KB.  This is fast (just a system call)
	 *  Returns 0 on error
	 */
	virtual long long totalPhysicalMemory() = 0;

	/**
	 *  Set the io priority for a process.  This is from 7 (very nice, lowest io priority) to
	 *  0 (highest priority).  The default value is determined as: io_nice = (cpu_nice + 20) / 5.
	 *
	 *  @return false if you do not have permission to set the priority
	 */
	virtual bool setIoNiceness(long pid, int priorityClass, int priority) = 0;

	/**
	 *  Returns true if ionice is supported on this system
	 */
	virtual bool supportsIoNiceness() = 0;

	/**
	 *  Return the number of processor cores enabled. 
	 *  (A system can disable procesors.  Disabled processors are not counted here).
	 *  This is fast (just a system call) */
	virtual long numberProcessorCores() = 0;

	/**
	 *  Get all the current process information from the machine.  When done, emit updateAllProcesses().
	 */
	virtual void updateAllProcesses() = 0;

      Q_SIGNALS:
	/**
	 *  This is emitted when the processes have been updated, and the view should be refreshed
	 */
        void processesUpdated();

    };
}

#endif // PROCESSES_BASE_P_H
