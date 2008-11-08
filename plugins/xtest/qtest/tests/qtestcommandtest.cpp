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

using QTest::Suite;
using QTest::Case;
using QTest::Command;
using QTest::Test::CommandTest;

void CommandTest::construct()
{
    Command c("cmd1", 0);
    KOMPARE("cmd1", c.name());
}

void CommandTest::cmdString()
{
    Suite* suite = new Suite("s1", QFileInfo("/a/b/"), 0);
    Case* caze = new Case("c1", QFileInfo("c.sh"), suite);
    Command* cmd = new Command("cmd", caze);
    suite->addChild(caze);
    caze->addChild(cmd);

    KOMPARE("/a/b/c.sh cmd", cmd->command());

    delete suite;
}

void CommandTest::cmdStringNoParent()
{
    Command* cmd = new Command("cmd1", 0);
    KOMPARE("", cmd->command());
    delete cmd;
}

void CommandTest::cmdStringNoSuite()
{
    Case* caze = new Case("c1", QFileInfo("c.sh"), 0);
    Command* cmd = new Command("cmd1", caze);
    caze->addChild(cmd);
    KOMPARE("c.sh cmd1", cmd->command());

    delete caze;
}


#include "qtestcommandtest.moc"
QTEST_KDEMAIN(CommandTest, NoGUI)
