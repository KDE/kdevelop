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

#ifndef PROJECTPROXYMODEL_H
#define PROJECTPROXYMODEL_H

#include <QtGui/QSortFilterProxyModel>
#include "projectexport.h"

#include <QList>
#include <QSharedPointer>
#include <QRegExp>
#include <QSet>

namespace KDevelop {
class ProjectModel;
class ProjectBaseItem;
}

class KDEVPLATFORMPROJECT_EXPORT ProjectProxyModel : public QSortFilterProxyModel
{
    public:
        ProjectProxyModel(QObject *parent);
        bool lessThan ( const QModelIndex & left, const QModelIndex & right ) const;

        QModelIndex proxyIndexFromItem(KDevelop::ProjectBaseItem* item) const;
        // Returns zero for items which do not correspond to project items (eg. the "... X Filtered" item)
        KDevelop::ProjectBaseItem* itemFromProxyIndex( const QModelIndex& ) const;
        
        void setFilterString(const QString &filters);
        
        // Used to expand "... X filtered" items
        void itemClicked ( const QModelIndex& item );

        virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
        
    private:
        KDevelop::ProjectModel* projectModel() const;
        bool filterAcceptsRow ( int source_row, const QModelIndex & source_parent ) const;
        QList<QSharedPointer<QRegExp> > mFilenameFilters; 
        QList<QSharedPointer<QRegExp> > mFilenameExcludeFilters;
        
        struct FilterCacheEntry {
            FilterCacheEntry() : visible(true), visibleChildren(0), filteredChildren(0) {}
            bool visible;
            unsigned int visibleChildren;
            unsigned int filteredChildren;
        };
        
        mutable QMap<KDevelop::ProjectBaseItem*, FilterCacheEntry> mFilterCache;
        QMap<KDevelop::ProjectBaseItem*, int> mExpandedFilters;
};

#endif
