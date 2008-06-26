#ifndef MONEYTEST_H
#define MONEYTEST_H

#include <cppunit/TestFixture.h>

class MoneyTest : public CppUnit::TestFixture
{
public:
    void testCreate();
    void testNegative();
    void testZero();

    static CppUnit::Test* suite();
};

#endif // MONEYTEST_H
