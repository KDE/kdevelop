#include "test.h"

void KdeTest::initTestCase()
{

}

void KdeTest::passingTestCase()
{
    QCOMPARE(1+1, 2);
}

void KdeTest::failingTestCase()
{
    QCOMPARE(2+2, 5);
}

void KdeTest::cleanupTestCase()
{
    
}

QTEST_KDEMAIN( KdeTest, NoGUI )
