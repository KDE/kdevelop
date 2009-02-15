#include <QtTest/QTest>

class QAssertCase : public QObject
{
Q_OBJECT
private slots:
    void command() {
        Q_ASSERT(0);
    }
};

QTEST_MAIN(QAssertCase)
#include "qassert.moc"

