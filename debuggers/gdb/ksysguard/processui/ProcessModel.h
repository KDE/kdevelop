/*
    KSysGuard, the KDE System Guard

	Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>
	Copyright (c) 2006 John Tapsell <john.tapsell@kde.org>

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

#ifndef PROCESSMODEL_H_
#define PROCESSMODEL_H_

#include <QtCore/QAbstractItemModel>

#include <kdemacros.h>

namespace KSysGuard {
	class Processes;
	class Process;
}

class ProcessModelPrivate;

#ifdef Q_CC_MSVC
// this workaround is needed to make krunner link under msvc
// please keep it this way even if you port this library to have a _export.h header file
#define KSYSGUARD_EXPORT
#else
#define KSYSGUARD_EXPORT KDE_EXPORT
#endif

class KSYSGUARD_EXPORT ProcessModel : public QAbstractItemModel
{
	Q_OBJECT
	Q_ENUMS(Units)
		
public:
	ProcessModel(QObject* parent = 0, const QString &host = QString() );
	virtual ~ProcessModel();

	/* Functions for our Model for QAbstractItemModel*/
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
        QVariant data(const QModelIndex &index, int role) const;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
	QModelIndex parent ( const QModelIndex & index ) const;
	
	bool hasChildren ( const QModelIndex & parent) const;

	/* Functions for drag and drop and copying to clipboard, inherited from QAbstractItemModel */
	QStringList mimeTypes() const;
	QMimeData *mimeData(const QModelIndexList &indexes) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;

	/* Functions for setting the model */

	/** Setup the column headings by inserting the appropriate headings into the model.
	 *  Can be called more than once to retranslate the headings if the system language changes.
	 */
	void setupHeader();

	/** Update data.  You can pass in the time between updates to only update if there hasn't
	 *  been an update within the last @p updateDurationMSecs milliseconds */
	void update(int updateDurationMSecs = 0);
	
	/** Return a string with the pid of the process and the name of the process.  E.g.  13343: ksyguard
	 */
	QString getStringForProcess(KSysGuard::Process *process) const;
	KSysGuard::Process *getProcess(qlonglong pid);

	/** This is used from ProcessFilter to get the process at a given index when in flat mode */	
	KSysGuard::Process *getProcessAtIndex(int index) const;
        
	/** Returns whether this user can log in or not.
	 *  @see mUidCanLogin
	 */
	bool canUserLogin(long uid) const;
	/** In simple mode, everything is flat, with no icons, few if any colors, no xres etc.
	 *  This can be changed at any time.  It is a fairly quick operation.  Basically it resets the model
	 */ 
	void setSimpleMode(bool simple);
	/** In simple mode, everything is flat, with no icons, few if any colors, no xres etc
	 */
	bool isSimpleMode() const;
	
	/** Returns the total amount of physical memory in the machine. */
	qlonglong totalMemory() const;

        /** This returns a QModelIndex for the given process.  It has to look up the parent for this pid, find the offset this 
	 *  pid is from the parent, and return that.  It's not that slow, but does involve a couple of hash table lookups.
	 */
	QModelIndex getQModelIndex ( KSysGuard::Process *process, int column) const;

	/** Whether this is showing the processes for the current machine
	 */
	bool isLocalhost() const;

	/** The host name that this widget is showing the processes of */
	QString hostName() const;
	
	/** Whether this process has a GUI window */
	bool hasGUIWindow(qlonglong pid) const;

	/** Returns for process controller pointer for this model
	 */
	KSysGuard::Processes *processController();   ///The processes instance

	/** The headings in the model.  The order here is the order that they are shown
	 *  in.  If you change this, make sure you also change the 
	 *  setup header function, and make sure you increase PROCESSHEADERVERSION.  This will ensure
	 *  that old saved settings won't be used
	 */
#define PROCESSHEADERVERSION 2
	enum { HeadingName=0, HeadingUser, HeadingPid, HeadingTty, HeadingNiceness, HeadingCPUUsage, HeadingVmSize, HeadingMemory, HeadingSharedMemory, HeadingCommand, HeadingXTitle };
	enum { UidRole = Qt::UserRole, SortingValueRole, WindowIdRole, TotalMemoryRole, NumberOfProcessorsRole, PlainValueRole };

	bool showTotals() const;

	/** When displaying memory sizes, this is the units it should be displayed in */
	enum Units { UnitsKB, UnitsMB, UnitsGB, UnitsPercentage  };
	/** Set the units memory sizes etc should be displayed in */
	void setUnits(Units units);
	/** The units memory sizes etc should be displayed in */
	Units units() const;
	/** Take an amount in kb, and return a string in the units set by setUnits() */
	QString formatMemoryInfo(qlonglong amountInKB) const;
	/** Whether to show the command line options in the process name column */
	bool isShowCommandLineOptions() const;
	/** Set whether to show the command line options in the process name column */
	void setShowCommandLineOptions(bool showCommandLineOptions);

	/** Whether to show tooltips when the mouse hovers over a process */
	bool isShowingTooltips() const;
	/** Set whether to show tooltips when the mouse hovers over a process */
	void setShowingTooltips(bool showTooltips);
	/** Whether to divide CPU usage by the number of CPUs */
	bool isNormalizedCPUUsage() const;
	/** Set whether to divide CPU usage by the number of CPUs */
	void setNormalizedCPUUsage(bool normalizeCPUUsage);

	/** Retranslate the GUI, for when the system language changes */
	void retranslateUi();

public Q_SLOTS:

	/** Whether to show the total cpu for the process plus all of its children */
	void setShowTotals(bool showTotals);

private:
	ProcessModelPrivate*  const d;
	friend class ProcessModelPrivate;
};

#endif

