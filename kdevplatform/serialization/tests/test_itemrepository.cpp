#include "itemrepositorytestbase.h"

#include <QTest>
#include <QElapsedTimer>

// enable various debug facilities in the ItemRepository code
// make sure to keep this block on the top, to ensure the include of itemrepository.h
// below picks it up correctly

#define DEBUG_MONSTERBUCKETS 1
#define DEBUG_ITEMREPOSITORY_LOADING 1
#define DEBUG_ITEM_REACHABILITY 1
#define DEBUG_INCORRECT_DELETE 1

#include <serialization/indexedstring.h>
#include <serialization/itemrepository.h>

#include <algorithm>
#include <memory>
#include <numeric>
#include <random>
#include <string_view>
#include <utility>
#include <vector>

using namespace KDevelop;

struct TestItem
{
    TestItem(uint hash, uint dataSize) : m_hash(hash)
        , m_dataSize(dataSize)
    {
    }

    TestItem& operator=(const TestItem& rhs) = delete;

    //Every item has to implement this function, and return a valid hash.
    //Must be exactly the same hash value as ExampleItemRequest::hash() has returned while creating the item.
    unsigned int hash() const
    {
        return m_hash;
    }

    //Every item has to implement this function, and return the complete size this item takes in memory.
    //Must be exactly the same value as ExampleItemRequest::itemSize() has returned while creating the item.
    unsigned int itemSize() const
    {
        return sizeof(TestItem) + m_dataSize;
    }

    bool equals(const TestItem* rhs) const
    {
        return rhs->m_hash == m_hash
               && itemSize() == rhs->itemSize()
               && memcmp(reinterpret_cast<const char*>(this), rhs, itemSize()) == 0;
    }

    std::string_view as_string_view() const
    {
        return std::string_view(reinterpret_cast<const char*>(this) + sizeof(*this), m_dataSize);
    }

    uint m_hash;
    uint m_dataSize;
};

struct TestItemRequest
{
    TestItem& m_item;
    bool m_compareData;

    TestItemRequest(TestItem& item, bool compareData = false)
        : m_item(item)
        , m_compareData(compareData)
    {
    }
    enum {
        AverageSize = 700 //This should be the approximate average size of an Item
    };

    uint hash() const
    {
        return m_item.hash();
    }

    //Should return the size of an item created with createItem
    uint itemSize() const
    {
        return m_item.itemSize();
    }

    void createItem(TestItem* item) const
    {
        memcpy(reinterpret_cast<void*>(item), &m_item, m_item.itemSize());
    }

    static void destroy(TestItem* item, AbstractItemRepository&)
    {
        // Fill with zeros to speed up failures.
        std::fill_n(reinterpret_cast<char*>(item), item->itemSize(), 0);
    }

    static bool persistent(const TestItem* /*item*/)
    {
        return true;
    }

    //Should return whether the here requested item equals the given item
    bool equals(const TestItem* item) const
    {
        return hash() == item->hash() && (!m_compareData || m_item.equals(item));
    }
};

uint smallItemsFraction = 20; //Fraction of items between 0 and 1 kb
uint largeItemsFraction = 1; //Fraction of items between 0 and 200 kb
uint cycles = 10000;
uint deletionProbability = 1; //Percentual probability that a checked item is deleted. Per-cycle probability must be multiplied with checksPerCycle.
uint checksPerCycle = 10;

TestItem* createItem(uint id, uint size)
{
    TestItem* ret;
    char* data = new char[size];
    uint dataSize = size - sizeof(TestItem);
    ret = new (data) TestItem(id, dataSize);

    //Fill in same random pattern
    for (uint a = 0; a < dataSize; ++a)
        data[sizeof(TestItem) + a] = ( char )(a + id);

    return ret;
}

template<typename Rng>
TestItem* createRandomItem(Rng& rngsource)
{
    // Create a randomized item up to 128Kib. A normal bucket can fit 64Kib before it is
    // converted to a monster bucket, so the generated item can span three buckets at most.
    std::uniform_int_distribution<uint> sizedist(1, 1024 * 128);
    uint size = sizeof(TestItem) + sizedist(rngsource);
    char* data = new char[size];
    uint dataSize = size - sizeof(TestItem);

    // Generate a random payload consisting of a few ASCII letters.
    std::uniform_int_distribution<int> gen(0, 16);
    auto* payload = data + sizeof(TestItem);
    std::generate_n(payload, dataSize, [&]() {
        return 'A' + gen(rngsource);
    });
    // Compute a hash value.
    auto hash = std::hash<std::string_view>()(std::string_view(payload, dataSize));
    return new (data) TestItem(hash, dataSize);
}

void freeItem(TestItem* ptr)
{
    if (!ptr)
        return;
    // No op, but needed for correctness.
    ptr->~TestItem();
    // ptr was allocated with "new char[]", so we must use delete[] (char*) to free it.
    delete[] reinterpret_cast<char*>(ptr);
}

struct TestItemDeleter
{
    void operator()(TestItem* ptr)
    {
        freeItem(ptr);
    }
};

using TestItemPtr = std::unique_ptr<TestItem, TestItemDeleter>;
// Fill up repository to keep ItemRepositoryBucketLinearGrowthFactor (ie 10) buckets non-allocated
// @returns next hash to insert
int almostFillRepository(ItemRepository<TestItem, TestItemRequest>& repository, int additionalSpace)
{
    int i = 0;
    // Two indices are reserved: 0x0 and 0xffff. Making the max number of items ItemRepositoryBucketLimit - 2
    for (; i < ItemRepositoryBucketLimit - ItemRepositoryBucketLinearGrowthFactor - 2; ++i) {
        //Fill up repository
        TestItem* item = createItem(i, ItemRepositoryBucketSize - additionalSpace - 1);
        unsigned int indexed = repository.index(TestItemRequest(*item));
        Q_ASSERT(indexed != 0);
    }
    return i;
}

///@todo Add a test where the complete content is deleted again, and make sure the result has a nice structure
///@todo More consistency and lost-space tests, especially about monster-buckets. Make sure their space is re-claimed
class TestItemRepository
    : public ItemRepositoryTestBase
{
    Q_OBJECT

private Q_SLOTS:
    void testItemRepository()
    {
        QMutex mutex;
        ItemRepository<TestItem, TestItemRequest> repository(QStringLiteral("TestItemRepository"), &mutex);

        // calling statistics here does some extended tests and shouldn't crash or assert
        repository.statistics();

        uint itemId = 0;
        QHash<uint, TestItem*> realItemsByIndex;
        QHash<uint, TestItem*> realItemsById;
        uint totalInsertions = 0, totalDeletions = 0;
        uint maxSize = 0;
        uint totalSize = 0;
        std::minstd_rand rngsource(12345);

        uint highestSeenIndex = 0;

        for (uint a = 0; a < cycles; ++a) {
            {
                //Insert an item
                uint itemSize;
                std::uniform_int_distribution<uint> largeOrSmall(0, smallItemsFraction + largeItemsFraction - 1);
                if (largeOrSmall(rngsource) < largeItemsFraction) {
                    //Create a large item: Up to 200kb
                    std::uniform_int_distribution<uint> sizedist(sizeof(TestItem), sizeof(TestItem) + 200000 - 1);
                    itemSize = sizedist(rngsource);
                } else {
                    std::uniform_int_distribution<uint> sizedist(sizeof(TestItem), sizeof(TestItem) + 1000 - 1);
                    itemSize = sizedist(rngsource);
                }
                TestItem* item = createItem(++itemId, itemSize);
                Q_ASSERT(item->hash() == itemId);
                QVERIFY(item->equals(item));
                uint index = repository.index(TestItemRequest(*item));
                if (index > highestSeenIndex)
                    highestSeenIndex = index;
                Q_ASSERT(index);
                realItemsByIndex.insert(index, item);
                realItemsById.insert(itemId, item);
                ++totalInsertions;
                totalSize += itemSize;
                if (itemSize > maxSize)
                    maxSize = itemSize;
            }

            for (uint a = 0; a < checksPerCycle; ++a) {
                //Check an item
                std::uniform_int_distribution<uint> pickdist(0, itemId - 1);
                uint pick = pickdist(rngsource);
                if (realItemsById.contains(pick)) {
                    uint index = repository.findIndex(*realItemsById[pick]);
                    QVERIFY(index);
                    QVERIFY(realItemsByIndex.contains(index));
                    QVERIFY(realItemsByIndex[index]->equals(repository.itemFromIndex(index)));

                    std::uniform_int_distribution<uint> deletiondist(0, 99);
                    if (deletiondist(rngsource) < deletionProbability) {
                        ++totalDeletions;
                        //Delete the item
                        repository.deleteItem(index);
                        QVERIFY(!repository.findIndex(*realItemsById[pick]));
                        uint newIndex = repository.index(*realItemsById[pick]);
                        QVERIFY(newIndex);
                        QVERIFY(realItemsByIndex[index]->equals(repository.itemFromIndex(newIndex)));

#ifdef POSITION_TEST
                        //Since we have previously deleted the item, there must be enough space
                        if (!((newIndex >> 16) <= (highestSeenIndex >> 16))) {
                            qDebug() << "size:" << realItemsById[pick]->itemSize();
                            qDebug() << "previous highest seen bucket:" << (highestSeenIndex >> 16);
                            qDebug() << "new bucket:" << (newIndex >> 16);
                        }
                        QVERIFY((newIndex >> 16) <= (highestSeenIndex >> 16));
#endif
                        repository.deleteItem(newIndex);
                        QVERIFY(!repository.findIndex(*realItemsById[pick]));
                        freeItem(realItemsById[pick]);
                        realItemsById.remove(pick);
                        realItemsByIndex.remove(index);
                    }
                }
            }

            // calling statistics here does some extended tests and shouldn't crash or assert
            repository.statistics();
        }

        // cleanup
        {
            for (auto it = realItemsByIndex.constBegin(); it != realItemsByIndex.constEnd(); ++it) {
                repository.deleteItem(it.key());
                freeItem(it.value());
            }

            realItemsById.clear();
            realItemsByIndex.clear();
        }

        qDebug() << "total insertions:" << totalInsertions << "total deletions:" << totalDeletions <<
        "average item size:" << (totalSize / totalInsertions) << "biggest item size:" << maxSize;

        const auto stats = repository.statistics();
        qDebug() << stats;
        QVERIFY(stats.freeUnreachableSpace < stats.freeSpaceInBuckets / 100); // < 1% of the free space is unreachable
        QVERIFY(stats.freeSpaceInBuckets < stats.usedSpaceForBuckets); // < 20% free space
    }

    void testItemRepositoryTorture()
    {
        /**
         * Temporally comment the below QSKIP() to run the ItemRepository verifier/torture test.
         * Set QTEST_FUNCTION_TIMEOUT environment variable to a large value: 1800000 (30 mins)
         * and optionally build KDevelop in release mode with Q_ASSERT enabled.
         *
         * The test hammers the ItemRepository index(), findIndex(), and deleteItem() methods until
         * a bug is detected, and will otherwise keep running forever. There is a 1/10000 chance
         * the repository is stored and subsequently reloaded from disk.
         */
        QSKIP("ItemRepository torture test.");

        QMutex mutex;
        auto repository =
            std::make_unique<ItemRepository<TestItem, TestItemRequest>>(QStringLiteral("TestItemRepository"), &mutex);

        // calling statistics here does some extended tests and shouldn't crash or assert
        repository->statistics();

        // std::string_view refers to the payload data in TestItem, so this is actually a set.
        QHash<std::string_view, TestItem*> verifyItems;
        QHash<std::string_view, TestItem*>::key_value_iterator itr;

        size_t loopCount = 0;
        size_t totalInsertions = 0, totalDeletions = 0;
        std::minstd_rand rngsource(12345);

        // Measure loops per second.
        QElapsedTimer elapsed;
        elapsed.start();

        while (true) {
            bool didInsert = false;
            bool deleteOnly = false;
            TestItemPtr item;

            /**
             * Cap out at ~128KiB * 4096 max. When loopCount reaches 4096 * 4096 there is a
             * high confidence that the ItemRepository basic operations are working as
             * expected.
             */
            if (verifyItems.size() > 4096) {
                // Pick a random item to be deleted.
                std::uniform_int_distribution<ptrdiff_t> pickdist(0, verifyItems.size() - 1);
                itr = verifyItems.keyValueBegin();
                std::advance(itr, pickdist(rngsource));
                item.reset(itr->second);
                deleteOnly = true;
            } else {
                // Create a randomized item and try insert it.
                item.reset(createRandomItem(rngsource));
                std::tie(itr, didInsert) = verifyItems.try_emplace(item->as_string_view(), item.get());
            }

            if (didInsert) {
                QCOMPARE(repository->findIndex(TestItemRequest(*item, true)), 0);
                ++totalInsertions;
                auto index = repository->index(TestItemRequest(*item, true));
                QCOMPARE_NE(index, 0);
                QCOMPARE(repository->findIndex(TestItemRequest(*item, true)), index);
                // verifyItems owns the TestItem now.
                item.release();
            } else {
                auto index = repository->findIndex(TestItemRequest(*item, true));
                QCOMPARE_NE(index, 0);
                ++totalDeletions;
                repository->deleteItem(index);
                QCOMPARE(repository->findIndex(TestItemRequest(*item, true)), 0);
                auto ptr = itr->second;
                verifyItems.remove(itr->first);
                // Delete the duplicate item?
                if (!deleteOnly) {
                    item.reset(ptr);
                }
            }

            if (loopCount % 1000 == 0) {
                auto speed = 1000.0 / elapsed.elapsed();
                qDebug() << "Loop:" << loopCount << "totalInsertions:" << totalInsertions
                         << "totalDeletions:" << totalDeletions << speed <<"loops/ms";
                if (loopCount % 10000 == 0) {
                    for (auto line : repository->printStatistics().split(QLatin1Char('\n'))) {
                        qDebug().noquote() << line;
                    }
                }
                elapsed.restart();
            }

            std::uniform_int_distribution<int> reload(0, 10000);
            if (reload(rngsource) == 0) {
                qDebug() << "ItemRepository::store() to disk and reload";
                repository->store();
                repository = std::make_unique<ItemRepository<TestItem, TestItemRequest>>(
                    QStringLiteral("TestItemRepository"), &mutex);
            }

            // ItemRepository contains a lot of Q_ASSERTs that usually fire before the below code would be useful.
#if 0
            // Exhaustively validate that none of the inserted items have "vanished" from the repository.
            for (auto checkItem : std::as_const(verifyItems)) {
                auto index = repository->findIndex(TestItemRequest(*checkItem, true));
                if (!index) {
                    qDebug() << "Loop:" << loopCount << "totalInsertions:" << totalInsertions << "totalDeletions:" << totalDeletions;
                    qDebug() << repository->statistics();
                    qDebug() << "TestItem" << checkItem << "with hash():" << checkItem->hash() << "itemSize():" << checkItem->itemSize() << "was not found!";
                    if (!didInsert) {
                        qDebug() << "deleteItem() or findIndex() has a bug!";
                    } else {
                        qDebug() << "index() or findIndex() has a bug!";
                    }
                }
                QCOMPARE_NE(index, 0);
            }
#endif
#if 0
            // Reverse, only items in verifyItems can exist in the repository.
            auto checker = [&](const TestItem* checkItem) {
                Q_ASSERT(checkItem);
                if (!verifyItems.contains(checkItem->as_string_view())) {
                    qDebug() << "Loop:" << loopCount << "totalInsertions:" << totalInsertions << "totalDeletions:" << totalDeletions;
                    qDebug() << repository->statistics();
                    qDebug() << "TestItem" << checkItem << "with hash():" << checkItem->hash() << "itemSize():" << checkItem->itemSize() << "should not exist!";
                    if (!didInsert) {
                        qDebug() << "deleteItem() or findIndex() has a bug!";
                    } else {
                        qDebug() << "index() or findIndex() has a bug!";
                    }
                }
                return true;
            };
            repository->visitAllItems(checker);
#endif
            ++loopCount;
        }
    }
    void testLeaks()
    {
        std::minstd_rand rngsource(12345);
        QMutex mutex;
        ItemRepository<TestItem, TestItemRequest> repository(QStringLiteral("TestItemRepository"), &mutex);
        QList<TestItem*> items;
        for (int i = 0; i < 10000; ++i) {
            std::uniform_int_distribution<uint> sizedist(sizeof(TestItem), sizeof(TestItem) + 1000 - 1);
            TestItem* item = createItem(i, sizedist(rngsource));
            items << item;
            repository.index(TestItemRequest(*item));
        }

        for (auto item : std::as_const(items)) {
            freeItem(item);
        }
    }

    void testOverflow()
    {
        QMutex mutex;
        ItemRepository<TestItem, TestItemRequest> repository(QStringLiteral("TestItemRepository"), &mutex);
        const int additionalSpace = ItemRepository<TestItem, TestItemRequest>::MyBucket::AdditionalSpacePerItem;
        int i = almostFillRepository(repository, additionalSpace);
        const int maxIndex = i + ItemRepositoryBucketLinearGrowthFactor;
        for (; i < maxIndex - 1; ++i) {
            // insert an item exactly the size of a bucket, so we can deterministically check the index.
            TestItemPtr item(createItem(i, ItemRepositoryBucketSize - additionalSpace - 1));
            unsigned int indexed = repository.index(TestItemRequest(*item));
            QCOMPARE_NE(indexed, 0);
        }

        // Insert last item
        TestItemPtr item(createItem(i, ItemRepositoryBucketSize - additionalSpace - 1));
        unsigned int indexed = repository.index(TestItemRequest(*item));
        QCOMPARE_NE(indexed, 0);

        // This insertion will fail
        item.reset(createItem(i++, ItemRepositoryBucketSize - additionalSpace - 1));
        indexed = repository.index(TestItemRequest(*item));
        QCOMPARE_NE(indexed, 0);
    }

    void testMonsterOverflow()
    {
        QMutex mutex;
        ItemRepository<TestItem, TestItemRequest> repository(QStringLiteral("TestItemRepository"), &mutex);
        for (int i = 0; i < ItemRepositoryBucketLimit / 2 + 3; ++i) {
            // insert an item that requires a monster bucket consisting of 2 buckets
            TestItemPtr item(createItem(i, ItemRepositoryBucketSize * 1.5));
            unsigned int indexed = repository.index(TestItemRequest(*item));

            if (i >= (ItemRepositoryBucketLimit - 2) / 2) {
                QCOMPARE(indexed, 0);
            } else {
                QCOMPARE_NE(indexed, 0);
            }
        }
    }

    void testLastBucketIsMonsterInsertFail()
    {
        QMutex mutex;
        ItemRepository<TestItem, TestItemRequest> repository(QStringLiteral("TestItemRepository"), &mutex);
        const int additionalSpace = ItemRepository<TestItem, TestItemRequest>::MyBucket::AdditionalSpacePerItem;

        int i = almostFillRepository(repository, additionalSpace);
        // Attempt to insert a monster bucket that is one bigger than what can be inserted
        TestItemPtr item(
            createItem(i, ItemRepositoryBucketSize * (ItemRepositoryBucketLinearGrowthFactor + 1) - additionalSpace));
        unsigned int indexed = repository.index(TestItemRequest(*item));
        QCOMPARE(indexed, 0);
    }

    void testLastBucketIsMonsterInsertSuccess()
    {
        QMutex mutex;
        ItemRepository<TestItem, TestItemRequest> repository(QStringLiteral("TestItemRepository"), &mutex);
        const int additionalSpace = ItemRepository<TestItem, TestItemRequest>::MyBucket::AdditionalSpacePerItem;
        int i = almostFillRepository(repository, additionalSpace);
        // Insert monster bucket that fills all remaining space.
        int indexedMonster = 0;
        {
            TestItemPtr item(
                createItem(i++, ItemRepositoryBucketSize * (ItemRepositoryBucketLinearGrowthFactor)-additionalSpace));
            indexedMonster = repository.index(TestItemRequest(*item));
            QCOMPARE_NE(indexedMonster, 0);
        }

        {
            // Check nothing more can be inserted
            //Normal item
            TestItemPtr item(createItem(i, ItemRepositoryBucketSize - additionalSpace - 1));
            int indexed = repository.index(TestItemRequest(*item));
            QCOMPARE(indexed, 0);

            // Monster item
            TestItemPtr item4(createItem(i++, ItemRepositoryBucketSize * (ItemRepositoryBucketLinearGrowthFactor)
                                        - additionalSpace - 1));
            indexed = repository.index(TestItemRequest(*item4));
            QCOMPARE(indexed, 0);
        }

        repository.deleteItem(indexedMonster);

        QCOMPARE(repository.m_freeSpaceBuckets.size(), 10);

        {
            // After freeing the last monster bucket, m_currentBucket points to the last bucket.
            // A forceful attempt to insert is performed (not checking isEmpty(), nor canAllocate())
            // which succeeds.
            // Remove this insertion when reuse of empty buckets is fixed.
            TestItemPtr item(createItem(i++, ItemRepositoryBucketSize - additionalSpace - 1));
            repository.index(TestItemRequest(*item));
        }

        // insert ItemRepositoryBucketLinearGrowthFactor (10 ) items to fill buckets emptied by removing the monster bucket
        // Currently fails because reuse of empty buckets is broken.
        const int maxi = i + ItemRepositoryBucketLinearGrowthFactor;
        for (; i < maxi; ++i) {
            TestItemPtr item(createItem(i, ItemRepositoryBucketSize - additionalSpace - 1));
            int indexed = repository.index(TestItemRequest(*item));
            // https://invent.kde.org/kdevelop/kdevelop/-/merge_requests/869 is the start of an attempt to fix this.
            QEXPECT_FAIL("", "Reusing empty buckets for non-monsters doesn't work.", Continue);
            QCOMPARE_NE(indexed, 0);
        }

        //Repository is full again
        {
            TestItemPtr item(createItem(i++, ItemRepositoryBucketSize - additionalSpace - 1));
            int indexed = repository.index(TestItemRequest(*item));
            QCOMPARE(indexed, 0);
        }

        {
            TestItemPtr item4(createItem(i++, ItemRepositoryBucketSize * (ItemRepositoryBucketLinearGrowthFactor)
                             - additionalSpace - 1));
            indexedMonster = repository.index(TestItemRequest(*item4));
            QCOMPARE(indexedMonster, 0);
        }
    }

    void testStringSharing()
    {
        QString qString;
        qString.fill('.', 1000);
        IndexedString indexedString(qString);
        const int repeat = 10000;
        QVector<QString> strings;
        strings.resize(repeat);
        for (int i = 0; i < repeat; ++i) {
            strings[i] = indexedString.str();
            QCOMPARE(qString, strings[i]);
        }
    }
    void deleteClashingMonsterBucket_data()
    {
        QTest::addColumn<QVector<uint>>("itemSizes");
        int testIndex = 0;

        auto addRow = [&testIndex](QVector<uint> itemSizes) {
            QTest::addRow("%d", testIndex++) << itemSizes;
        };

        const auto big = ItemRepositoryBucketSize + 10;
        const auto small = 20;
        addRow({small, big});
        addRow({small, small + 1});
        addRow({big, big + 1});
        addRow({small, small + 1, small + 2});
        addRow({small, small + 1, big});
        addRow({small, big, big + 1});
        addRow({big, big + 1, big + 2});
    }
    void deleteClashingMonsterBucket()
    {
        QFETCH(QVector<uint>, itemSizes);

        std::vector<std::size_t> originalItemIds(itemSizes.size());
        std::iota(originalItemIds.begin(), originalItemIds.end(), 0);

        QMutex mutex;
        auto repository = std::make_unique<ItemRepository<TestItem, TestItemRequest> >(QStringLiteral("TestItemRepository"), &mutex);

        // repeat the below for multiple hashes
        for (auto hash : {1235u, 1236u}) {
            // one permutation for the insertion order
            auto itemIdsToCreate = originalItemIds;
            do {
                // one permutation for the deletion order
                auto itemIdsToDelete = originalItemIds;
                do {
                    std::vector<uint> repoIndices;
                    repoIndices.reserve(itemSizes.size());
                    std::vector<TestItemPtr> items;
                    items.reserve(itemSizes.size());

                    // create items
                    for (auto itemId : itemIdsToCreate) {
                        TestItemPtr item(createItem(hash, itemSizes[itemId]));
                        const auto itemRequest = TestItemRequest(*item.get(), true);

                        QVERIFY(!repository->findIndex(itemRequest));

                        for (const auto& otherItem : items)
                            QVERIFY(!otherItem.get()->equals(item.get()));

                        auto index = repository->index(itemRequest);
                        QVERIFY(index);
                        QCOMPARE(repository->findIndex(itemRequest), index);

                        items.push_back(std::move(item));
                        repoIndices.push_back(index);
                    }

                    if (hash == 1236u) {
                        repository->store();
                        repository = std::make_unique<ItemRepository<TestItem, TestItemRequest> >(QStringLiteral("TestItemRepository"), &mutex);
                    }

                    // delete items
                    for (auto itemId : itemIdsToDelete) {
                        const auto& item = items[itemId];
                        const auto itemRequest = TestItemRequest(*item.get(), true);
                        const auto index = repoIndices[itemId];

                        QCOMPARE(repository->findIndex(itemRequest), index);
                        repository->deleteItem(index);
                        QCOMPARE(repository->findIndex(itemRequest), 0);
                    }

                } while (std::next_permutation(itemIdsToDelete.begin(), itemIdsToDelete.end()));
            } while (std::next_permutation(itemIdsToCreate.begin(), itemIdsToCreate.end()));
        }

        repository->store();
    }
    void loadAndUpdateRepoWithBucketAndMonsterBucket()
    {
        QMutex mutex;

        // Construct a bucket chain for hash 1235 that has a monster bucket in the second position and
        // another item that follows that (item 1 -> item 2 -> item4). Delete item 2 again and store and
        // reload the repository to add another item.

        TestItemPtr item1(createItem(1235, ItemRepositoryBucketSize / 2));
        const auto itemRequest1 = TestItemRequest(*item1.get(), true);
        // does not fit into a normal bucket, requires monster bucket that is extended by 1 bucket
        TestItemPtr item2(createItem(1235, ItemRepositoryBucketSize));
        const auto itemRequest2 = TestItemRequest(*item2.get(), true);
        TestItemPtr item3(createItem(1236, ItemRepositoryBucketSize / 2));
        const auto itemRequest3 = TestItemRequest(*item3.get(), true);

        TestItemPtr item4(createItem(1235, ItemRepositoryBucketSize / 2 + 1));
        const auto itemRequest4 = TestItemRequest(*item4.get(), true);

        // Some other item that will fit in the space that is freed by later deleting item 2
        TestItemPtr item5(createItem(1237, ItemRepositoryBucketSize));
        const auto itemRequest5 = TestItemRequest(*item5.get(), true);

        unsigned int index1, index2, index3, index5, index4;

        {
            ItemRepository<TestItem, TestItemRequest> repository(QStringLiteral("TestItemRepository"), &mutex);

            index1 = repository.index(itemRequest1);
            QVERIFY(index1);
            index2 = repository.index(itemRequest2);
            QVERIFY(index2);
            index3 = repository.index(itemRequest3);
            QVERIFY(index3);
            index4 = repository.index(itemRequest4);
            QVERIFY(index4);
            QCOMPARE(repository.findIndex(itemRequest1), index1);
            QCOMPARE(repository.findIndex(itemRequest2), index2);
            QCOMPARE(repository.findIndex(itemRequest3), index3);
            QCOMPARE(repository.findIndex(itemRequest4), index4);

            // remove item 2 to free space for two buckets
            repository.deleteItem(index2);

            repository.store();
            repository.statistics();
        }

        {
            ItemRepository<TestItem, TestItemRequest> repository(QStringLiteral("TestItemRepository"), &mutex);

            QCOMPARE(index1, repository.findIndex(itemRequest1));
            QVERIFY(!repository.findIndex(itemRequest2));
            QCOMPARE(index3, repository.findIndex(itemRequest3));
            QCOMPARE(index4, repository.findIndex(itemRequest4));

            // item 5 will fit in the space between item 1 and item 3, after it did not fit into a single bucket
            // after item 4. The space where it fits will be memory mapped before conversion, making that space write-protected.
            index5 = repository.index(itemRequest5);
            QCOMPARE(index5, index2);

            repository.deleteItem(index3);

            repository.store();
            repository.statistics();
        }

        {
            ItemRepository<TestItem, TestItemRequest> repository(QStringLiteral("TestItemRepository"), &mutex);

            QCOMPARE(index1, repository.findIndex(itemRequest1));
            QVERIFY(!repository.findIndex(itemRequest2));
            QVERIFY(!repository.findIndex(itemRequest3));
            QCOMPARE(index4, repository.findIndex(itemRequest4));
            QCOMPARE(index5, repository.findIndex(itemRequest5));

            repository.deleteItem(index1);
            repository.deleteItem(index5);
            repository.deleteItem(index4);

            repository.store();

            qDebug() << repository.printStatistics();
        }
    }
    void usePermissiveModuloWhenRemovingClashLinks()
    {
        QMutex mutex;
        ItemRepository<TestItem, TestItemRequest> repository(QStringLiteral("PermissiveModulo"), &mutex);

        const uint bucketHashSize = decltype(repository)::bucketHashSize;
        const uint nextBucketHashSize = decltype(repository)::MyBucket::NextBucketHashSize;
        auto bucketNumberForIndex = [](const uint index) {
                                        return index >> 16;
                                    };

        const uint clashValue = 2;

        // Choose sizes that ensure that the items fit in the desired buckets
        const uint bigItemSize = ItemRepositoryBucketSize * 0.55 - 1;
        const uint smallItemSize = ItemRepositoryBucketSize * 0.25 - 1;

        // Will get placed in bucket 1 (bucket zero is invalid), so the root bucket table at position 'clashValue' will be '1'
        const TestItemPtr firstChainFirstLink(createItem(clashValue, bigItemSize));
        const uint firstChainFirstLinkIndex = repository.index(*firstChainFirstLink);
        QCOMPARE(bucketNumberForIndex(firstChainFirstLinkIndex), 1u);

        // Will also get placed in bucket 1, so root bucket table at position 'nextBucketHashSize + clashValue' will be '1'
        const TestItemPtr secondChainFirstLink(createItem(nextBucketHashSize + clashValue, smallItemSize));
        const uint secondChainFirstLinkIndex = repository.index(*secondChainFirstLink);
        QCOMPARE(bucketNumberForIndex(secondChainFirstLinkIndex), 1u);

        // Will get placed in bucket 2, so bucket 1's next hash table at position 'clashValue' will be '2'
        const TestItemPtr firstChainSecondLink(createItem(bucketHashSize + clashValue, bigItemSize));
        const uint firstChainSecondLinkIndex = repository.index(*firstChainSecondLink);
        QCOMPARE(bucketNumberForIndex(firstChainSecondLinkIndex), 2u);

        // Will also get placed in bucket 2, reachable since bucket 1's next hash table at position 'clashValue' is '2'
        const TestItemPtr secondChainSecondLink(
            createItem(bucketHashSize + nextBucketHashSize + clashValue, smallItemSize));
        const uint secondChainSecondLinkIndex = repository.index(*secondChainSecondLink);
        QCOMPARE(bucketNumberForIndex(secondChainSecondLinkIndex), 2u);

        /*
         * At this point we have two chains in the repository, rooted at 'clashValue' and 'nextBucketHashSize + clashValue'
         * Both of the chains start in bucket 1 and end in bucket 2, but both chains share the same link to bucket 2
         * This is because two of the hashes clash the other two when % bucketHashSize, but all of them clash % nextBucketHashSize
         */

        repository.deleteItem(firstChainSecondLinkIndex);

        /*
         * Now we've deleted the second item in the first chain, this means the first chain no longer requires a link to the
         * second bucket where that item was... but the link must remain, since it's shared (clashed) by the second chain.
         *
         * When cutting a link out of the middle of the chain, we need to check if its items clash using the "permissive"
         * modulus (the size of the /next/ buckets map), which is always a factor of the "stricter" modulus (the size of the
         * /root/ buckets map).
         *
         * This behavior implies that there will sometimes be useless buckets in the bucket chain for a given hash, so when
         * cutting out the root link, it's safe to skip over them to the first clash with the 'stricter' modulus.
         */

        // The second item of the second chain must still be reachable
        QCOMPARE(repository.findIndex(*secondChainSecondLink), secondChainSecondLinkIndex);

        /*
         * As a memo to anyone who's still reading this, this also means the following situation can exist:
         *
         * bucketHashSize == 8
         * nextBucketHashSize == 4
         * U is a link table
         * B is a bucket
         * [...] are the hashes of the contained items
         *
         * U
         * U
         * U -> B1
         * U
         * U
         * U
         * U -> B2
         * U
         *
         * B0 (Invalid)
         * B1 -> [2, 6]
         *   U
         *   U
         *   U -> B3
         *   U
         * B2 -> [14]
         *   U
         *   U
         *   U -> B1
         *   U
         * B3 -> [10]
         *   U
         *   U
         *   U
         *   U
         *
         * The chain for hash 6 is:
         * Root[6] -> B2[2] -> B1[2] -> B3
         *
         * If you remove the item with hash 6, 6 and 2 will clash with mod 4 (permissive)
         *
         * So the useless link `B2[2] -> B1` will be preserved, even though its useless
         * as the item with hash 2 is reachable directly from the root.
         *
         * So TODO: Don't preserve links to items accessible from root buckets. This cannot
         * be done correctly using only Bucket::hasClashingItem as of now.
         */
    }

    void testFreeBucketOrdering()
    {
        QMutex mutex;
        ItemRepository<TestItem, TestItemRequest> repository(QStringLiteral("ordering"), &mutex);

        // All buckets are empty, they're ordered by bucket id
        QVERIFY(std::is_sorted(repository.m_freeSpaceBuckets.cbegin(), repository.m_freeSpaceBuckets.cend()));
        QVERIFY(std::is_sorted(repository.m_freeSpaceBuckets.cbegin(), repository.m_freeSpaceBuckets.cend(),
                               [&](BucketId left, BucketId right) {
                                   return repository.compareFreeSpaceBucketsIndex(left, right) == -1;
                               }));

        // Item in 1st bucket, 524 bytes left empty
        TestItemPtr item(createItem(1, 65000));
        repository.index(TestItemRequest(*item));

        // Item in 2nd bucket, 1524 bytes left empty
        item.reset(createItem(2, 64000));
        repository.index(TestItemRequest(*item));

        // Item in 3rd bucket, 224 bytes left empty
        item.reset(createItem(3, 65300));
        repository.index(TestItemRequest(*item));

        // Item in 4th bucket, 524 bytes left empty
        item.reset(createItem(4, 65000));
        repository.index(TestItemRequest(*item));

        // Item in 5th bucket, monster bucket. Removes 5 and 6 from free space
        item.reset(createItem(5, 66000));
        repository.index(TestItemRequest(*item));

        // Item in 7th bucket, 124 bytes left empty
        item.reset(createItem(6, 65400));
        repository.index(TestItemRequest(*item));

        // Ordering is by empty space, then by bucket id.
        // Buckets 8+ (completely empty) remain at end, sorted by bucket id.
        // sorting by free size of non empty buckets results in 7,3,1,4,2
        const QList<BucketId> check = {7, 3, 1, 4, 2, 8, 9};
        QCOMPARE(repository.m_freeSpaceBuckets, check);
        QVERIFY(std::is_sorted(repository.m_freeSpaceBuckets.cbegin(), repository.m_freeSpaceBuckets.cend(),
                               [&](BucketId left, BucketId right) {
                                   return repository.compareFreeSpaceBucketsIndex(left, right) == -1;
                               }));
    }
};

#include "test_itemrepository.moc"

QTEST_MAIN(TestItemRepository)
