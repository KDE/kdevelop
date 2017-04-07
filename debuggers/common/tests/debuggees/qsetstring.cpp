#include <QSet>
#include <QString>
int main()
{
    QSet<QString> s;
    s.insert(QStringLiteral("10"));
    s.insert(QStringLiteral("20"));
    s.insert(QStringLiteral("30"));
    return 0;
}
