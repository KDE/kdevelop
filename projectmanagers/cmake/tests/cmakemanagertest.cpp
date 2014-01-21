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
    loadProject("with_build_dir");
}

void CMakeManagerTest::testIncludePaths()
{
    IProject* project = loadProject("single_subdirectory");
    Path sourceDir = project->path();

    Path fooCpp(sourceDir, "subdir/foo.cpp");
    QVERIFY(QFile::exists(fooCpp.toLocalFile()));
    QList< ProjectBaseItem* > items = project->itemsForPath(fooCpp.toIndexed());
    QCOMPARE(items.size(), 2); // once the target, once the plain file
    ProjectBaseItem* fooCppItem = items.first();

    Path::List includeDirs = project->buildSystemManager()->includeDirectories(fooCppItem);

    Path buildDir(sourceDir, "build/");
    QVERIFY(includeDirs.contains(buildDir));

    Path subBuildDir(sourceDir, "build/subdir/");
    QVERIFY(includeDirs.contains(subBuildDir));

    Path subDir(sourceDir, "subdir/");
    QVERIFY(includeDirs.contains(subDir));
}

void CMakeManagerTest::testRelativePaths()
{
    IProject* project = loadProject("relative_paths", "/out");

    Path codeCpp(project->path(), "../src/code.cpp");
    QVERIFY(QFile::exists( codeCpp.toLocalFile()));
    QList< ProjectBaseItem* > items = project->itemsForPath( codeCpp.toIndexed() );
    QCOMPARE(items.size(), 1); // once in the target
    ProjectBaseItem* fooCppItem = items.first();

    Path::List includeDirs = project->buildSystemManager()->includeDirectories(fooCppItem);

    Path incDir(project->path(), "../inc/");
    QVERIFY(includeDirs.contains( incDir ));
}

void CMakeManagerTest::testTargetIncludePaths()
{
    IProject* project = loadProject("target_includes");

    Path mainCpp(project->path(), "main.cpp");
    QVERIFY(QFile::exists(mainCpp.toLocalFile()));
    QList< ProjectBaseItem* > items = project->itemsForPath(mainCpp.toIndexed());
    QCOMPARE(items.size(), 2); // once the plain file, once the target

    bool foundInTarget = false;
    foreach(ProjectBaseItem* mainCppItem, items) {
        ProjectBaseItem* mainContainer = mainCppItem->parent();

        Path::List includeDirs = project->buildSystemManager()->includeDirectories(mainCppItem);

        if (dynamic_cast<CMakeExecutableTargetItem*>( mainContainer )) {
            foundInTarget = true;
            Path targetIncludesDir(project->path(), "includes/");
            QVERIFY(includeDirs.contains(targetIncludesDir));
        }
    }
    QVERIFY(foundInTarget);
}

void CMakeManagerTest::testTargetIncludeDirectories()
{
    IProject* project = loadProject("target_include_directories");

    Path mainCpp(project->path(), "main.cpp");
    QVERIFY(QFile::exists(mainCpp.toLocalFile()));
    QList< ProjectBaseItem* > items = project->itemsForPath(mainCpp.toIndexed());
    QCOMPARE(items.size(), 2); // once the plain file, once the target

    bool foundInTarget = false;
    foreach(ProjectBaseItem* mainCppItem, items) {
        ProjectBaseItem* mainContainer = mainCppItem->parent();

        Path::List includeDirs = project->buildSystemManager()->includeDirectories(mainCppItem);

        if (dynamic_cast<CMakeExecutableTargetItem*>( mainContainer )) {
            foundInTarget = true;
            QVERIFY(includeDirs.contains(Path(project->path(), "includes/")));
            QVERIFY(includeDirs.contains(Path(project->path(), "libincludes/")));
        }
    }
    QVERIFY(foundInTarget);
}

void CMakeManagerTest::testQt5App()
{
    if (!qgetenv("KDEV_CMAKE_TEST_QT5").toInt()) {
        QSKIP("Test only passes if Qt5 is available, define KDEV_CMAKE_TEST_QT5 to enable this test.", SkipAll);
    }

    IProject* project = loadProject("qt5_app");

    Path mainCpp(project->path(), "main.cpp");
    QVERIFY(QFile::exists(mainCpp.toLocalFile()));
    QList< ProjectBaseItem* > items = project->itemsForPath(mainCpp.toIndexed());
    QCOMPARE(items.size(), 2); // once the plain file, once the target

    bool foundCore = false, foundGui = false, foundWidgets = false;
    foreach(ProjectBaseItem* mainCppItem, items) {
        Path::List includeDirs = project->buildSystemManager()->includeDirectories(mainCppItem);
        foreach(const Path& include, includeDirs) {
            QString filename = include.lastPathSegment();
            foundCore |= filename == "QtCore";
            foundGui |= filename == "QtGui";
            foundWidgets |= filename == "QtWidgets";
        }
    }
    QVERIFY(foundCore);
    QVERIFY(foundGui);
    QVERIFY(foundWidgets);
}

void CMakeManagerTest::testDefines()
{
    IProject* project = loadProject("defines");

    Path mainCpp(project->path(), "main.cpp");
    QVERIFY(QFile::exists(mainCpp.toLocalFile()));
    QList< ProjectBaseItem* > items = project->itemsForPath(mainCpp.toIndexed());
    QCOMPARE(items.size(), 2); // once the plain file, once the target

    bool foundInTarget = false;
    foreach(ProjectBaseItem* mainCppItem, items) {
        ProjectBaseItem* mainContainer = mainCppItem->parent();

        QHash<QString, QString> defines = project->buildSystemManager()->defines(mainCppItem);

        if (dynamic_cast<CMakeExecutableTargetItem*>( mainContainer )) {
            QEXPECT_FAIL("", "SOURCE definitions are not implemented yet", Continue);
            QCOMPARE(defines.size(), 14);
            QCOMPARE(defines.size(), 11);
            QCOMPARE(defines.value("B", QString("not found")), QString());
            QCOMPARE(defines.value("BV", QString("not found")), QString("1"));
            QCOMPARE(defines.value("BV2", QString("not found")), QString("2"));
            foundInTarget = true;
        } else {
            QEXPECT_FAIL("", "SOURCE definitions are not implemented yet", Continue);
            QCOMPARE(defines.size(), 11);
            QCOMPARE(defines.size(), 8);
        }
        QCOMPARE(defines.value("BAR", QString("not found")), QString("foo"));
        QCOMPARE(defines.value("FOO", QString("not found")), QString("bar"));
        QCOMPARE(defines.value("BLA", QString("not found")), QString("blub"));
        QCOMPARE(defines.value("ASDF", QString("not found")), QString("asdf"));
        QCOMPARE(defines.value("XYZ", QString("not found")), QString());
        QCOMPARE(defines.value("A", QString("not found")), QString());
        QCOMPARE(defines.value("AV", QString("not found")), QString("1"));
        QCOMPARE(defines.value("AV2", QString("not found")), QString("2"));
        QEXPECT_FAIL("", "", Continue);
        QCOMPARE(defines.value("C", QString("not found")), QString());
        QEXPECT_FAIL("", "", Continue);
        QCOMPARE(defines.value("CV", QString("not found")), QString("1"));
        QEXPECT_FAIL("", "", Continue);
        QCOMPARE(defines.value("CV2", QString("not found")), QString("2"));
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

    Path rootFooCpp(project->path(), "foo.cpp");
    QVERIFY(QFile::exists(rootFooCpp.toLocalFile()));
    QList< ProjectBaseItem* > rootFooItems = project->itemsForPath(rootFooCpp.toIndexed());
    QCOMPARE(rootFooItems.size(), 4); // three items for the targets, one item for the plain file

    Path subdirectoryFooCpp(project->path(), "subdirectory/foo.cpp");
    QVERIFY(QFile::exists(subdirectoryFooCpp.toLocalFile()));
    QList< ProjectBaseItem* > subdirectoryFooItems = project->itemsForPath(subdirectoryFooCpp.toIndexed());
    QCOMPARE(subdirectoryFooItems.size(), 4); // three items for the targets, one item for the plain file
}

void CMakeManagerTest::testFaultyTarget()
{
    loadProject("faulty_target");
}
