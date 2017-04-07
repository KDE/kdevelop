#include <QString>
int main()
{
    QString s = QStringLiteral("test最后一个不是特殊字符'\"\\u6211");
    s.append("x");
    return 0;
}
