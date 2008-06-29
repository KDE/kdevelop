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
#include <KDebug>
#include <modeltest.h>

namespace Veritas
{

ResultsModel::ResultsModel(const QStringList& headerData, QObject* parent)
        :  //QAbstractListModel(parent), m_headerData(headerData)
        QAbstractItemModel(parent), m_headerData(headerData)
{
    //ModelTest* tm = new ModelTest(this);
}

QModelIndex ResultsModel::index(int row, int column, const QModelIndex& parent) const
{
    if (row < 0 || column < 0) {
        return QModelIndex();
    }

    QModelIndex i;
    if (!parent.isValid()) {
        // top level item requested, ie a genuine test result
        if (row < m_result2runner.count()) {
            void* id = m_result2runner[row].internalPointer();
            i = createIndex(row, column, id);
        }
    } else if (isTopLevel(parent)) {
        // lvl1 item requested, ie some output line
        if (parent.column() == 0) {
            Test* t = itemFromIndex(m_result2runner.value(parent.row()));
            if (t->result().outputLineCount() > row) {
                const char* data = t->result().outputLine(row).constData();
                i = createIndex(row, column, (void*)data);
            } else {
                 //kDebug() << " row >= outputLineCount() ... "
                 //         << row << "<->" << t->result().outputLineCount();
            }
        }
    } else {
        // lv2 item requested, which do not exist.
        // kDebug() << "\n\tparent-> " << parent.internalId()
        //          << " | " << QString(static_cast<const char*>(parent.internalPointer()))
        //          << "\n\trow-> " << row
        //          << "\n\trunner2result " << m_runner2result;*/
    }
    return i;
}

bool ResultsModel::isTopLevel(const QModelIndex& i) const
{
    return m_runner2result.contains(i.internalId());
}

QModelIndex ResultsModel::parent(const QModelIndex& i) const
{
    // Resultsmodel is a tree of depth 2:
    // test results and their output
    if (!i.isValid()) {
        return QModelIndex();
    }
    if (isTopLevel(i)) {
        return QModelIndex();
    } else {
        const char* line = static_cast<const char*>(i.internalPointer());
        int row =  m_output2result[line];
        return index(row, 0, QModelIndex());
    }
}

ResultsModel::~ResultsModel()
{}

bool ResultsModel::hasChildren(const QModelIndex& index) const
{
    if (!index.isValid())
        return true;
    return (index.child(0,0).isValid());
}

QString ResultsModel::debug() const
{
    kDebug() << "\n\trunner2result " << m_runner2result
             << "\n\tresult2runner " << m_result2runner
             << "\n\toutput2result " << m_output2result;
    return "";
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

    if (isTopLevel(index)) {
        Test* item = itemFromIndex(m_result2runner.at(index.row()));
        if (role == Qt::DisplayRole) {
            return item->data(index.column());
        }
        if (role != Qt::DecorationRole || index.column() != 0) {
            // First column only has a decoration.
            return QVariant();
        }
        // Icon corresponding to the item's result code.
        return Utils::resultIcon(item->state());
    } else {
        if ( role != Qt::DisplayRole || index.column() != 0)
            return QVariant();
        return static_cast<const char*>(index.internalPointer());
    }
}

QVariant ResultsModel::headerData(int section, Qt::Orientation orientation,
                                  int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return m_headerData.at(section);
    }

    return QVariant();
}

int ResultsModel::rowCount(const QModelIndex& index) const
{
    if (!hasChildren(index))
        return 0;
    if (!index.isValid())
        return m_result2runner.count();

    QModelIndex testItemIndex = m_result2runner.value(index.row());
    Test* item = itemFromIndex(testItemIndex);
    if (item->state() == Veritas::RunError) {
        kDebug() << "nrof result lines: " << item->result().outputLineCount();
    }
    return item->result().outputLineCount();
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
        //return Veritas::NoResult;
        return Veritas::RunError;
    }
    Test* item = itemFromIndex(testItemIndex);
    return item->state();
}

QModelIndex ResultsModel::mapToTestIndex(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }
    return m_result2runner.value(index.row());
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
    if (m_runner2result.contains(id)) {
        modelIndex = index(m_runner2result[id], 0);
    }
    return modelIndex;
}

void ResultsModel::addResult(const QModelIndex& testItemIndex)
{
    if (!testItemIndex.isValid()) {
        return;
    }

    beginInsertRows(QModelIndex(), rowCount(), rowCount());

    m_result2runner.append(QPersistentModelIndex(testItemIndex));
    m_runner2result[testItemIndex.internalId()] = rowCount() - 1;

    TestResult res = itemFromIndex(testItemIndex)->result();
    QList<QByteArray>::ConstIterator it = res.m_output.begin();
    int id = rowCount() - 1;
    while (it != res.m_output.end()) {
        m_output2result[it->constData()] = id;
        it++;
    }

    endInsertRows();
}


void ResultsModel::clear()
{
    m_result2runner.clear();
    m_runner2result.clear();
    reset();
}

Test* ResultsModel::itemFromIndex(const QModelIndex& testItemIndex) const
{
    return static_cast<Test*>(testItemIndex.internalPointer());
}

} // namespace
