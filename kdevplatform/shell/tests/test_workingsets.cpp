/***************************************************************************
 *   Copyright 2021 Christoph Roick <chrisito@gmx.de>                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "test_workingsets.h"

#include "../core.h"
#include "../documentcontroller.h"
#include "../session.h"
#include "../uicontroller.h"
#include "../workingsetcontroller.h"
#include "../workingsets/workingset.h"

#include <sublime/area.h>

#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include <KParts/MainWindow>

#include <QTest>

#include <QLayout>
#include <QMainWindow>
#include <QMenuBar>

#include <algorithm>

using namespace KDevelop;

const QString setName = QStringLiteral("TestSet");
const QString setName2 = QStringLiteral("TestSet2");
const QString setName3 = QStringLiteral("TestSet3");
const QString text = QStringLiteral("Test");

void TestWorkingSetController::initTestCase()
{
    AutoTestShell::init({{}});
    TestCore::initialize();
    m_workingSetCtrl = Core::self()->workingSetControllerInternal();
    auto uiController = Core::self()->uiControllerInternal();
    m_area = uiController->activeArea();
    m_area_debug = nullptr;
    m_documentCtrl = Core::self()->documentController();
    QMainWindow *mainWindow;
    QVERIFY(mainWindow = uiController->activeMainWindow());
    auto areaDisplay = mainWindow->menuBar()->cornerWidget(Qt::TopRightCorner);
    // active working set + separator + closed working sets + tool button
    QCOMPARE(areaDisplay->layout()->count(), 4);
    // widget that contains the buttons of the currently closed working sets
    m_closedSets = areaDisplay->layout()->itemAt(2)->widget();
}

void TestWorkingSetController::cleanup()
{
    // Activate the default (in the context of these tests) working set
    // which will remain in the persistent QtTest config file
    m_area->setWorkingSet(setName);
    if (m_area_debug) {
        m_area_debug->setWorkingSet(setName);
    }
    for (auto set : m_workingSetCtrl->allWorkingSets()) {
        set->deleteSet(true);
    }
}


void TestWorkingSetController::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestWorkingSetController::restartSession()
{
    // End session and store it for now
    Core::self()->activeSession()->setTemporary(false);
    cleanupTestCase();
    // Reload session and make it temporary again
    initTestCase();
    Core::self()->activeSession()->setTemporary(true);
}


void TestWorkingSetController::createWorkingSet()
{
    // Create a persistent working set
    m_area->setWorkingSet(setName);
    const auto workingSets = m_workingSetCtrl->allWorkingSets();
    QCOMPARE(workingSets.size(), 1);
    const auto id = workingSets.first()->id();
    workingSets.first()->setPersistent(true);
    // Open a document such that the working set is not empty
    m_documentCtrl->openDocumentFromText(text);

    // Create and activate a non-persistent working set
    m_area->setWorkingSet(setName3);
    QCOMPARE(m_workingSetCtrl->allWorkingSets().size(), 2);
    m_documentCtrl->openDocumentFromText(text);

    // Create and activate another non-persistent working set
    m_area->setWorkingSet(setName2);
    QCOMPARE(m_workingSetCtrl->allWorkingSets().size(), 3);
    const auto id2 = m_workingSetCtrl->workingSet(m_area->workingSet())->id();
    m_documentCtrl->openDocumentFromText(text);

    QTRY_COMPARE(m_closedSets->layout()->count(), 2); // working sets 1 + 3 (2 is active)

    restartSession();

    // Check if last non-persistent working set is active and persistent set exists
    QCOMPARE(m_workingSetCtrl->allWorkingSets().size(), 2);
    auto set = m_workingSetCtrl->workingSet(m_area->workingSet());
    QCOMPARE(set->id(), id2);
    QVERIFY(!set->isPersistent());

    // Activate persistent working set
    m_area->setWorkingSet(setName);
    QCOMPARE(m_workingSetCtrl->allWorkingSets().size(), 2);
    set = m_workingSetCtrl->workingSet(m_area->workingSet());
    QCOMPARE(set->id(), id);
    QVERIFY(set->isPersistent());

    QTRY_COMPARE(m_closedSets->layout()->count(), 1); // working set 2 (1 is active)
}

void TestWorkingSetController::deleteWorkingSet()
{
    // Create a persistent working set
    m_area->setWorkingSet(setName2);
    auto set2 = m_workingSetCtrl->workingSet(m_area->workingSet());
    set2->setPersistent(true);
    m_documentCtrl->openDocumentFromText(text);

    // Create and activate another persistent working set
    m_area->setWorkingSet(setName);
    auto set = m_workingSetCtrl->workingSet(m_area->workingSet());
    set->setPersistent(true);
    m_documentCtrl->openDocumentFromText(text);

    QTRY_COMPARE(m_closedSets->layout()->count(), 1); // working set 2 (1 is active)

    // Delete the first set
    set2->deleteSet(false);
    // Try deleting the active set
    set->deleteSet(false);

    QCOMPARE(m_closedSets->layout()->count(), 0); // no closed working set

    restartSession();

    const auto sets = m_workingSetCtrl->allWorkingSets();
    QVERIFY(std::any_of(sets.constBegin(), sets.constEnd(), [&](WorkingSet* set){ return set->id() == setName; }));
    QVERIFY(std::none_of(sets.constBegin(), sets.constEnd(), [&](WorkingSet* set){ return set->id() == setName2; }));

    QCOMPARE(m_closedSets->layout()->count(), 0); // no closed working set
}

void TestWorkingSetController::switchArea()
{
    // Create a persistent working set
    m_area->setWorkingSet(setName2);
    auto set2 = m_workingSetCtrl->workingSet(m_area->workingSet());
    set2->setPersistent(true);
    m_documentCtrl->openDocumentFromText(text);

    // Create and activate another persistent working set
    m_area->setWorkingSet(setName);
    auto set = m_workingSetCtrl->workingSet(m_area->workingSet());
    set->setPersistent(true);
    m_documentCtrl->openDocumentFromText(text);

    Core::self()->uiController()->switchToArea(QStringLiteral("debug"), IUiController::ThisWindow);
    m_area_debug = Core::self()->uiControllerInternal()->activeArea();
    m_area_debug->setWorkingSet(setName); // explicitly set the current working set, as in DebugController::addSession

    QTRY_COMPARE(m_closedSets->layout()->count(), 1); // working set 2

    m_area_debug->setWorkingSet(setName2);
    QTest::qSleep(1000);
    QCOMPARE(m_closedSets->layout()->count(), 1); // working set 1, BUG 375446

    Core::self()->uiController()->switchToArea(QStringLiteral("code"), IUiController::ThisWindow);
    m_area->setWorkingSet(setName2); // explicitly set the current working set, as in DebugController::debuggerStateChanged
    m_area->setWorkingSet(setName);

    QTRY_COMPARE(m_closedSets->layout()->count(), 1); // working set 2, BUG 375446
}

QTEST_MAIN(TestWorkingSetController)
