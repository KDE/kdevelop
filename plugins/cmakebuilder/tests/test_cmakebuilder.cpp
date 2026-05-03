/*
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QLoggingCategory>
#include <QSignalSpy>
#include <QTest>

#include <KJob>

#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <language/backgroundparser/backgroundparser.h>
#include <project/interfaces/iprojectbuilder.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include "../../cmake/tests/testhelpers.h"

class TestCMakeBuilder : public QObject
{
    Q_OBJECT

    KDevelop::IProject* project = nullptr;

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void testConfigureEmitsManualConfiguredForExplicitRequest();
    void testConfigureDoesNotEmitManualConfiguredForAutomaticRequest();
    void testConfigureEmitsConfiguredOnFailure();
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
    if (!project) {
        return;
    }

    QSignalSpy projectClosedSpy(KDevelop::ICore::self()->projectController(),
                                SIGNAL(projectClosed(KDevelop::IProject*)));
    Q_ASSERT(projectClosedSpy.isValid());

    ICore::self()->projectController()->closeProject(project);
    if (projectClosedSpy.empty() && !projectClosedSpy.wait(60'000)) {
        qFatal("Timeout while waiting for closed signal");
    }

    project = nullptr;
}

void TestCMakeBuilder::testConfigureEmitsManualConfiguredForExplicitRequest()
{
    project = loadProject(QStringLiteral("tiny_project"));
    auto* builder = cmakeBuilder();
    auto* plugin = cmakeBuilderPlugin();
    QVERIFY(builder);
    QVERIFY(plugin);

    QSignalSpy configuredWithRequestSpy(
        plugin, SIGNAL(configured(KDevelop::IProject*, KDevelop::IProjectBuilder::ConfigureRequest)));
    QSignalSpy projectConfigurationChangedSpy(ICore::self()->projectController(),
                                              &IProjectController::projectConfigurationChanged);

    auto* job = builder->configure(project, IProjectBuilder::ConfigureRequest::Explicit);
    QVERIFY(job);
    QVERIFY(job->exec());

    QCOMPARE(configuredWithRequestSpy.count(), 1);
    const auto configuredWithRequestArgs = configuredWithRequestSpy.takeFirst();
    QCOMPARE(qvariant_cast<IProject*>(configuredWithRequestArgs.at(0)), project);
    QCOMPARE(qvariant_cast<IProjectBuilder::ConfigureRequest>(configuredWithRequestArgs.at(1)),
             IProjectBuilder::ConfigureRequest::Explicit);

    // Should be emitted on explicit configuration request
    projectConfigurationChangedSpy.empty() && projectConfigurationChangedSpy.wait();
    QCOMPARE(projectConfigurationChangedSpy.count(), 1);
}

void TestCMakeBuilder::testConfigureDoesNotEmitManualConfiguredForAutomaticRequest()
{
    project = loadProject(QStringLiteral("tiny_project"));
    auto* builder = cmakeBuilder();
    auto* plugin = cmakeBuilderPlugin();
    QVERIFY(builder);
    QVERIFY(plugin);

    QSignalSpy configuredWithRequestSpy(
        plugin, SIGNAL(configured(KDevelop::IProject*, KDevelop::IProjectBuilder::ConfigureRequest)));

    // ConfigureRequest defaulted to IProjectBuilder::ConfigureRequest::Automatic
    auto* job = builder->configure(project);
    QVERIFY(job);
    QVERIFY(job->exec());

    QCOMPARE(configuredWithRequestSpy.count(), 1);
    const auto configuredWithRequestArgs = configuredWithRequestSpy.takeFirst();
    QCOMPARE(qvariant_cast<IProject*>(configuredWithRequestArgs.at(0)), project);
    QCOMPARE(qvariant_cast<IProjectBuilder::ConfigureRequest>(configuredWithRequestArgs.at(1)),
             IProjectBuilder::ConfigureRequest::Automatic);
}

void TestCMakeBuilder::testConfigureEmitsConfiguredOnFailure()
{
    project = loadProject(QStringLiteral("faulty_target"));
    auto* builder = cmakeBuilder();
    auto* plugin = cmakeBuilderPlugin();
    QVERIFY(builder);
    QVERIFY(plugin);

    QSignalSpy configuredWithRequestSpy(
        plugin, SIGNAL(configured(KDevelop::IProject*, KDevelop::IProjectBuilder::ConfigureRequest)));
    QSignalSpy projectConfigurationChangedSpy(ICore::self()->projectController(),
                                              &IProjectController::projectConfigurationChanged);

    auto* job = builder->configure(project, IProjectBuilder::ConfigureRequest::Explicit);
    QVERIFY(job);
    QVERIFY(!job->exec());

    QCOMPARE(configuredWithRequestSpy.count(), 1);

    // Should be emitted on explicit configuration request
    projectConfigurationChangedSpy.empty() && projectConfigurationChangedSpy.wait();
    QCOMPARE(projectConfigurationChangedSpy.count(), 1);
}

#include "test_cmakebuilder.moc"
