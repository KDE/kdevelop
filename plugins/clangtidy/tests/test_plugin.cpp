/*
    SPDX-FileCopyrightText: 2016 Carlos Nihelton <carlosnsoliveira@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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
