/*
    SPDX-FileCopyrightText: 2012-2013 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2020 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "bench_indexedstring.h"

#include <language/util/kdevhash.h>
#include <serialization/indexedstring.h>
#include <serialization/indexedstringview.h>
#include <tests/testhelpers.h>

#include <QTest>

#include <type_traits>
#include <utility>
#include <vector>

QTEST_GUILESS_MAIN(BenchIndexedString)

using namespace KDevelop;

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
        for (const auto& array : std::as_const(byteArrays)) {
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
        for (const auto& array : std::as_const(byteArrays)) {
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

void BenchIndexedString::bench_string_vector_data()
{
    QTest::addColumn<ElementType>("elementType");
    QTest::addColumn<ContainerType>("containerType");

    for (const auto elementType : {ElementType::uint, ElementType::IndexedStringView, ElementType::IndexedString}) {
        for (const auto containerType : {ContainerType::StdVector, ContainerType::QVector}) {
            QTest::addRow("%s<%s>", enumeratorName(containerType), enumeratorName(elementType))
                << elementType << containerType;
        }
    }
}

template<class Container>
static void benchPopFront()
{
    const auto toElementType = [](int i) {
        using ElementType = typename Container::value_type;
        const auto str = QString::number(i);
        if constexpr (std::is_integral_v<ElementType>) {
            return IndexedString::indexForString(str);
        } else {
            return ElementType{str};
        }
    };

    Container container;
    for (int i = 0; i < 10000; ++i) {
        container.push_back(toElementType(i));
    }
    // deliberately do a worst-case remove-from-front here
    // we want to see how this performs without any noexcept move operators
    QBENCHMARK_ONCE {
        while (!container.empty()) {
            container.erase(container.begin());
        }
    }
}

void BenchIndexedString::bench_string_vector()
{
    QFETCH(const ElementType, elementType);
    QFETCH(const ContainerType, containerType);

    switch (elementType) {
    case ElementType::uint:
        switch (containerType) {
        case ContainerType::StdVector: {
            benchPopFront<std::vector<uint>>();
            break;
        }
        case ContainerType::QVector: {
            benchPopFront<QVector<uint>>();
            break;
        }
        }
        break;
    case ElementType::IndexedStringView:
        switch (containerType) {
        case ContainerType::StdVector: {
            benchPopFront<std::vector<IndexedStringView>>();
            break;
        }
        case ContainerType::QVector: {
            benchPopFront<QVector<IndexedStringView>>();
            break;
        }
        }
        break;
    case ElementType::IndexedString:
        switch (containerType) {
        case ContainerType::StdVector: {
            benchPopFront<std::vector<IndexedString>>();
            break;
        }
        case ContainerType::QVector: {
            benchPopFront<QVector<IndexedString>>();
            break;
        }
        }
        break;
    }
}

#include "moc_bench_indexedstring.cpp"
