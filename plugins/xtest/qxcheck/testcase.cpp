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

#include "testcase.h"
#include "testsuite.h"
#include <QDir>
#include <kprocess.h>

using QxCheck::TestCase;
using QxCheck::TestBase;
using QxCheck::TestCommand;
using QxCheck::TestSuite;

TestCase::TestCase()
    : TestBase("", 0)
{
}

TestCase::TestCase(const QString& name, TestBase* parent)
    : TestBase(name, parent)
{
}

TestCase::~TestCase()
{}

TestCommand* TestCase::testAt(unsigned i)
{
    return qobject_cast<TestCommand*>(childAt(i));
}

#include "testcase.moc"
