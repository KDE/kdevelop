/*
   Copyright 2012 Niko Sams <niko.sams@gmail.com>

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

inline int aPlusB(int a, int b) {
    return a+b;
}

inline int aPlusB(int a) {
    return a+1;
}

int main()
{
    int test1 = aPlusB(1, 1);
    (void)test1;
    int test4 = aPlusB(2);
    (void)test4;

    return 0;
}
