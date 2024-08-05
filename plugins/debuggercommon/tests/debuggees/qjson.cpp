#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

int main()
{
    QJsonParseError error;
    auto document = QJsonDocument::fromJson(QByteArrayLiteral("{\"key\": [42, 4.2, true, \"foobar\", null]}"), &error);
    auto object = document.object();

    auto objectValue = QJsonValue(object);
    auto arrayValue = object.value(QLatin1String("key"));

    auto array = arrayValue.toArray();

    auto v1 = array[0]; // int
    auto v2 = array[1]; // double
    auto v3 = array[2]; // bool
    auto v4 = array[3]; // str
    auto v5 = array[4]; // null
    auto v6 = array[5]; // undefined

    // prevent compiler optimizing away unused object1
    Q_UNUSED(v1);
    Q_UNUSED(v2);
    Q_UNUSED(v3);
    Q_UNUSED(v4);
    Q_UNUSED(v5);
    Q_UNUSED(v6);

    return 0;
}
