/*
    KSysGuard, the KDE System Guard

	Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>
	Copyright (c) 2006 - 2007 John Tapsell <john.tapsell@kde.org>

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

#include "../config-ksysguard.h"

#include <QTimer>
#include <QList>
#include <QShowEvent>
#include <QHideEvent>
#include <QSortFilterProxyModel>
#include <QHeaderView>
#include <QAction>
#include <QMenu>
#include <QSet>
#include <QComboBox>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QProcess>
#include <QLineEdit>
#include <QSignalMapper>


#include <signal.h> //For SIGTERM

#include <kapplication.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdialog.h>
#include <kicon.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <KWindowSystem>

#include "ksysguardprocesslist.moc"
#include "ksysguardprocesslist.h"
#include "ReniceDlg.h"
#include "ui_ProcessWidgetUI.h"

#ifdef WITH_MONITOR_PROCESS_IO
#include "DisplayProcessDlg.h"
#endif

#include <sys/types.h>
#include <unistd.h>

//Trolltech have a testing class for classes that inherit QAbstractItemModel.  If you want to run with this run-time testing enabled, put the modeltest.* files in this directory and uncomment the next line
//#define DO_MODELCHECK
#ifdef DO_MODELCHECK
#include "modeltest.h"
#endif

class ProgressBarItemDelegate : public QStyledItemDelegate
{
  public:
	ProgressBarItemDelegate(QObject *parent) : QStyledItemDelegate(parent), startProgressColor(0x00, 0x71, 0xBC, 100), endProgressColor(0x83, 0xDD, 0xF5, 100), totalMemory(-1), numCpuCores(-1) {}

  	virtual void paint(QPainter *painter, const QStyleOptionViewItem &opt, const QModelIndex &index) const
	{
		QStyleOptionViewItemV4 option = opt;
		initStyleOption(&option,index);

		Q_ASSERT(index.model());
		QModelIndex realIndex = (reinterpret_cast< const QAbstractProxyModel *> (index.model()))->mapToSource(index);
		KSysGuard::Process *process = reinterpret_cast< KSysGuard::Process * > (realIndex.internalPointer());
		Q_CHECK_PTR(process);
		if(index.column() == ProcessModel::HeadingCPUUsage) {
			if(numCpuCores == -1)
				numCpuCores = index.data(ProcessModel::NumberOfProcessorsRole).toInt();
			percentage = (process->userUsage + process->sysUsage) / numCpuCores;
		} else if(index.column() == ProcessModel::HeadingMemory) {
			long long memory = 0;
			if(process->vmURSS != -1)
				memory = process->vmURSS;
			else
				memory = process->vmRSS;
			if(totalMemory == -1)
				totalMemory = index.data(ProcessModel::TotalMemoryRole).toLongLong();
			if(totalMemory > 0)
				percentage = (int)(memory*100/totalMemory);
			else
				percentage = 0;
		} else if(index.column() == ProcessModel::HeadingSharedMemory) {
			if(process->vmURSS != -1) {
				if(totalMemory == -1)
					totalMemory = index.data(ProcessModel::TotalMemoryRole).toLongLong();
				if(totalMemory > 0)
					percentage = (int)((process->vmRSS - process->vmURSS)*100/totalMemory);
				else
					percentage = 0;
			}
		} else
			percentage = -1;

		if (percentage >= 0)
			drawPercentageDisplay(painter,option,index.data(Qt::DisplayRole).toString());
		else 
			QStyledItemDelegate::paint(painter, option, index);
	}

  private:
	void drawPercentageDisplay(QPainter *painter, const QStyleOptionViewItem& option, const QString& text) const
	{
		QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem,&option,painter);

		const QRect rect = option.rect;
		if(percentage * rect.width() > 100 ) { //make sure the line will have a width of more than 1 pixel
			if(percentage > 100) 
				percentage = 100;  //Don't draw outside our bounds
			QPen old = painter->pen();
			painter->setPen(Qt::NoPen);
			QLinearGradient  linearGrad( QPointF(rect.x(),rect.y()), QPointF(rect.x() + rect.width(), rect.y()));
			linearGrad.setColorAt(0, startProgressColor);
			linearGrad.setColorAt(1, endProgressColor);
			painter->fillRect( rect.x(), rect.y(), rect.width() * percentage /100 , rect.height(), QBrush(linearGrad));
			painter->setPen( old );
		}
		painter->drawText(rect,option.displayAlignment,text + ' ');
	}

	mutable int percentage;
	QColor startProgressColor;
	QColor endProgressColor;
	mutable long long totalMemory;
	mutable int numCpuCores;
};

struct KSysGuardProcessListPrivate {

	KSysGuardProcessListPrivate(KSysGuardProcessList* q, const QString &hostName)
            : mModel(q, hostName), mFilterModel(q), mUi(new Ui::ProcessWidget()), mProcessContextMenu(NULL), mUpdateTimer(NULL)
        {
		renice = new QAction(i18np("Renice Process...", "Renice Processes...", 1), q);
		selectParent = new QAction(i18n("Jump to Parent Process"), q);

		selectTracer = new QAction(i18n("Jump to Process Debugging This One"), q);
		window = new QAction(i18n("Show Application Window"), q);
#ifdef WITH_MONITOR_PROCESS_IO
		monitorio = new QAction(i18n("Monitor Input && Output"), q);
#else
		monitorio = 0;
#endif
		resume = new QAction(i18n("Resume Stopped Process"), q);
		kill = new QAction(i18np("Kill Process", "Kill Processes", 1), q);
		kill->setIcon(KIcon("process-stop"));

		sigStop = new QAction(i18n("Suspend (STOP)"), q);
		sigCont = new QAction(i18n("Continue (CONT)"), q);
		sigHup = new QAction(i18n("Hangup (HUP)"), q);
		sigInt = new QAction(i18n("Interrupt (INT)"), q);
		sigTerm = new QAction(i18n("Terminate (TERM)"), q);
		sigKill = new QAction(i18n("Kill (KILL)"), q);
		sigUsr1 = new QAction(i18n("User 1 (USR1)"), q);
		sigUsr2 = new QAction(i18n("User 2 (USR2)"), q);
	}

        ~KSysGuardProcessListPrivate() { delete mUi; mUi = NULL; }

	/** The process model.  This contains all the data on all the processes running on the system */
	ProcessModel mModel;

	/** The process filter.  The mModel is connected to this, and this filter model connects to the view.  This lets us
	 *  sort the view and filter (by using the combo box or the search line)
	 */
	ProcessFilter mFilterModel;

	/** The graphical user interface for this process list widget, auto-generated by Qt Designer */
	Ui::ProcessWidget *mUi;

	/** The context menu when you right click on a process */
	QMenu *mProcessContextMenu;

	/** A timer to call updateList() every mUpdateIntervalMSecs */
	QTimer *mUpdateTimer;

	/** The time to wait, in milliseconds, between updating the process list */
	int mUpdateIntervalMSecs;

	QAction *renice;
	QAction *kill;
	QAction *selectParent;
	QAction *selectTracer;
	QAction *window;
	QAction *monitorio;
	QAction *resume;
	QAction *sigStop;
	QAction *sigCont;
	QAction *sigHup;
	QAction *sigInt;
	QAction *sigTerm;
	QAction *sigKill;
	QAction *sigUsr1;
	QAction *sigUsr2;
};

KSysGuardProcessList::KSysGuardProcessList(QWidget* parent, const QString &hostName)
	: QWidget(parent), d(new KSysGuardProcessListPrivate(this, hostName))
{
	d->mUpdateIntervalMSecs = 2000; //Set 2 seconds as the default update interval
	d->mUi->setupUi(this);
	d->mFilterModel.setSourceModel(&d->mModel);
	d->mUi->treeView->setModel(&d->mFilterModel);
#ifdef DO_MODELCHECK
	new ModelTest(&d->mModel, this);
#endif
	d->mUi->treeView->setItemDelegate(new ProgressBarItemDelegate(d->mUi->treeView));

	d->mUi->treeView->header()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(d->mUi->treeView->header(), SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showColumnContextMenu(const QPoint&)));

	d->mProcessContextMenu = new QMenu(d->mUi->treeView);
	d->mUi->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(d->mUi->treeView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showProcessContextMenu(const QPoint&)));

	d->mUi->treeView->header()->setClickable(true);
	d->mUi->treeView->header()->setSortIndicatorShown(true);
	d->mUi->treeView->header()->setCascadingSectionResizes(false);
	connect(d->mUi->btnKillProcess, SIGNAL(clicked()), this, SLOT(killSelectedProcesses()));
	connect(d->mUi->txtFilter, SIGNAL(textChanged(const QString &)), this, SLOT(filterTextChanged(const QString &)));
	connect(d->mUi->cmbFilter, SIGNAL(currentIndexChanged(int)), this, SLOT(setStateInt(int)));
	connect(d->mUi->treeView, SIGNAL(expanded(const QModelIndex &)), this, SLOT(expandAllChildren(const QModelIndex &)));
	connect(d->mUi->treeView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection & , const QItemSelection & )), this, SLOT(selectionChanged()));
	connect(&d->mFilterModel, SIGNAL(rowsInserted( const QModelIndex &, int, int)), this, SLOT(rowsInserted(const QModelIndex &, int, int)));
	setMinimumSize(sizeHint());

	d->mFilterModel.setFilterKeyColumn(-1);


	/*  Hide the vm size column by default since it's not very useful */
	d->mUi->treeView->header()->hideSection(ProcessModel::HeadingVmSize);
	d->mUi->treeView->header()->hideSection(ProcessModel::HeadingNiceness);
	d->mUi->treeView->header()->hideSection(ProcessModel::HeadingTty);
	d->mUi->treeView->header()->hideSection(ProcessModel::HeadingCommand);
	d->mUi->treeView->header()->hideSection(ProcessModel::HeadingPid);
	// NOTE!  After this is all setup, the settings for the header are restored
	// from the user's last run.  (in restoreHeaderState)
	// So making changes here only affects the default settings.  To 
	// test changes temporarily, comment out the lines in restoreHeaderState.
	// When you are happy with the changes and want to commit, increase the 
	// value of PROCESSHEADERVERSION.  This will force the header state
	// to be reset back to the defaults for all users.
	d->mUi->treeView->header()->resizeSection(ProcessModel::HeadingCPUUsage, d->mUi->treeView->header()->sectionSizeHint(ProcessModel::HeadingCPUUsage));
	d->mUi->treeView->header()->resizeSection(ProcessModel::HeadingMemory, d->mUi->treeView->header()->sectionSizeHint(ProcessModel::HeadingMemory));
	d->mUi->treeView->header()->resizeSection(ProcessModel::HeadingSharedMemory, d->mUi->treeView->header()->sectionSizeHint(ProcessModel::HeadingSharedMemory));
	d->mUi->treeView->header()->setResizeMode(0, QHeaderView::Interactive);
	d->mUi->treeView->header()->setStretchLastSection(true);

	//Process names can have mixed case. Make the filter case insensitive.
	d->mFilterModel.setFilterCaseSensitivity(Qt::CaseInsensitive);
	d->mFilterModel.setSortCaseSensitivity(Qt::CaseInsensitive);

	d->mUi->txtFilter->installEventFilter(this);
	d->mUi->treeView->installEventFilter(this);


	d->mUi->treeView->setDragEnabled(true);
	d->mUi->treeView->setDragDropMode(QAbstractItemView::DragOnly);


	//Sort by username by default
	d->mUi->treeView->sortByColumn(ProcessModel::HeadingUser, Qt::AscendingOrder);
	d->mFilterModel.sort(ProcessModel::HeadingUser, Qt::AscendingOrder);

	// Add all the actions to the main widget, and get all the actions to call actionTriggered when clicked
	QSignalMapper *signalMapper = new QSignalMapper(this);
	QList<QAction *> actions;
	actions << d->renice << d->kill << d->selectParent << d->selectTracer << d->window;
	if (d->monitorio)
	    actions << d->monitorio;
	actions << d->resume << d->sigStop << d->sigCont << d->sigHup << d->sigInt << d->sigTerm << d->sigKill << d->sigUsr1 << d->sigUsr2;
	foreach(QAction *action, actions) {
		addAction(action);
		connect(action, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
		signalMapper->setMapping(action, action);
	}
	connect(signalMapper, SIGNAL(mapped(QObject *)), SLOT(actionTriggered(QObject *)));

	retranslateUi();

	d->mFilterModel.setDynamicSortFilter(true);

	d->mUpdateTimer = new QTimer(this);
	d->mUpdateTimer->setSingleShot(true);
	connect(d->mUpdateTimer, SIGNAL(timeout()), this, SLOT(updateList()));
	d->mUpdateTimer->start(d->mUpdateIntervalMSecs);

	d->mUi->btnKillProcess->setIcon(KIcon("edit-bomb"));

	//If the view resorts continually, then it can be hard to keep track of processes.  By doing it only every few seconds it reduces the 'jumping around'
//	QTimer *mTimer = new QTimer(this);
//	mTimer->start(4000);
// QT BUG?  We have to disable the sorting for now because there seems to be a bug in Qt introduced in Qt 4.4beta which makes the view scroll back to the top
        d->mModel.update(d->mUpdateIntervalMSecs);
}

KSysGuardProcessList::~KSysGuardProcessList()
{
	delete d;
}

QTreeView *KSysGuardProcessList::treeView() const {
	return d->mUi->treeView;
}

QLineEdit *KSysGuardProcessList::filterLineEdit() const {
	return d->mUi->txtFilter;
}

ProcessFilter::State KSysGuardProcessList::state() const
{
	return d->mFilterModel.filter();
}
void KSysGuardProcessList::setStateInt(int state) {
	setState((ProcessFilter::State) state);
	d->mUi->treeView->scrollTo( d->mUi->treeView->currentIndex());
}
void KSysGuardProcessList::setState(ProcessFilter::State state)
{  //index is the item the user selected in the combo box
	d->mFilterModel.setFilter(state);
	d->mModel.setSimpleMode( (state != ProcessFilter::AllProcessesInTreeForm) );
	d->mUi->cmbFilter->setCurrentIndex( (int)state);
	expandInit();
}
void KSysGuardProcessList::filterTextChanged(const QString &newText) {
	d->mFilterModel.setFilterRegExp(newText.trimmed());
	expandInit();
	d->mUi->btnKillProcess->setEnabled( d->mUi->treeView->selectionModel()->hasSelection() );
	d->mUi->treeView->scrollTo( d->mUi->treeView->currentIndex());
}
void KSysGuardProcessList::selectionChanged()
{
	int numSelected =  d->mUi->treeView->selectionModel()->selectedRows().size();
	d->mUi->btnKillProcess->setEnabled( numSelected != 0 );

	d->renice->setText(i18np("Renice Process...", "Renice Processes...", numSelected));
	d->kill->setText(i18np("Kill Process", "Kill Processes", numSelected));
}
void KSysGuardProcessList::showProcessContextMenu(const QModelIndex &index) {
	if(!index.isValid()) return;
	QRect rect = d->mUi->treeView->visualRect(index);
	QPoint point(rect.x() + rect.width()/4, rect.y() + rect.height()/2 );
	showProcessContextMenu(point);
}
void KSysGuardProcessList::showProcessContextMenu(const QPoint &point) {
	d->mProcessContextMenu->clear();

	QModelIndexList selectedIndexes = d->mUi->treeView->selectionModel()->selectedRows();
	int numProcesses = selectedIndexes.size();

	if(numProcesses == 0) return;  //No processes selected, so no context menu

	QModelIndex realIndex = d->mFilterModel.mapToSource(selectedIndexes.at(0));
	KSysGuard::Process *process = reinterpret_cast<KSysGuard::Process *> (realIndex.internalPointer());



	//If the selected process is a zombie, do not bother offering renice and kill options
	bool showSignalingEntries = numProcesses != 1 || process->status != KSysGuard::Process::Zombie;
	if(showSignalingEntries) {
		d->mProcessContextMenu->addAction(d->renice);
		QMenu *signalMenu = d->mProcessContextMenu->addMenu(i18n("Send Signal"));
		signalMenu->addAction(d->sigStop);
		signalMenu->addAction(d->sigCont);
		signalMenu->addAction(d->sigHup);
		signalMenu->addAction(d->sigInt);
		signalMenu->addAction(d->sigTerm);
		signalMenu->addAction(d->sigKill);
		signalMenu->addAction(d->sigUsr1);
		signalMenu->addAction(d->sigUsr2);
	}

	if(numProcesses == 1 && process->parent_pid > 1) {
		//As a design decision, I do not show the 'Jump to parent process' option when the
		//parent is just 'init'.

	        KSysGuard::Process *parent_process = d->mModel.getProcess(process->parent_pid);
		if(parent_process) { //it should not be possible for this process to not exist, but check just incase
			d->selectParent->setText(i18n("Jump to Parent Process (%1)", parent_process->name));
			d->mProcessContextMenu->addAction(d->selectParent);
		}
	}

	if(numProcesses == 1 && process->tracerpid > 0) {
		//If the process is being debugged, offer to select it
		d->mProcessContextMenu->addAction(d->selectTracer);
	}

	if (numProcesses == 1 && !d->mModel.data(realIndex, ProcessModel::WindowIdRole).isNull()) {
		d->mProcessContextMenu->addAction(d->window);
	}
	if (d->monitorio && numProcesses == 1 && d->mModel.isLocalhost() && (process->uid==0 || process->uid == getuid()) && process->pid != getpid() && process->pid != getppid()) { //Don't attach to ourselves - crashes
		d->mProcessContextMenu->addAction(d->monitorio);
	}

	if(numProcesses == 1 && process->status == KSysGuard::Process::Stopped) {
		//If the process is being debugged, offer to select it
		d->mProcessContextMenu->addAction(d->resume);
	}

	if (showSignalingEntries) {
		d->mProcessContextMenu->addSeparator();
		d->mProcessContextMenu->addAction(d->kill);
	}

	d->mProcessContextMenu->popup(d->mUi->treeView->viewport()->mapToGlobal(point));
}
void KSysGuardProcessList::actionTriggered(QObject *object) {
	//Reset the text back to normal
	d->selectParent->setText(i18n("Jump to Parent Process"));
	QAction *result = dynamic_cast<QAction *>(object);
	if(result == 0) {
		//Escape was pressed. Do nothing.
	} else if(result == d->renice) {
		reniceSelectedProcesses();
	} else if(result == d->kill) {
		killSelectedProcesses();
	} else if(result == d->selectParent) {
		QModelIndexList selectedIndexes = d->mUi->treeView->selectionModel()->selectedRows();
		int numProcesses = selectedIndexes.size();
		if(numProcesses == 0) return;  //No processes selected
		QModelIndex realIndex = d->mFilterModel.mapToSource(selectedIndexes.at(0));
		KSysGuard::Process *process = reinterpret_cast<KSysGuard::Process *> (realIndex.internalPointer());
		if(process)
			selectAndJumpToProcess(process->parent_pid);
	} else if(result == d->selectTracer) {
		QModelIndexList selectedIndexes = d->mUi->treeView->selectionModel()->selectedRows();
		int numProcesses = selectedIndexes.size();
		if(numProcesses == 0) return;  //No processes selected
		QModelIndex realIndex = d->mFilterModel.mapToSource(selectedIndexes.at(0));
		KSysGuard::Process *process = reinterpret_cast<KSysGuard::Process *> (realIndex.internalPointer());
		if(process)
			selectAndJumpToProcess(process->tracerpid);
	} else if(result == d->window) {
		QModelIndexList selectedIndexes = d->mUi->treeView->selectionModel()->selectedRows();
		int numProcesses = selectedIndexes.size();
		if(numProcesses == 0) return;  //No processes selected
		foreach( const QModelIndex &index, selectedIndexes) {
			QModelIndex realIndex = d->mFilterModel.mapToSource(index);
			QVariant widVar= d->mModel.data(realIndex, ProcessModel::WindowIdRole);
			if( !widVar.isNull() ) {
				int wid = widVar.toInt();
				KWindowSystem::activateWindow(wid);
			}
		}
#ifdef WITH_MONITOR_PROCESS_IO
	} else if(result == d->monitorio) {
		QModelIndexList selectedIndexes = d->mUi->treeView->selectionModel()->selectedRows();
		if(selectedIndexes.isEmpty()) return;  //No processes selected
		QModelIndex realIndex = d->mFilterModel.mapToSource(selectedIndexes.at(0));
		KSysGuard::Process *process = reinterpret_cast<KSysGuard::Process *> (realIndex.internalPointer());
		if(process) {
			DisplayProcessDlg *dialog = new DisplayProcessDlg( this,process );
			dialog->show();
		}
#endif
	} else {
		QList< long long > pidlist;
		QModelIndexList selectedIndexes = d->mUi->treeView->selectionModel()->selectedRows();
		int numProcesses = selectedIndexes.size();
		if(numProcesses == 0) return;  //No processes selected
		foreach( const QModelIndex &index, selectedIndexes) {
			QModelIndex realIndex = d->mFilterModel.mapToSource(index);
			KSysGuard::Process *process = reinterpret_cast<KSysGuard::Process *> (realIndex.internalPointer());
			if(process)
				pidlist << process->pid;
		}
		if(result == d->resume || result == d->sigCont)
			killProcesses(pidlist, SIGCONT);  //Despite the function name, this sends a signal, rather than kill it.  Silly unix :)
		else if(result == d->sigStop)
			killProcesses(pidlist, SIGSTOP);
		else if(result == d->sigHup)
			killProcesses(pidlist, SIGHUP);
		else if(result == d->sigInt)
			killProcesses(pidlist, SIGINT);
		else if(result == d->sigTerm)
			killProcesses(pidlist, SIGTERM);
		else if(result == d->sigKill)
			killProcesses(pidlist, SIGKILL);
		else if(result == d->sigUsr1)
			killProcesses(pidlist, SIGUSR1);
		else if(result == d->sigUsr2)
			killProcesses(pidlist, SIGUSR2);
		updateList();
	}
}

void KSysGuardProcessList::selectAndJumpToProcess(int pid) {
	KSysGuard::Process *process = d->mModel.getProcess(pid);
	if(!process) return;
	QModelIndex filterIndex = d->mFilterModel.mapFromSource( d->mModel.getQModelIndex(process, 0));
	d->mUi->treeView->clearSelection();
	d->mUi->treeView->setCurrentIndex(filterIndex);
	d->mUi->treeView->scrollTo( filterIndex, QAbstractItemView::PositionAtCenter);

}

void KSysGuardProcessList::showColumnContextMenu(const QPoint &point){
	QMenu *menu = new QMenu();

	QAction *action;
	int index = d->mUi->treeView->header()->logicalIndexAt(point);
	if(index >= 0) {
		//selected a column.  Give the option to hide it
		action = new QAction(menu);
		action->setData(-index-1); //We set data to be negative (and minus 1) to hide a column, and positive to show a column
		action->setText(i18n("Hide Column '%1'", d->mFilterModel.headerData(index, Qt::Horizontal, Qt::DisplayRole).toString()));
		menu->addAction(action);
		if(d->mUi->treeView->header()->sectionsHidden()) {
			menu->addSeparator();
		}
	}


	if(d->mUi->treeView->header()->sectionsHidden()) {
		int num_headings = d->mFilterModel.columnCount();
		for(int i = 0; i < num_headings; ++i) {
			if(d->mUi->treeView->header()->isSectionHidden(i)) {
				action = new QAction(menu);
				action->setText(i18n("Show Column '%1'", d->mFilterModel.headerData(i, Qt::Horizontal, Qt::DisplayRole).toString()));
				action->setData(i); //We set data to be negative (and minus 1) to hide a column, and positive to show a column
				menu->addAction(action);
			}
		}
	}
	QAction *actionKB = NULL;
	QAction *actionMB = NULL;
	QAction *actionGB = NULL;
	QAction *actionPercentage = NULL;
	QAction *actionShowCmdlineOptions = NULL;
	QAction *actionShowTooltips = NULL;
	QAction *actionNormalizeCPUUsage = NULL;
	

	if( index == ProcessModel::HeadingVmSize || index == ProcessModel::HeadingMemory ||  index == ProcessModel::HeadingSharedMemory) {
		//If the user right clicks on a column that contains a memory size, show a toggle option for displaying
		//the memory in different units.  e.g.  "2000 k" or "2 m"
		menu->addSeparator()->setText(i18n("Display Units"));
		QActionGroup *unitsGroup = new QActionGroup(menu);
		actionKB = new QAction(menu);
		actionKB->setText(i18n("Kilobytes"));
		actionKB->setCheckable(true);
		menu->addAction(actionKB);
		unitsGroup->addAction(actionKB);
		actionMB = new QAction(menu);
		actionMB->setText(i18n("Megabytes"));
		actionMB->setCheckable(true);
		menu->addAction(actionMB);
		unitsGroup->addAction(actionMB);
		actionGB = new QAction(menu);
		actionGB->setText(i18n("Gigabytes"));
		actionGB->setCheckable(true);
		menu->addAction(actionGB);
		unitsGroup->addAction(actionGB);
		actionPercentage = new QAction(menu);
		actionPercentage->setText(i18n("Percentage"));
		actionPercentage->setCheckable(true);
		menu->addAction(actionPercentage);
		unitsGroup->addAction(actionPercentage);

		unitsGroup->setExclusive(true);
		switch(d->mModel.units()) {
		  case ProcessModel::UnitsKB:
			actionKB->setChecked(true);
			break;
		  case ProcessModel::UnitsMB:
			actionMB->setChecked(true);
			break;
		  case ProcessModel::UnitsGB:
			actionGB->setChecked(true);
			break;
		  case ProcessModel::UnitsPercentage:
			actionPercentage->setChecked(true);
			break;
		}
	} else if(index == ProcessModel::HeadingName) {
		menu->addSeparator();
		actionShowCmdlineOptions = new QAction(menu);
		actionShowCmdlineOptions->setText(i18n("Display command line options"));
		actionShowCmdlineOptions->setCheckable(true);
		actionShowCmdlineOptions->setChecked(d->mModel.isShowCommandLineOptions());
		menu->addAction(actionShowCmdlineOptions);
	} else if(index == ProcessModel::HeadingCPUUsage) {
		menu->addSeparator();
		actionNormalizeCPUUsage = new QAction(menu);
		actionNormalizeCPUUsage->setText(i18n("Divide CPU usage by number of CPUs"));
		actionNormalizeCPUUsage->setCheckable(true);
		actionNormalizeCPUUsage->setChecked(d->mModel.isNormalizedCPUUsage());
		menu->addAction(actionNormalizeCPUUsage);
	}
	menu->addSeparator();
	actionShowTooltips = new QAction(menu);
	actionShowTooltips->setCheckable(true);
	actionShowTooltips->setChecked(d->mModel.isShowingTooltips());
	actionShowTooltips->setText(i18n("Show Tooltips"));
	menu->addAction(actionShowTooltips);


	QAction *result = menu->exec(d->mUi->treeView->header()->mapToGlobal(point));
	if(!result) return; //Menu cancelled
	if(result == actionKB) {
		d->mModel.setUnits(ProcessModel::UnitsKB);
		return;
	} else if(result == actionMB) {
		d->mModel.setUnits(ProcessModel::UnitsMB);
		return;
	} else if(result == actionGB) {
		d->mModel.setUnits(ProcessModel::UnitsGB);
		return;
	} else if(result == actionPercentage) {
		d->mModel.setUnits(ProcessModel::UnitsPercentage);
		return;
	} else if(result == actionShowCmdlineOptions) {
		d->mModel.setShowCommandLineOptions(actionShowCmdlineOptions->isChecked());
		return;
	} else if(result == actionNormalizeCPUUsage) {
		d->mModel.setNormalizedCPUUsage(actionNormalizeCPUUsage->isChecked());
		return;
	} else if(result == actionShowTooltips) {
		d->mModel.setShowingTooltips(actionShowTooltips->isChecked());
		return;
	}

	int i = result->data().toInt();
	//We set data to be negative to hide a column, and positive to show a column
	if(i < 0)
		d->mUi->treeView->hideColumn(-1-i);
	else {
		d->mUi->treeView->showColumn(i);
		d->mUi->treeView->resizeColumnToContents(i);
		d->mUi->treeView->resizeColumnToContents(d->mFilterModel.columnCount());
	}
	menu->deleteLater();
}

void KSysGuardProcessList::expandAllChildren(const QModelIndex &parent)
{
	//This is called when the user expands a node.  This then expands all of its
	//children.  This will trigger this function again recursively.
	QModelIndex sourceParent = d->mFilterModel.mapToSource(parent);
	for(int i = 0; i < d->mModel.rowCount(sourceParent); i++) {
		d->mUi->treeView->expand(d->mFilterModel.mapFromSource(d->mModel.index(i,0, sourceParent)));
	}
}

void KSysGuardProcessList::rowsInserted(const QModelIndex & parent, int start, int end )
{
	if(d->mModel.isSimpleMode()) return; //No tree - no need to expand init
	if(parent.isValid()) return; //Not a root node
	//It is a root node that we just inserted - expand it
	bool expanded = false;
	for(int i = start; i <= end; i++) {
		QModelIndex index = d->mFilterModel.index(i, 0, QModelIndex());
		if(!d->mUi->treeView->isExpanded(index)) {
			if(!expanded) {
				disconnect(d->mUi->treeView, SIGNAL(expanded(const QModelIndex &)), this, SLOT(expandAllChildren(const QModelIndex &)));
				expanded = true;
			}
			d->mUi->treeView->expand(index);
		}
	}
	if(expanded)
		connect(d->mUi->treeView, SIGNAL(expanded(const QModelIndex &)), this, SLOT(expandAllChildren(const QModelIndex &)));
}
void KSysGuardProcessList::expandInit()
{
	if(d->mModel.isSimpleMode()) return; //No tree - no need to expand init

	bool expanded = false;
	for(int i = 0; i < d->mFilterModel.rowCount(QModelIndex()); i++) {
		QModelIndex index = d->mFilterModel.index(i, 0, QModelIndex());
		if(!d->mUi->treeView->isExpanded(index)) {
			if(!expanded) {
				disconnect(d->mUi->treeView, SIGNAL(expanded(const QModelIndex &)), this, SLOT(expandAllChildren(const QModelIndex &)));
				expanded = true;
			}
			d->mUi->treeView->expand(index);
		}
	}
	if(expanded)
		connect(d->mUi->treeView, SIGNAL(expanded(const QModelIndex &)), this, SLOT(expandAllChildren(const QModelIndex &)));
}

void KSysGuardProcessList::hideEvent ( QHideEvent * event )  //virtual protected from QWidget
{
	//Stop updating the process list if we are hidden
	d->mUpdateTimer->stop();
	QWidget::hideEvent(event);
}

void KSysGuardProcessList::showEvent ( QShowEvent * event )  //virtual protected from QWidget
{
	//Start updating the process list again if we are shown again
	if(!d->mUpdateTimer->isActive()) {
		d->mUpdateTimer->start(d->mUpdateIntervalMSecs);
	}

	QWidget::showEvent(event);
}

void KSysGuardProcessList::changeEvent( QEvent * event )
{
	if (event->type() == QEvent::LanguageChange) {
		d->mModel.retranslateUi();
		d->mUi->retranslateUi(this);
		retranslateUi();
	}
	QWidget::changeEvent(event);
}

void KSysGuardProcessList::retranslateUi()
{
	d->mUi->cmbFilter->setItemIcon(ProcessFilter::AllProcesses, KIcon("view-process-all"));
	d->mUi->cmbFilter->setItemIcon(ProcessFilter::AllProcessesInTreeForm, KIcon("view-process-all-tree"));
	d->mUi->cmbFilter->setItemIcon(ProcessFilter::SystemProcesses, KIcon("view-process-system"));
	d->mUi->cmbFilter->setItemIcon(ProcessFilter::UserProcesses, KIcon("view-process-users"));
	d->mUi->cmbFilter->setItemIcon(ProcessFilter::OwnProcesses, KIcon("view-process-own"));
	d->mUi->cmbFilter->setItemIcon(ProcessFilter::ProgramsOnly, KIcon("view-process-all"));
}

void KSysGuardProcessList::updateList()
{
	if(isVisible()) {
		d->mModel.update(d->mUpdateIntervalMSecs);
		d->mUpdateTimer->start(d->mUpdateIntervalMSecs);
		emit updated();
	}
}

int KSysGuardProcessList::updateIntervalMSecs() const
{
	return d->mUpdateIntervalMSecs;
}

void KSysGuardProcessList::setUpdateIntervalMSecs(int intervalMSecs)
{
	d->mUpdateIntervalMSecs = intervalMSecs;
	d->mUpdateTimer->setInterval(d->mUpdateIntervalMSecs);
}

bool KSysGuardProcessList::reniceProcesses(const QList<long long> &pids, int niceValue)
{
	QList< long long> unreniced_pids;
        for (int i = 0; i < pids.size(); ++i) {
		bool success = d->mModel.processController()->setNiceness(pids.at(i), niceValue);
		if(!success) {
			unreniced_pids << pids.at(i);
		}
	}
	if(unreniced_pids.isEmpty()) return true; //All processes were reniced successfully
	if(!d->mModel.isLocalhost()) return false; //We can't use kdesu to renice non-localhost processes

	QStringList arguments;
	arguments << "--attach" << QString::number(window()->winId()) << "--noignorebutton";
	arguments << "--" << "renice" << QString::number(niceValue);

        for (int i = 0; i < unreniced_pids.size(); ++i) {
		arguments << QString::number(unreniced_pids.at(i));
	}

	QString su = KStandardDirs::findExe("kdesu");  //kdesu is a libexec program, so it will not be in the path.  findExe will find it correctly anyway
	if(su.isEmpty()) return false;  //Cannot find kdesu

	QProcess *reniceProcess = new QProcess(NULL);
	connect(reniceProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(reniceFailed()));
	connect(reniceProcess, SIGNAL(finished( int, QProcess::ExitStatus) ), this, SLOT(updateList()));
	reniceProcess->start(su, arguments);
	return true; //No way to tell if it was successful :(
}

QList<KSysGuard::Process *> KSysGuardProcessList::selectedProcesses() const
{
	QList<KSysGuard::Process *> processes;
	QModelIndexList selectedIndexes = d->mUi->treeView->selectionModel()->selectedRows();
	for(int i = 0; i < selectedIndexes.size(); ++i) {
		KSysGuard::Process *process = reinterpret_cast<KSysGuard::Process *> (d->mFilterModel.mapToSource(selectedIndexes.at(i)).internalPointer());
		processes << process;
	}
	return processes;

}

void KSysGuardProcessList::reniceSelectedProcesses()
{
	QList<KSysGuard::Process *> processes = selectedProcesses();
	QStringList selectedAsStrings;

	if (processes.isEmpty())
	{
		KMessageBox::sorry(this, i18n("You must select a process first."));
		return;
	}

	int sched = -2;
	int iosched = -2;
	foreach(KSysGuard::Process *process, processes) {
		selectedAsStrings << d->mModel.getStringForProcess(process);
		if(sched == -2) sched = (int)process->scheduler;
		else if(sched != -1 && sched != (int)process->scheduler) sched = -1;  //If two processes have different schedulers, disable the cpu scheduler stuff
		if(iosched == -2) iosched = (int)process->ioPriorityClass;
		else if(iosched != -1 && iosched != (int)process->ioPriorityClass) iosched = -1;  //If two processes have different schedulers, disable the cpu scheduler stuff

	}

	int firstPriority = processes.first()->niceLevel;
	int firstIOPriority = processes.first()->ioniceLevel;

	bool supportsIoNice = d->mModel.processController()->supportsIoNiceness();
	if(!supportsIoNice) { iosched = -2; firstIOPriority = -2; }
	ReniceDlg reniceDlg(d->mUi->treeView, selectedAsStrings, firstPriority, sched, firstIOPriority, iosched);
	if(reniceDlg.exec() == QDialog::Rejected) return;

	QList<long long> renicePids;
	QList<long long> changeCPUSchedulerPids;
	QList<long long> changeIOSchedulerPids;
	foreach(KSysGuard::Process *process, processes) {
		switch(reniceDlg.newCPUSched) {
			case -2:
			case -1:  //Invalid, not changed etc.
				break;  //So do nothing
			case KSysGuard::Process::Other:
			case KSysGuard::Process::Fifo:
				if(reniceDlg.newCPUSched != (int)process->scheduler) {
					changeCPUSchedulerPids << process->pid;
					renicePids << process->pid;
				} else if(reniceDlg.newCPUPriority != process->niceLevel)
					renicePids << process->pid;
				break;

			case KSysGuard::Process::RoundRobin:
			case KSysGuard::Process::Batch:
				if(reniceDlg.newCPUSched != (int)process->scheduler || reniceDlg.newCPUPriority != process->niceLevel) {
					changeCPUSchedulerPids << process->pid;
				}
				break;
		}
		switch(reniceDlg.newIOSched) {
			case -2:
			case -1:  //Invalid, not changed etc.
				break;  //So do nothing
			case KSysGuard::Process::None:
				if(reniceDlg.newIOSched != (int)process->ioPriorityClass) {
					// Unfortunately linux doesn't actually let us set the ioniceness back to none after being set to something else
					if(process->ioPriorityClass != KSysGuard::Process::BestEffort || reniceDlg.newIOPriority != process->ioniceLevel)
						changeIOSchedulerPids << process->pid;
				}
				break;
			case KSysGuard::Process::Idle:
				if(reniceDlg.newIOSched != (int)process->ioPriorityClass) {
					changeIOSchedulerPids << process->pid;
				}
				break;
			case KSysGuard::Process::BestEffort:
				if(process->ioPriorityClass == KSysGuard::Process::None && reniceDlg.newIOPriority  == (process->niceLevel + 20)/5)
					break;  //Don't set to BestEffort if it's on None and the nicelevel wouldn't change
			case KSysGuard::Process::RealTime:
				if(reniceDlg.newIOSched != (int)process->ioPriorityClass || reniceDlg.newIOPriority != process->ioniceLevel) {
					changeIOSchedulerPids << process->pid;
				}
				break;
		}

	}
	if(!changeCPUSchedulerPids.isEmpty()) {
		Q_ASSERT(reniceDlg.newCPUSched >= 0);
		if(!changeCpuScheduler(changeCPUSchedulerPids, (KSysGuard::Process::Scheduler) reniceDlg.newCPUSched, reniceDlg.newCPUPriority)) {
			KMessageBox::sorry(this, i18n("You do not have sufficient privileges to change the CPU scheduler. Aborting."));
			return;
		}

	}
	if(!renicePids.isEmpty()) {
		Q_ASSERT(reniceDlg.newCPUPriority <= 20 && reniceDlg.newCPUPriority >= -20);
		if(!reniceProcesses(renicePids, reniceDlg.newCPUPriority)) {
			KMessageBox::sorry(this, i18n("You do not have sufficient privileges to change the CPU priority. Aborting"));
			return;
		}
	}
	if(!changeIOSchedulerPids.isEmpty()) {
		if(!changeIoScheduler(changeIOSchedulerPids, (KSysGuard::Process::IoPriorityClass) reniceDlg.newIOSched, reniceDlg.newIOPriority)) {
			KMessageBox::sorry(this, i18n("You do not have sufficient privileges to change the IO scheduler and priority. Aborting"));
			return;
		}
	}
	updateList();
}

bool KSysGuardProcessList::changeIoScheduler(const QList< long long> &pids, KSysGuard::Process::IoPriorityClass newIoSched, int newIoSchedPriority)
{
	if(newIoSched == KSysGuard::Process::None) newIoSched = KSysGuard::Process::BestEffort;
	if(newIoSched == KSysGuard::Process::Idle) newIoSchedPriority = 0;
	QList< long long> unchanged_pids;
        for (int i = 0; i < pids.size(); ++i) {
		bool success = d->mModel.processController()->setIoNiceness(pids.at(i), newIoSched, newIoSchedPriority);
		if(!success) {
			unchanged_pids << pids.at(i);
		}
	}
	if(unchanged_pids.isEmpty()) return true;
	if(!d->mModel.isLocalhost()) return false; //We can't use kdesu to kill non-localhost processes


	QString su = KStandardDirs::findExe("kdesu");
	if(su.isEmpty()) return false;  //Cannot find kdesu

	//We must use kdesu to kill the process

	QStringList arguments;
	arguments << "--attach" << QString::number(window()->winId()) << "--noignorebutton";
	if(unchanged_pids.size() == 1) {
		arguments << "--" << "ionice" << "-p" << QString::number(unchanged_pids.at(0)) << "-c";
		switch(newIoSched) {
		  case KSysGuard::Process::Idle:
			arguments << "3";
			break;
		  case KSysGuard::Process::BestEffort:
			arguments << "2" << "-n" << QString::number(newIoSchedPriority);
			break;
		  case KSysGuard::Process::RealTime:
			arguments << "1" << "-n" << QString::number(newIoSchedPriority);
			break;
		  default:
			Q_ASSERT(false);
			return false; //should never happen - wtf?
		}
	} else {
		//Cope with multiple pids by doing a for loop
		arguments << "--" << "sh" << "-c";
		QString sh("for f in ");

	        for (int i = 0; i < unchanged_pids.size(); ++i) {
			sh += QString::number(unchanged_pids.at(i)) + " ";
		}
		 sh += "; do ionice -p \"$f\" ";
		switch(newIoSched) {
		  case KSysGuard::Process::Idle:
			sh += "-c 3";
			break;
		  case KSysGuard::Process::BestEffort:
			sh += "-c 2 -n " + QString::number(newIoSchedPriority);
			break;
		  case KSysGuard::Process::RealTime:
			sh += "-c 1 -n " + QString::number(newIoSchedPriority);
			break;
		  default:
			Q_ASSERT(false);
			return false; //should never happen - wtf?
		}
		sh += "; done";

		arguments << sh;
	}

	QProcess *process = new QProcess(NULL);
	connect(process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(ioniceFailed()));
	connect(process, SIGNAL(finished( int, QProcess::ExitStatus) ), this, SLOT(updateList()));
	process->start(su, arguments);
	return true;  //assume it ran successfully :(  We cannot seem to actually check if it did.  There must be a better solution
}

bool KSysGuardProcessList::changeCpuScheduler(const QList< long long> &pids, KSysGuard::Process::Scheduler newCpuSched, int newCpuSchedPriority)
{
	if(newCpuSched == KSysGuard::Process::Other || newCpuSched == KSysGuard::Process::Batch) newCpuSchedPriority = 0;
	QList< long long> unchanged_pids;
        for (int i = 0; i < pids.size(); ++i) {
		bool success = d->mModel.processController()->setScheduler(pids.at(i), newCpuSched, newCpuSchedPriority);
		if(!success) {
			unchanged_pids << pids.at(i);
		}
	}
	if(unchanged_pids.isEmpty()) return true;
	if(!d->mModel.isLocalhost()) {
		KMessageBox::sorry(this, i18n("No."));
		return false; //We can't use kdesu to kill non-localhost processes
	}

	QString su = KStandardDirs::findExe("kdesu");
	if(su.isEmpty()) {
		KMessageBox::sorry(this, i18n("Could not find kdesu executable"));
		return false;  //Cannot find kdesu
	}
	QString setscheduler = KStandardDirs::findExe("setscheduler");
	if(setscheduler.isEmpty()) {
		KMessageBox::sorry(this, i18n("Could not find setscheduler executable.  This should have been installed alongside system monitor."));
		return false;
	}

	//We must use kdesu to kill the process

	QStringList arguments;
	arguments << "--attach" << QString::number(window()->winId()) << "--noignorebutton";
	if(unchanged_pids.size() == 1) {
		arguments << "--" << setscheduler << QString::number(unchanged_pids.at(0)) << QString::number((int)newCpuSched) << QString::number(newCpuSchedPriority);
	} else {
		//Cope with multiple pids by doing a for loop
		arguments << "--" << "sh" << "-c";
		QString sh("for f in ");

	        for (int i = 0; i < unchanged_pids.size(); ++i) {
			sh += QString::number(unchanged_pids.at(i)) + " ";
		}
		sh += "; do " + setscheduler + "\"$f\" " + newCpuSched + " " + newCpuSchedPriority;
		sh += "; done";

		arguments << sh;
	}

	QProcess *process = new QProcess(NULL);
	connect(process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(ioniceFailed()));
	connect(process, SIGNAL(finished( int, QProcess::ExitStatus) ), this, SLOT(updateList()));
	process->start(su, arguments);
	return true;  //assume it ran successfully :(  We cannot seem to actually check if it did.  There must be a better solution
}

bool KSysGuardProcessList::killProcesses(const QList< long long> &pids, int sig)
{
	QList< long long> unkilled_pids;
        for (int i = 0; i < pids.size(); ++i) {
		bool success = d->mModel.processController()->sendSignal(pids.at(i), sig);
		if(!success) {
			unkilled_pids << pids.at(i);
		}
	}
	if(unkilled_pids.isEmpty()) return true;
	if(!d->mModel.isLocalhost()) return false; //We can't use kdesu to kill non-localhost processes

	QString su = KStandardDirs::findExe("kdesu");
	if(su.isEmpty()) return false;  //Cannot find kdesu

	kDebug() << "running " << su;
	//We must use kdesu to kill the process
	QStringList arguments;
	arguments << "--attach" << QString::number(window()->winId()) << "--noignorebutton";
	arguments << "--" << "kill";
	if(sig != SIGTERM) {
		arguments << ('-' + QString::number(sig));
	}

        for (int i = 0; i < unkilled_pids.size(); ++i) {
		arguments << QString::number(unkilled_pids.at(i));
	}

	QProcess *killProcess = new QProcess(NULL);
	connect(killProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(killFailed()));
	connect(killProcess, SIGNAL(finished( int, QProcess::ExitStatus) ), this, SLOT(updateList()));
	killProcess->start(su, arguments);
	return true;  //assume it ran successfully :(  We cannot seem to actually check if it did.  There must be a better solution

}
void KSysGuardProcessList::killSelectedProcesses()
{
	QModelIndexList selectedIndexes = d->mUi->treeView->selectionModel()->selectedRows();
	QStringList selectedAsStrings;
	QList< long long> selectedPids;

	QList<KSysGuard::Process *> processes = selectedProcesses();
	foreach(KSysGuard::Process *process, processes) {
		selectedPids << process->pid;
		selectedAsStrings << d->mModel.getStringForProcess(process);
	}

	if (selectedAsStrings.isEmpty())
	{
		KMessageBox::sorry(this, i18n("You must select a process first."));
		return;
	}
	else
	{
		int count = selectedAsStrings.count();
                QString  msg = i18np("Are you sure you want to kill this process?",
				"Are you sure you want to kill these %1 processes?",
				count);

		int res = KMessageBox::warningContinueCancelList(this, msg, selectedAsStrings,
								 i18np("Kill process", "Kill %1 processes", count),
								 KGuiItem(i18n("Kill"), "edit-bomb"),
								 KStandardGuiItem::cancel(),
								 "killconfirmation");
		if (res != KMessageBox::Continue)
		{
			return;
		}
	}


	Q_ASSERT(selectedPids.size() == selectedAsStrings.size());
	if(!killProcesses(selectedPids, SIGTERM)) return;
	foreach(KSysGuard::Process *process, processes) {
		process->timeKillWasSent.start();
	}
	updateList();
}

void KSysGuardProcessList::reniceFailed()
{
	KMessageBox::sorry(this, i18n("You do not have the permission to renice the process and there "
					"was a problem trying to run as root"));
}
void KSysGuardProcessList::killFailed()
{
	KMessageBox::sorry(this, i18n("You do not have the permission to kill the process and there "
					"was a problem trying to run as root"));
}
void KSysGuardProcessList::ioniceFailed()
{
	KMessageBox::sorry(this, i18n("You do not have the permission to set the I/O priority and there "
					"was a problem trying to run as root"));
}
bool KSysGuardProcessList::showTotals() const {
	return d->mModel.showTotals();
}

void KSysGuardProcessList::setShowTotals(bool showTotals)  //slot
{
	d->mModel.setShowTotals(showTotals);
}

ProcessModel::Units KSysGuardProcessList::units() const {
	return d->mModel.units();
}

void KSysGuardProcessList::setUnits(ProcessModel::Units unit) {
	d->mModel.setUnits(unit);
}

void KSysGuardProcessList::saveSettings(KConfigGroup &cg) {
	/* Note that the ksysguard program does not use these functions.  It saves the settings itself to an xml file instead */
	cg.writeEntry("units", (int)(units()));
	cg.writeEntry("showCommandLineOptions", d->mModel.isShowCommandLineOptions());
	cg.writeEntry("normalizeCPUUsage", d->mModel.isNormalizedCPUUsage());
	cg.writeEntry("showTooltips", d->mModel.isShowingTooltips());
	cg.writeEntry("showTotals", showTotals());
	cg.writeEntry("filterState", (int)(state()));
	cg.writeEntry("updateIntervalMSecs", updateIntervalMSecs());
	cg.writeEntry("headerState", d->mUi->treeView->header()->saveState());
	//If we change, say, the header between versions of ksysguard, then the old headerState settings will not be valid.
	//The version property lets us keep track of which version we are
	cg.writeEntry("version", PROCESSHEADERVERSION);
}

void KSysGuardProcessList::loadSettings(const KConfigGroup &cg) {
	/* Note that the ksysguard program does not use these functions.  It saves the settings itself to an xml file instead */
	setUnits((ProcessModel::Units) cg.readEntry("units", (int)ProcessModel::UnitsKB));
	d->mModel.setShowCommandLineOptions(cg.readEntry("showCommandLineOptions", false));
	d->mModel.setNormalizedCPUUsage(cg.readEntry("normalizeCPUUsage", true));
	d->mModel.setShowingTooltips(cg.readEntry("showTooltips", true));
	setShowTotals(cg.readEntry("showTotals", true));
	setStateInt(cg.readEntry("filterState", (int)ProcessFilter::AllProcesses));
	setUpdateIntervalMSecs(cg.readEntry("updateIntervalMSecs", 2000));
	int version = cg.readEntry("version", 0);
	if(version == PROCESSHEADERVERSION) {  //If the header has changed, the old settings are no longer valid.  Only restore if version is the same
		restoreHeaderState(cg.readEntry("headerState", QByteArray()));
	}
}

void KSysGuardProcessList::restoreHeaderState(const QByteArray & state) {
	d->mUi->treeView->header()->restoreState(state);
	d->mFilterModel.sort( d->mUi->treeView->header()->sortIndicatorSection(), d->mUi->treeView->header()->sortIndicatorOrder() );
}

bool KSysGuardProcessList::eventFilter(QObject *obj, QEvent *event) {
	if (event->type() == QEvent::KeyPress) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		if(obj == d->mUi->treeView) {
			if(  keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
				d->mUi->treeView->selectionModel()->select(d->mUi->treeView->currentIndex(), QItemSelectionModel::Select | QItemSelectionModel::Rows);
				showProcessContextMenu(d->mUi->treeView->currentIndex());
			}
		} else {
			// obj must be txtFilter
			if(keyEvent->matches(QKeySequence::MoveToNextLine) || keyEvent->matches(QKeySequence::SelectNextLine) ||
			   keyEvent->matches(QKeySequence::MoveToPreviousLine) || keyEvent->matches(QKeySequence::SelectPreviousLine) ||
			   keyEvent->matches(QKeySequence::MoveToNextPage) ||  keyEvent->matches(QKeySequence::SelectNextPage) ||
			   keyEvent->matches(QKeySequence::MoveToPreviousPage) ||  keyEvent->matches(QKeySequence::SelectPreviousPage) ||
			   keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return)
			{
				QApplication::sendEvent(d->mUi->treeView, event);
				return true;
			}
		}
	}
	return false;
}

ProcessModel *KSysGuardProcessList::processModel() {
	return &d->mModel;
}

void KSysGuardProcessList::setKillButtonVisible(bool visible)
{
	d->mUi->btnKillProcess->setVisible(visible);
}

bool KSysGuardProcessList::isKillButtonVisible() const
{
	return d->mUi->btnKillProcess->isVisible();
}
