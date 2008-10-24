#include <QtTest/QTest>

bool predicate() { return false; }

class MultipleFailuresCase : public QObject
{
Q_OBJECT
private slots:
    void command() {
        fail1();
        fail2();
    }
private:
    void fail1() {
        QVERIFY2(predicate(), "fail1");
    }
    void fail2() {
        QVERIFY2(predicate(), "fail2");
    }
};

QTEST_MAIN(MultipleFailuresCase)
#include "multipleFailuresSingleCommand.moc"
