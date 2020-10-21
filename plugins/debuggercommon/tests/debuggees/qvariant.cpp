#include <QVariant>
#include <QString>
#include <QObject>

struct SomeCustomType {
    int foo = 42;
};
Q_DECLARE_METATYPE(SomeCustomType)

int main()
{
    QObject myObj;
    QVariant v;

    v = QString::fromUtf8("KDevelop (QString)");
    v = QByteArray("KDevelop (QByteArray)");

    v = QVariant::fromValue(qint8(-8));
    v = QVariant::fromValue(quint8(8));
    v = QVariant::fromValue(qint16(-16));
    v = QVariant::fromValue(quint16(16));
    v = QVariant::fromValue(qint32(-32));
    v = QVariant::fromValue(quint32(32));
    v = QVariant::fromValue(qint64(-64));
    v = QVariant::fromValue(quint64(64));

    v = QVariant::fromValue(true);

    v = QVariant::fromValue(4.5f);
    v = QVariant::fromValue(42.5);

    v = QVariant::fromValue(&myObj);

    v = QVariant::fromValue(SomeCustomType());

    Q_UNUSED(v); // prevent compiler optimizing away unused object
    return 0;
}
