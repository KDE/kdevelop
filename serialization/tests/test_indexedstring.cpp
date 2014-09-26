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

#include <tests/testcore.h>
#include <tests/autotestshell.h>

#include <serialization/indexedstring.h>
#include <QtTest/QTest>

#include <utility>

QTEST_GUILESS_MAIN(TestIndexedString);

using namespace KDevelop;

void TestIndexedString::initTestCase()
{
  AutoTestShell::init();
  TestCore::initialize(Core::NoUi);
}

void TestIndexedString::cleanupTestCase()
{
  TestCore::shutdown();
}

void TestIndexedString::testUrl_data()
{
  QTest::addColumn<QUrl>("url");
  QTest::addColumn<QString>("string");
  QTest::newRow("empty") << QUrl() << QString();
  QTest::newRow("/") << QUrl::fromLocalFile("/") << QString("/");
  QTest::newRow("/foo/bar") << QUrl::fromLocalFile("/foo/bar") << QString("/foo/bar");
  QTest::newRow("http://foo.com/") << QUrl("http://foo.com/") << QString("http://foo.com/");
  QTest::newRow("http://foo.com/bar/asdf") << QUrl("http://foo.com/bar/asdf") << QString("http://foo.com/bar/asdf");
  QTest::newRow("file:///bar/asdf") << QUrl("file:///bar/asdf") << QString("/bar/asdf");
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
  for(int i = 0; i < NUM_ITEMS; ++i) {
    data[i] = QString("/foo/%1").arg(i);
  }
  return data;
}

void TestIndexedString::index()
{
  QVector<QString> data = generateData();
  QBENCHMARK {
    foreach(const QString& item, data) {
      IndexedString idx(item);
      Q_UNUSED(idx);
    }
  }
}

static QVector<uint> setupTest()
{
  QVector<QString> data = generateData();
  QVector<uint> indices;
  indices.reserve(data.size());
  foreach(const QString& item, data) {
    IndexedString idx(item);
    indices << idx.index();
  }
  return indices;
}

void TestIndexedString::length()
{
  QVector<uint> indices = setupTest();
  QBENCHMARK {
    foreach(uint index, indices) {
      IndexedString str = IndexedString::fromIndex(index);
      str.length();
    }
  }
}

void TestIndexedString::qstring()
{
  QVector<uint> indices = setupTest();
  QBENCHMARK {
    foreach(uint index, indices) {
      IndexedString str = IndexedString::fromIndex(index);
      str.str();
    }
  }
}

void TestIndexedString::kurl()
{
  QVector<uint> indices = setupTest();
  QBENCHMARK {
    foreach(uint index, indices) {
      IndexedString str = IndexedString::fromIndex(index);
      str.toUrl();
    }
  }
}

void TestIndexedString::hashString()
{
  QVector<QString> data = generateData();
  QBENCHMARK {
    foreach(const QString& item, data) {
      (void)qHash(item);
    }
  }
}

void TestIndexedString::hashIndexed()
{
  QVector<uint> indices = setupTest();
  QBENCHMARK {
    foreach(uint index, indices) {
      qHash(IndexedString::fromIndex(index));
    }
  }
}

void TestIndexedString::qSet()
{
  QVector<uint> indices = setupTest();
  QSet<IndexedString> set;
  QBENCHMARK {
    foreach(uint index, indices) {
      set.insert(IndexedString::fromIndex(index));
    }
  }
}

void TestIndexedString::test()
{
  QFETCH(QString, data);

  IndexedString indexed(data);
  QCOMPARE(indexed.str(), data);
  QEXPECT_FAIL("char-utf8", "UTF-8 gets decoded and the char data is stored internally", Continue);
  QEXPECT_FAIL("string-utf8", "UTF-8 gets decoded and the char data is stored internally", Continue);
  QCOMPARE(indexed.length(), data.length());
  // fallback until we rely on internal utf8 encoding
  QCOMPARE(indexed.length(), data.toUtf8().length());

  IndexedString moved = std::move(indexed);
  QCOMPARE(indexed, IndexedString());
  QVERIFY(indexed.isEmpty());
  QCOMPARE(moved.str(), data);
}

void TestIndexedString::test_data()
{
  QTest::addColumn<QString>("data");

  QTest::newRow("empty") << QString();
  QTest::newRow("char-ascii") << QString("a");
  QTest::newRow("char-utf8") << QString::fromUtf8("ä");
  QTest::newRow("string-ascii") << QString::fromLatin1("asdf()?=");
  QTest::newRow("string-utf8") << QString::fromUtf8("æſðđäöü");
}

