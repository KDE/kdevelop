/*
 *  SPDX-FileCopyrightText: 2024 Igor Kushnir <igorkuo@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "test_breakpoint.h"

#include <debugger/breakpoint/breakpoint.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <tests/testhelpers.h>

#include <KConfigGroup>
#include <KSharedConfig>

#include <QMetaType>
#include <QString>
#include <QTest>
#include <QUrl>

QTEST_MAIN(TestBreakpoint)

using namespace KDevelop;

Q_DECLARE_METATYPE(Breakpoint::BreakpointKind);

namespace {
QString nonAsciiFilePath()
{
    return QString::fromUtf8("/non-ASCII: ỸŒэſ/⅗∅»/—₢/ク");
}

struct BreakpointLocation
{
    explicit BreakpointLocation(const QString& location)
    {
        const auto lastColonIndex = location.lastIndexOf(':');
        QVERIFY(lastColonIndex >= 0);

        url = QUrl::fromLocalFile(location.left(lastColonIndex));

        bool ok;
        // Breakpoint::line() is zero-based
        line = location.mid(lastColonIndex + 1).toInt(&ok) - 1;
        QVERIFY(ok);
    }

    QUrl url;
    int line;
};

KConfigGroup generateBreakpointConfig(const BreakpointLocation& location)
{
    auto config = KSharedConfig::openConfig()->group("test_breakpoint");
    config.deleteGroup();

    config.writeEntry("url", location.url);
    config.writeEntry("line", location.line);
    config.writeEntry("expression", QString{});

    return config;
}

void verifyBreakpointExpression(const Breakpoint& breakpoint, const QString& expression)
{
    QCOMPARE(breakpoint.url(), {});
    QCOMPARE(breakpoint.line(), -1);
    QCOMPARE(breakpoint.expression(), expression);
}

void verifyBreakpointLocation(const Breakpoint& breakpoint, const BreakpointLocation& location)
{
    QCOMPARE(breakpoint.url(), location.url);
    QCOMPARE(breakpoint.line(), location.line);
    QCOMPARE(breakpoint.expression(), "");
}

} // unnamed namespace

TestBreakpoint::TestBreakpoint(QObject* parent)
    : QObject(parent)
{
}

void TestBreakpoint::initTestCase()
{
    AutoTestShell::init({{}});
    TestCore::initialize(Core::NoUi);
}

void TestBreakpoint::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestBreakpoint::testKindSetDataExpression_data()
{
    QTest::addColumn<Breakpoint::BreakpointKind>("kind");
    QTest::addColumn<QString>("expression");

    // A breakpoint of a kind other than CodeBreakpoint can have an expression, but never a URL and a line.
    QTest::newRow("Write") << Breakpoint::WriteBreakpoint << "/a/b.cpp:5";
    QTest::newRow("Read") << Breakpoint::ReadBreakpoint << "/etc/fstab:1";
    QTest::newRow("Access") << Breakpoint::AccessBreakpoint << "/usr/bin/kdevelop:321";
}

void TestBreakpoint::testKindSetDataExpression()
{
    QFETCH(const Breakpoint::BreakpointKind, kind);
    QFETCH(const QString, expression);

    Breakpoint b(nullptr, kind);
    b.setData(Breakpoint::LocationColumn, expression);

    verifyBreakpointExpression(b, expression);
}

void TestBreakpoint::testConfigInvalidUrl_data()
{
    QTest::addColumn<QString>("expression");

    // no (URL, line) location regex match
    QTest::newRow("no-filename") << ":1";

    // a URL unsupported because of an empty filename
    QTest::newRow("root-path") << "/:1";
    QTest::newRow("empty-filename") << "/a/b/c/:2";
    QTest::newRow("empty-filename-dir-extension") << "/a/b/c.d/:75";
    QTest::newRow("non-ASCII-empty-filename") << QString{nonAsciiFilePath() + "/:4"};

    // a URL unsupported because of a relative path
    QTest::newRow("single-word-path") << "noprefix:1";
    QTest::newRow("dot-in-path") << "./dotslash:1";
    QTest::newRow("dot-dot-in-path") << "../dotdotslash:1";
    QTest::newRow("single-percent-sign-path") << "%:1";
}

void TestBreakpoint::testSetDataExpression_data()
{
    testConfigInvalidUrl_data();

    // BreakpointLocation fails to parse the locations of the following test rows,
    // therefore they cannot be in testConfigInvalidUrl_data().

    // no (URL, line) location regex match
    QTest::newRow("empty") << "";
    QTest::newRow("word") << "word";
    QTest::newRow("no-line-number") << "/path/to/file.c:";
    QTest::newRow("invalid-line-number") << "/a/b.py:4a";
}

void TestBreakpoint::testSetDataExpression()
{
    QFETCH(const QString, expression);

    Breakpoint b(nullptr, Breakpoint::CodeBreakpoint);
    b.setData(Breakpoint::LocationColumn, expression);

    verifyBreakpointExpression(b, expression);
}

void TestBreakpoint::testSetDataUrlAndLine_data()
{
    QTest::addColumn<QString>("location");

    QTest::newRow("regular") << "/path/to/project/x.cpp:4";
    QTest::newRow("no-extension") << "/a/b/c:1";
    QTest::newRow("short-path") << "/a:54";
    QTest::newRow("capitals-and-numbers") << "/NotLOWER_caseAnymore/4/3/M:123";
    QTest::newRow("spaces") << "/a path/with   several\ts/pace.s:33";
    QTest::newRow("newline") << "/a\nb/c:14";
    QTest::newRow("punctuation") << "/a,b/c.d){e/($\\@/f !-_~%osay#`=]'\"n:54";
    QTest::newRow("colons") << "/M:N/O P::4";
    QTest::newRow("non-ASCII") << QString{nonAsciiFilePath() + ":1000"};
}

void TestBreakpoint::testConfigValidUrl_data()
{
    testSetDataUrlAndLine_data();
}

void TestBreakpoint::testSetDataUrlAndLine()
{
    QFETCH(const QString, location);

    const BreakpointLocation expectedLocation(location);
    RETURN_IF_TEST_FAILED();

    Breakpoint b(nullptr, Breakpoint::CodeBreakpoint);
    b.setData(Breakpoint::LocationColumn, location);

    verifyBreakpointLocation(b, expectedLocation);
}

void TestBreakpoint::testConfigInvalidUrl()
{
    QFETCH(const QString, expression);

    const BreakpointLocation location(expression);
    RETURN_IF_TEST_FAILED();
    const auto config = generateBreakpointConfig(location);

    const Breakpoint b(nullptr, config);

    verifyBreakpointExpression(b, "");
}

void TestBreakpoint::testConfigValidUrl()
{
    QFETCH(const QString, location);

    const BreakpointLocation expectedLocation(location);
    RETURN_IF_TEST_FAILED();
    const auto config = generateBreakpointConfig(expectedLocation);

    const Breakpoint b(nullptr, config);

    verifyBreakpointLocation(b, expectedLocation);
}

#include "moc_test_breakpoint.cpp"
