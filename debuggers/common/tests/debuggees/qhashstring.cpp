#include <QHash>
#include <QString>
int main()
{
    QHash<QString, QString> h;
    h[QStringLiteral("10")] = QStringLiteral("100");
    h[QStringLiteral("20")] = QStringLiteral("200");
    h[QStringLiteral("30")] = QStringLiteral("300");
    return 0;
}
