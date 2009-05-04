/*
    KSysGuard, the KDE System Guard

    Copyright (c) 2008 John Tapsell <tapsell@kde.org>

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

#ifndef _KMonitorProcessIO_h_
#define _KMonitorProcessIO_h_

#include <QtCore/QTimer>
#include "KTextEditVT.h"
#include <kdialog.h>
#include <kprocess.h>
#include "processes.h"

class KDE_EXPORT KMonitorProcessIO : public KTextEditVT
{
	Q_OBJECT
	Q_PROPERTY( bool includeChildProcesses READ includeChildProcesses WRITE setIncludeChildProcesses )
	Q_PROPERTY( bool updateInterval READ updateInterval WRITE setUpdateInterval )
	Q_PROPERTY( int attachedPid READ attachedPid WRITE attach )
	Q_PROPERTY( State status READ state WRITE setState )
	Q_ENUMS( State )

public:
	KMonitorProcessIO(QWidget* parent, int pid = -1);
	~KMonitorProcessIO();

	/** Whether to include the output from child processes.  If true, forks and clones will be monitored */
	bool includeChildProcesses() const;

	/** Interval to poll for new ptrace input.  Recommended around 20 (milliseconds).  Note that the process
	 *  being monitored cannot do anything if it is waiting for us to update.
	 *  Default is 20 (ms)
	 */
	int updateInterval() const;
	/** Set interval to poll for new ptrace input.  Recommended around 20 (milliseconds).  Note that the process
	 *  being monitored cannot do anything if it is waiting for us to update.
	 *  Default is 20 (ms)
	 */
	void setUpdateInterval(int msecs);

	/** Detached state indicates that we are not connected to the process and not monitoring it.
	 *
	 *  AttachedRunning state indicates that we are attached to the process and displaying its output.
	 *
	 *  AttachedPaused state indicates that we are attached but not reading its output.  This will block the process until we resume or detach.
	 */
	enum State { Detached, AttachedRunning, AttachedPaused };

	/** Return the current state. */
	KMonitorProcessIO::State state() const;

public Q_SLOTS:
	/** Set whether to include the output from child processes.  If true, forks and clones will be monitored */
	void setIncludeChildProcesses(bool include);
	/** If the state is in AttachedRunning, change to AttachedPaused.  This will block the process until we resume or detach.*/
	void pauseProcesses();
	/** If the state is in AttachedPaused, change to AttachedRunning.  This will allow the process to run again. */
	void resumeProcesses();
	/** Stop monitoring all processes*/
	void detach();
	/** Stop monitoring the given process */
	void detach(int pid);
	/** Start monitoring the given process.  If this is the first process being monitored, the state is set to
	 *  AttachedRunning if possible and attachedPid() will return @p pid 
	 *  @return true if successfully reattached.  Can fail if process has disappeared or we do not have the right to attach. */
	bool attach(int pid);
	/** Reattach the pid that was first attached. 
	 *  @return true if successfully reattached.  Can fail if process has disappeared or we do not have the right to attach. */
	bool reattach();
	/** Return the main pid that we are monitoring.*/
	int attachedPid() const;
	/** Attempts to set the state.  Check status() to confirm whether the state has changed successfully. */
	void setState(State new_state);

Q_SIGNALS:
	void finished();
private Q_SLOTS:
	/** Read in the next bit of data and display it.  This should be called very frequently. */
	void update(bool modified=false);

private:
	KProcess mIOProcess;
	KTextEditVT *mTextEdit;
	QTimer mTimer;
	int mPid;
	QList<int> attached_pids;

	int mUpdateInterval;
	bool mIncludeChildProcesses;
	bool remove_duplicates;

	unsigned int lastdir;
	QTextCursor mCursor;
};

#endif

