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

#ifndef VERITAS_RUNNERMODEL_H
#define VERITAS_RUNNERMODEL_H

#include <QtGui/QIcon>
#include <QtCore/QPair>
#include <QtCore/QAbstractItemModel>

namespace Veritas
{

class Test;
class TestExecutor;
class ResultsModel;
class RunnerModelThread;

/*!
 * \brief The RunnerModel class maintains the core data and executes
 *        runner items.
 *
 * This class stores data using Test objects that are linked
 * together in a pointer-based tree structure.
 *
 * Selected runner items can be executed. Status about ongoing item
 * execution is reported with signals. Items execute in a separate
 * thread which requires thread synchronization. Private classes which
 * subclass QEvent are used to post data from the thread to this class.
 *
 * The model can be set to minimal update mode. In this mode the data()
 * method returns data for column 0 only and not all signals reporting
 * item execution status get fired.
 *
 * RunnerModel also creates the ResultsModel object which collects
 * the results of executed runner items.
 *
 * \sa \ref runner_model_item and \ref implementation_ascpects
 */
class RunnerModel : public QAbstractItemModel
{
Q_OBJECT

public:
    explicit RunnerModel(QObject* parent = 0);

    /*! Destroys this runner model. Ongoing runner item 
     * execution is stopped. */
    ~RunnerModel();

    virtual QString name() const; // TODO dont  think this is used anymore
                                  //      find out and remove.
    void setName(const QString& name) { m_name = name;}

    /*! Returns the data stored under the given \a role for the item
     * referred to by \a index. */
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

    /*! Sets the \a role data for the item at \a index to \a value.
     * Returns true if successful, otherwise false. */
    bool setData (const QModelIndex& index, const QVariant& value,
                  int role = Qt::EditRole);

    /*! Returns the item flags for the given \a index. Column 0 is user
     * checkable, all other columns are read only.*/
    Qt::ItemFlags flags(const QModelIndex& index) const;

    /*! Returns the data for the given \a role and \a section in the
     * header with the specified \a orientation. */
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    /*! Returns the index of the item in the model specified by the
     *  given \a row, \a column and \a parent index. */
    QModelIndex index(int row, int column,
                      const QModelIndex& parent = QModelIndex()) const;

    /*! Returns the parent of the model item with the given \a index.
     * The returned index never corresponds to the root item. */
    QModelIndex parent(const QModelIndex& index) const;

    /*! Returns the number of rows under the given \a parent, or the
     * number of top-level items if an invalid index is specified. */
    int rowCount(const QModelIndex& parent = QModelIndex()) const;

    /*! Returns the number of columns for the given \a parent, or for
     * the root item if an invalid index is specified. */
    int columnCount(const QModelIndex& parent = QModelIndex()) const;

    /*! Returns the result types expected from runner items. Is a
     * combination of OR'ed Veritas::RunnerResult values. */
    int expectedResults() const;

    /*! Removes previous results and starts runner item execution.
     *  Forces attached views to update. */
    void runItems();

    /*! Tries to stop item execution. Returns true if successful,
     *  otherwise false. Must probably be called several times until
     *  stopping succeeds. */
    bool stopItems();

    /*! Fires itemCompleted() for every result contained in the results
     *  model. Forces attached views to update. */
    void emitItemResults();

    /*! Returns true if runner items are currently being executed in
     *  the thread, otherwise false. Waits max. \a time millisecons
     *  to determine the return value. */
    bool isRunning(unsigned long time = 0) const;

    bool someChildHasStatus(int status, const QModelIndex& parent) const;

    /*! Returns the test item the \a index refers to. */
    Test* itemFromIndex(const QModelIndex& index) const;

    void updateView(const QModelIndex& index);

    /*! Sets \a rootItem as the root item. */
    void setRootItem(Test* rootItem);

    /*! Sets the result types that must be expected from runner items.
     *  \a expectedResults is a combination of OR'ed
     *  Veritas::RunnerResult values. */
    void setExpectedResults(int expectedResults);

Q_SIGNALS:
    /*! Emitted when the runner item referred to by
     *  \a index is started. */
    void itemStarted(const QModelIndex& index) const;

    /*! Emitted when the runner item referred to by
     * \a index has completed */
    void itemCompleted(const QModelIndex& index) const;

    /*! Emitted when all runner items have completed. */
    void allItemsCompleted() const;

    /*! Emitted when the number of runner items has
     *  changed. */
    void numTotalChanged(int numItems) const;

    /*! Emitted when the number of selected runner items
     *  has changed. */
    void numSelectedChanged(int numItems) const;

    /*! Emitted when the number of started runner items
     *  has changed. */
    void numStartedChanged(int numItems) const;

    /*! Emitted when the number of completed runner items
     *  has changed. */
    void numCompletedChanged(int numItems) const;

    /*! Emitted when the number of successfully completed
     *  runner items has changed. */
    void numSuccessChanged(int numItems) const;

    /*! Emitted when the number of runner items that
     *  returned an info result has changed. */
    void numInfosChanged(int numItems) const;

    /*! Emitted when the number of runner items that
     *  returned a warning result has changed. */
    void numWarningsChanged(int numItems) const;

    /*! Emitted when the number of runner items that
     *  returned an error result has changed. */
    void numErrorsChanged(int numItems) const;

    /*! Emitted when the number of runner items that
     *  returned a fatal error result has changed. */
    void numFatalsChanged(int numItems) const;

    /*! Emitted when the number of runner items that
     *  produced an unhandled error has changed. */
    void numExceptionsChanged(int numItems) const;

public Q_SLOTS:
    void postItemCompleted(QModelIndex index);
    void postItemStarted(QModelIndex index);

    /*! Updates all internal counters and emits the signals which report
     *  counter values with the actual counts. */
    void countItems();

protected: // Operations

    /*! Returns the root item or 0 if no root item exists. */
    Test* rootItem() const;

private Q_SLOTS:
    void allDone();

private:  // Operations
    void initItemConnect(QModelIndex current);

    /*! Helper method to recursively clear all runner items. Starts
     *  with the runner item referred to by \a index. */
    void clearItem(const QModelIndex& index);

    /*! Sets the textual representation for the execution result in
     *  \a item to \a text. An existing text isn't overwritten. */
    void setResultText(Test* item, const QString& text) const;

    /*! helper for runItems() */
    void initCounters();

    // Copy and assignment not supported.
    RunnerModel(const RunnerModel&);
    RunnerModel& operator=(const RunnerModel&);

private:  // Constants
    static const int WAIT_TIME_MILLI = 200;

private:  // Attributes
    Test* m_rootItem;
    TestExecutor* m_executor;
    QString m_name;
    bool m_isRunning;
    QModelIndex m_startedItemIndex;
    int m_expectedResults;

    int m_numSelected;
    int m_numStarted;
    int m_numSuccess;
    int m_numInfos;
    int m_numWarnings;
    int m_numErrors;
    int m_numFatals;
    int m_numExceptions;

    QModelIndex m_nextInterestingIndex;
};


} // namespace

#endif // VERITAS_RUNNERMODEL_H
