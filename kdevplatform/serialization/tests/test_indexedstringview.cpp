/*
    SPDX-FileCopyrightText: 2012-2013 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2023 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "test_indexedstringview.h"

#include <serialization/indexedstring.h>
#include <serialization/indexedstringview.h>
#include <tests/testhelpers.h>

#include <QTest>

QTEST_GUILESS_MAIN(TestIndexedStringView)

using namespace KDevelop;

void TestIndexedStringView::testUrl_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QString>("string");
    QTest::newRow("empty") << QUrl() << QString();
    QTest::newRow("/") << QUrl::fromLocalFile(QStringLiteral("/")) << QStringLiteral("/");
    QTest::newRow("/foo/bar") << QUrl::fromLocalFile(QStringLiteral("/foo/bar")) << QStringLiteral("/foo/bar");
    QTest::newRow("http://foo.com/") << QUrl(QStringLiteral("http://foo.com/")) << QStringLiteral("http://foo.com/");
    QTest::newRow("http://foo.com/bar/asdf")
        << QUrl(QStringLiteral("http://foo.com/bar/asdf")) << QStringLiteral("http://foo.com/bar/asdf");
    QTest::newRow("file:///bar/asdf") << QUrl(QStringLiteral("file:///bar/asdf")) << QStringLiteral("/bar/asdf");

#ifdef Q_OS_WIN
    // Make sure we're not running into https://bugreports.qt.io/browse/QTBUG-41729
    QTest::newRow("file:///C:/bar/asdf") << QUrl("file:///C:/bar/asdf") << QStringLiteral("C:/bar/asdf");
#endif
}

void TestIndexedStringView::testUrl()
{
    QFETCH(const QUrl, url);
    IndexedStringView indexed(url);
    QCOMPARE(indexed.toString(), IndexedString{url});
    QCOMPARE(indexed.toString().toUrl(), url);
    QCOMPARE(indexed.index(), IndexedString::indexForUrl(url));
    QTEST(indexed.toString().str(), "string");
}

void TestIndexedStringView::test_data()
{
    QTest::addColumn<QString>("data");

    QTest::newRow("empty") << QString();
    QTest::newRow("char-ascii") << QStringLiteral("a");
    QTest::newRow("char-utf8") << QStringLiteral("ä");
    QTest::newRow("string-ascii") << QStringLiteral("asdf()?=");
    QTest::newRow("string-utf8") << QStringLiteral("æſðđäöü");
}

void TestIndexedStringView::test()
{
    QFETCH(const QString, data);

    const IndexedStringView indexed(data);
    QCOMPARE(indexed.toString(), IndexedString{data});
    QCOMPARE(indexed.toString().str(), data);
    QCOMPARE(indexed.index(), IndexedString::indexForString(data));
}

void TestIndexedStringView::testCString()
{
    const IndexedStringView str(nullptr);
    QCOMPARE(str.toString(), IndexedString{nullptr});
    QCOMPARE(str.index(), 0u);
    QVERIFY(str.isEmpty());
}

#include "moc_test_indexedstringview.cpp"
