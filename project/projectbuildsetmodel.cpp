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

#include <KLocalizedString>
#include <kconfiggroup.h>

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/isession.h>

#include "projectmodel.h"
#include <util/kdevstringhandler.h>
#include <QIcon>
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
    return model->itemFromIndex(idx);
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

void ProjectBuildSetModel::loadFromSession( ISession* session )
{
    if (!session) {
        return;
    }

    // Load the item ordering cache
    KConfigGroup sessionBuildSetConfig = session->config()->group( "Buildset" );
    QVariantList sessionBuildItems = KDevelop::stringToQVariant( sessionBuildSetConfig.readEntry( "BuildItems", QString() ) ).toList();
    foreach( const QVariant& item, sessionBuildItems ) {
        m_orderingCache.append( item.toStringList() );
    }
}

void ProjectBuildSetModel::storeToSession( ISession* session )
{
    if (!session) {
        return;
    }

    // Store the item ordering cache
    QVariantList sessionBuildItems;
    foreach( const QStringList& item, m_orderingCache) {
        sessionBuildItems.append( item );
    }
    KConfigGroup sessionBuildSetConfig = session->config()->group( "Buildset" );
    sessionBuildSetConfig.writeEntry("BuildItems", KDevelop::qvariantToString( QVariant( sessionBuildItems ) ));
    sessionBuildSetConfig.sync();
}


int ProjectBuildSetModel::findInsertionPlace( const QStringList& itemPath )
{
    /*
     * The ordering cache list is a superset of the build set, and must be ordered in the same way.
     * Example:
     * (m_items)         A - B ----- D --------- G
     * (m_orderingCache) A - B - C - D - E - F - G
     *
     * We scan m_orderingCache until we find the required item (absent in m_items: say, F).
     * In process of scanning we synchronize position in m_orderingCache with position in m_items;
     * so, when we reach F, we have D as last synchronization point and hence return it
     * as the insertion place (actually, we return the next item's index - here, index of G).
     *
     * If an item cannot be found in the ordering list, we append it to the list.
     */

    int insertionIndex = 0;
    bool found = false;
    QList<QStringList>::iterator orderingCacheIterator = m_orderingCache.begin();
    // Points to the item which is next to last synchronization point.
    QList<BuildItem>::iterator nextItemIterator = m_items.begin();

    while( orderingCacheIterator != m_orderingCache.end() ) {

        if( itemPath == *orderingCacheIterator ) {
            found = true;
            break;
        }
        if( nextItemIterator != m_items.end() &&
            nextItemIterator->itemPath() == *orderingCacheIterator ) {
            ++insertionIndex;
            ++nextItemIterator;
        }
        ++orderingCacheIterator;

    } // while

    if( !found ) {
        m_orderingCache.append( itemPath );
    }
    Q_ASSERT( insertionIndex >= 0 && insertionIndex <= m_items.size() );
    return insertionIndex;
}

void ProjectBuildSetModel::removeItemsWithCache( const QList<int>& itemIndices )
{
    /*
     * Removes the items with given indices from both the build set and the ordering cache.
     * List is given since removing many items together is more efficient than by one.
     * 
     * Indices in the list shall be sorted.
     */

    QList<int> itemIndicesCopy = itemIndices;

    beginRemoveRows( QModelIndex(), itemIndices.first(), itemIndices.last() );
    for( QList<QStringList>::iterator cacheIterator = m_orderingCache.end() - 1;
         cacheIterator >= m_orderingCache.begin() && !itemIndicesCopy.isEmpty(); ) {

        int index = itemIndicesCopy.back();
        Q_ASSERT( index >= 0 && index < m_items.size() );
        if( *cacheIterator == m_items.at( index ).itemPath() ) {
            m_orderingCache.erase( cacheIterator-- );
            m_items.removeAt( index );
            itemIndicesCopy.removeLast();
        } else {
            --cacheIterator;
        }

    } // for
    endRemoveRows();

    Q_ASSERT( itemIndicesCopy.isEmpty() );
}

void ProjectBuildSetModel::insertItemWithCache( const BuildItem& item )
{
    int insertionPlace = findInsertionPlace( item.itemPath() );
    beginInsertRows( QModelIndex(), insertionPlace, insertionPlace );
    m_items.insert( insertionPlace, item );
    endInsertRows();
}

void ProjectBuildSetModel::insertItemsOverrideCache( int index, const QList< BuildItem >& items )
{
    Q_ASSERT( index >= 0 && index <= m_items.size() );

    if( index == m_items.size() ) {
        beginInsertRows( QModelIndex(), index, index + items.size() - 1 );
        m_items.append( items );
        foreach( const BuildItem& item, items ) {
            m_orderingCache.append( item.itemPath() );
        }
        endInsertRows();
    } else {
        int indexInCache = m_orderingCache.indexOf( m_items.at( index ).itemPath() );
        Q_ASSERT( indexInCache >= 0 );

        beginInsertRows( QModelIndex(), index, index + items.size() - 1 );
        for( int i = 0; i < items.size(); ++i ) {
            const BuildItem& item = items.at( i );
            m_items.insert( index + i, item );
            m_orderingCache.insert( indexInCache + i, item.itemPath() );
        }
        endInsertRows();
    }
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
            return QIcon::fromTheme( item->iconName() );
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
    BuildItem buildItem( item );
    if( m_items.contains( buildItem ) )
        return;

    insertItemWithCache( buildItem );
}

bool ProjectBuildSetModel::removeRows( int row, int count, const QModelIndex& parent )
{
    if( parent.isValid() || row > rowCount() || row < 0 || (row+count) > rowCount() || count <= 0 )
        return false;

    QList<int> itemsToRemove;
    for( int i = row; i < row+count; i++ )
    {
        itemsToRemove.append( i );
    }
    removeItemsWithCache( itemsToRemove );
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
            insertItemWithCache( BuildItem( path.toStringList() ) );
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
    insertItemsOverrideCache( row + 1, items );
}

void ProjectBuildSetModel::moveRowsToBottom(int row, int count)
{
    QList<BuildItem> items = m_items.mid( row, count );
    removeRows( row, count );
    insertItemsOverrideCache( rowCount(), items );
}

void ProjectBuildSetModel::moveRowsUp(int row, int count)
{
    QList<BuildItem> items = m_items.mid( row, count );
    removeRows( row, count );
    insertItemsOverrideCache( row - 1, items );
}

void ProjectBuildSetModel::moveRowsToTop(int row, int count)
{
    QList<BuildItem> items = m_items.mid( row, count );
    removeRows( row, count );
    insertItemsOverrideCache( 0, items );
}

}
