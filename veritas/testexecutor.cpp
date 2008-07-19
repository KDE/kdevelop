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

using Veritas::Test;
using Veritas::TestExecutor;

TestExecutor::TestExecutor()
    : m_root(0)
{
    connect(this, SIGNAL(allDone()), this, SLOT(cleanup()));
}

void TestExecutor::go()
{
    m_previous = 0;
    traverse(m_root);
    fixLast();
    emit fireStarter();
}

void TestExecutor::cleanup()
{
    disconnectAll(m_root);
}

void TestExecutor::disconnectAll(Test* current)
{
    // TODO visitor with traverse.
    if (not current) return;
    current->disconnect(SIGNAL(executionFinished()));
    int nrof = current->childCount();
    if (nrof != 0) { // go down
        for(int i=0; i<nrof; i++) {
            disconnectAll(current->child(i));
        }
    }
}

void TestExecutor::fixLast()
{
    if (m_previous) {
        connect(m_previous, SIGNAL(executionFinished()),
                this, SIGNAL(allDone()));
    } else {
        emit allDone(); // nothing to be run.
    }
}

void TestExecutor::traverse(Test* current)
{
    // depth-first traversal
    if (not current) return;
    int nrof = current->childCount();
    if (nrof != 0) { // go down
        for(int i=0; i<nrof; i++) {
            traverse(current->child(i));
        }
    }
    setupChain(current);
}

void TestExecutor::setupChain(Test* current)
{
    if (not (current->shouldRun() && current->selected())) {
        return; // only run if is an exe and selected
    }
    if (not m_previous) {
        // first test in the chain.
        connect(this, SIGNAL(fireStarter()),
                current, SLOT(run()));
    } else {
        // start current when previous finished.
        kDebug() << "connect " << m_previous->name() << " -> " << current->name();
        connect(m_previous, SIGNAL(executionFinished()),
                current, SLOT(run()));
    }
    m_previous = current;
}

void TestExecutor::setRoot(Test* root)
{
    m_root = root;
    Q_ASSERT(m_root);
}

#include "testexecutor.moc"
