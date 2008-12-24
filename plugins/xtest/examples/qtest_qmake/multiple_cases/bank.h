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

/*! Trivial example class intended to showcase the KDevelop4 xTest runner */

#ifndef BANK_H
#define BANK_H

#include "money.h"
#include <QMap>

class Bank
{
public:
    void deposit(int clientID, const Money& amount) {
        if (!m_money4client.contains(clientID)) {
            m_money4client[clientID] = amount;
        } else {
            Money& current = m_money4client[clientID];
            Money sum(current.amount()+amount.amount(), current.currency());
            m_money4client[clientID] = sum;
        }
    }
    int numberOfClients() const {
        return m_money4client.size();
    }
    Money accountBalance(int clientID) {
        return m_money4client[clientID];
    }
private:
    QMap<int,Money> m_money4client;
};

#endif /* BANK_H */
