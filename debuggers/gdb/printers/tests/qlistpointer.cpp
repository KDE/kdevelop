#include <QList>
int main()
{
    QList<int*> l;
    l << new int(1);
    l << new int(2);
    l << new int(3);
    qDeleteAll(l);
    return 0;
}
