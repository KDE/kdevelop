#include "itemrepositorytestbase.h"

#include <QTest>

// enable various debug facilities in the ItemRepository code
// make sure to keep this block on the top, to ensure the include of itemrepository.h
// below picks it up correctly

#define DEBUG_MONSTERBUCKETS 1
#define DEBUG_ITEMREPOSITORY_LOADING 1
#define DEBUG_ITEM_REACHABILITY 1
#define DEBUG_INCORRECT_DELETE 1

#include <serialization/indexedstring.h>
#include <serialization/itemrepository.h>

#include <cstdlib>
#include <ctime>

#include <algorithm>
#include <memory>
#include <numeric>
#include <utility>
#include <vector>

using namespace KDevelop;

struct TestItem
{
    TestItem(size_t hash, uint dataSize)
        : m_hash(hash)
        , m_dataSize(dataSize)
    {
    }

    TestItem& operator=(const TestItem& rhs) = delete;

    //Every item has to implement this function, and return a valid hash.
    //Must be exactly the same hash value as ExampleItemRequest::hash() has returned while creating the item.
    size_t hash() const
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

    size_t m_hash;
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

    size_t hash() const
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

    static void destroy(TestItem* /*item*/, AbstractItemRepository&)
    {
        //Nothing to do
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
        srand(12345);
        uint highestSeenIndex = 0;

        for (uint a = 0; a < cycles; ++a) {
            {
                //Insert an item
                uint itemDecision = rand() % (smallItemsFraction + largeItemsFraction);
                uint itemSize;
                if (itemDecision < largeItemsFraction) {
                    //Create a large item: Up to 200kb
                    itemSize = (rand() % 200000) + sizeof(TestItem);
                } else
                    itemSize = (rand() % 1000) + sizeof(TestItem);
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
                uint pick = rand() % itemId;
                if (realItemsById.contains(pick)) {
                    uint index = repository.findIndex(*realItemsById[pick]);
                    QVERIFY(index);
                    QVERIFY(realItemsByIndex.contains(index));
                    QVERIFY(realItemsByIndex[index]->equals(repository.itemFromIndex(index)));

                    if (( uint ) (rand() % 100) < deletionProbability) {
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
                        delete[] realItemsById[pick];
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
                delete[] it.value();
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
    void testLeaks()
    {
        srand(12345);
        QMutex mutex;
        ItemRepository<TestItem, TestItemRequest> repository(QStringLiteral("TestItemRepository"), &mutex);
        QList<TestItem*> items;
        for (int i = 0; i < 10000; ++i) {
            TestItem* item = createItem(i, (rand() % 1000) + sizeof(TestItem));
            items << item;
            repository.index(TestItemRequest(*item));
        }

        for (auto item : qAsConst(items)) {
            delete[] item;
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
        ItemRepository<TestItem, TestItemRequest> repository(QStringLiteral("TestItemRepository"), &mutex);

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
                    std::vector<std::unique_ptr<TestItem[]>> items;
                    items.reserve(itemSizes.size());

                    // create items
                    for (auto itemId : itemIdsToCreate) {
                        std::unique_ptr<TestItem[]> item(createItem(hash, itemSizes[itemId]));
                        const auto itemRequest = TestItemRequest(*item.get(), true);

                        QVERIFY(!repository.findIndex(itemRequest));

                        for (const auto& otherItem : items)
                            QVERIFY(!otherItem.get()->equals(item.get()));

                        auto index = repository.index(itemRequest);
                        QVERIFY(index);
                        QCOMPARE(repository.findIndex(itemRequest), index);

                        items.push_back(std::move(item));
                        repoIndices.push_back(index);
                    }

                    // delete items
                    for (auto itemId : itemIdsToDelete) {
                        const auto& item = items[itemId];
                        const auto itemRequest = TestItemRequest(*item.get(), true);
                        const auto index = repoIndices[itemId];

                        QCOMPARE(repository.findIndex(itemRequest), index);
                        repository.deleteItem(index);
                        QCOMPARE(repository.findIndex(itemRequest), 0);
                    }
                } while (std::next_permutation(itemIdsToDelete.begin(), itemIdsToDelete.end()));
            } while (std::next_permutation(itemIdsToCreate.begin(), itemIdsToCreate.end()));
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
        const QScopedArrayPointer<TestItem> firstChainFirstLink(createItem(clashValue, bigItemSize));
        const uint firstChainFirstLinkIndex = repository.index(*firstChainFirstLink);
        QCOMPARE(bucketNumberForIndex(firstChainFirstLinkIndex), 1u);

        // Will also get placed in bucket 1, so root bucket table at position 'nextBucketHashSize + clashValue' will be '1'
        const QScopedArrayPointer<TestItem> secondChainFirstLink(createItem(nextBucketHashSize + clashValue,
                                                                            smallItemSize));
        const uint secondChainFirstLinkIndex = repository.index(*secondChainFirstLink);
        QCOMPARE(bucketNumberForIndex(secondChainFirstLinkIndex), 1u);

        // Will get placed in bucket 2, so bucket 1's next hash table at position 'clashValue' will be '2'
        const QScopedArrayPointer<TestItem> firstChainSecondLink(createItem(bucketHashSize + clashValue, bigItemSize));
        const uint firstChainSecondLinkIndex = repository.index(*firstChainSecondLink);
        QCOMPARE(bucketNumberForIndex(firstChainSecondLinkIndex), 2u);

        // Will also get placed in bucket 2, reachable since bucket 1's next hash table at position 'clashValue' is '2'
        const QScopedArrayPointer<TestItem> secondChainSecondLink(createItem(
                bucketHashSize + nextBucketHashSize + clashValue, smallItemSize));
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
};

#include "test_itemrepository.moc"

QTEST_MAIN(TestItemRepository)
