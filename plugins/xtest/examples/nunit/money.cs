using System;

public class NegativeAmountException : ApplicationException {
    public NegativeAmountException() :
	base("Negative amount does not make sense") {
    }
}

public class Money {
    private int m_amount;
    private string m_currency;

    public Money(int amount, string currency) {
	if (amount <= 0) {
	    throw new NegativeAmountException();
	}
	m_amount = amount;
	m_currency = currency;
    }

    public int amount() {
	return m_amount;
    }

    public string currency() {
	return m_currency;
    }
}
