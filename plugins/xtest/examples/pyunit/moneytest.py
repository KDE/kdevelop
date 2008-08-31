from unittest  import TestCase, makeSuite
from xmlrunner import XMLTestRunner
from money     import Money, NegativeAmountException

class MoneyTest(TestCase):
    def testCreate(self):
        m = Money(5, "USD")
        self.assertEqual(5, m.amount())
        self.assertEqual("USD", m.currency())

    def testNegative(self):
        self.assertRaises(NegativeAmountException, Money, -5, "FOO")

    def testZero(self):
        m = Money(0, "USD")
        self.assertEqual(0, m.amount())

if __name__ == "__main__":
    suite = makeSuite(MoneyTest)
    XMLTestRunner().run(suite)
