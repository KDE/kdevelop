/*
    SPDX-FileCopyrightText: 2016 Peje Nilsson <peje66@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_cppcheckparser.h"

#include <QTest>
#include <tests/testcore.h>
#include <tests/autotestshell.h>

#include <language/editor/documentrange.h>
#include <shell/problem.h>

#include "parser.h"

using namespace KDevelop;
using namespace cppcheck;

void TestCppcheckParser::initTestCase()
{
    AutoTestShell::init({"kdevcppcheck"});
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

    cppcheck::CppcheckParser parser;
    parser.addData(cppcheck_example_output);

    const auto problems = parser.parse();
    QVERIFY(!problems.empty());

    IProblem::Ptr p = problems[0];
    QCOMPARE(p->description(), QStringLiteral("Memory leak: ej"));
    QCOMPARE(p->explanation(), QStringLiteral("<html>Memory leak: ej</html>"));
    QCOMPARE(p->finalLocation().document.str(), QStringLiteral("/kdesrc/kdev-cppcheck/plugin.cpp"));
    QCOMPARE(p->finalLocation().start().line()+1, 169);
    QCOMPARE(p->severity(), IProblem::Error);
    QCOMPARE(p->source(), IProblem::Plugin);

    // test problem with 2 <location> elements
    p = problems[1];
    QCOMPARE(p->description(), QStringLiteral("(performance) location_test_msg"));
    QCOMPARE(p->explanation(), QStringLiteral("<html>location_test_verbose</html>"));
    QCOMPARE(p->finalLocation().document.str(), QStringLiteral("location_test.cpp"));
    QCOMPARE(p->finalLocation().start().line()+1, 120);
    QCOMPARE(p->severity(), IProblem::Hint);
    QCOMPARE(p->source(), IProblem::Plugin);

    // test problem with '\\012' tokens in verbose message
    p = problems[2];
    QCOMPARE(p->description(), QStringLiteral("(style, inconclusive) The scope of the variable..."));
    QCOMPARE(p->explanation(), QStringLiteral("<html>...Here is an example...:<pre>void f(int x)\n{\n    int i = 0;\n}</pre><br>...</html>"));
    QCOMPARE(p->finalLocation().document.str(), QStringLiteral("html_pre_test.cpp"));
    QCOMPARE(p->finalLocation().start().line()+1, 41);
    QCOMPARE(p->severity(), IProblem::Hint);
    QCOMPARE(p->source(), IProblem::Plugin);
}

QTEST_GUILESS_MAIN(TestCppcheckParser)
