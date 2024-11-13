#include <QCoreApplication>
#include <QCborMap>
#include <QCborArray>
#include <QCborValue>
#include <QDebug>
#include <QBuffer>

class Source
{
public:
    Source() {
        QCborMap cborMap;
        cborMap[QStringLiteral("name")] = "John Doe";
        cborMap[QStringLiteral("address")] = "Some street\nCity\nCountry";
        cborMap[QStringLiteral("year")] = 2024;
        QCborValue age(30.57);
        cborMap[QStringLiteral("age")] = age;
        QCborValue notMarried(false);
        cborMap[QStringLiteral("married")] = notMarried;
        cborMap[QStringLiteral("undefined")] = QCborValue();
        cborMap[QStringLiteral("null")] = QCborValue(nullptr);
        cborMap[QStringLiteral("url")] = QCborValue(QUrl("http://www.kde.org"));
        cborMap[QStringLiteral("regexp")] = QCborValue(QRegularExpression(QStringLiteral("^kde$")));
        cborMap[QStringLiteral("birth")] = QCborValue(QDateTime(QDate(2001, 5, 30), QTime(9, 31, 0)));
        cborMap[QStringLiteral("bytes")] = QByteArray("ABC\xFF\x00\xFE", 6);

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

        m_document = cborMap;
    }

    Q_NEVER_INLINE QCborValue documentValue();
    Q_NEVER_INLINE QCborMap mainMap();
    Q_NEVER_INLINE QCborArray childrenArray();

private:
    QCborValue m_document;
};

QCborValue Source::documentValue() { return m_document; }
QCborMap Source::mainMap() { return m_document.toMap(); }
QCborArray Source::childrenArray() { return m_document[QStringLiteral("children")].toArray(); }

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCborValue emptyValue;
    Source source;
    QCborValue cborValue = source.documentValue();
    QCborMap cborMap = source.mainMap();
    QByteArray cborData = cborValue.toCbor();

    // Deserialize the CBOR data back into a QCborValue
    QCborValue parsedValue = QCborValue::fromCbor(cborData);

    if (parsedValue.isMap()) {
        QCborMap parsedMap = parsedValue.toMap();

        QString nameStr = QStringLiteral("name");
        auto nameRef = parsedMap[nameStr];
        QCborValue name = nameRef;

        auto yearRef = parsedMap[QStringLiteral("year")];
        QCborValue year = yearRef;

        auto ageRef = parsedMap[QStringLiteral("age")];
        QCborValue age = ageRef;

        auto marriedRef = parsedMap[QStringLiteral("married")];
        QCborValue married = marriedRef;

        auto childrenValue = parsedMap[QStringLiteral("children")];
        QCborArray parsedChildren = childrenValue.toArray();

        auto bytesValueRef = parsedMap[QStringLiteral("bytes")];
        QCborValue bytesValue = bytesValueRef;

        for (const auto &child : parsedChildren) {
            QString childName = child.toString();
        }
    } else {
        qCritical() << "Failed to parse CBOR data.";
        return 1;
    }

    return 0;
}
