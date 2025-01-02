#include <QDateTime>
#include <QTimeZone>
#include <QDebug>
int main()
{
    QDateTime dt(QDate(2010, 1, 20), QTime(15, 31, 13)); // Local

    QDateTime utc = QDateTime::fromString("2024-02-01T16:28:07.123Z", Qt::ISODateWithMs); // UTC

    QTimeZone newYork = QTimeZone::fromSecondsAheadOfUtc(-5*3600);
    QDateTime lunchInNewYork(QDate(2025, 1, 2), QTime(12, 0, 0), newYork);

    QTimeZone hawaii("US/Hawaii");
    QDateTime lunchInHawaii(QDate(2025, 1, 2), QTime(12, 0, 0), hawaii);

    QTimeZone localTZ(QTimeZone::LocalTime);
    QTimeZone utcTZ(QTimeZone::UTC);

    return 0;
}
// clazy:skip
