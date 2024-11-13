#include <QString>
int main()
{
    QString s = QString::fromUtf8("test最后一个不是特殊字符'\"\\u6211");
    s.append("x");
    QStringView view(s);
    view = view.left(5);
    QLatin1String latin1String("abc\351"); // é in latin1
    QUtf8StringView utf8StringView("test最后");

    QString nullString;
    const auto emptyString = QString::fromUtf8("");
    QStringView emptyView;
    QLatin1String emptyLatin1String;
    QUtf8StringView emptyUtf8StringView;
    return 0;
}
