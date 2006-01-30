
#include <stdio.h>
#include <qstring.h>

void foo(int a)
{
    QString s = "foo";
    int i = 10;
    int i2 = 12;
    int i3 = i + i2;
    printf("i3 = %d\n", i3);
}