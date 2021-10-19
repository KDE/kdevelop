#include <QUrl>
int main()
{
    QUrl u(QStringLiteral("http://user@www.kdevelop.org:12345/foo?xyz=bar#asdf"));
    const auto localFile = QUrl::fromLocalFile(QStringLiteral("/usr/bin/kdevelop"));
    QUrl defaultConstructed;
    return 0;
}
// clazy:skip
