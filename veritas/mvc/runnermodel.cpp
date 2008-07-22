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

#include "veritas/mvc/runnermodel.h"

#include "veritas/test.h"
#include "veritas/testexecutor.h"
#include "veritas/utils.h"
#include "tests/common/modeltest.h"

#include <QColor>

#include <QCoreApplication>
#include <QStringList>
#include <QFont>
#include <QIcon>

#include <KDebug>
#include <KIcon>
#include <KIconLoader>
#include <KGlobal>
#include <KLocale>

using Veritas::RunnerModel;
using Veritas::Test;
using Veritas::TestExecutor;

RunnerModel::RunnerModel(QObject* parent)
        : QAbstractItemModel(parent)
{
    m_numSelected = 0;
    m_numStarted = 0;
    m_numSuccess = 0;
    m_numInfos = 0;
    m_numWarnings = 0;
    m_numErrors = 0;
    m_numFatals = 0;
    m_numExceptions = 0;

    m_rootItem = 0;
    m_executor = 0;
    m_isRunning = false;
    setExpectedResults(Veritas::AllStates);
    //ModelTest* tm = new ModelTest(this);
}

RunnerModel::~RunnerModel()
{
    stopItems();
    delete m_rootItem;
}

QString RunnerModel::name() const
{
    return m_name;
}

QVariant RunnerModel::data(const QModelIndex& index, int role) const
{
    // TODO switch

    if (!index.isValid()) {
        return QVariant();
    }
    if (role == Qt::TextAlignmentRole) {
        return int(Qt::AlignLeft | Qt::AlignTop);
    }
    Test* item = itemFromIndex(index);
    if (role == Qt::DisplayRole) {
        return item->data(index.column());
    }
    if (role == Qt::FontRole && !item->selected()) {
        //  QFont font;
        // font.setItalic(true);
        // return font;
        return QVariant();
    }

    // The other roles are supported for the first column only.
    // There factually is only one column left. TODO rewrite this.
    if (index.column() != 0) {
        return QVariant();
    }
    if (role == Qt::TextColorRole) {
        if (!item->selected())
            return Qt::lightGray;
        else
            return Qt::black;
    }
    if (role != Qt::DecorationRole) {
        return QVariant();
    }
    if (index.child(0, 0).isValid()) {
        if (someChildHasStatus(Veritas::RunError, index))
            return QIcon(":/icons/arrow-right-double-bordeaux-16.png");
        else if (someChildHasStatus(Veritas::NoResult, index))
            return QIcon(":/icons/arrow-right-double-grey-16.png");
        else // evrything ran successfully
            return KIcon("arrow-right-double");
    }
    // Icon corresponding to the item's result code.
    return Utils::resultIcon(item->state());
}

bool RunnerModel::someChildHasStatus(int status, const QModelIndex& parent) const
{
    QModelIndex currentIndex = parent;
    if (currentIndex.child(0, 0).isValid()) {
        currentIndex = currentIndex.child(0, 0);
        while (currentIndex.isValid()) {
            if (someChildHasStatus(status, currentIndex)) {
                return true;
            }
            currentIndex = currentIndex.sibling(currentIndex.row() + 1, 0);
        }
    } else {
        return (status == itemFromIndex(currentIndex)->state());
    }
    return false;
}

bool RunnerModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    Q_UNUSED(index);
    Q_UNUSED(value);
    Q_UNUSED(role);
    return false;
}

void RunnerModel::updateView(const QModelIndex& index)
{
    QModelIndex bottomRight = index;
    while (bottomRight.child(0, 0).isValid()) {
        bottomRight = bottomRight.child(rowCount(bottomRight) - 1, 0);
    }
    kDebug() << itemFromIndex(index)->data(0).toString() << " -> "
             << itemFromIndex(bottomRight)->data(0).toString();
    emit dataChanged(index, bottomRight);
}

Qt::ItemFlags RunnerModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return 0;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant RunnerModel::headerData(int section, Qt::Orientation orientation,
                                 int role) const
{
    if (!m_rootItem) {
        return QVariant();
    }
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return m_rootItem->data(section);
    }
    return QVariant();
}

QModelIndex RunnerModel::index(int row, int column,
                               const QModelIndex& parent) const
{
    if (!m_rootItem || row < 0 || column < 0) {
        return QModelIndex();
    }
    Test* parentItem;
    if (!parent.isValid()) {
        parentItem = m_rootItem;
    } else {
        parentItem = itemFromIndex(parent);
    }
    Test* childItem = parentItem->child(row);
    if (childItem) {
        childItem->setIndex(createIndex(row, column, childItem));
        return childItem->index();
    }
    return QModelIndex();
}

QModelIndex RunnerModel::parent(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }
    Test* childItem = itemFromIndex(index);
    Test* parentItem = childItem->parent();
    if (parentItem == m_rootItem) {
        return QModelIndex();
    }
    parentItem->setIndex(createIndex(parentItem->row(), 0, parentItem));
    return parentItem->index();
}

int RunnerModel::rowCount(const QModelIndex& parent) const
{
    if (!m_rootItem || parent.column() > 0) {
        return 0;
    }
    Test* parentItem;
    if (!parent.isValid()) {
        parentItem = m_rootItem;
    } else {
        parentItem = itemFromIndex(parent);
    }
    return parentItem->childCount();
}

int RunnerModel::columnCount(const QModelIndex& parent) const
{
    if (!m_rootItem) {
        return 0;
    }
    if (parent.isValid()) {
        return itemFromIndex(parent)->columnCount();
    }
    return m_rootItem->columnCount();
}

void RunnerModel::countItems()
{
    int numTotal = 0;

    m_numSelected = 0;
    m_numSuccess = 0;
    m_numInfos = 0;
    m_numWarnings = 0;
    m_numErrors = 0;
    m_numFatals = 0;
    m_numExceptions = 0;

    // Non-recursive traversal of the tree structure.
    QModelIndex currentIndex = index(0, 0);
    while (currentIndex.isValid()) {
        if (currentIndex.child(0, 0).isValid()) {
            currentIndex = currentIndex.child(0, 0); // Go down.
            continue;
        }
        Test* item = itemFromIndex(currentIndex);    // Have an item.
        numTotal++;
        if (item->selected() && !hasChildren(currentIndex)) {
            m_numSelected++;
        }
        switch (item->state()) {
        case Veritas::RunSuccess:
            m_numSuccess++;
            break;
        case Veritas::RunError:
            m_numErrors++;
            break;
        case Veritas::RunFatal:
            m_numFatals++;
            break;
        case Veritas::RunException:
            m_numExceptions++;
            break;
        }
        QModelIndex siblingIndex;
        siblingIndex = currentIndex.sibling(currentIndex.row() + 1, 0);
        if (siblingIndex.isValid()) {                // Proceed on same level.
            currentIndex = siblingIndex;
            continue;
        }
        QModelIndex parentIndex = currentIndex.parent(); // Go up one or more levels.
        currentIndex = parentIndex.sibling(parentIndex.row() + 1, 0);
        while (!currentIndex.isValid()) {
            if (parentIndex.isValid()) {
                parentIndex = parentIndex.parent();
                currentIndex = parentIndex.sibling(parentIndex.row() + 1, 0);
                continue;
            }
            break;  // No more items.
        }
    }

    emit numTotalChanged(numTotal);
    emit numSelectedChanged(m_numSelected);
    emit numSuccessChanged(m_numSuccess);
    emit numInfosChanged(m_numInfos);
    emit numWarningsChanged(m_numWarnings);
    emit numErrorsChanged(m_numErrors);
    emit numFatalsChanged(m_numFatals);
    emit numExceptionsChanged(m_numExceptions);
}

int RunnerModel::expectedResults() const
{
    return m_expectedResults;
}

// helper for runItems()
void RunnerModel::initCounters()
{
    // Initialize counters.
    m_numStarted = 0;
    m_numSuccess = 0;
    m_numInfos = 0;
    m_numWarnings = 0;
    m_numErrors = 0;
    m_numFatals = 0;
    m_numExceptions = 0;

    emit numStartedChanged(m_numStarted);
    emit numCompletedChanged(m_numStarted);
    emit numSuccessChanged(m_numSuccess);
    emit numInfosChanged(m_numInfos);
    emit numWarningsChanged(m_numWarnings);
    emit numErrorsChanged(m_numErrors);
    emit numFatalsChanged(m_numFatals);
    emit numExceptionsChanged(m_numExceptions);

}

void RunnerModel::runItems()
{
    if (isRunning()) return; // dont start while buzzy
    initCounters();
    if (not rootItem()) return;
    m_isRunning = true;
    clearItem(index(0, 0));  // Remove all results.
    if (m_executor) delete m_executor;

    m_executor = new TestExecutor;
    m_executor->setRoot(rootItem());
    connect(m_executor, SIGNAL(allDone()), this, SLOT(allDone()));
    m_executor->go();
}

void RunnerModel::allDone()
{
    m_isRunning = false;
    emit allItemsCompleted();
}

bool RunnerModel::stopItems()
{
    if (m_executor) m_executor->stop();
    return true;
}

bool RunnerModel::isRunning(unsigned long time) const
{
    return m_isRunning;
}

Test* RunnerModel::rootItem() const
{
    return m_rootItem;
}

void RunnerModel::setRootItem(Test* rootItem)
{
    delete m_rootItem;
    m_rootItem = rootItem;
    initItemConnect(index(0, 0));
}

void RunnerModel::initItemConnect(QModelIndex current)
{
    while (current.isValid()) {
        if (current.child(0, 0).isValid()) { // Go down 
            initItemConnect(current.child(0, 0));
        }
        Test* item = itemFromIndex(current);
        //kDebug() << "Connecting item with runnermodel: " << item->data(0).toString();
        connect(item, SIGNAL(started(QModelIndex)),   this, SLOT(postItemStarted(QModelIndex)));
        connect(item, SIGNAL(finished(QModelIndex)), this, SLOT(postItemCompleted(QModelIndex)));
        current = current.sibling(current.row() + 1, 0);
    }
}

void RunnerModel::setExpectedResults(int expectedResults)
{
    m_expectedResults = expectedResults;
}

Test* RunnerModel::itemFromIndex(const QModelIndex& index) const
{
    return static_cast<Test*>(index.internalPointer());
}

void RunnerModel::clearItem(const QModelIndex& index)
{
    if (!index.isValid()) {
        return;
    }
    QModelIndex currentIndex = index;
    while (currentIndex.isValid()) {
        if (currentIndex.child(0, 0).isValid()) {
            clearItem(currentIndex.child(0, 0));
        }
        Test* item = itemFromIndex(currentIndex);
        item->clear();
        QModelIndex lastIndex = this->index(currentIndex.row(),
                                            item->columnCount() - 1);
        emit dataChanged(currentIndex, lastIndex);
        currentIndex = currentIndex.sibling(currentIndex.row() + 1, 0);
    }
}

void RunnerModel::postItemCompleted(QModelIndex index)
{
    Test* item = itemFromIndex(index);
    // Update result counters and provide default result type string if not
    // set in the item itself.
    switch (item->state()) {
    case Veritas::RunSuccess:
        setResultText(item, i18n("Success"));
        m_numSuccess++;
        emit numSuccessChanged(m_numSuccess);
        break;

    case Veritas::RunError:
        setResultText(item, i18n("Error"));
        m_numErrors++;
        emit numErrorsChanged(m_numErrors);
        break;

    case Veritas::RunFatal:
        setResultText(item, i18n("Fatal"));
        m_numFatals++;
        emit numFatalsChanged(m_numFatals);
        break;

    case Veritas::RunException:
        setResultText(item, i18n("Exception"));
        m_numExceptions++;
        emit numExceptionsChanged(m_numExceptions);
        break;
    }

    emit numCompletedChanged(m_numStarted);
    emit itemCompleted(index);
    QModelIndex lastIndex = this->index(index.row(), item->columnCount() - 1);
    emit dataChanged(index, lastIndex);
}

void RunnerModel::postItemStarted(QModelIndex index)
{
    emit itemStarted(index);
    m_numStarted++;
    emit numStartedChanged(m_numStarted);
}

void RunnerModel::setResultText(Test* item, const QString& text) const
{
    // Set result text in column 1 but without overwriting an existing text.
    if (item->data(1).toString().trimmed().isEmpty()) {
        item->setData(1, text);
    }
}
