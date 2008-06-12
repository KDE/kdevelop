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

#include "qtestcommand.h"
#include "qtestsuite.h"
#include "qtestcase.h"

#include <QDir>

using QxQTest::QTestCommand;
using QxQTest::QTestBase;
using QxQTest::QTestCase;
using QxQTest::QTestResult;
using QxQTest::QTestSuite;

QTestCommand::QTestCommand()
    : QTestBase("", 0)
{}

QTestCommand::QTestCommand(const QString& name, QTestBase* parent)
    : QTestBase(name, parent)
{}

QTestCommand::~QTestCommand()
{}

QString QTestCommand::command()
{
    QFileInfo cmd;
    QTestBase* caze = owner();
    if (caze == 0 || qobject_cast<QTestCase*>(caze) == 0)
            return "";
    cmd = qobject_cast<QTestCase*>(caze)->executable();

    return cmd.filePath() + ' ' + name();
}


#include "qtestcommand.moc"
