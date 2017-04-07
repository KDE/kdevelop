#include <QUrl>
int main()
{
    QUrl u(QStringLiteral("http://user:password@www.kdevelop.org:12345/foo?xyz=bar#asdf"));
    return 0;
}
