
#include <QTest>

#define DEBUG_MONSTERBUCKETS 1
#define DEBUG_ITEMREPOSITORY_LOADING 1
#define DEBUG_ITEM_REACHABILITY 1
#define DEBUG_INCORRECT_DELETE 1
#define DEBUG_SMALL_REPO 1

#include "itemrepositorytestbase.h"
#include "itemrepositoryitem.h"

#include "serialization/itemrepository.h"

using namespace KDevelop;

// Fill up repository to keep ItemRepositoryBucketLinearGrowthFactor (ie 10) buckets non-allocated
// @returns next hash to insert
int almostFillRepository(ItemRepository<TestItem, TestItemRequest>& repository, int additionalSpace)
{
    int i = 0;
    // Two indices are reserved: 0x0 and 0xffff. Making the max number of items ItemRepositoryBucketLimit - 2
    for (; i < ItemRepositoryBucketLimit - ItemRepositoryBucketLinearGrowthFactor - 2; ++i) {
        //Fill up repository
        TestItemPtr item(createItem(i, ItemRepositoryBucketSize - additionalSpace - 1));
        unsigned int indexed = repository.index(TestItemRequest(*item));
        Q_ASSERT(indexed != 0);
    }
    return i;
}

class TestItemRepositoryOverflow : public ItemRepositoryTestBase
{
    Q_OBJECT

private Q_SLOTS:
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
            TestItemPtr item4(createItem(
                i++, ItemRepositoryBucketSize * (ItemRepositoryBucketLinearGrowthFactor)-additionalSpace - 1));
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
            TestItemPtr item4(createItem(
                i++, ItemRepositoryBucketSize * (ItemRepositoryBucketLinearGrowthFactor)-additionalSpace - 1));
            indexedMonster = repository.index(TestItemRequest(*item4));
            QCOMPARE(indexedMonster, 0);
        }
    }
};

#include "test_itemrepositoryoverflow.moc"

QTEST_MAIN(TestItemRepositoryOverflow);
