/*
    SPDX-FileCopyrightText: 2011 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "test_uicontroller.h"

#include <QTest>
#include <KParts/MainWindow>

#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <interfaces/iuicontroller.h>

QTEST_MAIN(TestUiController)

using namespace KDevelop;

void TestUiController::initTestCase()
{
    AutoTestShell::init({{}}); // load no plugins at all
    TestCore::initialize();
}

void TestUiController::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestUiController::showErrorMessage()
{
    IUiController* c = ICore::self()->uiController();

    // must be shown otherwise resize has no effect
    c->activeMainWindow()->showMaximized();
    QTest::qWait(1000);

    int initialWidth = c->activeMainWindow()->width();
    for(int i = 0; i < 10; ++i) {
        c->showErrorMessage(QString(500, 'c'), 1);
        QTest::qWait(10);
        QCOMPARE(c->activeMainWindow()->size().width(), initialWidth);
    }

    QTest::qWait(1050);
    c->showErrorMessage(QString(50000, 'c'), 1);
    QTest::qWait(10);
    QCOMPARE(c->activeMainWindow()->size().width(), initialWidth);
}
