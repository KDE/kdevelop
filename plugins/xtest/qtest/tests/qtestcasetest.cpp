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

#include "qtestcasetest.h"
#include <kasserts.h>
#include <qtest_kde.h>
#include <qtestcommand.h>

using QTest::QTestCase;
using QTest::QTestCommand;
using QTest::ut::QTestCaseTest;

namespace
{
QString someCmd()
{
    return "someCmd";
}
}// end anonymous namespace

void QTestCaseTest::init()
{
    m_exe = QFileInfo("my.exe");
    m_name = "test1";
    m_case = new QTestCase(m_name, m_exe, 0);
}

void QTestCaseTest::cleanup()
{
    delete m_case;
}

void QTestCaseTest::construct()
{
    KOMPARE(m_case->childCount(), 0);
    KOMPARE(m_case->name(), m_name);
    KOMPARE(m_case->executable(), m_exe);
}

void QTestCaseTest::addCommand()
{
    KOMPARE(m_case->childCount(), 0);
    QTestCommand* c1 = new QTestCommand(someCmd(), m_case);
    m_case->addChild(c1);

    KOMPARE(m_case->childCount(), 1);
    KOMPARE(m_case->child(0), c1);
}

void QTestCaseTest::addCommands()
{
    KOMPARE(m_case->childCount(), 0);
    QTestCommand* c1 = new QTestCommand(someCmd(), m_case);
    m_case->addChild(c1);
    QTestCommand* c2 = new QTestCommand(someCmd(), m_case);
    m_case->addChild(c2);

    KOMPARE(m_case->childCount(), 2);
    KOMPARE(m_case->child(0), c1);
    KOMPARE(m_case->child(1), c2);

}

#include "qtestcasetest.moc"
QTEST_KDEMAIN(QTestCaseTest, NoGUI)
