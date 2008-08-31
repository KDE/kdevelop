using NUnit.Framework;

[TestFixture]
public class MoneyTest {

    [Test]
    public void create() {
	Money m = new Money(5, "USD");
	Assert.AreEqual(5, m.amount());
	Assert.AreEqual("USD", m.currency());
    }

    [Test]
    public void negative() {
	try {
	    new Money(-5, "USD");
	    Assert.Fail("Should throw NegativeAmountException");
	} catch (NegativeAmountException e) {
	}
    }

    [Test]
    public void zero() {
	Money m = new Money(0, "USD");
	Assert.AreEqual(0, m.amount());
    }
}
