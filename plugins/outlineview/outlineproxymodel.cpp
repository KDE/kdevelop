/*
 *   KDevelop outline view
 *   Copyright 2010, 2015 Alex Richardson <alex.richardson@gmx.de>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "outlineproxymodel.h"

OutlineProxyModel::OutlineProxyModel(QObject* parent) : QSortFilterProxyModel(parent)
{
}

OutlineProxyModel::~OutlineProxyModel()
{
}

bool OutlineProxyModel::filterAcceptsRow(int sourceRow,
        const QModelIndex &sourceParent) const
{
    QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);

    QString q1 = sourceModel()->data(index0).toString();

    int childCount = sourceModel()->rowCount(index0);
    if (childCount > 0) {
        for (int i = 0; i < childCount; ++i) {
            if (filterAcceptsRow(i, index0)) {
                return true;
            }
        }
    }
    Q_ASSERT(filterRegExp().caseSensitivity() == Qt::CaseInsensitive);
    return sourceModel()->data(index0).toString().contains(filterRegExp());

}
