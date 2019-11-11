/*
 * Copyright 2019 Bernd Buschinski <b.buschinski@gmail.com>
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

#include "test_formattinghelpers.h"

#include "formattinghelpers.h"

#include <QString>
#include <QTest>

QTEST_MAIN(TestFormattingHelpers)

using namespace KDevelop;

void TestFormattingHelpers::testFuzzyMatching()
{
    QFETCH(QString, formattedMergedText);
    QFETCH(QString, selectedText);
    QFETCH(QString, leftContext);
    QFETCH(QString, rightContext);
    QFETCH(QString, expectedOutput);

    QString output = extractFormattedTextFromContext(formattedMergedText, selectedText, leftContext, rightContext);

    QCOMPARE(output, expectedOutput);
}

void TestFormattingHelpers::testFuzzyMatching_data()
{
    QTest::addColumn<QString>("formattedMergedText");
    QTest::addColumn<QString>("selectedText");
    QTest::addColumn<QString>("leftContext");
    QTest::addColumn<QString>("rightContext");
    QTest::addColumn<QString>("expectedOutput");

    QString selectedText = QStringLiteral("void bar() {\nint x;\n}");
    QString expectedOutput = QStringLiteral("void bar() {\n    int x;\n}");

    QTest::newRow("left-indentation-fixed")
        << QStringLiteral("void foo() {\n    int i;\n    int j;\n}\n\nvoid bar() {\n    int x;\n}")
        << selectedText
        << QStringLiteral("void foo() {\nint i;\n\n\nint j;\n}\n\n")
        << QStringLiteral("")
        << expectedOutput;

    QTest::newRow("right-indentation-fixed")
        << QStringLiteral("void bar() {\n    int x;\n}\n\nvoid foo() {\n    int i;\n    int j;\n}")
        << selectedText
        << QStringLiteral("")
        << QStringLiteral("\n\nvoid foo() {\nint i;\n\n\nint j;\n}")
        << expectedOutput;

    // clang-format can break long comments into multiple lines, adding new "//".
    // For the same of readability, the comments in the test are actually not very long.
    QTest::newRow("left-comment-break-fixed")
        << QStringLiteral("void foo() {\n    // very\n    // long\n}\n\nvoid bar() {\n    int x;\n}")
        << selectedText
        << QStringLiteral("void foo() {\n// very long\n}\n\n")
        << QStringLiteral("")
        << expectedOutput;

    QTest::newRow("right-comment-break-fixed")
        << QStringLiteral("void bar() {\n    int x;\n}\n\nvoid foo() {\n    // very\n    // long\n}")
        << selectedText
        << QStringLiteral("")
        << QStringLiteral("\n\nvoid foo() {\n// very long\n}")
        << expectedOutput;

    QTest::newRow("left-multilinecomment-break-fixed")
        << QStringLiteral("void foo() {\n    /* very\n    * long */\n}\n\nvoid bar() {\n    int x;\n}")
        << selectedText
        << QStringLiteral("void foo() {\n/* very long */\n}\n\n")
        << QStringLiteral("")
        << expectedOutput;

    QTest::newRow("right-multilinecomment-break-fixed")
        << QStringLiteral("void bar() {\n    int x;\n}\n\nvoid foo() {\n    /* very\n    * long */\n}")
        << selectedText
        << QStringLiteral("")
        << QStringLiteral("\n\nvoid foo() {\n/* very long */\n}")
        << expectedOutput;

    // clang-format can break long macros and add (or remove) "\"
    QTest::newRow("left-macro-break-removed")
        << QStringLiteral("#define foo(a,b) a = b\n\nvoid bar() {\n    int x;\n}")
        << selectedText
        << QStringLiteral("#define foo(a,b) \\    a = b\n\n")
        << QStringLiteral("")
        << expectedOutput;

    QTest::newRow("right-macro-break-removed")
        << QStringLiteral("void bar() {\n    int x;\n}\n\n#define foo(a,b) a = b")
        << selectedText
        << QStringLiteral("")
        << QStringLiteral("\n\n#define foo(a,b) \\    a = b")
        << expectedOutput;

    QTest::newRow("left-macro-break-added")
        << QStringLiteral("#define foo(a,b) \\ a = b\n\nvoid bar() {\n    int x;\n}")
        << selectedText
        << QStringLiteral("#define foo(a,b) a = b\n\n")
        << QStringLiteral("")
        << expectedOutput;

    QTest::newRow("right-macro-break-added")
        << QStringLiteral("void bar() {\n    int x;\n}\n\n#define foo(a,b) \\ a = b")
        << selectedText
        << QStringLiteral("")
        << QStringLiteral("\n\n#define foo(a,b) a = b")
        << expectedOutput;
}
