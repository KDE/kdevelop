#include <QMultiHash>
#include <QString>
int main()
{
    QMultiHash<QString, QString> h;
    h["10"] = "100";
    h["20"] = "200";
    h["30"] = "300";

    h.insert("20", "x");
    h.insert("20", "11");
    h.remove("20", "x");

    return 0;
}
