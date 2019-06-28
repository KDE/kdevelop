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

#include "test_cmakemanager.h"
#include "testhelpers.h"
#include "cmakemodelitems.h"
#include "cmakeutils.h"
#include "cmakeimportjsonjob.h"

#include <QLoggingCategory>
#include <QTemporaryFile>

#include <interfaces/icore.h>
#include <interfaces/itestsuite.h>
#include <interfaces/itestcontroller.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/abstractfilemanagerplugin.h>
#include <tests/autotestshell.h>
#include <tests/testproject.h>
#include <tests/testcore.h>
#include <testing/ctestsuite.h>

QTEST_MAIN(TestCMakeManager)

using namespace KDevelop;

void TestCMakeManager::initTestCase()
{
    QLoggingCategory::setFilterRules(QStringLiteral("*.debug=false\ndefault.debug=true\n"));

    AutoTestShell::init({"KDevCMakeManager", "KDevCMakeBuilder", "KDevMakeBuilder", "KDevStandardOutputView"});
    TestCore::initialize();

    cleanup();
}

void TestCMakeManager::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestCMakeManager::cleanup()
{
    const auto projects = ICore::self()->projectController()->projects();
    for (IProject* p : projects) {
        ICore::self()->projectController()->closeProject(p);
    }
    QVERIFY(ICore::self()->projectController()->projects().isEmpty());
}

void TestCMakeManager::testWithBuildDirProject()
{
    loadProject(QStringLiteral("with_build_dir"));
}

void TestCMakeManager::testIncludePaths()
{
    IProject* project = loadProject(QStringLiteral("single_subdirectory"));
    Path sourceDir = project->path();

    Path fooCpp(sourceDir, QStringLiteral("subdir/foo.cpp"));
    QVERIFY(QFile::exists(fooCpp.toLocalFile()));
    QList< ProjectBaseItem* > items = project->itemsForPath(IndexedString(fooCpp.pathOrUrl()));
    QEXPECT_FAIL("", "Will fix soon, hopefully", Continue);
    QCOMPARE(items.size(), 2); // once the target, once the plain file
    ProjectBaseItem* fooCppItem = items.first();

    Path::List includeDirs = project->buildSystemManager()->includeDirectories(fooCppItem);
    QVERIFY(includeDirs.size() >= 3);

    Path buildDir(project->buildSystemManager()->buildDirectory(fooCppItem));
    QVERIFY(includeDirs.contains(buildDir));

    Path subBuildDir(buildDir, QStringLiteral("subdir/"));
    QVERIFY(includeDirs.contains(subBuildDir));

    Path subDir(sourceDir, QStringLiteral("subdir/"));
    QVERIFY(includeDirs.contains(subDir));
}

void TestCMakeManager::testRelativePaths()
{
    IProject* project = loadProject(QStringLiteral("relative_paths"), QStringLiteral("/out"));

    Path codeCpp(project->path(), QStringLiteral("../src/code.cpp"));
    QVERIFY(QFile::exists( codeCpp.toLocalFile()));
    QList< ProjectBaseItem* > items = project->itemsForPath(IndexedString(codeCpp.pathOrUrl()));
    QEXPECT_FAIL("", "Will fix soon, hopefully", Abort);
    QCOMPARE(items.size(), 1); // once in the target
    ProjectBaseItem* fooCppItem = items.first();

    Path::List includeDirs = project->buildSystemManager()->includeDirectories(fooCppItem);

    Path incDir(project->path(), QStringLiteral("../inc/"));
    QVERIFY(includeDirs.contains( incDir ));
}

void TestCMakeManager::testTargetIncludePaths()
{
    IProject* project = loadProject(QStringLiteral("target_includes"));

    Path mainCpp(project->path(), QStringLiteral("main.cpp"));
    QVERIFY(QFile::exists(mainCpp.toLocalFile()));
    const QList<ProjectBaseItem*> items = project->itemsForPath(IndexedString(mainCpp.pathOrUrl()));
    QEXPECT_FAIL("", "Will fix soon, hopefully", Continue);
    QCOMPARE(items.size(), 2); // once the plain file, once the target

    bool foundInTarget = false;
    for (ProjectBaseItem* mainCppItem : items) {
        ProjectBaseItem* mainContainer = mainCppItem->parent();

        Path::List includeDirs = project->buildSystemManager()->includeDirectories(mainCppItem);

        if (mainContainer->target()) {
            foundInTarget = true;
            Path targetIncludesDir(project->path(), QStringLiteral("includes/"));
            QVERIFY(includeDirs.contains(targetIncludesDir));
        }
    }
    QEXPECT_FAIL("", "Will fix soon, hopefully", Continue);
    QVERIFY(foundInTarget);
}

void TestCMakeManager::testTargetIncludeDirectories()
{
    IProject* project = loadProject(QStringLiteral("target_include_directories"));

    Path mainCpp(project->path(), QStringLiteral("main.cpp"));
    QVERIFY(QFile::exists(mainCpp.toLocalFile()));
    const QList<ProjectBaseItem*> items = project->itemsForPath(IndexedString(mainCpp.pathOrUrl()));
    QEXPECT_FAIL("", "Will fix soon, hopefully", Continue);
    QCOMPARE(items.size(), 2); // once the plain file, once the target

    bool foundInTarget = false;
    for (ProjectBaseItem* mainCppItem : items) {
        ProjectBaseItem* mainContainer = mainCppItem->parent();

        Path::List includeDirs = project->buildSystemManager()->includeDirectories(mainCppItem);

        if (mainContainer->target()) {
            foundInTarget = true;
            QVERIFY(includeDirs.contains(Path(project->path(), "includes/")));
            QVERIFY(includeDirs.contains(Path(project->path(), "libincludes/")));
        }
    }
    QEXPECT_FAIL("", "files aren't being added to the target", Continue);
    QVERIFY(foundInTarget);
}

void TestCMakeManager::testQt5App()
{
    IProject* project = loadProject(QStringLiteral("qt5_app"));

    Path mainCpp(project->path(), QStringLiteral("main.cpp"));
    QVERIFY(QFile::exists(mainCpp.toLocalFile()));
    const QList<ProjectBaseItem*> items = project->itemsForPath(IndexedString(mainCpp.pathOrUrl()));
    QEXPECT_FAIL("", "Will fix soon, hopefully", Continue);
    QCOMPARE(items.size(), 2); // once the plain file, once the target

    bool foundCore = false, foundGui = false, foundWidgets = false;
    for (ProjectBaseItem* mainCppItem : items) {
        const Path::List includeDirs = project->buildSystemManager()->includeDirectories(mainCppItem);
        for (const Path& include : includeDirs) {
            QString filename = include.lastPathSegment();
            foundCore |= filename == QLatin1String("QtCore");
            foundGui |= filename == QLatin1String("QtGui");
            foundWidgets |= filename == QLatin1String("QtWidgets");
        }
    }
    QVERIFY(foundCore);
    QVERIFY(foundGui);
    QVERIFY(foundWidgets);
}

void TestCMakeManager::testQt5AppOld()
{
    IProject* project = loadProject(QStringLiteral("qt5_app_old"));

    Path mainCpp(project->path(), QStringLiteral("main.cpp"));
    QVERIFY(QFile::exists(mainCpp.toLocalFile()));
    const QList<ProjectBaseItem*> items = project->itemsForPath(IndexedString(mainCpp.pathOrUrl()));
    QEXPECT_FAIL("", "Will fix soon, hopefully", Continue);
    QCOMPARE(items.size(), 2); // once the plain file, once the target

    bool foundCore = false, foundGui = false, foundWidgets = false;
    for (ProjectBaseItem* mainCppItem : items) {
        const Path::List includeDirs = project->buildSystemManager()->includeDirectories(mainCppItem);
        for (const Path& include : includeDirs) {
            QString filename = include.lastPathSegment();
            foundCore |= filename == QLatin1String("QtCore");
            foundGui |= filename == QLatin1String("QtGui");
            foundWidgets |= filename == QLatin1String("QtWidgets");
        }
    }
    QVERIFY(foundCore);
    QVERIFY(foundGui);
    QVERIFY(foundWidgets);
}

void TestCMakeManager::testKF5App()
{
    IProject* project = loadProject(QStringLiteral("kf5_app"));

    Path mainCpp(project->path(), QStringLiteral("main.cpp"));
    QVERIFY(QFile::exists(mainCpp.toLocalFile()));
    const QList<ProjectBaseItem*> items = project->itemsForPath(IndexedString(mainCpp.pathOrUrl()));
    QEXPECT_FAIL("", "Will fix soon, hopefully", Continue);
    QCOMPARE(items.size(), 2); // once the plain file, once the target

    bool foundCore = false, foundGui = false, foundWidgets = false, foundWidgetsAddons = false;
    for (ProjectBaseItem* mainCppItem : items) {
        const Path::List includeDirs = project->buildSystemManager()->includeDirectories(mainCppItem);
        qDebug() << "xxxxxxxxx" << includeDirs;
        for (const Path& include : includeDirs) {
            QString filename = include.lastPathSegment();
            foundCore |= filename == QLatin1String("QtCore");
            foundGui |= filename == QLatin1String("QtGui");
            foundWidgets |= filename == QLatin1String("QtWidgets");
            foundWidgetsAddons |= filename == QLatin1String("KWidgetsAddons");
        }
    }
    QVERIFY(foundCore);
    QVERIFY(foundGui);
    QVERIFY(foundWidgets);
    QVERIFY(foundWidgetsAddons);
}

void TestCMakeManager::testDefines()
{
    IProject* project = loadProject(QStringLiteral("defines"));

    Path mainCpp(project->path(), QStringLiteral("main.cpp"));
    QVERIFY(QFile::exists(mainCpp.toLocalFile()));
    const QList<ProjectBaseItem*> items = project->itemsForPath(IndexedString(mainCpp.pathOrUrl()));
    QEXPECT_FAIL("", "Will fix soon, hopefully", Continue);
    QCOMPARE(items.size(), 2); // once the plain file, once the target

    bool foundInTarget = false;
    for (ProjectBaseItem* mainCppItem : items) {
        QHash<QString, QString> defines = project->buildSystemManager()->defines(mainCppItem);

        QCOMPARE(defines.value("B", QStringLiteral("not found")), QString());
        QCOMPARE(defines.value("BV", QStringLiteral("not found")), QStringLiteral("1"));
        QCOMPARE(defines.value("BV2", QStringLiteral("not found")), QStringLiteral("2"));

//         QCOMPARE(defines.value("BAR", QStringLiteral("not found")), QStringLiteral("foo"));
//         QCOMPARE(defines.value("FOO", QStringLiteral("not found")), QStringLiteral("bar"));
//         QCOMPARE(defines.value("BLA", QStringLiteral("not found")), QStringLiteral("blub"));
        QCOMPARE(defines.value("ASDF", QStringLiteral("not found")), QStringLiteral("asdf"));
        QCOMPARE(defines.value("XYZ", QStringLiteral("not found")), QString());
        QCOMPARE(defines.value("A", QStringLiteral("not found")), QString());
        QCOMPARE(defines.value("AV", QStringLiteral("not found")), QStringLiteral("1"));
        QCOMPARE(defines.value("AV2", QStringLiteral("not found")), QStringLiteral("2"));
        QCOMPARE(defines.value("C", QStringLiteral("not found")), QString());
        QCOMPARE(defines.value("CV", QStringLiteral("not found")), QStringLiteral("1"));
        QCOMPARE(defines.value("CV2", QStringLiteral("not found")), QStringLiteral("2"));
        QCOMPARE(defines.size(), 13);
        foundInTarget = true;
    }
    QVERIFY(foundInTarget);
}

void TestCMakeManager::testCustomTargetSources()
{
    IProject* project = loadProject(QStringLiteral("custom_target_sources"));

    QList<ProjectTargetItem*> targets = project->buildSystemManager()->targets(project->projectItem());
    QVERIFY(targets.size() == 1);

    QEXPECT_FAIL("", "Will fix soon, hopefully", Abort);
    ProjectTargetItem *target = targets.first();
    QCOMPARE(target->fileList().size(), 1);
    QCOMPARE(target->fileList().first()->baseName(), QStringLiteral("foo.cpp"));
}

void TestCMakeManager::testConditionsInSubdirectoryBasedOnRootVariables()
{
    IProject* project = loadProject(QStringLiteral("conditions_in_subdirectory_based_on_root_variables"));

    Path rootFooCpp(project->path(), QStringLiteral("foo.cpp"));
    QVERIFY(QFile::exists(rootFooCpp.toLocalFile()));
    QList< ProjectBaseItem* > rootFooItems = project->itemsForPath(IndexedString(rootFooCpp.pathOrUrl()));
    QEXPECT_FAIL("", "files aren't being added to the target", Continue);
    QCOMPARE(rootFooItems.size(), 4); // three items for the targets, one item for the plain file

    Path subdirectoryFooCpp(project->path(), QStringLiteral("subdirectory/foo.cpp"));
    QVERIFY(QFile::exists(subdirectoryFooCpp.toLocalFile()));
    QList< ProjectBaseItem* > subdirectoryFooItems = project->itemsForPath(IndexedString(subdirectoryFooCpp.pathOrUrl()));

    QEXPECT_FAIL("", "files aren't being added to the target", Continue);
    QCOMPARE(subdirectoryFooItems.size(), 4); // three items for the targets, one item for the plain file
}

void TestCMakeManager::testEnumerateTargets()
{
    QString tempDir = QDir::tempPath();

    QTemporaryFile targetDirectoriesFile;
    QTemporaryDir subdir;

    auto opened = targetDirectoriesFile.open();
    QVERIFY(opened);
    QVERIFY(subdir.isValid());

    const QString targetDirectoriesContent = tempDir + "/CMakeFiles/first_target.dir\n" +
                                             tempDir + "/CMakeFiles/second_target.dir\r\n" +
                                             tempDir + "/" + subdir.path() + "/CMakeFiles/third_target.dir";

    targetDirectoriesFile.write(targetDirectoriesContent.toLatin1());
    targetDirectoriesFile.close();

    QHash<KDevelop::Path, QStringList> targets = 
        CMake::enumerateTargets(Path(targetDirectoriesFile.fileName()),
            tempDir, Path(tempDir));

    QCOMPARE(targets.value(Path(tempDir)).value(0), QStringLiteral("first_target"));
    QCOMPARE(targets.value(Path(tempDir)).value(1), QStringLiteral("second_target"));
    QCOMPARE(targets.value(Path(tempDir + "/" + subdir.path())).value(0), QStringLiteral("third_target"));
}

void TestCMakeManager::testReload()
{
    IProject* project = loadProject(QStringLiteral("tiny_project"));
    const Path sourceDir = project->path();

    auto fmp = dynamic_cast<AbstractFileManagerPlugin*>(project->projectFileManager());
    QVERIFY(fmp);

    auto projectItem = project->projectItem();
    auto targets = projectItem->targetList();

    auto job = fmp->createImportJob(project->projectItem());
    project->setReloadJob(job);

    QSignalSpy spy(job, &KJob::finished);
    job->start();
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);

    QCOMPARE(projectItem, project->projectItem());
    QCOMPARE(targets, projectItem->targetList());
}

void TestCMakeManager::testFaultyTarget()
{
    loadProject(QStringLiteral("faulty_target"));
}

void TestCMakeManager::testParenthesesInTestArguments()
{
    IProject* project = loadProject(QStringLiteral("parentheses_in_test_arguments"));

    auto job = new CMakeImportJsonJob(project, this);
    QVERIFY(job->exec());
}

void TestCMakeManager::testExecutableOutputPath()
{
    const auto prevSuitesCount = ICore::self()->testController()->testSuites().count();
    qRegisterMetaType<KDevelop::ITestSuite*>("KDevelop::ITestSuite*");
    QSignalSpy spy(ICore::self()->testController(), &ITestController::testSuiteAdded);

    IProject* project = loadProject(QStringLiteral("randomexe"));
    const auto targets = project->projectItem()->targetList();
    QCOMPARE(targets.count(), 1);

    const auto target = targets.first()->executable();
    QVERIFY(target);
    const KDevelop::Path exePath(target->executable()->builtUrl());
    QCOMPARE(exePath, KDevelop::Path(project->buildSystemManager()->buildDirectory(project->projectItem()), QLatin1String("randomplace/mytest")));

    QVERIFY(spy.count() || spy.wait(100000));

    auto suites = ICore::self()->testController()->testSuites();
    QCOMPARE(suites.count(), prevSuitesCount + 1);
    const CTestSuite* suite = static_cast<CTestSuite*>(ICore::self()->testController()->findTestSuite(project, "mytest"));
    QCOMPARE(suite->executable(), exePath);
}
