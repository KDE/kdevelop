#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

class Source
{
public:
    Source() {
        QJsonObject jsonObj;
        jsonObj["name"] = "John Doe";
        jsonObj["address"] = "Some street\\nCity\\nCountry";
        jsonObj["year"] = 2024;
        jsonObj["age"] = 30.57;
        jsonObj["married"] = false;

        QJsonObject childObj;
        childObj["company"] = "KDAB";
        childObj["title"] = "Surface technician";
        childObj["emptyObj"] = QJsonObject();
        childObj["emptyArray"] = QJsonArray();
        childObj["emptyValue"] = QJsonValue();
        jsonObj["job"] = childObj;

        QJsonArray children;
        children.append("Alice");
        children.append("Mickaël");
        jsonObj["children"] = children;

        m_jsonDoc = QJsonDocument(jsonObj);
    }

    Q_NEVER_INLINE QJsonDocument jsonDocument();
    Q_NEVER_INLINE QJsonObject jsonObject();
    Q_NEVER_INLINE QJsonValue jsonValue();
    Q_NEVER_INLINE QJsonArray childrenArray();

private:
    QJsonDocument m_jsonDoc;
};

QJsonDocument Source::jsonDocument() { return m_jsonDoc; }
QJsonObject Source::jsonObject() { return m_jsonDoc.object(); }
QJsonValue Source::jsonValue() { return m_jsonDoc.object(); }
QJsonArray Source::childrenArray() { return m_jsonDoc.object()["children"].toArray(); }

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QJsonDocument emptyDoc;
    Source source;
    QJsonDocument jsonDoc = source.jsonDocument();
    QJsonObject jsonObj = jsonDoc.object();

    QByteArray jsonString = jsonDoc.toJson(QJsonDocument::Indented);

    // Parsing JSON string back to QJsonDocument
    QJsonDocument parsedDoc = QJsonDocument::fromJson(jsonString);
    // Clear the very long string after use to prevent overly wide Value column in
    // KDevelop's Variables tool view, which moves the Type column far to the right.
    jsonString.clear();

    // Check if the parsing was successful
    if (!parsedDoc.isNull() && parsedDoc.isObject()) {
        QJsonObject parsedObj = parsedDoc.object();

        // Output parsed JSON object
        QString nameStr = "name";
        const auto nameRef = parsedObj[nameStr];
        const QJsonValue name = nameRef;
        const auto yearRef = parsedObj["year"];
        const QJsonValue year = yearRef;
        const auto ageRef = parsedObj["age"];
        const QJsonValue age = ageRef;
        const auto marriedRef = parsedObj["married"];
        const QJsonValue married = marriedRef;

        const auto parsedChildrenRef = parsedObj["children"];
        const QJsonValue parsedChildrenValue = parsedChildrenRef;
        QJsonArray parsedChildren = parsedChildrenRef.toArray();
        const QJsonDocument childrenDoc(parsedChildren);
        for (const auto &child : parsedChildren) {
            QString childName = child.toString();
        }
    } else {
        qCritical() << "Failed to parse JSON string.";
        return 1;
    }

    return 0;
}
