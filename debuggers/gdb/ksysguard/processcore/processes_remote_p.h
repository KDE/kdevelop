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

#ifndef PROCESSES_REMOTE_P_H_
#define PROCESSES_REMOTE_P_H_

#include "processes_base_p.h"
#include <QSet>
class Process;
namespace KSysGuard
{
    /**
     * This is used to connect to a remote host
     */
    class ProcessesRemote : public AbstractProcesses {
      Q_OBJECT
      public:
	ProcessesRemote(const QString &hostname);
	virtual ~ProcessesRemote();
	virtual QSet<long> getAllPids();
	virtual long getParentPid(long pid);
	virtual bool updateProcessInfo(long pid, Process *process);
	virtual bool sendSignal(long pid, int sig);
        virtual bool setNiceness(long pid, int priority);
	virtual bool setScheduler(long pid, int priorityClass, int priority);
	virtual long long totalPhysicalMemory();
	virtual bool setIoNiceness(long pid, int priorityClass, int priority);
	virtual bool supportsIoNiceness();
	virtual long numberProcessorCores();
	virtual void updateAllProcesses();


      Q_SIGNALS:
	/** For a remote machine, we rely on being able to communicate with ksysguardd.
	 *  This must be dealt with by the program including this widget.  It must listen to our
	 *  'runCommand' signal, and run the given command, with the given id. */
	void runCommand(const QString &command, int id);

      public Q_SLOTS:
	/** For a remote machine, we rely on being able to communicate with ksysguardd.
	 *  The programming using this must call this slot when an answer is received from ksysguardd,
	 *  in response to a runCommand request.  The id identifies the answer */
	void answerReceived( int id, const QList<QByteArray>& answer );
	/** Called soon after */
	void setup();

      protected:
	enum { PsInfo, Ps, UsedMemory, FreeMemory, Kill, Renice, Ionice };

      private:
	/**
	 * You can use this for whatever data you want.  Be careful about preserving state in between getParentPid and updateProcessInfo calls
	 * if you chose to do that. getParentPid may be called several times for different pids before the relevant updateProcessInfo calls are made.
	 * This is because the tree structure has to be sorted out first.
	 */
        class Private;
        Private *d;

    };
}
#endif 
