/*
 * This file is part of KDevelop
 *
 * Copyright 2016 Carlos Nihelton <carlosnsoliveira@gmail.com>
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

#include "test_plugin.h"

// SUT
#include "plugin.h"
// KDevPlatform
#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <shell/problem.h>
// Qt
#include <QTest>

using namespace KDevelop;
// using namespace ClangTidy;

void TestClangTidyPlugin::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
}

void TestClangTidyPlugin::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestClangTidyPlugin::testPlugin()
{
    ClangTidy::Plugin plugin(nullptr);
    QCOMPARE(plugin.configPages(), 1);
    QVERIFY(!plugin.allAvailableChecks().isEmpty());
}

QTEST_GUILESS_MAIN(TestClangTidyPlugin);
