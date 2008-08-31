import java.lang.Exception;
import org.junit.Test;
import static org.junit.Assert.assertEquals;

public class MoneyTest {
    @Test
    public void create() throws Exception {
        Money m = new Money(5, "USD");
        assertEquals(5, m.amount());
        assertEquals("USD", m.currency());
    }

    @Test(expected = Money.NegativeAmountException.class)
    public void negative() throws Exception {
         new Money(-5, "FOO");
    }

    @Test
    public void zero() throws Exception {
        Money m = new Money(0, "USD");
        assertEquals(0, m.amount());
    }
}
