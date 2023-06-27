/*
    SPDX-FileCopyrightText: 2019 Bernd Buschinski <b.buschinski@gmail.com>
    SPDX-FileCopyrightText: 2023 Igor Kushnir <igorkuo@gmail.com>

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

    QString formattedMergedText;
    QString leftContext;
    QString rightContext;
    const auto addNewRow = [&](const char* dataTag) {
        QTest::newRow(dataTag) << formattedMergedText << selectedText << leftContext << rightContext << expectedOutput;
    };

    // The following 3 test data rows contain adapted code samples from https://bugs.kde.org/show_bug.cgi?id=365437
    // In all these tests an inserted brace is at the left-context-text boundary. extractFormattedTextFromContext()
    // gives up formatting, prints a warning and returns selectedText. Ideally it would figure out that the brace
    // belongs to the left context and return formatted selectedText (without the redundant brace complained about in
    // the bug report of course). But this ideal requires non-trivial implementation and could even worsen a good enough
    // existing formatting, because whitespace at context-text boundaries depends on whether braces are present.

    leftContext = R"(
	if (param) {
		if (string.isEmpty())
			)";
    selectedText = "stat";
    rightContext = R"( = true;
		else
			stat = false;
	}
)";
    formattedMergedText = R"(
    if ( param ) {
        if ( string.isEmpty() ) {
            stat = true;
        } else {
            stat = false;
        }
    }
)";
    expectedOutput = selectedText;
    addNewRow("insert-braces-around-renamed-variable");

    leftContext = R"(
    if (condition)
        value = 5;
    else
        value = -1;
    )";
    selectedText = "global_renamed_member";
    rightContext = " += value;";
    formattedMergedText = R"(
    if ( condition ) {
        value = 5;
    } else {
        value = -1;
    }
    global_renamed_member += value;)";
    expectedOutput = selectedText;
    addNewRow("insert-braces-before-renamed-variable");

    leftContext = R"(
			if (condition2) {
				if (! condition3)
					one_line_of_code;

//				one_line_of_code;
			}
			else {
				if (condition4)
					one_line_of_code;
			})";
    selectedText = R"(
			some_more_code;)";
    rightContext = QString();
    formattedMergedText = R"(
    if ( condition2 ) {
        if ( ! condition3 ) {
            one_line_of_code;
        }

//				one_line_of_code;
    } else {
        if ( condition4 ) {
            one_line_of_code;
        }
    }
    some_more_code;)";
    expectedOutput = selectedText;
    addNewRow("insert-braces-before-final-closing-brace-in-left-context");

    leftContext = R"(
    if (x)
    {
        ++z;
    }
    )";
    selectedText = "//comment";
    rightContext = "\n";
    formattedMergedText = R"(
    if (x)
        ++z;
    // comment
)";
    expectedOutput = "// comment";
    addNewRow("remove-braces-in-prefix-before-comment-in-text");

    for (auto* str : {&selectedText, &formattedMergedText, &expectedOutput}) {
        str->replace("comment", "}");
    }
    addNewRow("remove-braces-in-prefix-before-commented-out-closing-brace-in-text");

    leftContext = R"(
    if (x)
    {)";
    selectedText = R"(
        //c
        ;)";
    rightContext = "\n}";
    formattedMergedText = R"(
    if (x)
        // c
        ;)";
    expectedOutput = R"(
        // c
        ;)";
    addNewRow("remove-braces-around-comment-text");

    for (auto* str : {&selectedText, &formattedMergedText, &expectedOutput}) {
        str->replace('c', '}');
    }
    addNewRow("remove-braces-around-commented-out-closing-brace-text");

    for (auto* str : {&selectedText, &formattedMergedText, &expectedOutput}) {
        str->replace('}', '{');
    }
    addNewRow("remove-braces-around-commented-out-opening-brace-text");

    leftContext = "    \"A very very looooooooooooooong";
    selectedText = " string this is. Even ";
    rightContext = "longer now.\"\n";
    formattedMergedText = "    \"A very very looooooooooooooong string this is. \"\n    \"Even longer now.\"\n";
    expectedOutput = " string this is. \"\n    \"Even ";
    addNewRow("insert-double-quotes-within-text");

    formattedMergedText = "    \"A very very \"\n    \"looooooooooooooong string this is. Even longer now.\"\n";
    expectedOutput = selectedText;
    addNewRow("insert-double-quotes-in-left-context");

    formattedMergedText = "    \"A very very looooooooooooooong string this is. Even longer\"\n    \" now.\"\n";
    addNewRow("insert-double-quotes-in-right-context");

    // In the following 5 test data rows, an inserted pair of double quotes is at a context-text boundary.
    // extractFormattedTextFromContext() gives up formatting, prints a warning and returns selectedText
    // in order to reduce the risk of breaking whitespace changes within string literals.

    formattedMergedText = "    \"A very very looooooooooooooong\"\n    \" string this is. Even longer now.\"\n";
    expectedOutput = selectedText;
    addNewRow("insert-double-quotes-just-before-text");

    formattedMergedText = "    \"A very very looooooooooooooong \"\n    \"string this is. Even longer now.\"\n";
    addNewRow("insert-double-quotes-after-initial-space-in-text");

    formattedMergedText = "    \"A very very looooooooooooooong string this is. Even \"\n    \"longer now.\"\n";
    addNewRow("insert-double-quotes-just-after-text");

    selectedText.chop(1);
    rightContext.push_front(' ');
    expectedOutput = selectedText;
    addNewRow("insert-double-quotes-after-space-just-after-text");

    leftContext.push_back(' ');
    selectedText.remove(0, 1);
    formattedMergedText = "    \"A very very looooooooooooooong\"\n    \" string this is. Even longer now.\"\n";
    expectedOutput = selectedText;
    addNewRow("insert-double-quotes-before-space-just-before-text");

    leftContext = "\"this is ";
    selectedText = "a\"\n\" rather ";
    rightContext = "short string\"";
    formattedMergedText = "\"this is a rather short string\"";
    expectedOutput = "a rather ";
    addNewRow("remove-double-quotes-from-text");

    leftContext = "\"this \"\n\"is ";
    selectedText = "a rather ";
    addNewRow("remove-double-quotes-from-left-context");

    leftContext = "\"this is ";
    rightContext = "short \"\n\"string\"";
    addNewRow("remove-double-quotes-from-right-context");

    // In the following 5 test data rows, a removed pair of double quotes is at a context-text boundary.
    // extractFormattedTextFromContext() gives up formatting, prints a warning and returns selectedText
    // in order to reduce the risk of breaking whitespace changes within string literals.

    rightContext = "\"\n\"short string\"";
    addNewRow("remove-double-quotes-just-before-right-context");

    selectedText = "a rather\"\n\" ";
    rightContext = "short string\"";
    expectedOutput = selectedText;
    addNewRow("remove-double-quotes-before-final-space-in-text");

    selectedText = "\"\n\"a rather ";
    expectedOutput = selectedText;
    addNewRow("remove-double-quotes-at-the-beginning-of-text");

    leftContext = "\"this is \"\n\"";
    selectedText = "a rather ";
    expectedOutput = selectedText;
    addNewRow("remove-double-quotes-at-the-end-of-left-context");

    leftContext = "\"this is\"\n\" ";
    addNewRow("remove-double-quotes-before-final-space-in-left-context");
}

#include "moc_test_formattinghelpers.cpp"
