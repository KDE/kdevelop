#include <QList>
#include <QString>
struct A {
    QString a;
    QString b;
    int c;
    int d;
};
int main()
{
    QList<A> l;
    l << A();
    l[0].a = "a";
    l[0].c = 100;
    l << A();
    return 0;
}
