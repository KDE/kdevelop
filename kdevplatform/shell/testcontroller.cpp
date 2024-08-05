/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "testcontroller.h"
#include "interfaces/itestsuite.h"
#include "debug.h"
#include <interfaces/icore.h>

#include <KLocalizedString>

using namespace KDevelop;

class KDevelop::TestControllerPrivate
{
public:
    QList<ITestSuite*> suites;
};

TestController::TestController(QObject *parent)
: ITestController(parent)
, d_ptr(new TestControllerPrivate)
{
}

TestController::~TestController() = default;

void TestController::initialize()
{

}

void TestController::cleanup()
{
    Q_D(TestController);

    d->suites.clear();
}

QList<ITestSuite*> TestController::testSuites() const
{
    Q_D(const TestController);

    return d->suites;
}

void TestController::removeTestSuite(ITestSuite* suite)
{
    Q_D(TestController);

    if (d->suites.removeAll(suite)) {
        emit testSuiteRemoved(suite);
    }
}

void TestController::addTestSuite(ITestSuite* suite)
{
    Q_D(TestController);

    if (ITestSuite* existingSuite = findTestSuite(suite->project(), suite->name()))
    {
        if (existingSuite == suite) {
            return;
        }
        removeTestSuite(existingSuite);
        delete existingSuite;
    }
    d->suites.append(suite);
    if(!ICore::self()->shuttingDown())
        emit testSuiteAdded(suite);
}

ITestSuite* TestController::findTestSuite(IProject* project, const QString& name) const
{
    const auto suites = testSuitesForProject(project);
    auto it = std::find_if(suites.begin(), suites.end(), [&](ITestSuite* suite) {
        return (suite->name() == name);
    });

    return (it != suites.end()) ? *it : nullptr;
}


QList< ITestSuite* > TestController::testSuitesForProject(IProject* project) const
{
    Q_D(const TestController);

    QList<ITestSuite*> suites;
    for (ITestSuite* suite : std::as_const(d->suites)) {
        if (suite->project() == project)
        {
            suites << suite;
        }
    }
    return suites;
}

void TestController::notifyTestRunFinished(ITestSuite* suite, const TestResult& result)
{
    qCDebug(SHELL) << "Test run finished for suite" << suite->name();
    emit testRunFinished(suite, result);
}

void TestController::notifyTestRunStarted(ITestSuite* suite, const QStringList& test_cases)
{
    qCDebug(SHELL) << "Test run started for suite" << suite->name();
    emit testRunStarted(suite, test_cases);
}

#include "moc_testcontroller.cpp"
