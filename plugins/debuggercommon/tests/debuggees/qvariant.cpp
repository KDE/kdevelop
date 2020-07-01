#include <QVariant>
#include <QString>
int main() {
    QVariant v(QStringLiteral("KDevelop"));
    Q_UNUSED(v.toString()); // prevent compiler optimizing away unused object
    return 0;
}
