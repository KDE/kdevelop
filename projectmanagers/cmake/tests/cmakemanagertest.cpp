/* This file is part of KDevelop
    Copyright 2010 Esben Mose Hansen<kde@mosehansen.dk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "cmakemanagertest.h"
#include "cmake-test-paths.h"

#include <icmakemanager.h>
#include <cmakebuilddirchooser.h>

#include <qtest.h>
#include <qtest_kde.h>

#include <interfaces/iplugincontroller.h>
#include <interfaces/icore.h>
#include <project/interfaces/iprojectfilemanager.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <tests/autotestshell.h>
#include <tests/testproject.h>
#include <tests/testcore.h>

QTEST_KDEMAIN(CMakeManagerTest, GUI )

#define WAIT_FOR_OPEN_SIGNAL \
{\
    bool gotSignal = QTest::kWaitForSignal(ICore::self()->projectController(), SIGNAL(projectOpened(KDevelop::IProject*)), 30000);\
    QVERIFY2(gotSignal, "Timeout while waiting for opened signal");\
} void(0)

static QString currentBuildDirKey = "CurrentBuildDir";
static QString currentCMakeBinaryKey = "Current CMake Binary";
static QString currentBuildTypeKey = "CurrentBuildType";
static QString currentInstallDirKey = "CurrentInstallDir";
static QString currentExtraArgumentsKey = "Extra Arguments";
static QString projectRootRelativeKey = "ProjectRootRelative";
static QString projectBuildDirs = "BuildDirs";

struct TestProjectPaths {
    // foo/
    KUrl sourceDir;
    // foo/foo.kdev4
    KUrl projectFile;
    // foo/.kdev4/foo.kdev4
    KUrl configFile;
};

TestProjectPaths projectPaths(const QString& project, QString name = QString())
{
    if (name.isEmpty()) {
        name = project;
    }

    TestProjectPaths paths;
    QFileInfo info(CMAKE_TESTS_PROJECTS_DIR "/" + project);
    Q_ASSERT(info.exists());

    paths.sourceDir = info.canonicalFilePath();
    paths.sourceDir.adjustPath(KUrl::AddTrailingSlash);

    paths.projectFile = paths.sourceDir;
    paths.projectFile.addPath(name + ".kdev4");
    Q_ASSERT(QFile::exists(paths.projectFile.toLocalFile()));

    paths.configFile = paths.sourceDir;
    paths.configFile.addPath(".kdev4/" + name + ".kdev4");

    return paths;
}

/**
 * apply default configuration to project in @p sourceDir called @p projectName
 * 
 * this prevents the dialog to popup asking for user interaction
 * which should never happen in an automated unit test
 */
void defaultConfigure(const TestProjectPaths& paths)
{
    KConfig config(paths.configFile.toLocalFile());
    // clear config
    config.deleteGroup("CMake");

    // apply default configuration
    CMakeBuildDirChooser bd;
    bd.setSourceFolder(paths.sourceDir);
    // we don't want to execute, just pick the defaults from the dialog

    KConfigGroup cmakeGrp = config.group("CMake");
    {
        QDir buildFolder( bd.buildFolder().toLocalFile() );
        if ( !buildFolder.exists() ) {
            if ( !buildFolder.mkpath( buildFolder.absolutePath() ) ) {
                QFAIL("The build directory did not exist and could not be created.");
            }
        }
    }

    cmakeGrp.writeEntry( currentBuildDirKey, bd.buildFolder() );
    cmakeGrp.writeEntry( currentCMakeBinaryKey, bd.cmakeBinary() );
    cmakeGrp.writeEntry( currentInstallDirKey, bd.installPrefix() );
    cmakeGrp.writeEntry( currentExtraArgumentsKey, bd.extraArguments() );
    cmakeGrp.writeEntry( currentBuildTypeKey, bd.buildType() );
    cmakeGrp.writeEntry( projectBuildDirs, QStringList() << bd.buildFolder().toLocalFile());

    config.sync();
}

using namespace KDevelop;

void CMakeManagerTest::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize();

    cleanup();
}

void CMakeManagerTest::cleanupTestCase()
{
    TestCore::shutdown();
}

void CMakeManagerTest::cleanup()
{
    foreach(IProject* p, ICore::self()->projectController()->projects()) {
        ICore::self()->projectController()->closeProject(p);
    }
    QVERIFY(ICore::self()->projectController()->projects().isEmpty());
}

void CMakeManagerTest::testWithBuildDirProject()
{
    const TestProjectPaths paths = projectPaths("with_build_dir");

    defaultConfigure(paths);

    // Import project
    ICore::self()->projectController()->openProject(paths.projectFile);

    WAIT_FOR_OPEN_SIGNAL;

    IProject* project = ICore::self()->projectController()->findProjectByName("with_build_dir");
    QCOMPARE(paths.projectFile, project->projectFileUrl());
    QCOMPARE(paths.sourceDir, project->folder());
}

void CMakeManagerTest::testIncludePaths()
{
    const TestProjectPaths paths = projectPaths("single_subdirectory");
    defaultConfigure(paths);

    ICore::self()->projectController()->openProject(paths.projectFile);

    WAIT_FOR_OPEN_SIGNAL;

    IProject* project = ICore::self()->projectController()->findProjectByName("single_subdirectory");
    QVERIFY(project->buildSystemManager());

    QCOMPARE(paths.projectFile, project->projectFileUrl());
    QCOMPARE(paths.sourceDir, project->folder());

    KUrl fooCpp(paths.sourceDir, "subdir/foo.cpp");
    QVERIFY(QFile::exists(fooCpp.toLocalFile()));
    QList< ProjectBaseItem* > items = project->itemsForUrl(fooCpp);
    QCOMPARE(items.size(), 2); // once the target, once the plain file
    ProjectBaseItem* fooCppItem = items.first();

    KUrl::List _includeDirs = project->buildSystemManager()->includeDirectories(fooCppItem);
    QSet<KUrl> includeDirs;
    foreach(KUrl url, _includeDirs) {
        url.cleanPath(KUrl::SimplifyDirSeparators);
        url.adjustPath(KUrl::AddTrailingSlash);
        includeDirs << url;
    }

    QCOMPARE(includeDirs.size(), _includeDirs.size());

    KUrl buildDir(paths.sourceDir, "build/");
    QVERIFY(includeDirs.contains(buildDir));

    KUrl subBuildDir(paths.sourceDir, "build/subdir/");
    QVERIFY(includeDirs.contains(subBuildDir));

    KUrl subDir(paths.sourceDir, "subdir/");
    QVERIFY(includeDirs.contains(subDir));
}

void CMakeManagerTest::testRelativePaths()
{
    const TestProjectPaths paths = projectPaths("relative_paths/out", "relative_paths");
    defaultConfigure(paths);

    ICore::self()->projectController()->openProject(paths.projectFile);

    WAIT_FOR_OPEN_SIGNAL;

    IProject* project = ICore::self()->projectController()->findProjectByName("relative_paths");
    QVERIFY(project);
    QVERIFY(project->buildSystemManager());

    QCOMPARE(paths.projectFile, project->projectFileUrl());
    QCOMPARE(paths.sourceDir, project->folder());

    KUrl codeCpp(paths.sourceDir, "../src/code.cpp");
    codeCpp.cleanPath();
    QVERIFY(QFile::exists( codeCpp.toLocalFile()));
    QList< ProjectBaseItem* > items = project->itemsForUrl( codeCpp );
    QCOMPARE(items.size(), 1); // once in the target
    ProjectBaseItem* fooCppItem = items.first();

    KUrl::List _includeDirs = project->buildSystemManager()->includeDirectories(fooCppItem);
    QSet<KUrl> includeDirs;
    foreach(KUrl url, _includeDirs) {
        url.cleanPath(KUrl::SimplifyDirSeparators);
        url.adjustPath(KUrl::AddTrailingSlash);
        includeDirs << url;
    }

    QCOMPARE(includeDirs.size(), _includeDirs.size());

    KUrl incDir(paths.sourceDir, "../inc/");
    incDir.cleanPath();
    QVERIFY(includeDirs.contains( incDir ));
}
