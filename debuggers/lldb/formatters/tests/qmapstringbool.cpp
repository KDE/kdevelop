#include <QMap>
#include <QString>
int main()
{
    QMap<QString, bool> m;
    m["10"] = true;
    m["20"] = false;
    m["30"] = true;
    return 0;
}
