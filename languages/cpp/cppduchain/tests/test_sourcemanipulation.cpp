/*
  This file is part of KDevelop

  Copyright 2010 Milian Wolff <mail@milianw.de>

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

#include "test_sourcemanipulation.h"

#include <QtTest>
#include <tests/testcore.h>

#include "sourcemanipulation.h"

QTEST_MAIN(TestSourceManipulation)

using namespace KDevelop;
using namespace Cpp;

void TestSourceManipulation::initTestCase()
{
  initShell();
}

void TestSourceManipulation::cleanupTestCase()
{
  TestCore::shutdown();
}

void TestSourceManipulation::testFirstValidCodeLineBefore_data()
{
  QTest::addColumn<QString>("code");
  QTest::addColumn<int>("lineIn");
  QTest::addColumn<int>("lineOut");

  QTest::newRow("onlyFunction") << "void foo() {\n" // we want this line
                                   "  MissingInclude;\n"
                                   "}\n"
                                << 1 << 0;

  QTest::newRow("afterComment") << "/**\n"
                                   " *\n"
                                   " **/\n"
                                   "\n" // we want this line
                                   "void foo() {\n"
                                   "  MissingInclude;\n"
                                   "}\n"
                                << 5 << 3;

  QTest::newRow("afterComment2") << "/**\n"
                                   " *\n"
                                   " **/\n"
                                   "void foo() {\n" // we want this line
                                   "  MissingInclude;\n"
                                   "}\n"
                                << 4 << 3;

  QTest::newRow("afterInclude") << "#include <cstddef>\n"
                                   "\n" // we want this line
                                   "void foo() {\n"
                                   "  MissingInclude;\n"
                                   "}\n"
                                << 3 << 1;

  QTest::newRow("afterInclude2") << "/**\n"
                                   " *\n"
                                   " **/\n"
                                   "#include <cstddef>\n"
                                   "\n" // we want this line
                                   "void foo() {\n"
                                   "  MissingInclude;\n"
                                   "}\n"
                                << 6 << 4;

  QTest::newRow("afterInclude3") << "#include <cstddef>\n"
                                   "void foo() {\n" // we want this line
                                   "  MissingInclude;\n"
                                   "}\n"
                                << 2 << 1;
}

void TestSourceManipulation::testFirstValidCodeLineBefore()
{
  QFETCH(QString, code);
  QFETCH(int, lineIn);
  QFETCH(int, lineOut);

  LockedTopDUContext top = parse(code.toLocal8Bit(), DumpNone);
  QVERIFY(top);

  InsertArtificialCodeRepresentation repr(top->url(), code);

  Cpp::SourceCodeInsertion insertion(top);
  QCOMPARE(insertion.firstValidCodeLineBefore(lineIn), lineOut);
}
