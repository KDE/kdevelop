#include <QObject>
#include <QTest>

class KdeTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {}
    void passingTestCase()
    {
        QCOMPARE(1+1, 2);
    }

    void failingTestCase()
    {
        QCOMPARE(2+2, 5);
    }

    void expectedFailTestCase()
    {
        QEXPECT_FAIL("", "Apparently, everything Physics teachers told me was a lie", Continue);
        QCOMPARE(3*3, 10);
    }

    void unexpectedPassTestCase()
    {
        QEXPECT_FAIL("", "Well, not everything", Continue);
        QCOMPARE(5*2, 10);
    }

    void skippedTestCase()
    {
        QSKIP("Don't argue with calculators", SkipAll);
        QCOMPARE(2+2*2, 8);
    }

    void notATest(int par)
    {
    }

    void cleanupTestCase();
};

class NotTheTestClass : public QObject
{
    Q_OBJECT
public:
    KdeTest* test;
private Q_SLOTS:
    void helper();
};

int main()
{
    NotTheTestClass help;
    auto& test = help.test;
    bool otherVariable;
    int r = QTest::qExec(test);
    return r;
}
