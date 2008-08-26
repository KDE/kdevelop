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

#ifndef QXQTEST_QTESTRUNNERTEST_H
#define QXQTEST_QTESTRUNNERTEST_H

#include <QObject>
#include <veritas/testresult.h>

namespace Veritas { class Test; class RunnerTestHelper; }

namespace QTest { namespace Test {

class QTestRunnerTest : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void empty();
    void sunnyDay();
    void runTwice();

private:
    void assertAllFilesClosed(Veritas::Test*);
    Veritas::Test* fetchRoot(QByteArray& regXml);
    void assertTestStatus(QMap<QString, Veritas::TestState>, Veritas::Test* root);

private:
    Veritas::RunnerTestHelper* m_runner;
};

}}


#endif // QXQTEST_QTESTRUNNERTEST_H
