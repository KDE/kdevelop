#include <QByteArray>
int main()
{
    QByteArray ba("\xe6\x98\xaf'\"\\u6211");
    ba.append("x");
    return 0;
}
