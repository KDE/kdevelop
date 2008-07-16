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

#ifndef VERITAS_TEST_H
#define VERITAS_TEST_H

#include "veritas/testresult.h"
#include "veritas/veritasexport.h"

#include <QtCore/QMap>
#include <QtCore/QList>
#include <QtCore/QVariant>
#include <QtCore/QStringList>
#include <QtCore/QModelIndex>

class QAbstractItemModel;

namespace Veritas
{

/*!
 * Prime class, represents a single test entity in a
 * test tree. Also used as data for the RunnerModel.
 * It is not intended to be used directly, but must
 * be subclassed. Concrete test runner implementations
 * can introduce structure in this test tree, but this
 * is not a must.
 *
 * eg
 * TestSuite
 *   TestCase
 *      TestCommand
 *
 * Subclasses must reimplement the abstract run() method to do useful
 * application specific work.
 *
 */
// TODO d-pointer
class VERITAS_EXPORT Test : public QObject
{
Q_OBJECT

public: // Operations

    /*!
     * Constructs a test item with the given \a parent and the
     * \a data associated with each column. Ensures that number of
     * columns equals number of columns in \a parent. Initial result
     * is set to Veritas::NoResult.
     *
     * TODO this constructor is deprecated and should be removed
     */
    explicit Test(const QList<QVariant>& data, Test* parent = 0);

    explicit Test(const QString& name, Test* parent = 0);
    virtual ~Test();

    /*! Implement this in a subclass. Default to doing nada. */
    virtual int run();

    /*! Returns false if this test's run method should not be executed. 
        eg because it is a test container (suite).
        Implement this in a subclass. Default to not run. */
    virtual bool shouldRun() const; // TODO get rid of this.

    /*! Identifies this test. Also shown in the runnerview-tree */
    QString name() const;

    /*! The test that owns this one.
     * Eg a suite can be parent of a case */
    Test* parent() const;

    /*! Append a child test */
    void addChild(Test* child);

    /*! Retrieve the row-th child */
    Test* child(int row) const;

    /*! Find the (first) direct child with the specified name. */
    Test* childNamed(const QString& name) const;

    /*! The number of children */
    int childCount() const;

    /*! Reports the item's location within its parent's list */
    int row() const; // whats a better name for this?

    /*! Fetch all leafs of this test in the test tree. */
    QList<Test*> leafs() const;

    /*! Is this item checked by the user in the tree-runnerview? */
    bool selected() const;
    void setSelected(bool select);

    int columnCount() const;

    QVariant data(int column) const; // TODO might want to get rid of this
    void setData(int column, const QVariant& value);

    /*! The overall state of the test.
     * This can be NotRun, RunSuccess, RunError (among others) */
    TestState state() const;
    void setState(TestState result);
    void setResult(TestResult* res);
    TestResult* result() const;

    /*! Reset the TestResult */
    void clear();

    void setIndex(const QModelIndex& index); // TODO should be removed
    QModelIndex index() const;

    void signalStarted();
    void signalFinished();

Q_SIGNALS:
    /*!
     * Implementor needs to emit this when execution
     * of this test commences.
     *
     * TODO: remove the model index parameter
     */
    void started(QModelIndex);
    /*!
     * Implementor needs to emit this when execution 
     * finished.
     * 
     * TODO remove this model index parameter
     */
    void finished(QModelIndex);

private: // Operations

    // Copy and assignment not supported.
    Test(const Test&);
    Test& operator=(const Test&);

private: // Attributes
    Test* m_parentItem;
    QList<QVariant> m_itemData;
    QList<Test*> m_children;
    QMap<QString, Test*> m_childMap;
    QModelIndex m_index;
    QString m_name;
    TestState m_state;
    TestResult* m_result;
    bool m_selected;
    static const int s_columnCount;
};

} // namespace

#endif // VERITAS_TEST_H
