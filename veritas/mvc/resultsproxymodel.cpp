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

#include "veritas/mvc/resultsproxymodel.h"
#include "veritas/mvc/resultsmodel.h"
#include "veritas/testresult.h"
#include "veritas/test.h"

#include <KDebug>

using Veritas::Test;
using Veritas::ResultsModel;
using Veritas::ResultsProxyModel;

ResultsProxyModel::ResultsProxyModel(QObject* parent,  int filter)
        : QSortFilterProxyModel(parent), m_filter(filter)
{}

ResultsProxyModel::~ResultsProxyModel()
{}

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
        resetTestFilter();
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
    if ((result & m_filter) || (result == Veritas::RunException)) {
        if (m_testFilter.isEmpty()) {
            return true;
        }
        QModelIndex i = model()->index(source_row, 0, source_parent);
        Test* t = model()->testFromIndex(i);
        bool b= m_testFilter.contains(t);
        return b;
    } else {
        if (source_parent.isValid()) { // lvl2 item, ie output line
            return true;
        }
        QModelIndex i = model()->index(source_row, 0, source_parent);
        //kDebug() << " filtering " << model()->data(i, Qt::DisplayRole).toString();
        return false;
    }
}

ResultsModel* ResultsProxyModel::model() const
{
    return static_cast<ResultsModel*>(sourceModel());
}

void ResultsProxyModel::setTestFilter(const QList<Test*>& t)
{
    m_testFilter = t;
    reset();
}

void ResultsProxyModel::resetTestFilter()
{
    m_testFilter.clear();
}

