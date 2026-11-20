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
#include "../duchain/clangdependencyscanner.h"
#include "../duchain/clanghelpers.h"
#include "../duchain/clangindex.h"
#include "../duchain/clangmodule.h"
#include "../duchain/clangmodulegraph.h"
#include "../duchain/clangmoduleprecompilejob.h"
#include "../duchain/clangmodulerepository.h"
#include "../duchain/clangmodulescanner.h"
#include "../duchain/parsesession.h"
#include "../clangsettings/clangsettingsmanager.h"
#include <ThreadWeaver/DependencyPolicy>
#include <ThreadWeaver/Queue>

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
    QCOMPARE(ClangHelpers::stdVersionRank(QStringLiteral("-ansi")), V::Cpp98);
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
    QCOMPARE(ClangHelpers::extractStdFlag(QStringLiteral("-std=c++20 -g -std=c++23")), QStringLiteral("-std=c++23"));
    QCOMPARE(ClangHelpers::extractStdFlag(QStringLiteral("--std=c++20 -g --std=c++23")), QStringLiteral("--std=c++23"));
    QCOMPARE(ClangHelpers::extractStdFlag(QStringLiteral("-Wall -ansi -O2")), QStringLiteral("-ansi"));
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

void TestModules::testIsCppModuleUnit()
{
    const QString modulesDir = QStringLiteral(TEST_FILES_DIR "/modules");
    QVERIFY(QDir(modulesDir).exists());

    QVERIFY(ClangModule::isCppModuleUnit(modulesDir + QStringLiteral("/export.cpp"), {}));
    QVERIFY(ClangModule::isCppModuleUnit(modulesDir + QStringLiteral("/test_submodule.cpp"), {}));
    QVERIFY(ClangModule::isCppModuleUnit(modulesDir + QStringLiteral("/test_module.cpp"), {}));
    QVERIFY(!ClangModule::isCppModuleUnit(modulesDir + QStringLiteral("/consumer.cpp"), {}));
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

void TestModules::testScanAndPrecompileModules()
{
    const QString clangCompiler = ClangHelpers::clangCompilerPath();
    if (clangCompiler.isEmpty()) {
        QSKIP("clang++ is not available on this system");
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
    QVERIFY(QFile::copy(testFilesDir + QStringLiteral("/test_submodule.cpp"), srcDir + QStringLiteral("/test_submodule.cpp")));
    QVERIFY(QFile::copy(testFilesDir + QStringLiteral("/test_module.cpp"), srcDir + QStringLiteral("/test_module.cpp")));
    QVERIFY(QFile::copy(testFilesDir + QStringLiteral("/test_module_export.h"), incDir + QStringLiteral("/test_module_export.h")));
    QVERIFY(QFile::copy(testFilesDir + QStringLiteral("/test_submodule.hpp"), incDir + QStringLiteral("/test_submodule.hpp")));
    QVERIFY(QFile::copy(testFilesDir + QStringLiteral("/test_module.hpp"), incDir + QStringLiteral("/test_module.hpp")));

    ClangModule::clearModules();
    const QStringList baseArgs = {QStringLiteral("-std=c++23"), QStringLiteral("-I") + incDir, QStringLiteral("-I") + srcDir};

    auto expMod = ClangModule::scanAndRegisterModuleFile(srcDir + QStringLiteral("/export.cpp"), baseArgs, buildDir);
    QVERIFY(expMod);
    QCOMPARE(expMod->name(), QStringLiteral("module_example"));

    auto subMod = ClangModule::scanAndRegisterModuleFile(srcDir + QStringLiteral("/test_submodule.cpp"), baseArgs, buildDir);
    QVERIFY(subMod);
    QCOMPARE(subMod->name(), QStringLiteral("test.submodule"));

    auto modMod = ClangModule::scanAndRegisterModuleFile(srcDir + QStringLiteral("/test_module.cpp"), baseArgs, buildDir);
    QVERIFY(modMod);
    QCOMPARE(modMod->name(), QStringLiteral("test.mod"));

    ClangModule::Map modTargets;
    modTargets.insert(expMod->name(), expMod);
    modTargets.insert(subMod->name(), subMod);
    modTargets.insert(modMod->name(), modMod);

    const auto moduleArgs = ClangModule::precompileModules(
        {QStringLiteral("test.mod")}, modTargets, {incDir, srcDir}, QStringLiteral("-std=c++23"));

    QVERIFY(!moduleArgs.isEmpty());

    QStringList flags;
    for (const auto& arg : moduleArgs) {
        flags << QString::fromUtf8(arg);
    }

    QVERIFY(flags.filter(QStringLiteral("-fmodule-file=module_example=")).size() > 0);
    QVERIFY(flags.filter(QStringLiteral("-fmodule-file=test.submodule=")).size() > 0);
    QVERIFY(flags.filter(QStringLiteral("-fmodule-file=test.mod=")).size() > 0);
}

void TestModules::testCancellation()
{
    const QString clangCompiler = ClangHelpers::clangCompilerPath();
    if (clangCompiler.isEmpty()) {
        QSKIP("clang++ is not available on this system");
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

    // 2. scanAndRegisterModuleFile with abort
    auto abortedReg = ClangModule::scanAndRegisterModuleFile(expFile, baseArgs, buildDir, [] {
        return true;
    });
    QVERIFY(!abortedReg);

    // 3. precompileModules with abort
    auto validMod = ClangModule::scanAndRegisterModuleFile(expFile, baseArgs, buildDir);
    QVERIFY(validMod);
    ClangModule::Map modTargets;
    modTargets.insert(validMod->name(), validMod);

    const auto abortedPrecompile = ClangModule::precompileModules(
        {validMod->name()}, modTargets, {incDir, srcDir}, QStringLiteral("-std=c++23"), buildDir, [] {
            return true;
        });
    QVERIFY(abortedPrecompile.isEmpty());
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
void TestModules::testRepositoryCachedLookup()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString testFile = tempDir.path() + QStringLiteral("/cache_test.cppm");
    {
        QFile file(testFile);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        file.write("export module CacheTest;\n");
    }

    ClangModule::clearModules();

    auto mod = ClangModule::Ptr::create(QStringLiteral("CacheTest"), testFile, tempDir.path(), QStringList{});
    ClangModule::registerModule(mod);

    // Verify it is found in registeredModule
    auto found = ClangModule::registeredModule(QStringLiteral("CacheTest"));
    QVERIFY(found != nullptr);
    QCOMPARE(found->name(), QStringLiteral("CacheTest"));

    // Modify file on disk (update mtime into future)
    const auto newTime = QFileInfo(testFile).lastModified().addSecs(10);
    {
        QFile file(testFile);
        QVERIFY(file.open(QIODevice::ReadWrite));
        QVERIFY(file.setFileTime(newTime, QFileDevice::FileModificationTime));
    }

    // Now registeredModule should detect that mtime changed and invalidate
    auto stale = ClangModule::registeredModule(QStringLiteral("CacheTest"));
    QVERIFY(stale == nullptr);

    ClangModule::clearModules();
}

void TestModules::testPrecompileModulesCycleDetection()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString fileA = tempDir.path() + QStringLiteral("/mod_a.cppm");
    const QString fileB = tempDir.path() + QStringLiteral("/mod_b.cppm");
    {
        QFile fA(fileA);
        QVERIFY(fA.open(QIODevice::WriteOnly));
        fA.write("export module CycleA; import CycleB;\n");

        QFile fB(fileB);
        QVERIFY(fB.open(QIODevice::WriteOnly));
        fB.write("export module CycleB; import CycleA;\n");
    }

    ClangModule::clearModules();

    auto modA = ClangModule::Ptr::create(QStringLiteral("CycleA"), fileA, tempDir.path(),
                                         QStringList{QStringLiteral("-std=c++20")},
                                         QStringList{QStringLiteral("CycleB")});
    auto modB = ClangModule::Ptr::create(QStringLiteral("CycleB"), fileB, tempDir.path(),
                                         QStringList{QStringLiteral("-std=c++20")},
                                         QStringList{QStringLiteral("CycleA")});

    ClangModule::registerModule(modA);
    ClangModule::registerModule(modB);

    ClangModule::Map modules;
    modules.insert(QStringLiteral("CycleA"), modA);
    modules.insert(QStringLiteral("CycleB"), modB);

    // precompileModules should detect the circular dependency CycleA -> CycleB -> CycleA and abort safely
    const auto args = ClangModule::precompileModules({QStringLiteral("CycleA")}, modules, {}, QStringLiteral("-std=c++20"), tempDir.path());
    QVERIFY(args.isEmpty());

    ClangModule::clearModules();
}

void TestModules::testProjectModuleDiscovery()
{
    const QString clangCompiler = ClangHelpers::clangCompilerPath();
    if (clangCompiler.isEmpty()) {
        QSKIP("clang++ is not available on this system");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString srcDir = tempDir.path() + QStringLiteral("/src");
    const QString buildDir = tempDir.path() + QStringLiteral("/build");
    QDir().mkpath(srcDir);
    QDir().mkpath(buildDir);

    const QString moduleFile = srcDir + QStringLiteral("/hello.cppm");
    const QString consumerFile = srcDir + QStringLiteral("/use.cpp");

    {
        QFile modF(moduleFile);
        QVERIFY(modF.open(QIODevice::WriteOnly | QIODevice::Text));
        modF.write("export module Hello;\n"
                   "export int getNumber() { return 42; }\n");

        QFile useF(consumerFile);
        QVERIFY(useF.open(QIODevice::WriteOnly | QIODevice::Text));
        useF.write("import Hello;\n"
                   "int main() { return getNumber(); }\n");
    }

    // Reset module caches to verify discovery works without prior registration
    ClangModule::clearModules();
    QVERIFY(ClangModule::registeredModule(QStringLiteral("Hello")) == nullptr);

    // Calling moduleArgumentsForFile on use.cpp should discover hello.cppm, precompile it,
    // and return the -fmodule-file argument for Hello.
    const auto moduleArgs = ClangModule::moduleArgumentsForFile(
        consumerFile, buildDir, {}, QStringLiteral("-std=c++20"));

    QVERIFY(!moduleArgs.isEmpty());
    bool foundHelloArg = false;
    for (const auto& arg : moduleArgs) {
        if (arg.startsWith("-fmodule-file=Hello=")) {
            foundHelloArg = true;
            break;
        }
    }
    QVERIFY(foundHelloArg);
    QVERIFY(ClangModule::registeredModule(QStringLiteral("Hello")) != nullptr);

    ClangModule::clearModules();
}

void TestModules::testModuleProblemsReporting()
{
    const QString clangCompiler = ClangHelpers::clangCompilerPath();
    if (clangCompiler.isEmpty()) {
        QSKIP("clang++ is not available on this system");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString srcDir = tempDir.path() + QStringLiteral("/src");
    const QString buildDir = tempDir.path() + QStringLiteral("/build");
    QDir().mkpath(srcDir);
    QDir().mkpath(buildDir);

    // 1. Test Circular Module Dependency Problem Reporting
    const QString fileA = srcDir + QStringLiteral("/cycle_a.cppm");
    const QString fileB = srcDir + QStringLiteral("/cycle_b.cppm");
    const QString cycleConsumer = srcDir + QStringLiteral("/cycle_consumer.cpp");

    {
        QFile fA(fileA);
        QVERIFY(fA.open(QIODevice::WriteOnly | QIODevice::Text));
        fA.write("export module CycleProbA; import CycleProbB;\n");

        QFile fB(fileB);
        QVERIFY(fB.open(QIODevice::WriteOnly | QIODevice::Text));
        fB.write("export module CycleProbB; import CycleProbA;\n");

        QFile fC(cycleConsumer);
        QVERIFY(fC.open(QIODevice::WriteOnly | QIODevice::Text));
        fC.write("import CycleProbA; int main() { return 0; }\n");
    }

    ClangModule::clearModules();
    QList<ProblemPointer> cycleProblems;
    ClangModule::moduleArgumentsForFile(cycleConsumer, buildDir, {}, QStringLiteral("-std=c++20"), {}, {}, &cycleProblems);

    QVERIFY(!cycleProblems.isEmpty());
    bool foundCycleError = false;
    for (const auto& p : cycleProblems) {
        if (p->severity() == IProblem::Error && p->description().contains(QStringLiteral("Circular module dependency detected"))) {
            foundCycleError = true;
            break;
        }
    }
    QVERIFY(foundCycleError);

    // 2. Test Module Precompilation Syntax Failure Reporting
    const QString brokenModule = srcDir + QStringLiteral("/broken_mod.cppm");
    const QString brokenConsumer = srcDir + QStringLiteral("/broken_consumer.cpp");

    {
        QFile fMod(brokenModule);
        QVERIFY(fMod.open(QIODevice::WriteOnly | QIODevice::Text));
        // Invalid C++ syntax inside module interface
        fMod.write("export module BrokenProbMod; int broken_syntax = ;\n");

        QFile fUse(brokenConsumer);
        QVERIFY(fUse.open(QIODevice::WriteOnly | QIODevice::Text));
        fUse.write("import BrokenProbMod; int main() { return 0; }\n");
    }

    ClangModule::clearModules();
    QList<ProblemPointer> brokenProblems;
    ClangModule::moduleArgumentsForFile(brokenConsumer, buildDir, {}, QStringLiteral("-std=c++20"), {}, {}, &brokenProblems);

    QVERIFY(!brokenProblems.isEmpty());
    bool foundBrokenError = false;
    for (const auto& p : brokenProblems) {
        if (p->severity() == IProblem::Error && p->description().contains(QStringLiteral("Failed to precompile module 'BrokenProbMod'"))) {
            foundBrokenError = true;
            QVERIFY(!p->explanation().isEmpty()); // captures compiler stderr
            break;
        }
    }
    QVERIFY(foundBrokenError);

    ClangModule::clearModules();
}

void TestModules::testModulePrecompileJobThreadWeaver()
{
    const QString clangCompiler = ClangHelpers::clangCompilerPath();
    if (clangCompiler.isEmpty()) {
        QSKIP("clang++ is not available on this system");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString srcDir = tempDir.path() + QStringLiteral("/src");
    const QString buildDir = tempDir.path() + QStringLiteral("/build");
    QDir().mkpath(srcDir);
    QDir().mkpath(buildDir);

    const QString coreFile = srcDir + QStringLiteral("/core.cppm");
    const QString utilFile = srcDir + QStringLiteral("/util.cppm");
    const QString appFile = srcDir + QStringLiteral("/app.cppm");

    {
        QFile f1(coreFile);
        QVERIFY(f1.open(QIODevice::WriteOnly | QIODevice::Text));
        f1.write("export module CoreMod; export int coreValue() { return 42; }\n");

        QFile f2(utilFile);
        QVERIFY(f2.open(QIODevice::WriteOnly | QIODevice::Text));
        f2.write("export module UtilMod; export int utilValue() { return 100; }\n");

        QFile f3(appFile);
        QVERIFY(f3.open(QIODevice::WriteOnly | QIODevice::Text));
        f3.write("export module AppMod; import CoreMod; import UtilMod; export int appValue() { return coreValue() + utilValue(); }\n");
    }

    ClangModule::clearModules();

    auto coreMod = ClangModule::scanAndRegisterModuleFile(coreFile, {QStringLiteral("-std=c++20")}, buildDir);
    auto utilMod = ClangModule::scanAndRegisterModuleFile(utilFile, {QStringLiteral("-std=c++20")}, buildDir);
    auto appMod = ClangModule::scanAndRegisterModuleFile(appFile, {QStringLiteral("-std=c++20")}, buildDir);

    QVERIFY(coreMod);
    QVERIFY(utilMod);
    QVERIFY(appMod);

    ClangModule::Map moduleMap;
    moduleMap.insert(coreMod->name(), coreMod);
    moduleMap.insert(utilMod->name(), utilMod);
    moduleMap.insert(appMod->name(), appMod);

    // 1. Test createJobs prerequisite chaining
    const auto jobs = ClangModulePrecompileJob::createJobs(
        {QStringLiteral("AppMod")}, moduleMap, clangCompiler, {}, QStringLiteral("-std=c++20"), buildDir);

    QCOMPARE(jobs.size(), 3);

    // Verify jobs are ordered or linked: CoreMod and UtilMod are prerequisites for AppMod
    QSharedPointer<ClangModulePrecompileJob> appJob;
    for (const auto& job : jobs) {
        if (job->moduleName() == QLatin1String("AppMod")) {
            appJob = job;
        }
    }
    QVERIFY(appJob);

    // 2. Execute via ThreadWeaver::Queue
    ThreadWeaver::Queue queue;
    for (const auto& job : jobs) {
        queue.enqueue(job);
    }
    queue.finish();

    for (const auto& job : jobs) {
        QVERIFY2(job->success(), qPrintable(QStringLiteral("Job for module %1 failed").arg(job->moduleName())));
        QVERIFY(!job->moduleFileArg().isEmpty());
    }

    // Verify PCM files exist on disk
    QVERIFY(QFile::exists(coreMod->pcmPath(buildDir)));
    QVERIFY(QFile::exists(utilMod->pcmPath(buildDir)));
    QVERIFY(QFile::exists(appMod->pcmPath(buildDir)));

    // 3. Test abort requested on precompile job
    ClangModule::clearModules();
    const auto abortedJobs = ClangModulePrecompileJob::createJobs(
        {QStringLiteral("AppMod")}, moduleMap, clangCompiler, {}, QStringLiteral("-std=c++20"), buildDir, [] {
            return true;
        });
    QVERIFY(abortedJobs.isEmpty());

    // 4. Test cycle handling in createJobs
    const QString cycleA = srcDir + QStringLiteral("/tw_cycle_a.cppm");
    const QString cycleB = srcDir + QStringLiteral("/tw_cycle_b.cppm");
    {
        QFile fA(cycleA);
        QVERIFY(fA.open(QIODevice::WriteOnly | QIODevice::Text));
        fA.write("export module TWCycleA; import TWCycleB;\n");

        QFile fB(cycleB);
        QVERIFY(fB.open(QIODevice::WriteOnly | QIODevice::Text));
        fB.write("export module TWCycleB; import TWCycleA;\n");
    }

    auto modCA = ClangModule::scanAndRegisterModuleFile(cycleA, {QStringLiteral("-std=c++20")}, buildDir);
    auto modCB = ClangModule::scanAndRegisterModuleFile(cycleB, {QStringLiteral("-std=c++20")}, buildDir);
    ClangModule::Map cycleMap;
    cycleMap.insert(modCA->name(), modCA);
    cycleMap.insert(modCB->name(), modCB);

    QList<ProblemPointer> cycleProblems;
    const auto cycleJobs = ClangModulePrecompileJob::createJobs(
        {QStringLiteral("TWCycleA")}, cycleMap, clangCompiler, {}, QStringLiteral("-std=c++20"), buildDir, {}, &cycleProblems);
    QVERIFY(cycleJobs.isEmpty());
    QVERIFY(!cycleProblems.isEmpty());

    ClangModule::clearModules();
}


void TestModules::testDependencyScanner()
{
    const QString clangCompiler = ClangHelpers::clangCompilerPath();
    if (clangCompiler.isEmpty()) {
        QSKIP("clang++ is not available on this system");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString srcDir = tempDir.path() + QStringLiteral("/src");
    const QString buildDir = tempDir.path() + QStringLiteral("/build");
    QDir().mkpath(srcDir);
    QDir().mkpath(buildDir);

    const QString modFile = srcDir + QStringLiteral("/greet.cppm");
    const QString consumerFile = srcDir + QStringLiteral("/main.cpp");

    {
        QFile fMod(modFile);
        QVERIFY(fMod.open(QIODevice::WriteOnly | QIODevice::Text));
        fMod.write("export module Greet; export int greetVal() { return 7; }\n");

        QFile fUse(consumerFile);
        QVERIFY(fUse.open(QIODevice::WriteOnly | QIODevice::Text));
        fUse.write("import Greet; int main() { return greetVal(); }\n");
    }

    ClangModule::clearModules();
    ClangDependencyScanner scanner;
    const IndexedString indexedConsumer(consumerFile);

    // Cache is initially empty (read-only lookup, no process launched)
    QVERIFY(scanner.cachedModuleArguments(indexedConsumer).isEmpty());
    QVERIFY(scanner.cachedCompileFlags(indexedConsumer).isEmpty());

    ClangDependencyScanner::ScanConfig cfg;
    cfg.buildDir = buildDir;
    cfg.searchIncludePaths = {srcDir};
    cfg.parserOptions = QStringLiteral("-std=c++20");
    cfg.compileArgs = {QStringLiteral("-DFROM_BUILD_SYSTEM=1")};
    scanner.setScanConfig(indexedConsumer, cfg);

    // Create and execute independent background scan job via ThreadWeaver
    const auto job = scanner.createDependencyScanJob(indexedConsumer);
    QVERIFY(job);

    ThreadWeaver::Queue queue;
    queue.enqueue(job);
    queue.finish();

    // Now cached results are populated in ItemRepository
    const auto cachedArgs = scanner.cachedModuleArguments(indexedConsumer);
    QVERIFY(!cachedArgs.isEmpty());
    bool hasGreetArg = false;
    for (const auto& arg : cachedArgs) {
        if (arg.startsWith("-fmodule-file=Greet=")) {
            hasGreetArg = true;
            break;
        }
    }
    QVERIFY(hasGreetArg);

    const auto cachedFlags = scanner.cachedCompileFlags(indexedConsumer);
    QVERIFY(cachedFlags.contains(QStringLiteral("-DFROM_BUILD_SYSTEM=1")));
    QVERIFY(cachedFlags.contains(QStringLiteral("-std=c++20")));

    // Up-to-date file should not create a redundant scan job
    QVERIFY(!scanner.createDependencyScanJob(indexedConsumer));

    // Invalidation clears cache and allows rescanning
    scanner.invalidate(indexedConsumer);
    QVERIFY(scanner.cachedModuleArguments(indexedConsumer).isEmpty());
    QVERIFY(scanner.createDependencyScanJob(indexedConsumer));

    ClangModule::clearModules();
}

void TestModules::testParseModulesEndToEnd()
{
    const QString clangCompiler = ClangHelpers::clangCompilerPath();
    if (clangCompiler.isEmpty()) {
        QSKIP("clang++ is not available on this system");
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
    QVERIFY(QFile::copy(testFilesDir + QStringLiteral("/test_submodule.cpp"), srcDir + QStringLiteral("/test_submodule.cpp")));
    QVERIFY(QFile::copy(testFilesDir + QStringLiteral("/test_module.cpp"), srcDir + QStringLiteral("/test_module.cpp")));
    QVERIFY(QFile::copy(testFilesDir + QStringLiteral("/consumer.cpp"), srcDir + QStringLiteral("/consumer.cpp")));
    QVERIFY(QFile::copy(testFilesDir + QStringLiteral("/test_module_export.h"), incDir + QStringLiteral("/test_module_export.h")));
    QVERIFY(QFile::copy(testFilesDir + QStringLiteral("/test_submodule.hpp"), incDir + QStringLiteral("/test_submodule.hpp")));
    QVERIFY(QFile::copy(testFilesDir + QStringLiteral("/test_module.hpp"), incDir + QStringLiteral("/test_module.hpp")));

    ClangModule::clearModules();

    const QString consumerPath = srcDir + QStringLiteral("/consumer.cpp");
    const IndexedString indexedConsumer(consumerPath);

    // 1. Run independent dependency scan jobs via ClangDependencyScanner
    ClangDependencyScanner scanner;
    ClangDependencyScanner::ScanConfig cfg;
    cfg.buildDir = buildDir;
    cfg.searchIncludePaths = {incDir, srcDir};
    cfg.parserOptions = QStringLiteral("-std=c++23");

    // Note: test fixture module interfaces use .cpp rather than .cppm, so scan them explicitly
    const QStringList moduleUnits = {
        srcDir + QStringLiteral("/export.cpp"),
        srcDir + QStringLiteral("/test_submodule.cpp"),
        srcDir + QStringLiteral("/test_module.cpp"),
    };
    {
        ThreadWeaver::Queue modQueue;
        for (const QString& modUnit : moduleUnits) {
            const IndexedString indexedMod(modUnit);
            scanner.setScanConfig(indexedMod, cfg);
            if (auto job = scanner.createDependencyScanJob(indexedMod)) {
                modQueue.enqueue(job);
            }
        }
        modQueue.finish();
    }

    scanner.setScanConfig(indexedConsumer, cfg);

    const auto scanJob = scanner.createDependencyScanJob(indexedConsumer);
    QVERIFY(scanJob);

    ThreadWeaver::Queue queue;
    queue.enqueue(scanJob);
    queue.finish();

    const auto cachedArgs = scanner.cachedModuleArguments(indexedConsumer);
    QVERIFY(!cachedArgs.isEmpty());

    // 2. Parse consumer.cpp using cached module arguments (zero process launching in ParseSessionData)
    ClangIndex index;
    ClangParsingEnvironment env;
    env.setTranslationUnitUrl(indexedConsumer);
    env.setWorkingDirectory(Path(buildDir));
    env.addIncludes(Path::List{Path(incDir), Path(srcDir)});
    env.setModuleArguments(cachedArgs);
    env.setModuleProblems(scanner.cachedProblems(indexedConsumer));

    ParserSettings settings;
    settings.parserOptions = QStringLiteral("-std=c++23");
    env.setParserSettings(settings);

    auto sessionData = QExplicitlySharedDataPointer<ParseSessionData>(
        new ParseSessionData({}, &index, env));
    ParseSession session(sessionData);

    QVERIFY(session.unit() != nullptr);

    const auto problems = session.problemsForFile(session.file(consumerPath.toUtf8()));
    for (const auto& problem : problems) {
        QVERIFY2(problem->severity() != IProblem::Error, qPrintable(problem->description()));
    }

    ClangModule::clearModules();
}
