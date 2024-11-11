#include <QCoreApplication>
#include <QCborMap>
#include <QCborArray>
#include <QCborValue>
#include <QDebug>
#include <QBuffer>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCborMap cborMap;
    QString nameStr = QStringLiteral("name");
    cborMap[nameStr] = "John Doe";
    cborMap[QStringLiteral("address")] = "Some street\nCity\nCountry";
    QCborValue thirty(30);
    cborMap[QStringLiteral("age")] = thirty;
    QCborValue notMarried(false);
    cborMap[QStringLiteral("married")] = notMarried;
    cborMap[QStringLiteral("undefined")] = QCborValue();
    cborMap[QStringLiteral("null")] = QCborValue(nullptr);
    cborMap[QStringLiteral("url")] = QCborValue(QUrl("http://www.kde.org"));
    cborMap[QStringLiteral("regexp")] = QCborValue(QRegularExpression(QStringLiteral("^kde$")));
    cborMap[QStringLiteral("birth")] = QCborValue(QDateTime(QDate(2001, 5, 30), QTime(9, 31, 0)));

    QCborMap childMap;
    childMap[QStringLiteral("company")] = "KDAB";
    childMap[QStringLiteral("title")] = "Surface technician";
    childMap[QStringLiteral("emptyObj")] = QCborMap();
    childMap[QStringLiteral("emptyArray")] = QCborArray();
    cborMap[QStringLiteral("job")] = childMap;

    QCborArray childrenArray;
    childrenArray.append("Alice");
    childrenArray.append("Mickaël");
    cborMap[QStringLiteral("children")] = childrenArray;

    QCborValue cborValue(cborMap);

    QByteArray cborData = cborValue.toCbor();

    // Deserialize the CBOR data back into a QCborValue
    QCborValue parsedValue = QCborValue::fromCbor(cborData);

    if (parsedValue.isMap()) {
        QCborMap parsedMap = parsedValue.toMap();

        auto nameRef = parsedMap[nameStr];
        QCborValue name = nameRef;

        auto ageRef = parsedMap[QStringLiteral("age")];
        QCborValue age = ageRef;

        auto marriedRef = parsedMap[QStringLiteral("married")];
        QCborValue married = marriedRef;

        auto childrenValue = parsedMap[QStringLiteral("children")];
        QCborArray parsedChildren = childrenValue.toArray();

        for (const auto &child : parsedChildren) {
            QString childName = child.toString();
        }
    } else {
        qCritical() << "Failed to parse CBOR data.";
        return 1;
    }

    return 0;
}
