/* KDevelop QMake Support
 *
 * Copyright 2011 Julien Desgats <julien.desgats@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
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
#include <kio/deletejob.h>

QTEST_MAIN(TestQMakeProject);

using namespace KDevelop;

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
    AutoTestShell::init({ "kdevqmakemanager" });
    TestCore::initialize();
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
    QTest::newRow("Subdirs Project (root)") << "subdirs_project"
                                            << ""
                                            << "";
    QTest::newRow("Subdirs Project (dir_a)") << "subdirs_project"
                                             << "dir_a"
                                             << "dir_a";
}

void TestQMakeProject::testBuildDirectory()
{
    QFETCH(QString, projectName);
    QFETCH(QString, target);
    QFETCH(QString, expected);

    const QString buildDir = "/tmp/some/path"; // some dummy directory to build (nothing will be built anyway)

    foreach (IProject* p, ICore::self()->projectController()->projects()) {
        ICore::self()->projectController()->closeProject(p);
    }

    // setup project config, to avoid build dir chooser dialog popping up
    {
        // note: all checks from QMakeProjectManager::projectNeedsConfiguration must be satisfied
        const QString fileName
            = QString("%1/%2/.kdev4/%3.kdev4").arg(QMAKE_TESTS_PROJECTS_DIR).arg(projectName).arg(projectName);

        KConfig cfg(fileName);
        KConfigGroup group(&cfg, QMakeConfig::CONFIG_GROUP);

        group.writeEntry(QMakeConfig::BUILD_FOLDER, buildDir);
        group.writeEntry(QMakeConfig::QMAKE_BINARY, QMAKE_TESTS_QMAKE_BINARY);
        group.sync();

        /// create subgroup for one build dir
        KConfigGroup buildDirGroup = KConfigGroup(&cfg, QMakeConfig::CONFIG_GROUP).group(buildDir);
        buildDirGroup.writeEntry(QMakeConfig::QMAKE_BINARY, QMAKE_TESTS_QMAKE_BINARY);
        buildDirGroup.sync();

        QVERIFY(QFileInfo::exists(fileName));
    }

    // opens project with kdevelop
    const QUrl projectUrl = QUrl::fromLocalFile(
        QString("%1/%2/%3.kdev4").arg(QMAKE_TESTS_PROJECTS_DIR).arg(projectName).arg(projectName));
    ICore::self()->projectController()->openProject(projectUrl);

    // wait for loading finished
    QSignalSpy spy(ICore::self()->projectController(), SIGNAL(projectOpened(KDevelop::IProject*)));
    bool gotSignal = spy.wait(30000);
    QVERIFY2(gotSignal, "Timeout while waiting for opened signal");

    IProject* project = ICore::self()->projectController()->findProjectByName(projectName);

    // adds expected directory to our base path
    Path expectedPath(Path(buildDir), expected);

    // path for files to build
    Path buildUrl(QString("%1/%2/%3").arg(QMAKE_TESTS_PROJECTS_DIR).arg(projectName).arg(target));
    QList<ProjectFolderItem*> buildItems = project->foldersForPath(IndexedString(buildUrl.pathOrUrl()));
    QCOMPARE(buildItems.size(), 1);
    IBuildSystemManager* buildManager = project->buildSystemManager();
    const auto buildFolder = buildItems.first();

    const Path actual = buildManager->buildDirectory(buildFolder);

    QCOMPARE(actual, expectedPath);

    auto buildJob = buildManager->builder()->configure(project);
    QVERIFY(buildJob->exec());
}
