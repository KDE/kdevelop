#ifndef MONEY_H
#define MONEY_H

#include <QString>

class Money
{
public:
    enum State { Good, Bad };
    Money(int amount, QString currency)
        : m_amount(amount), m_currency(currency){
        m_state = (amount > 0) ? Good : Bad;
    }
    int amount() const { return m_amount; }
    QString currency() const { return m_currency; }
    State state() const { return m_state; }
private:
    int m_amount;
    QString m_currency;
    State m_state;
};

#endif /* MONEY_H */
