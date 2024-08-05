/*
    SPDX-FileCopyrightText: 2012-2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "bench_itemrepository.h"

#include <serialization/itemrepository.h>
#include <serialization/indexedstring.h>
#include <serialization/referencecounting.h>

#include <algorithm>
#include <limits>
#include <random>
#include <vector>
#include <QTest>

QTEST_GUILESS_MAIN(BenchItemRepository)

using namespace KDevelop;

struct TestData
{
    uint length;

    TestData& operator=(const TestData& rhs) = delete;

    uint itemSize() const
    {
        return sizeof(TestData) + length;
    }
    uint hash() const
    {
        const char* str = reinterpret_cast<const char*>(this) + sizeof(TestData);
        return IndexedString::hashString(str, length);
    }
};

struct TestDataRepositoryItemRequest
{
    //The text is supposed to be utf8 encoded
    TestDataRepositoryItemRequest(const char* text, uint length)
        : m_length(length)
        , m_text(text)
        , m_hash(IndexedString::hashString(text, length))
    {
    }

    enum {
        AverageSize = 10 //This should be the approximate average size of an Item
    };

    using HashType = uint;

    //Should return the hash-value associated with this request(For example the hash of a string)
    HashType hash() const
    {
        return m_hash;
    }

    //Should return the size of an item created with createItem
    uint itemSize() const
    {
        return sizeof(TestData) + m_length;
    }
    //Should create an item where the information of the requested item is permanently stored. The pointer
    //@param item equals an allocated range with the size of itemSize().
    void createItem(TestData* item) const
    {
        item->length = m_length;
        void* itemText = reinterpret_cast<void*>(item + 1);
        memcpy(itemText, m_text, m_length);
    }

    static void destroy(TestData* item, AbstractItemRepository&)
    {
        Q_UNUSED(item);
        //Nothing to do here (The object is not intelligent)
    }

    static bool persistent(const TestData* item)
    {
        Q_UNUSED(item);
        return true;
    }

    //Should return whether the here requested item equals the given item
    bool equals(const TestData* item) const
    {
        return item->length == m_length && (memcmp(++item, m_text, m_length) == 0);
    }
    unsigned short m_length;
    const char* m_text;
    unsigned int m_hash;
};

using TestDataRepository = ItemRepository<TestData, TestDataRepositoryItemRequest, false>;

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

static QVector<uint> insertData(const QVector<QString>& data, TestDataRepository& repo)
{
    QVector<uint> indices;
    indices.reserve(data.size());
    for (const QString& item : data) {
        const QByteArray byteArray = item.toUtf8();
        indices << repo.index(TestDataRepositoryItemRequest(byteArray.constData(), byteArray.length()));
    }

    return indices;
}

void BenchItemRepository::insert()
{
    QMutex mutex;
    TestDataRepository repo("TestDataRepositoryInsert", &mutex);
    const QVector<QString> data = generateData();
    QVector<uint> indices;
    QBENCHMARK_ONCE {
        indices = insertData(data, repo);
        repo.store();
    }
    Q_ASSERT(indices.size() == data.size());
    QCOMPARE(repo.statistics().totalItems, uint(data.size()));
}

void BenchItemRepository::remove()
{
    QMutex mutex;
    TestDataRepository repo("TestDataRepositoryRemove", &mutex);
    const QVector<QString> data = generateData();
    const QVector<uint> indices = insertData(data, repo);
    repo.store();
    QVERIFY(indices.size() == QSet<uint>(indices.begin(), indices.end()).size());
    QVERIFY(indices.size() == data.size());
    QBENCHMARK_ONCE {
        for (uint index : indices) {
            repo.deleteItem(index);
        }

        repo.store();
    }
    QCOMPARE(repo.statistics().totalItems, 0u);
}

void BenchItemRepository::removeDisk()
{
    const QVector<QString> data = generateData();
    QVector<uint> indices;
    QMutex mutex;
    {
        TestDataRepository repo("TestDataRepositoryRemoveDisk", &mutex);
        indices = insertData(data, repo);
        repo.store();
    }
    TestDataRepository repo("TestDataRepositoryRemoveDisk", &mutex);
    QVERIFY(repo.statistics().totalItems == static_cast<uint>(data.size()));
    QBENCHMARK_ONCE {
        for (uint index : std::as_const(indices)) {
            repo.deleteItem(index);
        }

        repo.store();
    }
    QCOMPARE(repo.statistics().totalItems, 0u);
}

void BenchItemRepository::lookupKey()
{
    QMutex mutex;
    TestDataRepository repo("TestDataRepositoryLookupKey", &mutex);
    const QVector<QString> data = generateData();
    QVector<uint> indices = insertData(data, repo);
    std::shuffle(indices.begin(), indices.end(), std::default_random_engine(0));
    QBENCHMARK {
        for (uint index : std::as_const(indices)) {
            repo.itemFromIndex(index);
        }
    }
}

void BenchItemRepository::lookupValue()
{
    QMutex mutex;
    TestDataRepository repo("TestDataRepositoryLookupValue", &mutex);
    const QVector<QString> data = generateData();
    QVector<uint> indices = insertData(data, repo);
    std::shuffle(indices.begin(), indices.end(), std::default_random_engine(0));
    QBENCHMARK {
        for (const QString& item : data) {
            const QByteArray byteArray = item.toUtf8();
            repo.findIndex(TestDataRepositoryItemRequest(byteArray.constData(), byteArray.length()));
        }
    }
}

void BenchItemRepository::shouldDoReferenceCounting_data()
{
    QTest::addColumn<bool>("enableReferenceCounting");
    QTest::newRow("disabled") << false;
    QTest::newRow("enabled") << true;
}

void BenchItemRepository::shouldDoReferenceCounting()
{
    using Type = unsigned;
    static_assert(sizeof(Type) == sizeof(IndexedString),
                  "Type emulates inlined IndexedString and should be of the same size.");
    constexpr std::size_t valueCount{100'000'000};
    std::vector<Type> values(valueCount);

    const auto referenceCountingStart = &values.front();
    constexpr auto referenceCountingSize = valueCount * sizeof(Type);
    QFETCH(bool, enableReferenceCounting);
    if (enableReferenceCounting) {
        enableDUChainReferenceCounting(referenceCountingStart, referenceCountingSize);
    }

    // NOTE: switching CountType from int to std::size_t slows down the "disabled" benchmark
    // by 17% and the "enabled" benchmark by 5%, as does removing the static_cast<CountType> below!
    using CountType = int;
    CountType count{0};
    static_assert(valueCount <= std::numeric_limits<CountType>::max());
    QBENCHMARK_ONCE {
        for (auto& v : values) {
            count += shouldDoDUChainReferenceCounting(&v);
        }
    }

    if (enableReferenceCounting) {
        disableDUChainReferenceCounting(referenceCountingStart, referenceCountingSize);
        QCOMPARE(count, static_cast<CountType>(values.size()));
    } else {
        QCOMPARE(count, 0);
    }
}

#include "moc_bench_itemrepository.cpp"
