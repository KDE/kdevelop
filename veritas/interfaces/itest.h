/*
 * This file is part of KDevelop
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

#ifndef VERITAS_ITEST_H
#define VERITAS_ITEST_H

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QModelIndex>

#include "veritas/testresult.h"
#include "veritas/veritasexport.h"

namespace Veritas
{

/*!
 * Prime class, represents a single test entity in a
 * test tree. Also used as data for the underlying 
 * (runner)model.
 * Concrete plugins can introduce structure in this
 * test tree, but this is not a must.
 *
 * eg
 * TestSuite
 *   TestCase
 *      TestCommand
 *
 * Implemented in Veritas::Test
 * Subclasses should implement the run method.
 *
 */
class VERITAS_EXPORT ITest : public QObject
{
Q_OBJECT
public:
    ITest();
    virtual ~ITest();

    /*!
     * String identifier which must be unique 
     * across siblings
     */
    virtual QString name() const = 0;
    static const int s_columnCount;

    /*!
     * Custom code to be executed when this test is run.
     */
    virtual int run() = 0;

    /*!
     * Notify the toolview that execution commenced
     */
    void started();
    /*!
     * Notify the toolview that execution completed
     */
    void finished();

    // these should be removed. instead keep a map
    // of indicides <-> items in the runnermodel itself.
    // remove modelindex param from started & finished signals.
    virtual QModelIndex index() const = 0;
    virtual void setIndex(const QModelIndex& index) = 0;

    /*!
     * Returns false if this test run method
     * should not be executed. eg because it is
     * a test container (suite).
     */
    virtual bool shouldRun() const = 0;
    //virtual void setShouldRun(bool) const = 0;

    /*!
     * Returns true if this test is checked/selected
     * by the user, and thus should be run.
     */
    virtual bool selected() const = 0;
    virtual void setSelected(bool select) = 0;

    /*!
     * Reset the TestResult
     */
    virtual void clear() = 0;
    virtual void setResult(TestResult*) = 0;
    /*!
     * The overall state of the test.
     * This can be NotRun, RunSuccess, RunError (among
     * others)
     */
    virtual Veritas::TestState state() const = 0;
    virtual void setState(TestState) = 0;
    virtual TestResult* result() const = 0;

    /*!
     * The test that owns this one.
     * Ie a suite can be parent of a case
     */
    virtual ITest* parent() const = 0;
    /*!
     * Append a child test
     */
    virtual void addChild(ITest*) = 0;
    /*!
     * Retrieve the i-th child
     */
    virtual ITest* child(int /*i*/) const = 0;
    /*!
     * Find a first-level child with a specific name.
     */
    virtual ITest* childNamed(const QString& name) const = 0;
    /*!
     * The number of children
     */
    virtual int childCount() const = 0;

    // TODO To be removed. this accesses the 
    // data for a 
    virtual QVariant data(int column) const = 0;

    /*!
     * Reports the item's location within its parent's list
     * TODO rename this
     */
    virtual int row() const = 0;

    virtual QList<ITest*> leafs() const = 0;

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
};

} // namespace

#endif // VERITAS_ITEST_H
