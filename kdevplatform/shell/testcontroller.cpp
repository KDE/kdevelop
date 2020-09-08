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

    d->suites.removeAll(suite);
    emit testSuiteRemoved(suite);
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
    Q_D(const TestController);

    const auto it = std::find_if(d->suites.cbegin(), d->suites.cend(), [&](ITestSuite* suite) {
        return suite->project() == project && suite->name() == name;
    });

    return (it != d->suites.cend()) ? *it : nullptr;
}


QList< ITestSuite* > TestController::testSuitesForProject(IProject* project) const
{
    Q_D(const TestController);

    QList<ITestSuite*> suites;
    for (ITestSuite* suite : qAsConst(d->suites)) {
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


