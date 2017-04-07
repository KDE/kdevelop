#include <QString>
int main()
{
    QString s(QStringLiteral("test string"));
    s.append("x");
    return 0;
}
