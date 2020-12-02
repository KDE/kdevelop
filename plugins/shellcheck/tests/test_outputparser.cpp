/*
 * This file is part of KDevelop
 * Copyright 2020  Morten Danielsen Volden <mvolden2@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "test_outputparser.h"

#include <QTest>
#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <language/editor/documentrange.h>
#include <shell/problem.h>

#include "outputparser.h"

QTEST_GUILESS_MAIN(TestOutputParser)

void TestOutputParser::initTestCase()
{
    KDevelop::AutoTestShell::init({QStringLiteral("kdevshellcheck")});
    KDevelop::TestCore::initialize(KDevelop::Core::NoUi);
}

void TestOutputParser::cleanupTestCase()
{
    KDevelop::TestCore::shutdown();
}

void TestOutputParser::init()
{
    // Called before each testfunction is executed
}

void TestOutputParser::cleanup()
{
    // Called after every testfunction
}

void TestOutputParser::testParserEmptyShellOutput()
{
    shellcheck::OutputParser testee;

    testee.parse();

    const auto problems = testee.parse();
    QVERIFY(problems.empty());

}

void TestOutputParser::testParser()
{
    shellcheck::OutputParser testee;

    QStringList lines;
    QStringList linesSecond;
    QStringList linesThird;

    lines.append(QStringLiteral("{"));
    lines.append(QStringLiteral("    \"comments\": ["));
    lines.append(QStringLiteral("    {"));
    lines.append(QStringLiteral("    \"file\": \"/home/mvo/kde/src/kdevelop/plugins/shellcheck/Messages.sh\","));
    lines.append(QStringLiteral("    \"line\": 2,"));
    lines.append(QStringLiteral("    \"endLine\": 2,"));
    lines.append(QStringLiteral("    \"column\": 12,"));
    lines.append(QStringLiteral("    \"endColumn\": 32,"));
    lines.append(QStringLiteral("    \"level\": \"warning\","));
    lines.append(QStringLiteral("    \"code\": 2046,"));
    lines.append(QStringLiteral("    \"message\": \"Quote this to prevent word splitting.\","));
    lines.append(QStringLiteral("    \"fix\": null"));
    lines.append(QStringLiteral("    },"));
    lines.append(QStringLiteral("    {"));
    lines.append(QStringLiteral("    \"file\": \"/home/mvo/kde/src/kdevelop/plugins/shellcheck/Messages.sh\","));
    lines.append(QStringLiteral("    \"line\": 2,"));
    lines.append(QStringLiteral("    \"endLine\": 2,"));
    lines.append(QStringLiteral("    \"column\": 12,"));
    lines.append(QStringLiteral("    \"endColumn\": 32,"));
    lines.append(QStringLiteral("    \"level\": \"style\","));
    lines.append(QStringLiteral("    \"code\": 2006,"));
    lines.append(QStringLiteral("    \"message\": \"Use $(...) notation instead of legacy backticked `...`.\","));
    lines.append(QStringLiteral("    \"fix\": {"));
    lines.append(QStringLiteral("        \"replacements\": ["));
    lines.append(QStringLiteral("        {"));
    lines.append(QStringLiteral("            \"line\": 2,"));
    lines.append(QStringLiteral("            \"endLine\": 2,"));
    lines.append(QStringLiteral("            \"precedence\": 7,"));
    lines.append(QStringLiteral("            \"insertionPoint\": \"afterEnd\","));
    lines.append(QStringLiteral("            \"column\": 12,"));
    lines.append(QStringLiteral("            \"replacement\": \"$(\","));
    lines.append(QStringLiteral("            \"endColumn\": 13"));
    lines.append(QStringLiteral("        },"));
    lines.append(QStringLiteral("        {"));
    lines.append(QStringLiteral("            \"line\": 2,"));
    lines.append(QStringLiteral("            \"endLine\": 2,"));
    lines.append(QStringLiteral("            \"precedence\": 7,"));
    lines.append(QStringLiteral("            \"insertionPoint\": \"beforeStart\","));
    lines.append(QStringLiteral("            \"column\": 31,"));
    lines.append(QStringLiteral("            \"replacement\": \")\","));
    lines.append(QStringLiteral("            \"endColumn\": 32"));
    lines.append(QStringLiteral("        }"));
    lines.append(QStringLiteral("        ]"));
    lines.append(QStringLiteral("    }"));
    lines.append(QStringLiteral("    },"));
    lines.append(QStringLiteral("    {"));
    lines.append(QStringLiteral("    \"file\": \"/home/mvo/kde/src/kdevelop/plugins/shellcheck/Messages.sh\","));
    lines.append(QStringLiteral("    \"line\": 2,"));
    lines.append(QStringLiteral("    \"endLine\": 2,"));
    lines.append(QStringLiteral("    \"column\": 33,"));
    lines.append(QStringLiteral("    \"endColumn\": 53,"));
    lines.append(QStringLiteral("    \"level\": \"warning\","));
    lines.append(QStringLiteral("    \"code\": 2046,"));
    lines.append(QStringLiteral("    \"message\": \"Quote this to prevent word splitting.\","));
    lines.append(QStringLiteral("    \"fix\": null"));
    lines.append(QStringLiteral("    },"));
    lines.append(QStringLiteral("    {"));
    lines.append(QStringLiteral("    \"file\": \"/home/mvo/kde/src/kdevelop/plugins/shellcheck/Messages.sh\","));
    lines.append(QStringLiteral("    \"line\": 2,"));
    lines.append(QStringLiteral("    \"endLine\": 2,"));
    lines.append(QStringLiteral("    \"column\": 33,"));
    lines.append(QStringLiteral("    \"endColumn\": 53,"));
    lines.append(QStringLiteral("    \"level\": \"style\","));
    lines.append(QStringLiteral("    \"code\": 2006,"));
    lines.append(QStringLiteral("    \"message\": \"Use $(...) notation instead of legacy backticked `...`.\","));
    lines.append(QStringLiteral("    \"fix\": {"));
    lines.append(QStringLiteral("        \"replacements\": ["));
    lines.append(QStringLiteral("        {"));
    lines.append(QStringLiteral("            \"line\": 2,"));
    lines.append(QStringLiteral("            \"endLine\": 2,"));
    lines.append(QStringLiteral("            \"precedence\": 7,"));
    lines.append(QStringLiteral("            \"insertionPoint\": \"afterEnd\","));
    lines.append(QStringLiteral("            \"column\": 33,"));
    lines.append(QStringLiteral("            \"replacement\": \"$(\","));
    lines.append(QStringLiteral("            \"endColumn\": 34"));
    lines.append(QStringLiteral("        },"));
    lines.append(QStringLiteral("        {"));
    lines.append(QStringLiteral("            \"line\": 2,"));
    lines.append(QStringLiteral("            \"endLine\": 2,"));
    lines.append(QStringLiteral("            \"precedence\": 7,"));
    lines.append(QStringLiteral("            \"insertionPoint\": \"beforeStart\","));
    lines.append(QStringLiteral("            \"column\": 52,"));
    lines.append(QStringLiteral("            \"replacement\": \")\","));
    lines.append(QStringLiteral("            \"endColumn\": 53"));
    lines.append(QStringLiteral("        }"));
    lines.append(QStringLiteral("        ]"));
    lines.append(QStringLiteral("    }"));
    lines.append(QStringLiteral("    },"));

    linesSecond.append(QStringLiteral("    {"));

    linesThird.append(QStringLiteral("    \"file\": \"/home/mvo/kde/src/kdevelop/plugins/shellcheck/Messages.sh\","));
    linesThird.append(QStringLiteral("    \"line\": 3,"));
    linesThird.append(QStringLiteral("    \"endLine\": 3,"));
    linesThird.append(QStringLiteral("    \"column\": 11,"));
    linesThird.append(QStringLiteral("    \"endColumn\": 61,"));
    linesThird.append(QStringLiteral("    \"level\": \"warning\","));
    linesThird.append(QStringLiteral("    \"code\": 2046,"));
    linesThird.append(QStringLiteral("    \"message\": \"Quote this to prevent word splitting.\","));
    linesThird.append(QStringLiteral("    \"fix\": null"));
    linesThird.append(QStringLiteral("    },"));
    linesThird.append(QStringLiteral("    {"));
    linesThird.append(QStringLiteral("    \"file\": \"/home/mvo/kde/src/kdevelop/plugins/shellcheck/Messages.sh\","));
    linesThird.append(QStringLiteral("    \"line\": 3,"));
    linesThird.append(QStringLiteral("    \"endLine\": 3,"));
    linesThird.append(QStringLiteral("    \"column\": 11,"));
    linesThird.append(QStringLiteral("    \"endColumn\": 61,"));
    linesThird.append(QStringLiteral("    \"level\": \"style\","));
    linesThird.append(QStringLiteral("    \"code\": 2006,"));
    linesThird.append(QStringLiteral("    \"message\": \"Use $(...) notation instead of legacy backticked `...`.\","));
    linesThird.append(QStringLiteral("    \"fix\": {"));
    linesThird.append(QStringLiteral("        \"replacements\": ["));
    linesThird.append(QStringLiteral("        {"));
    linesThird.append(QStringLiteral("            \"line\": 3,"));
    linesThird.append(QStringLiteral("            \"endLine\": 3,"));
    linesThird.append(QStringLiteral("            \"precedence\": 7,"));
    linesThird.append(QStringLiteral("            \"insertionPoint\": \"afterEnd\","));
    linesThird.append(QStringLiteral("            \"column\": 11,"));
    linesThird.append(QStringLiteral("            \"replacement\": \"$(\","));
    linesThird.append(QStringLiteral("            \"endColumn\": 12"));
    linesThird.append(QStringLiteral("        },"));
    linesThird.append(QStringLiteral("        {"));
    linesThird.append(QStringLiteral("            \"line\": 3,"));
    linesThird.append(QStringLiteral("            \"endLine\": 3,"));
    linesThird.append(QStringLiteral("            \"precedence\": 7,"));
    linesThird.append(QStringLiteral("            \"insertionPoint\": \"beforeStart\","));
    linesThird.append(QStringLiteral("            \"column\": 60,"));
    linesThird.append(QStringLiteral("            \"replacement\": \")\","));
    linesThird.append(QStringLiteral("            \"endColumn\": 61"));
    linesThird.append(QStringLiteral("        }"));
    linesThird.append(QStringLiteral("        ]"));
    linesThird.append(QStringLiteral("    }"));
    linesThird.append(QStringLiteral("    },"));
    linesThird.append(QStringLiteral("    {"));
    linesThird.append(QStringLiteral("    \"file\": \"/home/mvo/kde/src/kdevelop/plugins/shellcheck/Messages.sh\","));
    linesThird.append(QStringLiteral("    \"line\": 3,"));
    linesThird.append(QStringLiteral("    \"endLine\": 3,"));
    linesThird.append(QStringLiteral("    \"column\": 65,"));
    linesThird.append(QStringLiteral("    \"endColumn\": 71,"));
    linesThird.append(QStringLiteral("    \"level\": \"warning\","));
    linesThird.append(QStringLiteral("    \"code\": 2154,"));
    linesThird.append(QStringLiteral("    \"message\": \"podir is referenced but not assigned.\","));
    linesThird.append(QStringLiteral("    \"fix\": null"));
    linesThird.append(QStringLiteral("    },"));
    linesThird.append(QStringLiteral("    {"));
    linesThird.append(QStringLiteral("    \"file\": \"/home/mvo/kde/src/kdevelop/plugins/shellcheck/Messages.sh\","));
    linesThird.append(QStringLiteral("    \"line\": 3,"));
    linesThird.append(QStringLiteral("    \"endLine\": 3,"));
    linesThird.append(QStringLiteral("    \"column\": 65,"));
    linesThird.append(QStringLiteral("    \"endColumn\": 71,"));
    linesThird.append(QStringLiteral("    \"level\": \"info\","));
    linesThird.append(QStringLiteral("    \"code\": 2086,"));
    linesThird.append(QStringLiteral("    \"message\": \"Double quote to prevent globbing and word splitting.\","));
    linesThird.append(QStringLiteral("    \"fix\": {"));
    linesThird.append(QStringLiteral("        \"replacements\": ["));
    linesThird.append(QStringLiteral("        {"));
    linesThird.append(QStringLiteral("            \"line\": 3,"));
    linesThird.append(QStringLiteral("            \"endLine\": 3,"));
    linesThird.append(QStringLiteral("            \"precedence\": 7,"));
    linesThird.append(QStringLiteral("            \"insertionPoint\": \"afterEnd\","));
    linesThird.append(QStringLiteral("            \"column\": 65,"));
    linesThird.append(QStringLiteral("            \"replacement\": \"\\\"\","));
    linesThird.append(QStringLiteral("            \"endColumn\": 65"));
    linesThird.append(QStringLiteral("        },"));
    linesThird.append(QStringLiteral("        {"));
    linesThird.append(QStringLiteral("            \"line\": 3,"));
    linesThird.append(QStringLiteral("            \"endLine\": 3,"));
    linesThird.append(QStringLiteral("            \"precedence\": 7,"));
    linesThird.append(QStringLiteral("            \"insertionPoint\": \"beforeStart\","));
    linesThird.append(QStringLiteral("            \"column\": 71,"));
    linesThird.append(QStringLiteral("            \"replacement\": \"\\\"\","));
    linesThird.append(QStringLiteral("            \"endColumn\": 71"));
    linesThird.append(QStringLiteral("        }"));
    linesThird.append(QStringLiteral("        ]"));
    linesThird.append(QStringLiteral("    }"));
    linesThird.append(QStringLiteral("    }"));
    linesThird.append(QStringLiteral("]"));
    linesThird.append(QStringLiteral("}"));
    
    testee.add(lines);
    testee.add(linesSecond);
    testee.add(linesThird);
    
    const auto problems = testee.parse();
    QVERIFY(!problems.empty());
    QCOMPARE(problems.size(), 8);

    // Verify the first two and the last problems
    KDevelop::IProblem::Ptr p = problems[0];
    QCOMPARE(p->description(), QStringLiteral("Quote this to prevent word splitting."));
    QCOMPARE(p->explanation(), QStringLiteral(""));
    QCOMPARE(p->finalLocation().document.str(), QStringLiteral("/home/mvo/kde/src/kdevelop/plugins/shellcheck/Messages.sh"));
    QCOMPARE(p->finalLocation().start().line(), 2);
    QCOMPARE(p->finalLocation().start().column(), 12);
    QCOMPARE(p->finalLocation().end().line(), 2);
    QCOMPARE(p->finalLocation().end().column(), 32);
    QCOMPARE(p->severity(), KDevelop::IProblem::Warning);
    QCOMPARE(p->source(), KDevelop::IProblem::Plugin);

    p = problems[1];
    QCOMPARE(p->description(), QStringLiteral("Use $(...) notation instead of legacy backticked `...`."));
    QCOMPARE(p->explanation(), QStringLiteral(""));
    QCOMPARE(p->finalLocation().document.str(), QStringLiteral("/home/mvo/kde/src/kdevelop/plugins/shellcheck/Messages.sh"));
    QCOMPARE(p->finalLocation().start().line(), 2);
    QCOMPARE(p->finalLocation().start().column(), 12);
    QCOMPARE(p->finalLocation().end().line(), 2);
    QCOMPARE(p->finalLocation().end().column(), 32);
    QCOMPARE(p->severity(), KDevelop::IProblem::NoSeverity);
    QCOMPARE(p->source(), KDevelop::IProblem::Plugin);

    p = problems[7];
    QCOMPARE(p->description(), QStringLiteral("Double quote to prevent globbing and word splitting."));
    QCOMPARE(p->explanation(), QStringLiteral(""));
    QCOMPARE(p->finalLocation().document.str(), QStringLiteral("/home/mvo/kde/src/kdevelop/plugins/shellcheck/Messages.sh"));
    QCOMPARE(p->finalLocation().start().line(), 3);
    QCOMPARE(p->finalLocation().start().column(), 65);
    QCOMPARE(p->finalLocation().end().line(), 3);
    QCOMPARE(p->finalLocation().end().column(), 71);
    QCOMPARE(p->severity(), KDevelop::IProblem::Hint);
    QCOMPARE(p->source(), KDevelop::IProblem::Plugin);
}

void TestOutputParser::testParserFaultyJson()
{
    shellcheck::OutputParser testee;

    QStringList lines;
    lines.append(QStringLiteral("{\"comments\": [{\"file\": \"/home/mvo/kde/src/kdevelop/plugins/shellcheck/Messages.sh\",\"line\": 2,\"endLine\": 2,\"column\": 12,\"endColumn\": 32,\"level\": \"warning\"\": 2046,THIS DOCUMENT IS BROKEN!!!!! prevent word splitting.\",\"fix\": null}]}"));
    testee.add(lines);

    const auto problems = testee.parse();
    QVERIFY(problems.empty());
}

