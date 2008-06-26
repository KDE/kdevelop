#ifndef MONEY_H
#define MONEY_H

#include <string>
#include <exception>

class NegativeMoneyException : public std::exception
{
public:
    virtual const char* what() const throw() {
        return "Negative amount does not make sense.";
    }
};

class Money
{
public:
    Money(int amount, std::string currency)
        : m_amount(amount), m_currency(currency) {
        if (m_amount <= 0)
            throw NegativeMoneyException();
    }
    int amount() const { return m_amount; }
    std::string currency() const { return m_currency; }
private:
    int m_amount;
    std::string m_currency;
};

#endif /* MONEY_H */
