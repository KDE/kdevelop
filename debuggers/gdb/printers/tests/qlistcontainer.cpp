#include <QList>
#include <QVector>
#include <QQueue>
#include <QStack>
#include <QString>
struct A {
    QString a;
    QString b;
    int c;
    int d;
};

template<template <typename> class Container>
void doStuff()
{
    Container<int> intList;
    intList << 10 << 20;
    intList << 30;

    Container<QString> stringList;
    stringList << "a" << "bc";
    stringList << "d";

    Container<A> structList;
    structList << A();
    structList[0].a = "a";
    structList[0].c = 100;
    structList << A();

    Container<int*> pointerList;
    pointerList << new int(1) << new int(2);
    pointerList << new int(3);
    qDeleteAll(pointerList);
}

int main()
{
    doStuff<QList>();
    doStuff<QVector>();
    doStuff<QQueue>();
    doStuff<QStack>();
    return 0;
}
