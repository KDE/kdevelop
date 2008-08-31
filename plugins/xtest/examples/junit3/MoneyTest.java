import junit.framework.Test;
import junit.framework.TestCase;
import junit.framework.TestSuite;
import java.lang.Exception;

public class MoneyTest extends TestCase {
    public void testCreate() throws Exception {
	Money m = new Money(5, "USD");
	assertEquals(5, m.amount());
	assertEquals("USD", m.currency());
    }

    public void testNegative() {
	try {
	    new Money(-5, "FOO");
	    fail("Should throw NegativeAmountException");
	} catch (Money.NegativeAmountException e) {}
    }

    public void testZero() throws Exception {
	Money m = new Money(0, "USD");
	assertEquals(0, m.amount());
    }

    public static Test suite() {
	return new TestSuite(MoneyTest.class);
    }
}
