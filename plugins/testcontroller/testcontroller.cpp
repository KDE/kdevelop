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

#include <KUrl>
#include <KPluginFactory>
#include <KLocalizedString>
#include <KAboutData>

K_PLUGIN_FACTORY(TestControllerFactory, registerPlugin<TestController>(); )
K_EXPORT_PLUGIN(TestControllerFactory(KAboutData("kdevtestcontroller","kdevtestcontroller", ki18n("Test Controller"), "0.1", ki18n("Manages unit tests"), KAboutData::License_GPL)))

using namespace KDevelop;

class TestControllerPrivate
{
public:
    QList<ITestSuite*> suites;
};

TestController::TestController(QObject *parent, const QVariantList &args) : IPlugin(TestControllerFactory::componentData(), parent),
d(new TestControllerPrivate)
{
    KDEV_USE_EXTENSION_INTERFACE(ITestController);
    Q_UNUSED(args)
}

TestController::~TestController()
{
    delete d;
}

QList< KDevelop::ITestSuite* > TestController::testSuites() const
{
    return d->suites;
}

void TestController::removeTestSuite(KDevelop::ITestSuite* suite)
{
    d->suites.removeAll(suite);
}

void TestController::addTestSuite(KDevelop::ITestSuite* suite)
{
    d->suites.append(suite);
}

ITestSuite* TestController::testSuiteForUrl(const KUrl& url) const
{
    foreach(ITestSuite* suite, d->suites)
    {
        if (suite->url() == url)
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


