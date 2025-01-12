#include <QDateTime>
#include <QDebug>
#include <QTimeZone>

int main()
{
    QDateTime local(QDate(2010, 1, 20), QTime(15, 31, 13));
    qDebug() << "local:" << local;

    QDateTime utc = QDateTime::fromString("2024-02-01T16:28:07.123Z", Qt::ISODateWithMs);
    qDebug() << "utc:" << utc;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QTimeZone localTZ(QTimeZone::LocalTime);
    qDebug() << "localTZ:" << localTZ;
    QTimeZone utcTZ(QTimeZone::UTC);
    qDebug() << "utcTZ:" << utcTZ;

    QTimeZone newYork = QTimeZone::fromSecondsAheadOfUtc(-5*3600);
    qDebug() << "newYork:" << newYork;
    QDateTime lunchInNewYork(QDate(2025, 1, 2), QTime(12, 0, 0, 1), newYork);
    qDebug() << "lunchInNewYork:" << lunchInNewYork;

    QTimeZone utc0 = QTimeZone::fromSecondsAheadOfUtc(0);
    qDebug() << "utc0:" << utc0;
    QDateTime lunchInUtc0(QDate(2025, 1, 2), QTime(12, 0, 0, 999), utc0);
    qDebug() << "lunchInUtc0:" << lunchInUtc0;

    QTimeZone utc20 = QTimeZone::fromSecondsAheadOfUtc(20*3600);
    // From the documentation: "The offset from UTC must be in the range
    // -16 hours to +16 hours otherwise an invalid time zone will be returned."
    Q_ASSERT(!utc20.isValid());
    qDebug() << "utc20:" << utc20;
    QDateTime lunchInUtc20(QDate(2025, 1, 2), QTime(12, 0, 0, 505), utc20);
    qDebug() << "lunchInUtc20:" << lunchInUtc20;
#endif
    QTimeZone hawaii("US/Hawaii");
    qDebug() << "hawaii:" << hawaii;
    QDateTime lunchInHawaii(QDate(2025, 1, 2), QTime(12, 0, 0), hawaii);
    qDebug() << "lunchInHawaii:" << lunchInHawaii;

    QTimeZone defaultConstructedTZ;
    Q_ASSERT(!defaultConstructedTZ.isValid());
    qDebug() << "defaultConstructedTZ:" << defaultConstructedTZ;
    QDateTime dateTimeDefaultConstructedTZ(QDate(1010, 11, 23), QTime(5, 6, 7, 89), defaultConstructedTZ);
    qDebug() << dateTimeDefaultConstructedTZ;

    QTimeZone emptyStringTZ;
    Q_ASSERT(!emptyStringTZ.isValid());
    qDebug() << "emptyStringTZ:" << emptyStringTZ;
    QDateTime dateTimeEmptyStringTZ(QDate(1510, 11, 23), QTime(5, 6, 7, 89), emptyStringTZ);
    qDebug() << dateTimeEmptyStringTZ;

    QTimeZone invalidTZ("invalid");
    Q_ASSERT(!invalidTZ.isValid());
    qDebug() << "invalidTZ:" << invalidTZ;
    QDateTime dateTimeInvalidTZ(QDate(2025, 1, 2), QTime(12, 0, 0, 1), invalidTZ);
    qDebug() << dateTimeInvalidTZ;

    return 0;
}
// clazy:skip
