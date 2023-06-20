/*
    SPDX-FileCopyrightText: 2008 Aleix Pol <aleixpol@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "projectproxymodel.h"
#include <project/projectmodel.h>

ProjectProxyModel::ProjectProxyModel(QObject * parent)
    : QSortFilterProxyModel(parent)
    , m_showTargets(true)
{
    setDynamicSortFilter(true);
    sort(0); //initiate sorting regardless of the view
}

KDevelop::ProjectModel * ProjectProxyModel::projectModel() const
{
    return qobject_cast<KDevelop::ProjectModel*>( sourceModel() );
}

bool ProjectProxyModel::lessThan(const QModelIndex & left, const QModelIndex & right) const
{
    KDevelop::ProjectBaseItem *iLeft=projectModel()->itemFromIndex(left), *iRight=projectModel()->itemFromIndex(right);
    if(!iLeft || !iRight) return false;

    return( iLeft->lessThan( iRight ) );
}

void ProjectProxyModel::showTargets(bool visible)
{
    if (visible != m_showTargets) {
        m_showTargets = visible;
        invalidateFilter();
    }
}

bool ProjectProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (m_showTargets) {
        return true;
    }
    else {
        // Get the base item for the associated parent and row.
        QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
        auto *item = projectModel()->itemFromIndex(index);
        // If it's a target, return false, otherwise true.
        return item->type() != KDevelop::ProjectBaseItem::Target
            && item->type() != KDevelop::ProjectBaseItem::LibraryTarget
            && item->type() != KDevelop::ProjectBaseItem::ExecutableTarget;
    }
}

QModelIndex ProjectProxyModel::proxyIndexFromItem(KDevelop::ProjectBaseItem* item) const
{
    return mapFromSource(projectModel()->indexFromItem(item));
}

KDevelop::ProjectBaseItem* ProjectProxyModel::itemFromProxyIndex( const QModelIndex& idx ) const
{
    return projectModel()->itemFromIndex( mapToSource(idx));
}

#include "moc_projectproxymodel.cpp"
