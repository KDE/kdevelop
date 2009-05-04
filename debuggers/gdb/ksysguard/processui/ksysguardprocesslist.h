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

#ifndef _KSysGuardProcessList_h_
#define _KSysGuardProcessList_h_

#include <QtGui/QWidget>
#include <kapplication.h>
#include "ProcessModel.h"
#include "ProcessFilter.h"
#include "processes.h"

class QShowEvent;
class QHideEvent;
class QLineEdit;
class QTreeView;
struct KSysGuardProcessListPrivate;

/**
 * This widget implements a process list page. Besides the process
 * list which is implemented as a ProcessList, it contains two
 * comboxes and two buttons.  The combo boxes are used to set the
 * update rate and the process filter.  The buttons are used to force
 * an immediate update and to kill a process.
 */
class KDE_EXPORT KSysGuardProcessList : public QWidget
{
	Q_OBJECT
	Q_PROPERTY( bool showTotalsInTree READ showTotals WRITE setShowTotals )
	Q_PROPERTY( ProcessFilter::State state READ state WRITE setState )
	Q_PROPERTY( int updateIntervalMSecs READ updateIntervalMSecs WRITE setUpdateIntervalMSecs )
	Q_PROPERTY( ProcessModel::Units units READ units WRITE setUnits )
	Q_PROPERTY( bool killButtonVisible READ isKillButtonVisible WRITE setKillButtonVisible )
	Q_ENUMS( ProcessFilter::State )
	Q_ENUMS( ProcessModel::Units )

public:
	KSysGuardProcessList(QWidget* parent, const QString &hostName = QString());
	virtual ~KSysGuardProcessList();

	QLineEdit *filterLineEdit() const;
	QTreeView *treeView() const;

	/** Returns which processes we are currently filtering for and the way in which we show them.
	 *  @see setState()
	 */
	ProcessFilter::State state() const;
	
	/** Returns the number of milliseconds that have to elapse before updating the list of processes */
	int updateIntervalMSecs() const;

	/** Whether the widget will show child totals for CPU and Memory etc usage */
	bool showTotals() const;

	/** The units to display memory sizes etc in.  E.g. kb/mb/gb */
	ProcessModel::Units units() const;

	/** Returns a list of the processes that have been selected by the user. */
	QList<KSysGuard::Process *> selectedProcesses() const;

	/** Save the current state of the widget to the given config group 
	 *
	 *  @param[in] cg Config group to add these settings to
	 * */
	void saveSettings(KConfigGroup &cg);
	
	/** Load the saved state of the widget from the given config group */
	void loadSettings(const KConfigGroup &cg);

	/** Returns the process model used. Use with caution. */
	ProcessModel *processModel();
	
	/** Restore the headings to the given state. */
	void restoreHeaderState(const QByteArray & state);
    
	/** @returns whether the Kill Process button is visible. */
	bool isKillButtonVisible() const;
    
	/** @param visible defines whether the Kill Process button is shown or not. */
	void setKillButtonVisible(bool visible);
    
Q_SIGNALS:
	/** Emitted when the display has been updated */
	void updated();
public Q_SLOTS:
	/** Inform the view that the user has changed the selection */
	void selectionChanged();
	
	/** Send a kill signal to all the processes that the user has selected.  Pops up a dialog box to confirm with the user */
	void killSelectedProcesses();
	
	/** Send a signal to a list of given processes.
	 *   @p pids A list of PIDs that should be sent the signal 
	 *   @p sig  The signal to send. 
	 *   @return Whether the kill went ahead. True if successful or user cancelled.  False if there was a problem
	 */
	bool killProcesses(const QList< long long> &pids, int sig);

	/** Renice all the processes that the user has selected.  Pops up a dialog box to ask for the nice value and confirm */
	void reniceSelectedProcesses();
	
	/** Change the CPU scheduler for the given of processes to the given scheduler, with the given scheduler priority.
	 *  If the scheduler is Other or Batch, @p newCpuSchedPriority is ignored.
	 *   @return Whether the cpu scheduler changing went ahead.  True if successful or user cancelled.  False if there was a problem
	 */
	bool changeCpuScheduler(const QList< long long> &pids, KSysGuard::Process::Scheduler newCpuSched, int newCpuSchedPriority);

	/** Change the I/O scheduler for the given of processes to the given scheduler, with the given scheduler priority.
	 *  If the scheduler is Other or Batch, @p newCpuSchedPriority is ignored.
	 *   @return Whether the cpu scheduler changing went ahead.  True if successful or user cancelled.  False if there was a problem
	 */
	bool changeIoScheduler(const QList< long long> &pids, KSysGuard::Process::IoPriorityClass newIoSched, int newIoSchedPriority);
	/** Renice the processes given to the given niceValue. 
	 *   @return Whether the kill went ahead.  True if successful or user cancelled.  False if there was a problem
	 * */ 
	bool reniceProcesses(const QList<long long> &pids, int niceValue);

	/** Fetch new process information and redraw the display */
	void updateList();
	
	/** Set which processes we are currently filtering for and the way in which we show them. */
	void setState(ProcessFilter::State state);
	
 	/** Set the number of milliseconds that have to elapse before updating the list of processes */
	void setUpdateIntervalMSecs(int intervalMSecs);

	/** Set whether to show child totals for CPU and Memory etc usage */
	void setShowTotals(bool showTotals);

        /** Focus on a particular process, and select it */
        void selectAndJumpToProcess(int pid);

	/** The units to display memory sizes etc in. */
	void setUnits(ProcessModel::Units unit);

	/** Row was just inserted in the filter model */
	void rowsInserted ( const QModelIndex & parent, int start, int end );

private Q_SLOTS:

	/** Expand all the children, recursively, of the node given.  Pass an empty QModelIndex to expand all the top level children */
	void expandAllChildren(const QModelIndex &parent);

	/** Expand init to show its children, but not the sub children processes. */
	void expandInit();
	
	/** Display a context menu for the column headings allowing the user to show or hide columns. */
	void showColumnContextMenu(const QPoint &point);
	
	/**  Display a context menu for the given process allowing the user to kill etc the process */
	void showProcessContextMenu(const QModelIndex &index);
	/** Display a context menu for the selected processes allowing the user to kill etc the process */
	void showProcessContextMenu(const QPoint &point);
	
	/** Handle the situation where killing a process has failed - usually due to insufficent rights */
	void killFailed();
	
	/** Handle the situation where renicing a process has failed - usually due to insufficent rights */
	void reniceFailed();

	/** Handle the situation where ionice'ing a process has failed - usually due to insufficent rights */
	void ioniceFailed();
	
	/** Set state from combo box int value */
	void setStateInt(int state);

	/** Called when the text in the gui filter text box has changed */
	void filterTextChanged(const QString &newText);

	/** Called when one of the actions (kill, renice etc) is clicked etc */
	void actionTriggered(QObject *object);
protected:
	/** Inherit QWidget::showEvent(QShowEvent *) to enable the timer, for updates, when visible */
	virtual void showEvent(QShowEvent*);
	
	/** Inherit QWidget::hideEvent(QShowEvent *) to disable the timer, for updates, when not visible */
	virtual void hideEvent(QHideEvent*);
	
	/** Capture any change events sent to this widget.  In particular QEvent::LanguageChange */
	virtual void changeEvent ( QEvent * event );

	bool eventFilter(QObject *obj, QEvent *event);

	/** Retranslate the Ui as needed */
	void retranslateUi();

private:
	KSysGuardProcessListPrivate* const d;
};

#endif
