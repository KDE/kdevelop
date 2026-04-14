/*
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QLoggingCategory>
#include <QSignalSpy>
#include <QTest>

#include <KJob>

#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <project/interfaces/iprojectbuilder.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include "../../cmake/tests/testhelpers.h"

class TestCMakeBuilder : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void testConfigureEmitsManualConfiguredForExplicitRequest();
    void testConfigureDoesNotEmitManualConfiguredForAutomaticRequest();
    void testConfigureDoesNotEmitConfiguredOnFailure();
};

QTEST_MAIN(TestCMakeBuilder)

using namespace KDevelop;

namespace {
IPlugin* cmakeBuilderPlugin()
{
    return ICore::self()->pluginController()->pluginForExtension(QStringLiteral("org.kdevelop.IProjectBuilder"),
                                                                 QStringLiteral("KDevCMakeBuilder"));
}

IProjectBuilder* cmakeBuilder()
{
    auto* plugin = cmakeBuilderPlugin();
    return plugin ? plugin->extension<IProjectBuilder>() : nullptr;
}
}

void TestCMakeBuilder::initTestCase()
{
    AutoTestShell::init({QStringLiteral("KDevCMakeManager"), QStringLiteral("KDevCMakeBuilder"),
                         QStringLiteral("KDevMakeBuilder"), QStringLiteral("KDevStandardOutputView")});
    TestCore::initialize();

    qRegisterMetaType<IProject*>();
    qRegisterMetaType<IProjectBuilder::ConfigureRequest>();
    cleanup();
}

void TestCMakeBuilder::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestCMakeBuilder::cleanup()
{
    const auto projects = ICore::self()->projectController()->projects();
    for (IProject* project : projects) {
        ICore::self()->projectController()->closeProject(project);
    }
    QVERIFY(ICore::self()->projectController()->projects().isEmpty());
}

void TestCMakeBuilder::testConfigureEmitsManualConfiguredForExplicitRequest()
{
    IProject* project = loadProject(QStringLiteral("tiny_project"));
    auto* builder = cmakeBuilder();
    auto* plugin = cmakeBuilderPlugin();
    QVERIFY(builder);
    QVERIFY(plugin);

    QSignalSpy configuredSpy(plugin, SIGNAL(configured(KDevelop::IProject*)));
    QSignalSpy configuredWithRequestSpy(
        plugin, SIGNAL(configured(KDevelop::IProject*, KDevelop::IProjectBuilder::ConfigureRequest)));
    QSignalSpy configurationChangedSpy(ICore::self()->projectController(),
                                       &IProjectController::projectConfigurationChanged);

    auto* job = builder->configure(project, IProjectBuilder::ConfigureRequest::Explicit);
    QVERIFY(job);
    QVERIFY(job->exec());

    QCOMPARE(configuredSpy.count(), 1);
    QCOMPARE(qvariant_cast<IProject*>(configuredSpy.takeFirst().at(0)), project);

    QCOMPARE(configuredWithRequestSpy.count(), 1);
    const auto configuredWithRequestArgs = configuredWithRequestSpy.takeFirst();
    QCOMPARE(qvariant_cast<IProject*>(configuredWithRequestArgs.at(0)), project);
    QCOMPARE(qvariant_cast<IProjectBuilder::ConfigureRequest>(configuredWithRequestArgs.at(1)),
             IProjectBuilder::ConfigureRequest::Explicit);

    configurationChangedSpy.wait();
    QCOMPARE(configurationChangedSpy.count(), 1);
    QCOMPARE(qvariant_cast<IProject*>(configurationChangedSpy.takeFirst().at(0)), project);
    QVERIFY(project->isReady());
}

void TestCMakeBuilder::testConfigureDoesNotEmitManualConfiguredForAutomaticRequest()
{
    IProject* project = loadProject(QStringLiteral("tiny_project"));
    auto* builder = cmakeBuilder();
    auto* plugin = cmakeBuilderPlugin();
    QVERIFY(builder);
    QVERIFY(plugin);

    QSignalSpy configuredSpy(plugin, SIGNAL(configured(KDevelop::IProject*)));
    QSignalSpy configuredWithRequestSpy(
        plugin, SIGNAL(configured(KDevelop::IProject*, KDevelop::IProjectBuilder::ConfigureRequest)));

    // ConfigureRequest defaulted to IProjectBuilder::ConfigureRequest::Automatic
    auto* job = builder->configure(project);
    QVERIFY(job);
    QVERIFY(job->exec());

    QCOMPARE(configuredSpy.count(), 1);
    QCOMPARE(qvariant_cast<IProject*>(configuredSpy.takeFirst().at(0)), project);

    QCOMPARE(configuredWithRequestSpy.count(), 1);
    const auto configuredWithRequestArgs = configuredWithRequestSpy.takeFirst();
    QCOMPARE(qvariant_cast<IProject*>(configuredWithRequestArgs.at(0)), project);
    QCOMPARE(qvariant_cast<IProjectBuilder::ConfigureRequest>(configuredWithRequestArgs.at(1)),
             IProjectBuilder::ConfigureRequest::Automatic);
}

void TestCMakeBuilder::testConfigureDoesNotEmitConfiguredOnFailure()
{
    IProject* project = loadProject(QStringLiteral("faulty_target"));
    auto* builder = cmakeBuilder();
    auto* plugin = cmakeBuilderPlugin();
    QVERIFY(builder);
    QVERIFY(plugin);

    QSignalSpy configuredSpy(plugin, SIGNAL(configured(KDevelop::IProject*)));
    QSignalSpy configuredWithRequestSpy(
        plugin, SIGNAL(configured(KDevelop::IProject*, KDevelop::IProjectBuilder::ConfigureRequest)));

    auto* job = builder->configure(project, IProjectBuilder::ConfigureRequest::Explicit);
    QVERIFY(job);
    QVERIFY(!job->exec());

    QCOMPARE(configuredSpy.count(), 0);
    QCOMPARE(configuredWithRequestSpy.count(), 0);
}

#include "test_cmakebuilder.moc"
