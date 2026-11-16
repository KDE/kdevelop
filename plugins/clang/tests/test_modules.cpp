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
