#include <project/path.h>

#include <QDebug>

using namespace KDevelop;

int main()
{
    Path path1("/tmp/foo");
    Path path2("http://www.test.com/tmp/asdf.txt");

    return 0;
}
