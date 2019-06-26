/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "vcseventmodel.h"

#include <QModelIndex>
#include <QVariant>
#include <QDateTime>
#include <QList>
#include <QLocale>

#include <KLocalizedString>

#include "../vcsevent.h"
#include "../vcsrevision.h"
#include <vcsjob.h>
#include <interfaces/ibasicversioncontrol.h>
#include <interfaces/icore.h>
#include <interfaces/iruncontroller.h>

namespace KDevelop
{

class VcsBasicEventModelPrivate
{
public:
    QList<KDevelop::VcsEvent> m_events;
};

VcsBasicEventModel::VcsBasicEventModel(QObject* parent)
    : QAbstractTableModel(parent)
    , d_ptr(new VcsBasicEventModelPrivate)
{
}

VcsBasicEventModel::~VcsBasicEventModel() = default;

int VcsBasicEventModel::rowCount(const QModelIndex& parent) const
{
    Q_D(const VcsBasicEventModel);

    return parent.isValid() ? 0 : d->m_events.count();
}

int VcsBasicEventModel::columnCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : ColumnCount;
}

QVariant VcsBasicEventModel::data(const QModelIndex& idx, int role) const
{
    Q_D(const VcsBasicEventModel);

    if( !idx.isValid() || role != Qt::DisplayRole )
        return QVariant();

    if( idx.row() < 0 || idx.row() >= rowCount() || idx.column() < 0 || idx.column() >= columnCount() )
        return QVariant();

    KDevelop::VcsEvent ev = d->m_events.at( idx.row() );
    switch( idx.column() )
    {
        case RevisionColumn:
            return QVariant( ev.revision().revisionValue() );
        case SummaryColumn:
            // show the first line only
            return QVariant( ev.message().section(QLatin1Char('\n'), 0, 0) );
        case AuthorColumn:
            return QVariant( ev.author() );
        case DateColumn:
            return QVariant( QLocale().toString( ev.date() ) );
        default:
            break;
    }
    return QVariant();
}

QVariant VcsBasicEventModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( section < 0 || section >= columnCount() || orientation != Qt::Horizontal || role != Qt::DisplayRole )
        return QVariant();
    switch( section )
    {
        case RevisionColumn:
            return QVariant( i18n("Revision") );
        case SummaryColumn:
            return QVariant( i18n("Message") );
        case AuthorColumn:
            return QVariant( i18n("Author") );
        case DateColumn:
            return QVariant( i18n("Date") );
        default:
            break;
    }
    return QVariant();
}

void VcsBasicEventModel::addEvents(const QList<KDevelop::VcsEvent>& list)
{
    Q_D(VcsBasicEventModel);

    if( list.isEmpty() )
        return;

    beginInsertRows( QModelIndex(), rowCount(), rowCount()+list.count()-1 );
    d->m_events += list;
    endInsertRows();
}

KDevelop::VcsEvent VcsBasicEventModel::eventForIndex(const QModelIndex& idx) const
{
    Q_D(const VcsBasicEventModel);

    if( !idx.isValid() || idx.row() < 0 || idx.row() >= rowCount() )
    {
        return KDevelop::VcsEvent();
    }
    return d->m_events.at( idx.row() );
}

class VcsEventLogModelPrivate
{
public:
    KDevelop::IBasicVersionControl* m_iface;
    VcsRevision m_rev;
    QUrl m_url;
    bool done;
    bool fetching;
};

VcsEventLogModel::VcsEventLogModel(KDevelop::IBasicVersionControl* iface, const VcsRevision& rev, const QUrl& url, QObject* parent)
    : KDevelop::VcsBasicEventModel(parent)
    , d_ptr(new VcsEventLogModelPrivate)
{
    Q_D(VcsEventLogModel);

    d->m_iface = iface;
    d->m_rev = rev;
    d->m_url = url;
    d->done = false;
    d->fetching = false;
}

VcsEventLogModel::~VcsEventLogModel() = default;

bool VcsEventLogModel::canFetchMore(const QModelIndex& parent) const
{
    Q_D(const VcsEventLogModel);

    return !d->done && !d->fetching && !parent.isValid();
}

void VcsEventLogModel::fetchMore(const QModelIndex& parent)
{
    Q_D(VcsEventLogModel);

    d->fetching = true;
    Q_ASSERT(!parent.isValid());
    Q_UNUSED(parent);
    VcsJob* job = d->m_iface->log(d->m_url, d->m_rev, qMax(rowCount(), 100));
    connect(this, &VcsEventLogModel::destroyed, job, [job] { job->kill(); });
    connect(job, &VcsJob::finished, this, &VcsEventLogModel::jobReceivedResults);
    ICore::self()->runController()->registerJob( job );
}

void VcsEventLogModel::jobReceivedResults(KJob* job)
{
    Q_D(VcsEventLogModel);

    const QList<QVariant> l = qobject_cast<KDevelop::VcsJob *>(job)->fetchResults().toList();
    if(l.isEmpty() || job->error()!=0) {
        d->done = true;
        return;
    }
    QList<KDevelop::VcsEvent> newevents;
    for (const QVariant& v : l) {
        if( v.canConvert<KDevelop::VcsEvent>() )
        {
            newevents << v.value<KDevelop::VcsEvent>();
        }
    }
    d->m_rev = newevents.last().revision();
    if (rowCount()) {
        newevents.removeFirst();
    }
    d->done = newevents.isEmpty();
    addEvents( newevents );
    d->fetching = false;
}

}
