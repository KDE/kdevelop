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
#include "testhelpers.h"
#include "cmakemodelitems.h"
#include <icmakemanager.h>

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
    Q_ASSERT(gotSignal && "Timeout while waiting for opened signal");\
} void(0)

using namespace KDevelop;

IProject* CMakeManagerTest::loadProject(const QString& name, const QString& relative)
{
    const TestProjectPaths paths = projectPaths(name+relative, name);
    defaultConfigure(paths);

    ICore::self()->projectController()->openProject(paths.projectFile);
    WAIT_FOR_OPEN_SIGNAL;

    IProject* project = ICore::self()->projectController()->findProjectByName(name);
    Q_ASSERT(project);
    Q_ASSERT(project->buildSystemManager());
    Q_ASSERT(paths.projectFile == project->projectFileUrl());
    Q_ASSERT(project->folder() == project->folder());
    return project;
}

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
    loadProject("with_build_dir");
}

void CMakeManagerTest::testIncludePaths()
{
    IProject* project = loadProject("single_subdirectory");
    KUrl sourceDir = project->folder();

    KUrl fooCpp(sourceDir, "subdir/foo.cpp");
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

    KUrl buildDir(sourceDir, "build/");
    QVERIFY(includeDirs.contains(buildDir));

    KUrl subBuildDir(sourceDir, "build/subdir/");
    QVERIFY(includeDirs.contains(subBuildDir));

    KUrl subDir(sourceDir, "subdir/");
    QVERIFY(includeDirs.contains(subDir));
}

void CMakeManagerTest::testRelativePaths()
{
    IProject* project = loadProject("relative_paths", "/out");

    KUrl codeCpp(project->folder(), "../src/code.cpp");
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

    KUrl incDir(project->folder(), "../inc/");
    incDir.cleanPath();
    QVERIFY(includeDirs.contains( incDir ));
}

void CMakeManagerTest::testTargetIncludePaths()
{
    IProject* project = loadProject("target_includes");

    KUrl mainCpp(project->folder(), "main.cpp");
    QVERIFY(QFile::exists(mainCpp.toLocalFile()));
    QList< ProjectBaseItem* > items = project->itemsForUrl(mainCpp);
    QCOMPARE(items.size(), 2); // once the plain file, once the target

    bool foundInTarget = false;
    foreach(ProjectBaseItem* mainCppItem, items) {
        ProjectBaseItem* mainContainer = mainCppItem->parent();

        KUrl::List _includeDirs = project->buildSystemManager()->includeDirectories(mainCppItem);
        QSet<KUrl> includeDirs;
        foreach(KUrl url, _includeDirs) {
            url.cleanPath(KUrl::SimplifyDirSeparators);
            url.adjustPath(KUrl::AddTrailingSlash);
            includeDirs << url;
        }

        QCOMPARE(includeDirs.size(), _includeDirs.size());

        if (dynamic_cast<CMakeExecutableTargetItem*>( mainContainer )) {
            foundInTarget = true;
            KUrl targetIncludesDir(project->folder(), "includes/");
            QVERIFY(includeDirs.contains(targetIncludesDir));
        }
    }
    QVERIFY(foundInTarget);
}

void CMakeManagerTest::testTargetIncludeDirectories()
{
    IProject* project = loadProject("target_include_directories");

    KUrl mainCpp(project->folder(), "main.cpp");
    QVERIFY(QFile::exists(mainCpp.toLocalFile()));
    QList< ProjectBaseItem* > items = project->itemsForUrl(mainCpp);
    QCOMPARE(items.size(), 2); // once the plain file, once the target

    bool foundInTarget = false;
    foreach(ProjectBaseItem* mainCppItem, items) {
        ProjectBaseItem* mainContainer = mainCppItem->parent();

        KUrl::List _includeDirs = project->buildSystemManager()->includeDirectories(mainCppItem);
        QSet<KUrl> includeDirs;
        foreach(KUrl url, _includeDirs) {
            url.cleanPath(KUrl::SimplifyDirSeparators);
            url.adjustPath(KUrl::AddTrailingSlash);
            includeDirs << url;
        }

        QCOMPARE(includeDirs.size(), _includeDirs.size());

        if (dynamic_cast<CMakeExecutableTargetItem*>( mainContainer )) {
            foundInTarget = true;
            KUrl targetIncludesDir(project->folder(), "includes/");
            QVERIFY(includeDirs.contains(targetIncludesDir));
        }
    }
    QVERIFY(foundInTarget);
}

void CMakeManagerTest::testTargetDefines()
{
    IProject* project = loadProject("target_defines");

    KUrl mainCpp(project->folder(), "main.cpp");
    QVERIFY(QFile::exists(mainCpp.toLocalFile()));
    QList< ProjectBaseItem* > items = project->itemsForUrl(mainCpp);
    QCOMPARE(items.size(), 2); // once the plain file, once the target

    bool foundInTarget = false;
    foreach(ProjectBaseItem* mainCppItem, items) {
        ProjectBaseItem* mainContainer = mainCppItem->parent();

        QHash<QString, QString> defines = project->buildSystemManager()->defines(mainCppItem);

        if (dynamic_cast<CMakeExecutableTargetItem*>( mainContainer )) {
            QCOMPARE(defines.size(), 1);
            QVERIFY(defines.contains(QString("VALUE")));
            QCOMPARE(defines.value("VALUE"), QString("1"));
            foundInTarget = true;
        }
    }
    QVERIFY(foundInTarget);
}

void CMakeManagerTest::testCustomTargetSources()
{
    IProject* project = loadProject("custom_target_sources");

    QList<ProjectTargetItem*> targets = project->buildSystemManager()->targets(project->projectItem());
    QVERIFY(targets.size() == 1);

    ProjectTargetItem *target = targets.first();
    QCOMPARE(target->fileList().size(), 1);
    QCOMPARE(target->fileList().first()->baseName(), QString("foo.cpp"));
}

void CMakeManagerTest::testConditionsInSubdirectoryBasedOnRootVariables()
{
    IProject* project = loadProject("conditions_in_subdirectory_based_on_root_variables");

    KUrl rootFooCpp(project->folder(), "foo.cpp");
    QVERIFY(QFile::exists(rootFooCpp.toLocalFile()));
    QList< ProjectBaseItem* > rootFooItems = project->itemsForUrl(rootFooCpp);
    QCOMPARE(rootFooItems.size(), 4); // three items for the targets, one item for the plain file

    KUrl subdirectoryFooCpp(project->folder(), "subdirectory/foo.cpp");
    QVERIFY(QFile::exists(subdirectoryFooCpp.toLocalFile()));
    QList< ProjectBaseItem* > subdirectoryFooItems = project->itemsForUrl(subdirectoryFooCpp);
    QCOMPARE(subdirectoryFooItems.size(), 4); // three items for the targets, one item for the plain file
}
