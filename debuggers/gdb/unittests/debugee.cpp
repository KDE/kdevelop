/*
   Copyright 2009 Niko Sams <niko.sams@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include <iostream>
struct testStruct { int a; int b; int c; };
void noop() {}
void foo() {
    static int i=0;
    ++i;
    noop();
    noop();
}
int main() {
    std::cout << "Hello, world!" << std::endl;
    foo();
    foo();

    const char *x = "Hello";
    std::cout << x << std::endl;
    
    testStruct ts;
    ts.a = 0;
    ts.b = 1;
    ts.c = 2;
    ts.a++;
    return 0;
}
