/*
    SPDX-FileCopyrightText: 2021 Christoph Roick <chrisito@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "test_workingsets.h"

#include "../core.h"
#include "../documentcontroller.h"
#include "../session.h"
#include "../uicontroller.h"
#include "../workingsetcontroller.h"
#include "../workingsets/workingset.h"

#include <sublime/area.h>
#include <sublime/document.h>
#include <sublime/view.h>

#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include <KParts/MainWindow>

#include <QTest>

#include <QLayout>
#include <QMainWindow>
#include <QMenuBar>
#include <QSplitter>

#include <algorithm>

using namespace KDevelop;

const QString setName = QStringLiteral("TestSet");
const QString setName2 = QStringLiteral("TestSet2");
const QString setName3 = QStringLiteral("TestSet3");

void TestWorkingSetController::initTestCase()
{
    AutoTestShell::init({{}});
    TestCore::initialize();
    m_workingSetCtrl = Core::self()->workingSetControllerInternal();
    auto uiController = Core::self()->uiControllerInternal();
    m_area = uiController->activeArea();
    m_area_debug = nullptr;
    m_documentCtrl = Core::self()->documentController();
    QVERIFY(m_mainWindow = uiController->activeMainWindow());
    auto areaDisplay = m_mainWindow->menuBar()->cornerWidget(Qt::TopRightCorner);
    // active working set + separator + closed working sets + tool button
    QCOMPARE(areaDisplay->layout()->count(), 4);
    // widget that contains the buttons of the currently closed working sets
    m_closedSets = areaDisplay->layout()->itemAt(2)->widget();
}

void TestWorkingSetController::init()
{
    m_file.setFileTemplate(m_tempDir.path() + "/tmp_XXXXXX.txt");
    if(!m_file.open()) {
        QFAIL("Cannot create temp file");
    }
}

void TestWorkingSetController::cleanupTestCase()
{
    m_mainWindow->close();
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
    QVERIFY(workingSets.first()->isPersistent());
    // Open a document such that the working set is not empty
    m_documentCtrl->openDocument(QUrl::fromLocalFile(m_file.fileName()));

    // Create and activate a non-persistent working set
    m_area->setWorkingSet(setName3, false);
    QCOMPARE(m_workingSetCtrl->allWorkingSets().size(), 2);
    m_documentCtrl->openDocument(QUrl::fromLocalFile(m_file.fileName()));

    // Create and activate another non-persistent working set
    m_area->setWorkingSet(setName2, false);
    QCOMPARE(m_workingSetCtrl->allWorkingSets().size(), 3);
    const auto id2 = m_workingSetCtrl->workingSet(m_area->workingSet())->id();
    m_documentCtrl->openDocument(QUrl::fromLocalFile(m_file.fileName()));

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
    m_documentCtrl->openDocument(QUrl::fromLocalFile(m_file.fileName()));

    // Create and activate another persistent working set
    m_area->setWorkingSet(setName);
    auto set = m_workingSetCtrl->workingSet(m_area->workingSet());
    set->setPersistent(true);
    m_documentCtrl->openDocument(QUrl::fromLocalFile(m_file.fileName()));

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
    m_documentCtrl->openDocument(QUrl::fromLocalFile(m_file.fileName()));

    // Create and activate another persistent working set
    m_area->setWorkingSet(setName);
    auto set = m_workingSetCtrl->workingSet(m_area->workingSet());
    set->setPersistent(true);
    m_documentCtrl->openDocument(QUrl::fromLocalFile(m_file.fileName()));
    m_documentCtrl->openDocumentFromText(QString()); // add an unsaved document

    Core::self()->uiController()->switchToArea(QStringLiteral("debug"), IUiController::ThisWindow);
    m_area_debug = Core::self()->uiControllerInternal()->activeArea();
     // explicitly set the current working set, as in DebugController::addSession
    m_area_debug->setWorkingSet(setName, m_area->workingSetPersistent(), m_area);

    QTRY_COMPARE(m_closedSets->layout()->count(), 1); // working set 2
    QTRY_COMPARE(m_area_debug->views().size(), 2); // check that the unsaved document is displayed, BUG 486746

    m_area_debug->setWorkingSet(setName2);
    m_documentCtrl->openDocumentFromText(QString()); // add two unsaved documents
    m_documentCtrl->openDocumentFromText(QString());

    QTest::qSleep(1000);
    QCOMPARE(m_closedSets->layout()->count(), 1); // working set 1, BUG 375446

    Core::self()->uiController()->switchToArea(QStringLiteral("code"), IUiController::ThisWindow);
     // explicitly set the current working set, as in DebugController::debuggerStateChanged
    m_area->setWorkingSet(setName2, m_area_debug->workingSetPersistent(), m_area_debug);

    QTRY_COMPARE(m_area->views().size(), 3); // check that both unsaved documents are displayed, BUG 486746

    m_area->setWorkingSet(setName);

    QTRY_COMPARE(m_closedSets->layout()->count(), 1); // working set 2, BUG 375446
}

void TestWorkingSetController::restoreSplits()
{
    // we need to show the window to calculate actual widget sizes
    m_mainWindow->show();

    // Create a persistent working set with 4 split views
    m_area->setWorkingSet(setName);
    auto set2 = m_workingSetCtrl->workingSet(m_area->workingSet());
    set2->setPersistent(true);
    auto doc = m_documentCtrl->openDocument(QUrl::fromLocalFile(m_file.fileName()));

    // Split view
    auto view_bottom_left = dynamic_cast<Sublime::Document*>(doc)->createView();
    m_area->addView(view_bottom_left, m_area->activeView(), Qt::Vertical);
    auto view_top_right = dynamic_cast<Sublime::Document*>(doc)->createView();
    m_area->addView(view_top_right, m_area->activeView(), Qt::Horizontal);
    auto view_bottom_right = dynamic_cast<Sublime::Document*>(doc)->createView();
    m_area->addView(view_bottom_right, view_bottom_left, Qt::Horizontal);

    // required to arrange the view widgets in the window
    QApplication::processEvents();

    QSet<Sublime::AreaIndex*> indices;
    for (auto &view : m_area->views()) {
        indices.insert(m_area->indexOf(view));
    }
    QCOMPARE(indices.size(), 4); // number of view containers

    auto *splitter_bottom = qobject_cast<QSplitter*>(view_bottom_left->widget()
                                                         ->parentWidget()
                                                         ->parentWidget()   // view container
                                                         ->parentWidget()   // splitter containing a single container
                                                         ->parentWidget()); // splitter containing lower containers
    QVERIFY(splitter_bottom);

    QVERIFY(splitter_bottom->isVisible());
    // ensure the splitter handle is visible, which is esp. required for running the test with the offscreen QPA
    splitter_bottom->handle(0)->setVisible(true);
    QVERIFY(splitter_bottom->handle(0)->isVisible());

    QApplication::processEvents();

    splitter_bottom->setSizes({1, 1000});
    auto sizes_bottom = splitter_bottom->sizes();
    QVERIFY(sizes_bottom.at(0) < sizes_bottom.at(1));

    auto *splitter_mid = qobject_cast<QSplitter*>(splitter_bottom->parentWidget());
    QVERIFY(splitter_mid);
    splitter_mid->setSizes({1, 1000});
    auto sizes_mid = splitter_mid->sizes();
    QVERIFY(sizes_mid.at(0) < sizes_mid.at(1));

    QVERIFY(!qobject_cast<QSplitter*>(splitter_mid->parentWidget()));

    // Create and activate another persistent working set with a single view
    m_area->setWorkingSet(setName2);
    auto set = m_workingSetCtrl->workingSet(m_area->workingSet());
    set->setPersistent(true);
    m_documentCtrl->openDocument(QUrl::fromLocalFile(m_file.fileName()));

    indices.clear();
    for (auto &view : m_area->views()) {
        indices.insert(m_area->indexOf(view));
    }
    QCOMPARE(indices.size(), 1);

    m_area->setWorkingSet(setName);

    QApplication::processEvents();

    indices.clear();
    for (auto &view : m_area->views()) {
        indices.insert(m_area->indexOf(view));
    }
    QCOMPARE(indices.size(), 4);
    view_bottom_left = m_area->views().at(2);

    // check if the splitter sizes are restored
    splitter_bottom = qobject_cast<QSplitter*>(view_bottom_left->widget()
                                                   ->parentWidget()
                                                   ->parentWidget()   // view container
                                                   ->parentWidget()   // splitter containing a single container
                                                   ->parentWidget()); // splitter containing lower containers
    QVERIFY(splitter_bottom);
    QCOMPARE(splitter_bottom->sizes(), sizes_bottom);

    splitter_mid = qobject_cast<QSplitter*>(splitter_bottom->parentWidget());
    QVERIFY(splitter_mid);
    QCOMPARE(splitter_mid->sizes(), sizes_mid);
}

QTEST_MAIN(TestWorkingSetController)

#include "moc_test_workingsets.cpp"
