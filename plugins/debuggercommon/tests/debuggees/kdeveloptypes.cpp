#include <util/path.h>

#include <QDebug>

using namespace KDevelop;

int main()
{
    Path path1(QStringLiteral("/tmp/foo"));
    Path path2(QStringLiteral("http://www.test.com/tmp/asdf.txt"));

    return 0;
}
