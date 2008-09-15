#include <QtTest/QTest>

class ExpectedFailureCase : public QObject
{
Q_OBJECT
private slots:
    void command() {
        QEXPECT_FAIL("", "expectedFailure", Abort);
        QVERIFY(0);
    }

    void commandData() {
        QFETCH(bool, pass);

        QEXPECT_FAIL("row2", "expectedFailureData", Continue);
        QVERIFY2(pass, "fail");

        QEXPECT_FAIL("row3", "expectedFailureData", Continue);
        QVERIFY(pass);
    }

    void commandData_data() {
        QTest::addColumn<bool>("pass");

        QTest::newRow("row1") << true;  // normal pass
        QTest::newRow("row2") << false; // xfail
        QTest::newRow("row3") << true;  // xpass
        QTest::newRow("row4") << false; // normal fail
    }
};

QTEST_MAIN(ExpectedFailureCase)
#include "expectedFailure.moc"
