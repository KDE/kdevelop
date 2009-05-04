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


/* For getuid() */
#include <unistd.h>
#include <sys/types.h>

#include <QVariant>

#include <kdebug.h>

#include "ProcessModel.h"
#include "ProcessModel_p.h"
#include "ProcessFilter.h"

bool ProcessFilter::filterAcceptsRow( int source_row, const QModelIndex & source_parent ) const
{
	if( (mFilter == AllProcesses || mFilter == AllProcessesInTreeForm)
			&& filterRegExp().isEmpty()) return true; //Shortcut for common case

	ProcessModel *model = static_cast<ProcessModel *>(sourceModel());
	const KSysGuard::Process *process;
        if(model->isSimpleMode()) {
		if(source_parent.isValid()) {
			kDebug() << "Serious error with data.  In simple mode, there should be no children";
			return true;
		}
		process = model->getProcessAtIndex(source_row);
	} else {
		KSysGuard::Process *parent_process = NULL;
		if(source_parent.isValid()) {
			parent_process = reinterpret_cast<KSysGuard::Process *>(source_parent.internalPointer());
		       	Q_ASSERT(parent_process);
		} else {
			//if(!model->isSimpleMode()) {
				parent_process = model->getProcess(0); //Get our 'special' process which should have the root init child
		        	Q_ASSERT(parent_process);
			//}
		}
		if(!model->isSimpleMode() && source_row >= parent_process->children.size()) {
			kDebug() << "Serious error with data.  Source row requested for a non existent row. Requested " << source_row << " of " << parent_process->children.size() << " for " << parent_process->pid;
			return true;
		}

		process = parent_process->children.at(source_row);
	}
	Q_ASSERT(process);
	long uid = process->uid;
	long euid = process->euid;

	bool accepted = true;
	switch(mFilter) {
	case AllProcesses:
	case AllProcessesInTreeForm:
		break;
        case SystemProcesses:
                if( uid >= 100 && model->canUserLogin(uid))
			accepted = false;
		break;
        case UserProcesses:
		if( (uid < 100 || !model->canUserLogin(uid)) && (euid < 100 || !model->canUserLogin(euid)))
			accepted = false;
		break;
        case OwnProcesses: {
		long ownuid = getuid();
                if(uid != ownuid && process->suid != ownuid && process->fsuid != ownuid && euid != ownuid)
			accepted = false;
		break;
	}
	case ProgramsOnly:
		if(process->tty.isEmpty()) {
			if(!model->hasGUIWindow(process->pid))
				accepted = false;
		} else {
			// login and getty kinda _are_ the tty, so I do not really count them as 'programs'. So make a special case and hide them
			// Their ppid are 1 (init) so by checking we try to avoid false matches, and speed up checking overall
			QString name = process->name.section(' ', 0,0);
			if(process->parent_pid == 1 && (name == "login" || name.endsWith("getty")))
				accepted = false;
		}
		break;
        default:
		break;
        }

	if(accepted) {
		if(filterRegExp().isEmpty()) return true;

		//Allow the user to search by PID
		if(QString::number(process->pid).contains(filterRegExp())) return true;

		//None of our tests have rejected it.  Pass it on to qsortfilterproxymodel's filter
		if(QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent))
			return true;
	}


	//We did not accept this row at all.

	//If we are in flat mode, then give up now
	if(mFilter != AllProcessesInTreeForm)
		return false;

	//one of our children might be accepted, so accept this row if our children are accepted.
	QModelIndex source_index = sourceModel()->index(source_row, 0, source_parent);
	for(int i = 0 ; i < sourceModel()->rowCount(source_index); i++) {
		if(filterAcceptsRow(i, source_index)) return true;
	}
	return false;
}

bool ProcessFilter::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
	if(right.isValid() && left.isValid()) {
		Q_ASSERT(left.model());
		Q_ASSERT(right.model());
		QVariant l = (left.model() ? left.model()->data(left, ProcessModel::SortingValueRole) : QVariant());
		QVariant r = (right.model() ? right.model()->data(right, ProcessModel::SortingValueRole) : QVariant());
		if(l.isValid() && r.isValid())
			return l.toDouble() < r.toDouble();
	}
	return QSortFilterProxyModel::lessThan(left,right);
}


void ProcessFilter::setFilter(State filter) {
	mFilter = filter;
	filterChanged();//Tell the proxy view to refresh all its information
}
#include "ProcessFilter.moc"
