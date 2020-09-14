/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; see the file COPYING.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "testcontroller.h"
#include "interfaces/itestsuite.h"
#include "debug.h"
#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>

#include <KLocalizedString>

#include <QHash>
#include <QList>
#include <QtAlgorithms>

using namespace KDevelop;

class KDevelop::TestControllerPrivate
{
public:
    void assertRegistered(ITestSuite* suite) const {
        Q_ASSERT_X(suite && suites.value(suite->project()).contains(suite), Q_FUNC_INFO,
                   "Either this suite hasn't been added yet or it is already removed.");
    }

    // The test suites are owned and destroyed by this object, but stored as
    // raw pointers to allow implementing testSuitesForProject() efficiently.
    QHash<IProject*, QList<ITestSuite*>> suites;
};

TestController::TestController(QObject *parent)
: ITestController(parent)
, d_ptr(new TestControllerPrivate)
{
}

TestController::~TestController()
{
    Q_D(const TestController);

    for (auto& list : qAsConst(d->suites)) {
        qDeleteAll(list);
    }
}

void TestController::initialize()
{
    Q_D(const TestController);
    Q_ASSERT(d->suites.empty());

    const auto* const projectController = ICore::self()->projectController();
    Q_ASSERT_X(projectController->projectCount() == 0, Q_FUNC_INFO,
              "If projects somehow get opened this early, we must add them to d->suites."
              " But we cannot add projects that are currently being opened!"
              " So we require that project opening begins after this initialization.");

    connect(projectController, &IProjectController::projectAboutToBeOpened,
            this, &TestController::projectOpening);
    connect(projectController, &IProjectController::projectOpeningAborted,
            this, &TestController::projectClosing);
    connect(projectController, &IProjectController::projectClosing,
            this, &TestController::projectClosing);
}

void TestController::cleanup()
{
    // Rely on ProjectController::cleanup() to close all projects and thus
    // trigger the removal of all test suites.
}

void TestController::addTestSuite(std::unique_ptr<ITestSuite> suite)
{
    Q_D(TestController);

    const auto it = d->suites.find(suite->project());
    Q_ASSERT_X(it != d->suites.end(), Q_FUNC_INFO,
               "suite->project() must be registered as a precondition.");

    auto* const addedSuite = suite.release();
    it->push_back(addedSuite);
    emit testSuiteAdded(addedSuite);
}

bool TestController::removeTestSuitesForProject(IProject* project)
{
    Q_D(TestController);

    const auto it = d->suites.find(project);
    if (it == d->suites.end()) {
        return false;
    }

    QList<ITestSuite*> suitesForProject;
    // Remove project from d->suites only in projectClosing().
    it->swap(suitesForProject);
    emit testSuitesForProjectRemoved(project);
    qDeleteAll(suitesForProject);
    return true;
}

QList<ITestSuite*> TestController::testSuites() const
{
    Q_D(const TestController);

    QList<ITestSuite*> allSuites;
    for (auto& list : qAsConst(d->suites)) {
        allSuites << list;
    }
    return allSuites;
}

QList< ITestSuite* > TestController::testSuitesForProject(IProject* project) const
{
    Q_D(const TestController);

    return d->suites.value(project);
}

void TestController::notifyTestRunFinished(ITestSuite* suite, const TestResult& result)
{
    Q_D(const TestController);
    d->assertRegistered(suite);

    qCDebug(SHELL) << "Test run finished for suite" << suite->name();
    emit testRunFinished(suite, result);
}

void TestController::notifyTestRunStarted(ITestSuite* suite, const QStringList& test_cases)
{
    Q_D(const TestController);
    d->assertRegistered(suite);

    qCDebug(SHELL) << "Test run started for suite" << suite->name();
    emit testRunStarted(suite, test_cases);
}

void TestController::projectOpening(IProject* project)
{
    Q_D(TestController);

    Q_ASSERT(!d->suites.contains(project));
    d->suites.insert(project, {});
}

void TestController::projectClosing(IProject* project)
{
    Q_D(TestController);

    Q_ASSERT(d->suites.contains(project));
    const auto suitesForProject = d->suites.take(project);
    emit testSuitesForProjectRemoved(project);
    qDeleteAll(suitesForProject);
}
