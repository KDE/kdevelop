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
#include <language/duchain/types/functiontype.h>
#include <language/duchain/types/referencetype.h>
#include <language/duchain/types/integraltype.h>

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

void TestDUChain::testDecltype()
{
  // see also: spec 7.1.6/4
  QByteArray code = "const int&& foo();\n"
                    "int i;\n"
                    "struct A { double x; };\n"
                    "const A* a = new A();\n"
                    // start decltype examples
                    "decltype(foo()) x1 = i;\n"
                    "decltype(i) x2;\n"
                    "decltype(a->x) x3;\n"
                    "decltype((a->x)) x4;\n"
                    // other examples
                    "const int& j = i;\n"
                    "decltype((j)) x5;\n"
                    "int& j2 = i;\n"
                    "decltype((j2)) x6;\n"
                    ;

  LockedTopDUContext top = parse(code, DumpAll);
  QVERIFY(top);
  DUChainReadLocker lock;

  // x1
  QCOMPARE(top->localDeclarations().size(), 12);
  FunctionDeclaration* fooDec = dynamic_cast<FunctionDeclaration*>(top->localDeclarations().at(0));
  QVERIFY(fooDec);
  QVERIFY(fooDec->type<FunctionType>());
  QCOMPARE(fooDec->type<FunctionType>()->returnType()->toString(), QString("const int&&"));
  Declaration* x1Dec = top->localDeclarations().at(4);
  QVERIFY(x1Dec->abstractType()->equals(fooDec->type<FunctionType>()->returnType().constData()));

  // x2
  Declaration* iDec = top->localDeclarations().at(1);
  Declaration* x2Dec = top->localDeclarations().at(5);
  QVERIFY(x2Dec->abstractType()->equals(iDec->abstractType().constData()));

  // x3
  Declaration* ADec = top->localDeclarations().at(2);
  QVERIFY(ADec->internalContext());
  QCOMPARE(ADec->internalContext()->localDeclarations().size(), 1);
  Declaration* AxDec = ADec->internalContext()->localDeclarations().first();
  Declaration* x3Dec = top->localDeclarations().at(6);
  QVERIFY(x3Dec->abstractType()->equals(AxDec->abstractType().constData()));

  // x4
  // const& due to additional parens
  Declaration* x4Dec = top->localDeclarations().at(7);
  QVERIFY(!x4Dec->abstractType()->equals(AxDec->abstractType().constData()));
  QVERIFY(!(x4Dec->abstractType()->modifiers() & AbstractType::ConstModifier));
  QVERIFY(x4Dec->abstractType().cast<ReferenceType>());
  QVERIFY(x4Dec->abstractType().cast<ReferenceType>()->baseType()->modifiers() & AbstractType::ConstModifier);
  QVERIFY(x4Dec->abstractType().cast<ReferenceType>()->baseType().cast<IntegralType>());
  QCOMPARE(x4Dec->abstractType().cast<ReferenceType>()->baseType().cast<IntegralType>()->dataType(),
           (uint) IntegralType::TypeDouble);

  // x5
  // already const&, make sure it's not doubled due to additional parens
  Declaration* jDec = top->localDeclarations().at(8);
  Declaration* x5Dec = top->localDeclarations().at(9);
  QVERIFY(x5Dec->abstractType()->equals(jDec->abstractType().constData()));
  QVERIFY(!(x5Dec->abstractType()->modifiers() & AbstractType::ConstModifier));
  QVERIFY(x5Dec->abstractType().cast<ReferenceType>());
  QVERIFY(x5Dec->abstractType().cast<ReferenceType>()->baseType()->modifiers() & AbstractType::ConstModifier);
  QVERIFY(x5Dec->abstractType().cast<ReferenceType>()->baseType().cast<IntegralType>());
  QCOMPARE(x5Dec->abstractType().cast<ReferenceType>()->baseType().cast<IntegralType>()->dataType(),
           (uint) IntegralType::TypeInt);

  // x6
  // already &, make sure it's not doubled due to additional parens, but must be const
  Declaration* j2Dec = top->localDeclarations().at(10);
  Declaration* x6Dec = top->localDeclarations().at(11);
  // jDec has the type that we want
  QVERIFY(x6Dec->abstractType()->equals(jDec->abstractType().constData()));
  QVERIFY(!(x6Dec->abstractType()->modifiers() & AbstractType::ConstModifier));
  QVERIFY(x6Dec->abstractType().cast<ReferenceType>());
  QVERIFY(x6Dec->abstractType().cast<ReferenceType>()->baseType()->modifiers() & AbstractType::ConstModifier);
  QVERIFY(x6Dec->abstractType().cast<ReferenceType>()->baseType().cast<IntegralType>());
  QCOMPARE(x6Dec->abstractType().cast<ReferenceType>()->baseType().cast<IntegralType>()->dataType(),
           (uint) IntegralType::TypeInt);
}
