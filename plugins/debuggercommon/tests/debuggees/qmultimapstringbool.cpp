#include <QMultiMap>
#include <QString>
int main()
{
    QMultiMap<QString, bool> m;
    m.insert("10", true);
    m.insert("20", false);
    m.insert("30", true);
    return 0;
}
