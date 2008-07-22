#include <QtTest/QtTest>
#include <QDebug>

#include "money.h"
#include "moneytest.h"

void MoneyTest::create()
{
    Money m(5, "USD");
    QCOMPARE(Money::Good, m.state());
    QCOMPARE(5, m.amount());
    QVERIFY("USD" == m.currency());
}

void MoneyTest::negative()
{
    Money m(-5, "USD");
    QCOMPARE(Money::Bad, m.state());
}

void MoneyTest::zero()
{
    qDebug() << "supposed to fail";
    qDebug() << "more garbage output";
    Money m(0, "USD");
    QCOMPARE(Money::Good, m.state());
    QCOMPARE(0, m.amount());
}

QTEST_MAIN( MoneyTest )
#include "moneytest.moc"
