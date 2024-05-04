#include <QString>
int main()
{
    QString s = QString::fromUtf8("test最后一个不是特殊字符'\"\\u6211");
    s.append("x");
    const QString qstringLiteral = QStringLiteral("string literal");
    QString nullString;
    const auto emptyString = QString::fromUtf8("");
    return 0;
}
