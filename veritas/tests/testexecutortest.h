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

#ifndef VERITAS_TESTEXECUTORTEST_H
#define VERITAS_TESTEXECUTORTEST_H

#include <QtCore/QObject>
#include <QtTest/QtTest>
#include <QString>

class QSignalSpy;

namespace Veritas {
class Test;
class TestExecutor;

class TestExecutorTest : public QObject
{
Q_OBJECT

private slots:
    void initTestCase();
    void init();
    void cleanup();

    void rootOnly();
    void noneShouldRun();
    void levelOneAggregate();
    void levelTwoAggregate();
    void multipleAggregates();
    void unbalancedAggregates(); // test runs, but this is not supported.
    void noneSelected();
    void deselectedTests();
    void runTwice();

private:
    void assertRun(QSignalSpy*, Test*);
    void assertRunTwice(QSignalSpy*, Test*);
    void assertNotRun(QSignalSpy*, Test*);
    void assertAllDone();

private:
    TestExecutor* m_executor;
    QSignalSpy* m_allDoneSpy;
};

}

#endif // VERITAS_TESTEXECUTORTEST_H
