/*
 * Copyright 2015 Laszlo Kis-Adam <laszlo.kis-adam@kdemail.net>
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

#include <QTest>
#include <QAction>
#include <QTabWidget>

#include "../problemsview.h"

#include <tests/testcore.h>
#include <tests/autotestshell.h>

#include <interfaces/ilanguagecontroller.h>
#include <shell/problemmodelset.h>
#include <shell/problemmodel.h>
#include <shell/problem.h>

using namespace KDevelop;

class TestProblemsView : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testLoad();
    void testAddModel();
    void testSwitchTab();
    void testRemoveModel();
    void testAddRemoveProblems();
    void testSetProblems();

private:
    QTabWidget* tabWidget();

    QScopedPointer<ProblemsView> m_view;
};

void TestProblemsView::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);

    ProblemModelSet* pms = ICore::self()->languageController()->problemModelSet();
    auto* model = new ProblemModel(pms);
    IProblem::Ptr p(new DetectedProblem());
    model->addProblem(p);
    pms->addModel(QStringLiteral("MODEL1_ID"), QStringLiteral("MODEL1"), model);

    m_view.reset(new ProblemsView());
}

void TestProblemsView::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestProblemsView::testLoad()
{
    m_view->load();

    // Check that the initial model's tab shows up
    QTabWidget* tab = tabWidget();
    QVERIFY(tab);
    QCOMPARE(tab->count(), 1);
    QCOMPARE(tab->tabText(0), QStringLiteral("MODEL1 (1)"));
}

void TestProblemsView::testAddModel()
{
    ProblemModelSet* pms = ICore::self()->languageController()->problemModelSet();
    pms->addModel(QStringLiteral("MODEL2_ID"), QStringLiteral("MODEL2"), new ProblemModel(pms));

    QTabWidget* tab = tabWidget();
    QVERIFY(tab);
    QCOMPARE(tab->count(), 2);
    QCOMPARE(tab->tabText(0), QStringLiteral("MODEL1 (1)"));
    QCOMPARE(tab->tabText(1), QStringLiteral("MODEL2 (0)"));
}

QVector<bool> visibilites(const QList<QAction*>& actions)
{
    QVector<bool> visibilites;
    for (auto action : actions) {
        visibilites << action->isVisible();
    }
    return visibilites;
}

void TestProblemsView::testSwitchTab()
{
    QTabWidget* tab = tabWidget();
    QVERIFY(tab);

    // Check that the current widget's actions are in the toolbar
    QWidget* oldWidget = tab->currentWidget();
    QVERIFY(oldWidget);
    const auto oldVisibilites = visibilites(m_view->actions());

    tab->setCurrentIndex(1);

    // Check that the new widget's actions are in the toolbar
    QWidget* newWidget = tab->currentWidget();
    QVERIFY(newWidget);
    QVERIFY(newWidget != oldWidget);
    const auto newVisibilites = visibilites(m_view->actions());
    QCOMPARE(oldVisibilites, newVisibilites);
}

void TestProblemsView::testRemoveModel()
{
    // Remove the model
    ProblemModelSet* pms = ICore::self()->languageController()->problemModelSet();
    ProblemModel* model = pms->findModel(QStringLiteral("MODEL1_ID"));
    QVERIFY(model);
    pms->removeModel(QStringLiteral("MODEL1_ID"));
    delete model;
    model = nullptr;

    // Now let's see if the view has been updated!
    QTabWidget* tab = tabWidget();
    QVERIFY(tab);
    QCOMPARE(tab->count(), 1);
    QCOMPARE(tab->tabText(0), QStringLiteral("MODEL2 (0)"));
}

void TestProblemsView::testAddRemoveProblems()
{
    ProblemModelSet* pms = ICore::self()->languageController()->problemModelSet();
    ProblemModel* model = pms->findModel(QStringLiteral("MODEL2_ID"));
    QVERIFY(model);

    QTabWidget* tab = tabWidget();
    QVERIFY(tab);

    // Make sure there are no problems right now
    model->clearProblems();
    QCOMPARE(tab->tabText(0), QStringLiteral("MODEL2 (0)"));

    // Let's add some problems
    int c = 0;
    for (int i = 0; i < 3; i++) {
        IProblem::Ptr p(new DetectedProblem());
        model->addProblem(p);
        c++;

        // Check if the view has noticed the addition
        QString label = QStringLiteral("MODEL2 (%1)").arg(c);
        QCOMPARE(tab->tabText(0), label);
    }

    // Clear the problems
    model->clearProblems();

    // Check if the view has noticed the clear
    QCOMPARE(tab->tabText(0), QStringLiteral("MODEL2 (0)"));
}

void TestProblemsView::testSetProblems()
{
    ProblemModelSet* pms = ICore::self()->languageController()->problemModelSet();
    ProblemModel* model = pms->findModel(QStringLiteral("MODEL2_ID"));
    QVERIFY(model);

    QTabWidget* tab = tabWidget();
    QVERIFY(tab);

    // Make sure there are no problems right now
    model->clearProblems();
    QCOMPARE(tab->tabText(0), QStringLiteral("MODEL2 (0)"));

    // Build a problem vector and set the problems
    QVector<IProblem::Ptr> problems;
    for (int i = 0; i < 3; i++) {
        IProblem::Ptr p(new DetectedProblem());
        problems.push_back(p);
    }
    model->setProblems(problems);

    // Check if the view has noticed
    QCOMPARE(tab->tabText(0), QStringLiteral("MODEL2 (3)"));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

QTabWidget* TestProblemsView::tabWidget()
{
    auto* tab = m_view->findChild<QTabWidget*>();
    return tab;
}

QTEST_MAIN(TestProblemsView)

#include "test_problemsview.moc"
