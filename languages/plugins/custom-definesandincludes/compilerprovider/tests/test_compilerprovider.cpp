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

#include <QtTest/QtTest>

#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include "../compilerprovider.h"
#include "../settingsmanager.h"

using namespace KDevelop;

void TestCompilerProvider::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
}

void TestCompilerProvider::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestCompilerProvider::testRegisterCompiler()
{
    SettingsManager settings;
    auto provider = settings.provider();
    auto cf = provider->compilerFactories();
    for (int i = 0 ; i < cf.size(); ++i) {
        auto compiler = cf[i]->createCompiler(QString::number(i), QString::number(i));
        QVERIFY(provider->registerCompiler(compiler));
        QVERIFY(!provider->registerCompiler(compiler));
        QVERIFY(provider->compilers().contains(compiler));
    }
    QVERIFY(!provider->registerCompiler({}));
}

void TestCompilerProvider::testSetCompiler()
{
    SettingsManager settings;
    auto provider = settings.provider();
    provider->setCompiler(nullptr, {});
    QVERIFY(provider->currentCompiler(nullptr));

    for (auto c : provider->compilers()) {
        provider->setCompiler(nullptr, c);
        QCOMPARE(provider->currentCompiler(nullptr), c);
    }
}

void TestCompilerProvider::testCompilerIncludesAndDefines()
{
    SettingsManager settings;
    auto provider = settings.provider();
    for (auto c : provider->compilers()) {
        if (!c->editable() && !c->path().isEmpty()) {
            provider->setCompiler(nullptr, c);
            QVERIFY(!c->defines().isEmpty());
            QVERIFY(!c->includes().isEmpty());
            QCOMPARE(provider->defines(nullptr), c->defines());
            QCOMPARE(provider->includes(nullptr), c->includes());
        }
    }
}

void TestCompilerProvider::testStorageBackwardsCompatible()
{
    SettingsManager settings;
    QTemporaryFile file;
    QVERIFY(file.open());
    QTextStream stream(&file);
    stream << "[Buildset]\n" <<
      "BuildItems=@Variant(\\x00\\x00\\x00\\t\\x00\\x00\\x00\\x00\\x01\\x00\\x00\\x00\\x0b\\x00\\x00\\x00\\x00\\x01\\x00\\x00\\x00\\x1a\\x00S\\x00i\\x00m\\x00p\\x00l\\x00e\\x00P\\x00r\\x00o\\x00j\\x00e\\x00c\\x00t)\n" <<
      "[CustomBuildSystem]\n" << "CurrentConfiguration=BuildConfig0\n" <<
      "[CustomDefinesAndIncludes][ProjectPath0]\n" <<
      "Defines=\\x00\\x00\\x00\\x02\\x00\\x00\\x00\\x0c\\x00_\\x00D\\x00E\\x00B\\x00U\\x00G\\x00\\x00\\x00\\n\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x10\\x00V\\x00A\\x00R\\x00I\\x00A\\x00B\\x00L\\x00E\\x00\\x00\\x00\\n\\x00\\x00\\x00\\x00\\n\\x00V\\x00A\\x00L\\x00U\\x00E\n" <<
      "Includes=\\x00\\x00\\x00\\x01\\x00\\x00\\x00$\\x00/\\x00u\\x00s\\x00r\\x00/\\x00i\\x00n\\x00c\\x00l\\x00u\\x00d\\x00e\\x00/\\x00m\\x00y\\x00d\\x00i\\x00r\n" <<
      "Path=/\n";
    file.close();
    KConfig config(file.fileName());
    auto entries = settings.readPaths(&config);
    QCOMPARE(entries.size(), 1);
    auto entry = entries.first();
    Defines defines;
    defines["VARIABLE"] = "VALUE";
    defines["_DEBUG"] = QString();
    QCOMPARE(entry.defines, defines);
    QStringList includes = QStringList() << "/usr/include/mydir";
    QCOMPARE(entry.includes, includes);
    QCOMPARE(entry.path, QString("/"));

    ConfigEntry otherEntry;
    otherEntry.defines["TEST"] = "lalal";
    otherEntry.includes = QStringList() << "/foo";
    otherEntry.path = "test";
    entries << otherEntry;
    settings.writePaths(&config, entries);

    auto readWriteEntries = settings.readPaths(&config);
    QCOMPARE(readWriteEntries.size(), 2);
    QCOMPARE(readWriteEntries.at(0).path, entry.path);
    QCOMPARE(readWriteEntries.at(0).defines, entry.defines);
    QCOMPARE(readWriteEntries.at(0).includes, entry.includes);

    QCOMPARE(readWriteEntries.at(1).path, otherEntry.path);
    QCOMPARE(readWriteEntries.at(1).defines, otherEntry.defines);
    QCOMPARE(readWriteEntries.at(1).includes, otherEntry.includes);
}

QTEST_GUILESS_MAIN(TestCompilerProvider)
