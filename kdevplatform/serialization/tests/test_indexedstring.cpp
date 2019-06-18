/*
 * This file is part of KDevelop
 * Copyright 2012-2013 Milian Wolff <mail@milianw.de>
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
 */

#include "test_indexedstring.h"

#include <language/util/kdevhash.h>
#include <serialization/itemrepositoryregistry.h>
#include <serialization/indexedstring.h>
#include <QTest>

#include <utility>

QTEST_GUILESS_MAIN(TestIndexedString)

using namespace KDevelop;

void TestIndexedString::initTestCase()
{
    ItemRepositoryRegistry::initialize(m_repositoryPath);
}

void TestIndexedString::cleanupTestCase()
{
    ItemRepositoryRegistry::deleteRepositoryFromDisk(m_repositoryPath);
}

void TestIndexedString::testUrl_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QString>("string");
    QTest::newRow("empty") << QUrl() << QString();
    QTest::newRow("/") << QUrl::fromLocalFile(QStringLiteral("/")) << QStringLiteral("/");
    QTest::newRow("/foo/bar") << QUrl::fromLocalFile(QStringLiteral("/foo/bar")) << QStringLiteral("/foo/bar");
    QTest::newRow("http://foo.com/") << QUrl(QStringLiteral("http://foo.com/")) << QStringLiteral("http://foo.com/");
    QTest::newRow("http://foo.com/bar/asdf") << QUrl(QStringLiteral("http://foo.com/bar/asdf")) << QStringLiteral(
        "http://foo.com/bar/asdf");
    QTest::newRow("file:///bar/asdf") << QUrl(QStringLiteral("file:///bar/asdf")) << QStringLiteral("/bar/asdf");

#ifdef Q_OS_WIN
    // Make sure we're not running into https://bugreports.qt.io/browse/QTBUG-41729
    QTest::newRow("file:///C:/bar/asdf") << QUrl("file:///C:/bar/asdf") << QStringLiteral("C:/bar/asdf");
#endif
}

void TestIndexedString::testUrl()
{
    QFETCH(QUrl, url);
    IndexedString indexed(url);
    QCOMPARE(indexed.toUrl(), url);
    QTEST(indexed.str(), "string");
}

static QVector<QString> generateData()
{
    QVector<QString> data;
    static const int NUM_ITEMS = 100000;
    data.resize(NUM_ITEMS);
    for (int i = 0; i < NUM_ITEMS; ++i) {
        data[i] = QStringLiteral("/foo/%1").arg(i);
    }

    return data;
}

void TestIndexedString::bench_index()
{
    const QVector<QString> data = generateData();
    QBENCHMARK {
        for (const QString& item : data) {
            IndexedString idx(item);
            Q_UNUSED(idx);
        }
    }
}

static QVector<uint> setupTest()
{
    const QVector<QString> data = generateData();
    QVector<uint> indices;
    indices.reserve(data.size());
    for (const QString& item : data) {
        IndexedString idx(item);
        indices << idx.index();
    }

    return indices;
}

void TestIndexedString::bench_length()
{
    const QVector<uint> indices = setupTest();
    QBENCHMARK {
        for (uint index : indices) {
            IndexedString str = IndexedString::fromIndex(index);
            str.length();
        }
    }
}

void TestIndexedString::bench_qstring()
{
    const QVector<uint> indices = setupTest();
    QBENCHMARK {
        for (uint index : indices) {
            IndexedString str = IndexedString::fromIndex(index);
            str.str();
        }
    }
}

void TestIndexedString::bench_kurl()
{
    const QVector<uint> indices = setupTest();
    QBENCHMARK {
        for (uint index : indices) {
            IndexedString str = IndexedString::fromIndex(index);
            str.toUrl();
        }
    }
}

void TestIndexedString::bench_qhashQString()
{
    const QVector<QString> data = generateData();
    quint64 sum = 0;
    QBENCHMARK {
        for (const auto& string : data) {
            sum += qHash(string);
        }
    }
    QVERIFY(sum > 0);
}

void TestIndexedString::bench_qhashIndexedString()
{
    const QVector<uint> indices = setupTest();
    quint64 sum = 0;
    QBENCHMARK {
        for (uint index : indices) {
            sum += qHash(IndexedString::fromIndex(index));
        }
    }
    QVERIFY(sum > 0);
}

void TestIndexedString::bench_hashString()
{
    const QVector<QString> strings = generateData();
    QVector<QByteArray> byteArrays;
    byteArrays.reserve(strings.size());
    for (const auto& string : strings) {
        byteArrays << string.toUtf8();
    }

    quint64 sum = 0;
    QBENCHMARK {
        for (const auto& array : qAsConst(byteArrays)) {
            sum += IndexedString::hashString(array.constData(), array.length());
        }
    }
    QVERIFY(sum > 0);
}

void TestIndexedString::bench_kdevhash()
{
    const QVector<QString> strings = generateData();
    QVector<QByteArray> byteArrays;
    byteArrays.reserve(strings.size());
    for (const auto& string : strings) {
        byteArrays << string.toUtf8();
    }

    quint64 sum = 0;
    QBENCHMARK {
        for (const auto& array : qAsConst(byteArrays)) {
            sum += KDevHash() << array;
        }
    }
    QVERIFY(sum > 0);
}

void TestIndexedString::bench_qSet()
{
    const QVector<uint> indices = setupTest();
    QSet<IndexedString> set;
    QBENCHMARK {
        for (uint index : indices) {
            set.insert(IndexedString::fromIndex(index));
        }
    }
}

void TestIndexedString::test()
{
    QFETCH(QString, data);

    IndexedString indexed(data);
    QCOMPARE(indexed.str(), data);
    QCOMPARE(indexed.index(), IndexedString::indexForString(data));
    const auto byteArrayData = data.toUtf8();
    QEXPECT_FAIL("char-utf8", "UTF-8 gets decoded and the char data is stored internally", Continue);
    QEXPECT_FAIL("string-utf8", "UTF-8 gets decoded and the char data is stored internally", Continue);
    QCOMPARE(indexed.length(), data.length());
    // fallback until we rely on internal utf8 encoding
    QCOMPARE(indexed.length(), byteArrayData.length());

    QCOMPARE(indexed.byteArray(), byteArrayData);
    QVERIFY(!strncmp(indexed.c_str(), byteArrayData.data(), byteArrayData.length()));
    QCOMPARE(indexed.index(), IndexedString::indexForString(byteArrayData.data(), byteArrayData.length()));

    IndexedString moved = std::move(indexed);
    QCOMPARE(indexed, IndexedString());
    QVERIFY(indexed.isEmpty());
    QCOMPARE(moved.str(), data);
}

void TestIndexedString::test_data()
{
    QTest::addColumn<QString>("data");

    QTest::newRow("empty") << QString();
    QTest::newRow("char-ascii") << QStringLiteral("a");
    QTest::newRow("char-utf8") << QStringLiteral("ä");
    QTest::newRow("string-ascii") << QStringLiteral("asdf()?=");
    QTest::newRow("string-utf8") << QStringLiteral("æſðđäöü");
}

void TestIndexedString::testCString()
{
    IndexedString str(nullptr);
    QCOMPARE(str.index(), 0u);
    QVERIFY(str.isEmpty());
}
