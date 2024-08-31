/*
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "test_stringhandler.h"

#include "kdevstringhandler.h"

#include <QTest>
#include <QStandardPaths>

QTEST_MAIN(TestStringHandler)

using namespace KDevelop;

Q_DECLARE_METATYPE(HtmlToPlainTextMode)

void TestStringHandler::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void TestStringHandler::testHtmlToPlainText()
{
    QFETCH(QString, html);
    QFETCH(HtmlToPlainTextMode, mode);
    QFETCH(QString, expectedPlainText);

    QString plainText = htmlToPlainText(html, mode);
    QCOMPARE(plainText, expectedPlainText);
}

void TestStringHandler::testHtmlToPlainText_data()
{
    QTest::addColumn<QString>("html");
    QTest::addColumn<HtmlToPlainTextMode>("mode");
    QTest::addColumn<QString>("expectedPlainText");

    QTest::newRow("simple-fast")
        << "<p>bar() </p><dl><dt class=\"param-name-index-0\">a</dt><dd class=\"param-descr-index-0\"> foo</dd></dl>"
        << KDevelop::FastMode << "bar() a foo";
    QTest::newRow("simple-complete")
        << "<p>bar() </p><dl><dt class=\"param-name-index-0\">a</dt><dd class=\"param-descr-index-0\"> foo</dd></dl>"
        << KDevelop::CompleteMode << "bar() \na\nfoo";
}

void TestStringHandler::testEscapeJavaScriptString()
{
    QFETCH(QByteArray, unescaped);
    QFETCH(QByteArray, escaped);

    const auto actual = escapeJavaScriptString(unescaped);
    QCOMPARE(actual, escaped);
}

void TestStringHandler::testEscapeJavaScriptString_data()
{
    QTest::addColumn<QByteArray>("unescaped");
    QTest::addColumn<QByteArray>("escaped");

    const auto nothingToEscape = QByteArrayLiteral("html { background: white !important; }");
    QTest::newRow("nothing to escape") << nothingToEscape << nothingToEscape;

    QTest::newRow("newlines and single quotes")
        << QByteArrayLiteral("body {\nfont-family: 'Liberation Serif', sans-serif;\n }\n")
        << QByteArrayLiteral("body {\\nfont-family: \\'Liberation Serif\\', sans-serif;\\n }\\n");

    QTest::newRow("HTML and double quotes") << QByteArrayLiteral(R"(<img src="my-icon (2).png" alt="[app icon]">)")
                                            << QByteArrayLiteral(R"(<img src=\"my-icon (2).png\" alt=\"[app icon]\">)");

    // Prevent '\0' from terminating the string.
    constexpr char allUnescaped[] = "\\ \0\" \b\f\n\r\t\v '";
    const auto allUnescapedSize = sizeof(allUnescaped) / sizeof(char) - 1;
    constexpr char allEscaped[] = "\\\\ \\0\\\" \\b\\f\\n\\r\\t\\v \\'";
    const auto allEscapedSize = sizeof(allEscaped) / sizeof(char) - 1;
    QTest::newRow("all special characters") << QByteArray(allUnescaped, allUnescapedSize)
                                            << QByteArray(allEscaped, allEscapedSize);
}

namespace {
void addAsciiIdentifierData()
{
    QTest::addColumn<QString>("str");
    QTest::addColumn<int>("identifierBegin");
    QTest::addColumn<int>("identifierEnd");

    QString str;

    str = "u0_cBQp.ad";
    const auto indexOfPeriod = str.indexOf('.');
    QTest::newRow("identifier at index 0") << str << 0 << indexOfPeriod;
    QTest::newRow("identifier ends at string end") << str << indexOfPeriod + 1 << str.size();
    QTest::newRow("one letter at string end") << str << indexOfPeriod + 2 << str.size();
    QTest::newRow("string end") << str << str.size() << str.size();

    str = "AbcD901z95-24";
    QTest::newRow("identifier after a letter") << str << 2 << str.indexOf('-');

    str = "a=$B+c";
    QTest::newRow("identifier after a dollar") << str << 3 << str.indexOf('+');

    str = "-_,";
    QTest::newRow("underscore identifier") << str << 1 << 2;

    str = "_aBt_9";
    QTest::newRow("entire-string identifier") << str << 0 << str.size();
}

void addUnmatchedAsciiIdentifierData()
{
    QTest::addColumn<QString>("str");
    QTest::addColumn<int>("unmatchedCharsBegin");
    QTest::addColumn<int>("unmatchedCharsEnd");

    QString str;

    str = "12970abC3D5";
    QTest::newRow("a digit") << str << 0 << str.indexOf('a');
    auto indexOfDigit = str.indexOf('3');
    QTest::newRow("a digit between letters") << str << indexOfDigit << indexOfDigit + 1;
    indexOfDigit = str.indexOf('5');
    QTest::newRow("a digit at string end") << str << indexOfDigit << indexOfDigit + 1;

    str = ".,-[/\\]$&{(=*+!#\r\n)~%`}\0\t |@^?;:<>\"'";
    QTest::newRow("a non-ID character") << str << 0 << str.size();

    str = QString::fromUtf8("\u5c07\u4f0a\u00f8\u00e5\ub418\uc9c0\u0414\u041b\u0407"
                            "\u062c\u0628\u062a\u044a\u044b\u043c\u00ae\u00bf\u00ab"
                            "\u00bb\u00b6\u00fc\u00a7\u201c\u201d\u6d77\u9b5a\u300d\u3232");
    QTest::newRow("a non-ASCII character") << str << 0 << str.size();
}
}

void TestStringHandler::testFindAsciiIdentifierLength()
{
    QFETCH(QString, str);
    QFETCH(int, identifierBegin);
    QFETCH(int, identifierEnd);

    const auto length = identifierEnd - identifierBegin;
    Q_ASSERT_X(length >= 0, Q_FUNC_INFO, "Wrong data.");
    QCOMPARE(findAsciiIdentifierLength(QStringView{str}.sliced(identifierBegin)), length);
}

void TestStringHandler::testFindAsciiIdentifierLength_data()
{
    addAsciiIdentifierData();
}

void TestStringHandler::testFindAsciiIdentifierLengthNoMatch()
{
    QFETCH(QString, str);
    QFETCH(int, unmatchedCharsBegin);
    QFETCH(int, unmatchedCharsEnd);

    Q_ASSERT_X(unmatchedCharsBegin < unmatchedCharsEnd, Q_FUNC_INFO,
               "Nothing to test. A mistake in our data?");
    for (int i = unmatchedCharsBegin; i < unmatchedCharsEnd; ++i) {
        QCOMPARE(findAsciiIdentifierLength(QStringView{str}.sliced(i)), 0);
    }
}

void TestStringHandler::testFindAsciiIdentifierLengthNoMatch_data()
{
    addUnmatchedAsciiIdentifierData();
}

void TestStringHandler::testMatchUnbracedAsciiVariable()
{
    QFETCH(QString, str);
    QFETCH(int, identifierBegin);
    QFETCH(int, identifierEnd);

    const auto length = identifierEnd - identifierBegin;
    Q_ASSERT_X(length >= 0, Q_FUNC_INFO, "Wrong data.");
    const auto match = matchPossiblyBracedAsciiVariable(QStringView{str}.sliced(identifierBegin));
    QCOMPARE(match.length, length);
    QCOMPARE(match.name, str.mid(identifierBegin, length));
}

void TestStringHandler::testMatchUnbracedAsciiVariable_data()
{
    addAsciiIdentifierData();

    QString str;

    str = "a{b";
    QTest::newRow("no closing brace") << str << 1 << 1;

    str = "1x}";
    QTest::newRow("no opening brace") << str << 1 << 2;

    str = "{{u}}";
    QTest::newRow("nested braces") << str << 0 << 0;
}

void TestStringHandler::testUnmatchedAsciiVariable()
{
    QFETCH(QString, str);
    QFETCH(int, unmatchedCharsBegin);
    QFETCH(int, unmatchedCharsEnd);

    Q_ASSERT_X(unmatchedCharsBegin < unmatchedCharsEnd, Q_FUNC_INFO,
               "Nothing to test. A mistake in our data?");
    for (int i = unmatchedCharsBegin; i < unmatchedCharsEnd; ++i) {
        const auto match = matchPossiblyBracedAsciiVariable(QStringView{str}.sliced(i));
        QCOMPARE(match.length, 0);
        QCOMPARE(match.name, QString{});
    }
}

void TestStringHandler::testUnmatchedAsciiVariable_data()
{
    addUnmatchedAsciiIdentifierData();

    QTest::newRow("empty braces") << QStringLiteral("{}a}") << 0 << 2;
    QTest::newRow("a digit inside braces") << QStringLiteral("{4}") << 0 << 3;
    QTest::newRow("a non-ID character inside braces") << QStringLiteral("a{!}b") << 1 << 4;
}

void TestStringHandler::testMatchBracedAsciiVariable()
{
    QFETCH(QString, str);
    QFETCH(int, openingBraceIndex);
    QFETCH(int, closingBraceIndex);

    const auto variableLength = closingBraceIndex - openingBraceIndex - 1;
    Q_ASSERT_X(variableLength > 0, Q_FUNC_INFO, "Wrong data.");
    const auto match = matchPossiblyBracedAsciiVariable(QStringView{str}.sliced(openingBraceIndex));
    QCOMPARE(match.length, variableLength + 2);
    QCOMPARE(match.name, str.mid(openingBraceIndex + 1, variableLength));
}

void TestStringHandler::testMatchBracedAsciiVariable_data()
{
    QTest::addColumn<QString>("str");
    QTest::addColumn<int>("openingBraceIndex");
    QTest::addColumn<int>("closingBraceIndex");

    QString str;

    str = "{a1}-a{_}";
    QTest::newRow("variable at index 0") << str << 0 << str.indexOf('}');
    QTest::newRow("variable ends at string end") << str << str.lastIndexOf('{') << str.size() - 1;

    str = "{a2_bDX__45Ek}";
    QTest::newRow("entire-string variable") << str << 0 << str.size() - 1;
}

void TestStringHandler::testStripAnsiSequences()
{
    QFETCH(QString, input);
    QFETCH(QString, expectedOutput);

    const auto output = stripAnsiSequences(input);
    QCOMPARE(output, expectedOutput);
}

void TestStringHandler::testStripAnsiSequences_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expectedOutput");

    QTest::newRow("simple")
        << QStringLiteral("foo [31m[34mbar[0m:")
        << "foo bar:";
}

void TestStringHandler::testNormalizeLineEndings()
{
    QFETCH(QByteArray, text);
    QFETCH(QByteArray, expectedOutput);

    normalizeLineEndings(text);
    QCOMPARE(text, expectedOutput);
}

void TestStringHandler::testNormalizeLineEndings_data()
{
    QTest::addColumn<QByteArray>("text");
    QTest::addColumn<QByteArray>("expectedOutput");

    QTest::newRow("trivial")
        << QByteArray("foo\nbar\n")
        << QByteArray("foo\nbar\n");
    QTest::newRow("dos")
        << QByteArray("foo\r\nbar\r\n")
        << QByteArray("foo\nbar\n");
    QTest::newRow("macos_classic")
        << QByteArray("foo\rbar\r")
        << QByteArray("foo\nbar\n");
    QTest::newRow("mess")
        << QByteArray("\r\n\n\r\r\r\n\r")
        << QByteArray("\n\n\n\n\n\n");
}

#include "moc_test_stringhandler.cpp"
