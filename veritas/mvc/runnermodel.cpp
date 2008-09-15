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

namespace
{
inline Test* testFromIndex(const QModelIndex& index)
{
    return static_cast<Test*>(index.internalPointer());
}

QVariant g_failIcon;
QVariant g_notRunIcon;
QVariant g_successIcon;

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
    m_executor = 0;
    m_isRunning = false;
    setExpectedResults(Veritas::AllStates);
    //ModelTest* tm = new ModelTest(this);

    g_failIcon = QIcon(":/icons/arrow-right-double-bordeaux-16.png");
    g_notRunIcon = QIcon(":/icons/arrow-right-double-grey-16.png");
    g_successIcon = KIcon("arrow-right-double");
}

RunnerModel::~RunnerModel()
{
    if (m_executor) m_executor->stop();
    if (m_rootItem) delete m_rootItem;
    if (m_executor) delete m_executor;
}

QString RunnerModel::name() const
{
    return m_name;
}


void RunnerModel::checkAll()
{
    if (m_rootItem) m_rootItem->check();
}

void RunnerModel::uncheckAll()
{
    if (m_rootItem) m_rootItem->unCheck();
}

QVariant RunnerModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return QVariant();
    switch(role) {
    case Qt::TextAlignmentRole :
        return int(Qt::AlignLeft | Qt::AlignTop);
    case Qt::DisplayRole :
        return testFromIndex(index)->data(index.column());
    case Qt::TextColorRole :
        return testFromIndex(index)->isChecked() ?
                Qt::black :
                Qt::lightGray;
    case Qt::DecorationRole :
        if (index.column() != 0) {
            return QVariant();
        }
        if (index.child(0, 0).isValid()) { // not a leaf test
            return computeIconFromChildState(testFromIndex(index));
        } else { // a leaf test
            return Utils::resultIcon(testFromIndex(index)->state());
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
    ParentStateResolver() : done(false) { state = Veritas::RunSuccess; }
    void operator()(Test* current) {
        if (done) return;
        switch(current->state()) {
        case Veritas::RunFatal:
        case Veritas::RunError:
            state = Veritas::RunError;
            done = true;
            break;
        case Veritas::NoResult:
            if (current->childCount()==0) {
                state = Veritas::NoResult;
            }
            break;
        default: {}
        }
    }
    bool done;
    Veritas::TestState state;
};
}

QVariant RunnerModel::computeIconFromChildState(Veritas::Test* test) const
{
    ParentStateResolver psr;
    traverseTree(test, psr);

    switch(psr.state) {
    case Veritas::RunSuccess:
        return g_successIcon;
    case Veritas::RunFatal:
    case Veritas::RunError:
        return g_failIcon;
    case Veritas::NoResult:
        return g_notRunIcon;
    default: {}
    }
    return QVariant();
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
    kDebug() << testFromIndex(index)->data(0).toString() << " -> "
             << testFromIndex(bottomRight)->data(0).toString();
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
        parentItem = testFromIndex(parent);
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
    Test* childItem = testFromIndex(index);
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
        parentItem = testFromIndex(parent);
    }
    return parentItem->childCount();
}

int RunnerModel::columnCount(const QModelIndex& parent) const
{
    if (!m_rootItem) {
        return 0;
    }
    if (parent.isValid()) {
        return testFromIndex(parent)->columnCount();
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
        Test* item = testFromIndex(currentIndex);    // Have an item.
        numTotal++;
        if (item->isChecked() && !hasChildren(currentIndex)) {
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

void RunnerModel::runItems()
{
    if (isRunning()) return; // do not start while buzzy
    initCounters();
    if (!rootItem()) return;
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
    allDone();
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
        Test* item = testFromIndex(current);
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

void RunnerModel::clearItem(const QModelIndex& index)
{
    QModelIndex currentIndex = index;
    while (currentIndex.isValid()) {
        if (currentIndex.child(0, 0).isValid()) {
            clearItem(currentIndex.child(0, 0));
        }
        Test* item = testFromIndex(currentIndex);
        item->clear();
        QModelIndex lastIndex = this->index(currentIndex.row(),
                                            item->columnCount() - 1);
        emit dataChanged(currentIndex, lastIndex);
        currentIndex = currentIndex.sibling(currentIndex.row() + 1, 0);
    }
}

void RunnerModel::postItemCompleted(QModelIndex index)
{
    if (!isRunning()) return;
    Test* item = testFromIndex(index);
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

    default : {}
    }

    emit numCompletedChanged(m_numStarted);
    emit itemCompleted(index);
    QModelIndex lastIndex = this->index(index.row(), item->columnCount() - 1);
    emit dataChanged(index, lastIndex);
}

void RunnerModel::postItemStarted(QModelIndex index)
{
    if (!isRunning()) return;
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
