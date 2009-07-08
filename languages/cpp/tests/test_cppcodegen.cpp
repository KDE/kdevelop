/*
   This file is part of KDevelop
   Copyright 2009 Ramón Zarazúa <killerfox512+kde@gmail.com>
   
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

#include "test_cppcodegen.h"

#include <language/codegen/coderepresentation.h>

#include <QtTest/QTest>

QTEST_MAIN(TestCppCodegen)

using namespace KDevelop;

TestCppCodegen::TestCppCodegen()
{
  //Insert all the test data as a string representation
  InsertArtificialCodeRepresentation(IndexedString("ClassA.h"), "class ClassA { public: ClassA(); private: int i;  float f, j;\
                                                                  struct ContainedStruct { int i; ClassA * p;  } structVar; };");
  InsertArtificialCodeRepresentation(IndexedString("ClassA.cpp"), "ClassA::ClassA() : i(0), j(0.0) {structVar.i = 0; }");
  
}


void TestCppCodegen::testAstDuChainMapping ()
{

}


#include "test_cppcodegen.moc"
