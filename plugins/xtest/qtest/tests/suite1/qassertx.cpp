#include <QtTest/QTest>

class QAssertXCase : public QObject
{
Q_OBJECT
private slots:
    void command() {
        Q_ASSERT_X(0, "command", "message");
    }
};

QTEST_MAIN(QAssertXCase)
#include "qassertx.moc"

