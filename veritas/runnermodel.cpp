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
 * \file  runnermodel.cpp
 *
 * \brief Implements class RunnerModel.
 */

#include "runnermodel.h"
#include "runnermodelthread.h"
#include "test.h"
#include "resultsmodel.h"
#include "utils.h"
#include <modeltest.h>

#include <QColor>
#include <QFont>
#include <QCoreApplication>
#include <QStringList>
#include <QIcon>
#include <KGlobal>
#include <KIcon>
#include <KDebug>
#include <KIconLoader>

namespace Veritas
{

RunnerModel::RunnerModel(QObject* parent)
        : QAbstractItemModel(parent)
{
    // Initialize counters.
    m_numSelected = 0;
    m_numStarted = 0;
    m_numSuccess = 0;
    m_numInfos = 0;
    m_numWarnings = 0;
    m_numErrors = 0;
    m_numFatals = 0;
    m_numExceptions = 0;

    // Other attributes.
    m_rootItem = 0;
    m_resultsModel = 0;

    setMustStop(false);
    setMustWait(false);
    setMinimalUpdate(false);
    setExpectedResults(Veritas::AllStates);

    // Thread for asynchronous execution of items.
    m_thread = new RunnerModelThread(this);
    //ModelTest* tm = new ModelTest(this);
}

RunnerModel::~RunnerModel()
{
    stopItems();

    delete m_resultsModel;
    delete m_rootItem;

    // Learned by experience that the thread should get deleted last.
    m_thread->terminate();
    delete m_thread;

    ///
    /// \todo Stopping items and deleting the thread must become
    ///       bullet proof.
    ///
}

QString RunnerModel::name() const
{
    return m_name;
}

QVariant RunnerModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == Qt::TextAlignmentRole) {
        return int(Qt::AlignLeft | Qt::AlignTop);
    }

    if (isRunning() && isMinimalUpdate()) {
        return dataForMinimalUpdate(index, role);
    }

    Test* item = itemFromIndex(index);

    if (role == Qt::DisplayRole) {
        return item->data(index.column());
    }

    if (role == Qt::FontRole && !item->selected()) {
//         QFont font;
//         font.setItalic(true);
//         return font;
        return QVariant();
    }

    // The other roles are supported for the first column only.
    if (index.column() != 0) {
        return QVariant();
    }

    if (role == Qt::TextColorRole) {
        //kDebug() << item->data(0).toString();
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
        else // evrything ran succesfully
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
            if (someChildHasStatus(status, currentIndex))
                return true;
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
            // Go down.
            currentIndex = currentIndex.child(0, 0);
            continue;
        }

        // Have an item.
        Test* item = itemFromIndex(currentIndex);

        numTotal++;

        if (item->selected()) {
            m_numSelected++;
        }

        switch (item->state()) {
        case Veritas::RunSuccess:
            m_numSuccess++;
            break;

//         case Veritas::RunInfo:
//             m_numInfos++;
//             break;
// 
//         case Veritas::RunWarning:
//             m_numWarnings++;
//             break;

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

        if (siblingIndex.isValid()) {
            // Proceed on same level.
            currentIndex = siblingIndex;
            continue;
        }

        // Go up one or more levels.
        QModelIndex parentIndex = currentIndex.parent();
        currentIndex = parentIndex.sibling(parentIndex.row() + 1, 0);

        while (!currentIndex.isValid()) {
            if (parentIndex.isValid()) {
                parentIndex = parentIndex.parent();
                currentIndex = parentIndex.sibling(parentIndex.row() + 1, 0);
                continue;
            }

            // No more items.
            break;
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

void RunnerModel::runItems()
{
    // No start when thread already/still runs.
    if (isRunning(WAIT_TIME_MILLI)) {
        return;
    }

    // Remove all results.
    clearItem(index(0, 0));
    if (resultsModel()) resultsModel()->clear();

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

    setMustStop(false);

    // Start asynchronous processing of the items.
    m_thread->start();
}

bool RunnerModel::stopItems()
{
    if (!isRunning()) {
        return true;
    }

    setMustWait(false);  // Unblock waiting thread
    setMustStop(true);  // Force soft stop

    // Give thread a chance to run if it's waiting.
    isRunning(WAIT_TIME_MILLI);

    setMustStop(false);  // Proceed if not successful

    if (!isRunning()) {
        return true;
    } else {
        return false;
    }
}

void RunnerModel::emitItemResults()
{
    // Recursively process the items.
    emitItemResult(index(0, 0));
}

bool RunnerModel::isRunning(unsigned long time) const
{
    if (!m_thread->isRunning()) {
        return false;
    }

    if (!time) {
        return true;
    }

    m_thread->wait(time);

    return m_thread->isRunning();
}

ResultsModel* RunnerModel::resultsModel()
{
    if (!m_rootItem) {
        return 0;
    }

    if (!m_resultsModel) {
        // Create the results model.
        QStringList resultHeaders;

        for (int i = 0; i < m_rootItem->columnCount(); i++) {
            resultHeaders.append(m_rootItem->data(i).toString());
        }

        m_resultsModel = new ResultsModel(resultHeaders);
    }

    return m_resultsModel;
}

void RunnerModel::setMinimalUpdate(bool minimalUpdate)
{
    m_minimalUpdate = minimalUpdate;
}

Test* RunnerModel::rootItem() const
{
    return m_rootItem;
}

void RunnerModel::setRootItem(Test* rootItem)
{
    delete m_rootItem;
    m_rootItem = rootItem;

    delete m_resultsModel;
    m_resultsModel = 0;

    // Create the results model (anew).
    if (m_rootItem) resultsModel()->clear();
    initItemConnect(index(0, 0));
}

void RunnerModel::initItemConnect(QModelIndex current)
{
    while (current.isValid()) {
        if (current.child(0, 0).isValid()) {
            // Go down one level.
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

QVariant RunnerModel::dataForMinimalUpdate(const QModelIndex& index, int role) const
{
    // dead code.

    if (!index.isValid()) {
        return QVariant();
    }

    // Only first column gets displayed in minimal update mode.
    if (index.column() != 0) {
        return QVariant();
    }

    Test* item = itemFromIndex(index);

    if (role == Qt::DecorationRole) {
        if (index.child(0, 0).isValid()) {
            return KIcon("arrow-right-double");
        }

        if (item->selected()) {
            return KIcon("arrow-right");
        }
        return KIcon("arrow-right");
    }

    if (role == Qt::DisplayRole) {
        return item->data(index.column());
    }

    return QVariant();
}

// void RunnerModel::initializeSelectionMap()
// {
//     m_selectionMap.clear();
//
//     // Process all top level items.
//     QModelIndex currentIndex = index(0, 0);
//
//     while (currentIndex.isValid()) {
//         if (currentIndex.child(0, 0).isValid()) {
//             // Recursively process sub branches.
//             updateSelectionMap(currentIndex.child(0, 0));
//         }
//
//         currentIndex = currentIndex.sibling(currentIndex.row() + 1, 0);
//     }
// }
//
// void RunnerModel::updateSelectionMap(const QModelIndex& index)
// {
//     if (!index.isValid()) {
//         return;
//     }
//
//     QModelIndex currentIndex = index;
//
//     while (currentIndex.isValid()) {
//         if (currentIndex.child(0, 0).isValid()) {
//             // Recursively process sub branches.
//             updateSelectionMap(currentIndex.child(0, 0));
//
//             currentIndex = currentIndex.sibling(currentIndex.row() + 1, 0);
//             continue;
//         }
//
//         // Update counters for all parents depending on the item state.
//         Test* item = itemFromIndex(currentIndex);
//
//         QModelIndex parentIndex = currentIndex.parent();
//         SelectionPair pair;
//
//         while (parentIndex.isValid()) {
//             if (m_selectionMap.contains(parentIndex.internalId())) {
//                 pair = m_selectionMap.value(parentIndex.internalId());
//             } else {
//                 pair = SelectionPair(0, 0);
//             }
//
//             pair.first++;
//
//             if (item->selected()) {
//                 pair.second++;
//             }
//
//             m_selectionMap[parentIndex.internalId()] = pair;
//
//             // Next parent.
//             parentIndex = parentIndex.parent();
//         }
//
//         currentIndex = currentIndex.sibling(currentIndex.row() + 1, 0);
//     }
// }

void RunnerModel::threadCode()
{
    // Recursively process the items.
    runItem(index(0, 0));

    // Notify that all items have completed.
    AllItemsCompletedEvent* completedEvent;
    completedEvent = new AllItemsCompletedEvent(QModelIndex());
    QCoreApplication::postEvent(this, completedEvent);
}

bool RunnerModel::mustStop()
{
    QMutexLocker locker(&m_lock);

    return m_stop;
}

void RunnerModel::setMustStop(bool stop)
{
    QMutexLocker locker(&m_lock);

    m_stop = stop;
}

bool RunnerModel::mustWait(bool block)
{
    m_lock.lock();

    while (block) {
        if (m_wait) {
            m_lock.unlock();
            m_thread->msleep(10); // Minimal wait
            m_lock.lock();
        } else {
            block = false;
        }
    }

    bool b = m_wait;
    m_lock.unlock();

    return b;
}

void RunnerModel::setMustWait(bool wait)
{
    QMutexLocker locker(&m_lock);

    m_wait = wait;
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

void RunnerModel::runItem(const QModelIndex& index)
{
    // This is thread code!

    if (!index.isValid() || mustStop())
        return;
    QModelIndex currentIndex = index;

    while (currentIndex.isValid()) {
        if (mustStop())
            break;

        if (currentIndex.child(0, 0).isValid()) {
            // Go down one level.
            runItem(currentIndex.child(0, 0));
        }

        Test* item = itemFromIndex(currentIndex);
        if (!item->shouldRun() || !item->selected()) {
            currentIndex = currentIndex.sibling(currentIndex.row() + 1, 0);
            continue;
        }

        if (mustStop())
            break;

        // Execute custom code.
        int res = 0;
        try {
            // results get reported with signals, initialized in initItemConnect()
            item->run();
        } catch (...) {
            item->setState(Veritas::RunException);
            postItemCompleted(currentIndex);
        }

        currentIndex = currentIndex.sibling(currentIndex.row() + 1, 0);
    }
}

void RunnerModel::postItemCompleted(QModelIndex index)
{
    // Send notification to main thread.
    ItemCompletedEvent* completedEvent;
    completedEvent = new ItemCompletedEvent(index);
    QCoreApplication::postEvent(this, completedEvent);
}

void RunnerModel::postItemStarted(QModelIndex index)
{
    // Send notification to main thread.
    ItemGetsStartedEvent* startedEvent;
    startedEvent = new ItemGetsStartedEvent(index);
    QCoreApplication::postEvent(this, startedEvent);
}

void RunnerModel::emitItemResult(const QModelIndex& index)
{
    if (!index.isValid()) {
        return;
    }

    QModelIndex currentIndex = index;

    while (currentIndex.isValid()) {
        if (currentIndex.child(0, 0).isValid()) {
            emitItemResult(currentIndex.child(0, 0));
        }

        Test* item = itemFromIndex(currentIndex);

        if (item->selected() && item->state() != Veritas::NoResult) {
            Test* item = itemFromIndex(currentIndex);
            QModelIndex lastIndex = this->index(currentIndex.row(),
                                                item->columnCount() - 1);

            emit dataChanged(currentIndex, lastIndex); // Also update views
            emit itemCompleted(currentIndex);
        }

        currentIndex = currentIndex.sibling(currentIndex.row() + 1, 0);
    }
}

void RunnerModel::setResultText(Test* item, const QString& text) const
{
    // Set result text in column 1 but without overwriting an existing text.
    if (item->data(1).toString().trimmed().isEmpty()) {
        item->setData(1, text);
    }
}

bool RunnerModel::isMinimalUpdate() const
{
    return m_minimalUpdate;
}


// void RunnerModel::setChildItemChecked(const QModelIndex& index, bool checked)
// {
//     if (!index.isValid()) {
//         return;
//     }
//
//     // The same item can get processed more than once per activation due to
//     // how it gets selected in the GUI and how the model/view framework reacts
//     // to the selection, therefore only 'real' changes are counted.
//     bool changed = false;
//
//     Test* item = itemFromIndex(index);
//
//     bool newState = checked;
//     bool oldState = item->selected();
//
//     if (newState != oldState) {
//         if (newState) {
//             m_numSelected++;
//         } else {
//             m_numSelected--;
//         }
//
//         item->setSelected(newState);
//         changed = true;
//
//         emit numSelectedChanged(m_numSelected);
//     }
//
//     // Update views anyway.
//     emit dataChanged(index, index);
//
//     // Done when no changes occurred.
//     if (!changed) {
//         return;
//     }
//
//     // Updated counters of the child item's parents for their
//     // tri-state handling.
//     QModelIndex parentIndex = index.parent();
//     SelectionPair pair;
//
//     while (parentIndex.isValid()) {
//         if (m_selectionMap.contains(parentIndex.internalId())) {
//             pair = m_selectionMap.value(parentIndex.internalId());
//         } else {
//             pair = SelectionPair(0, 0); // Shouldn't happen
//         }
//
//         if (item->selected()) {
//             pair.second++;
//         } else {
//             pair.second--;
//         }
//
//         m_selectionMap[parentIndex.internalId()] = pair;
//
//         // Update views to reflect state of parent item.
//         emit dataChanged(parentIndex, parentIndex);
//
//         parentIndex = parentIndex.parent();
//     }
// }
//
// QVariant RunnerModel::itemCheckState(const QModelIndex& index) const
// {
//     if (!index.isValid()) {
//         return Qt::Unchecked;
//     }
//
//     Test* item = itemFromIndex(index);
//
//     if (!index.child(0, 0).isValid()) {
//         // Non-parents have two states.
//         if (item->selected()) {
//             return Qt::Checked;
//         } else {
//             return Qt::Unchecked;
//         }
//     }
//
//     // If not yet there then setup selection map for tri-state handling.
//     if (m_selectionMap.count() < 1) {
//         const_cast<RunnerModel*>(this)->initializeSelectionMap();
//     }
//
//     // Parent items are tri-state.
//     SelectionPair pair;
//
//     if (m_selectionMap.contains(index.internalId())) {
//         pair = m_selectionMap.value(index.internalId());
//     } else {
//         pair = SelectionPair(0, 0); // Shouldn't happen
//     }
//
//     if (pair.first == pair.second) {
//         return Qt::Checked;
//     } else if (pair.second > 0) {
//         return Qt::PartiallyChecked;
//     } else {
//         return Qt::Unchecked;
//     }
// }

bool RunnerModel::event(QEvent* event)
{
    if (event->type() < QEvent::User) {
        return QAbstractItemModel::event(event);
    }

    if (event->type() == AllItemsCompleted) {
        emit allItemsCompleted();  // Last item completed

        return true;
    }

    ItemStateChangedEvent* stateChangedEvent = static_cast<ItemStateChangedEvent*>(event);

    QModelIndex firstIndex = stateChangedEvent->index();
    Test* item = itemFromIndex(firstIndex);
    QModelIndex lastIndex = index(firstIndex.row(), item->columnCount() - 1);

    if (event->type() == ItemGetsStarted) {
        m_startedItemIndex = stateChangedEvent->index();
        emit itemStarted(m_startedItemIndex);

        m_numStarted++;
        emit numStartedChanged(m_numStarted);
    } else {
        m_startedItemIndex = QModelIndex();
    }

    if (event->type() == ItemCompleted) {
        // Update result counters and provide default result type string if not
        // set in the item itself.
        switch (item->state()) {
        case Veritas::RunSuccess:
            setResultText(item, tr("Success"));
            m_numSuccess++;
            emit numSuccessChanged(m_numSuccess);
            break;

//         case Veritas::RunInfo:
//             setResultText(item, tr("Info"));
//             m_numInfos++;
//             emit numInfosChanged(m_numInfos);
//             break;
// 
//         case Veritas::RunWarning:
//             setResultText(item, tr("Warning"));
//             m_numWarnings++;
//             emit numWarningsChanged(m_numWarnings);
//             break;

        case Veritas::RunError:
            setResultText(item, tr("Error"));
            m_numErrors++;
            emit numErrorsChanged(m_numErrors);
            break;

        case Veritas::RunFatal:
            setResultText(item, tr("Fatal"));
            m_numFatals++;
            emit numFatalsChanged(m_numFatals);
            break;

        case Veritas::RunException:
            setResultText(item, tr("Exception"));
            m_numExceptions++;
            emit numExceptionsChanged(m_numExceptions);
            break;
        }

        emit numCompletedChanged(m_numStarted);

        if (!isMinimalUpdate()) {
            emit itemCompleted(stateChangedEvent->index());
        }
    }

    // Thread can continue after posted events are processed.
    setMustWait(false);

    // Force view updates.
    // As an optimization no updates are forced when an item gets started since this
    // does not involve any data changes (but could change in the future).
    // Additionally when minimal update is active the views should not get updated
    // when an item has completed in order to speed up item execution since updating
    // the views is time consuming.

    if (event->type() == ItemGetsStarted) {
        return true;
    }

    if (event->type() != ItemCompleted) {
        emit dataChanged(firstIndex, lastIndex);

        return true;
    }

    if (!isMinimalUpdate()) {
        emit dataChanged(firstIndex, lastIndex);
    }

    return true;
}

} // namespace
