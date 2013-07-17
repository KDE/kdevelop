/* This file is part of KDevelop
    Copyright 2008 Aleix Pol <aleixpol@gmail.com>

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

#include "projectproxymodel.h"
#include <project/projectmodel.h>
#include <KDebug>
#include <KLocalizedString>
#include <qfileinfo.h>
#include <kmimetype.h>
#include <kicon.h>

ProjectProxyModel::ProjectProxyModel(QObject * parent)
    : QSortFilterProxyModel(parent)
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

QModelIndex ProjectProxyModel::proxyIndexFromItem(KDevelop::ProjectBaseItem* item) const
{
    return mapFromSource(projectModel()->indexFromItem(item));
}

KDevelop::ProjectBaseItem* ProjectProxyModel::itemFromProxyIndex( const QModelIndex& idx ) const
{
    return static_cast<KDevelop::ProjectBaseItem*>( projectModel()->itemFromIndex( mapToSource( idx ) ) );
}

QVariant ProjectProxyModel::data(const QModelIndex& index, int role) const
{
    switch(role) {
        case Qt::DecorationRole:
            if( index.isValid() ) {
                return KIcon( QSortFilterProxyModel::data(index, role).toString() );
            }
            break;
        case Qt::DisplayRole:
            if(index.isValid() && hasChildren(index) && (!mFilenameFilters.isEmpty() || !mFilenameExcludeFilters.isEmpty())) {
                QString text = QSortFilterProxyModel::data(index, role).toString();
                int hiddenElements = sourceModel()->rowCount(mapToSource(index)) - rowCount(index);
                return i18n("%1 (%2 hidden)", text, hiddenElements);
            }
            break;
    }
    return QSortFilterProxyModel::data(index, role);
}

bool ProjectProxyModel::recursiveFilterAcceptsRow(KDevelop::ProjectBaseItem *item) const {

    if (!item) {
        return true;
    }

    if (mFilenameFilters.isEmpty() && mFilenameExcludeFilters.isEmpty()) {
        return true;
    }

    bool retval; // Show all items by default

    if (mFilenameFilters.isEmpty() && !mFilenameExcludeFilters.isEmpty()) {
        // Exclude filter is specified only -> show all by default
        retval = true;
    } else {
        // Do not show until it is matched to filter
        retval = false;
    }

    QSharedPointer<QRegExp> filter;

    foreach(filter, mFilenameFilters) {
        if (filter->exactMatch(item->text())) {
            retval = true;
            break;
        }
    }

    if (retval) {
        foreach(filter, mFilenameExcludeFilters) {
            if (filter->exactMatch(item->text())) {
                retval = false;
                break;
            }
        }
    }

    if (!retval &&
        (item->type() == KDevelop::ProjectBaseItem::Folder ||
         item->type() == KDevelop::ProjectBaseItem::BuildFolder))
    {

        foreach(KDevelop::ProjectBaseItem* child, item->children()) {
            bool visible = recursiveFilterAcceptsRow(child);
            if (visible) {
                retval = true;
                break;
            }
        }
    }
    return retval;
}

bool ProjectProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const QModelIndex& index = sourceModel()->index(sourceRow, 0, sourceParent);
    KDevelop::ProjectBaseItem *item = projectModel()->itemFromIndex(index);
    return recursiveFilterAcceptsRow(item);
}

void ProjectProxyModel::setFilterString(const QString &filters)
{
    QStringList patterns(filters.split(QRegExp("[; ]"), QString::SkipEmptyParts));

    QString pattern;
    mFilenameFilters.clear();
    mFilenameExcludeFilters.clear();

    foreach(pattern, patterns)
    {
        // Check for special case: single pattern without special chars -> force prefixed search (qwerty ->qwerty*)
        if (!pattern.contains('*') &&
            !pattern.contains('?') &&
            !pattern.contains('[') &&
            !pattern.contains(']'))
        {
            // Filter has no special symbols (?, *) so adjust it for prefixed search
            pattern += '*';
        }

        if (pattern.startsWith('!')) {
            pattern = pattern.mid(1); // Strip '!'

            mFilenameExcludeFilters.push_back(QSharedPointer<QRegExp>(new QRegExp(pattern, Qt::CaseInsensitive, QRegExp::Wildcard)));
        } else {
            mFilenameFilters.push_back(QSharedPointer<QRegExp>(new QRegExp(pattern, Qt::CaseInsensitive, QRegExp::Wildcard)));
        }
    }

    invalidateFilter();
    recursivelyEmitParentsChanged(QModelIndex());
}

void ProjectProxyModel::recursivelyEmitParentsChanged(const QModelIndex& idx)
{
    if(!hasChildren(idx))
        return;

    for(int i = 0, count = rowCount(idx); i < count; i++) {
        recursivelyEmitParentsChanged(index(i, 0, idx));
    }
    emit dataChanged(idx, idx);
}
