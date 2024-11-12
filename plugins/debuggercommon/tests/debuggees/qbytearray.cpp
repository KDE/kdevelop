#include <QByteArray>
int main()
{
    QByteArray ba("\xe6\x98\xaf'\"\\u6211");
    ba.append("x");
    QByteArray nonUtf("ABC\xFF\x00\xFE", 6);
    return 0;
}
