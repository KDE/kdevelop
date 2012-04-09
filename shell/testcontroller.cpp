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

#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/itestprovider.h>
#include <interfaces/iruncontroller.h>
#include <util/executecompositejob.h>

#include <KUrl>
#include <KLocalizedString>
#include <KDebug>

using namespace KDevelop;

class TestController::TestControllerPrivate
{
public:
    QList<ITestSuite*> suites;
};

TestController::TestController(QObject *parent)
: ITestController(parent)
, d(new TestControllerPrivate)
{
}

TestController::~TestController()
{
    delete d;
}

void TestController::initialize()
{

}

void TestController::cleanup()
{
    d->suites.clear();
}

QList< KDevelop::ITestSuite* > TestController::testSuites() const
{
    return d->suites;
}

void TestController::removeTestSuite(KDevelop::ITestSuite* suite)
{
    d->suites.removeAll(suite);
    emit testSuiteRemoved(suite);
}

void TestController::addTestSuite(KDevelop::ITestSuite* suite)
{
    d->suites.append(suite);
    emit testSuiteAdded(suite);
}

ITestSuite* TestController::findTestSuite(IProject* project, const QString& name) const
{
    foreach (ITestSuite* suite, testSuitesForProject(project))
    {
        if (suite->name() == name)
        {
            return suite;
        }
    }
    return 0;
}


QList< ITestSuite* > TestController::testSuitesForProject(IProject* project) const
{
    QList<ITestSuite*> suites;
    foreach (ITestSuite* suite, d->suites)
    {
        if (suite->project() == project)
        {
            suites << suite;
        }
    }
    return suites;
}

KJob* TestController::reloadTestSuites()
{
    QList<KJob*> jobs;
    foreach (IPlugin* plugin, ICore::self()->pluginController()->allPluginsForExtension("org.kdevelop.ITestProvider"))
    {
        if (ITestProvider* provider = plugin->extension<ITestProvider>())
        {
            if (KJob* job = provider->findTests())
            {
                jobs << job;
            }
        }
    }
    if (!jobs.isEmpty())
    {
        ExecuteCompositeJob* compositeJob = new ExecuteCompositeJob(this, jobs);
        compositeJob->setObjectName("Searching for unit tests");
        return compositeJob;
    }
    return 0;
}

void TestController::notifyTestRunFinished(ITestSuite* suite)
{
    kDebug() << "Test run finished for suite" << suite->name();
    emit testRunFinished(suite);
}

#include "testcontroller.moc"
