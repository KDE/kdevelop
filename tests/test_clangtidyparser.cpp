/*************************************************************************************
 *  Copyright 2016 (C) Peje Nilsson <peje66@gmail.com>                               *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "test_clangtidyparser.h"

#include <QtTest/QTest>
#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include "clangtidyparser.h"
#include "kdevplatform/shell/problem.h"

using namespace KDevelop;
using namespace ClangTidy;

void TestClangtidyParser::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
}

void TestClangtidyParser::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestClangtidyParser::testParser()
{
    const QString clangtidy_example_output = QStringLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<results version=\"2\">\n"
        "    <clangtidy version=\"1.72\"/>"
        "    <errors>"
        "        <error id=\"memleak\" severity=\"error\" msg=\"Memory leak: ej\" verbose=\"Memory leak: ej\" "
        "cwe=\"401\">"
        "            <location file=\"/kdesrc/kdev-clangtidy/plugin.cpp\" line=\"169\"/>"
        "        </error>"
        "        <error id=\"redundantAssignment\" severity=\"performance\""
        "               msg=\"location_test_msg\" verbose=\"location_test_verbose\">"
        "            <location file=\"location_test.cpp\" line=\"120\"/>"
        "            <location file=\"location_test.cpp\" line=\"100\"/>"
        "        </error>"
        "        <error id=\"variableScope\" severity=\"style\" inconclusive=\"true\""
        "               msg=\"The scope of the variable...\""
        "               verbose=\"...Here is an example...:\\012void f(int x)\\012{\\012    int i = 0;\\012}\\012...\">"
        "            <location file=\"html_pre_test.cpp\" line=\"41\"/>"
        "        </error>"
        "    </errors>"
        "</results>");

    ClangTidy::ClangtidyParser parser;
    parser.addData(clangtidy_example_output);
    parser.parse();

    const auto problems = parser.problems();
    QVERIFY(!problems.empty());

    IProblem::Ptr p = problems[0];
    QCOMPARE(p->description(), QStringLiteral("Memory leak: ej"));
    QCOMPARE(p->explanation(), QStringLiteral("<html>Memory leak: ej</html>"));
    QCOMPARE(p->finalLocation().document.str(), QStringLiteral("/kdesrc/kdev-clangtidy/plugin.cpp"));
    QCOMPARE(p->finalLocation().start().line() + 1, 169);
    QCOMPARE(p->severity(), IProblem::Error);
    QCOMPARE(p->source(), IProblem::Plugin);

    // test problem with 2 <location> elements
    p = problems[1];
    QCOMPARE(p->description(), QStringLiteral("(performance) location_test_msg"));
    QCOMPARE(p->explanation(), QStringLiteral("<html>location_test_verbose</html>"));
    QCOMPARE(p->finalLocation().document.str(), QStringLiteral("location_test.cpp"));
    QCOMPARE(p->finalLocation().start().line() + 1, 120);
    QCOMPARE(p->severity(), IProblem::Hint);
    QCOMPARE(p->source(), IProblem::Plugin);

    // test problem with '\\012' tokens in verbose message
    p = problems[2];
    QCOMPARE(p->description(), QStringLiteral("(style, inconclusive) The scope of the variable..."));
    QCOMPARE(
        p->explanation(),
        QStringLiteral("<html>...Here is an example...:<pre>void f(int x)\n{\n    int i = 0;\n}</pre><br>...</html>"));
    QCOMPARE(p->finalLocation().document.str(), QStringLiteral("html_pre_test.cpp"));
    QCOMPARE(p->finalLocation().start().line() + 1, 41);
    QCOMPARE(p->severity(), IProblem::Hint);
    QCOMPARE(p->source(), IProblem::Plugin);
}

QTEST_GUILESS_MAIN(TestClangtidyParser);
