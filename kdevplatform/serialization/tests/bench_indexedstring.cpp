/*
    SPDX-FileCopyrightText: 2012-2013 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2020 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "bench_indexedstring.h"

#include <language/util/kdevhash.h>
#include <serialization/itemrepositoryregistry.h>
#include <serialization/indexedstring.h>
#include <tests/testhelpers.h>

#include <QTest>
#include <QStandardPaths>

#include <utility>
#include <vector>

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

static std::vector<IndexedString> createIndexedStrings(std::size_t count)
{
    std::vector<IndexedString> result;
    // result.reserve(count) is called after verifying that count is not too great.

    constexpr char first{33};
    constexpr char last{127};
    constexpr std::size_t dataSize{4};

    std::size_t maxCount{1};
    for (std::size_t i = 0; i < dataSize; ++i) {
        maxCount *= (last - first);
    }
    // Subtract 1 to account for the fact that count is checked at the beginning
    // of the innermost loop in order to support count == 0.
    --maxCount;
    QVERIFY_RETURN(count <= maxCount, result);

    result.reserve(count);

    char data[dataSize + 1] = {};
    QCOMPARE_RETURN(data[dataSize], 0, result);
    for (char a = first; a != last; ++a) {
        data[0] = a;
        for (char b = first; b != last; ++b) {
            data[1] = b;
            for (char c = first; c != last; ++c) {
                data[2] = c;
                for (char d = first; d != last; ++d) {
                    if (count-- == 0) {
                        return result;
                    }
                    data[3] = d;
                    result.emplace_back(data, dataSize);
                }
            }
        }
    }
    Q_UNREACHABLE();
}

void BenchIndexedString::bench_create()
{
    QBENCHMARK_ONCE {
        createIndexedStrings(1'000'000);
    }
}

void BenchIndexedString::bench_destroy()
{
    auto strings = createIndexedStrings(10'000'000);
    QBENCHMARK_ONCE {
        strings = {};
    }
}

void BenchIndexedString::bench_swap()
{
    IndexedString a("foo");
    IndexedString b("bar");
    QBENCHMARK {
        using std::swap;
        swap(a, b);
    }
}
