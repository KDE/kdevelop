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
#include <language/duchain/classfunctiondeclaration.h>
#include <language/duchain/functiondefinition.h>
#include <language/duchain/forwarddeclaration.h>

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

void TestDUChain::testRValueReference() {
  //                                         1         2         3
  //                                123456789012345678901234567890
  LockedTopDUContext top = parse(  "class A {};"
                                 "\nint&& intRef;"
                                 "\nA&& aRef;"
                                 "\n", DumpAll
                           );
  QVERIFY(top);
  DUChainReadLocker lock;
  QVERIFY(top->problems().isEmpty());

  QVector< Declaration* > decls = top->localDeclarations();
  QCOMPARE(decls.size(), 3);

  QCOMPARE(decls.at(0)->toString(), QString("class A"));
  QCOMPARE(decls.at(1)->toString(), QString("int&& intRef"));
  QCOMPARE(decls.at(2)->toString(), QString("A&& aRef"));
}

void TestDUChain::testDefaultDelete() {
  LockedTopDUContext top = parse("class A {\n"
                                 "  A() = default;\n"
                                 "  A(const A&) = delete;\n"
                                 "  virtual ~A();\n"
                                 "};\n"
                                 "A::~A() = default;\n"
                                 "void foo(int) = delete;\n"
                                 , DumpAll);
  QVERIFY(top);
  DUChainReadLocker lock;
  QVERIFY(top->problems().isEmpty());

  DUContext* ACtx = top->childContexts().first();
  const QVector< Declaration* > Adecs = ACtx->localDeclarations();
  QCOMPARE(Adecs.size(), 3);

  {
  ClassFunctionDeclaration* aCtor = dynamic_cast<ClassFunctionDeclaration*>(Adecs.at(0));
  QVERIFY(aCtor);
  QVERIFY(!aCtor->isAbstract());
  QVERIFY(!aCtor->isDestructor());
  QVERIFY(aCtor->isConstructor());
  QCOMPARE(aCtor->isExplicitlyDeleted(), false);
  QCOMPARE(aCtor->isDefinition(), true);
  }

  {
  ClassFunctionDeclaration* copyCtor = dynamic_cast<ClassFunctionDeclaration*>(Adecs.at(1));
  QVERIFY(copyCtor);
  QVERIFY(!copyCtor->isAbstract());
  QVERIFY(!copyCtor->isDestructor());
  QVERIFY(copyCtor->isConstructor());
  QCOMPARE(copyCtor->isExplicitlyDeleted(), true);
  QCOMPARE(copyCtor->isDefinition(), true);
  }

  {
  ClassFunctionDeclaration* aDtor = dynamic_cast<ClassFunctionDeclaration*>(Adecs.at(2));
  QVERIFY(aDtor);
  QVERIFY(!aDtor->isAbstract());
  QVERIFY(aDtor->isVirtual());
  QVERIFY(aDtor->isDestructor());
  QVERIFY(!aDtor->isConstructor());
  QCOMPARE(aDtor->isDefinition(), false);
  FunctionDefinition* definition = FunctionDefinition::definition(aDtor);
  QVERIFY(definition);
  QCOMPARE(definition->range().start.line, 5);
  }

  {
  FunctionDeclaration* fooDec = dynamic_cast<FunctionDeclaration*>(top->localDeclarations().last());
  QVERIFY(fooDec);
  QCOMPARE(fooDec->isExplicitlyDeleted(), true);
  QCOMPARE(fooDec->isDefinition(), true);
  }
}

void TestDUChain::testEnum2011_data()
{
  QTest::addColumn<QString>("code");
  QTest::addColumn<bool>("scoped");
  QTest::addColumn<bool>("opaque");

  QTest::newRow("enum") << "enum Foo {A, B};" << false << false;
  QTest::newRow("enum-empty") << "enum Foo {};" << false << false;
  QTest::newRow("enum-class") << "enum class Foo {A, B};" << true << false;
  QTest::newRow("enum-struct") << "enum struct Foo {A, B};" << true << false;
  QTest::newRow("enum-typespec") << "enum Foo : int {A, B};" << false << false;
  QTest::newRow("enum-opaque") << "enum Foo;" << false << true;
  QTest::newRow("enum-opaque-class") << "enum class Foo;" << true << true;
  QTest::newRow("enum-opaque-class-typespec") << "enum class Foo : char;" << true << true;
  QTest::newRow("enum-opaque-typespec") << "enum Foo : unsigned int;" << false << true;
}

void TestDUChain::testEnum2011()
{
  QFETCH(QString, code);
  QFETCH(bool, scoped);
  QFETCH(bool, opaque);

  LockedTopDUContext top = parse(code.toUtf8(), DumpAll);
  QVERIFY(top);
  DUChainReadLocker lock;
  QVERIFY(top->problems().isEmpty());

  QCOMPARE(top->localDeclarations().size(), 1);
  Declaration* dec = top->localDeclarations().first();
  QCOMPARE(dec->identifier().toString(), QString("Foo"));

  if (opaque) {
    QVERIFY(dec->isForwardDeclaration());
  } else {
    QVERIFY(!dec->isForwardDeclaration());
    QCOMPARE(dec->internalContext()->localScopeIdentifier().isEmpty(), !scoped);
  }
}
