/*
 * This file is part of KDevelop
 *
 * Copyright 2011 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
    AutoTestShell::init();
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
