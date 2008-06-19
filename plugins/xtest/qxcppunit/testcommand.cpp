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

#include "testcommand.h"
#include "testsuite.h"
#include "testcase.h"

#include <QDir>

using QxCppUnit::TestCommand;
using QxCppUnit::TestBase;
using QxCppUnit::TestCase;
using QxCppUnit::TestSuite;

TestCommand::TestCommand()
    : TestBase("", 0)
{}

TestCommand::TestCommand(const QString& name, TestBase* parent)
    : TestBase(name, parent)
{}

TestCommand::~TestCommand()
{}

#include "testcommand.moc"
