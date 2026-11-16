/*
    SPDX-FileCopyrightText: 2026 Fernando Vilas <fvilas@iname.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "test_modules.h"

#include <QTest>

#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <tests/testplugincontroller.h>

#include "../duchain/clanghelpers.h"
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
