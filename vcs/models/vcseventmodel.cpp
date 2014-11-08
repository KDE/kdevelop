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

class VcsEventModelPrivate
{
public:
    QList<KDevelop::VcsEvent> m_events;
    KDevelop::IBasicVersionControl* m_iface;
    VcsRevision m_rev;
    QUrl m_url;
    bool done;
    bool fetching;
};

VcsEventModel::VcsEventModel( KDevelop::IBasicVersionControl* iface, const VcsRevision& rev, const QUrl& url, QObject* parent )
    : QAbstractTableModel( parent ), d(new VcsEventModelPrivate)
{
    d->m_iface = iface;
    d->m_rev = rev;
    d->m_url = url;
    d->done = false;
    d->fetching = false;
}

VcsEventModel::~VcsEventModel()
{
    delete d;
}

int VcsEventModel::rowCount( const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : d->m_events.count();
}

int VcsEventModel::columnCount( const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : ColumnCount;
}

QVariant VcsEventModel::data( const QModelIndex& idx, int role ) const
{
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
            return QVariant( ev.message().section('\n', 0, 0) );
        case AuthorColumn:
            return QVariant( ev.author() );
        case DateColumn:
            return QVariant( QLocale().toString( ev.date() ) );
        default:
            break;
    }
    return QVariant();
}

QVariant VcsEventModel::headerData( int section, Qt::Orientation orientation, int role ) const
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

void VcsEventModel::addEvents( const QList<KDevelop::VcsEvent>& list )
{
    if( list.isEmpty() )
        return;

    beginInsertRows( QModelIndex(), rowCount(), rowCount()+list.count()-1 );
    d->m_events += list;
    endInsertRows();
}

KDevelop::VcsEvent VcsEventModel::eventForIndex( const QModelIndex& idx ) const
{
    if( !idx.isValid() || idx.row() < 0 || idx.row() >= rowCount() )
    {
        return KDevelop::VcsEvent();
    }
    return d->m_events.at( idx.row() );
}

bool VcsEventModel::canFetchMore(const QModelIndex& parent) const
{
    return !d->done && !d->fetching && !parent.isValid();
}

void VcsEventModel::fetchMore(const QModelIndex& parent)
{
    d->fetching = true;
    Q_ASSERT(!parent.isValid());
    Q_UNUSED(parent);
    VcsJob* job = d->m_iface->log(d->m_url, d->m_rev, qMax(rowCount(), 100));
    connect(this, &VcsEventModel::destroyed, job, [&] { job->kill(); });
    connect(job, &VcsJob::finished, this, &VcsEventModel::jobReceivedResults);
    ICore::self()->runController()->registerJob( job );
}

void VcsEventModel::jobReceivedResults(KJob* job)
{
    QList<QVariant> l = qobject_cast<KDevelop::VcsJob *>(job)->fetchResults().toList();
    if(l.isEmpty() || job->error()!=0) {
        d->done = true;
        return;
    }
    QList<KDevelop::VcsEvent> newevents;
    foreach( const QVariant &v, l )
    {
        if( v.canConvert<KDevelop::VcsEvent>() )
        {
            newevents << v.value<KDevelop::VcsEvent>();
        }
    }
    d->m_rev = newevents.last().revision();
    if(!d->m_events.isEmpty()) {
        newevents.removeFirst();
    }
    d->done = newevents.isEmpty();
    addEvents( newevents );
    d->fetching = false;
}

}

