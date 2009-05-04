/*
    KSysGuard, the KDE System Guard

	Copyright (c) 2006-2007 John Tapsell <john.tapsell@kde.org>

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

#ifndef PROCESSMODEL_P_H_
#define PROCESSMODEL_P_H_

#include <kapplication.h>
#include <kuser.h>
#include <QPixmap>
#include <QObject>
#include <QList>
#include <QVariant>
#include <QHash>
#include <QSet>
#include <QTime>


#ifdef Q_WS_X11
#include <kwindowsystem.h>
#include <netwm.h>
#include <QtGui/QX11Info>
#include <X11/Xatom.h>
#include <kxerrorhandler.h>
#endif

#include "processcore/process.h"


#ifdef Q_WS_X11
struct WindowInfo {
	QPixmap icon;
	WId wid;
	NETWinInfo *netWinInfo;
};

#endif 


class ProcessModel;
class ProcessModelPrivate : public QObject
{
	Q_OBJECT
public:
        ProcessModelPrivate();
        ~ProcessModelPrivate();
public slots:

#ifdef Q_WS_X11
	/** When an X window is changed, this is called */
	void windowChanged(WId wid, unsigned int properties);
	/** When an X window is created, this is called
	 */
	void windowAdded(WId wid);
	/** When an X window is closed, this is called
	 */
	void windowRemoved(WId wid);
#endif

	/** Change the data for a process.  This is called from KSysGuard::Processes
	 *  if @p onlyCpuOrMem is set, only the total cpu usuage is updated.
         *  process->changes  contains a bitfield of what has been changed 
	 */
        void processChanged(KSysGuard::Process *process, bool onlyCpuOrMem);
        /** Called from KSysGuard::Processes
	 *  This indicates we are about to insert a process in the model.  Emit the appropriate signals
	 */
	void beginInsertRow( KSysGuard::Process *parent);
        /** Called from KSysGuard::Processes
	 *  We have finished inserting a process
	 */
        void endInsertRow();
        /** Called from KSysGuard::Processes
	 *  This indicates we are about to remove a process in the model.  Emit the appropriate signals
	 */
	void beginRemoveRow( KSysGuard::Process *process);
        /** Called from KSysGuard::Processes
	 *  We have finished removing a process
	 */
        void endRemoveRow();
	/** Called from KSysGuard::Processes
	 *  This indicates we are about to move a process in the model from one parent process to another.  Emit the appropriate signals
	 */
	void beginMoveProcess(KSysGuard::Process *process, KSysGuard::Process *new_parent);
        /** Called from KSysGuard::Processes
	 *  We have finished moving a process
	 */
        void endMoveRow();

public:
	/** On X11 system, connects to the signals emitted when windows are created/destroyed */
	void setupWindows();
	/** Connects to the host */
	void setupProcesses();
        /** A mapping of running,stopped,etc  to a friendly description like 'Stopped, either by a job control signal or because it is being traced.'*/
	QString getStatusDescription(KSysGuard::Process::ProcessStatus status) const;
		
	/** Return a qt markup tooltip string for a local user.  It will have their full name etc.
	 *  This will be slow the first time, as it practically indirectly reads the whole of /etc/passwd
	 *  But the second time will be as fast as hash lookup as we cache the result
	 */
	inline QString getTooltipForUser(const KSysGuard::Process *process) const;

	/** Return a username for a local user if it can, otherwise just their uid.
	 *  This may have been given from the result of "ps" (but not necessarily).  If it's not found, then it
	 *  needs to find out the username from the uid. This will be slow the first time, as it practically indirectly reads the whole of /etc/passwd
	 *  But the second time will be as fast as hash lookup as we cache the result
	 *
	 *  If withuid is set, and the username is found, return: "username (Uid: uid)"
	 */
	inline QString getUsernameForUser(long uid, bool withuid) const;

	/** Return the groupname for a given gid.  This is in the form of "gid" if not known, or
	 *  "groupname (Uid: gid)" if known.
	 */
	inline QString getGroupnameForGroup(long gid) const;
	/** @see setIsLocalhost */
	bool mIsLocalhost;

	/** A caching hash for tooltips for a user.
	 *  @see getTooltipForUser */
	mutable QHash<long long,QString> mUserTooltips;

	/** A caching hash for username for a user uid, or just their uid if it can't be found (as a long long)
	 *  @see getUsernameForUser */
	mutable QHash<long long, QString> mUserUsername;

	/** A mapping of a user id to whether this user can log in.  We have to guess based on the shell. All are set to true to non localhost.
	 *  It is set to:
	 *    0 if the user cannot login
	 *    1 is the user can login
	 *  The reason for using an int and not a bool is so that we can do  mUidCanLogin.value(uid,-1)  and thus we get a tristate for whether
	 *  they are logged in, not logged in, or not known yet.
	 *  */
	mutable QHash<long long, int> mUidCanLogin; 


	/** A translated list of headings (column titles) in the order we want to display them. Used in headerData() */
	QStringList mHeadings;

	QMultiHash< long long, WindowInfo> mPidToWindowInfo;  ///Map a process pid to X window info if available
	QHash< WId, long long> mWIdToPid; ///Map an X window id to a process pid
	

	bool mShowChildTotals; ///If set to true, a parent will return the CPU usage of all its children recursively

	bool mSimple; ///In simple mode, the model returns everything as flat, with no icons, etc.  This is set by changing cmbFilter

	QTime mLastUpdated; ///Time that we last updated the processes.

	long long mMemTotal; /// the total amount of physical memory in kb in the machine.  We can used this to determine the percentage of memory an app is using
	int mNumProcessorCores;  /// The number of (enabled) processor cores in the this machine

	KSysGuard::Processes *mProcesses;  ///The processes instance

	bool mIsChangingLayout;

	QPixmap mBlankPixmap; ///Used to pad out process names which don't have an icon

	/** Show the process command line options in the process name column */
	bool mShowCommandLineOptions;

	bool mShowingTooltips;
	bool mNormalizeCPUUsage;
	/** When displaying memory sizes, this is the units it should be displayed in */
	int mUnits;

	/** The hostname */
	QString mHostName;

	ProcessModel* q;
};

#endif
