/*************************************************************************************
 *  Copyright (C) 2016 by Carlos Nihelton <carlosnsoliveira@gmail.com>               *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "test_plugin.h"

#include <QtTest/QTest>
#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include <shell/problem.h>
#include "plugin/plugin.h"

using namespace KDevelop;
// using namespace ClangTidy;

void TestClangtidyPlugin::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
}

void TestClangtidyPlugin::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestClangtidyPlugin::testPlugin()
{
    ClangTidy::Plugin plugin(nullptr);
    QCOMPARE(plugin.configPages(), 1);
    QVERIFY(!plugin.allAvailableChecks().isEmpty());
}

QTEST_GUILESS_MAIN(TestClangtidyPlugin);
