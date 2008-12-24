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

#ifndef MONEY_H
#define MONEY_H

#include <QString>

class Money
{
public:
    enum State { Good, Bad };

    Money()
        : m_amount(0), m_currency("USD") {}
    Money(int amount, QString currency)
        : m_amount(amount), m_currency(currency){
        m_state = (amount > 0) ? Good : Bad;
    }
    int amount() const {
        return m_amount;
    }
    QString currency() const {
        return m_currency;
    }
    State state() const {
        return m_state;
    }
    bool operator==(const Money& other) const {
        return (other.amount() == amount()) && 
               (other.currency() == currency());
    }

private:
    int m_amount;
    QString m_currency;
    State m_state;
};

#endif /* MONEY_H */
