#include "itemrepositorytestbase.h"
#include "itemrepositoryitem.h"

#include <QTest>

#define DEBUG_ITEMREPOSITORY_LOADING 1

#include "serialization/itemrepository.h"

using namespace KDevelop;

class TestItemRepository : public ItemRepositoryTestBase
{
    Q_OBJECT

private:
    using Repo = ItemRepository<TestItem, TestItemRequest>;
    using Bucket = Repo::MyBucket;

private Q_SLOTS:
    void testBasic()
    {
        QMutex mutex;
        Repo repository("bucketTestDummy", &mutex); // needed for delete
        Bucket bucket;
        bucket.initialize(0);
        TestItemPtr item(createItem(5, ItemRepositoryBucketSize / 2));
        auto index = bucket.index(TestItemRequest(*item), item->itemSize());
        QCOMPARE_EQ(bucket.findIndex(*item), index);
        bucket.deleteItem(index, item->hash(), repository);
        QCOMPARE_EQ(bucket.findIndex(*item), 0);
    }

    void testClashes()
    {
        QMutex mutex;
        Repo repository("bucketTestDummy", &mutex); // needed for delete
        Bucket bucket;
        bucket.initialize(0);
        TestItemPtr item1(createItem(1, ItemRepositoryBucketSize / 2));
        auto index1 = bucket.index(TestItemRequest(*item1, true), item1->itemSize());
        TestItemPtr item2(createItem(1, ItemRepositoryBucketSize / 2 - 6));
        auto index2 = bucket.index(TestItemRequest(*item2, true), item2->itemSize());
        QCOMPARE_EQ(bucket.m_objectMap[0], 0);
        QCOMPARE_EQ(bucket.m_objectMap[1], bucket.AdditionalSpacePerItem);
        QVERIFY(std::all_of(bucket.m_objectMap + 2, bucket.m_objectMap + Bucket::ObjectMapSize, [](const auto i) {
            return i == 0;
        }));
        QCOMPARE_EQ(bucket.findIndex(TestItemRequest(*item1, true)), index1);
        QCOMPARE_EQ(bucket.findIndex(TestItemRequest(*item2, true)), index2);

        bucket.deleteItem(index1, item1->hash(), repository);
        QCOMPARE_EQ(bucket.findIndex(TestItemRequest(*item2, true)), index2);
        QCOMPARE_EQ(bucket.findIndex(TestItemRequest(*item1, true)), 0);
    }
};

#include "test_itemrepositorybucket.moc"

QTEST_MAIN(TestItemRepository)
