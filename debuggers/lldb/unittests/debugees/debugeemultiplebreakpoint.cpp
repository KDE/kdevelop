/*
   Copyright 2013 Vlas Puhov <vlas.puhov@mail.ru>

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

class Str2 {
    public: void print(void) {
        std::cout << "Str2\n";
    }
};

class Str {
    public: Str() {
        S="str\n";
    }
    public: void print(void) {
        std::cout << S;
    }
    private: std::string S;
};


void Print ( Str2& c, Str& s, bool b )
{
    if(b){
       c.print();
       s.print(); 
    }
}

int main ( void )
{
    Str2 c;
    Str s;
    Print ( c, s, true );
    return 1;
}
