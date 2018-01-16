/*
 * This file is part of KDevelop
 *
 * Copyright 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "test_compilerprovider.h"

#include <QTest>
#include <QTemporaryFile>
#include <QSignalBlocker>

#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <project/projectmodel.h>

#include <serialization/indexedstring.h>

#include <algorithm>

#include "../compilerprovider.h"
#include "../settingsmanager.h"
#include "../tests/projectsgenerator.h"

using namespace KDevelop;

namespace
{
void testCompilerEntry(SettingsManager* settings, KConfig* config){
    auto entries = settings->readPaths(config);
    auto entry = entries.first();
    auto compilers = settings->provider()->compilers();
    Q_ASSERT(!compilers.isEmpty());
    bool gccCompilerInstalled = std::any_of(compilers.begin(), compilers.end(), [](const CompilerPointer& compiler){return compiler->name().contains(QLatin1String("gcc"), Qt::CaseInsensitive);});
    if (gccCompilerInstalled) {
        QCOMPARE(entry.compiler->name(), QStringLiteral("GCC"));
    }
}

void testAddingEntry(SettingsManager* settings, KConfig* config){
    auto entries = settings->readPaths(config);
    auto entry = entries.first();
    auto compilers = settings->provider()->compilers();
    ConfigEntry otherEntry;
    otherEntry.defines[QStringLiteral("TEST")] = QStringLiteral("lalal");
    otherEntry.includes = QStringList() << QStringLiteral("/foo");
    otherEntry.path = QStringLiteral("test");
    otherEntry.compiler = compilers.first();
    entries << otherEntry;
    settings->writePaths(config, entries);

    auto readWriteEntries = settings->readPaths(config);
    QCOMPARE(readWriteEntries.size(), 2);
    QCOMPARE(readWriteEntries.at(0).path, entry.path);
    QCOMPARE(readWriteEntries.at(0).defines, entry.defines);
    QCOMPARE(readWriteEntries.at(0).includes, entry.includes);
    QCOMPARE(readWriteEntries.at(0).compiler->name(), entry.compiler->name());

    QCOMPARE(readWriteEntries.at(1).path, otherEntry.path);
    QCOMPARE(readWriteEntries.at(1).defines, otherEntry.defines);
    QCOMPARE(readWriteEntries.at(1).includes, otherEntry.includes);
    QCOMPARE(readWriteEntries.at(1).compiler->name(), otherEntry.compiler->name());
}
}

void TestCompilerProvider::initTestCase()
{
    AutoTestShell::init({QStringLiteral("kdevdefinesandincludesmanager"), QStringLiteral("KDevCustomBuildSystem"), QStringLiteral("KDevStandardOutputView")});
    TestCore::initialize();
}

void TestCompilerProvider::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestCompilerProvider::testRegisterCompiler()
{
    auto settings = SettingsManager::globalInstance();
    auto provider = settings->provider();
    auto cf = provider->compilerFactories();
    for (int i = 0 ; i < cf.size(); ++i) {
        auto compiler = cf[i]->createCompiler(QString::number(i), QString::number(i));
        QVERIFY(provider->registerCompiler(compiler));
        QVERIFY(!provider->registerCompiler(compiler));
        QVERIFY(provider->compilers().contains(compiler));
    }
    QVERIFY(!provider->registerCompiler({}));
}

void TestCompilerProvider::testCompilerIncludesAndDefines()
{
    auto settings = SettingsManager::globalInstance();
    auto provider = settings->provider();
    for (auto c : provider->compilers()) {
        if (!c->editable() && !c->path().isEmpty()) {
            QVERIFY(!c->defines({}).isEmpty());
            QVERIFY(!c->includes({}).isEmpty());
        }
    }

    QVERIFY(!provider->defines(nullptr).isEmpty());
    QVERIFY(!provider->includes(nullptr).isEmpty());

    auto compiler = provider->compilerForItem(nullptr);
    QVERIFY(compiler);
    QVERIFY(!compiler->defines(QStringLiteral("--std=c++11")).isEmpty());
    QVERIFY(!compiler->includes(QStringLiteral("--std=c++11")).isEmpty());
}

void TestCompilerProvider::testStorageBackwardsCompatible()
{
    auto settings = SettingsManager::globalInstance();
    QTemporaryFile file;
    QVERIFY(file.open());
    QTextStream stream(&file);
    stream << "[Buildset]\n" <<
      "BuildItems=@Variant(\\x00\\x00\\x00\\t\\x00\\x00\\x00\\x00\\x01\\x00\\x00\\x00\\x0b\\x00\\x00\\x00\\x00\\x01\\x00\\x00\\x00\\x1a\\x00S\\x00i\\x00m\\x00p\\x00l\\x00e\\x00P\\x00r\\x00o\\x00j\\x00e\\x00c\\x00t)\n" <<
      "[CustomBuildSystem]\n" << "CurrentConfiguration=BuildConfig0\n" <<
      "[CustomDefinesAndIncludes][ProjectPath0]\n" <<
      "Defines=\\x00\\x00\\x00\\x02\\x00\\x00\\x00\\x0c\\x00_\\x00D\\x00E\\x00B\\x00U\\x00G\\x00\\x00\\x00\\n\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x10\\x00V\\x00A\\x00R\\x00I\\x00A\\x00B\\x00L\\x00E\\x00\\x00\\x00\\n\\x00\\x00\\x00\\x00\\n\\x00V\\x00A\\x00L\\x00U\\x00E\n" <<
      "Includes=\\x00\\x00\\x00\\x01\\x00\\x00\\x00$\\x00/\\x00u\\x00s\\x00r\\x00/\\x00i\\x00n\\x00c\\x00l\\x00u\\x00d\\x00e\\x00/\\x00m\\x00y\\x00d\\x00i\\x00r\n" <<
      "Path=/\n" <<
      "[CustomDefinesAndIncludes][ProjectPath0][Compiler]\nName=GCC\nPath=gcc\nType=GCC\n";
    file.close();
    KConfig config(file.fileName());
    auto entries = settings->readPaths(&config);
    QCOMPARE(entries.size(), 1);
    auto entry = entries.first();
    Defines defines;
    defines[QStringLiteral("VARIABLE")] = QStringLiteral("VALUE");
    defines[QStringLiteral("_DEBUG")] = QString();
    QCOMPARE(entry.defines, defines);
    QStringList includes = QStringList() << QStringLiteral("/usr/include/mydir");
    QCOMPARE(entry.includes, includes);
    QCOMPARE(entry.path, QString("/"));
    QVERIFY(entry.compiler);

    testCompilerEntry(settings, &config);
    testAddingEntry(settings, &config);
}

void TestCompilerProvider::testStorageNewSystem()
{
    auto settings = SettingsManager::globalInstance();
    QTemporaryFile file;
    QVERIFY(file.open());
    QTextStream stream(&file);
    stream << "[Buildset]\n" <<
      "BuildItems=@Variant(\\x00\\x00\\x00\\t\\x00\\x00\\x00\\x00\\x01\\x00\\x00\\x00\\x0b\\x00\\x00\\x00\\x00\\x01\\x00\\x00\\x00\\x1a\\x00S\\x00i\\x00m\\x00p\\x00l\\x00e\\x00P\\x00r\\x00o\\x00j\\x00e\\x00c\\x00t)\n\n" <<
      "[CustomBuildSystem]\n" << "CurrentConfiguration=BuildConfig0\n\n" <<
      "[CustomDefinesAndIncludes][ProjectPath0]\n" << "Path=/\n\n" <<
      "[CustomDefinesAndIncludes][ProjectPath0][Defines]\n" <<
      "_DEBUG=\n" <<
      "VARIABLE=VALUE\n" <<
      "[CustomDefinesAndIncludes][ProjectPath0][Includes]\n" <<
      "1=/usr/include/mydir\n" <<
      "2=/usr/local/include/mydir\n" <<
      "[CustomDefinesAndIncludes][ProjectPath0][Compiler]\nName=GCC\nPath=gcc\nType=GCC\n";
    file.close();
    KConfig config(file.fileName());
    auto entries = settings->readPaths(&config);
    QCOMPARE(entries.size(), 1);
    auto entry = entries.first();
    QCOMPARE(entry.path, QString("/"));
    Defines defines;
    defines[QStringLiteral("VARIABLE")] = QStringLiteral("VALUE");
    defines[QStringLiteral("_DEBUG")] = QString();
    QCOMPARE(entry.defines, defines);
    QMap<QString, QString> includeMap;
    includeMap[QStringLiteral("1")] = QStringLiteral("/usr/include/mydir");
    includeMap[QStringLiteral("2")] = QStringLiteral("/usr/local/include/mydir");

    int i = 0;
    for(auto it = includeMap.begin(); it != includeMap.end(); it++)
    {
        QCOMPARE(entry.includes.at(i++), it.value());
    }

    testCompilerEntry(settings, &config);
    testAddingEntry(settings, &config);
}

void TestCompilerProvider::testCompilerIncludesAndDefinesForProject()
{
    auto project = ProjectsGenerator::GenerateMultiPathProject();
    Q_ASSERT(project);

    auto settings = SettingsManager::globalInstance();
    auto provider = settings->provider();

    Q_ASSERT(!provider->compilerFactories().isEmpty());
    auto compiler = provider->compilerFactories().first()->createCompiler(QStringLiteral("name"), QStringLiteral("path"));

    QVERIFY(provider->registerCompiler(compiler));
    QVERIFY(provider->compilers().contains(compiler));

    auto projectCompiler = provider->compilerForItem(project->projectItem());

    QVERIFY(projectCompiler);
    QVERIFY(projectCompiler != compiler);

    ProjectBaseItem* mainfile = nullptr;
    for (const auto& file: project->fileSet() ) {
        for (auto i: project->filesForPath(file)) {
            if( i->text() == QLatin1String("main.cpp") ) {
                mainfile = i;
                break;
            }
        }
    }
    QVERIFY(mainfile);
    auto mainCompiler = provider->compilerForItem(mainfile);
    QVERIFY(mainCompiler);
    QVERIFY(mainCompiler->name() == projectCompiler->name());

    ConfigEntry entry;
    entry.path = QStringLiteral("src/main.cpp");
    entry.compiler = compiler;

    auto entries = settings->readPaths(project->projectConfiguration().data());

    entries.append(entry);
    settings->writePaths(project->projectConfiguration().data(), entries);

    QVERIFY(provider->compilers().contains(compiler));

    mainCompiler = provider->compilerForItem(mainfile);
    QVERIFY(mainCompiler);
    QVERIFY(mainCompiler->name() == compiler->name());

    ICore::self()->projectController()->closeProject(project);
}

QTEST_MAIN(TestCompilerProvider)
