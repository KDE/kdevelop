/*
 * Copyright 2014 Kevin Funk <kfunk@kde.org>
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
    QCOMPARE(findAsciiIdentifierLength(str.midRef(identifierBegin)), length);
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
        QCOMPARE(findAsciiIdentifierLength(str.midRef(i)), 0);
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
    const auto match = matchPossiblyBracedAsciiVariable(str.midRef(identifierBegin));
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
        const auto match = matchPossiblyBracedAsciiVariable(str.midRef(i));
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
    const auto match = matchPossiblyBracedAsciiVariable(str.midRef(openingBraceIndex));
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
