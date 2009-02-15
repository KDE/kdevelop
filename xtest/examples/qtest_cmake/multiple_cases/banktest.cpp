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

#include "banktest.h"
#include "bank.h"

#include <QtTest/QtTest>

void BankTest::emptyBankHasZeroClients()
{
    Bank b;
    QCOMPARE(0, b.numberOfClients());
}

void BankTest::depositMoney()
{
    Bank b;
    int clientID = 10;
    Money m(5, "USD");

    b.deposit(clientID, m);
    QCOMPARE(Money(5, "USD"), b.accountBalance(clientID));
    QCOMPARE(1, b.numberOfClients());
}

void BankTest::depositTwice()
{
    Bank b;
    int clientID = 10;
    Money m(5, "USD");
    b.deposit(clientID, m);
    Money m2(7, "USD");
    b.deposit(clientID, m2);

    QCOMPARE(Money(5+7, "USD"), b.accountBalance(clientID));
    QCOMPARE(1, b.numberOfClients());
}

QTEST_MAIN( BankTest )
#include "banktest.moc"
