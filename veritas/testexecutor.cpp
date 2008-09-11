/* KDevelop xUnit plugin
 *
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

#include "testexecutor.h"
#include "test.h"

#include <QtGlobal>
#include <KDebug>
#include "utils.h"

using Veritas::Test;
using Veritas::TestExecutor;

namespace
{

/*! functor which disconnects a tests' execution finished signal */
class DisconnectTest
{
public:
    void operator()(Test* t);
};

void DisconnectTest::operator()(Test* t)
{
    t->disconnect(SIGNAL(executionFinished()));
}

/*! functor which inter-connects the tests to be run in a signal-slot chain,
 *  so that the next starts when previous finished */
class SetupChain
{
public:
    SetupChain(TestExecutor* exec) : m_previous(0), m_exec(exec) {}
    void operator()(Test* current);
    Test* m_previous;     // previously discovered test-exe
    TestExecutor* m_exec;
    Test* m_first;
};

void SetupChain::operator()(Test* current)
{
    if (!(current->shouldRun() && current->isChecked())) {
        return;           // only run if is an exe and selected
    }
    if (!m_previous) { // first test in the chain.
        QObject::connect(m_exec, SIGNAL(fireStarter()),
                         current, SLOT(run()));
        m_first = current;
    } else {              // start current when previous finished.
        kDebug() << "connect " << m_previous->name() 
                 << " -> "     << current->name();
        QObject::connect(m_previous,SIGNAL(executionFinished()),
                         current, SLOT(run()));
    }
    m_previous = current;
}

} // end anonymous namespace


TestExecutor::TestExecutor()
        : m_root(0)
{
    connect(this, SIGNAL(allDone()), this, SLOT(cleanup()));
}

TestExecutor::~TestExecutor()
{}

void TestExecutor::go()
{
    SetupChain sc(this);
    traverseTree(m_root, sc);
    fixLast(sc.m_previous);
    emit fireStarter();
}

void TestExecutor::stop()
{
    cleanup();
}

void TestExecutor::cleanup()
{
    DisconnectTest dt;
    traverseTree(m_root, dt);
}

void TestExecutor::fixLast(Test* last)
{
    if (last) {
        connect(last, SIGNAL(executionFinished()),
                this, SIGNAL(allDone()));
    } else {
        emit allDone(); // nothing to be run.
    }
}

void TestExecutor::setRoot(Test* root)
{
    m_root = root;
    Q_ASSERT(m_root);
}

#include "testexecutor.moc"
