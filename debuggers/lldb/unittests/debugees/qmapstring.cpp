#include <QMap>
#include <QString>
int main()
{
    QMap<QString, QString> m;
    m[QStringLiteral("10")] = QStringLiteral("100");
    m[QStringLiteral("20")] = QStringLiteral("200");
    m[QStringLiteral("30")] = QStringLiteral("300");
    return 0;
}
