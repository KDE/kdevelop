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

#ifndef VERITAS_TESTEXECUTOR_H
#define VERITAS_TESTEXECUTOR_H

#include <QtCore/QObject>

namespace Veritas
{
class Test;

/*! Initiates and starts execution of test executable in a test tree.
 *  Links succeeding tests together in a signal-slot chain triggered 
 *  with the `fireStarter' signal. */
class TestExecutor : public QObject
{
Q_OBJECT
public:
    TestExecutor();

    /*! The root of the test tree which needs to be executed.
     *  Clients must set this before invoking go(). */
    void setRoot(Test* root);

    /*! Initialize and start execution of user selected tests in the test
     *  tree */
    void go();

    /*! Soft stop, will not start the next executable in the chain */
    void stop();

signals:
    /*! triggers first execution */
    void fireStarter();

    /*! emitted when all tests completed */
    void allDone();

private slots:
    /*! disconnects the test signal-chain */
    void cleanup();

private:
    /*! Recursive function which traverses the tree in a depth first manner.
     *  Applies the visit functor on each test */
    template<typename V>
    void traverse(Test* current, V& visit);

    /*! Connects the last test with allDone() */
    void fixLast(Test*);

private:
    Test* m_root;
};

}

#endif // VERITAS_TESTEXECUTOR_H
