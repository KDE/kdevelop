/*
 * KDevelop Debugger Support
 *
 * Copyright 2016  Aetf <aetf@unlimitedcodeworks.xyz>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "test_ivariablecontroller.h"

#include <debugger/interfaces/ivariablecontroller.h>
#include <debugger/interfaces/iframestackmodel.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <tests/testdebugsession.h>
#include <tests/testvariablecontroller.h>

#include <QTest>

QTEST_MAIN(KDevelop::TestIVariableController)

using namespace KDevelop;

TestIVariableController::TestIVariableController(QObject* parent)
    : QObject(parent)
{
}

void TestIVariableController::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
    m_debugSession = new TestDebugSession();
    // Simulate an already started and paused debug session
    m_debugSession->runToCursor();
}

void KDevelop::TestIVariableController::updateRightAfterEnableAutoUpdate_data()
{
    QTest::addColumn<int>("startAt");
    QTest::addColumn<int>("switchTo");
    QTest::addColumn<int>("jumpTo");

    QTest::newRow("jump to somewhere else") << 1 << 0 << 2;
    QTest::newRow("jump back") << 1 << 0 << 1;
}

void TestIVariableController::updateRightAfterEnableAutoUpdate()
{
    QFETCH(int, startAt);
    QFETCH(int, switchTo);
    QFETCH(int, jumpTo);

    auto frameStackModel = m_debugSession->frameStackModel();
    auto variableController = qobject_cast<TestVariableController*>(m_debugSession->variableController());
    if (!variableController) {
        return;
    }

    frameStackModel->setCurrentThread(0);
    frameStackModel->setCurrentFrame(startAt);

    int oldCounter = variableController->updatedTimes();

    variableController->setAutoUpdate(IVariableController::UpdateNone);
    frameStackModel->setCurrentFrame(switchTo); // no update
    variableController->setAutoUpdate(IVariableController::UpdateLocals); // trigger update
    // switch back to frame we were at before disable auto update
    frameStackModel->setCurrentFrame(jumpTo); // trigger another update

    int updatedTimes = variableController->updatedTimes() - oldCounter;
    QCOMPARE(updatedTimes, 2);
}

void TestIVariableController::cleanupTestCase()
{
    delete m_debugSession;
    TestCore::shutdown();
}
