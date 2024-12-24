#include <QCoreApplication>
#include <QCborMap>
#include <QCborArray>
#include <QCborValue>
#include <QDebug>
#include <QBuffer>
#include <QUuid>
#include <QTimeZone>

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
        QCborValue urlValue(QUrl("http://www.kde.org"));
        cborMap[QStringLiteral("url")] = urlValue;
        QUuid uuid("{67C8770B-44F1-410A-AB9A-F9B5446F13EE}");
        QCborValue uuidValue(uuid);
        cborMap[QStringLiteral("uuid")] = uuidValue;
        cborMap[QStringLiteral("regexp")] = QCborValue(QRegularExpression(QStringLiteral("^kde$")));
        cborMap[QStringLiteral("birth")] = QCborValue(QDateTime(QDate(2001, 5, 30), QTime(9, 31, 0), QTimeZone(3600)));
        cborMap[QStringLiteral("bytes")] = QByteArray("ABC\xFF\x00\xFE", 6);
        QCborValue otherSimpleType(QCborSimpleType(12));
        cborMap[QStringLiteral("otherSimpleType")] = otherSimpleType;

        QCborMap childMap;
        childMap[QStringLiteral("company")] = "KDAB";
        childMap[QStringLiteral("title")] = "Surface technician";
        childMap[QStringLiteral("emptyObj")] = QCborMap();
        childMap[QStringLiteral("emptyArray")] = QCborArray();
        // The byte value 0x01 followed by 8 zero bytes represents the number 2^64 (example from Qt API docs)
        QCborValue bigNum(QCborTag(QCborKnownTags::PositiveBignum), QByteArray("\x01\0\0\0\0\0\0\0\0", 9));
        childMap[QStringLiteral("bigNum")] = bigNum;
        QCborMap taggedMap;
        taggedMap[QStringLiteral("planet")] = QString("earth");
        QCborValue tagWithMap(QCborTag(42), taggedMap);
        childMap[QStringLiteral("tagWithMap")] = tagWithMap;

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
    // Clear the very long string after use to prevent overly wide Value column in
    // KDevelop's Variables tool view, which moves the Type column far to the right.
    cborData.clear();

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

        auto childMap = parsedMap[QStringLiteral("job")].toMap();
        auto bigNumValueRef = childMap[QStringLiteral("bigNum")];
        QCborValue bigNumValue = bigNumValueRef;

        for (const auto &child : parsedChildren) {
            QString childName = child.toString();
        }
    } else {
        qCritical() << "Failed to parse CBOR data.";
        return 1;
    }

    return 0;
}
