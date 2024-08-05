/*
    SPDX-FileCopyrightText: 2012 Miha ?an?ula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "testviewplugin.h"
#include "testview.h"

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/itestsuite.h>
#include <interfaces/itestcontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <util/executecompositejob.h>

#include <KPluginFactory>
#include <KLocalizedString>
#include <KActionCollection>

#include <QAction>

K_PLUGIN_FACTORY_WITH_JSON(TestViewFactory, "kdevtestview.json", registerPlugin<TestViewPlugin>();)

using namespace KDevelop;

class TestToolViewFactory: public KDevelop::IToolViewFactory
{
    public:
        explicit TestToolViewFactory( TestViewPlugin *plugin ): mplugin( plugin )
        {}
        QWidget* create( QWidget *parent = nullptr ) override
        {
            return new TestView( mplugin, parent );
        }
        Qt::DockWidgetArea defaultPosition() const override
        {
            return Qt::LeftDockWidgetArea;
        }
        QString id() const override
        {
            return QStringLiteral("org.kdevelop.TestView");
        }
        QList< QAction* > contextMenuActions(QWidget* viewWidget) const override
        {
            return qobject_cast<TestView*>(viewWidget)->contextMenuActions();
        }
    private:
        TestViewPlugin *mplugin;
};

TestViewPlugin::TestViewPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& args)
    : IPlugin(QStringLiteral("kdevtestview"), parent, metaData)
{
    Q_UNUSED(args)

    auto* runAll = new QAction( QIcon::fromTheme(QStringLiteral("system-run")), i18nc("@action", "Run All Tests"), this );
    connect(runAll, &QAction::triggered, this, &TestViewPlugin::runAllTests);
    actionCollection()->addAction(QStringLiteral("run_all_tests"), runAll);
    
    auto* stopTest = new QAction( QIcon::fromTheme(QStringLiteral("process-stop")), i18nc("@action", "Stop Running Tests"), this );
    connect(stopTest, &QAction::triggered, this, &TestViewPlugin::stopRunningTests);
    actionCollection()->addAction(QStringLiteral("stop_running_tests"), stopTest);

    setXMLFile(QStringLiteral("kdevtestview.rc"));

    m_viewFactory = new TestToolViewFactory(this);
    core()->uiController()->addToolView(i18nc("@title:window", "Unit Tests"), m_viewFactory);
    
    connect(core()->runController(),&IRunController::jobRegistered, this, &TestViewPlugin::jobStateChanged);
    connect(core()->runController(),&IRunController::jobUnregistered, this, &TestViewPlugin::jobStateChanged);
    
    jobStateChanged();
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
    ITestController* tc = core()->testController();
    const auto projects = core()->projectController()->projects();
    for (IProject* project : projects) {
        QList<KJob*> jobs;
        const auto suites = tc->testSuitesForProject(project);
        for (ITestSuite* suite : suites) {
            if (KJob* job = suite->launchAllCases(ITestSuite::Silent))
            {
                jobs << job;
            }
        }
        if (!jobs.isEmpty())
        {
            auto* compositeJob = new KDevelop::ExecuteCompositeJob(this, jobs);
            compositeJob->setObjectName(i18np("Run 1 test in %2", "Run %1 tests in %2",
                                              jobs.size(), project->name()));
            compositeJob->setProperty("test_job", true);
            core()->runController()->registerJob(compositeJob);
        }
    }
}

void TestViewPlugin::stopRunningTests()
{
    const auto jobs = core()->runController()->currentJobs();
    for (KJob* job : jobs) {
        if (job->property("test_job").toBool())
        {
            job->kill();
        }
    }
}

void TestViewPlugin::jobStateChanged()
{
    const auto jobs = core()->runController()->currentJobs();
    const bool found = std::any_of(jobs.begin(), jobs.end(), [](KJob* job) {
        return (job->property("test_job").toBool());
    });

    actionCollection()->action(QStringLiteral("run_all_tests"))->setEnabled(!found);
    actionCollection()->action(QStringLiteral("stop_running_tests"))->setEnabled(found);
}

#include "testviewplugin.moc"
#include "moc_testviewplugin.cpp"
