#include <QMultiMap>
#include <QString>
int main()
{
    QMultiMap<QString, QString> m;
    m.insert("10", "100");
    m.insert("20", "200");
    m.insert("30", "300");

    m.insert("20", "x");
    m.insert("20", "11");
    m.remove("20", "x");

    return 0;
}
