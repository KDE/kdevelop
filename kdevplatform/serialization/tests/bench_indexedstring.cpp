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

#include "bench_indexedstring.h"

#include <language/util/kdevhash.h>
#include <serialization/itemrepositoryregistry.h>
#include <serialization/indexedstring.h>
#include <QTest>
#include <QStandardPaths>

QTEST_GUILESS_MAIN(BenchIndexedString)

using namespace KDevelop;

void BenchIndexedString::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    ItemRepositoryRegistry::initialize(m_repositoryPath);
}

void BenchIndexedString::cleanupTestCase()
{
    ItemRepositoryRegistry::deleteRepositoryFromDisk(m_repositoryPath);
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

void BenchIndexedString::bench_index()
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

void BenchIndexedString::bench_length()
{
    const QVector<uint> indices = setupTest();
    QBENCHMARK {
        for (uint index : indices) {
            IndexedString str = IndexedString::fromIndex(index);
            str.length();
        }
    }
}

void BenchIndexedString::bench_qstring()
{
    const QVector<uint> indices = setupTest();
    QBENCHMARK {
        for (uint index : indices) {
            IndexedString str = IndexedString::fromIndex(index);
            str.str();
        }
    }
}

void BenchIndexedString::bench_kurl()
{
    const QVector<uint> indices = setupTest();
    QBENCHMARK {
        for (uint index : indices) {
            IndexedString str = IndexedString::fromIndex(index);
            str.toUrl();
        }
    }
}

void BenchIndexedString::bench_qhashQString()
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

void BenchIndexedString::bench_qhashIndexedString()
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

void BenchIndexedString::bench_hashString()
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

void BenchIndexedString::bench_kdevhash()
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

void BenchIndexedString::bench_qSet()
{
    const QVector<uint> indices = setupTest();
    QSet<IndexedString> set;
    QBENCHMARK {
        for (uint index : indices) {
            set.insert(IndexedString::fromIndex(index));
        }
    }
}
