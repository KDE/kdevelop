/*
    SPDX-FileCopyrightText: 2010 Esben Mose Hansen <kde@mosehansen.dk>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
    QLoggingCategory::setFilterRules("*.debug=false\nkdevplatform.outputview.debug=true\n"
                                     "kdevelop.plugins.cmake.debug=true\ndefault.debug=true\n");

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
    QVERIFY(foundInTarget);
}

void TestCMakeManager::testTargetIncludeDirectories()
{
    IProject* project = loadProject(QStringLiteral("target_include_directories"));

    Path mainCpp(project->path(), QStringLiteral("main.cpp"));
    QVERIFY(QFile::exists(mainCpp.toLocalFile()));
    const QList<ProjectBaseItem*> items = project->itemsForPath(IndexedString(mainCpp.pathOrUrl()));
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
    QVERIFY(foundInTarget);
}

void TestCMakeManager::testQt6App()
{
    IProject* project = loadProject(QStringLiteral("qt6_app"));

    Path mainCpp(project->path(), QStringLiteral("main.cpp"));
    QVERIFY(QFile::exists(mainCpp.toLocalFile()));
    const QList<ProjectBaseItem*> items = project->itemsForPath(IndexedString(mainCpp.pathOrUrl()));
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

void TestCMakeManager::testKF6App()
{
    IProject* project = loadProject(QStringLiteral("kf6_app"));

    Path mainCpp(project->path(), QStringLiteral("main.cpp"));
    QVERIFY(QFile::exists(mainCpp.toLocalFile()));
    const QList<ProjectBaseItem*> items = project->itemsForPath(IndexedString(mainCpp.pathOrUrl()));
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
    QCOMPARE(items.size(), 2); // once the plain file, once the target

    bool foundInTarget = false;
    for (ProjectBaseItem* mainCppItem : items) {
        QHash<QString, QString> defines = project->buildSystemManager()->defines(mainCppItem);

        QCOMPARE(defines.value("B", QStringLiteral("not found")), QString());
        QCOMPARE(defines.value("BV", QStringLiteral("not found")), QStringLiteral("1"));
        QCOMPARE(defines.value("BV2", QStringLiteral("not found")), QStringLiteral("2"));

        QCOMPARE(defines.value("BAR", QStringLiteral("not found")), QStringLiteral("foo"));
        QCOMPARE(defines.value("FOO", QStringLiteral("not found")), QStringLiteral("bar"));
        QCOMPARE(defines.value("BLA", QStringLiteral("not found")), QStringLiteral("blub"));

        QCOMPARE(defines.value("ASDF", QStringLiteral("not found")), QStringLiteral("asdf"));
        QCOMPARE(defines.value("XYZ", QStringLiteral("not found")), QString());
        QCOMPARE(defines.value("A", QStringLiteral("not found")), QString());
        QCOMPARE(defines.value("AV", QStringLiteral("not found")), QStringLiteral("1"));
        QCOMPARE(defines.value("AV2", QStringLiteral("not found")), QStringLiteral("2"));
        QCOMPARE(defines.value("C", QStringLiteral("not found")), QString());
        QCOMPARE(defines.value("CV", QStringLiteral("not found")), QStringLiteral("1"));
        QCOMPARE(defines.value("CV2", QStringLiteral("not found")), QStringLiteral("2"));

        QCOMPARE(defines.size(), 14);
        foundInTarget = true;
    }
    QVERIFY(foundInTarget);
}

void TestCMakeManager::testCustomTargetSources()
{
    IProject* project = loadProject(QStringLiteral("custom_target_sources"));

    QList<ProjectTargetItem*> targets = project->buildSystemManager()->targets(project->projectItem());
    QVERIFY(targets.size() == 1);

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
    QCOMPARE(rootFooItems.size(), 4); // three items for the targets, one item for the plain file

    Path subdirectoryFooCpp(project->path(), QStringLiteral("subdirectory/foo.cpp"));
    QVERIFY(QFile::exists(subdirectoryFooCpp.toLocalFile()));
    QList< ProjectBaseItem* > subdirectoryFooItems = project->itemsForPath(IndexedString(subdirectoryFooCpp.pathOrUrl()));

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
    QCOMPARE(targets.size(), 1);
    auto target = dynamic_cast<CMakeTargetItem*>(targets.first());
    QVERIFY(target);
    QCOMPARE(target->text(), QStringLiteral("foo"));

    auto job = fmp->createImportJob(project->projectItem());
    project->setReloadJob(job);

    QSignalSpy spy(job, &KJob::finished);
    job->start();
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);

    QCOMPARE(projectItem, project->projectItem());
    targets = projectItem->targetList();
    QCOMPARE(targets.size(), 1);
    target = dynamic_cast<CMakeTargetItem*>(targets.first());
    QVERIFY(target);
    QCOMPARE(target->text(), QStringLiteral("foo"));
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
    QString executableSuffix;
#ifdef Q_OS_WIN
    executableSuffix = ".exe";
#endif
    QCOMPARE(exePath,
             Path(project->buildSystemManager()->buildDirectory(project->projectItem()),
                  "randomplace/mytest" + executableSuffix));

    QVERIFY(spy.count() || spy.wait(100000));

    auto suites = ICore::self()->testController()->testSuites();
    QCOMPARE(suites.count(), prevSuitesCount + 1);
    const CTestSuite* suite = static_cast<CTestSuite*>(ICore::self()->testController()->findTestSuite(project, "mytest"));
    QCOMPARE(suite->executable(), exePath);
}

#include "moc_test_cmakemanager.cpp"
