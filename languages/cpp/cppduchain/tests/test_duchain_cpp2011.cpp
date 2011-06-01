/* This file is part of KDevelop

   Copyright 2011 Milian Wolff <mail@milianw.de>

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

#include "test_duchain.h"

#include <QTest>

#include <language/duchain/topducontext.h>
#include <language/duchain/declaration.h>

using namespace KDevelop;
using namespace Cpp;

void TestDUChain::testRangeBasedFor() {
  //                                         1         2         3
  //                                123456789012345678901234567890
  LockedTopDUContext top = parse(  "int main() {"
                                 "\n  int a[5] = {1,2,3,4,5};"
                                 "\n  for(int i : a) {"
                                 "\n    int j = i + 1;"
                                 "\n  }"
                                 "\n}"
                                 "\n", DumpAll
                           );
  QVERIFY(top);
  DUChainReadLocker lock;
  QVERIFY(top->problems().isEmpty());
  QCOMPARE(top->childContexts().size(), 2);
  QCOMPARE(top->localDeclarations().size(), 1);
  DUContext* ctx = top->childContexts().last();
  QVector< Declaration* > decls = ctx->localDeclarations();
  QCOMPARE(decls.size(), 1);

  QCOMPARE(decls.at(0)->identifier().toString(), QString("a"));
  QCOMPARE(decls.at(0)->abstractType()->toString(), QString("int[5]"));
  QCOMPARE(decls.at(0)->uses().constBegin()->size(), 1);
  QCOMPARE(decls.at(0)->uses().constBegin()->first().start.line, 2);

  QCOMPARE(ctx->childContexts().size(), 2);
  decls = ctx->childContexts().first()->localDeclarations();
  QCOMPARE(decls.size(), 1);
  QCOMPARE(decls.at(0)->identifier().toString(), QString("i"));
  QCOMPARE(decls.at(0)->abstractType()->toString(), QString("int"));
  QCOMPARE(decls.at(0)->range().start.line, 2);
  QCOMPARE(decls.at(0)->uses().constBegin()->size(), 1);
  QCOMPARE(decls.at(0)->uses().constBegin()->first().start.line, 3);

  decls = ctx->childContexts().last()->localDeclarations();
  QCOMPARE(decls.size(), 1);
  QCOMPARE(decls.at(0)->identifier().toString(), QString("j"));
  QCOMPARE(decls.at(0)->abstractType()->toString(), QString("int"));
  QVERIFY(decls.at(0)->uses().isEmpty());
}