/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "projectbuildsetmodel.h"

#include <QVariant>

#include <KLocalizedString>
#include <KConfigGroup>

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/isession.h>

#include "projectmodel.h"
#include <util/kdevstringhandler.h>
#include <QIcon>

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
    : m_itemPath(rhs.itemPath())
{
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


class ProjectBuildSetModelPrivate
{
public:
    QList<BuildItem> items;
    QList<QStringList> orderingCache;
};


ProjectBuildSetModel::ProjectBuildSetModel( QObject* parent )
    : QAbstractTableModel( parent )
    , d_ptr(new ProjectBuildSetModelPrivate)
{
}

ProjectBuildSetModel::~ProjectBuildSetModel() = default;

void ProjectBuildSetModel::loadFromSession( ISession* session )
{
    Q_D(ProjectBuildSetModel);

    if (!session) {
        return;
    }

    // Load the item ordering cache
    KConfigGroup sessionBuildSetConfig = session->config()->group(QStringLiteral("Buildset"));
    const QVariantList sessionBuildItems = KDevelop::stringToQVariant(sessionBuildSetConfig.readEntry("BuildItems", QString())).toList();
    d->orderingCache.reserve(d->orderingCache.size() + sessionBuildItems.size());
    for (const QVariant& item : sessionBuildItems) {
        d->orderingCache.append(item.toStringList());
    }
}

void ProjectBuildSetModel::storeToSession( ISession* session )
{
    Q_D(ProjectBuildSetModel);

    if (!session) {
        return;
    }

    // Store the item ordering cache
    QVariantList sessionBuildItems;
    sessionBuildItems.reserve(d->orderingCache.size());
    for (const QStringList& item : std::as_const(d->orderingCache)) {
        sessionBuildItems.append( item );
    }
    KConfigGroup sessionBuildSetConfig = session->config()->group(QStringLiteral("Buildset"));
    sessionBuildSetConfig.writeEntry("BuildItems", KDevelop::qvariantToString( QVariant( sessionBuildItems ) ));
    sessionBuildSetConfig.sync();
}


int ProjectBuildSetModel::findInsertionPlace( const QStringList& itemPath )
{
    /*
     * The ordering cache list is a superset of the build set, and must be ordered in the same way.
     * Example:
     * (items)         A - B ----- D --------- G
     * (orderingCache) A - B - C - D - E - F - G
     *
     * We scan orderingCache until we find the required item (absent in items: say, F).
     * In process of scanning we synchronize position in orderingCache with position in items;
     * so, when we reach F, we have D as last synchronization point and hence return it
     * as the insertion place (actually, we return the next item's index - here, index of G).
     *
     * If an item cannot be found in the ordering list, we append it to the list.
     */

    Q_D(ProjectBuildSetModel);

    int insertionIndex = 0;
    bool found = false;
    // Points to the item which is next to last synchronization point.
    QList<BuildItem>::iterator nextItemIterator = d->items.begin();

    for (auto& orderedItemPath : std::as_const(d->orderingCache)) {
        if (itemPath == orderedItemPath) {
            found = true;
            break;
        }
        if (nextItemIterator != d->items.end() &&
            nextItemIterator->itemPath() == orderedItemPath) {
            ++insertionIndex;
            ++nextItemIterator;
        }
    }

    if( !found ) {
        d->orderingCache.append(itemPath);
    }
    Q_ASSERT( insertionIndex >= 0 && insertionIndex <= d->items.size() );
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

    Q_D(ProjectBuildSetModel);

    QList<int> itemIndicesCopy = itemIndices;

    beginRemoveRows( QModelIndex(), itemIndices.first(), itemIndices.last() );
    for (QList<QStringList>::iterator cacheIterator = d->orderingCache.end() - 1;
         cacheIterator >= d->orderingCache.begin() && !itemIndicesCopy.isEmpty();) {

        int index = itemIndicesCopy.back();
        Q_ASSERT( index >= 0 && index < d->items.size() );
        if (*cacheIterator == d->items.at(index).itemPath()) {
            cacheIterator = d->orderingCache.erase(cacheIterator);
            d->items.removeAt(index);
            itemIndicesCopy.removeLast();
        }
        --cacheIterator;

    } // for
    endRemoveRows();

    Q_ASSERT( itemIndicesCopy.isEmpty() );
}

void ProjectBuildSetModel::insertItemWithCache( const BuildItem& item )
{
    Q_D(ProjectBuildSetModel);

    int insertionPlace = findInsertionPlace( item.itemPath() );
    beginInsertRows( QModelIndex(), insertionPlace, insertionPlace );
    d->items.insert(insertionPlace, item);
    endInsertRows();
}

void ProjectBuildSetModel::insertItemsOverrideCache( int index, const QList< BuildItem >& items )
{
    Q_D(ProjectBuildSetModel);

    Q_ASSERT( index >= 0 && index <= d->items.size() );

    if (index == d->items.size()) {
        beginInsertRows( QModelIndex(), index, index + items.size() - 1 );
        d->items.append(items);
        d->orderingCache.reserve(d->orderingCache.size() + items.size());
        for (const BuildItem& item : items) {
            d->orderingCache.append(item.itemPath());
        }
        endInsertRows();
    } else {
        int indexInCache = d->orderingCache.indexOf(d->items.at(index).itemPath());
        Q_ASSERT( indexInCache >= 0 );

        beginInsertRows( QModelIndex(), index, index + items.size() - 1 );
        for( int i = 0; i < items.size(); ++i ) {
            const BuildItem& item = items.at( i );
            d->items.insert(index + i, item);
            d->orderingCache.insert(indexInCache + i, item.itemPath());
        }
        endInsertRows();
    }
}

QVariant ProjectBuildSetModel::data( const QModelIndex& idx, int role ) const
{
    Q_D(const ProjectBuildSetModel);

    if( !idx.isValid() || idx.row() < 0 || idx.column() < 0
         || idx.row() >= rowCount() || idx.column() >= columnCount())
    {
        return QVariant();
    }
    
    if(role == Qt::DisplayRole) {
        switch( idx.column() )
        {
            case 0:
                return d->items.at(idx.row()).itemName();
            case 1:
                return KDevelop::joinWithEscaping(d->items.at(idx.row()).itemPath(), QLatin1Char('/'), QLatin1Char('\\'));
        }
    } else if(role == Qt::DecorationRole && idx.column()==0) {
        KDevelop::ProjectBaseItem* item = d->items.at(idx.row()).findItem();
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
        case 1:
            return i18nc("@title:column buildset item path", "Path");
    }
    return QVariant();
}

int ProjectBuildSetModel::rowCount( const QModelIndex& parent ) const
{
    Q_D(const ProjectBuildSetModel);

    if( parent.isValid() )
        return 0;
    return d->items.count();
}

int ProjectBuildSetModel::columnCount( const QModelIndex& parent ) const
{
    if( parent.isValid() )
        return 0;
    return 2;
}

void ProjectBuildSetModel::addProjectItem( KDevelop::ProjectBaseItem* item )
{
    Q_D(ProjectBuildSetModel);

    BuildItem buildItem( item );
    if (d->items.contains(buildItem))
        return;

    insertItemWithCache( buildItem );
}

bool ProjectBuildSetModel::removeRows( int row, int count, const QModelIndex& parent )
{
    if( parent.isValid() || row > rowCount() || row < 0 || (row+count) > rowCount() || count <= 0 )
        return false;

    QList<int> itemsToRemove;
    itemsToRemove.reserve(count);
    for( int i = row; i < row+count; i++ )
    {
        itemsToRemove.append( i );
    }
    removeItemsWithCache( itemsToRemove );
    return true;
}

QList<BuildItem> ProjectBuildSetModel::items() const
{
    Q_D(const ProjectBuildSetModel);

    return d->items;
}

void ProjectBuildSetModel::projectClosed( KDevelop::IProject* project )
{
    Q_D(ProjectBuildSetModel);

    for (int i = d->items.count() - 1; i >= 0; --i) {
        if (d->items.at(i).itemProject() == project->name()) {
            beginRemoveRows( QModelIndex(), i, i );
            d->items.removeAt(i);
            endRemoveRows();
        }
    }  
}

void ProjectBuildSetModel::saveToProject( KDevelop::IProject* project ) const
{
    Q_D(const ProjectBuildSetModel);

    QVariantList paths;
    for (const BuildItem& item : std::as_const(d->items)) {
        if( item.itemProject() == project->name() )
            paths.append(item.itemPath());
    }
    KConfigGroup base = project->projectConfiguration()->group(QStringLiteral("Buildset"));
    base.writeEntry("BuildItems", KDevelop::qvariantToString( QVariant( paths ) ));
    base.sync();
}

void ProjectBuildSetModel::loadFromProject( KDevelop::IProject* project )
{
    KConfigGroup base = project->projectConfiguration()->group(QStringLiteral("Buildset"));
    if (base.hasKey("BuildItems")) {
        const QVariantList items = KDevelop::stringToQVariant(base.readEntry("BuildItems", QString())).toList();

        for (const QVariant& path : items) {
            insertItemWithCache( BuildItem( path.toStringList() ) );
        }
    } else {
        // Add project to buildset, but only if there is no configuration for this project yet.
        addProjectItem( project->projectItem() );
    }
}

void ProjectBuildSetModel::moveRowsDown(int row, int count)
{
    Q_D(ProjectBuildSetModel);

    QList<BuildItem> items = d->items.mid(row, count);
    removeRows( row, count );
    insertItemsOverrideCache( row + 1, items );
}

void ProjectBuildSetModel::moveRowsToBottom(int row, int count)
{
    Q_D(ProjectBuildSetModel);

    QList<BuildItem> items = d->items.mid(row, count);
    removeRows( row, count );
    insertItemsOverrideCache( rowCount(), items );
}

void ProjectBuildSetModel::moveRowsUp(int row, int count)
{
    Q_D(ProjectBuildSetModel);

    QList<BuildItem> items = d->items.mid(row, count);
    removeRows( row, count );
    insertItemsOverrideCache( row - 1, items );
}

void ProjectBuildSetModel::moveRowsToTop(int row, int count)
{
    Q_D(ProjectBuildSetModel);

    QList<BuildItem> items = d->items.mid(row, count);
    removeRows( row, count );
    insertItemsOverrideCache( 0, items );
}

}

#include "moc_projectbuildsetmodel.cpp"
