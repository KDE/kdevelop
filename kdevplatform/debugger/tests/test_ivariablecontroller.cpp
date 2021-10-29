/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
