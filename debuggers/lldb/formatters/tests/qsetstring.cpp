#include <QSet>
#include <QString>
int main()
{
    QSet<QString> s;
    s.insert(QString("10"));
    s.insert(QString("20"));
    s.insert(QString("30"));
    return 0;
}
