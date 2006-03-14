#include <QtTest/QtTest>
#include <limits.h>
#include <vector>

#include "memorypool.h"

class PoolObject {
public:
    PoolObject() { foo = 3; }
    ~PoolObject() { foo = 0; }

    int foo;
};

class TestPool: public QObject {
    Q_OBJECT

public:
    TestPool()
    {
    }

private slots:
    void initTestCase()
    {
    }

    void testSimpleAllocation()
    {
        rxx_allocator<int> alloc;
        int *p = alloc.allocate(3);
        p[0] = 10;
        p[1] = 3;
        p[2] = INT_MAX;
        QCOMPARE(p[0], 10);
        QCOMPARE(p[1], 3);
        QCOMPARE(p[2], INT_MAX);

        int *p2 = alloc.allocate(1);
        alloc.construct(p2, 11);
        QCOMPARE(*p2, 11);
        alloc.destroy(p);
        //nothing happens here (???)
        QCOMPARE(*p2, 11);
    }

    void testObjectAllocation()
    {
        rxx_allocator<PoolObject> alloc;
        PoolObject *p = alloc.allocate(1);
        //object is not yet initialized (it's actually zeroed
        //because the block in the pool is zeroed
        QCOMPARE(p->foo, 0);
        alloc.construct(p, PoolObject());
        //now we have the object there
        QCOMPARE(p->foo, 3);
        alloc.destroy(p);
        //destructor was called, the "foo" field is zeroed again
        QCOMPARE(p->foo, 0);
    }

    void testNewBlockAllocation()
    {
        rxx_allocator<int> alloc;
        int *p = alloc.allocate(alloc._S_block_size / sizeof(int));
        //the last one in a block
        int lastOne = alloc._S_block_size / sizeof(int) - 1;
        p[lastOne] = 10;
        //the first one in another block
        alloc.allocate(1);
        p[lastOne+1] = 11;
        QCOMPARE(p[lastOne], 10);
        QCOMPARE(p[lastOne+1], 11);
    }

    void testWastedMemoryDueToBlockAllocation()
    {
        rxx_allocator<int> alloc;
        //allocate a block and leave 2 last elements unallocated
        int *p = alloc.allocate(alloc._S_block_size / sizeof(int) - 2);
        //the last one in a block
        int lastOne = alloc._S_block_size / sizeof(int) - 3;
        p[lastOne] = 10;
        //allocate 5 elements and watch that 2 elements in the previous block
        //are forgotten and a new block is created to allocate 5 elements continuously
        int *p2 = alloc.allocate(5);
        p2[0] = 11;

        QCOMPARE(p[lastOne], 10);
        //those are the two forgotten elements from the first block
        QCOMPARE(p[lastOne+1], 0);
        QCOMPARE(p[lastOne+2], 0);
        //new block will not start immediatelly after the old one (???)
        QVERIFY((p + lastOne + 3) != p2);
        QCOMPARE(p2[0], 11);
    }

    void testStdlibCompliance()
    {
        std::vector<int, rxx_allocator<int> > v;
        v.push_back(5);
        v.push_back(55);
        v.push_back(555);
        QCOMPARE(v[0], 5);
        QCOMPARE(v[1], 55);
        QCOMPARE(v[2], 555);
        v.pop_back();
        QCOMPARE(v[0], 5);
        QCOMPARE(v[1], 55);
        QCOMPARE(v.size(), size_t(2));
        v.push_back(10);
        QCOMPARE(v[2], 10);
    }

};

#include "test_pool.moc"

QTEST_MAIN(TestPool)
