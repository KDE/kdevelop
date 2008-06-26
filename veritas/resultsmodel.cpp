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
 * \file  resultsmodel.cpp
 *
 * \brief Implements class ResultsModel.
 */

#include "resultsmodel.h"
#include "test.h"
#include "utils.h"
//#include <modeltest.h>

namespace Veritas
{

ResultsModel::ResultsModel(const QStringList& headerData, QObject* parent)
        :  QAbstractListModel(parent), m_headerData(headerData)
{
//    ModelTest* tm = new ModelTest(this);
}

ResultsModel::~ResultsModel()
{

}

bool ResultsModel::hasChildren(const QModelIndex& index) const
{
    return !index.isValid();
}

QVariant ResultsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == Qt::CheckStateRole) {
        // Results have no items with checked state.
        return QVariant();
    }

    if (role == Qt::TextAlignmentRole) {
        return int(Qt::AlignLeft | Qt::AlignTop);
    }

    Test* item = itemFromIndex(m_testItemIndexes.at(index.row()));

    if (role == Qt::DisplayRole) {
        return item->data(index.column());
    }

    if (role != Qt::DecorationRole || index.column() != 0) {
        // First column only has a decoration.
        return QVariant();
    }

    // Icon corresponding to the item's result code.
    return Utils::resultIcon(item->state());
}

QVariant ResultsModel::headerData(int section, Qt::Orientation orientation,
                                  int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return m_headerData.at(section);
    }

    return QVariant();
}

int ResultsModel::rowCount(const QModelIndex& parent) const
{
    if (hasChildren(parent))
        return m_testItemIndexes.count();
    else
        return 0;
}

int ResultsModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_headerData.count();
}

int ResultsModel::result(int row) const
{
    QModelIndex testItemIndex = mapToTestIndex(index(row, 0));

    if (!testItemIndex.isValid()) {
        return Veritas::NoResult;
    }

    Test* item = itemFromIndex(testItemIndex);

    return item->state();
}

QModelIndex ResultsModel::mapToTestIndex(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    return m_testItemIndexes.value(index.row());

    // Note: QList provides sensible default values if the row
    // number is out of range.
}

QModelIndex ResultsModel::mapFromTestIndex(const QModelIndex& testItemIndex) const
{
    if (!testItemIndex.isValid()) {
        return QModelIndex();
    }

    QModelIndex modelIndex;

    qint64 id = testItemIndex.internalId();

    if (m_testItemMap.contains(id)) {
        modelIndex = index(m_testItemMap[id], 0);
    }

    return modelIndex;
}

void ResultsModel::addResult(const QModelIndex& testItemIndex)
{
    if (!testItemIndex.isValid()) {
        return;
    }

    //beginInsertRows(QModelIndex(), rowCount(), rowCount() + 1);
    beginInsertRows(QModelIndex(), rowCount(), rowCount());

    m_testItemIndexes.append(QPersistentModelIndex(testItemIndex));
    m_testItemMap[testItemIndex.internalId()] = rowCount() - 1;

    endInsertRows();
}

void ResultsModel::clear()
{
    m_testItemIndexes.clear();
    m_testItemMap.clear();
    reset();
}

Test* ResultsModel::itemFromIndex(const QModelIndex& testItemIndex) const
{
    return static_cast<Test*>(testItemIndex.internalPointer());
}

} // namespace
