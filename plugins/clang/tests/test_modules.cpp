/*
    SPDX-FileCopyrightText: 2026 Fernando Vilas <fvilas@iname.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_modules.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QTest>

#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <tests/testplugincontroller.h>

#include "testfilepaths.h"
#include "../duchain/clanghelpers.h"
#include "../duchain/clangindex.h"
#include "../duchain/clangmodule.h"
#include "../duchain/parsesession.h"

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
    QCOMPARE(ClangModule::stdVersionRank(QStringLiteral("-std=c++14")), 14);
    QCOMPARE(ClangModule::stdVersionRank(QStringLiteral("-std=c++17")), 17);
    QCOMPARE(ClangModule::stdVersionRank(QStringLiteral("-std=c++20")), 20);
    QCOMPARE(ClangModule::stdVersionRank(QStringLiteral("-std=c++2a")), 20);
    QCOMPARE(ClangModule::stdVersionRank(QStringLiteral("-std=gnu++20")), 20);
    QCOMPARE(ClangModule::stdVersionRank(QStringLiteral("-std=c++23")), 23);
    QCOMPARE(ClangModule::stdVersionRank(QStringLiteral("-std=c++2b")), 23);
    QCOMPARE(ClangModule::stdVersionRank(QStringLiteral("-std=gnu++23")), 23);
    QCOMPARE(ClangModule::stdVersionRank(QStringLiteral("-std=c++26")), 26);
    QCOMPARE(ClangModule::stdVersionRank(QStringLiteral("-std=c++2c")), 26);
    QCOMPARE(ClangModule::stdVersionRank(QStringLiteral("-std=gnu++26")), 26);
    QCOMPARE(ClangModule::stdVersionRank(QStringLiteral("-std=c++latest")), 99);
    QCOMPARE(ClangModule::stdVersionRank(QStringLiteral("-std=unknown")), 0);
    QCOMPARE(ClangModule::stdVersionRank(QString()), 0);
}

void TestModules::testIsCpp20OrLater()
{
    ParserSettings settings;
    settings.parserOptions = QStringLiteral("-std=c++17");
    QVERIFY(!ClangModule::isCpp20OrLater(settings));

    settings.parserOptions = QStringLiteral("-std=c++20");
    QVERIFY(ClangModule::isCpp20OrLater(settings));

    settings.parserOptions = QStringLiteral("-std=gnu++23");
    QVERIFY(ClangModule::isCpp20OrLater(settings));

    settings.parserOptions = QStringLiteral("-std=c++26");
    QVERIFY(ClangModule::isCpp20OrLater(settings));

    settings.parserOptions = QStringLiteral("-std=c++latest");
    QVERIFY(ClangModule::isCpp20OrLater(settings));
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

void TestModules::testGccModmapResolutionAndPrecompilation()
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

    // Write dynamic relative .modmap files
    const QString exportModmap = buildDir + QStringLiteral("/export.modmap");
    const QString subModmap = buildDir + QStringLiteral("/test_submodule.modmap");
    const QString modModmap = buildDir + QStringLiteral("/test_module.modmap");

    {
        QFile file(exportModmap);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        file.write("$root .\nmodule_example export.gcm\n");
    }
    {
        QFile file(subModmap);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        file.write("$root .\ntest.submodule test_submodule.gcm\nstd std.gcm\nmodule_example export.gcm\n");
    }
    {
        QFile file(modModmap);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        file.write("$root .\ntest.mod test_module.gcm\ntest.submodule test_submodule.gcm\nmodule_example export.gcm\n");
    }

    // Write dynamic compile_commands.json
    const QString compileCommandsFile = buildDir + QStringLiteral("/compile_commands.json");
    {
        QFile file(compileCommandsFile);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        const QString json = QStringLiteral(R"([
  {
    "directory": "%1",
    "command": "g++ -std=c++23 -fPIC -I%2 -I%3 -fmodule-mapper=%4 -c %3/export.cpp",
    "file": "%3/export.cpp"
  },
  {
    "directory": "%1",
    "command": "g++ -std=c++23 -fPIC -I%2 -I%3 -fmodule-mapper=%5 -c %3/test_submodule.cpp",
    "file": "%3/test_submodule.cpp"
  },
  {
    "directory": "%1",
    "command": "g++ -std=c++23 -fPIC -I%2 -I%3 -fmodule-mapper=%6 -c %3/test_module.cpp",
    "file": "%3/test_module.cpp"
  }
])").arg(buildDir, incDir, srcDir, exportModmap, subModmap, modModmap);
        file.write(json.toUtf8());
    }

    CXCompilationDatabase_Error err = CXCompilationDatabase_NoError;
    CXCompilationDatabase db = clang_CompilationDatabase_fromDirectory(buildDir.toUtf8().constData(), &err);
    QVERIFY(err == CXCompilationDatabase_NoError && db != nullptr);

    const auto targets = ClangModule::scanGccModules(db, buildDir);
    QCOMPARE(targets.size(), 3);
    QVERIFY(targets.contains(QStringLiteral("module_example")));
    QVERIFY(targets.contains(QStringLiteral("test.submodule")));
    QVERIFY(targets.contains(QStringLiteral("test.mod")));

    // Process modmap for test_module.cpp and assert all required module flags are generated
    const auto moduleArgs = ClangModule::processGccModmap(
        modModmap, db, buildDir, false, {incDir, srcDir}, QStringLiteral("-std=c++23"));

    clang_CompilationDatabase_dispose(db);

    QVERIFY(!moduleArgs.isEmpty());

    // Verify generated module-file flags
    QStringList flags;
    for (const auto& arg : moduleArgs) {
        flags << QString::fromUtf8(arg);
    }

    QVERIFY(flags.filter(QStringLiteral("-fmodule-file=module_example=")).size() > 0);
    QVERIFY(flags.filter(QStringLiteral("-fmodule-file=test.submodule=")).size() > 0);
    QVERIFY(flags.filter(QStringLiteral("-fmodule-file=test.mod=")).size() > 0);
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

    const QString exportModmap = buildDir + QStringLiteral("/export.modmap");
    const QString subModmap = buildDir + QStringLiteral("/test_submodule.modmap");
    const QString modModmap = buildDir + QStringLiteral("/test_module.modmap");
    const QString consumerModmap = buildDir + QStringLiteral("/consumer.modmap");

    {
        QFile file(exportModmap);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        file.write("$root .\nmodule_example export.gcm\n");
    }
    {
        QFile file(subModmap);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        file.write("$root .\ntest.submodule test_submodule.gcm\nstd std.gcm\nmodule_example export.gcm\n");
    }
    {
        QFile file(modModmap);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        file.write("$root .\ntest.mod test_module.gcm\ntest.submodule test_submodule.gcm\nmodule_example export.gcm\n");
    }
    {
        QFile file(consumerModmap);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        file.write("$root .\nmodule_example export.gcm\n");
    }

    const QString compileCommandsFile = buildDir + QStringLiteral("/compile_commands.json");
    {
        QFile file(compileCommandsFile);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        const QString json = QStringLiteral(R"([
  {
    "directory": "%1",
    "command": "g++ -std=c++23 -fPIC -I%2 -I%3 -fmodule-mapper=%4 -c %3/export.cpp",
    "file": "%3/export.cpp"
  },
  {
    "directory": "%1",
    "command": "g++ -std=c++23 -fPIC -I%2 -I%3 -fmodule-mapper=%5 -c %3/test_submodule.cpp",
    "file": "%3/test_submodule.cpp"
  },
  {
    "directory": "%1",
    "command": "g++ -std=c++23 -fPIC -I%2 -I%3 -fmodule-mapper=%6 -c %3/test_module.cpp",
    "file": "%3/test_module.cpp"
  },
  {
    "directory": "%1",
    "command": "g++ -std=c++23 -fPIC -I%2 -I%3 -fmodule-mapper=%7 -c %3/consumer.cpp",
    "file": "%3/consumer.cpp"
  }
])").arg(buildDir, incDir, srcDir, exportModmap, subModmap, modModmap, consumerModmap);
        file.write(json.toUtf8());
    }

    ClangIndex index;
    ClangParsingEnvironment environment;
    const QString consumerPath = srcDir + QStringLiteral("/consumer.cpp");
    environment.setTranslationUnitUrl(IndexedString(consumerPath));
    environment.setWorkingDirectory(Path(buildDir));

    ParserSettings settings;
    settings.parserOptions = QStringLiteral("-std=c++23");
    environment.setParserSettings(settings);

    ParseSession session(ParseSessionData::Ptr(new ParseSessionData({}, &index, environment)));
    QVERIFY(session.unit() != nullptr);

    // Verify 0 syntax / parse problems on consumer
    const auto problems = session.problemsForFile(session.mainFile());
    for (const auto& problem : problems) {
        qWarning() << "Problem:" << problem->description() << problem->finalLocation();
    }
    QCOMPARE(problems.size(), 0);
}
