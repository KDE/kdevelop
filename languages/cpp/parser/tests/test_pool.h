#ifndef TEST_POOL_H
#define TEST_POOL_H
#include <QtTest/QtTest>
#include <QtCore/QObject>

class PoolObject {
public:
    PoolObject();
    ~PoolObject();

    int foo;
};

class TestPool: public QObject {
    Q_OBJECT

public:
    TestPool();

private slots:
    void initTestCase();

    void testSimpleAllocation();
    void testObjectAllocation();

    void testNewBlockAllocation();

    void testWastedMemoryDueToBlockAllocation();

    void testStdlibCompliance();

};

#endif

