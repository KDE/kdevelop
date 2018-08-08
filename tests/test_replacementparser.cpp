/*
 * This file is part of KDevelop
 *
 * Copyright 2016 Carlos Nihelton <carlosnsoliveira@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "test_replacementparser.h"

// SUT
#include "parsers/replacementparser.h"
// KDevPlatform
#include <tests/autotestshell.h>
#include <tests/testcore.h>
// Qt
#include <QtTest>

using namespace KDevelop;
using namespace ClangTidy;

void ReplacementsParserTester::initTestCase()
{
    // Called before the first testfunction is executed
    AutoTestShell::init({ "kdevclangtidy" });
    TestCore::initialize(Core::NoUi);
}

void ReplacementsParserTester::cleanupTestCase()
{
    // Called after the last testfunction was executed
    TestCore::shutdown();
}

void ReplacementsParserTester::doTest()
{
    ReplacementParser parser("data/plugin.cpp.yaml", "data/plugin.cpp");
    parser.parse();
    auto v = parser.allReplacements();

    QVERIFY(!v.isEmpty());
    QCOMPARE(v.length(), static_cast<int>(parser.count()));

    QCOMPARE(v[0].range.document.str(), QStringLiteral("data/plugin.cpp"));
    QCOMPARE(v[0].offset, size_t(6263));
    QCOMPARE(v[0].length, size_t(1));
    QCOMPARE(v[0].replacementText, QString());
    QCOMPARE(v[0].range.start().line() + 1, 155); // as would appear in editor.
    QCOMPARE(v[0].range.start().column() + 1, 9); // as would appear in editor.

    QCOMPARE(v[1].range.document.str(), QStringLiteral("data/plugin.cpp"));
    QCOMPARE(v[1].offset, size_t(6267));
    QCOMPARE(v[1].length, size_t(0));
    QCOMPARE(v[1].replacementText, QString(" == nullptr"));
    QCOMPARE(v[1].range.start().line() + 1, 155);
    QCOMPARE(v[1].range.start().column() + 1, 13);

    QCOMPARE(v[2].range.document.str(), QStringLiteral("data/plugin.cpp"));
    QCOMPARE(v[2].offset, size_t(6561));
    QCOMPARE(v[2].length, size_t(1));
    QCOMPARE(v[2].replacementText, QString());
    QCOMPARE(v[2].range.start().line() + 1, 162);
    QCOMPARE(v[2].range.start().column() + 1, 9);

    QCOMPARE(v[3].range.document.str(), QStringLiteral("data/plugin.cpp"));
    QCOMPARE(v[3].offset, size_t(6569));
    QCOMPARE(v[3].length, size_t(0));
    QCOMPARE(v[3].replacementText, QStringLiteral(" == nullptr"));
    QCOMPARE(v[3].range.start().line() + 1, 162);
    QCOMPARE(v[3].range.start().column() + 1, 17);

    QCOMPARE(v[4].range.document.str(), QStringLiteral("data/plugin.cpp"));
    QCOMPARE(v[4].offset, size_t(7233));
    QCOMPARE(v[4].length, size_t(69));
    QCOMPARE(v[4].replacementText, QStringLiteral("// TODO(cnihelton): auto detect clang-tidy executable"
                                                  " instead of hard-coding it."));
    QCOMPARE(v[4].range.start().line() + 1, 181);
    QCOMPARE(v[4].range.start().column() + 1, 5);

    QCOMPARE(v[5].range.document.str(), QStringLiteral("data/plugin.cpp"));
    QCOMPARE(v[5].offset, size_t(8800));
    QCOMPARE(v[5].length, size_t(4));
    QCOMPARE(v[5].replacementText, QStringLiteral("auto "));
    QCOMPARE(v[5].range.start().line() + 1, 210);
    QCOMPARE(v[5].range.start().column() + 1, 9);

    QCOMPARE(v[6].range.document.str(), QStringLiteral("data/plugin.cpp"));
    QCOMPARE(v[6].offset, size_t(8945));
    QCOMPARE(v[6].length, size_t(4));
    QCOMPARE(v[6].replacementText, QStringLiteral("auto "));
    QCOMPARE(v[6].range.start().line() + 1, 214);
    QCOMPARE(v[6].range.start().column() + 1, 5);

    QCOMPARE(v[7].range.document.str(), QStringLiteral("data/plugin.cpp"));
    QCOMPARE(v[7].offset, size_t(9406));
    QCOMPARE(v[7].length, size_t(1));
    QCOMPARE(v[7].replacementText, QString());
    QCOMPARE(v[7].range.start().line() + 1, 238);
    QCOMPARE(v[7].range.start().column() + 1, 9);

    // Invalid stuff.
    // Modified the plugin.cpp.yaml file to try to cause and exception during parsing.
    // Added three lines from other file, which goes beyond the end of the plugin.cpp file in the last offsets.
    // The parser didn't throw exception event though it might have generated empty ranges for the last replacements.
    // And also invalidates the generated replacement.
    QVERIFY(!v[15].range.isValid());
    QCOMPARE(v[15].range.document.str(), QStringLiteral(""));
    QCOMPARE(v[15].offset, size_t(0));
    QCOMPARE(v[15].length, size_t(0));
    QCOMPARE(v[15].replacementText, QString());
    QCOMPARE(v[15].range.start().line() + 1, 0);
    QCOMPARE(v[15].range.start().column() + 1, 0);

    // testing multibyte chars in source code.
    ReplacementParser nihonParser("data/nihon_plugin.cpp.yaml", "data/nihon_plugin.cpp");
    nihonParser.parse();
    auto nv = nihonParser.allReplacements();
    QVERIFY(!nv.isEmpty());
    QCOMPARE(nv.length(), static_cast<int>(nihonParser.count()));

    QCOMPARE(nv[0].range.document.str(), QStringLiteral("data/nihon_plugin.cpp"));
    QCOMPARE(nv[0].offset, size_t(10165));
    QCOMPARE(nv[0].length, size_t(1));
    QCOMPARE(nv[0].replacementText, QString());
    QCOMPARE(nv[0].range.start().line() + 1, 288);
    QCOMPARE(nv[0].range.start().column() + 1, 9);

    QCOMPARE(nv[1].range.document.str(), QStringLiteral("data/nihon_plugin.cpp"));
    QCOMPARE(nv[1].offset, size_t(10169));
    QCOMPARE(nv[1].length, size_t(0));
    QCOMPARE(nv[1].replacementText, QStringLiteral(" == nullptr"));
    QCOMPARE(nv[1].range.start().line() + 1, 288);
    QCOMPARE(nv[1].range.start().column() + 1, 13);

    QCOMPARE(nv[2].range.document.str(), QStringLiteral("data/nihon_plugin.cpp"));
    QCOMPARE(nv[2].offset, size_t(10463));
    QCOMPARE(nv[2].length, size_t(1));
    QCOMPARE(nv[2].replacementText, QString());
    QCOMPARE(nv[2].range.start().line() + 1, 295);
    QCOMPARE(nv[2].range.start().column() + 1, 9);

    QCOMPARE(nv[3].range.document.str(), QStringLiteral("data/nihon_plugin.cpp"));
    QCOMPARE(nv[3].offset, size_t(10471));
    QCOMPARE(nv[3].length, size_t(0));
    QCOMPARE(nv[3].replacementText, QStringLiteral(" == nullptr"));
    QCOMPARE(nv[3].range.start().line() + 1, 295);
    QCOMPARE(nv[3].range.start().column() + 1, 17);

    QCOMPARE(nv[4].range.document.str(), QStringLiteral("data/nihon_plugin.cpp"));
    QCOMPARE(nv[4].offset, size_t(11135));
    QCOMPARE(nv[4].length, size_t(69));
    QCOMPARE(nv[4].replacementText, QStringLiteral("// TODO(cnihelton): auto detect clang-tidy executable "
                                                   "instead of hard-coding it."));
    QCOMPARE(nv[4].range.start().line() + 1, 314);
    QCOMPARE(nv[4].range.start().column() + 1, 5);
}

QTEST_GUILESS_MAIN(ReplacementsParserTester);
