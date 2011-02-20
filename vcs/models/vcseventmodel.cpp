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

#include <klocale.h>

#include "../vcsevent.h"
#include "../vcsrevision.h"

namespace KDevelop
{

class VcsEventModelPrivate
{
public:
    QList<KDevelop::VcsEvent> m_events;
};

VcsEventModel::VcsEventModel( QObject* parent )
    : QAbstractTableModel( parent ), d(new VcsEventModelPrivate)
{
}

VcsEventModel::~VcsEventModel()
{
    delete d;
}

int VcsEventModel::rowCount( const QModelIndex& ) const
{
    return d->m_events.count();
}

int VcsEventModel::columnCount( const QModelIndex& ) const
{
    return 4;
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
        case 0:
            return QVariant( ev.revision().revisionValue() );
            break;
        case 1:
            return QVariant( ev.author() );
            break;
        case 2:
            return QVariant( ev.date() );
            break;
        case 3:
            return QVariant( ev.message() );
            break;
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
        case 0:
            return QVariant( i18n("Revision") );
            break;
        case 1:
            return QVariant( i18n("Author") );
            break;
        case 2:
            return QVariant( i18n("Date") );
            break;
        case 3:
            return QVariant( i18n("Message") );
            break;
        default:
            break;
    }
    return QVariant();
}

void VcsEventModel::addEvents( const QList<KDevelop::VcsEvent>& list )
{
    if( list.isEmpty() )
        return;
    if( rowCount() > 0 )
        beginInsertRows( QModelIndex(), rowCount(), rowCount()+list.count()-1 );
    else
        beginInsertRows( QModelIndex(), rowCount(), list.count()-1 );
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

void VcsEventModel::clear()
{
    if( rowCount() == 0 )
        beginRemoveRows( QModelIndex(), 0, 0 );
    else
        beginRemoveRows( QModelIndex(), 0, rowCount()-1 );
    d->m_events.clear();
    endRemoveRows();
}

}

#include "vcseventmodel.moc"
