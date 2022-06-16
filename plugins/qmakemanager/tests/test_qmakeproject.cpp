/*
    SPDX-FileCopyrightText: 2011 Julien Desgats <julien.desgats@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_qmakeproject.h"
#include "../qmakeconfig.h"
#include "qmaketestconfig.h"

#include <shell/core.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/interfaces/iprojectbuilder.h>
#include <project/projectmodel.h>
#include <serialization/indexedstring.h>

#include <QFileInfo>
#include <QTest>
#include <QSignalSpy>
#include <KConfigGroup>
#include <KJob>

QTEST_MAIN(TestQMakeProject)

using namespace KDevelop;

namespace
{
ProjectTargetItem* findTarget(const ProjectFolderItem* folder, const QString& name)
{
    const auto items = folder->children();
    for (const auto* item : items) {
        if (item->target() && item->baseName() == name) {
            return item->target();
        } else if (item->folder()) {
            auto ret = findTarget(item->folder(), name);
            if (ret) {
                return ret;
            }
        }
    }
    return nullptr;
}

}

TestQMakeProject::TestQMakeProject(QObject* parent)
    : QObject(parent)
{
    qRegisterMetaType<IProject*>();
}

TestQMakeProject::~TestQMakeProject()
{
}

void TestQMakeProject::initTestCase()
{
    AutoTestShell::init({ "KDevQMakeManager", "KDevQMakeBuilder", "KDevMakeBuilder", "KDevStandardOutputView" });
    TestCore::initialize();

    // Verify m_buildDir after initialization. Otherwise cleanupTestCase() crashes if the check fails.
    QVERIFY2(m_buildDir.isValid(), qPrintable("couldn't create temporary directory: " + m_buildDir.errorString()));
}

void TestQMakeProject::cleanupTestCase()
{
    Core::self()->cleanup();
}

void TestQMakeProject::testBuildDirectory_data()
{
    QTest::addColumn<QString>("projectName"); // name of the project (both directory and .kde4 file)
    QTest::addColumn<QString>("target"); // directory to compile from project root
    QTest::addColumn<QString>("expected"); // expected build directory from build dir

    QTest::newRow("Basic Project") << "basic_project"
                                   << ""
                                   << "";
    QTest::newRow("Subdirs Project (dir_a)") << "subdirs_project"
                                             << "dir_a"
                                             << "dir_a";
    QTest::newRow("Subdirs Project (dir_b)") << "subdirs_project"
                                             << "dir_b"
                                             << "dir_b";
}

void TestQMakeProject::testBuildDirectory()
{
    QFETCH(QString, projectName);
    QFETCH(QString, target);
    QFETCH(QString, expected);

    foreach (IProject* p, ICore::self()->projectController()->projects()) {
        ICore::self()->projectController()->closeProject(p);
    }

    // setup project config, to avoid build dir chooser dialog popping up
    {
        // note: all checks from QMakeProjectManager::projectNeedsConfiguration must be satisfied
        const QString fileName
            = QStringLiteral("%1/%2/.kdev4/%2.kdev4").arg(QMAKE_TESTS_PROJECTS_DIR, projectName);

        KConfig cfg(fileName);
        KConfigGroup group(&cfg, QMakeConfig::CONFIG_GROUP);

        group.writeEntry(QMakeConfig::BUILD_FOLDER, m_buildDir.path());
        group.writeEntry(QMakeConfig::QMAKE_EXECUTABLE, QMAKE_TESTS_QMAKE_EXECUTABLE);
        group.sync();

        /// create subgroup for one build dir
        KConfigGroup buildDirGroup = KConfigGroup(&cfg, QMakeConfig::CONFIG_GROUP).group(m_buildDir.path());
        buildDirGroup.writeEntry(QMakeConfig::QMAKE_EXECUTABLE, QMAKE_TESTS_QMAKE_EXECUTABLE);
        buildDirGroup.sync();

        QVERIFY(QFileInfo::exists(fileName));
    }

    // opens project with kdevelop
    const QUrl projectUrl = QUrl::fromLocalFile(
        QStringLiteral("%1/%2/%2.kdev4").arg(QMAKE_TESTS_PROJECTS_DIR, projectName));
    ICore::self()->projectController()->openProject(projectUrl);

    // wait for loading finished
    QSignalSpy spy(ICore::self()->projectController(), SIGNAL(projectOpened(KDevelop::IProject*)));
    bool gotSignal = spy.wait(30000);
    QVERIFY2(gotSignal, "Timeout while waiting for opened signal");

    IProject* project = ICore::self()->projectController()->findProjectByName(projectName);

    // adds expected directory to our base path
    const Path expectedPath(Path{m_buildDir.path()}, expected);

    auto targetItem = findTarget(project->projectItem(), target.isEmpty() ? projectName : target);
    QVERIFY(targetItem);

    IBuildSystemManager* buildManager = project->buildSystemManager();

    const Path actual = buildManager->buildDirectory(targetItem);
    QCOMPARE(actual, expectedPath);

    auto buildJob = buildManager->builder()->configure(project);
    QVERIFY(buildJob->exec());
}
