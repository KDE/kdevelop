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
    if( role == Qt::DecorationRole && index.isValid() ) {
        return KIcon( QSortFilterProxyModel::data(index, role).toString() );
    }
    return QSortFilterProxyModel::data(index, role);
}

bool ProjectProxyModel::filterAcceptsRow ( int source_row, const QModelIndex & source_parent ) const
{
    if (mFilenameFilters.isEmpty() && mFilenameExcludeFilters.empty()) {
        return true;
    }

    bool retval = true; // Show all items by default
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

    KDevelop::ProjectBaseItem *item = projectModel()->itemFromIndex(index);

    if (item) {
        if (item->type() != KDevelop::ProjectBaseItem::Folder &&
            item->type() != KDevelop::ProjectBaseItem::BuildFolder)
        {

            if (mFilenameFilters.isEmpty() && !mFilenameExcludeFilters.isEmpty()) {
                // Exclude filter is specified only -> show all by default
                retval = true;
            } else {
                retval = false; // Do not show until it is matched to filter
            }

            QSharedPointer<QRegExp> filter;

            foreach(filter, mFilenameFilters)
            {
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

        }
    }
    return retval;
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
            // Filter has no specia symbols (?, *) so adjust it for prefixed search
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
};
