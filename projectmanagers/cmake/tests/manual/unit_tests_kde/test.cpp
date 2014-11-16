#include "test.h"

#include <QtTest>

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

void KdeTest::expectedFailTestCase()
{
    QEXPECT_FAIL("", "Apparently, everything Physics teachers told me was a lie", Continue);
    QCOMPARE(3*3, 10);
}

void KdeTest::unexpectedPassTestCase()
{
    QEXPECT_FAIL("", "Well, not everything", Continue);
    QCOMPARE(5*2, 10);
}

void KdeTest::skippedTestCase()
{
    QSKIP("Don't argue with calculators", SkipAll);
    QCOMPARE(2+2*2, 8);
}


void KdeTest::cleanupTestCase()
{
    
}

QTEST_KDEMAIN( KdeTest, NoGUI )
