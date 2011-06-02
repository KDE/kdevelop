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

#include "vcsitemeventmodel.h"

#include <QModelIndex>
#include <QVariant>
#include <QList>

#include <klocale.h>

#include "../vcsrevision.h"
#include "../vcsevent.h"

namespace KDevelop
{

class VcsItemEventModelPrivate
{
public:
    QList<VcsItemEvent> m_itemEvents;
};

VcsItemEventModel::VcsItemEventModel( QObject* parent )
    : QAbstractTableModel( parent ), d( new VcsItemEventModelPrivate )
{
}

VcsItemEventModel::~VcsItemEventModel()
{
    delete d;
}

int VcsItemEventModel::rowCount( const QModelIndex& ) const
{
    return d->m_itemEvents.count();
}

int VcsItemEventModel::columnCount( const QModelIndex& ) const
{
    return 5;
}

QVariant VcsItemEventModel::data( const QModelIndex& idx, int role ) const
{
    if( !idx.isValid() || role != Qt::DisplayRole )
        return QVariant();

    if( idx.row() < 0 || idx.row() >= rowCount()
        || idx.column() < 0 || idx.column() >= columnCount() )
        return QVariant();

    KDevelop::VcsItemEvent ev = d->m_itemEvents.at( idx.row() );
    switch( idx.column() )
    {
        case 0:
            return QVariant( ev.revision().revisionValue() );
            break;
        case 1:
            return QVariant( ev.repositoryLocation() );
            break;
        case 2:
        {
            KDevelop::VcsItemEvent::Actions act = ev.actions();
            QStringList actionStrings;
            if( act & KDevelop::VcsItemEvent::Added )
            {
                actionStrings << i18n("Added");
            }else if( act & KDevelop::VcsItemEvent::Deleted )
            {
                actionStrings << i18n("Deleted");
            }else if( act & KDevelop::VcsItemEvent::Modified )
            {
                actionStrings << i18n("Modified");
            }else if( act & KDevelop::VcsItemEvent::Copied )
            {
                actionStrings << i18n("Copied");
            }
            return QVariant( actionStrings.join(",") );
            break;
        }
        case 3:
            return QVariant( ev.repositoryCopySourceLocation() );
            break;
        case 4:
            return QVariant( ev.repositoryCopySourceRevision().revisionValue() );
            break;
        default:
            break;
    }
    return QVariant();
}

QVariant VcsItemEventModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if( section < 0 || section >= columnCount()
        || orientation != Qt::Horizontal || role != Qt::DisplayRole )
        return QVariant();
    switch( section )
    {
        case 0:
            return QVariant( i18n("Revision") );
            break;
        case 1:
            return QVariant( i18n("Location") );
            break;
        case 2:
            return QVariant( i18n("Actions") );
            break;
        case 3:
            return QVariant( i18n("Source Location") );
            break;
        case 4:
            return QVariant( i18n("Source Revision") );
            break;
        default:
            break;
    }
    return QVariant();
}

void VcsItemEventModel::addItemEvents( const QList<KDevelop::VcsItemEvent>& list )
{
    if( list.isEmpty() )
        return;
    if( rowCount() > 0 )
        beginInsertRows( QModelIndex(), rowCount(), rowCount()+list.count()-1 );
    else
        beginInsertRows( QModelIndex(), rowCount(), list.count()-1 );
    d->m_itemEvents += list;
    endInsertRows();
}

KDevelop::VcsItemEvent VcsItemEventModel::itemEventForIndex( const QModelIndex& idx ) const
{
    if( !idx.isValid() || idx.row() < 0 || idx.row() >= rowCount() )
    {
        return KDevelop::VcsItemEvent();
    }
    return d->m_itemEvents.at( idx.row() );
}

void VcsItemEventModel::clear()
{
    if( rowCount() == 0 )
        beginRemoveRows( QModelIndex(), 0, 0 );
    else
        beginRemoveRows( QModelIndex(), 0, rowCount()-1 );
    d->m_itemEvents.clear();
    endRemoveRows();
}

}

#include "vcsitemeventmodel.moc"
