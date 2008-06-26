#include <kdevelop/veritas/cppunitwrapper.h>

#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>

#include "money.h"
#include "moneytest.h"

using namespace std;
using namespace CppUnit;

void MoneyTest::testCreate()
{
    Money m(5, "USD");
    CPPUNIT_ASSERT_EQUAL(5, m.amount());
    CPPUNIT_ASSERT_EQUAL(string("USD"), m.currency());
}

void MoneyTest::testNegative()
{
    CPPUNIT_ASSERT_THROW(Money(-5,"USD"), NegativeMoneyException);
}

void MoneyTest::testZero()
{
    Money m(0, "USD");
    CPPUNIT_ASSERT_EQUAL(0, m.amount());
}

Test* MoneyTest::suite()
{
    TestSuite* s = new TestSuite("MoneyTest");
    typedef TestCaller<MoneyTest> mc;
    s->addTest(new mc("testCreate", &MoneyTest::testCreate));
    s->addTest(new mc("testNegative", &MoneyTest::testNegative));
    s->addTest(new mc("testZero", &MoneyTest::testZero));
    TestSuite* r = new TestSuite("Root");
    r->addTest(s);
    return r;
}

CPPUNIT_XTEST_MAIN( MoneyTest::suite() )
