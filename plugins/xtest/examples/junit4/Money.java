import java.lang.Exception;
import java.lang.String;

public class Money {
    public class NegativeAmountException extends Exception {}

    private int m_amount;
    private String m_currency;

    public Money(int amount, String currency) throws NegativeAmountException {
        if (amount <= 0) throw new NegativeAmountException();
        m_amount = amount;
        m_currency = currency;
    }

    public int amount() {
        return m_amount;
    }

    public String currency() {
        return m_currency;
    }
}
