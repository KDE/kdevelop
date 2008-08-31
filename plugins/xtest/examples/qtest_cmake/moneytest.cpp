/*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
* 02110-1301, USA.
*/

#include "moneytest.h"
#include "money.h"

#include <QtTest/QtTest>
#include <QDebug>

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
