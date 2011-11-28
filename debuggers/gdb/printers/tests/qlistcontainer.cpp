#include <QList>
#include <QVector>
#include <QQueue>
#include <QStack>
#include <QLinkedList>
#include <QSet>
#include <QString>
#include <QDebug>

struct A {
    A(const QString& _a = QString(), const QString& _b = QString(),
      int _c = -1, int _d = -1)
    : a(_a), b(_b), c(_c), d(_d)
    {}
    bool operator==(const A& other) const
    {
        return a == other.a && b == other.b && c == other.c && d == other.d;
    }
    QString a;
    QString b;
    int c;
    int d;
};

uint qHash(const A& a)
{
    return qHash(a.a) + qHash(a.b);
}

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
    structList << A("a", "b", 100, -200);
    structList << A();

    Container<int*> pointerList;
    pointerList << new int(1) << new int(2);
    pointerList << new int(3);
    qDeleteAll(pointerList);

    Container<QPair<int, int> > pairList;
    pairList << QPair<int, int>(1, 2) << qMakePair(2, 3);
    pairList << qMakePair(4, 5);
}

int main()
{
    doStuff<QList>();
    doStuff<QVector>();
    doStuff<QQueue>();
    doStuff<QStack>();
    doStuff<QLinkedList>();
    doStuff<QSet>();
    return 0;
}
