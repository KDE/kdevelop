#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QJsonObject jsonObj;
    QString nameStr = "name";
    jsonObj[nameStr] = "John Doe";
    jsonObj["address"] = "Some street\\nCity\\nCountry";
    jsonObj["age"] = 30;
    jsonObj["married"] = false;

    QJsonObject childObj;
    childObj["company"] = "KDAB";
    childObj["title"] = "Surface technician";
    childObj["emptyObj"] = QJsonObject();
    childObj["emptyArray"] = QJsonArray();
    jsonObj["job"] = childObj;

    QJsonArray children;
    children.append("Alice");
    children.append("Mickaël");
    jsonObj["children"] = children;

    QJsonDocument jsonDoc(jsonObj);

    QJsonDocument emptyDoc;

    QString jsonString = jsonDoc.toJson(QJsonDocument::Indented);

    // Parsing JSON string back to QJsonDocument
    QJsonDocument parsedDoc = QJsonDocument::fromJson(jsonString.toUtf8());

    // Check if the parsing was successful
    if (!parsedDoc.isNull() && parsedDoc.isObject()) {
        QJsonObject parsedObj = parsedDoc.object();

        // Output parsed JSON object
        const auto nameRef = parsedObj[nameStr];
        const QJsonValue name = nameRef;
        const auto ageRef = parsedObj["age"];
        const QJsonValue age = ageRef;
        const auto marriedRef = parsedObj["married"];
        const QJsonValue married = marriedRef;

        const auto parsedChildrenRef = parsedObj["children"];
        const QJsonValue parsedChildrenValue = parsedChildrenRef;
        QJsonArray parsedChildren = parsedChildrenRef.toArray();
        for (const auto &child : parsedChildren) {
            QString childName = child.toString();
        }
    } else {
        qCritical() << "Failed to parse JSON string.";
        return 1;
    }

    return 0;
}
