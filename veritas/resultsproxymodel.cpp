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
 * \file  resultsproxymodel.cpp
 *
 * \brief Implements class ResultsProxyModel.
 */

#include "resultsproxymodel.h"
#include "resultsmodel.h"
#include <testresult.h>
#include <QDebug>

namespace Veritas
{

ResultsProxyModel::ResultsProxyModel(QObject* parent,  int filter)
        : QSortFilterProxyModel(parent), m_filter(filter)
{

}

ResultsProxyModel::~ResultsProxyModel()
{

}

QVariant ResultsProxyModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (!isColumnEnabled(index.column())) {
        return QVariant();
    }

    return QSortFilterProxyModel::data(index, role);
}

int ResultsProxyModel::filter() const
{
    return m_filter;
}

void ResultsProxyModel::setFilter(int filter)
{
    if (m_filter != filter) {
        // Update only when not same filter.
        m_filter = filter;
        reset();
    }
}

bool ResultsProxyModel::filterAcceptsRow(int source_row,
        const QModelIndex& source_parent) const
{
    // No data when proxy model is inactive.
    if (!isActive()) {
        return false;
    }

    int result = model()->result(source_row);

    if ( (result & m_filter) || (result == Veritas::RunException)) {
        return true;
    } else {
        return false;
    }
}

ResultsModel* ResultsProxyModel::model() const
{
    return static_cast<ResultsModel*>(sourceModel());
}

} // namespace

