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

#include "test_cppcheckparser.h"

#include <QtTest/QTest>
#include <tests/testcore.h>
#include <tests/autotestshell.h>

#include "kdevplatform/shell/problem.h"
#include "cppcheckparser.h"

using namespace KDevelop;
using namespace cppcheck;

void TestCppcheckParser::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
}

void TestCppcheckParser::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestCppcheckParser::testParser()
{
    const QString cppcheck_example_output = QStringLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<results version=\"2\">\n"
        "    <cppcheck version=\"1.72\"/>"
        "    <errors>"
        "        <error id=\"memleak\" severity=\"error\" msg=\"Memory leak: ej\" verbose=\"Memory leak: ej\" cwe=\"401\">"
        "            <location file=\"/kdesrc/kdev-cppcheck/plugin.cpp\" line=\"169\"/>"
        "        </error>"
        "        <error id=\"redundantAssignment\" severity=\"performance\" "
        "               msg=\"location_test_msg\" verbose=\"location_test_verbose\">"
        "            <location file=\"location_test.cpp\" line=\"120\"/>"
        "            <location file=\"location_test.cpp\" line=\"100\"/>"
        "        </error>"
        "    </errors>"
        "</results>");

    cppcheck::CppcheckParser parser;
    parser.addData(cppcheck_example_output);
    parser.parse();

    const auto problems = parser.problems();
    QVERIFY(!problems.empty());

    IProblem::Ptr p = problems[0];
    QCOMPARE(p->description(), QStringLiteral("Memory leak: ej"));
    QCOMPARE(p->explanation(), QStringLiteral("Memory leak: ej"));
    QCOMPARE(p->finalLocation().document.str(), QStringLiteral("/kdesrc/kdev-cppcheck/plugin.cpp"));
    QCOMPARE(p->finalLocation().start().line()+1, 169);
    QCOMPARE(p->severity(), IProblem::Error);
    QCOMPARE(p->source(), IProblem::Plugin);

    // test problem with 2 <location> elements
    p = problems[1];
    QCOMPARE(p->description(), QStringLiteral("location_test_msg"));
    QCOMPARE(p->explanation(), QStringLiteral("location_test_verbose"));
    QCOMPARE(p->finalLocation().document.str(), QStringLiteral("location_test.cpp"));
    QCOMPARE(p->finalLocation().start().line()+1, 120);
    QCOMPARE(p->severity(), IProblem::Hint);
    QCOMPARE(p->source(), IProblem::Plugin);
}

QTEST_GUILESS_MAIN(TestCppcheckParser);
