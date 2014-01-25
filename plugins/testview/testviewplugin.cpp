/*  This file is part of KDevelop
    Copyright 2012 Miha ?an?ula <miha@noughmad.eu>

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

#include "testviewplugin.h"
#include "testview.h"

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/itestsuite.h>
#include <interfaces/itestcontroller.h>
#include <interfaces/ilaunchconfiguration.h>
#include <interfaces/launchconfigurationtype.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/ilauncher.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <util/executecompositejob.h>

#include <KPluginFactory>
#include <KAboutData>
#include <KLocalizedString>
#include <KActionCollection>

#include <QAction>

K_PLUGIN_FACTORY(TestViewFactory, registerPlugin<TestViewPlugin>(); )
// K_EXPORT_PLUGIN(TestViewFactory(KAboutData("kdevtestview","kdevtestview", ki18n("Unit Test View"), "0.1", ki18n("Lets you see and run unit tests"), KAboutData::License_GPL)))

using namespace KDevelop;

class TestToolViewFactory: public KDevelop::IToolViewFactory
{
    public:
        TestToolViewFactory( TestViewPlugin *plugin ): mplugin( plugin )
        {}
        virtual QWidget* create( QWidget *parent = 0 )
        {
            return new TestView( mplugin, parent );
        }
        virtual Qt::DockWidgetArea defaultPosition()
        {
            return Qt::LeftDockWidgetArea;
        }
        virtual QString id() const
        {
            return "org.kdevelop.TestView";
        }
        virtual QList< QAction* > contextMenuActions(QWidget* viewWidget) const
        {
            return qobject_cast<TestView*>(viewWidget)->contextMenuActions();
        }
    private:
        TestViewPlugin *mplugin;
};

TestViewPlugin::TestViewPlugin(QObject* parent, const QVariantList& args)
    : IPlugin("kdevtestview", parent)
{
    Q_UNUSED(args)

    QAction* runAll = new QAction( QIcon::fromTheme("system-run"), i18n("Run All Tests"), this );
    connect(runAll, SIGNAL(triggered(bool)), SLOT(runAllTests()));
    actionCollection()->addAction("run_all_tests", runAll);

    setXMLFile("kdevtestview.rc");

    m_viewFactory = new TestToolViewFactory(this);
    core()->uiController()->addToolView(i18n("Unit Tests"), m_viewFactory);
}

TestViewPlugin::~TestViewPlugin()
{

}

void TestViewPlugin::unload()
{
    core()->uiController()->removeToolView(m_viewFactory);
}

void TestViewPlugin::runAllTests()
{
    QString jobName = "Run %1 tests in %2";
    ITestController* tc = core()->testController();
    foreach (IProject* project, core()->projectController()->projects())
    {
        QList<KJob*> jobs;
        foreach (ITestSuite* suite, tc->testSuitesForProject(project))
        {
            if (KJob* job = suite->launchAllCases(ITestSuite::Silent))
            {
                jobs << job;
            }
        }
        if (!jobs.isEmpty())
        {
            KDevelop::ExecuteCompositeJob* compositeJob = new KDevelop::ExecuteCompositeJob(this, jobs);
            compositeJob->setObjectName(i18np("Run 1 test in %2", "Run %1 tests in %2",
                                              jobs.size(), project->name()));
            core()->runController()->registerJob(compositeJob);
        }
    }
}

#include "testviewplugin.moc"
