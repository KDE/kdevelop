#include <QHash>
#include <QString>
int main()
{
    QHash<QString, QString> h;
    h[QString("10")] = QString("100");
    h[QString("20")] = QString("200");
    h[QString("30")] = QString("300");
    return 0;
}
