/* KDevelop xUnit plugin
 *
 * Copyright 2006 systest.ch <qxrunner@systest.ch>
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

/*!
 * \file  viewcontrollercommon.cpp
 *
 * \brief Implements class ViewControllerCommon.
 */

#include "viewcontrollercommon.h"
#include "utils.h"

namespace QxRunner
{

ViewControllerCommon::ViewControllerCommon(QTreeView* view) : m_view(view)
{
    header()->setDefaultAlignment(Qt::AlignLeft);
    header()->setMovable(false);
}

ViewControllerCommon::~ViewControllerCommon()
{

}

QModelIndex ViewControllerCommon::highlightedRow() const
{
    QItemSelectionModel* selectionModel = view()->selectionModel();
    QModelIndexList indexes = selectionModel->selectedIndexes();

    QModelIndex index;

    if (indexes.count() > 0) {
        index = indexes.first();
    }

    return index;
}

void ViewControllerCommon::setHighlightedRow(const QModelIndex& index) const
{
    // Row gets highlighted anyway, independent of current selection mode.
    view()->clearSelection();

    QItemSelectionModel* selectionModel = view()->selectionModel();
    selectionModel->setCurrentIndex(index, QItemSelectionModel::Select |
                                    QItemSelectionModel::Rows);

    // This will expand any parents.
    view()->scrollTo(index);
}

QTreeView* ViewControllerCommon::view() const
{
    return m_view;
}

QHeaderView* ViewControllerCommon::header() const
{
    return view()->header();
}

QAbstractItemModel* ViewControllerCommon::model() const
{
    return static_cast<QAbstractItemModel*>(Utils::modelFromProxy(view()->model()));
}

QSortFilterProxyModel* ViewControllerCommon::proxyModel() const
{
    return static_cast<QSortFilterProxyModel*>(view()->model());
}

} // namespace

