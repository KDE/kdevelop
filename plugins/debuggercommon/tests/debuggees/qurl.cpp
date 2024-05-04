#include <QUrl>
int main()
{
    QUrl u(QStringLiteral("http://user@www.kdevelop.org:12345/foo?xyz=bar#asdf"));
    const auto localFile = QUrl::fromLocalFile(QStringLiteral("/usr/bin/kdevelop"));
    QUrl defaultConstructed;

    // Verify that QUrlPrinter does not treat a real QString("<uninitialized>")
    // component as an uninitialized QString variable.
    u.setPassword("<uninitialized>");
    u.setFragment(QStringLiteral("<uninitialized>"));

    return 0;
}
// clazy:skip
