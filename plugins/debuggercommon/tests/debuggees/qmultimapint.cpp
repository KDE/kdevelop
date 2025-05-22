#include <QMultiMap>
int main()
{
    QMultiMap<int, int> m;
    m.insert(10, 100);
    m.insert(20, 200);
    m.insert(30, 300);

    m.insert(10, 123);
    m.insert(30, 82);
    m.insert(4, 99);
    m.insert(10, 0);
    m.insert(30, 300); // insert another element {30, 300}
    m.remove(20, 200);

    return 0;
}
