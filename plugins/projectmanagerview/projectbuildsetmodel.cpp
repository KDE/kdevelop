/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                        *
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

#include "projectbuildsetmodel.h"

#include <QVariant>

#include <kurl.h>
#include <klocale.h>
#include <kdebug.h>

#include "projectmodel.h"
#include "iproject.h"

QString getFolder( KDevelop::ProjectBaseItem* item )
{
    if( !item )
        return "";

    if( item->type() == KDevelop::ProjectBaseItem::Folder
          || item->type() == KDevelop::ProjectBaseItem::BuildFolder )
    {

        return item->project()->relativeUrl( item->folder()->url() ).path();
    }else
    {
        return getFolder( dynamic_cast<KDevelop::ProjectBaseItem*>( item->parent() ) );
    }
}

ProjectBuildSetModel::ProjectBuildSetModel( QObject* parent )
    : QAbstractTableModel( parent )
{
}

QVariant ProjectBuildSetModel::data( const QModelIndex& idx, int role ) const
{
    if( !idx.isValid() || idx.row() < 0 || idx.column() < 0
         || idx.row() >= rowCount() || idx.column() >= columnCount()
         || role != Qt::DisplayRole )
    {
        return QVariant();
    }
    switch( idx.column() )
    {
        case 0:
            return m_items.at( idx.row() )->text();
            break;
        case 1:
            return m_items.at( idx.row() )->project()->name();
            break;
        case 2:
            return getFolder( m_items.at( idx.row() ) );
            break;
    }
    return QVariant();
}

QVariant ProjectBuildSetModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if( section < 0 || section >= columnCount()
        || orientation != Qt::Horizontal || role != Qt::DisplayRole )
        return QVariant();

    switch( section )
    {
        case 0:
            return i18n("Name");
            break;
        case 1:
            return i18n("Project");
            break;
        case 2:
            return i18n("Folder");
            break;
    }
    return QVariant();
}

int ProjectBuildSetModel::rowCount( const QModelIndex& ) const
{
    return m_items.count();
}

int ProjectBuildSetModel::columnCount( const QModelIndex& ) const
{
    return 3;
}

void ProjectBuildSetModel::addProjectItem( KDevelop::ProjectBaseItem* item )
{
    if( m_items.contains( item ) )
        return;
    beginInsertRows( QModelIndex(), rowCount(), rowCount() );
    m_items.append( item );
    endInsertRows();
}

void ProjectBuildSetModel::removeProjectItem( KDevelop::ProjectBaseItem* item )
{
    if( !m_items.contains( item ) || m_items.isEmpty() )
        return;
    int idx = m_items.indexOf( item );
    beginRemoveRows( QModelIndex(), idx, idx );
    m_items.removeAll( item );
    endInsertRows();
}

KDevelop::ProjectBaseItem* ProjectBuildSetModel::itemForIndex( const QModelIndex& idx )
{
    if( !idx.isValid() || idx.row() < 0 || idx.column() < 0
         || idx.column() >= columnCount() || idx.row() >= rowCount() )
        return 0;
    return m_items.at( idx.row() );
}

QList<KDevelop::ProjectBaseItem*> ProjectBuildSetModel::items()
{
    return m_items ;
}

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
