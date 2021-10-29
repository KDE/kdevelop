/*
    SPDX-FileCopyrightText: 2019 Bernd Buschinski <b.buschinski@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "test_formattinghelpers.h"

#include "formattinghelpers.h"

#include <QString>
#include <QTest>
#include <QStandardPaths>

QTEST_MAIN(TestFormattingHelpers)

using namespace KDevelop;

void TestFormattingHelpers::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

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
    // For the sake of readability, the comments in the test are actually not very long.
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

    // clang-format can break long strings into multiple small strings, adding new quotation mark.
    // For the sake of readability, the strings in the test are actually not very long.
    QTest::newRow("left-string-break-fixed")
        << QStringLiteral("void foo() {\n    string a = \"very \"\n    \"long\";\n}\n\nvoid bar() {\n    int x;\n}")
        << selectedText
        << QStringLiteral("void foo() {\nstring a = \"very long\";\n}\n\n")
        << QStringLiteral("")
        << expectedOutput;

    QTest::newRow("right-string-break-fixed")
        << QStringLiteral("void bar() {\n    int x;\n}\n\nvoid foo() {\n    string a = \"very \"\n    \"long\";\n}")
        << selectedText
        << QStringLiteral("")
        << QStringLiteral("\n\nvoid foo() {\nstring a = \"very long\";\n}")
        << expectedOutput;
}
