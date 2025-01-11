#include <QDateTime>
#include <QTimeZone>
#include <QDebug>

int main()
{
    QDateTime dt(QDate(2010, 1, 20), QTime(15, 31, 13)); // Local
    qDebug() << "dt:" << dt;

    QDateTime utc = QDateTime::fromString("2024-02-01T16:28:07.123Z", Qt::ISODateWithMs); // UTC
    qDebug() << "utc:" << utc;

    QTimeZone newYork = QTimeZone::fromSecondsAheadOfUtc(-5*3600);
    qDebug() << "newYork:" << newYork;
    QDateTime lunchInNewYork(QDate(2025, 1, 2), QTime(12, 0, 0, 1), newYork);
    qDebug() << "lunchInNewYork:" << lunchInNewYork;

    QTimeZone hawaii("US/Hawaii");
    qDebug() << "hawaii:" << hawaii;
    QDateTime lunchInHawaii(QDate(2025, 1, 2), QTime(12, 0, 0), hawaii);
    qDebug() << "lunchInHawaii:" << lunchInHawaii;

    QTimeZone localTZ(QTimeZone::LocalTime);
    qDebug() << "localTZ:" << localTZ;
    QTimeZone utcTZ(QTimeZone::UTC);
    qDebug() << "utcTZ:" << utcTZ;

    QTimeZone invalidTZ("invalid");
    Q_ASSERT(!invalidTZ.isValid());
    qDebug() << "invalidTZ:" << invalidTZ;
    QDateTime dateTimeInvalidTZ(QDate(2025, 1, 2), QTime(12, 0, 0, 1), invalidTZ);
    qDebug() << dateTimeInvalidTZ;

    return 0;
}
// clazy:skip
