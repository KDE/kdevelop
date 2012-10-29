/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *   Copyright 2009 Aleix Pol <aleixpol@kde.org>                           *
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
#include <kconfiggroup.h>

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/isession.h>

#include "projectmodel.h"
#include <util/kdevstringhandler.h>
#include <kicon.h>
#include <kmimetype.h>

namespace KDevelop
{

BuildItem::BuildItem()
{
}

BuildItem::BuildItem( const QStringList & itemPath )
    : m_itemPath( itemPath )
{
}

BuildItem::BuildItem( KDevelop::ProjectBaseItem* item )
{
    initializeFromItem( item );
}

BuildItem::BuildItem( const BuildItem& rhs )
{
    m_itemPath = rhs.itemPath();
}

void BuildItem::initializeFromItem( KDevelop::ProjectBaseItem* item )
{
    Q_ASSERT(item);
    KDevelop::ProjectModel* model=KDevelop::ICore::self()->projectController()->projectModel();
        
    m_itemPath = model->pathFromIndex(item->index());
}

QString BuildItem::itemName() const
{
    return m_itemPath.last();
}

QString BuildItem::itemProject() const
{
    return m_itemPath.first();
}

KDevelop::ProjectBaseItem* BuildItem::findItem() const
{
    KDevelop::ProjectModel* model=KDevelop::ICore::self()->projectController()->projectModel();
    QModelIndex idx = model->pathToIndex(m_itemPath);
    KDevelop::ProjectBaseItem* item = dynamic_cast<KDevelop::ProjectBaseItem*>(model->itemFromIndex(idx));
    
    return item;
}

bool operator==( const BuildItem& rhs, const BuildItem& lhs  )
{
    return( rhs.itemPath() == lhs.itemPath() );
}

BuildItem& BuildItem::operator=( const BuildItem& rhs )
{
    if( this == &rhs )
        return *this;
    m_itemPath = rhs.itemPath();
    return *this;
}

ProjectBuildSetModel::ProjectBuildSetModel( QObject* parent )
    : QAbstractTableModel( parent )
{
}

QVariant ProjectBuildSetModel::data( const QModelIndex& idx, int role ) const
{
    if( !idx.isValid() || idx.row() < 0 || idx.column() < 0
         || idx.row() >= rowCount() || idx.column() >= columnCount())
    {
        return QVariant();
    }
    
    if(role == Qt::DisplayRole) {
        switch( idx.column() )
        {
            case 0:
                return m_items.at( idx.row() ).itemName();
                break;
            case 1:
                return KDevelop::joinWithEscaping( m_items.at( idx.row() ).itemPath(), '/', '\\');
                break;
        }
    } else if(role == Qt::DecorationRole && idx.column()==0) {
        KDevelop::ProjectBaseItem* item = m_items.at( idx.row() ).findItem();
        if( item ) {
            return KIcon( item->iconName() );
        }
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
            return i18nc("@title:column buildset item name", "Name");
            break;
        case 1:
            return i18nc("@title:column buildset item path", "Path");
            break;
    }
    return QVariant();
}

int ProjectBuildSetModel::rowCount( const QModelIndex& parent ) const
{
    if( parent.isValid() )
        return 0;
    return m_items.count();
}

int ProjectBuildSetModel::columnCount( const QModelIndex& parent ) const
{
    if( parent.isValid() )
        return 0;
    return 2;
}

void ProjectBuildSetModel::addProjectItem( KDevelop::ProjectBaseItem* item )
{
    if( m_items.contains( BuildItem(item) ) )
        return;
    beginInsertRows( QModelIndex(), rowCount(), rowCount() );
    m_items.append(BuildItem(item));
    endInsertRows();
}

bool ProjectBuildSetModel::removeRows( int row, int count, const QModelIndex& parent )
{
    if( parent.isValid() || row > rowCount() || row < 0 || (row+count) > rowCount() || count <= 0 )
        return false;

    beginRemoveRows( QModelIndex(), row, row+count-1 );
    for( int i = row; i < row+count; i++ )
    {
        m_items.removeAt( row );
    }
    endRemoveRows();
    return true;
}

QList<BuildItem> ProjectBuildSetModel::items()
{
    return m_items ;
}

void ProjectBuildSetModel::projectClosed( KDevelop::IProject* project )
{
    for( int i = m_items.count() - 1; i >= 0; i-- )
    {
        if( m_items.at(i).itemProject() == project->name())
        {
            beginRemoveRows( QModelIndex(), i, i );
            m_items.removeAt(i);
            endRemoveRows();
        }
    }  
}

void ProjectBuildSetModel::saveToProject( KDevelop::IProject* project ) const
{
    QVariantList paths;
    foreach( const BuildItem &item, m_items)
    {
        if( item.itemProject() == project->name() )
            paths.append(item.itemPath());
    }
    KConfigGroup base = project->projectConfiguration()->group("Buildset");
    base.writeEntry("BuildItems", KDevelop::qvariantToString( QVariant( paths ) ));
    base.sync();
}

void ProjectBuildSetModel::loadFromProject( KDevelop::IProject* project )
{
    KConfigGroup base = project->projectConfiguration()->group("Buildset");
    if (base.hasKey("BuildItems")) {
        QVariantList items = KDevelop::stringToQVariant(base.readEntry("BuildItems", QString())).toList();

        foreach(const QVariant& path, items)
        {
            beginInsertRows( QModelIndex(), rowCount(), rowCount() );
            m_items.append( BuildItem( path.toStringList() ) );
            endInsertRows();
        }
    } else {
        // Add project to buildset, but only if there is no configuration for this project yet.
        addProjectItem( project->projectItem() );
    }
}

void ProjectBuildSetModel::moveRowsDown(int row, int count)
{
    QList<BuildItem> items = m_items.mid( row, count );
    removeRows( row, count );
    beginInsertRows( QModelIndex(), row+1, row+count );
    for( int i = 0; i < count; i++ )
    {
        m_items.insert( row+1+i, items.at( i ) ); 
    }
    endInsertRows();
}

void ProjectBuildSetModel::moveRowsToBottom(int row, int count)
{
    QList<BuildItem> items = m_items.mid( row, count );
    removeRows( row, count );
    beginInsertRows( QModelIndex(), rowCount(), rowCount()+count-1 );
    m_items += items;
    endInsertRows();
}

void ProjectBuildSetModel::moveRowsUp(int row, int count)
{
    QList<BuildItem> items = m_items.mid( row, count );
    removeRows( row, count );
    beginInsertRows( QModelIndex(), row-1, row-2+count );
    for( int i = 0; i < count; i++ )
    {
        m_items.insert( row-1+i, items.at( i ) ); 
    }
    endInsertRows();
}

void ProjectBuildSetModel::moveRowsToTop(int row, int count)
{
    QList<BuildItem> items = m_items.mid( row, count );
    removeRows( row, count );
    beginInsertRows( QModelIndex(), 0, count-1 );
    for( int i = 0; i < count; i++ )
    {
        m_items.insert( 0+i, items.at( i ) ); 
    }
    endInsertRows();
}

}
