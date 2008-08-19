/*! multi line
* comment header */

void fun1()
{
    // invoke code completion.
    Foo f;
    f.
    // include should be added underneath the file comment
}

#include "dummy.h"

void fun2()
{
    // invoke code completion.
    Bar b;
    b.
    // include should be added under dummy.h
}

class Baz;
void fun3()
{
    // invoke code completion
    Baz b;
    b.
    // include should be added somewhere in front
}

void fun4()
{
    // invoke code completion
    Foo f;
    f.
    // include should added in front of the FIRST Foo declaration
}

#include "dummy.moc"
