#include <QObject>
#include <QtTest/QtTest>
#include <serialization/itemrepository.h>
#include <serialization/indexedstring.h>
#include <stdlib.h>
#include <time.h>

#include <tests/testcore.h>
#include <tests/autotestshell.h>

struct TestItem {
  TestItem(uint hash, uint dataSize) : m_hash(hash), m_dataSize(dataSize) {
  }
  //Every item has to implement this function, and return a valid hash.
  //Must be exactly the same hash value as ExampleItemRequest::hash() has returned while creating the item.
  unsigned int hash() const {
    return m_hash;
  }

  //Every item has to implement this function, and return the complete size this item takes in memory.
  //Must be exactly the same value as ExampleItemRequest::itemSize() has returned while creating the item.
  unsigned int itemSize() const {
    return sizeof(TestItem) + m_dataSize;
  }

  bool equals(const TestItem* rhs) const
  {
    return rhs->m_hash == m_hash
        && itemSize() == rhs->itemSize()
        && memcmp((char*)this, rhs, itemSize()) == 0;
  }

  uint m_hash;
  uint m_dataSize;
};

struct TestItemRequest {
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

  uint hash() const {
    return m_item.hash();
  }

  //Should return the size of an item created with createItem
  size_t itemSize() const {
      return m_item.itemSize();
  }

  void createItem(TestItem* item) const {
    memcpy(item, &m_item, m_item.itemSize());
  }

  static void destroy(TestItem* /*item*/, KDevelop::AbstractItemRepository&) {
    //Nothing to do
  }

  static bool persistent(const TestItem* /*item*/) {
    return true;
  }

  //Should return whether the here requested item equals the given item
  bool equals(const TestItem* item) const {
    return hash() == item->hash() && (!m_compareData || m_item.equals(item));
  }
};

uint smallItemsFraction = 20; //Fraction of items betwen 0 and 1 kb
uint largeItemsFraction = 1; //Fraction of items between 0 and 200 kb
uint cycles = 100000;
uint deletionProbability = 1; //Percentual probability that a checked item is deleted. Per-cycle probability must be multiplied with checksPerCycle.
uint checksPerCycle = 10;

TestItem* createItem(uint id, uint size) {
  TestItem* ret;
  char* data = new char[size];
  uint dataSize = size - sizeof(TestItem);
  ret = new (data) TestItem(id, dataSize);

  //Fill in same random pattern
  for(uint a = 0; a < dataSize; ++a)
    data[sizeof(TestItem) + a] = (char)(a + id);

  return ret;
}

///@todo Add a test where the complete content is deleted again, and make sure the result has a nice structure
///@todo More consistency and lost-space tests, especially about monster-buckets. Make sure their space is re-claimed
class TestItemRepository : public QObject {
  Q_OBJECT
  private slots:
    void initTestCase() {
      KDevelop::AutoTestShell::init();
      KDevelop::TestCore* core = new KDevelop::TestCore();
      core->initialize(KDevelop::Core::NoUi);
    }
    void cleanupTestCase() {
        KDevelop::TestCore::shutdown();
    }
    void testItemRepository() {
      KDevelop::ItemRepository<TestItem, TestItemRequest> repository("TestItemRepository");
      uint itemId = 0;
      QHash<uint, TestItem*> realItemsByIndex;
      QHash<uint, TestItem*> realItemsById;
      uint totalInsertions = 0, totalDeletions = 0;
      uint maxSize = 0;
      uint totalSize = 0;
      srand(time(NULL));
      uint highestSeenIndex = 0;

      for(uint a = 0; a < cycles; ++a) {

        {
          //Insert an item
          uint itemDecision = rand() % (smallItemsFraction + largeItemsFraction);
          uint itemSize;
          if(itemDecision < largeItemsFraction) {
            //Create a large item: Up to 200kb
            itemSize = (rand() % 200000) + sizeof(TestItem);
          } else
            itemSize = (rand() % 1000) + sizeof(TestItem);
          TestItem* item = createItem(++itemId, itemSize);
          Q_ASSERT(item->hash() == itemId);
          QVERIFY(item->equals(item));
          uint index = repository.index(TestItemRequest(*item));
          if(index > highestSeenIndex)
            highestSeenIndex = index;
          Q_ASSERT(index);
          realItemsByIndex.insert(index, item);
          realItemsById.insert(itemId, item);
          ++totalInsertions;
          totalSize += itemSize;
          if(itemSize > maxSize)
            maxSize = itemSize;
        }

        for(uint a = 0; a < checksPerCycle; ++a) {
            //Check an item
            uint pick = rand() % itemId;
            if(realItemsById.contains(pick)) {
              uint index = repository.findIndex(*realItemsById[pick]);
              QVERIFY(index);
              QVERIFY(realItemsByIndex.contains(index));
              QVERIFY(realItemsByIndex[index]->equals(repository.itemFromIndex(index)));

              if((uint) (rand() % 100) < deletionProbability) {
                ++totalDeletions;
                //Delete the item
                repository.deleteItem(index);
                QVERIFY(!repository.findIndex(*realItemsById[pick]));
                uint newIndex = repository.index(*realItemsById[pick]);
                QVERIFY(newIndex);
                QVERIFY(realItemsByIndex[index]->equals(repository.itemFromIndex(newIndex)));

#ifdef POSITION_TEST
                //Since we have previously deleted the item, there must be enough space
                if(!((newIndex >> 16) <= (highestSeenIndex >> 16))) {
                  qDebug() << "size:" << realItemsById[pick]->itemSize();
                  qDebug() << "previous highest seen bucket:" << (highestSeenIndex >> 16);
                  qDebug() << "new bucket:" << (newIndex >> 16);
                }
                QVERIFY((newIndex >> 16) <= (highestSeenIndex >> 16));
#endif
                repository.deleteItem(newIndex);
                QVERIFY(!repository.findIndex(*realItemsById[pick]));
                delete realItemsById[pick];
                realItemsById.remove(pick);
                realItemsByIndex.remove(index);
              }
           }
        }


      }
      qDebug() << "total insertions:" << totalInsertions << "total deletions:" << totalDeletions << "average item size:" << (totalSize / totalInsertions) << "biggest item size:" << maxSize;

      KDevelop::ItemRepository<TestItem, TestItemRequest>::Statistics stats = repository.statistics();
      qDebug() << stats;
      QVERIFY(stats.freeUnreachableSpace < stats.freeSpaceInBuckets/100); // < 1% of the free space is unreachable
      QVERIFY(stats.freeSpaceInBuckets < stats.usedSpaceForBuckets); // < 20% free space
    }
    void testLeaks()
    {
      KDevelop::ItemRepository<TestItem, TestItemRequest> repository("TestItemRepository");
      QList<TestItem*> items;
      for(int i = 0; i < 10000; ++i) {
        TestItem* item = createItem(i, (rand() % 1000) + sizeof(TestItem));
        items << item;
        repository.index(TestItemRequest(*item));
      }
      qDeleteAll(items);
      items.clear();
    }
    void testStringSharing()
    {
      QString qString;
        qString.fill('.', 1000);
      KDevelop::IndexedString indexedString(qString);
      const int repeat = 10000;
      QVector<QString> strings;
      strings.resize(repeat);
      for(int i = 0; i < repeat; ++i) {
        strings[i] = indexedString.str();
        QCOMPARE(qString, strings[i]);
      }
    }
    void deleteClashingMonsterBucket()
    {
      KDevelop::ItemRepository<TestItem, TestItemRequest> repository("TestItemRepository");
      const uint hash = 1235;

      QScopedPointer<TestItem> monsterItem(createItem(hash, KDevelop::ItemRepositoryBucketSize + 10));
      QScopedPointer<TestItem> smallItem(createItem(hash, 20));
      QVERIFY(!monsterItem->equals(smallItem.data()));

      uint smallIndex = repository.index(TestItemRequest(*smallItem, true));
      uint monsterIndex = repository.index(TestItemRequest(*monsterItem, true));
      QVERIFY(monsterIndex != smallIndex);

      repository.deleteItem(smallIndex);
      QVERIFY(!repository.findIndex(TestItemRequest(*smallItem, true)));
      QCOMPARE(monsterIndex, repository.findIndex(TestItemRequest(*monsterItem, true)));
      repository.deleteItem(monsterIndex);

      // now in reverse order, with different data see: https://bugs.kde.org/show_bug.cgi?id=272408

      monsterItem.reset(createItem(hash + 1, KDevelop::ItemRepositoryBucketSize + 10));
      smallItem.reset(createItem(hash + 1, 20));
      QVERIFY(!monsterItem->equals(smallItem.data()));
      monsterIndex = repository.index(TestItemRequest(*monsterItem, true));
      smallIndex = repository.index(TestItemRequest(*smallItem, true));

      repository.deleteItem(monsterIndex);
      QCOMPARE(smallIndex, repository.findIndex(TestItemRequest(*smallItem, true)));
      QVERIFY(!repository.findIndex(TestItemRequest(*monsterItem, true)));
      repository.deleteItem(smallIndex);
    }
};

#include "test_itemrepository.moc"

QTEST_MAIN(TestItemRepository)
