#include <QtTest/QTest>

class SkipCase : public QObject
{
Q_OBJECT
private slots:
    void command() {
        QSKIP("skipCommand", SkipSingle);
    }

    void commandData() {
        QFETCH(QString, foo);
        QSKIP("skipDataCommand", SkipAll);
    }

    void commandData_data() {
        QTest::addColumn<QString>("foo");
        QTest::newRow("row1") << "moo";
        QTest::newRow("row2") << "zoo";
    }
};

QTEST_MAIN(SkipCase)
#include "skip.moc"
