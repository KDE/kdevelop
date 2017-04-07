#include <QMap>
#include <QString>
int main()
{
    QMap<QString, QString> m;
    m[QStringLiteral("10")] = QLatin1String("100");
    m[QStringLiteral("20")] = QLatin1String("200");
    m[QStringLiteral("30")] = QLatin1String("300");
    return 0;
}
