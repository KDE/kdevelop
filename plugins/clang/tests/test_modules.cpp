/*
    SPDX-FileCopyrightText: 2026 Fernando Vilas <fvilas@iname.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "test_modules.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>

#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <tests/testplugincontroller.h>

#include "testfilepaths.h"
#include "../duchain/clanghelpers.h"
#include "../duchain/clangmodulegraph.h"
#include "../duchain/clangmodulerepository.h"
#include "../duchain/clangmodulescanner.h"
#include "../clangsettings/clangsettingsmanager.h"

using namespace KDevelop;

QTEST_MAIN(TestModules)

void TestModules::initTestCase()
{
    AutoTestShell::init();
    auto* core = new TestCore;
    core->setPluginController(new TestPluginController(core));
    TestCore::initialize(Core::NoUi);
}

void TestModules::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestModules::testStdVersionRank()
{
    using V = ClangHelpers::CppStdVersion;
    QCOMPARE(ClangHelpers::stdVersionRank(QStringLiteral("-std=c++98")), V::Cpp98);
    QCOMPARE(ClangHelpers::stdVersionRank(QStringLiteral("-std=gnu++98")), V::Cpp98);
    QCOMPARE(ClangHelpers::stdVersionRank(QStringLiteral("-std=c++03")), V::Cpp03);
    QCOMPARE(ClangHelpers::stdVersionRank(QStringLiteral("-std=gnu++03")), V::Cpp03);
    QCOMPARE(ClangHelpers::stdVersionRank(QStringLiteral("-std=c++11")), V::Cpp11);
    QCOMPARE(ClangHelpers::stdVersionRank(QStringLiteral("-std=c++0x")), V::Cpp11);
    QCOMPARE(ClangHelpers::stdVersionRank(QStringLiteral("-std=gnu++11")), V::Cpp11);
    QCOMPARE(ClangHelpers::stdVersionRank(QStringLiteral("-std=c++14")), V::Cpp14);
    QCOMPARE(ClangHelpers::stdVersionRank(QStringLiteral("-std=c++1y")), V::Cpp14);
    QCOMPARE(ClangHelpers::stdVersionRank(QStringLiteral("-std=gnu++14")), V::Cpp14);
    QCOMPARE(ClangHelpers::stdVersionRank(QStringLiteral("-std=c++17")), V::Cpp17);
    QCOMPARE(ClangHelpers::stdVersionRank(QStringLiteral("-std=c++1z")), V::Cpp17);
    QCOMPARE(ClangHelpers::stdVersionRank(QStringLiteral("-std=gnu++17")), V::Cpp17);
    QCOMPARE(ClangHelpers::stdVersionRank(QStringLiteral("-std=c++20")), V::Cpp20);
    QCOMPARE(ClangHelpers::stdVersionRank(QStringLiteral("-std=c++2a")), V::Cpp20);
    QCOMPARE(ClangHelpers::stdVersionRank(QStringLiteral("-std=gnu++20")), V::Cpp20);
    QCOMPARE(ClangHelpers::stdVersionRank(QStringLiteral("-std=c++23")), V::Cpp23);
    QCOMPARE(ClangHelpers::stdVersionRank(QStringLiteral("-std=c++2b")), V::Cpp23);
    QCOMPARE(ClangHelpers::stdVersionRank(QStringLiteral("-std=gnu++23")), V::Cpp23);
    QCOMPARE(ClangHelpers::stdVersionRank(QStringLiteral("-std=c++26")), V::Cpp26);
    QCOMPARE(ClangHelpers::stdVersionRank(QStringLiteral("-std=c++2c")), V::Cpp26);
    QCOMPARE(ClangHelpers::stdVersionRank(QStringLiteral("-std=gnu++26")), V::Cpp26);
    QCOMPARE(ClangHelpers::stdVersionRank(QStringLiteral("-std=c++29")), V::Cpp29);
    QCOMPARE(ClangHelpers::stdVersionRank(QStringLiteral("-std=c++2d")), V::Cpp29);
    QCOMPARE(ClangHelpers::stdVersionRank(QStringLiteral("-std=gnu++29")), V::Cpp29);
    QCOMPARE(ClangHelpers::stdVersionRank(QStringLiteral("-std=c++latest")), V::Cpp26);
    QCOMPARE(ClangHelpers::stdVersionRank(QStringLiteral("-std=unknown")), V::Unknown);
    QCOMPARE(ClangHelpers::stdVersionRank(QString()), V::Unknown);

    QVERIFY(V::Unknown < V::Cpp98);
    QVERIFY(V::Cpp17 < V::Cpp20);
    QVERIFY(V::Cpp20 < V::Cpp23);
    QVERIFY(V::Cpp23 < V::Cpp26);
    QVERIFY(V::Cpp26 < V::Cpp29);

    QCOMPARE(ClangHelpers::extractStdFlag(QStringLiteral("-Wall -std=gnu++23 -O2")), QStringLiteral("-std=gnu++23"));
    QCOMPARE(ClangHelpers::extractStdFlag(QStringLiteral("-Wall -O2")), QString());
}

void TestModules::testIsCpp20OrLater()
{
    ParserSettings settings;
    settings.parserOptions = QStringLiteral("-std=c++17");
    QVERIFY(!ClangHelpers::isCpp20OrLater(settings));

    settings.parserOptions = QStringLiteral("-std=c++20");
    QVERIFY(ClangHelpers::isCpp20OrLater(settings));

    settings.parserOptions = QStringLiteral("-std=gnu++23");
    QVERIFY(ClangHelpers::isCpp20OrLater(settings));

    settings.parserOptions = QStringLiteral("-std=c++26");
    QVERIFY(ClangHelpers::isCpp20OrLater(settings));

    settings.parserOptions = QStringLiteral("-std=c++latest");
    QVERIFY(ClangHelpers::isCpp20OrLater(settings));
}

void TestModules::testRemoveHarmfulModuleArguments()
{
    QVector<QByteArray> byteArgs = {
        "-std=c++20",
        "-fmodule-mapper=/tmp/foo.modmap",
        "-fmodules-ts",
        "-I/usr/include",
        "-fdeps-format=p1689r5",
        "-fmodule-file=mod=mod.pcm",
        "-DFOO=1",
        "-fmodule-only",
        "-fdeps-format", "p1689r5",
    };

    ClangHelpers::removeHarmfulModuleArguments(byteArgs);

    QVERIFY(byteArgs.contains("-std=c++20"));
    QVERIFY(byteArgs.contains("-I/usr/include"));
    QVERIFY(byteArgs.contains("-fmodule-file=mod=mod.pcm"));
    QVERIFY(byteArgs.contains("-DFOO=1"));

    QVERIFY(!byteArgs.contains("-fmodules-ts"));
    QVERIFY(!byteArgs.contains("-fmodule-only"));
    QVERIFY(std::none_of(byteArgs.begin(), byteArgs.end(), [](const QByteArray& a) {
        return a.startsWith("-fmodule-mapper") || a.startsWith("-fdeps-format");
    }));

    QStringList strArgs = {
        QStringLiteral("-std=c++23"),
        QStringLiteral("-fmodule-mapper=build/bar.modmap"),
        QStringLiteral("-fmodules-ts"),
        QStringLiteral("-fmodule-file=std=std.pcm"),
        QStringLiteral("-O2"),
    };

    ClangHelpers::removeHarmfulModuleArguments(strArgs);

    QCOMPARE(strArgs, (QStringList{
        QStringLiteral("-std=c++23"),
        QStringLiteral("-fmodule-file=std=std.pcm"),
        QStringLiteral("-O2"),
    }));
}

void TestModules::testParseP1689Json()
{
    const QByteArray validP1689 = QByteArrayLiteral(R"({
  "version": 1,
  "revision": 0,
  "rules": [
    {
      "primary-output": "build/test_submodule.o",
      "provides": [
        {
          "logical-name": "test.submodule",
          "is-interface": true
        }
      ],
      "requires": [
        {
          "logical-name": "std"
        },
        {
          "logical-name": "module_example"
        }
      ]
    },
    {
      "primary-output": "build/consumer.o",
      "provides": [],
      "requires": [
        {
          "logical-name": "test.submodule"
        }
      ]
    }
  ]
})");

    QHash<QString, QString> outputToSource = {
        {QStringLiteral("build/test_submodule.o"), QStringLiteral("/path/to/test_submodule.cpp")},
        {QStringLiteral("build/consumer.o"), QStringLiteral("/path/to/consumer.cpp")},
    };

    const auto result = ClangModuleScanner::parseP1689Json(validP1689, outputToSource);
    QVERIFY(result.success);
    QCOMPARE(result.modules.size(), 1);

    const auto subDesc = result.modules.value(QStringLiteral("test.submodule"));
    QCOMPARE(subDesc.name, QStringLiteral("test.submodule"));
    QCOMPARE(subDesc.sourceFile, QStringLiteral("/path/to/test_submodule.cpp"));
    QVERIFY(subDesc.isInterface);
    QCOMPARE(subDesc.requiredModules.size(), 2);
    QVERIFY(subDesc.requiredModules.contains(QStringLiteral("std")));
    QVERIFY(subDesc.requiredModules.contains(QStringLiteral("module_example")));

    const auto consumerReqs = result.fileRequirements.value(QStringLiteral("/path/to/consumer.cpp"));
    QCOMPARE(consumerReqs, QStringList{QStringLiteral("test.submodule")});

    // Test invalid JSON error handling
    const auto badResult = ClangModuleScanner::parseP1689Json("invalid json");
    QVERIFY(!badResult.success);
    QVERIFY(!badResult.errorString.isEmpty());
}

void TestModules::testClangScanDeps()
{
    const QString scanDeps = ClangHelpers::clangScanDepsPath();
    if (scanDeps.isEmpty()) {
        QSKIP("clang-scan-deps is not available on this system");
    }

    const QString modulesDir = QStringLiteral(TEST_FILES_DIR "/modules");
    const QString subSrc = modulesDir + QStringLiteral("/test_submodule.cpp");

    const auto result = ClangModuleScanner::scanFile(subSrc, {QStringLiteral("-std=c++23"), QStringLiteral("-I") + modulesDir});
    if (!result.success) {
        // If scan-deps fails (e.g. clang-scan-deps missing headers in environment), skip gracefully
        qWarning() << "clang-scan-deps execution note:" << result.errorString;
        QSKIP("clang-scan-deps could not process file in current environment");
    }

    QVERIFY(result.modules.contains(QStringLiteral("test.submodule")));
    const auto desc = result.modules.value(QStringLiteral("test.submodule"));
    QCOMPARE(desc.name, QStringLiteral("test.submodule"));
    QVERIFY(desc.requiredModules.contains(QStringLiteral("module_example")));
}

void TestModules::testCancellation()
{
    const QString scanDeps = ClangHelpers::clangScanDepsPath();
    if (scanDeps.isEmpty()) {
        QSKIP("clang-scan-deps is not available on this system");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString sandbox = tempDir.path();
    const QString srcDir = sandbox + QStringLiteral("/src");
    const QString incDir = sandbox + QStringLiteral("/include");
    const QString buildDir = sandbox + QStringLiteral("/build");

    QDir().mkpath(srcDir);
    QDir().mkpath(incDir);
    QDir().mkpath(buildDir);

    const QString testFilesDir = QStringLiteral(TEST_FILES_DIR "/modules");
    QVERIFY(QFile::copy(testFilesDir + QStringLiteral("/export.cpp"), srcDir + QStringLiteral("/export.cpp")));
    QVERIFY(QFile::copy(testFilesDir + QStringLiteral("/test_module_export.h"), incDir + QStringLiteral("/test_module_export.h")));
    QVERIFY(QFile::copy(testFilesDir + QStringLiteral("/test_submodule.hpp"), incDir + QStringLiteral("/test_submodule.hpp")));
    QVERIFY(QFile::copy(testFilesDir + QStringLiteral("/test_module.hpp"), incDir + QStringLiteral("/test_module.hpp")));

    const QString expFile = srcDir + QStringLiteral("/export.cpp");
    const QStringList baseArgs = {QStringLiteral("-std=c++23"), QStringLiteral("-I") + incDir, QStringLiteral("-I") + srcDir};

    // 1. scanFile with immediate abort
    const auto abortedScan = ClangModuleScanner::scanFile(expFile, baseArgs, buildDir, [] {
        return true;
    });
    QVERIFY(!abortedScan.success);
}

void TestModules::testModuleGraphBasic()
{
    ClangModuleGraph graph;
    QCOMPARE(graph.size(), 0);
    QVERIFY(!graph.hasCycle());

    const IndexedString modA(QStringLiteral("mod.a"));
    const IndexedString modB(QStringLiteral("mod.b"));
    const IndexedString modC(QStringLiteral("mod.c"));

    graph.addModule(modA, {});
    graph.addModule(modB, {modA});
    graph.addModule(modC, {modB});

    QCOMPARE(graph.size(), 3);
    QVERIFY(graph.contains(modA));
    QVERIFY(graph.contains(modB));
    QVERIFY(graph.contains(modC));

    QCOMPARE(graph.dependenciesOf(modB), QList<IndexedString>{modA});
    QCOMPARE(graph.dependentsOf(modA), QList<IndexedString>{modB});

    graph.removeModule(modB);
    QCOMPARE(graph.size(), 2);
    QVERIFY(!graph.contains(modB));
    QVERIFY(graph.dependentsOf(modA).isEmpty());

    graph.clear();
    QCOMPARE(graph.size(), 0);
}

void TestModules::testModuleGraphTopologicalSort()
{
    ClangModuleGraph graph;
    const IndexedString a(QStringLiteral("A"));
    const IndexedString b(QStringLiteral("B"));
    const IndexedString c(QStringLiteral("C"));
    const IndexedString d(QStringLiteral("D"));

    graph.addModule(a, {});
    graph.addModule(b, {a});
    graph.addModule(c, {a});
    graph.addModule(d, {b, c});

    QVERIFY(!graph.hasCycle());
    const auto sortRes = graph.topologicalSort();
    QVERIFY(sortRes.success);
    QVERIFY(sortRes.cycle.isEmpty());
    QCOMPARE(sortRes.order.size(), 4);

    const int idxA = sortRes.order.indexOf(a);
    const int idxB = sortRes.order.indexOf(b);
    const int idxC = sortRes.order.indexOf(c);
    const int idxD = sortRes.order.indexOf(d);

    QVERIFY(idxA < idxB);
    QVERIFY(idxA < idxC);
    QVERIFY(idxB < idxD);
    QVERIFY(idxC < idxD);

    const auto depsD = graph.dependenciesFor(d);
    QVERIFY(depsD.success);
    QCOMPARE(depsD.order.last(), d);
    QVERIFY(depsD.order.indexOf(a) < depsD.order.indexOf(d));

    const auto depsB = graph.dependenciesFor(b);
    QVERIFY(depsB.success);
    QCOMPARE(depsB.order.size(), 2);
    QCOMPARE(depsB.order.at(0), a);
    QCOMPARE(depsB.order.at(1), b);
}

void TestModules::testModuleGraphCycleDetection()
{
    ClangModuleGraph graph;
    const IndexedString a(QStringLiteral("A"));
    const IndexedString b(QStringLiteral("B"));
    const IndexedString c(QStringLiteral("C"));

    graph.addModule(a, {b});
    graph.addModule(b, {a});

    QVERIFY(graph.hasCycle());
    const auto cycle = graph.findCycle();
    QVERIFY(!cycle.isEmpty());
    QCOMPARE(cycle.first(), cycle.last());
    const QString cycleStr = ClangModuleGraph::formatCycleString(cycle);
    QVERIFY(cycleStr.contains(QStringLiteral(" -> ")));

    const auto sortRes = graph.topologicalSort();
    QVERIFY(!sortRes.success);
    QVERIFY(!sortRes.cycle.isEmpty());

    graph.clear();
    graph.addModule(a, {b});
    graph.addModule(b, {c});
    graph.addModule(c, {a});

    QVERIFY(graph.hasCycle());
    const auto cycle3 = graph.findCycle();
    QCOMPARE(cycle3.size(), 4);
    QCOMPARE(cycle3.first(), cycle3.last());

    graph.clear();
    graph.addModule(a, {a});
    QVERIFY(graph.hasCycle());
    const auto selfCycle = graph.findCycle();
    QCOMPARE(selfCycle, (QList<IndexedString>{a, a}));
}

void TestModules::testModuleGraphMultiTargetSort()
{
    ClangModuleGraph graph;
    const IndexedString a(QStringLiteral("mod.A"));
    const IndexedString b(QStringLiteral("mod.B"));
    const IndexedString c(QStringLiteral("mod.C"));
    const IndexedString d(QStringLiteral("mod.D"));

    graph.addModule(a, {b});
    graph.addModule(b, {});
    graph.addModule(c, {b});
    graph.addModule(d, {c});

    const auto res = graph.dependenciesFor(QList<IndexedString>{a, d});
    QVERIFY(res.success);
    QVERIFY(res.cycle.isEmpty());

    const int idxB = res.order.indexOf(b);
    const int idxA = res.order.indexOf(a);
    const int idxC = res.order.indexOf(c);
    const int idxD = res.order.indexOf(d);

    QVERIFY(idxB != -1);
    QVERIFY(idxA != -1);
    QVERIFY(idxC != -1);
    QVERIFY(idxD != -1);

    QVERIFY(idxB < idxA);
    QVERIFY(idxB < idxC);
    QVERIFY(idxC < idxD);
}

void TestModules::testModuleRepository()
{
    auto& repo = ClangModuleRepository::self();
    repo.clear();
    QCOMPARE(repo.allModules().size(), 0);

    const IndexedString modName(QStringLiteral("test.repository.module"));
    const IndexedString srcFile(QStringLiteral("/path/to/repo_mod.cppm"));
    const IndexedString workDir(QStringLiteral("/path/to/build"));
    const qint64 modTime = 123456789;
    const QList<IndexedString> deps = {IndexedString(QStringLiteral("dep.one")), IndexedString(QStringLiteral("dep.two"))};

    ClangModuleRepository::ModuleMetadata meta;
    meta.moduleName = modName;
    meta.sourceFile = srcFile;
    meta.workingDir = workDir;
    meta.sourceModificationTime = modTime;
    meta.dependencies = deps;

    repo.insert(meta);

    QVERIFY(repo.contains(modName));
    QCOMPARE(repo.allModules().size(), 1);
    QCOMPARE(repo.allModules().first(), modName);

    const auto found = repo.find(modName);
    QVERIFY(found.has_value());
    QCOMPARE(found->moduleName, modName);
    QCOMPARE(found->sourceFile, srcFile);
    QCOMPARE(found->workingDir, workDir);
    QCOMPARE(found->sourceModificationTime, modTime);
    QCOMPARE(found->dependencies, deps);

    ClangModuleGraph graph;
    repo.loadIntoGraph(graph);
    QVERIFY(graph.contains(modName));
    QCOMPARE(graph.dependenciesOf(modName), deps);

    repo.remove(modName);
    QVERIFY(!repo.contains(modName));
    QVERIFY(!repo.find(modName).has_value());
    QCOMPARE(repo.allModules().size(), 0);

    // Test FileScanMetadata storage
    const IndexedString consumerFile(QStringLiteral("/path/to/consumer.cpp"));
    ClangModuleRepository::FileScanMetadata fileMeta;
    fileMeta.sourceFile = consumerFile;
    fileMeta.sourceModificationTime = 987654321;
    fileMeta.requiredModules = {modName};
    fileMeta.compileFlags = {IndexedString(QStringLiteral("-std=c++23")), IndexedString(QStringLiteral("-DFOO=1"))};
    fileMeta.moduleArguments = {IndexedString(QStringLiteral("-fmodule-file=test.repository.module=test.pcm"))};

    repo.insertFileScan(fileMeta);
    const auto foundScan = repo.findFileScan(consumerFile);
    QVERIFY(foundScan.has_value());
    QCOMPARE(foundScan->sourceFile, consumerFile);
    QCOMPARE(foundScan->sourceModificationTime, 987654321);
    QCOMPARE(foundScan->requiredModules, fileMeta.requiredModules);
    QCOMPARE(foundScan->compileFlags, fileMeta.compileFlags);
    QCOMPARE(foundScan->moduleArguments, fileMeta.moduleArguments);

    repo.removeFileScan(consumerFile);
    QVERIFY(!repo.findFileScan(consumerFile).has_value());
}
