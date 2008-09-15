#include <QtTest/QTest>

class DataTestCase : public QObject
{
Q_OBJECT
private slots:
    void command() {
        QFETCH(bool, pass);
        QVERIFY2(pass, "fail");
    }

    void command_data() {
        QTest::addColumn<bool>("pass");

        QTest::newRow("row1") << true;
        QTest::newRow("row2") << false;
        QTest::newRow("row3") << true;
        QTest::newRow("row4") << false;
    }

};

QTEST_MAIN(DataTestCase)
#include "dataTest.moc"
