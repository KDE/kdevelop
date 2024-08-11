#include <QMultiHash>
int main()
{
    QMultiHash<int, int> h;
    h[10] = 100;
    h[20] = 200;
    h[30] = 300;

    h.insert(10, 123);
    h.insert(30, 82);
    h.insert(4, 99);
    h.insert(10, 0);
    h.insert(30, 300); // insert another element {30, 300}
    h.remove(20, 200); // should remove the now-empty node

    return 0;
}
