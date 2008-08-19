#!/usr/bin/python
# resets manual test input after messing with it

FOO_H=\
'''#ifndef CPP_EXAMPLES_FOO_H
#define CPP_EXAMPLES_FOO_H
class Foo { public: void foo(); };
#endif // CPP_EXAMPLES_FOO_H
'''

BAR_H=\
'''#ifndef CPP_EXAMPLES_BAR_H
#define CPP_EXAMPLES_BAR_H
class Bar { public: void bar(); };
#endif // CPP_EXAMPLES_BAR_H
'''

BAZ_H=\
'''#ifndef CPP_EXAMPLES_BAZ_H
#define CPP_EXAMPLES_BAZ_H
class Baz { public: void baz(); };
#endif // CPP_EXAMPLES_BAZ_H
'''

INCLUDECOMPLETION_CPP=\
'''/*! multi line
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
'''

file_contents = dict()
file_contents["includecompletion.cpp"] = INCLUDECOMPLETION_CPP
file_contents["foo.h"] = FOO_H
file_contents["bar.h"] = BAR_H
file_contents["baz.h"] = BAZ_H
file_contents["dummy.h"] = "\n"

for file_, contents in file_contents.items():
    f = open(file_, 'w')
    f.write(contents)
    f.close()

from os import system
system('find . -name "*~" -delete')
