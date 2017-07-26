#include <QString>
int main()
{
    QString s = QString::fromUtf8("test最后一个不是特殊字符'\"\\u6211");
    s.append("x");
    return 0;
}
