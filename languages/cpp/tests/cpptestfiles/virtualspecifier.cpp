/* This file is part of KDevelop
   Copyright 2012 Olivier de Gaalon <olivier.jg@gmail.com>

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

//"internalContext" : { "type" : "Class" }
class Base
{
public:
  //"isVirtual" : false
  Base();
  //"isVirtual" : true
  virtual ~Base();
  //"isVirtual" : true
  virtual int anIntFunc();
  //"isVirtual" : false
  int anIntFunc(int foo);
  //"isVirtual" : true
  virtual operator int();
  //"isVirtual" : false
  operator char();
};

template<class T> class Derived : public Base
{
public:
  //"isVirtual" : false
  Derived();
  //"EXPECT_FAIL" : { "isVirtual" : "Destructors currently don't inherit the virtual specifier as they should" },
  //"isVirtual" : true
  ~Derived();
  //"isVirtual" : true
  int anIntFunc();
  //"EXPECT_FAIL" : { "isVirtual" : "Virtual specifier inheritance doesn't match functions with their overloads" },
  //"isVirtual" : false #Hides a non-virtual function
  int anIntFunc(int foo);
  //"isVirtual" : true
  operator int();
  //"EXPECT_FAIL" : { "isVirtual" : "Virtual specifier inheritance doesn't differentiate between cast operators" },
  //"isVirtual" : false #Hides a non-virtual cast operator in base
  operator char();
};

template<> class Derived<int> : public Base
{
  //"isVirtual" : false
  Derived<int>();
  //"EXPECT_FAIL" : { "isVirtual" : "Destructors currently don't inherit the virtual specifier as they should" },
  //"isVirtual" : true
  ~Derived<int>();
  //"isVirtual" : true
  int anIntFunc();
  //"EXPECT_FAIL" : { "isVirtual" : "Virtual specifier inheritance doesn't match functions with their overloads" },
  //"isVirtual" : false #Hides a non-virtual function
  int anIntFunc(int foo);
  //"isVirtual" : true
  operator int();
  //"EXPECT_FAIL" : { "isVirtual" : "Virtual specifier inheritance doesn't differentiate between cast operators" },
  //"isVirtual" : false #Hides a non-virtual cast operator in base
  operator char();
};

