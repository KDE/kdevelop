#include "test_pool.h"
#include <limits.h>
#include <vector>

#include "memorypool.h"

QTEST_MAIN(TestPool)


PoolObject::PoolObject() { foo = 3; }
PoolObject::~PoolObject() { foo = 0; }

TestPool::TestPool()
{
}

void TestPool::initTestCase()
{
}

void TestPool::testSimpleAllocation()
{
    MemoryPool pool;
    int *p = pool.allocate<int>(3);
    p[0] = 10;
    p[1] = 3;
    p[2] = INT_MAX;
    QCOMPARE(p[0], 10);
    QCOMPARE(p[1], 3);
    QCOMPARE(p[2], INT_MAX);

    int *p2 = pool.allocate<int>(1);
    pool.construct(p2, 11);
    QCOMPARE(*p2, 11);
    pool.destroy(p);
    //nothing happens here (???)
    QCOMPARE(*p2, 11);
}

void TestPool::testObjectAllocation()
{
    MemoryPool pool;
    PoolObject *p = pool.allocate<PoolObject>(1);
    //object is not yet initialized (it's actually zeroed
    //because the block in the pool is zeroed
    QCOMPARE(p->foo, 0);
    pool.construct(p, PoolObject());
    //now we have the object there
    QCOMPARE(p->foo, 3);
    pool.destroy(p);
    //destructor was called, the "foo" field is zeroed again
    QCOMPARE(p->foo, 0);
}

void TestPool::testNewBlockAllocation()
{
    MemoryPool pool;
    int *p = pool.allocate<int>(MemoryPool::BLOCK_SIZE / sizeof(int));
    //the last one in a block
    int lastOne = MemoryPool::BLOCK_SIZE / sizeof(int) - 1;
    p[lastOne] = 10;
    //the first one in another block
    int *p2 = pool.allocate<int>();
    p2[0] = 11;
    QCOMPARE(p[lastOne], 10);
    QCOMPARE(p2[0], 11);
}

void TestPool::testWastedMemoryDueToBlockAllocation()
{
    MemoryPool alloc;
    //allocate a block and leave 2 last elements unallocated
    int *p = alloc.allocate<int>(MemoryPool::BLOCK_SIZE / sizeof(int) - 2);
    //the last one in a block
    int lastOne = MemoryPool::BLOCK_SIZE / sizeof(int) - 3;
    p[lastOne] = 10;
    //allocate 5 elements and watch that 2 elements in the previous block
    //are forgotten and a new block is created to allocate 5 elements continuously
    int *p2 = alloc.allocate<int>(5);
    p2[0] = 11;

    QCOMPARE(p[lastOne], 10);
    //those are the two forgotten elements from the first block
    QCOMPARE(p[lastOne+1], 0);
    QCOMPARE(p[lastOne+2], 0);
    //new block will not start immediatelly after the old one (???)
    QVERIFY((p + lastOne + 3) != p2);
    QCOMPARE(p2[0], 11);
}

void TestPool::benchManyAllocations()
{
  MemoryPool pool;
  QBENCHMARK {
    pool.allocate<char>(64);
  }
}

void TestPool::benchManyPools()
{
  QBENCHMARK {
    MemoryPool pool;
    for(int i = 0; i < 1000; ++i) {
      pool.allocate<char>(64);
    }
  }
}

#include "test_pool.moc"

