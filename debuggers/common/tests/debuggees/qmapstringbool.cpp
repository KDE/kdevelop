#include <QMap>
#include <QString>
int main()
{
    QMap<QString, bool> m;
    m[QStringLiteral("10")] = true;
    m[QStringLiteral("20")] = false;
    m[QStringLiteral("30")] = true;
    return 0;
}
