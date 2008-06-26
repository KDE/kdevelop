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

#include "qtestcommandtest.h"
#include <kasserts.h>
#include <qtest_kde.h>

#include <qtestsuite.h>
#include <qtestcase.h>

using QTest::QTestSuite;
using QTest::QTestCase;
using QTest::QTestCommand;
using QTest::ut::QTestCommandTest;

void QTestCommandTest::construct()
{
    QTestCommand c("cmd1", 0);
    KOMPARE("cmd1", c.name());
}

void QTestCommandTest::cmdString()
{
    QTestSuite* suite = new QTestSuite("s1", QFileInfo("/a/b/"), 0);
    QTestCase* caze = new QTestCase("c1", QFileInfo("c.sh"), suite);
    QTestCommand* cmd = new QTestCommand("cmd", caze);
    suite->addChild(caze);
    caze->addChild(cmd);

    KOMPARE("/a/b/c.sh cmd", cmd->command());
}

void QTestCommandTest::cmdStringNoParent()
{
    QTestCommand* cmd = new QTestCommand("cmd1", 0);
    KOMPARE("", cmd->command());
}

void QTestCommandTest::cmdStringNoSuite()
{
    QTestCase* caze = new QTestCase("c1", QFileInfo("c.sh"), 0);
    QTestCommand* cmd = new QTestCommand("cmd1", caze);
    caze->addChild(cmd);
    KOMPARE("c.sh cmd1", cmd->command());
}


#include "qtestcommandtest.moc"
QTEST_KDEMAIN(QTestCommandTest, NoGUI)
