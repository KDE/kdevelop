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

#include "runnermodel.h"

#include "../test.h"
#include "test_p.h"
#include "testexecutor.h"
#include "utils.h"
#include "tests/modeltest.h"

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

namespace
{
inline Test* testFromIndex(const QModelIndex& index)
{
    return static_cast<Test*>(index.internalPointer());
}

QVariant g_failIcon;
QVariant g_notRunIcon;
QVariant g_successIcon;
QVariant g_leafRunningIcon;
QVariant g_aggregateRunnningIcon;
}

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
    setExpectedResults(Veritas::AllStates);
    //ModelTest* tm = new ModelTest(this);

    g_aggregateRunnningIcon = QIcon(":/icons/go-next-run.png");
    g_successIcon = QIcon(":/icons/go-next-success.png");
    g_notRunIcon = QIcon(":/icons/go-next2.png");
    g_failIcon = QIcon(":/icons/go-next-bug.png");
    g_leafRunningIcon = QIcon(":/icons/system-running-small.png");
}

RunnerModel::~RunnerModel()
{
    if (m_rootItem) delete m_rootItem;
}

void RunnerModel::checkAll()
{
    if (m_rootItem) m_rootItem->internal()->check();
}

void RunnerModel::uncheckAll()
{
    if (m_rootItem) m_rootItem->internal()->unCheck();
}

QVariant RunnerModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return QVariant();
    Q_ASSERT(index.column() == 0);
    Test* t = 0;
    switch(role) {
    case Qt::TextAlignmentRole :
        return int(Qt::AlignLeft | Qt::AlignTop);
    case Qt::DisplayRole :
        return testFromIndex(index)->name();
    case Qt::TextColorRole :
        return testFromIndex(index)->internal()->isChecked() ?
                Qt::black :
                Qt::lightGray;
    case Qt::DecorationRole :
        if (index.child(0, 0).isValid()) { // not a leaf test
            return computeIconFromChildState(testFromIndex(index));
        } else { // a leaf test
            t = testFromIndex(index);
            if (t->internal()->isRunning()) {
                return g_leafRunningIcon;
            } else {
                return Utils::resultIcon(testFromIndex(index)->state());
            }
        }
    default: {}
    }
    return QVariant();
}

namespace
{
/*! Functor which traverses a sub-tree and determines the root's state.
 *  If a single test in the sub-tree failed, the root will have a failed state as well.
 *  If no sub-test failed and some leaf-test has the NoResult state, the root gets that state.
 *  Else everything succeeded and root gets RunSuccess. */
class ParentStateResolver
{
public:
    ParentStateResolver() : isRunning(false), done(false) {
        icon = g_successIcon;
    }
    void operator()(Test* current) {
        if (done) return;
        switch(current->state()) {
        case Veritas::RunFatal:
        case Veritas::RunError:
            icon = g_failIcon;
            done = true;
            break;
        case Veritas::NoResult:
            if (current->childCount() != 0) break;
            if (current->internal()->isRunning()) {
                icon = g_aggregateRunnningIcon;
                isRunning = true;
            } else if (!isRunning) {
                icon = g_notRunIcon;
            }
            break;
        default: {}
        }
    }
    bool isRunning;
    bool done;
    QVariant icon;
};
}

QVariant RunnerModel::computeIconFromChildState(Veritas::Test* test) const
{
    ParentStateResolver psr;
    traverseTree(test, psr);
    return psr.icon;
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
    emit dataChanged(index, bottomRight);
}

Qt::ItemFlags RunnerModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return 0;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex RunnerModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!m_rootItem || row < 0 || column < 0) {
        return QModelIndex();
    }
    Test* parentItem;
    if (!parent.isValid()) {
        parentItem = m_rootItem;
    } else {
        parentItem = testFromIndex(parent);
    }
    Test* childItem = parentItem->child(row);
    if (childItem) {
        childItem->internal()->setIndex(createIndex(row, column, childItem));
        return childItem->internal()->index();
    }
    return QModelIndex();
}

QModelIndex RunnerModel::parent(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }
    Test* childItem = testFromIndex(index);
    Test* parentItem = childItem->parent();
    if (parentItem == m_rootItem) {
        return QModelIndex();
    }
    parentItem->internal()->setIndex(createIndex(parentItem->row(), 0, parentItem));
    return parentItem->internal()->index();
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
        parentItem = testFromIndex(parent);
    }
    return parentItem->childCount();
}

int RunnerModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
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
        Test* item = testFromIndex(currentIndex);    // Have an item.
        numTotal++;
        if (item->internal()->isChecked() && !hasChildren(currentIndex)) {
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
        default: {}
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

    emit numSelectedChanged(m_numSelected);
    emit numSuccessChanged(m_numSuccess);
    emit numInfosChanged(m_numInfos);
    emit numWarningsChanged(m_numWarnings);
    emit numErrorsChanged(m_numErrors);
    emit numFatalsChanged(m_numFatals);
    emit numExceptionsChanged(m_numExceptions);
    emit numTotalChanged(numTotal);
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
        Test* item = testFromIndex(current);
        connect(item, SIGNAL(started(QModelIndex)),   this, SLOT(postItemStarted(QModelIndex)));
        connect(item, SIGNAL(finished(QModelIndex)), this, SLOT(postItemCompleted(QModelIndex)));
        current = current.sibling(current.row() + 1, 0);
    }
}

void RunnerModel::setExpectedResults(int expectedResults)
{
    m_expectedResults = expectedResults;
}

namespace {
struct ClearTest
{
    void operator()(Veritas::Test* test) { test->internal()->clear(); }
};
}

void RunnerModel::clearTree()
{
    if (!m_rootItem) return;
    ClearTest ct;
    traverseTree(m_rootItem, ct);
}

namespace {
/*! Returns true when @p test is last in it's parents childlist */
bool isLastSibling(Test* test)
{
    return test && test->parent() && (test->row() == test->parent()->childCount()-1);
}
}

void RunnerModel::postItemCompleted(QModelIndex index)
{
    Test* item = testFromIndex(index);
    // Update result counters
    switch (item->state()) {
    case Veritas::RunSuccess:
        m_numSuccess++;
        emit numSuccessChanged(m_numSuccess);
        break;

    case Veritas::RunError:
        m_numErrors++;
        emit numErrorsChanged(m_numErrors);
        break;

    case Veritas::RunFatal:
        m_numFatals++;
        emit numFatalsChanged(m_numFatals);
        break;

    case Veritas::RunException:
        m_numExceptions++;
        emit numExceptionsChanged(m_numExceptions);
        break;

    default : {}
    }

    emit numCompletedChanged(m_numStarted);
    emit itemCompleted(index);
    emit dataChanged(index, index);
    index = index.parent();
    if ((item->state() != Veritas::RunSuccess) && (item->state() != Veritas::RunInfo)) {
        emitParentsChanged(index);
    } else {
        // only emit dataChanged singals for the last siblings
        // purpose of this is to get rid of icon flickering for 
        // the parents from running->ready->running->ready->running
        while (index.isValid() && isLastSibling(item)) {
            emit dataChanged(index, index);
            index = index.parent();
            item = item->parent();
        }
    }
}

void RunnerModel::emitParentsChanged(QModelIndex index)
{
    while (index.isValid()) {
        emit dataChanged(index, index);
        index = index.parent(); // the parent changed as well, since parent state is deduced 
                                // from the state of it's children
    }
}

void RunnerModel::postItemStarted(QModelIndex index)
{
    //emit itemStarted(index);
    m_numStarted++;
    emit dataChanged(index, index);
    Test* item = testFromIndex(index);
    index = index.parent();
    while (index.isValid() && item && item->row() == 0) { // to reduce icon flickering
        emit dataChanged(index, index);
        index = index.parent();
        item = item->parent();
    }
    emit numStartedChanged(m_numStarted);
}
