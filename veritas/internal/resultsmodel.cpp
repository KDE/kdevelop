/* KDevelop xUnit plugin
 *
 * Copyright 2006 Ernst Huber <qxrunner@systest.ch>
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

#include "resultsmodel.h"

#include "../test.h"
#include "../testresult.h"
#include "test_p.h"
#include "utils.h"
#include "tests/common/modeltest.h"

#include <KDebug>

using Veritas::Test;
using Veritas::TestResult;
using Veritas::ResultsModel;

Test* ResultsModel::testFromIndex(const QModelIndex& i) const
{
    if (!i.isValid()) return 0;
    if (i.row() >= m_results.count()) return 0;
    Q_ASSERT(m_results[i.row()]);
    Q_ASSERT(i.model() == this);
    return m_results[i.row()]->owner();
}

void ResultsModel::changed()
{
    emit dataChanged(index(0, 0), index(rowCount(), 0));
}

ResultsModel::ResultsModel(const QStringList& headerData, QObject* parent)
        :  QAbstractListModel(parent), m_headerData(headerData)
{
//    ModelTest* tm = new ModelTest(this);
}

ResultsModel::~ResultsModel()
{}

bool ResultsModel::hasChildren(const QModelIndex& index) const
{
    return !index.isValid() && !m_results.isEmpty();
}

QVariant ResultsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return QVariant();

    Test* ownerTest = 0;
    switch (role) {
    case Qt::TextAlignmentRole:
        return int(Qt::AlignLeft | Qt::AlignTop);
    case Qt::DisplayRole:
        Q_ASSERT(index.row() < m_results.size());
        switch(index.column()) {
        case 0:
            ownerTest = m_results[index.row()]->owner();
            if (!ownerTest) {
                qWarning() << "Owner test not set for result.";
                return QVariant();
            }
            return ownerTest->name();
        case 1:
            return m_results[index.row()]->message();
        case 2:
            return m_results[index.row()]->file().pathOrUrl();
        case 3:
            return QString::number(m_results[index.row()]->line());
        default:
            kDebug()<< "INVALID COLUMN" << index.column() <<"row " << index.column();
            Q_ASSERT(0);
        }
    case Qt::DecorationRole:
        if (index.column() == 0) {
            return Utils::resultIcon(result(index.row()));
        }
        break;
    default: {}
    }

    return QVariant();
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
    if (hasChildren(parent)) {
        return m_results.count();
    } else {
        return 0;
    }
}

int ResultsModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_headerData.count();
}

int ResultsModel::result(int row) const
{
    if (row < 0 || row >= rowCount()) return -1;
    Q_ASSERT(row < m_results.size());
    Q_ASSERT(m_results[row]);
    return m_results[row]->state();
}

QModelIndex ResultsModel::mapToTestIndex(const QModelIndex& index) const
{
    if (!index.isValid()) return QModelIndex();
    Test* t = testFromIndex(index);
    if (!t) return QModelIndex();
    return t->internal()->index();
}

void ResultsModel::addResult(const QModelIndex& testItemIndex)
{
    if (!testItemIndex.isValid()) return;
    Test* t = static_cast<Test*>(testItemIndex.internalPointer());
    Q_ASSERT(t);
    addResult(t->result());
}

void ResultsModel::clear()
{
    m_results.clear();
    reset();
}

void ResultsModel::addResult(TestResult* result)
{
    if (!result || result->state() == Veritas::RunSuccess) return;
    if (result->childCount() == 0) {
        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        m_results << result;
        endInsertRows();
    } else {
        beginInsertRows(QModelIndex(), rowCount(), rowCount()+result->childCount()-1);
        for(int i=0; i<result->childCount(); i++) {
            m_results << result->child(i);
        }
        endInsertRows();
    }
}
