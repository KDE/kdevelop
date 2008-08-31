/*! Trivial example class intended to showcase the KDevelop4 xTest runner */

#ifndef MONEY_H
#define MONEY_H

#include <string>

class Money
{
public:
    Money(int amount, std::string currency) :
        m_amount(amount), m_currency(currency) {}

    int amount() const { return m_amount; }
    std::string currency() const { return m_currency; }
    bool isValid() { return m_amount > 0; }
private:
    int m_amount;
    std::string m_currency;
};

#endif /* MONEY_H */
