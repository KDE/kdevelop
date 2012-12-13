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

// "definition" : { "internalFunctionContext" : { "type" : "Function", "localDeclarationCount" : 1 } }
void someFunc(int a);
// "declaration" : { "null" : true }
void someFunc(int a, int f) { }
// "declaration" : { "internalFunctionContext" : { "type" : "Function", "localDeclarationCount" : 1 } }
void someFunc(int a) { }

class someClass
{
  void someClassFunc();
  void someClassFunc(int arg, char arg);
  void anotherClassFunc();
  void anotherClassFunc(someClass foo);
  void anotherClassFunc(int foo);
  void steal();
  void steal(int arg);
};

// "declaration" : { "internalFunctionContext" : { "type" : "Function", "localDeclarationCount" : 2 } }
void someClass::someClassFunc(int arg, char arg) { }
// "declaration" : { "internalFunctionContext" : { "type" : "Function", "localDeclarationCount" : 0 } }
void someClass::someClassFunc() { }
// "declaration" : { "internalFunctionContext" : { "type" : "Function", "localDeclarationCount" : 0 } }
void someClass::anotherClassFunc() { }
// "declaration" : { "internalFunctionContext" : { "findDeclarations" : { "foo" : { "null" : false } } } }
void someClass::anotherClassFunc(char foo) { }
// "declaration" : { "internalFunctionContext" : { "findDeclarations" : { "foo" : { "null" : false } } } }
void someClass::anotherClassFunc(char foo, int bar) { }
// "declaration" : { "null" : true }
void someClass::anotherClassFunc(char foo, int bar, int another) { }
// "declaration" : { "EXPECT_FAIL" : { "null" :
//   "Definitions steal declarations if they match better, but ideally the robbed declaration should get another chance to grab one" }, "null" : false }
void someClass::steal(int arg, int arg2) { }
// "declaration" : { "internalFunctionContext" : { "type" : "Function", "localDeclarationCount" : 0 } }
void someClass::steal() { }

class ClassA {};
class ClassB {};
class ClassC {};
class ClassD {};
template<typename E_T1, typename E_T2>
class ClassE
{
public:
  template<typename E_A_T1, typename E_A_T2>
  void E_FuncA(E_A_T1, E_A_T2) { }
  void E_FuncB(E_T1, E_T2) { }
};
template<> template<>
void ClassE<ClassA, ClassB>::E_FuncA<ClassA, ClassA>(ClassA, ClassA);
// "declaration" : { "EXPECT_FAIL" : { "null" : "Specialization declarations outside of classes aren't found" }, "null" : false }
template<> template<>
void ClassE<ClassA, ClassB>::E_FuncA<ClassA, ClassA>(ClassA, ClassA) { }
template<>
void ClassE<ClassA, ClassB>::E_FuncB(ClassA, ClassB);
// "declaration" : { "EXPECT_FAIL" : { "null" : "Specialization declarations outside of classes aren't found" }, "null" : false }
template<>
void ClassE<ClassA, ClassB>::E_FuncB(ClassA, ClassB) { }
