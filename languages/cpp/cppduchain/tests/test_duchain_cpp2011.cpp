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
#include <language/duchain/types/pointertype.h>
#include <language/duchain/types/arraytype.h>

#include "typeutils.h"
#include "templatedeclaration.h"
#include "cppduchain.h"

using namespace KDevelop;
using namespace Cpp;

extern template char *QTest::toString(const KDevelop::RangeInRevision &range);

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

void TestDUChain::testRangeBasedForClass() {
  //                                         1         2         3
  //                                123456789012345678901234567890
  LockedTopDUContext top = parse(  "struct MyType{};"
                                 "\nstruct Iterator{"
                                 "\n  MyType operator*(){return 0;}"
                                 "\n  bool operator!=(const Iterator&){return false;}"
                                 "\n  Iterator& operator++(){return *this;}"
                                 "\n};"
                                 // begin + end as normal functions
                                 "\nstruct List1{ }; Iterator begin(List1); Iterator end(List1);"
                                 // begin with const& + end as normal functions
                                 "\nstruct List2{ }; Iterator begin(const List2&); Iterator end(const List2&);"
                                 "\nint main() {"
                                 "\n  List1 l1;"
                                 "\n  for(auto i : l1) { }"
                                 "\n  List2 l2;"
                                 "\n  for(auto i : l2) { }"
                                 "\n}"
                                 "\n", DumpAll
                           );
  QVERIFY(top);
  DUChainReadLocker lock;
  QVERIFY(top->problems().isEmpty());
  QCOMPARE(top->childContexts().size(), 10);
  QCOMPARE(top->localDeclarations().size(), 9);
  DUContext* ctx = top->childContexts().last();
  QVector< Declaration* > decls = ctx->localDeclarations();
  QCOMPARE(decls.size(), 2);

  QCOMPARE(decls.at(0)->identifier().toString(), QString("l1"));
  QCOMPARE(decls.at(0)->abstractType()->toString(), QString("List1"));
  QCOMPARE(decls.at(0)->uses().constBegin()->size(), 1);
  QCOMPARE(decls.at(0)->uses().constBegin()->first().start.line, 10);

  QCOMPARE(decls.at(1)->identifier().toString(), QString("l2"));
  QCOMPARE(decls.at(1)->abstractType()->toString(), QString("List2"));
  QCOMPARE(decls.at(1)->uses().constBegin()->size(), 1);
  QCOMPARE(decls.at(1)->uses().constBegin()->first().start.line, 12);

  QCOMPARE(ctx->childContexts().size(), 4);
  decls = ctx->childContexts().at(0)->localDeclarations();
  QCOMPARE(decls.size(), 1);
  QCOMPARE(decls.at(0)->identifier().toString(), QString("i"));
  QVERIFY(decls.at(0)->abstractType());
  QCOMPARE(decls.at(0)->abstractType()->toString(), QString("MyType"));
  QCOMPARE(decls.at(0)->range().start.line, 10);

  decls = ctx->childContexts().at(2)->localDeclarations();
  QCOMPARE(decls.size(), 1);
  QCOMPARE(decls.at(0)->identifier().toString(), QString("i"));
  QVERIFY(decls.at(0)->abstractType());
  QCOMPARE(decls.at(0)->abstractType()->toString(), QString("MyType"));
  QCOMPARE(decls.at(0)->range().start.line, 12);
}

void TestDUChain::testRangeBasedForClass2() {
  // see: https://bugs.kde.org/show_bug.cgi?id=299310
  // see: 24.6.5 in the c++11 standard: range access [iterator.range]
  //                                         1         2         3
  //                                123456789012345678901234567890
  LockedTopDUContext top = parse(  "template<typename T> struct list{"
                                 "\n  T*& begin();"
                                 "\n  const T* const& begin() const;"
                                 "\n};"
                                 "\nnamespace std{"
                                 "\n  template<class C> auto begin(C& c) -> decltype(c.begin());"
                                 "\n  template<class C> auto begin(const C& c) -> decltype(c.begin());"
                                 "\n}"
                                 "\nint main() {"
                                 "\n  list<double> l1;"
                                 "\n  int a = 0;" // make sure lastType is bogus
                                 "\n  for(auto i : l1) { }"
                                 "\n  const list<float> l2;"
                                 "\n  int b = 0;" // make sure lastType is bogus
                                 "\n  for(auto i : l2) { }"
                                 "\n}"
                                 "\n", DumpNone
                           );
  QVERIFY(top);
  DUChainReadLocker lock;
  QVERIFY(top->problems().isEmpty());
  QCOMPARE(top->childContexts().size(), 5);
  QCOMPARE(top->localDeclarations().size(), 3);
  DUContext* ctx = top->childContexts().last();
  QVector< Declaration* > decls = ctx->localDeclarations();
  QCOMPARE(decls.size(), 4);

  QCOMPARE(decls.at(0)->identifier().toString(), QString("l1"));
  QCOMPARE(decls.at(0)->abstractType()->toString(), QString("list< double >"));
  QCOMPARE(decls.at(0)->uses().constBegin()->size(), 1);
  QCOMPARE(decls.at(0)->uses().constBegin()->first().start.line, 11);

  QCOMPARE(decls.at(2)->identifier().toString(), QString("l2"));
  QCOMPARE(decls.at(2)->abstractType()->toString(), QString("const list< float >"));
  QCOMPARE(decls.at(2)->uses().constBegin()->size(), 1);
  QCOMPARE(decls.at(2)->uses().constBegin()->first().start.line, 14);

  QCOMPARE(ctx->childContexts().size(), 4);
  decls = ctx->childContexts().at(0)->localDeclarations();
  QCOMPARE(decls.size(), 1);
  QCOMPARE(decls.at(0)->identifier().toString(), QString("i"));
  QVERIFY(decls.at(0)->abstractType());
  QCOMPARE(decls.at(0)->abstractType()->toString(), QString("double"));
  QCOMPARE(decls.at(0)->range().start.line, 11);

  // note: auto e = begin(const list<>&) will return a *non* const type
  // this is expected as per definition of the auto functionality
  decls = ctx->childContexts().at(2)->localDeclarations();
  QCOMPARE(decls.size(), 1);
  QCOMPARE(decls.at(0)->identifier().toString(), QString("i"));
  QVERIFY(decls.at(0)->abstractType());
  QCOMPARE(decls.at(0)->abstractType()->toString(), QString("float"));
  QCOMPARE(decls.at(0)->range().start.line, 14);
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

void TestDUChain::testDelete_Bug278781()
{
  // don't crash, see also: https://bugs.kde.org/show_bug.cgi?id=278781
  LockedTopDUContext top = parse("NonCopyable(const NonCopyable&) = delete;\n"
                                 , DumpNone);
  QVERIFY(top);
  DUChainReadLocker lock;
  QVERIFY(top->problems().isEmpty());
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

void TestDUChain::testDecltypeUses()
{
  QByteArray method("int c; decltype(c) c2 = decltype(c2)(c+5) + Test(q+5);");

  LockedTopDUContext top = parse(method, DumpAll);

  QCOMPARE(top->localDeclarations().size(), 2);
  QVERIFY(top->localDeclarations()[0]->indexedType() == top->localDeclarations()[1]->indexedType());
  QCOMPARE(top->localDeclarations()[0]->uses().size(), 1);
  QCOMPARE(top->localDeclarations()[0]->uses().begin()->size(), 2);
  QCOMPARE(top->localDeclarations()[1]->uses().size(), 1);
  QCOMPARE(top->localDeclarations()[1]->uses().begin()->size(), 1);
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
                    "decltype((i)) x7;\n"
                    ;

  LockedTopDUContext top = parse(code, DumpAll);
  QVERIFY(top);
  DUChainReadLocker lock;

  // x1
  QCOMPARE(top->localDeclarations().size(), 13);
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
  // & due to additional parens, const because of const A*
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
  // already &, make sure it's not doubled due to additional parens, but must not be const
  Declaration* j2Dec = top->localDeclarations().at(10);
  Declaration* x6Dec = top->localDeclarations().at(11);
  QVERIFY(x6Dec->abstractType()->equals(j2Dec->abstractType().constData()));
  QVERIFY(!(x6Dec->abstractType()->modifiers() & AbstractType::ConstModifier));
  QVERIFY(x6Dec->abstractType().cast<ReferenceType>());
  QVERIFY(!(x6Dec->abstractType().cast<ReferenceType>()->baseType()->modifiers() & AbstractType::ConstModifier));
  QVERIFY(x6Dec->abstractType().cast<ReferenceType>()->baseType().cast<IntegralType>());
  QCOMPARE(x6Dec->abstractType().cast<ReferenceType>()->baseType().cast<IntegralType>()->dataType(),
           (uint) IntegralType::TypeInt);

  // x7
  // make sure it's & due to additional parens, but must not be const
  Declaration* x7Dec = top->localDeclarations().at(11);
  // x6 has the type that we want
  QVERIFY(x7Dec->abstractType()->equals(x6Dec->abstractType().constData()));
}

void TestDUChain::testDecltypeTypedef()
{
  LockedTopDUContext top = parse(readCodeFile("testDecltypeTypedef.cpp"), DumpNone);
  QVERIFY(top);

  QCOMPARE(top->localDeclarations().size(), 4);
  QCOMPARE(TypeUtils::unAliasedType(top->localDeclarations().at(2)->abstractType())->toString(), QString("int"));
  QCOMPARE(TypeUtils::unAliasedType(top->localDeclarations().at(3)->abstractType())->toString(), QString("int"));
}

void TestDUChain::testTrailingReturnType()
{
  {
    QByteArray code = "auto foo() -> int;\n";
    LockedTopDUContext top = parse(code, DumpAll);
    QVERIFY(top);
    DUChainReadLocker lock;

    QCOMPARE(top->localDeclarations().size(), 1);
    Declaration* dec = top->localDeclarations().first();
    QVERIFY(dynamic_cast<FunctionDeclaration*>(dec));
    QVERIFY(dec->abstractType());
    FunctionType::Ptr funcType = dec->abstractType().cast<FunctionType>();
    QVERIFY(funcType);
    QVERIFY(funcType->returnType());
    QVERIFY(funcType->returnType().cast<IntegralType>());
    QCOMPARE(funcType->returnType().cast<IntegralType>()->dataType(), (uint) IntegralType::TypeInt);
  }

  {
    QByteArray code = "class A { int x; }; auto foo(A* arg) -> decltype(arg->x);\n";
    LockedTopDUContext top = parse(code, DumpAll);
    QVERIFY(top);
    DUChainReadLocker lock;

    QCOMPARE(top->localDeclarations().size(), 2);
    Declaration* dec = top->localDeclarations().at(1);
    QVERIFY(dynamic_cast<FunctionDeclaration*>(dec));
    QVERIFY(dec->abstractType());
    FunctionType::Ptr funcType = dec->abstractType().cast<FunctionType>();
    QVERIFY(funcType);
    QVERIFY(funcType->returnType());
    qDebug() << funcType->returnType()->toString();
    QVERIFY(funcType->returnType().cast<IntegralType>());
    QCOMPARE(funcType->returnType().cast<IntegralType>()->dataType(), (uint) IntegralType::TypeInt);
  }

  {
    // example from the spec, 8.0/5
    QByteArray code = "auto f() -> int(*)[4];\n";
    LockedTopDUContext top = parse(code, DumpAll);
    QVERIFY(top);
    DUChainReadLocker lock;

    QCOMPARE(top->localDeclarations().size(), 1);
    Declaration* dec = top->localDeclarations().first();
    QVERIFY(dynamic_cast<FunctionDeclaration*>(dec));
    QVERIFY(dec->abstractType());
    FunctionType::Ptr funcType = dec->abstractType().cast<FunctionType>();
    QVERIFY(funcType);
    QVERIFY(funcType->returnType());
    qDebug() << funcType->returnType()->toString();
    QEXPECT_FAIL("", "type is parsed as 'array[4] of pointer to int, which is wrong.", Continue);
    QVERIFY(funcType->returnType().cast<PointerType>());
    /* TODO: uncomment once the above has been fixed
    QVERIFY(funcType->returnType().cast<PointerType>()->baseType().cast<ArrayType>());
    QCOMPARE(funcType->returnType().cast<PointerType>()->baseType().cast<ArrayType>()->dimension(), 4);
    QVERIFY(funcType->returnType().cast<PointerType>()->baseType().cast<ArrayType>()->elementType().cast<IntegralType>());
    QCOMPARE(funcType->returnType().cast<PointerType>()->baseType().cast<ArrayType>()->elementType().cast<IntegralType>()->dataType(),
             (uint) IntegralType::TypeInt);
    QCOMPARE(funcType->returnType().cast<IntegralType>()->dataType(), (uint) IntegralType::TypeInt);
    */
  }

  {
    // make sure we don't crash due to assertion on m_context in TypeASTVisitor ctor
    QByteArray code = "void func() { auto f = []() { return 1; }; }\n";
    LockedTopDUContext top = parse(code, DumpAll);
    QVERIFY(top);
    QByteArray code2 = "void func() { auto f = []() -> int { return 1; }; }\n";
    TopDUContext* top2 = parse(code2, DumpAll, top);
    QVERIFY(top2);
    QCOMPARE(top2, top.m_top);
  }
}

void TestDUChain::testConstexpr()
{
  const QByteArray code = "constexpr int square(int x) { return x * x; }\n"
                          "constexpr double a = 4.2 * square(2);\n"
                          "class A {\n"
                          "  constexpr A();\n"
                          "  constexpr int foo() { return 1; }\n"
                          "};\n";
  LockedTopDUContext top = parse(code, DumpAll);
  QVERIFY(top);
  DUChainReadLocker lock;

  QCOMPARE(top->localDeclarations().size(), 3);
  // square
  QEXPECT_FAIL("", "constexpr functions are not handled yet", Continue);
  QVERIFY(TypeUtils::isConstant(top->localDeclarations().at(0)->abstractType()));
  // double a
  QVERIFY(TypeUtils::isConstant(top->localDeclarations().at(1)->abstractType()));
  // class A
  QVERIFY(top->localDeclarations().at(2)->internalContext());
  DUContext* aCtx = top->localDeclarations().at(2)->internalContext();
  QCOMPARE(aCtx->localDeclarations().size(), 2);
  // A::A
  QEXPECT_FAIL("", "constexpr constructors are not handled yet", Continue);
  QVERIFY(TypeUtils::isConstant(aCtx->localDeclarations().at(0)->abstractType()));
  // A::foo
  QEXPECT_FAIL("", "constexpr member functions are not handled yet", Continue);
  QVERIFY(TypeUtils::isConstant(aCtx->localDeclarations().at(1)->abstractType()));
}

void TestDUChain::testBug284536()
{
  // see also: https://bugs.kde.org/show_bug.cgi?id=284536
  const QByteArray code = "template<typename T> struct A { typedef T type; };\n"
                          "template<typename T, typename... Args>\n"
                          "A<typename T<_Functor>::type(Args...)> func() {}\n";
  // baby don't crash me, oh no
  LockedTopDUContext top = parse(code, DumpAll);
  QVERIFY(top);
  DUChainReadLocker lock;
  QCOMPARE(top->localDeclarations().size(), 2);
}

void TestDUChain::testBug285004()
{
  // see also: https://bugs.kde.org/show_bug.cgi?id=285004
  // NOTE: I couldn't come up with something shorter - what a strange bug -.-'
  // source is gcc 4.5's tr1_impl/type_traits
  const QByteArray code = "namespace std {\n"
                          "  template<typename T> struct is_f;\n"
                          "  template<typename _Res, typename... Args> struct is_f<_Res(Args...)> { };\n"
                          "  template<typename _Res, typename... Args> struct is_f<_Res(Args......)> { };\n"
                          "  template<typename T> struct is_a : public i_c<(is_i<T>::value || is_i<T>::value)> { };\n"
                          "  template<typename T> struct is_f : public i_c<(is_a<T>::value)> { };\n"
                          "}\n";
  // baby don't crash me, oh no
  LockedTopDUContext top = parse(code, DumpAll);
  QVERIFY(top);
  DUChainReadLocker lock;
}

void TestDUChain::testLambda()
{
  // see also: https://bugs.kde.org/show_bug.cgi?id=279699
  const QByteArray code = "int main() {\n"
                          "  int i;\n"
                          "  auto f = [] (int i) { i = 0; };\n"
                          "}\n";
  LockedTopDUContext top = parse(code, DumpAll);
  QVERIFY(top);
  DUChainReadLocker lock;
  QVERIFY(top->problems().isEmpty());

  DUContext* mainCtx = top->childContexts().last();
  QCOMPARE(mainCtx->childContexts().size(), 2);
  // (int i)
  QCOMPARE(mainCtx->childContexts().first()->type(), DUContext::Function);
  QCOMPARE(mainCtx->childContexts().first()->range(), RangeInRevision(2, 15, 2, 20));
  // { i = 0; }
  QCOMPARE(mainCtx->childContexts().last()->type(), DUContext::Other);
  QCOMPARE(mainCtx->childContexts().last()->range(), RangeInRevision(2, 22, 2, 32));

  // int i; in main context
  QCOMPARE(mainCtx->localDeclarations().size(), 2);
  Declaration* iDecl = mainCtx->localDeclarations().at(0);
  // no uses
  QCOMPARE(iDecl->uses().size(), 0);

  // (int i) in lambda argument context
  QCOMPARE(mainCtx->childContexts().first()->localDeclarations().size(), 1);
  Declaration* iLambdaDecl = mainCtx->childContexts().first()->localDeclarations().first();
  QCOMPARE(iLambdaDecl->uses().size(), 1);
  QCOMPARE(iLambdaDecl->uses().begin()->size(), 1);
  QCOMPARE(iLambdaDecl->uses().begin()->first(), RangeInRevision(2, 24, 2, 25));

  Declaration* fDecl = mainCtx->localDeclarations().at(1);
  TypePtr< FunctionType > funType = fDecl->type<FunctionType>();
  QVERIFY(funType);
  QCOMPARE(funType->indexedArgumentsSize(), 1u);
  QVERIFY(funType->arguments().first()->equals(iLambdaDecl->abstractType().constData()));
  QVERIFY(funType->returnType());
  QVERIFY(funType->returnType().cast<IntegralType>());
  QCOMPARE(funType->returnType().cast<IntegralType>()->dataType(), (uint) IntegralType::TypeVoid);
}

void TestDUChain::testLambdaReturn()
{
  // see also: https://bugs.kde.org/show_bug.cgi?id=279699
  const QByteArray code = "int main() {\n"
                          "  auto f = [] () -> int { return 1 };\n"
                          "}\n";
  LockedTopDUContext top = parse(code, DumpAll);
  QVERIFY(top);
  DUChainReadLocker lock;
  QVERIFY(top->problems().isEmpty());

  DUContext* mainCtx = top->childContexts().last();
  QCOMPARE(mainCtx->localDeclarations().size(), 1);
  Declaration* fDecl = mainCtx->localDeclarations().at(0);
  TypePtr< FunctionType > funType = fDecl->type<FunctionType>();
  QVERIFY(funType->returnType());
  QVERIFY(funType->returnType().cast<IntegralType>());
  QCOMPARE(funType->returnType().cast<IntegralType>()->dataType(), (uint) IntegralType::TypeInt);
}

void TestDUChain::testLambdaCapture()
{
  // see also: https://bugs.kde.org/show_bug.cgi?id=279699
  const QByteArray code = "int main() {\n"
                          "  int i;\n"
                          "  auto f = [&i] { i = 0; };\n"
                          "}\n";
  LockedTopDUContext top = parse(code, DumpAll);
  QVERIFY(top);
  DUChainReadLocker lock;
  QVERIFY(top->problems().isEmpty());

  DUContext* mainCtx = top->childContexts().last();
  QCOMPARE(mainCtx->childContexts().size(), 1);
  // { i = 0; }
  QCOMPARE(mainCtx->childContexts().last()->type(), DUContext::Other);
  QCOMPARE(mainCtx->childContexts().last()->range(), RangeInRevision(2, 16, 2, 26));

  // int i; in main context
  QCOMPARE(mainCtx->localDeclarations().size(), 2);
  Declaration* iDecl = mainCtx->localDeclarations().at(0);
  QCOMPARE(iDecl->uses().size(), 1);
  QCOMPARE(iDecl->uses().begin()->size(), 2);
  QCOMPARE(iDecl->uses().begin()->first(), RangeInRevision(2, 13, 2, 14));
  QCOMPARE(iDecl->uses().begin()->last(), RangeInRevision(2, 18, 2, 19));

  Declaration* fDecl = mainCtx->localDeclarations().at(1);
  TypePtr< FunctionType > funType = fDecl->type<FunctionType>();
  QVERIFY(funType);
  QCOMPARE(funType->indexedArgumentsSize(), 0u);
  QVERIFY(funType->returnType());
  QVERIFY(funType->returnType().cast<IntegralType>());
  QCOMPARE(funType->returnType().cast<IntegralType>()->dataType(), (uint) IntegralType::TypeVoid);
}

void TestDUChain::testTemplateSpecializeVolatile()
{
  // see also: https://bugs.kde.org/show_bug.cgi?id=321885
  const QByteArray code(
    "template<typename _Tp>\n"
    "struct numeric_limits { };\n"
    "\n"
    "template<typename _Tp>\n"
    "  struct numeric_limits<const _Tp>\n"
    "  : public numeric_limits<_Tp> { };\n"
    "\n"
    "template<typename _Tp>\n"
    "  struct numeric_limits<volatile _Tp>\n"
    "  : public numeric_limits<_Tp> { };\n"
    "\n"
    "template<typename _Tp>\n"
    "  struct numeric_limits<const volatile _Tp>\n"
    "  : public numeric_limits<_Tp> { };\n"
    "\n"
    "template<>\n"
    "struct numeric_limits<double>\n"
    "{\n"
    "  static double epsilon();\n"
    "};\n"
    "\n"
    "int main() {\n"
    "  numeric_limits< double > a;\n"
    "  a.epsilon();\n"
    "  numeric_limits< const double > b;\n"
    "  b.epsilon();\n"
    "  numeric_limits< volatile double > c;\n"
    "  c.epsilon();\n"
    "  numeric_limits< const volatile double > d;\n"
    "  d.epsilon();\n"
    "}\n"
  );
  LockedTopDUContext top = parse(code, DumpAll);
  QVERIFY(top);
  DUChainReadLocker lock;
  QVERIFY(top->problems().isEmpty());

  QCOMPARE(top->localDeclarations().size(), 6);

  TemplateDeclaration* tplDec = dynamic_cast<TemplateDeclaration*>(top->localDeclarations().at(0));
  QVERIFY(tplDec);
  QCOMPARE(tplDec->specializationsSize(), 4u);
  QCOMPARE(tplDec->instantiations().size(), 7);

  int declIdx = 1;
  foreach(const QString id, QStringList() << "const _Tp" << "volatile _Tp" << "const volatile _Tp") {
    Declaration* specRaw = top->localDeclarations().at(declIdx++);
    TemplateDeclaration* spec = dynamic_cast<TemplateDeclaration*>(specRaw);
    QVERIFY(spec);
    QVERIFY(spec->specializedFrom().isValid());
    QVERIFY(spec->specializedWith().isValid());
    InstantiationInformation info = spec->specializedWith().information();
    QCOMPARE(info.templateParametersSize(), 1u);
    QVERIFY(info.templateParameters()[0].isValid());
    AbstractType::Ptr specParam = info.templateParameters()[0].abstractType();
    QVERIFY(specParam);
    QCOMPARE(specParam->toString(), id);
  }

  QCOMPARE(top->childContexts().last()->localDeclarations().size(), 4);
  QCOMPARE(top->childContexts().last()->localDeclarations().at(0)->abstractType()->toString(),
           QString("numeric_limits< double >"));
  QVERIFY(top->childContexts().last()->findUseAt(CursorInRevision(23, 6)) != -1);
  QEXPECT_FAIL("", "Delegation of the const/volatile to the non-const/volatile does not work", Abort);
  QCOMPARE(top->childContexts().last()->localDeclarations().at(1)->abstractType()->toString(),
           QString("numeric_limits< const double >"));
  QCOMPARE(top->childContexts().last()->localDeclarations().at(2)->abstractType()->toString(),
           QString("numeric_limits< volatile double >"));
  QCOMPARE(top->childContexts().last()->localDeclarations().at(3)->abstractType()->toString(),
           QString("numeric_limits< const volatile double >"));
  // should all work
  QVERIFY(top->childContexts().last()->findUseAt(CursorInRevision(25, 6)) != -1);
  QVERIFY(top->childContexts().last()->findUseAt(CursorInRevision(27, 6)) != -1);
  QVERIFY(top->childContexts().last()->findUseAt(CursorInRevision(29, 6)) != -1);
}

void TestDUChain::testTemplateSpecializeArray()
{
  // see also: https://bugs.kde.org/show_bug.cgi?id=294306
  const QByteArray code(
    "template <typename T>\n"
    "class test\n"
    "{\n"
    "public:\n"
    "    void foo() {}\n"
    "};\n"
    "template<typename T>\n"
    "class test<T[]>\n"
    "{\n"
    "};\n"
    "int main() {\n"
    "    test<int> t1;\n"
    "    // should work:\n"
    "    t1.foo();\n"
    "    test<int[]> t2;\n"
    "    // should not work:\n"
    "    t2.foo();\n"
    "\n"
    "    return 0;\n"
    "}\n"
  );
  LockedTopDUContext top = parse(code, DumpAll);
  QVERIFY(top);
  DUChainReadLocker lock;
  QVERIFY(top->problems().isEmpty());

  QCOMPARE(top->localDeclarations().size(), 3);

  Declaration* tplDecRaw = top->localDeclarations().at(0);
  TemplateDeclaration* tplDec = dynamic_cast<TemplateDeclaration*>(tplDecRaw);
  QVERIFY(tplDec);
  QCOMPARE(tplDec->specializationsSize(), 1u);
  QCOMPARE(tplDec->instantiations().size(), 2);

  Declaration* specRaw = top->localDeclarations().at(1);
  TemplateDeclaration* spec = dynamic_cast<TemplateDeclaration*>(specRaw);
  QVERIFY(spec);
  QVERIFY(spec->specializedFrom().isValid());
  QVERIFY(spec->specializedWith().isValid());
  InstantiationInformation info = spec->specializedWith().information();
  QCOMPARE(info.templateParametersSize(), 1u);
  QVERIFY(info.templateParameters()[0].isValid());
  AbstractType::Ptr specParam = info.templateParameters()[0].abstractType();
  QVERIFY(specParam);
  QVERIFY(specParam.cast<ArrayType>());
  QCOMPARE(specParam->toString(), QString("T[]"));

  QCOMPARE(top->childContexts().last()->localDeclarations().size(), 2);
  QCOMPARE(top->childContexts().last()->localDeclarations().at(0)->abstractType()->toString(),
           QString("test< int >"));
  QCOMPARE(top->childContexts().last()->localDeclarations().at(1)->abstractType()->toString(),
           QString("test< int[] >"));
  // should work
  QVERIFY(top->childContexts().last()->findUseAt(CursorInRevision(13, 9)) != -1);
  // should not work
  QVERIFY(top->childContexts().last()->findUseAt(CursorInRevision(16, 9)) == -1);
}

void TestDUChain::testTemplateSpecializeRValue()
{
  const QByteArray code(
    "template <typename T>\n"
    "class test\n"
    "{\n"
    "public:\n"
    "    void foo() {}\n"
    "};\n"
    "template<typename T>\n"
    "class test<T&&>\n"
    "{\n"
    "};\n"
    "int main() {\n"
    "    test<int> t1;\n"
    "    // should work:\n"
    "    t1.foo();\n"
    "    test<int&&> t2;\n"
    "    // should not work:\n"
    "    t2.foo();\n"
    "\n"
    "    return 0;\n"
    "}\n"
  );
  LockedTopDUContext top = parse(code, DumpAll);
  QVERIFY(top);
  DUChainReadLocker lock;
  QVERIFY(top->problems().isEmpty());

  QCOMPARE(top->localDeclarations().size(), 3);

  Declaration* tplDecRaw = top->localDeclarations().at(0);
  TemplateDeclaration* tplDec = dynamic_cast<TemplateDeclaration*>(tplDecRaw);
  QVERIFY(tplDec);
  QCOMPARE(tplDec->specializationsSize(), 1u);
  QCOMPARE(tplDec->instantiations().size(), 2);

  Declaration* specRaw = top->localDeclarations().at(1);
  TemplateDeclaration* spec = dynamic_cast<TemplateDeclaration*>(specRaw);
  QVERIFY(spec);
  QVERIFY(spec->specializedFrom().isValid());
  QVERIFY(spec->specializedWith().isValid());
  InstantiationInformation info = spec->specializedWith().information();
  QCOMPARE(info.templateParametersSize(), 1u);
  QVERIFY(info.templateParameters()[0].isValid());
  AbstractType::Ptr specParam = info.templateParameters()[0].abstractType();
  QVERIFY(specParam);
  QVERIFY(specParam.cast<ReferenceType>());
  QVERIFY(specParam.cast<ReferenceType>()->isRValue());
  QCOMPARE(specParam->toString(), QString("T&&"));
  QCOMPARE(Cpp::simplifiedTypeString(specParam, top), QString("T&&"));

  QCOMPARE(top->childContexts().last()->localDeclarations().size(), 2);
  QCOMPARE(top->childContexts().last()->localDeclarations().at(0)->abstractType()->toString(),
           QString("test< int >"));
  QCOMPARE(top->childContexts().last()->localDeclarations().at(1)->abstractType()->toString(),
           QString("test< int&& >"));

  // should work
  QVERIFY(top->childContexts().last()->findUseAt(CursorInRevision(13, 9)) != -1);
  // should not work
  QVERIFY(top->childContexts().last()->findUseAt(CursorInRevision(16, 9)) == -1);
}

void TestDUChain::testAliasDeclaration()
{
  const QByteArray code(
    "struct foo { static void asdf(); };\n"
    "using bar = foo;\n"
    "void t() { bar::asdf(); }"
  );
  LockedTopDUContext top = parse(code, DumpAll);
  QVERIFY(top);
  DUChainReadLocker lock;
  QVERIFY(top->problems().isEmpty());

  QCOMPARE(top->localDeclarations().count(), 3);

  // foo
  Declaration* dec = top->localDeclarations().at(0);
  QCOMPARE(dec->kind(), Declaration::Type);
  QCOMPARE(dec->uses().count(), 1);
  QCOMPARE(dec->uses().begin()->count(), 1);
  QCOMPARE(dec->uses().begin()->first().start.line, 1);

  // bar
  dec = top->localDeclarations().at(1);
  QVERIFY(dec->isTypeAlias());
  QCOMPARE(dec->uses().count(), 1);
  QCOMPARE(dec->uses().begin()->count(), 1);
  QCOMPARE(dec->uses().begin()->first().start.line, 2);

  // foo::asdf
  dec = top->childContexts().first()->localDeclarations().at(0);
  QCOMPARE(dec->uses().count(), 1);
  QCOMPARE(dec->uses().begin()->count(), 1);
  QCOMPARE(dec->uses().begin()->first().start.line, 2);
}

void TestDUChain::testAuto()
{
  const QByteArray code(
    "char i = 1;\n"
    "auto a1 = i;\n"
    "auto& a2 = i;\n"
    "auto&& a3 = i;\n"
    "const auto a4 = i;\n"
    "const auto& a5 = i;\n"
    "const auto&& a6 = i;\n"
    "auto a7(1.1d);\n"
    "auto a8(i);\n"
    "auto a9 /** test **/ (i);\n"
    "auto a10(a2);\n"
    "auto a11(a3);\n"
    "auto a12(a4);\n"
    "auto a13(a5);\n"
    "auto a14(a6);\n"
    "void f() {\n"
    "  if (auto c1 = 0.0) {}\n"
    "}\n"
    "struct FOO {};\n"
    "auto a15 = FOO{};\n"
    "auto a16 = FOO{1};\n"
    "auto a17 = new int;\n"
    "auto* a18 = new int;\n"
    "auto a19 = new FOO;\n"
    "auto* a20 = new FOO;\n"

  );
  LockedTopDUContext top = parse(code, DumpAll);
  QVERIFY(top);
  DUChainReadLocker lock;
  QVERIFY(top->problems().isEmpty());

  QCOMPARE(top->localDeclarations().count(), 23);

  Declaration* dec = top->localDeclarations().at(1);
  QVERIFY(dec->type<IntegralType>());
  QCOMPARE(dec->type<IntegralType>()->dataType(), (uint) IntegralType::TypeChar);

  dec = top->localDeclarations().at(2);
  QVERIFY(dec->type<ReferenceType>());
  QVERIFY(!dec->type<ReferenceType>()->isRValue());
  QVERIFY(dec->type<ReferenceType>()->baseType().cast<IntegralType>());
  QCOMPARE(dec->type<ReferenceType>()->baseType().cast<IntegralType>()->dataType(), (uint) IntegralType::TypeChar);

  dec = top->localDeclarations().at(3);
  QVERIFY(dec->type<ReferenceType>());
  QVERIFY(dec->type<ReferenceType>()->isRValue());
  QVERIFY(dec->type<ReferenceType>()->baseType().cast<IntegralType>());
  QCOMPARE(dec->type<ReferenceType>()->baseType().cast<IntegralType>()->dataType(), (uint) IntegralType::TypeChar);

  dec = top->localDeclarations().at(4);
  QVERIFY(dec->type<IntegralType>());
  QVERIFY(dec->abstractType()->modifiers() & AbstractType::ConstModifier);
  QCOMPARE(dec->type<IntegralType>()->dataType(), (uint) IntegralType::TypeChar);

  dec = top->localDeclarations().at(5);
  QVERIFY(dec->type<ReferenceType>());
  QVERIFY(dec->type<ReferenceType>()->baseType()->modifiers() & AbstractType::ConstModifier);
  QVERIFY(!dec->type<ReferenceType>()->isRValue());
  QVERIFY(dec->type<ReferenceType>()->baseType().cast<IntegralType>());
  QCOMPARE(dec->type<ReferenceType>()->baseType().cast<IntegralType>()->dataType(), (uint) IntegralType::TypeChar);

  dec = top->localDeclarations().at(6);
  QVERIFY(dec->type<ReferenceType>());
  QVERIFY(dec->type<ReferenceType>()->baseType()->modifiers() & AbstractType::ConstModifier);
  QVERIFY(dec->type<ReferenceType>()->isRValue());
  QVERIFY(dec->type<ReferenceType>()->baseType().cast<IntegralType>());
  QCOMPARE(dec->type<ReferenceType>()->baseType().cast<IntegralType>()->dataType(), (uint) IntegralType::TypeChar);

  dec = top->localDeclarations().at(7);
  QVERIFY(dec->type<IntegralType>());
  QCOMPARE(dec->type<IntegralType>()->dataType(), (uint) IntegralType::TypeDouble);

  for (int i = 8; i < 15; ++i) {
    dec = top->localDeclarations().at(i);
    QVERIFY(dec->type<IntegralType>());
    QCOMPARE(dec->type<IntegralType>()->dataType(), (uint) IntegralType::TypeChar);
    QCOMPARE(dec->abstractType()->modifiers(), (quint64) AbstractType::NoModifiers);
  }

  dec = top->localDeclarations().at(15)->internalContext()->childContexts().first()->findDeclarations(Identifier("c1")).first();
  QVERIFY(dec->type<IntegralType>());
  QCOMPARE(dec->type<IntegralType>()->dataType(), (uint) IntegralType::TypeDouble);

  StructureType::Ptr foo = top->localDeclarations().at(16)->type<StructureType>();
  QVERIFY(foo);

  dec = top->localDeclarations().at(17);
  QVERIFY(dec->abstractType()->equals(foo.constData()));

  dec = top->localDeclarations().at(18);
  QVERIFY(dec->abstractType()->equals(foo.constData()));

  dec = top->localDeclarations().at(19);
  QVERIFY(dec->abstractType().cast<PointerType>());
  QVERIFY(dec->abstractType().cast<PointerType>()->baseType().cast<IntegralType>());

  dec = top->localDeclarations().at(20);
  QVERIFY(dec->abstractType().cast<PointerType>());
  QVERIFY(dec->abstractType().cast<PointerType>()->baseType().cast<IntegralType>());

  dec = top->localDeclarations().at(21);
  QVERIFY(dec->abstractType().cast<PointerType>());
  QVERIFY(dec->abstractType().cast<PointerType>()->baseType()->equals(foo.constData()));

  dec = top->localDeclarations().at(22);
  qDebug() << dec->toString();
  QVERIFY(dec->abstractType().cast<PointerType>());
  QVERIFY(dec->abstractType().cast<PointerType>()->baseType()->equals(foo.constData()));
}

void TestDUChain::testNoexcept()
{
    const QByteArray code(
    "void f1() noexcept;\n"
    "void f2() noexcept(false);\n"
    "class a { void m1() noexcept; void m2() noexcept(true); };\n"
    "void asdf() { bool a = noexcept(true); }\n"
  );
  LockedTopDUContext top = parse(code, DumpAll);
  QVERIFY(top);
  DUChainReadLocker lock;
  QVERIFY(top->problems().isEmpty());
}

void TestDUChain::testInlineNamespace()
{
    const QByteArray code(
      "inline namespace a { void foo (); }\n"
      "int main() {\n"
      "  a::foo();\n"
      "  foo();\n"
      "}\n"
  );
  LockedTopDUContext top = parse(code, DumpAll);
  QVERIFY(top);
  DUChainReadLocker lock;
  QVERIFY(top->problems().isEmpty());

  Declaration* foo = top->childContexts().first()->localDeclarations().first();
  QCOMPARE(foo->uses().size(), 1);
  QCOMPARE(foo->uses().begin()->size(), 2);
  QCOMPARE(foo->uses().begin()->at(0), RangeInRevision(2, 5, 2, 8));
  QCOMPARE(foo->uses().begin()->at(1), RangeInRevision(3, 2, 3, 5));
}
