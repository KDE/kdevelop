/* This file is part of KDevelop
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
class KJob;

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
 * Test loosely implements the Composite pattern.
 *
 * eg
 * TestSuite
 *   TestCase
 *      TestCommand
 *
 * Subclasses must reimplement the abstract run() method to do useful
 * application specific work.
 *
 * @unittest Veritas::TestTest
 */
// TODO Factually there are 3 kinds of tests
//        -> aggregates without associated executable
//        -> aggregates with exe
//        -> leaf tests.
//      This should be reflected in the class hierarchy.
class VERITAS_EXPORT Test : public QObject
{
Q_OBJECT
Q_PROPERTY(bool verboseToggle READ needVerboseToggle WRITE setVerboseToggle)
Q_PROPERTY(bool selectionToggle READ needSelectionToggle WRITE setSelectionToggle)
Q_PROPERTY(bool toSource READ supportsToSource WRITE setSupportsToSource)

public Q_SLOTS:

    /*! Implement this in a subclass. Default to doing nada. */
    virtual int run();

public: // Operations

    bool needVerboseToggle() const;
    void setVerboseToggle(bool);
    bool needSelectionToggle() const;
    void setSelectionToggle(bool);
    bool supportsToSource() const;
    void setSupportsToSource(bool);

    /*! Open sourcelocation, to be implemented by concrete tests */
    virtual void toSource() const;

    /*! Factory method that constructs a blank root test. Always use
     *  this method to construct test-tree roots. */
    static Test* createRoot();

    /*!
     * Constructs a test item with the given \a parent and the
     * \a data associated with each column. Ensures that number of
     * columns equals number of columns in \a parent. Initial result
     * is set to Veritas::NoResult.
     *
     * TODO should be removed
     */
    explicit Test(const QList<QVariant>& data, Test* parent = 0);

    explicit Test(const QString& name, Test* parent = 0);
    virtual ~Test();

    /*! Returns false if this test's run method should not be executed. 
        eg because it is a test container (suite).
        Implement this in a subclass. Default to not run. */
    virtual bool shouldRun() const;

    /*! Performs a hard stop if test was running, to be implemented by
     *  concrete tests. */
    virtual void kill();

    /*! Identifies this test. Also shown in the runnerview-tree */
    QString name() const;

    /*! The test that owns this one.
     * Eg a suite can be parent of a case */
    Test* parent() const;

    /*! Append a child test.
     *  Return true if succesful, false if an equally named child
     *  exists. */
    bool addChild(Test* child);

    /*! Retrieve the row-th child */
    Test* child(int row) const;

    /*! Find the direct child with the specified name. 
     *  If no such child exists, 0 is returned. */
    Test* childNamed(const QString& name) const;

    /*! The number of children */
    int childCount() const;

    /*! Reports the item's location within its parent's list */
    int row() const; // whats a better name for this?

    /*! Fetch all leafs of this test in the test tree. */
    QList<Test*> leafs() const;

    /*! The overall state of the test.
     * This can be NotRun, RunSuccess, RunError (among others) */
    TestState state() const;
    void setResult(TestResult* res);
    TestResult* result() const;

    void signalStarted();
    void signalFinished();

    /*! Factory method. May return 0. The job it returns is supposed to show
     *  verbose test output. Eg a KDevelop::OutputJob. */
    virtual KJob* createVerboseOutputJob();

    /*! Contains methods that are only to be used inside the library.
     *  This nested class is not exported nor is the header installed.
     *  Sole purpose is to hide stuff from the outside. */
    class Internal;
    Internal* internal();

Q_SIGNALS:
    /*! Implementor needs to emit this when an aggregate
     *  Test completed executing its children */ 
    void executionFinished();

    /*! Implementor needs to emit this when execution
     * of a leaf test commences. */
    void started(QModelIndex);

    /*! Implementor needs to emit this when a leaf test
     * finished. */
    void finished(QModelIndex);

private:
    friend class Internal;
    Internal* const d;

    // Copy and assignment not supported.
    Test(const Test&);
    Test& operator=(const Test&);
};

} // namespace

#endif // VERITAS_TEST_H
