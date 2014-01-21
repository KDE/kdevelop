/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda<rodda@kde.org>
    Copyright 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "test_cppcodecompletion.h"

#include <QtTest/QtTest>
#include <typeinfo>

#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/forwarddeclaration.h>
#include <language/duchain/declaration.h>
#include <language/duchain/aliasdeclaration.h>
#include <language/editor/documentrange.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <language/duchain/dumpchain.h>
#include "declarationbuilder.h"
#include "usebuilder.h"
#include "cppeditorintegrator.h"
#include "dumptypes.h"
#include "environmentmanager.h"

#include "tokens.h"
#include "parsesession.h"

#include "rpp/preprocessor.h"
#include "rpp/pp-engine.h"
#include "rpp/pp-environment.h"
#include "expressionvisitor.h"
#include "expressionparser.h"
#include "codecompletion/context.h"
#include "codecompletion/helpers.h"
#include "codecompletion/item.h"
#include "codecompletion/implementationhelperitem.h"
#include "cpppreprocessenvironment.h"
#include <language/duchain/classdeclaration.h>
#include "cppduchain/missingdeclarationproblem.h"
#include "cppduchain/missingdeclarationassistant.h"
#include "overloadresolutionhelper.h"
#include <language/duchain/functiondefinition.h>

#include <language/codecompletion/codecompletiontesthelper.h>
#include <language/duchain/persistentsymboltable.h>
#include <cpputils.h>

#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include <KTempDir>
#include <KTextEditor/Editor>
#include <KTextEditor/View>

using namespace KTextEditor;

using namespace KDevelop;

QTEST_MAIN(TestCppCodeCompletion)

QString testFile1 = "class Erna; struct Honk { int a,b; enum Enum { Number1, Number2 }; Erna& erna; operator int() {}; }; struct Pointer { Honk* operator ->() const {}; Honk& operator * () {}; }; Honk globalHonk; Honk honky; \n#define HONK Honk\n";

QString testFile2 = "struct Honk { int a,b; enum Enum { Number1, Number2 }; Erna& erna; operator int() {}; }; struct Erna { Erna( const Honk& honk ) {} }; struct Heinz : public Erna {}; Erna globalErna; Heinz globalHeinz; int globalInt; Heinz globalFunction(const Heinz& h ) {}; Erna globalFunction( const Erna& erna); Honk globalFunction( const Honk&, const Heinz& h ) {}; int globalFunction(int ) {}; HONK globalMacroHonk; struct GlobalClass { Heinz function(const Heinz& h ) {}; Erna function( const Erna& erna);  }; GlobalClass globalClass;\n#undef HONK\n";

QString testFile3 = "struct A {}; struct B : public A {};";

QString testFile4 = "void test1() {}; class TestClass() { TestClass() {} };";

typedef CodeCompletionItemTester<Cpp::CodeCompletionContext> CompletionItemTester;

#define TEST_FILE_PARSE_ONLY if (testFileParseOnly) QSKIP("Skip", SkipSingle);
TestCppCodeCompletion::TestCppCodeCompletion()
{
  testFileParseOnly = false;
}

void TestCppCodeCompletion::initTestCase()
{
  AutoTestShell::init(QStringList() << "kdevcppsupport");
  TestCore::initialize(Core::NoUi);
  Cpp::EnvironmentManager::init();

  DUChain::self()->disablePersistentStorage();
  typeInt = AbstractType::Ptr(new IntegralType(IntegralType::TypeInt));

  addInclude( "testFile1.h", testFile1 );
  addInclude( "testFile2.h", testFile2 );
  addInclude( "testFile3.h", testFile3 );
}

void TestCppCodeCompletion::cleanupTestCase()
{
  TestCore::shutdown();
}

Declaration* TestCppCodeCompletion::findDeclaration(DUContext* context, const Identifier& id, const CursorInRevision& position)
{
  QList<Declaration*> ret = context->findDeclarations(id, position);
  if (ret.count())
    return ret.first();
  return 0;
}

Declaration* TestCppCodeCompletion::findDeclaration(DUContext* context, const QualifiedIdentifier& id, const CursorInRevision& position)
{
  QList<Declaration*> ret = context->findDeclarations(id, position);
  if (ret.count())
    return ret.first();
  return 0;
}

void TestCppCodeCompletion::testCommentClearing()
{
  QByteArray method = "struct a { int i; }; int foo() { a inst; }";
  TopDUContext* top = parse(method, DumpNone);
  int ctxt = 2;
  DUChainWriteLocker lock(DUChain::lock());
  CompletionItemTester test(top->childContexts()[ctxt], "//* \n inst.");
  QCOMPARE(test.names, QStringList() << "i");
  CompletionItemTester test2(top->childContexts()[ctxt], "// \n inst.");
  QCOMPARE(test2.names, QStringList() << "i");
  CompletionItemTester test3(top->childContexts()[ctxt], "/*//*/ inst.");
  QCOMPARE(test3.names, QStringList() << "i");
  CompletionItemTester test4(top->childContexts()[ctxt], " ///*//*/ \n inst.");
  QCOMPARE(test4.names, QStringList() << "i");
  CompletionItemTester test5(top->childContexts()[ctxt], "/*// inst.");
  QCOMPARE(test5.names, QStringList());
  CompletionItemTester test6(top->childContexts()[ctxt], "// inst.");
  QCOMPARE(test6.names, QStringList());
  CompletionItemTester test7(top->childContexts()[ctxt], "/*// \n/* // \n */ inst.");
  QCOMPARE(test7.names, QStringList() << "i");
  CompletionItemTester test8(top->childContexts()[ctxt], "// \n /*// \n/* // \n */ /*/*/ inst.");
  QCOMPARE(test8.names, QStringList() << "i");
  release(top);
}

void TestCppCodeCompletion::testExpressionBefore()
{
  QString exp1 = "int x";
  QCOMPARE(Cpp::expressionBefore(exp1, exp1.length()), 4);
  QString exp2 = "x = x().";
  QCOMPARE(Cpp::expressionBefore(exp2, exp2.length()), 4);
  QString exp3 = "x = (x().)";
  QCOMPARE(Cpp::expressionBefore(exp3, exp3.length()), 4);
  QString exp4 = "x = ((x().))";
  QCOMPARE(Cpp::expressionBefore(exp4, exp4.length()), 4);
  QString exp5 = "x = asdfasdf";
  QCOMPARE(Cpp::expressionBefore(exp5, exp5.length()), 4);
  QString exp6 = "++asdfasdf";
  QCOMPARE(Cpp::expressionBefore(exp6, exp6.length()), 2);
  QString exp7 = "(asd)";
  QCOMPARE(Cpp::expressionBefore(exp7, exp7.length()), 0);
  QString exp8 = "x = x[]->";
  QCOMPARE(Cpp::expressionBefore(exp8, exp8.length()), 4);
  QString exp9 = "x = x()::";
  QCOMPARE(Cpp::expressionBefore(exp9, exp9.length()), 4);
  QString exp10 = "x = x.y.z->x::a[]->h()[q]::S<n>";
  QCOMPARE(Cpp::expressionBefore(exp10, exp10.length()), 4);
  QString exp11 = "x >";
  QCOMPARE(Cpp::expressionBefore(exp11, exp11.length()), 3);
  QString exp12 = "x = *(*y)";
  QCOMPARE(Cpp::expressionBefore(exp12, exp12.length()), 5);
  QString exp13 = "q > xyz";
  QCOMPARE(Cpp::expressionBefore(exp13, exp13.length()), 4);
  QString exp14 = "q>x()[asdf].";
  QCOMPARE(Cpp::expressionBefore(exp14, exp14.length()), 2);
  QString exp15 = "x = y(\"(\")";
  QCOMPARE(Cpp::expressionBefore(exp15, exp15.length()), 4);
  QString exp16 = "x = y<\"<\">";
  QCOMPARE(Cpp::expressionBefore(exp16, exp16.length()), 4);
  QString exp17 = "x = y[z(\"[\")]";
  QCOMPARE(Cpp::expressionBefore(exp17, exp17.length()), 4);
  QString exp18 = "x = y(";
  QCOMPARE(Cpp::expressionBefore(exp18, exp18.length()), 6);
  QString exp19 = "x = y[";
  QCOMPARE(Cpp::expressionBefore(exp19, exp19.length()), 6);
  QString exp20 = "x = y<";
  QCOMPARE(Cpp::expressionBefore(exp20, exp20.length()), 6);
  QString exp21 = " ";
  QCOMPARE(Cpp::expressionBefore(exp21, exp21.length()), 1);
  QString exp22 = "";
  QCOMPARE(Cpp::expressionBefore(exp22, exp22.length()), 0);
  QString exp23 = "   ";
  QCOMPARE(Cpp::expressionBefore(exp23, exp23.length()), 3);
}

void TestCppCodeCompletion::testSpecialItems()
{
  QByteArray method = "enum Color { Red = 0, Green = 1, Blue = 2 }; void test(Color c) { }";
  TopDUContext* top = parse(method, DumpNone);
  int ctxt = 2;
  //There is duplication here, but in this case we want all the enum values added in their own group here
  //It's probably not worth it to go through the list and remove the previously added enums when in scope
  const QStringList enumGroupCompletions(QStringList() << "Red" << "Green" << "Blue");
  DUChainWriteLocker lock(DUChain::lock());
  CompletionItemTester test(top->childContexts()[ctxt], "c = ");
  qDebug() << "actual names:::" << test.names;
  QCOMPARE(test.names, QStringList() << "Color c =" << "c" << "Color" << "test" << "Red" << "Green" << "Blue" << enumGroupCompletions);
  CompletionItemTester test2(top->childContexts()[ctxt], "test(");
  QCOMPARE(test2.names, QStringList() << "test" << "c" << "Color" << "test" << "Red" << "Green" << "Blue" << enumGroupCompletions);
  CompletionItemTester test3(top->childContexts()[ctxt], "if (c == ");
  QCOMPARE(test3.names, QStringList() << "Color c ==" << "c" << "Color" << "test" << "Red" << "Green" << "Blue" << enumGroupCompletions);
  CompletionItemTester test4(top->childContexts()[ctxt], "if (c > ");
  QCOMPARE(test4.names, QStringList() << "Color c >" << "c" << "Color" << "test" << "Red" << "Green" << "Blue" << enumGroupCompletions);
  CompletionItemTester test5(top->childContexts()[ctxt], "c -= ");
  QCOMPARE(test5.names, QStringList() << "Color c -=" << "c" << "Color" << "test" << "Red" << "Green" << "Blue" << enumGroupCompletions);
  release(top);
}

void TestCppCodeCompletion::testOnlyShow()
{
  QByteArray method = "template<class T1> class T { }; namespace A { struct B {}; } struct C { }; int testMe() { }";
  TopDUContext* top = parse(method, DumpAll);
  int fctxt = 5;
  int sctxt = 3;
  int nctxt = 2;
  DUChainWriteLocker lock(DUChain::lock());

  CompletionItemTester implementationHelpers(top, "void ");
  QVERIFY(implementationHelpers.completionContext->onlyShow() == Cpp::CodeCompletionContext::ShowImplementationHelpers);

  CompletionItemTester showTypes(top->childContexts()[fctxt], "A::B* b = new ");
  QVERIFY(showTypes.completionContext->onlyShow() == Cpp::CodeCompletionContext::ShowTypes);
  CompletionItemTester showTypes2(top->childContexts()[fctxt], "A::B* b = const_cast< ");
  QVERIFY(showTypes2.completionContext->onlyShow() == Cpp::CodeCompletionContext::ShowTypes);
  CompletionItemTester showTypes3(top->childContexts()[fctxt], "A::B* b = static_cast< ");
  QVERIFY(showTypes3.completionContext->onlyShow() == Cpp::CodeCompletionContext::ShowTypes);
  CompletionItemTester showTypes4(top->childContexts()[fctxt], "A::B* b = dynamic_cast< ");
  QVERIFY(showTypes4.completionContext->onlyShow() == Cpp::CodeCompletionContext::ShowTypes);
  CompletionItemTester showTypes5(top->childContexts()[fctxt], "A::B* b = reinterpret_cast< ");
  QVERIFY(showTypes5.completionContext->onlyShow() == Cpp::CodeCompletionContext::ShowTypes);
  CompletionItemTester showTypes6(top->childContexts()[fctxt], "const ");
  QVERIFY(showTypes6.completionContext->onlyShow() == Cpp::CodeCompletionContext::ShowTypes);
  CompletionItemTester showTypes7(top, "typedef ");
  QVERIFY(showTypes7.completionContext->onlyShow() == Cpp::CodeCompletionContext::ShowTypes);
  CompletionItemTester showTypes8(top->childContexts()[sctxt], "public  ");
  QVERIFY(showTypes8.completionContext->onlyShow() == Cpp::CodeCompletionContext::ShowTypes);
  CompletionItemTester showTypes9(top->childContexts()[sctxt], "protected ");
  QVERIFY(showTypes9.completionContext->onlyShow() == Cpp::CodeCompletionContext::ShowTypes);
  CompletionItemTester showTypes10(top->childContexts()[sctxt], "private ");
  QVERIFY(showTypes10.completionContext->onlyShow() == Cpp::CodeCompletionContext::ShowTypes);
  CompletionItemTester showTypes11(top->childContexts()[sctxt], "virtual ");
  QVERIFY(showTypes11.completionContext->onlyShow() == Cpp::CodeCompletionContext::ShowTypes);

  CompletionItemTester showTypes12(top->childContexts()[fctxt], "T<");
  QVERIFY(showTypes12.completionContext->onlyShow() == Cpp::CodeCompletionContext::ShowTypes);

  CompletionItemTester showTypes13(top, "");
  QVERIFY(showTypes13.completionContext->onlyShow() == Cpp::CodeCompletionContext::ShowTypes);
  CompletionItemTester showTypes14(top->childContexts()[sctxt], "");
  QVERIFY(showTypes14.completionContext->onlyShow() == Cpp::CodeCompletionContext::ShowTypes);
  CompletionItemTester showTypes15(top->childContexts()[nctxt], "");
  QVERIFY(showTypes15.completionContext->onlyShow() == Cpp::CodeCompletionContext::ShowTypes);

  CompletionItemTester dontShowTypes(top, "int i = ");
  QVERIFY(dontShowTypes.completionContext->onlyShow() == Cpp::CodeCompletionContext::ShowAll);
  CompletionItemTester dontShowTypes2(top->childContexts()[sctxt], "int i = ");
  QVERIFY(dontShowTypes2.completionContext->onlyShow() == Cpp::CodeCompletionContext::ShowAll);
  CompletionItemTester dontShowTypes3(top->childContexts()[nctxt], "int i = ");
  QVERIFY(dontShowTypes3.completionContext->onlyShow() == Cpp::CodeCompletionContext::ShowAll);
  CompletionItemTester dontShowTypes4(top, "int i = i +");
  QVERIFY(dontShowTypes4.completionContext->onlyShow() == Cpp::CodeCompletionContext::ShowAll);
  CompletionItemTester dontShowTypes5(top->childContexts()[sctxt], "int i = i +");
  QVERIFY(dontShowTypes5.completionContext->onlyShow() == Cpp::CodeCompletionContext::ShowAll);
  CompletionItemTester dontShowTypes6(top->childContexts()[nctxt], "int i = i +");
  QVERIFY(dontShowTypes6.completionContext->onlyShow() == Cpp::CodeCompletionContext::ShowAll);

  release(top);
}

void TestCppCodeCompletion::testFriends()
{
  QByteArray method = "class Friendly{};";
  TopDUContext* top = parse(method, DumpNone);
  int ctxt = 0;
  DUChainWriteLocker lock(DUChain::lock());
  CompletionItemTester test(top->childContexts()[ctxt], "friend ");
  QVERIFY(test.completionContext->isValid());
  QVERIFY(test.completionContext->onlyShow() == Cpp::CodeCompletionContext::ShowTypes);
  CompletionItemTester test2(top->childContexts()[ctxt], "friend class ");
  QVERIFY(test2.completionContext->isValid());
  QVERIFY(test2.completionContext->onlyShow() == Cpp::CodeCompletionContext::ShowTypes);
  CompletionItemTester test3(top->childContexts()[ctxt], "class ");
  QVERIFY(!test3.completionContext->isValid());

  release(top);
}

void TestCppCodeCompletion::testInvalidContexts()
{
  QByteArray method = "namespace A { struct B {}; } int testMe() { }";
  TopDUContext* top = parse(method, DumpNone);
  int ctxt = 2;
  DUChainWriteLocker lock(DUChain::lock());

  //TODO: these expressions should generally result in an invalid context, but
  // the ExpressionParser doesn't seem to think that "asdf" is an invalid exp.
  // Either the expressionParser should be fixed, or testContextValidity() in
  // context.cpp should be updated

  CompletionItemTester invalidExp(top->childContexts()[ctxt], "asdf->");
  //Should be invalid (and is, but not as soon as it should be)
  QVERIFY(!invalidExp.completionContext->isValid());

  CompletionItemTester invalidExp2(top->childContexts()[ctxt], "asdf.");
  //Should be invalid (and is, but not as soon as it should be)
  QVERIFY(!invalidExp2.completionContext->isValid());

  CompletionItemTester invalidExp3(top->childContexts()[ctxt], "asdf::");
  //Should be valid in case it's a namespace, but asdf should eval to false
  QVERIFY(invalidExp3.completionContext->isValid());

  CompletionItemTester invalidExp31(top->childContexts()[ctxt], "A::");
  //Should be valid as namespace, but A should evaluate to false
  QVERIFY(invalidExp31.completionContext->isValid());

  CompletionItemTester invalidExp4(top->childContexts()[ctxt], "asdf(");
  //kept valid for MissingIncludeCompletion, but asdf should eval to false
  QVERIFY(invalidExp4.completionContext->parentContext()->isValid());

  CompletionItemTester invalidExp5(top->childContexts()[ctxt], "asdf<");
  //TODO: testContextValidity() says templateAccess should be kept valid even with invalid expression
  // as it could get MissingIncludeCompletion, but this is considered a "less than", since an invalid
  // expression will not create a templateAccess context
  QEXPECT_FAIL("", "Test mimics an unknown template expression, but there's no way to differentiate it from a less than operator with an unknown expression, which it gets considered to be. If it were a template, it would be valid.", Continue);
  QVERIFY(invalidExp5.completionContext->parentContext()->isValid());

  CompletionItemTester invalidExp6(top->childContexts()[ctxt], "asdf &&");
  QVERIFY(!invalidExp6.completionContext->parentContext()->isValid());
  CompletionItemTester invalidExp7(top->childContexts()[ctxt], "void "); 
  QVERIFY(!invalidExp7.completionContext->isValid());
  CompletionItemTester invalidExp71(top->childContexts()[ctxt], "int* ");
  QVERIFY(!invalidExp71.completionContext->isValid());
  CompletionItemTester invalidExp8(top->childContexts()[ctxt], "A::B &&");
  QVERIFY(!invalidExp8.completionContext->parentContext()->isValid());
  CompletionItemTester invalidExp9(top->childContexts()[ctxt], ".");
  QVERIFY(!invalidExp9.completionContext->isValid());
  CompletionItemTester invalidExp10(top->childContexts()[ctxt], "->");
  QVERIFY(!invalidExp10.completionContext->isValid());
  CompletionItemTester invalidExp11(top->childContexts()[ctxt], "::");
  QVERIFY(!invalidExp11.completionContext->isValid());
  release(top);
}

void TestCppCodeCompletion::testMemberAccess()
{
  QByteArray method = "template<class T1, class T2> class T { public: T1 ta(); class U { public: class V{ };  }; };"
                      "class X { public: X(){}; int a(int a, T<int,int> b); int b;};"
                      "T<int,int> t;"
                      "X* getX() { }"
                      "class Z { public: Z() = delete; static int a(int b); };";
  TopDUContext* top = parse(method, DumpNone);
  int ctxt = 4;
  DUChainWriteLocker lock(DUChain::lock());
  CompletionItemTester testDot(top->childContexts()[ctxt], "t.");
  QCOMPARE(testDot.names, QStringList() << "ta");
  QCOMPARE(testDot.completionContext->accessType(), Cpp::CodeCompletionContext::MemberAccess);
  CompletionItemTester testDot2(top->childContexts()[ctxt], "(*getX()).");
  QCOMPARE(testDot2.names, QStringList() << "a" << "b");
  QCOMPARE(testDot2.completionContext->accessType(), Cpp::CodeCompletionContext::MemberAccess);
  CompletionItemTester testArrow(top->childContexts()[ctxt], "getX()->");
  QCOMPARE(testArrow.names, QStringList() << "a" << "b");
  QCOMPARE(testArrow.completionContext->accessType(), Cpp::CodeCompletionContext::ArrowMemberAccess);
  CompletionItemTester testArrow2(top->childContexts()[ctxt], "(&t)->");
  QCOMPARE(testArrow2.names, QStringList() << "ta");
  QCOMPARE(testArrow2.completionContext->accessType(), Cpp::CodeCompletionContext::ArrowMemberAccess);
  CompletionItemTester testColons(top->childContexts()[ctxt], "X::");
  QCOMPARE(testColons.names, QStringList() << "X" << "a" << "b");
  QCOMPARE(testColons.completionContext->accessType(), Cpp::CodeCompletionContext::StaticMemberChoose);
  CompletionItemTester testColons2(top->childContexts()[ctxt], "T::U::");
  QCOMPARE(testColons2.names, QStringList() << "V");
  QCOMPARE(testColons2.completionContext->accessType(), Cpp::CodeCompletionContext::StaticMemberChoose);
  CompletionItemTester testDeleted(top->childContexts()[ctxt], "Z::");
  QCOMPARE(testDeleted.names, QStringList() << "a");
  QCOMPARE(testDeleted.completionContext->accessType(), Cpp::CodeCompletionContext::StaticMemberChoose);
  release(top);
}

void TestCppCodeCompletion::testParentContexts()
{
  //Binary operators (also parent contexts) are tested in testBinaryOperators
  QByteArray method = "template<class T1, class T2> class Templ { T1 ta(); T2 tb(); }; class X { X(); int a(int a, Templ<int,int> b); int b;}; X::X() { }";
  TopDUContext* top = parse(method, DumpAll);
  int ctxt = 4;
  DUChainWriteLocker lock(DUChain::lock());
  CompletionItemTester templateContext(top->childContexts()[ctxt], "Templ<");
  QCOMPARE(templateContext.names, QStringList() << "Templ" << "Templ" << "X");
  QCOMPARE(templateContext.completionContext->parentContext()->accessType(), Cpp::CodeCompletionContext::TemplateAccess);
  CompletionItemTester templateContext2(top->childContexts()[ctxt], "Templ<int, ");
  QCOMPARE(templateContext2.names, QStringList() << "Templ" << "Templ" << "X");
  QCOMPARE(templateContext2.completionContext->parentContext()->accessType(), Cpp::CodeCompletionContext::TemplateAccess);
  CompletionItemTester templateContext3(top->childContexts()[ctxt], "Templ< int , int > t = new Templ<");
  QCOMPARE(templateContext3.names, QStringList() << "Templ" << "Templ" << "X");
  QCOMPARE(templateContext3.completionContext->parentContext()->accessType(), Cpp::CodeCompletionContext::TemplateAccess);
  CompletionItemTester templateContext4(top->childContexts()[ctxt], "Templ< int , int > t = new Templ<int,");
  QCOMPARE(templateContext4.names, QStringList() << "Templ" << "Templ" << "X");
  QCOMPARE(templateContext4.completionContext->parentContext()->accessType(), Cpp::CodeCompletionContext::TemplateAccess);
  CompletionItemTester templateContext5(top->childContexts()[ctxt], "Templ< int , Templ<");
  QCOMPARE(templateContext5.names, QStringList() << "Templ" << "Templ" << "Templ" << "X");
  QCOMPARE(templateContext5.completionContext->parentContext()->accessType(), Cpp::CodeCompletionContext::TemplateAccess);
  QCOMPARE(templateContext5.completionContext->parentContext()->parentContext()->accessType(), Cpp::CodeCompletionContext::TemplateAccess);
  CompletionItemTester notATemplate(top->childContexts()[ctxt], "if ( a > b < a > this->");
  QCOMPARE(notATemplate.names, QStringList() << "a" << "b");
  QCOMPARE(notATemplate.completionContext->parentContext()->accessType(), Cpp::CodeCompletionContext::BinaryOpFunctionCallAccess);
  CompletionItemTester functionContext(top->childContexts()[ctxt], "a(");
  QCOMPARE(functionContext.names, QStringList() << "a" << "X" << "a" << "b" << "Templ" << "this");
  QCOMPARE(functionContext.completionContext->parentContext()->accessType(), Cpp::CodeCompletionContext::FunctionCallAccess);
  CompletionItemTester functionContext2(top->childContexts()[ctxt], "a(0, ");
  QCOMPARE(functionContext2.names, QStringList() << "a" << "X" << "a" << "b" << "Templ" << "this");
  QCOMPARE(functionContext2.completionContext->parentContext()->accessType(), Cpp::CodeCompletionContext::FunctionCallAccess);
  CompletionItemTester functionContext3(top->childContexts()[ctxt], "a(a(100, ");
  QCOMPARE(functionContext3.names, QStringList() << "a" << "a" << "X" << "a" << "b" << "Templ" << "this");
  QCOMPARE(functionContext3.completionContext->parentContext()->accessType(), Cpp::CodeCompletionContext::FunctionCallAccess);
  CompletionItemTester functionContext4(top->childContexts()[ctxt], "a(a(100, Templ<int,int>), ");
  QCOMPARE(functionContext4.names, QStringList() << "a" << "X" << "a" << "b" << "Templ" << "this");
  QCOMPARE(functionContext4.completionContext->parentContext()->accessType(), Cpp::CodeCompletionContext::FunctionCallAccess);
  //See also testKeywords
  CompletionItemTester returnContext(top->childContexts()[ctxt], "return ");
  QCOMPARE(returnContext.names, QStringList() << "X" << "a" << "b" << "Templ" << "this");
  QCOMPARE(returnContext.completionContext->parentContext()->accessType(), Cpp::CodeCompletionContext::ReturnAccess);
  //See also testCaseContext
  release(top);
}

/*
 * A comment on expect-fails in following 3 tests:
 * "case" statement completion uses DUChainBase::createRangeMoving()->text() to get the switch'd expression
 * and parse its type. Unfortunately, PersistentRangeMoving class itself is not yet completed and doesn't work without an editor.
 * Hence the expression type cannot be resolved, which leads to 2 problems:
 * 1) best matches are not selected, so all items have match quality of 0;
 * 2) specialItemsForArgumentType() which is responsible for fetching enumerators from different scopes doesn't work either.
 */

void TestCppCodeCompletion::testCaseContext()
{
  QByteArray method = "enum testEnum { foo, bar }; void test() { switch( testEnum ) { } }";
  TopDUContext* top = parse(method, DumpNone);
  int ctxt = 2;
  int sctxt = 1;
  DUChainWriteLocker lock(DUChain::lock());
  CompletionItemTester caseContext(top->childContexts()[ctxt]->childContexts()[sctxt], "case ");
  QCOMPARE(caseContext.names, QStringList() << "testEnum" << "foo" << "bar");
  QCOMPARE(caseContext.completionContext->parentContext()->accessType(), Cpp::CodeCompletionContext::CaseAccess);
  QCOMPARE(caseContext.itemData("testEnum",   KDevelop::CodeCompletionModel::MatchQuality).toInt(), 0);
  QEXPECT_FAIL("", "PersistentRangeMoving needs to be fixed", Continue);
  QCOMPARE(caseContext.itemData("foo",        KDevelop::CodeCompletionModel::MatchQuality).toInt(), 10);
  QEXPECT_FAIL("", "PersistentRangeMoving needs to be fixed", Continue);
  QCOMPARE(caseContext.itemData("bar",        KDevelop::CodeCompletionModel::MatchQuality).toInt(), 10);
  release(top);
}

void TestCppCodeCompletion::testCaseContextComplexExpression()
{
  QByteArray method = "enum testEnum { foo, bar }; struct testStruct { testEnum e; }; void test(testStruct s) { switch (s.e) { } }";
  TopDUContext* top = parse(method, DumpNone);
  int ctxt = 3;
  int sctxt = 1;
  DUChainWriteLocker lock(DUChain::lock());
  CompletionItemTester caseContext(top->childContexts()[ctxt]->childContexts()[sctxt], "case ");
  QCOMPARE(caseContext.names, QStringList() << "testEnum" << "testStruct" << "foo" << "bar");
  QCOMPARE(caseContext.completionContext->parentContext()->accessType(), Cpp::CodeCompletionContext::CaseAccess);
  QCOMPARE(caseContext.itemData("testEnum",   KDevelop::CodeCompletionModel::MatchQuality).toInt(), 0);
  QCOMPARE(caseContext.itemData("testStruct", KDevelop::CodeCompletionModel::MatchQuality).toInt(), 0);
  QEXPECT_FAIL("", "PersistentRangeMoving needs to be fixed", Continue);
  QCOMPARE(caseContext.itemData("foo",        KDevelop::CodeCompletionModel::MatchQuality).toInt(), 10);
  QEXPECT_FAIL("", "PersistentRangeMoving needs to be fixed", Continue);
  QCOMPARE(caseContext.itemData("bar",        KDevelop::CodeCompletionModel::MatchQuality).toInt(), 10);
  release(top);
}

void TestCppCodeCompletion::testCaseContextDifferentScope()
{
  QByteArray method = "struct testStruct { enum testEnum { foo, bar } e; }; void test(testStruct s) { switch (s.e) { } }";
  TopDUContext* top = parse(method, DumpNone);
  int ctxt = 2;
  int sctxt = 1;
  DUChainWriteLocker lock(DUChain::lock());
  CompletionItemTester caseContext(top->childContexts()[ctxt]->childContexts()[sctxt], "case ");
  QCOMPARE(caseContext.completionContext->parentContext()->accessType(), Cpp::CodeCompletionContext::CaseAccess);
  QEXPECT_FAIL("", "PersistentRangeMoving needs to be fixed", Continue);
  QCOMPARE(caseContext.names, QStringList() << "testStruct" << "testStruct::foo" << "testStruct::bar");
  release(top);
}

void TestCppCodeCompletion::testCaseContextConstants()
{
  QByteArray method = "enum testEnum { foo, bar };"
                      "testEnum enum_nc; const testEnum enum_c; const testEnum enum_cc = foo;"
                      "int int_nc; const int int_c; const int int_cc = 0;"
                      "float float_nc; const float float_c; const float float_cc = 0.0;"
                      "testEnum func_enum(); constexpr testEnum func_enum_cc();"
                      "int func_int(); constexpr int func_int_cc();"
                      "void func_void(); float func_float(); constexpr float func_float_cc();"
                      "void testcase() { switch (enum_nc) { } }";
  TopDUContext* top = parse(method, DumpNone);
  int sctxt = 1;
  DUChainWriteLocker lock(DUChain::lock());
  CompletionItemTester caseContext(top->childContexts().last()->childContexts()[sctxt], "case ");
  QStringList names = caseContext.names;
  QVERIFY(names.contains("testEnum"));

  QVERIFY(!names.contains("enum_nc"));
  QVERIFY(!names.contains("enum_c"));
  QVERIFY( names.contains("enum_cc"));

  QVERIFY(!names.contains("int_nc"));
  QVERIFY(!names.contains("int_c"));
  QVERIFY( names.contains("int_cc"));

  QVERIFY(!names.contains("float_nc"));
  QVERIFY(!names.contains("float_c"));
  QVERIFY(!names.contains("float_cc"));

  QVERIFY(!names.contains("func_void"));
  QVERIFY(!names.contains("func_float"));
  QVERIFY(!names.contains("func_float_cc"));

  QEXPECT_FAIL("", "constexpr needs to be handled", Continue);
  QVERIFY(!names.contains("func_enum"));
  QVERIFY(names.contains("func_enum_cc"));

  QEXPECT_FAIL("", "constexpr needs to be handled", Continue);
  QVERIFY(!names.contains("func_int"));
  QVERIFY(names.contains("func_int_cc"));

  release (top);
}

void TestCppCodeCompletion::testUnaryOperators()
{
  QByteArray method = "class X { X(); int a; int b;}; int x,*z; X::X() { }";
  TopDUContext* top = parse(method, DumpAll);
  DUChainWriteLocker lock(DUChain::lock());
  int ctxt = 2;
  CompletionItemTester plusplus(top->childContexts()[ctxt], "x *= ++");
  QCOMPARE(plusplus.names, QStringList() << "int x *=" << "X" << "a" << "b" << "x" << "z" << "this");
  CompletionItemTester plusplus2(top->childContexts()[ctxt], "x *= *z + ++");
  QCOMPARE(plusplus2.names, QStringList() << "int* z +" << "X" << "a" << "b" << "x" << "z" << "this");
  CompletionItemTester plusplus3(top->childContexts()[ctxt], "a + b = - ++");
  QCOMPARE(plusplus3.names, QStringList() << "int b =" << "X" << "a" << "b" << "x" << "z" << "this");
  CompletionItemTester plusplus4(top->childContexts()[ctxt], "x *= &*++");
  QCOMPARE(plusplus4.names, QStringList() << "int x *=" << "X" << "a" << "b" << "x" << "z" << "this");
  CompletionItemTester minusminus(top->childContexts()[ctxt], "x ~= --");
  QCOMPARE(minusminus.names, QStringList() << "int x ~=" << "X" << "a" << "b" << "x" << "z" << "this");
  CompletionItemTester minusminus2(top->childContexts()[ctxt], "x + *z + --");
  QCOMPARE(minusminus2.names, QStringList() << "int* z +" << "X" << "a" << "b" << "x" << "z" << "this");
  //TODO: should get "int b -" hint, but binary operators don't work behind paren
  CompletionItemTester minusminus3(top->childContexts()[ctxt], "a + b -(----");
  QCOMPARE(minusminus3.names, QStringList() << "X" << "a" << "b" << "x" << "z" << "this");
  CompletionItemTester minusminus4(top->childContexts()[ctxt], "x ^= ++--");
  QCOMPARE(minusminus4.names, QStringList() << "int x ^=" << "X" << "a" << "b" << "x" << "z" << "this");
  CompletionItemTester notop(top->childContexts()[ctxt], "!++");
  QCOMPARE(notop.names, QStringList() << "X" << "a" << "b" << "x" << "z" << "this");
  CompletionItemTester notop2(top->childContexts()[ctxt], "!a & !");
  QCOMPARE(notop2.names, QStringList() << "int a &" << "X" << "a" << "b" << "x" << "z" << "this");
  CompletionItemTester notop3(top->childContexts()[ctxt], "b != !");
  QCOMPARE(notop3.names, QStringList() << "int b !=" << "X" << "a" << "b" << "x" << "z" << "this");
  CompletionItemTester notop4(top->childContexts()[ctxt], "x *= ~!");
  QCOMPARE(notop4.names, QStringList() << "int x *=" << "X" << "a" << "b" << "x" << "z" << "this");
  CompletionItemTester bitnot(top->childContexts()[ctxt], "~--");
  QCOMPARE(bitnot.names, QStringList() << "X" << "a" << "b" << "x" << "z" << "this");
  CompletionItemTester bitnot2(top->childContexts()[ctxt], "++~");
  QCOMPARE(bitnot2.names, QStringList() << "X" << "a" << "b" << "x" << "z" << "this");
  CompletionItemTester bitnot3(top->childContexts()[ctxt], "b ~= ~");
  QCOMPARE(bitnot3.names, QStringList() << "int b ~=" << "X" << "a" << "b" << "x" << "z" << "this");
  CompletionItemTester bitnot4(top->childContexts()[ctxt], "a * ~~~");
  QCOMPARE(bitnot4.names, QStringList() << "int a *" << "X" << "a" << "b" << "x" << "z" << "this");
  CompletionItemTester plus(top->childContexts()[ctxt], "*z = +");
  QCOMPARE(plus.names, QStringList() << "int* z =" << "X" << "a" << "b" << "x" << "z" << "this");
  //TODO: same as minusminus3
  CompletionItemTester plus2(top->childContexts()[ctxt], "a + b +(+");
  QCOMPARE(plus2.names, QStringList() << "X" << "a" << "b" << "x" << "z" << "this");
  CompletionItemTester plus3(top->childContexts()[ctxt], "b ~= +b + -+");
  QCOMPARE(bitnot3.names, QStringList() << "int b ~=" << "X" << "a" << "b" << "x" << "z" << "this");
  CompletionItemTester plus4(top->childContexts()[ctxt], "a * +");
  QCOMPARE(plus4.names, QStringList() << "int a *" << "X" << "a" << "b" << "x" << "z" << "this");
  //TODO: same as minusminus3
  CompletionItemTester minus(top->childContexts()[ctxt], "*z -(-");
  QCOMPARE(minus.names, QStringList() << "X" << "a" << "b" << "x" << "z" << "this");
  CompletionItemTester minus2(top->childContexts()[ctxt], "a ^ -(-");
  QCOMPARE(minus2.names, QStringList() << "int a ^" << "X" << "a" << "b" << "x" << "z" << "this");
  CompletionItemTester minus3(top->childContexts()[ctxt], "b ~= -");
  QCOMPARE(minus3.names, QStringList() << "int b ~=" << "X" << "a" << "b" << "x" << "z" << "this");
  CompletionItemTester minus4(top->childContexts()[ctxt], "a * +-");
  QCOMPARE(minus4.names, QStringList() << "int a *" << "X" << "a" << "b" << "x" << "z" << "this");
  CompletionItemTester refderef1(top->childContexts()[ctxt], "**&(&");
  QCOMPARE(refderef1.names, QStringList() << "X" << "a" << "b" << "x" << "z" << "this");
  //TODO: same as minusminus3
  CompletionItemTester refderef2(top->childContexts()[ctxt], "a & b & (&");
  QCOMPARE(refderef2.names, QStringList() << "X" << "a" << "b" << "x" << "z" << "this");
  CompletionItemTester refderef3(top->childContexts()[ctxt], "a * x * *z * *");
  QCOMPARE(refderef3.names, QStringList() << "int* z *" << "X" << "a" << "b" << "x" << "z" << "this");
  CompletionItemTester refderef4(top->childContexts()[ctxt], "b & b & *");
  QCOMPARE(refderef4.names, QStringList() << "int b &" << "X" << "a" << "b" << "x" << "z" << "this");
  release(top);
}

void TestCppCodeCompletion::testBinaryOperators()
{
  QByteArray method = "class X { X(); int a; int b;}; X::X() { }";
  TopDUContext* top = parse(method, DumpAll);
  DUChainWriteLocker lock(DUChain::lock());
  CompletionItemTester gt(top->childContexts()[2], "if (a > ");
  QCOMPARE(gt.names, QStringList() << "int a >" << "X" << "a" << "b" << "this");
  CompletionItemTester lt(top->childContexts()[2], "if (a < ");
  QCOMPARE(lt.names, QStringList() << "int a <" << "X" << "a" << "b" << "this");
  CompletionItemTester plus(top->childContexts()[2], "if (a + ");
  QCOMPARE(plus.names, QStringList() << "int a +" << "X" << "a" << "b" << "this");
  CompletionItemTester minus(top->childContexts()[2], "if (a - ");
  QCOMPARE(minus.names, QStringList() << "int a -" << "X" << "a" << "b" << "this");
  CompletionItemTester multiply(top->childContexts()[2], "if (a * ");
  QCOMPARE(multiply.names, QStringList() << "int a *" << "X" << "a" << "b" << "this");
  CompletionItemTester divide(top->childContexts()[2], "if (a / ");
  QCOMPARE(divide.names, QStringList() << "int a /" << "X" << "a" << "b" << "this");
  CompletionItemTester modulus(top->childContexts()[2], "if (a % ");
  QCOMPARE(modulus.names, QStringList() << "int a %" << "X" << "a" << "b" << "this");
  CompletionItemTester xorop(top->childContexts()[2], "if (a ^ ");
  QCOMPARE(xorop.names, QStringList() << "int a ^" << "X" << "a" << "b" << "this");
  CompletionItemTester bitandop(top->childContexts()[2], "if (a & ");
  QCOMPARE(bitandop.names, QStringList() << "int a &" << "X" << "a" << "b" << "this");
  CompletionItemTester bitorop(top->childContexts()[2], "if (a | ");
  QCOMPARE(bitorop.names, QStringList() << "int a |" << "X" << "a" << "b" << "this");
  CompletionItemTester notequal(top->childContexts()[2], "if (a != ");
  QCOMPARE(notequal.names, QStringList() << "int a !=" << "X" << "a" << "b" << "this");
  CompletionItemTester ltequal(top->childContexts()[2], "if (a <= ");
  QCOMPARE(ltequal.names, QStringList() << "int a <=" << "X" << "a" << "b" << "this");
  CompletionItemTester gtequal(top->childContexts()[2], "if (a >= ");
  QCOMPARE(gtequal.names, QStringList() << "int a >=" << "X" << "a" << "b" << "this");
  CompletionItemTester plusequal(top->childContexts()[2], "if (a += ");
  QCOMPARE(plusequal.names, QStringList() << "int a +=" << "X" << "a" << "b" << "this");
  CompletionItemTester minusequal(top->childContexts()[2], "if (a -= ");
  QCOMPARE(minusequal.names, QStringList() << "int a -=" << "X" << "a" << "b" << "this");
  CompletionItemTester multiplyequal(top->childContexts()[2], "if (a *= ");
  QCOMPARE(multiplyequal.names, QStringList() << "int a *=" << "X" << "a" << "b" << "this");
  CompletionItemTester divideequal(top->childContexts()[2], "if (a /= ");
  QCOMPARE(divideequal.names, QStringList() << "int a /=" << "X" << "a" << "b" << "this");
  CompletionItemTester modulusequal(top->childContexts()[2], "if (a %= ");
  QCOMPARE(modulusequal.names, QStringList() << "int a %=" << "X" << "a" << "b" << "this");
  CompletionItemTester xorequal(top->childContexts()[2], "if (a ^= ");
  QCOMPARE(xorequal.names, QStringList() << "int a ^=" << "X" << "a" << "b" << "this");
  CompletionItemTester bitandequal(top->childContexts()[2], "if (a &= ");
  QCOMPARE(bitandequal.names, QStringList() << "int a &=" << "X" << "a" << "b" << "this");
  CompletionItemTester bitorequal(top->childContexts()[2], "if (a |= ");
  QCOMPARE(bitorequal.names, QStringList() << "int a |=" << "X" << "a" << "b" << "this");
  CompletionItemTester bitnotequal(top->childContexts()[2], "if (a ~= ");
  QCOMPARE(bitnotequal.names, QStringList() << "int a ~=" << "X" << "a" << "b" << "this");
  CompletionItemTester leftshift(top->childContexts()[2], "if (a << ");
  QCOMPARE(leftshift.names, QStringList() << "int a <<" << "X" << "a" << "b" << "this");
  CompletionItemTester rightshift(top->childContexts()[2], "if (a >> ");
  QCOMPARE(rightshift.names, QStringList() << "int a >>" << "X" << "a" << "b" << "this");
  CompletionItemTester leftshifteq(top->childContexts()[2], "if (a <<= ");
  QCOMPARE(leftshifteq.names, QStringList() << "int a <<=" << "X" << "a" << "b" << "this");
  CompletionItemTester rightshifteq(top->childContexts()[2], "if (a >>= ");
  QCOMPARE(rightshifteq.names, QStringList() << "int a >>=" << "X" << "a" << "b" << "this");
  CompletionItemTester equal(top->childContexts()[2], "if (a == ");
  QCOMPARE(equal.names, QStringList() << "int a ==" << "X" << "a" << "b" << "this");
  CompletionItemTester andop(top->childContexts()[2], "if (a && ");
  QCOMPARE(andop.names, QStringList() << "int a &&" << "X" << "a" << "b" << "this");
  CompletionItemTester orop(top->childContexts()[2], "if (a || ");
  QCOMPARE(orop.names, QStringList() << "int a ||" << "X" << "a" << "b" << "this");
  CompletionItemTester bracket(top->childContexts()[2], "if (a[ ");
  QCOMPARE(bracket.names, QStringList() << "int a []" << "X" << "a" << "b" << "this");
  CompletionItemTester assign(top->childContexts()[2], "if (a = ");
  QCOMPARE(assign.names, QStringList() << "int a =" << "X" << "a" << "b" << "this");
  CompletionItemTester assign2(top->childContexts()[2], "X z = ");
  QCOMPARE(assign2.names, QStringList() << "class X =" << "X" << "a" << "b" << "this");
  CompletionItemTester assign3(top->childContexts()[2], "X *z = new");
  QCOMPARE(assign3.names, QStringList() << "class X =" << "X");
  release(top);
}

void TestCppCodeCompletion::testDeclarationIsInitialization()
{
  QByteArray method = "template<class T1, class T2> class Templ { T1 ta(); T2 tb(); }; class X { X(); int a; int b;}; X::X() { }";
  TopDUContext* top = parse(method, DumpAll);
  DUChainWriteLocker lock(DUChain::lock());
  CompletionItemTester notActuallyADecl(top->childContexts()[4], "int g; if (a > this->");
  QCOMPARE(notActuallyADecl.names, QStringList() << "int a >" << "a" << "b");
  CompletionItemTester notActuallyADecl2(top->childContexts()[4], "int g; if (a < a > this->");
  QCOMPARE(notActuallyADecl2.names, QStringList() << "int a >" << "a" << "b");
  CompletionItemTester notTemplateEqOp(top->childContexts()[4], "int g = 1; int h =");
  QCOMPARE(notTemplateEqOp.names, QStringList() << "int =" << "X" << "a" << "b" << "Templ" << "this");
  CompletionItemTester isTemplateEqOp(top->childContexts()[4], "int g = 1; Templ<int,int> h =");
  QCOMPARE(isTemplateEqOp.names, QStringList() << "class Templ< int, int > =" << "X" << "a" << "b" << "Templ" << "this");
  CompletionItemTester isTemplate(top->childContexts()[4], "int g = 1; Templ<int,int> h(");
  QCOMPARE(isTemplate.names, QStringList() << "Templ< int, int >(" << "X" << "a" << "b" << "Templ" << "this");
  CompletionItemTester isTemplate2(top->childContexts()[4], "Templ<int,int> h(");
  QCOMPARE(isTemplate2.names, isTemplate.names);
  CompletionItemTester isTemplate3(top->childContexts()[4], "int g; Templ<int,int> h(");
  QCOMPARE(isTemplate2.names, isTemplate.names);
  CompletionItemTester constructInit(top->childContexts()[4], "int g = 1; X myx(");
  QCOMPARE(constructInit.names, QStringList() << "X" << "X(" << "X" << "a" << "b" << "Templ" << "this");
  CompletionItemTester ptr(top->childContexts()[4], "int g = 1; int* test =");
  QCOMPARE(ptr.names, QStringList() << "int* =" << "X" << "a" << "b" << "Templ" << "this");
  CompletionItemTester ptr2(top->childContexts()[4], "int g = 1; int** test =");
  QCOMPARE(ptr2.names, QStringList() << "int** =" << "X" << "a" << "b" << "Templ" << "this");
  release(top);
}


void TestCppCodeCompletion::testNoMemberAccess() {
  QByteArray test = "class MyClass{ public:\n int myint; };\n\n";
  
  TopDUContext* context = parse(test, DumpNone);
  DUChainWriteLocker lock(DUChain::lock());
  QCOMPARE(context->childContexts().count(), 1);
  
  CompletionItemTester testCase(context, "void "); //NoMemberAccess with non-empty valid-type expression
  QVERIFY(testCase.completionContext->isValid());
  QCOMPARE(testCase.names, QStringList()); //Valid, but should not offer any completions in this case
  
  CompletionItemTester testCase1(context, "asdf "); //NoMemberAccess with non-empty invalid-type expression
  QVERIFY(!testCase1.completionContext->isValid());
  
  CompletionItemTester testCase2(context, " "); //NoMemberAccess with empty expression
  QVERIFY(testCase2.completionContext->isValid());
  QCOMPARE(testCase.names, QStringList()); //Theoretically should have "MyClass", but global completions aren't included

  CompletionItemTester testCase3(context, "public: ");
  QVERIFY(testCase2.completionContext->isValid());

  CompletionItemTester testCase4(context, "protected: ");
  QVERIFY(testCase2.completionContext->isValid());

  CompletionItemTester testCase5(context, "private: ");
  QVERIFY(testCase2.completionContext->isValid());

  release(context);
}

void TestCppCodeCompletion::testFunctionImplementation() {
  //__hidden1 and _Hidden2 should not be visible in the code-completion, as their identifiers are reserved to C++ implementations and standard libraries.
  addInclude("myclass.h", "namespace mynamespace { class myclass { void students(); }; }; class __hidden1; int _Hidden2; ");
  QByteArray test = "#include \"myclass.h\"\nnamespace mynamespace { }";
  
  TopDUContext* context = parse(test, DumpNone);
  DUChainWriteLocker lock(DUChain::lock());
  QCOMPARE(context->childContexts().count(), 1);
  
  CompletionItemTester testCase(context->childContexts()[0]);
  QVERIFY(testCase.completionContext->isValid());
  QCOMPARE(testCase.names, QStringList() << "mynamespace" << "myclass");
  
  //TODO: If it ever becomes possible to test implementationhelpers, here it should be done
  release(context);
}

void TestCppCodeCompletion::testAliasDeclarationAccessPolicy() {
  QByteArray test = "namespace Base { int One; int Two; int Three };\
  class List { public: using Base::One; protected: using Base::Two; private: using Base::Three; }; int main(List a) {}";
  
  TopDUContext* context = parse(test, DumpNone);
  DUChainWriteLocker lock(DUChain::lock());
  QCOMPARE(context->childContexts().count(), 4);
  
  CompletionItemTester testCase(context->childContexts()[3], "a.");
  QVERIFY(testCase.completionContext->isValid());
  QCOMPARE(testCase.names, QStringList() << "One");
  
  AliasDeclaration* aliasDeclOne = dynamic_cast<AliasDeclaration*>(context->childContexts()[1]->localDeclarations()[0]);
  AliasDeclaration* aliasDeclTwo = dynamic_cast<AliasDeclaration*>(context->childContexts()[1]->localDeclarations()[1]);
  AliasDeclaration* aliasDeclThree = dynamic_cast<AliasDeclaration*>(context->childContexts()[1]->localDeclarations()[2]);
  QVERIFY(aliasDeclOne && aliasDeclTwo && aliasDeclThree);
  QVERIFY(aliasDeclOne->accessPolicy() == KDevelop::Declaration::Public);
  QVERIFY(aliasDeclTwo->accessPolicy() == KDevelop::Declaration::Protected);
  QVERIFY(aliasDeclThree->accessPolicy() == KDevelop::Declaration::Private);
  
  release(context);
}

void TestCppCodeCompletion::testKeywords() {
  QByteArray test = "struct Values { int Value1; int Value2; struct Sub { int SubValue; }; }; Values v; int test(int a) {}";

  TopDUContext* context = parse(test, DumpNone);
  DUChainWriteLocker lock(DUChain::lock());
  QCOMPARE(context->childContexts().count(), 3);

  int ctxt = 2;
  CompletionItemTester constcast(context->childContexts()[ctxt], "Values* y = const_cast<");
  QVERIFY(constcast.completionContext->isValid());
  QCOMPARE(constcast.names, QStringList() << "struct Values =" << "Values");
  CompletionItemTester staticcast(context->childContexts()[ctxt], "Values* y = static_cast<");
  QVERIFY(staticcast.completionContext->isValid());
  QCOMPARE(staticcast.names, QStringList() << "struct Values =" << "Values");
  CompletionItemTester dynamiccast(context->childContexts()[ctxt], "Values* y = dynamic_cast<");
  QVERIFY(dynamiccast.completionContext->isValid());
  QCOMPARE(dynamiccast.names, QStringList() << "struct Values =" << "Values");
  CompletionItemTester reinterpretcast(context->childContexts()[ctxt], "Values* y = reinterpret_cast<");
  QVERIFY(reinterpretcast.completionContext->isValid());
  QCOMPARE(reinterpretcast.names, QStringList() << "struct Values =" << "Values");
  CompletionItemTester testConst(context->childContexts()[ctxt], "const ");
  QVERIFY(testConst.completionContext->isValid());
  QCOMPARE(testConst.names, QStringList() << "Values");
  CompletionItemTester testTypedef(context->childContexts()[ctxt], "typedef ");
  QVERIFY(testTypedef.completionContext->isValid());
  QCOMPARE(testTypedef.names, QStringList() << "Values");
  CompletionItemTester testPublic(context->childContexts()[ctxt], "public ");
  QVERIFY(testPublic.completionContext->isValid());
  QCOMPARE(testPublic.names, QStringList() << "Values");
  CompletionItemTester testPublic2(context->childContexts()[0], "public: ");
  QVERIFY(testPublic2.completionContext->isValid());
  QCOMPARE(testPublic2.names, QStringList() << "Sub" << "Values");
  CompletionItemTester testFakePublic(context->childContexts()[ctxt], "fakepublic ");
  QVERIFY(!testFakePublic.completionContext->isValid());
  CompletionItemTester testProtected(context->childContexts()[ctxt], "protected ");
  QVERIFY(testProtected.completionContext->isValid());
  QCOMPARE(testProtected.names, QStringList() << "Values");
  CompletionItemTester testProtected2(context->childContexts()[0], "protected: ");
  QVERIFY(testProtected2.completionContext->isValid());
  QCOMPARE(testProtected2.names, QStringList() << "Sub" << "Values");
  CompletionItemTester testFakeProtected(context->childContexts()[ctxt], "fakeprotected ");
  QVERIFY(!testFakeProtected.completionContext->isValid());
  CompletionItemTester testPrivate(context->childContexts()[ctxt], "private ");
  QVERIFY(testPrivate.completionContext->isValid());
  QCOMPARE(testPrivate.names, QStringList() << "Values");
  CompletionItemTester testPrivate2(context->childContexts()[0], "private: ");
  QVERIFY(testPrivate2.completionContext->isValid());
  QCOMPARE(testPrivate2.names, QStringList() << "Sub" << "Values");
  CompletionItemTester testVirtual(context->childContexts()[ctxt], "virtual ");
  QVERIFY(testVirtual.completionContext->isValid());
  QCOMPARE(testVirtual.names, QStringList() << "Values");
  //TODO: fix ShowSignals, make this show only signals, and move emit tests to where it matters
  CompletionItemTester testEmit(context->childContexts()[ctxt], "emit ");
  QVERIFY(testEmit.completionContext->isValid());
  QCOMPARE(testEmit.names, QStringList() << "a" << "Values" << "v" << "test");
  CompletionItemTester testQEmit(context->childContexts()[ctxt], "Q_EMIT ");
  QVERIFY(testQEmit.completionContext->isValid());
  QCOMPARE(testQEmit.names, QStringList() << "a" << "Values" << "v" << "test");
  CompletionItemTester testCase(context->childContexts()[ctxt], "switch (a) { case ");
  QVERIFY(testCase.completionContext->isValid());
  QCOMPARE(testCase.names, QStringList() << "Values" << "test");
  CompletionItemTester testCase2(context->childContexts()[ctxt], "switch (a) { case v.");
  QVERIFY(testCase2.completionContext->isValid());
  QCOMPARE(testCase2.names, QStringList());
  CompletionItemTester testReturn(context->childContexts()[ctxt], "return ");
  QVERIFY(testReturn.completionContext->isValid());
  QCOMPARE(testReturn.names, QStringList()  << "return int" << "a" << "Values" << "v" << "test" << "v.Value1" << "v.Value2");
  CompletionItemTester testReturn2(context->childContexts()[ctxt], "return v.");
  QVERIFY(testReturn2.completionContext->isValid());
  QCOMPARE(testReturn2.names, QStringList() << "return int" << "Value1" << "Value2");
  CompletionItemTester testNew(context->childContexts()[ctxt], "Values* b = new ");
  QVERIFY(testNew.completionContext->isValid());
  QCOMPARE(testNew.names, QStringList() << "struct Values =" << "Values");
  //TODO: make "new" onlyShow types in this case, and then skip it to create return context
  CompletionItemTester testNew2(context->childContexts()[ctxt], "Values::Sub* b = new Values::");
  QVERIFY(testNew2.completionContext->isValid());
  QCOMPARE(testNew2.names, QStringList() << "Value1" << "Value2" << "Sub" );
  CompletionItemTester testFakeNew(context->childContexts()[ctxt], "mynew ");
  QVERIFY(!testFakeNew.completionContext->isValid());
  CompletionItemTester testElse(context->childContexts()[ctxt], "if (a) {} else ");
  QVERIFY(testElse.completionContext->isValid());
  QCOMPARE(testElse.names, QStringList() << "a" << "Values" << "v" << "test");
  CompletionItemTester testElse2(context->childContexts()[ctxt], "if (a) {} else Values::");
  QVERIFY(testElse2.completionContext->isValid());
  QCOMPARE(testElse2.names, QStringList() << "Value1" << "Value2" << "Sub" );
  CompletionItemTester testThrow(context->childContexts()[ctxt], "throw ");
  QVERIFY(testThrow.completionContext->isValid());
  QCOMPARE(testThrow.names, QStringList() << "a" << "Values" << "v" << "test");
  CompletionItemTester testThrow2(context->childContexts()[ctxt], "thow Values::");
  QVERIFY(testThrow2.completionContext->isValid());
  QCOMPARE(testThrow2.names, QStringList() << "Value1" << "Value2" << "Sub" );
  CompletionItemTester testThrowNew(context->childContexts()[ctxt], "throw new");
  QVERIFY(testThrowNew.completionContext->isValid());
  QCOMPARE(testThrowNew.names, QStringList() << "Values");
  //TODO: suboptimal results, see also testNew2
  CompletionItemTester testThrowNew2(context->childContexts()[ctxt], "throw new Values::");
  QVERIFY(testThrowNew2.completionContext->isValid());
  QCOMPARE(testThrowNew2.names, QStringList() << "Value1" << "Value2" << "Sub");
  CompletionItemTester testDelete(context->childContexts()[ctxt], "delete ");
  QVERIFY(testDelete.completionContext->isValid());
  QCOMPARE(testDelete.names, QStringList() << "a" << "Values" << "v" << "test");
  CompletionItemTester testDelete2(context->childContexts()[ctxt], "delete Values::");
  QVERIFY(testDelete2.completionContext->isValid());
  QCOMPARE(testDelete2.names, QStringList() << "Value1" << "Value2" << "Sub" );
  CompletionItemTester testDelete3(context->childContexts()[ctxt], "delete [ ] ");
  QVERIFY(testDelete3.completionContext->isValid());
  CompletionItemTester testFakeDelete(context->childContexts()[ctxt], "fakedelete ");
  QVERIFY(!testFakeDelete.completionContext->isValid());
  CompletionItemTester testFakeDelete2(context->childContexts()[ctxt], "fakedelete [] ");
  QVERIFY(!testFakeDelete2.completionContext->isValid());
  QCOMPARE(testDelete3.names, QStringList() << "a" << "Values" << "v" << "test");
  release(context);
}

void TestCppCodeCompletion::testArgumentMatching() {
  {
    QByteArray test = "struct A{ int m;}; void test(int q) { A a;  }";

    TopDUContext* context = parse( test, DumpNone /*DumpDUChain | DumpAST */);
    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(context->childContexts().count(), 3);
    CompletionItemTester tester(context->childContexts()[2], "test(a.");
    QVERIFY(tester.completionContext->parentContext());
    QCOMPARE(tester.completionContext->parentContext()->knownArgumentTypes().count(), 0);
    QCOMPARE(tester.completionContext->parentContext()->functionName(), QString("test"));
    bool abort = false;
    QCOMPARE(tester.completionContext->parentContext()->completionItems(abort).size(), 1);
    release(context);
  }
  {
    QByteArray test = "#define A(x) #x\n void test(char* a, char* b, int c) { } ";

    TopDUContext* context = parse( test, DumpNone /*DumpDUChain | DumpAST */);
    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(context->childContexts().count(), 2);
    CompletionItemTester tester(context->childContexts()[1], "test(\"hello\", A(a),");
    QVERIFY(tester.completionContext->parentContext());
    QCOMPARE(tester.completionContext->parentContext()->knownArgumentTypes().count(), 2);
    QVERIFY(tester.completionContext->parentContext()->knownArgumentTypes()[0].type.abstractType());
    QVERIFY(tester.completionContext->parentContext()->knownArgumentTypes()[1].type.abstractType());
    QCOMPARE(tester.completionContext->parentContext()->knownArgumentTypes()[0].type.abstractType()->toString(), QString("const char*"));
    QCOMPARE(tester.completionContext->parentContext()->knownArgumentTypes()[1].type.abstractType()->toString(), QString("const char*"));
    release(context);
  }
}

void TestCppCodeCompletion::testSubClassVisibility() {
  {
    QByteArray test = "typedef struct { int am; } A; void test() { A b; } ";

    TopDUContext* context = parse( test, DumpAll /*DumpDUChain | DumpAST */);
    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(context->childContexts().count(), 3);
    QCOMPARE(context->childContexts()[0]->localDeclarations().count(), 1);
    QCOMPARE(context->childContexts()[0]->localDeclarations()[0]->kind(), Declaration::Instance);
    QCOMPARE(CompletionItemTester(context->childContexts()[2], "b.").names, QStringList() << "am");

    release(context);
  }
  {
    QByteArray test = "struct A { int am; struct B { int bm; }; }; void test() { A::B b; } ";

    TopDUContext* context = parse( test, DumpNone /*DumpDUChain | DumpAST */);
    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(context->childContexts().count(), 3);
    QCOMPARE(CompletionItemTester(context->childContexts()[2], "b.").names, QStringList() << "bm");

    release(context);
  }
  {
    QByteArray test = "struct A { int am; struct B; }; struct A::B {int bm; }; void test() { A::B b; } ";

    TopDUContext* context = parse( test, DumpNone /*DumpDUChain | DumpAST */);
    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(context->childContexts().count(), 4);
    QCOMPARE(CompletionItemTester(context->childContexts()[3], "b.").names, QStringList() << "bm");

    release(context);
  }
}

void TestCppCodeCompletion::testMacrosInCodeCompletion()
{
  QByteArray test = "#define test foo\n #define testfunction(X) x\n #define test2 fee\n struct A {int mem;}; void fun() { A foo; A* fee;\n }";
  TopDUContext* context = parse( test, DumpNone /*DumpDUChain | DumpAST */);
  DUChainWriteLocker lock(DUChain::lock());
  
  QCOMPARE(context->childContexts().size(), 3);
  
  QCOMPARE(CompletionItemTester(context->childContexts()[2], "foo.", QString(), CursorInRevision(3, 0)).names, QStringList() << "mem");
  QCOMPARE(CompletionItemTester(context->childContexts()[2], "test.", QString(), CursorInRevision(3, 0)).names, QStringList() << "mem");
  
  QCOMPARE(CompletionItemTester(context->childContexts()[2], "fee->", QString(), CursorInRevision(3, 0)).names, QStringList() << "mem");
  QCOMPARE(CompletionItemTester(context->childContexts()[2], "test2->", QString(), CursorInRevision(3, 0)).names, QStringList() << "mem");
  QCOMPARE(CompletionItemTester(context->childContexts()[2], "testfunction2(", QString(), CursorInRevision(4, 0)).names.toSet(), QSet<QString>() << "testfunction2(" << "A" << "foo" << "fee");
  QCOMPARE(CompletionItemTester(context->childContexts()[2], "testfunction(", QString(), CursorInRevision(4, 0)).names.toSet(), QSet<QString>() << "testfunction(" << "A" << "foo" << "fee");
  
  release(context);
}

void TestCppCodeCompletion::testConstructorCompletion() {
  {
    QByteArray test = "class A {}; class Class : public A { Class(); int m_1; float m_2; char m_3; static int m_4; }; ";

    //74
    TopDUContext* context = parse( test, DumpNone /*DumpDUChain | DumpAST */);
    DUChainWriteLocker lock(DUChain::lock());
    
    {
      kDebug() << "TEST 2";
      CompletionItemTester tester(context, "class Class { Class(); int m_1; float m_2; char m_3; }; Class::Class(int m1, float m2, char m3) : m_1(1), m_2(m2), ");
      
      //At first, only the members should be shown
      kDebug() << tester.names;
      QCOMPARE(tester.names, QStringList() << "m_3"); //m_1 should not be shown, because it is already initialized
      ///@todo Make sure that the item also inserts parens
    }

    {
      CompletionItemTester tester(context, "Class::Class(int m1, float m2, char m3) : ");
      
      //At first, only the members should be shown
      kDebug() << tester.names;
      QCOMPARE(tester.names, QStringList() << "A" <<  "m_1" << "m_2" << "m_3");
      ///@todo Make sure that the item also inserts parens
    }

    {
      kDebug() << "TEST 3";
      CompletionItemTester tester(context, "Class::Class(int m1, float m2, char m3) : m_1(");
      
      //At first, only the members should be shown
      QVERIFY(tester.names.size());
      QVERIFY(tester.completionContext->parentContext()); //There must be a type-hinting context
      QVERIFY(tester.completionContext->parentContext()->parentContext()); //There must be a type-hinting context
      QVERIFY(!tester.completionContext->isConstructorInitialization());
      QVERIFY(!tester.completionContext->parentContext()->isConstructorInitialization());
      QVERIFY(tester.completionContext->parentContext()->accessType() == Cpp::CodeCompletionContext::FunctionCallAccess);
      QVERIFY(tester.completionContext->parentContext()->parentContext()->isConstructorInitialization());
    }
    
    release(context);
  }
}

void TestCppCodeCompletion::testConstructorUsageCompletion_data()
{
  QTest::addColumn<QString>("code");        // existing source code
  QTest::addColumn<QStringList>("args");        // completion arguments
  QTest::addColumn<QStringList>("not_args");    // forbidden completion arguments

  // only the default constructor is present
  QTest::newRow("only default")
    << "class A { A(); };"
    << (QStringList() << "()")
    << QStringList();
  // the default constructor and one overload
  QTest::newRow("Default and int")
    << "class A { A(); A(int); };"
    << (QStringList() << "()" << "(int)")
    << QStringList();
  // the default constructor and two overloads
  QTest::newRow("Default, int and float")
    << "class A { A(); A(int); A(float); };"
    << (QStringList() << "()" << "(int)" << "(float)")
    << QStringList();
  // deleted default constructor and two overloads
  QTest::newRow("no default, int and float")
    << "class A { A() = delete; A(int); A(float); };"
    << (QStringList() << "(int)" << "(float)")
    << (QStringList() << "()");
  // the default and copy constructors are present
  QTest::newRow("default and copy")
    << "class A { A(); A(const A&); };"
    << (QStringList() << "()" << "(const A&)")
    << QStringList();
  // the default and copy constructors are deleted
  QTest::newRow("default and copy")
    << "class A { A() = delete; A(const A&) = delete; };"
    << QStringList()
    << (QStringList() << "()" << "(const A&)");
  // the default and copy constructors are deleted, but another one is present
  QTest::newRow("default and copy")
    << "class A { A() = delete; A(const A&) = delete; A(int); };"
    << (QStringList() << "(int)")
    << (QStringList() << "()" << "(const A&)");
}

void TestCppCodeCompletion::testConstructorUsageCompletion()
{
  QFETCH(QString, code);
  QFETCH(QStringList, args);
  QFETCH(QStringList, not_args);

  TopDUContext* context = parse( code.toAscii(), DumpNone /*DumpDUChain | DumpAST */);
  DUChainWriteLocker lock(DUChain::lock());

  CompletionItemTester tester(context, "void k() { A *a = new A(");

  QStringList foundargs;
  for (int i = 0; i < tester.items.count(); i++)
    foundargs << tester.itemData(i, KTextEditor::CodeCompletionModel::Arguments).toString();
  qDebug() << foundargs << args << not_args;
  foreach(const QString &arg, args)
    QVERIFY(foundargs.contains(arg));
  foreach(const QString &arg, not_args)
    QVERIFY(!foundargs.contains(arg));

  release(context);
}

void TestCppCodeCompletion::testParentConstructor_data()
{
  QTest::addColumn<QString>("code");        // existing source code
  QTest::addColumn<QString>("completion");  // completion contents

  // Parent has no constructor
  QTest::newRow("NoParentConstructor") << "class A {}; class B : public A { B (int i); };" << "B::B(int i) { }";
  // Argument name mismatch - choose it anyway
  QTest::newRow("ArgNameMismatch") << "class A { A(int j) {} }; class B : public A { B(int i); };" << "B::B(int i): A(i) { }";
  // Between two arguments with matching type, prefer one with matching name
  QTest::newRow("PreferNameMatch") << "class A { A(int i, int j) {} }; class B : public A { B(int j, int i); };" << "B::B(int j, int i): A(i, j) { }";
  // Argument type mismatch - do not choose anything for this argument
  QTest::newRow("ArgTypeMismatch") << "class A { A(char i) {} }; class B : public A { B(int i); };" << "B::B(int i): A() { }";
  // Some arguments match, others do not - leave the latter blank
  QTest::newRow("SomeArgumentsMatch") << "class A { A(int i, char j) {} }; class B : public A { B(int i, int j); };" << "B::B(int i, int j): A(i, ) { }";
  // Parent class has multiple constructors - choose the best matching
  QTest::newRow("MultipleConstructors") << "class A { A(char c) {} A(int i, int j) {} }; class B : public A { B(int i); };" << "B::B(int i): A(i, i) { }";
  QTest::newRow("MultipleConstructorsNoFullMatch") << "class A { A(char c, long l) {} A(int i, long l) {} }; class B : public A { B(int i); };" << "B::B(int i): A(i, ) { }";
  // Omit call to default constructor if it is the best match
  QTest::newRow("BestMatchDefault") << "class A { A() {} A(char c) {} }; class B : public A { B(int i); };" << "B::B(int i) { }";

  // Multiple parents, each has its own matches
  QTest::newRow("MultipleParents") << "class A { A(int i, char c) {} }; class B { B(char d, long l) {} }; class C : public A, public B { C(int i, long l, char c); };"
    << "C::C(int i, long int l, char c): A(i, c), B(c, l) { }";
  // Last check
  QTest::newRow("ComplexCase") << "class A { A(int i, double d, char c) {} A(int i, double d, long l) {} };"
    "class B { B(double d, int j) {} B(char c) {} };"
    "class C { };" 
    "class D : public A, public B, public C { D(short a1, double a2, long a3, int a4); };" <<
    "D::D(short int a1, double a2, long int a3, int a4): A(a4, a2, a3), B(a2, a4) { }";
}

void TestCppCodeCompletion::testParentConstructor()
{
  QFETCH(QString, code);
  QFETCH(QString, completion);
  TopDUContext* context = parse(code.toAscii(), DumpNone);
  DUChainWriteLocker lock(DUChain::lock());
  CompletionItemTester tester(context, "void");  // Force a function context
  Cpp::ImplementationHelperItem* constructorItem = dynamic_cast<Cpp::ImplementationHelperItem*>(tester.items[0].data());
  QCOMPARE(constructorItem->insertionText().simplified(), completion.simplified());
  release(context);
}

void TestCppCodeCompletion::testOverride_data()
{
  QTest::addColumn<QString>("parentCode");  // code in parent class
  QTest::addColumn<QString>("prefix");      // completion prefix
  QTest::addColumn<QString>("completion");  // expected completion contents

  QTest::newRow("empty-prefix") << "virtual void foo();" << "" << "virtual void foo();";
  QTest::newRow("public") << "virtual void foo();" << "public: " << "virtual void foo();";
  QTest::newRow("protected") << "virtual void foo();" << "protected: " << "virtual void foo();";
  QTest::newRow("private") << "virtual void foo();" << "private: " << "virtual void foo();";
}

void TestCppCodeCompletion::testOverride()
{
  QFETCH(QString, parentCode);
  QFETCH(QString, prefix);
  QFETCH(QString, completion);
  QString tmpl = QString(" class A {public: %1  };\n"
                         " class B : public A { };").arg(parentCode);
  TopDUContext* context = parse(tmpl.toLocal8Bit(), DumpNone);
  DUChainWriteLocker lock;
  DUContext* BCtx = context->childContexts().last();
  QCOMPARE(BCtx->localScopeIdentifier().toString(), QLatin1String("B"));
  CompletionItemTester tester(BCtx, prefix);
  bool found = false;
  foreach(CompletionItemTester::Item item, tester.items) {
    Cpp::ImplementationHelperItem* override = dynamic_cast<Cpp::ImplementationHelperItem*>(item.data());
    if (!override) {
      continue;
    }
    QCOMPARE(override->m_type, Cpp::ImplementationHelperItem::Override);
    QCOMPARE(override->insertionText().simplified(), completion.simplified());
    found = true;
    break;
  }
  QVERIFY(found);
  release(context);
}

void TestCppCodeCompletion::testOverrideDeleted()
{
  const QByteArray tmpl("class A {public: virtual void foo() = delete; };\n"
                       "class B : public A { };");
  TopDUContext* context = parse(tmpl, DumpNone);
  DUChainWriteLocker lock;
  DUContext* BCtx = context->childContexts().last();
  QCOMPARE(BCtx->localScopeIdentifier().toString(), QLatin1String("B"));
  CompletionItemTester tester(BCtx, "");
  bool found = false;
  foreach(CompletionItemTester::Item item, tester.items) {
    Cpp::ImplementationHelperItem* override = dynamic_cast<Cpp::ImplementationHelperItem*>(item.data());
    if (!override) {
      continue;
    }
    found = true;
    break;
  }
  QVERIFY(!found);
  release(context);
}

void TestCppCodeCompletion::testSignalSlotCompletion() {
    // By processing qobjectdefs.h, we make sure that the qt-specific macros are defined in the duchain through overriding (see setuphelpers.cpp)
    addInclude("/qobjectdefs.h", "#define signals\n#define slots\n#define Q_SIGNALS\n#define Q_SLOTS\n#define Q_PRIVATE_SLOT\n#define SIGNAL\n#define SLOT\n int n;\n");
  
    addInclude("QObject.h", "#include \"/qobjectdefs.h\"\n class QObject { void connect(QObject* from, const char* signal, QObject* to, const char* slot); void connect(QObject* from, const char* signal, const char* slot); };");
    
    QByteArray test("#include \"QObject.h\"\n class TE; class A : public QObject { public slots: void slot1(); void slot2(TE*); signals: void signal1(TE*, char);void signal2(); public: void test() { } private: Q_PRIVATE_SLOT(d,void slot3(TE*))  };");

    TopDUContext* context = parse( test, DumpAll );
    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(context->childContexts().count(), 1);
    QCOMPARE(context->childContexts()[0]->childContexts().count(), 8);
    CompletionItemTester(context->childContexts()[0]->childContexts()[5], "connect( this, ");
    QCOMPARE(CompletionItemTester(context->childContexts()[0]->childContexts()[5], "connect( this, ").names.toSet(), (QStringList() << "connect" << "signal1" << "signal2").toSet());
    QCOMPARE(CompletionItemTester(context->childContexts()[0]->childContexts()[5], "connect( this, SIGNAL(").names.toSet(), (QStringList() << "connect" << "signal1" << "signal2").toSet());
    kDebug() << "ITEMS:" << CompletionItemTester(context->childContexts()[0]->childContexts()[5], "connect( this, SIGNAL(signal2()), this, SLOT(").names;
    QCOMPARE(CompletionItemTester(context->childContexts()[0]->childContexts()[5], "connect( this, SIGNAL(signal2()), this, ").names.toSet(), (QStringList() << "connect" << "signal1" << "signal2" << "slot1" << "slot2" << "slot3" << "Connect to A::signal2 ()").toSet());
    QCOMPARE(CompletionItemTester(context->childContexts()[0]->childContexts()[5], "connect( this, SIGNAL(signal2()), this, SIGNAL(").names.toSet(), (QStringList() << "connect" << "signal1" << "signal2" << "Connect to A::signal2 ()").toSet());
    QCOMPARE(CompletionItemTester(context->childContexts()[0]->childContexts()[5], "connect( this, SIGNAL(signal2()), this, SLOT(").names.toSet(), (QStringList() << "connect" << "slot1" << "slot2" << "slot3" << "Connect to A::signal2 ()" << "signal2").toSet());
    QVERIFY(((QStringList() << "connect" << "signal1" << "signal2" << "slot1" << "slot2" << "slot3" << "Connect to A::signal2 ()").toSet() - CompletionItemTester(context->childContexts()[0]->childContexts()[5], "connect( this, SIGNAL(signal2()), ").names.toSet()).isEmpty());
    QVERIFY(((QStringList() << "connect" << "signal1" << "signal2" << "Connect to A::signal2 ()").toSet() - CompletionItemTester(context->childContexts()[0]->childContexts()[5], "connect( this, SIGNAL(signal2()), SIGNAL(").names.toSet()).isEmpty());
    QVERIFY(((QStringList() << "connect" << "slot1" << "slot2" << "slot3"<< "Connect to A::signal2 ()").toSet() - CompletionItemTester(context->childContexts()[0]->childContexts()[5], "connect( this, SIGNAL(signal2()), SLOT(").names.toSet()).isEmpty());
    
    Declaration* decl = context->childContexts().last()->findDeclarations(Identifier("slot3")).first();
    QVERIFY(decl);
    QVERIFY(dynamic_cast<ClassFunctionDeclaration*>(decl));
    QVERIFY(dynamic_cast<ClassFunctionDeclaration*>(decl)->accessPolicy() == Declaration::Private);
    QVERIFY(dynamic_cast<ClassFunctionDeclaration*>(decl)->isSlot());
    release(context);
}

void TestCppCodeCompletion::testSignalSlotExecution()
{
    // By processing qobjectdefs.h, we make sure that the qt-specific macros are defined in the duchain through overriding (see setuphelpers.cpp)
    addInclude("/qobjectdefs.h", "#define signals\n#define slots\n#define Q_SIGNALS\n#define Q_SLOTS\n#define Q_PRIVATE_SLOT\n#define SIGNAL\n#define SLOT\n int n;\n");
    addInclude("QObject.h", "#include \"/qobjectdefs.h\"\n class QObject { void connect(QObject* from, const char* signal, QObject* to, const char* slot); void connect(QObject* from, const char* signal, const char* slot); };");

    QByteArray test("#include \"QObject.h\""
                    "\nclass A : public QObject { public slots: void slot1(); void slot2(void*);"
                                                        "signals: void signal1(void*, char); void signal2(void*); "
                    "\nvoid test() { connect( this, SIGNAL(signal2(void*)), SLOT() ); } };");

    TopDUContext* top = parse( test, DumpNone );
    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(top->childContexts().count(), 1);
    QCOMPARE(top->childContexts()[0]->childContexts().count(), 6);
    QVERIFY(top->problems().isEmpty());

    DUContext* context = top->childContexts()[0]->childContexts()[5];

    KTextEditor::Editor* editor = KTextEditor::Editor::instance();
    QVERIFY(editor);
    KTextEditor::Document* doc = editor->createDocument(this);
    QVERIFY(doc);

    doc->setText(test);
    KTextEditor::View* v = doc->createView(0);

    // Test 1: SIGNAL(<here>signal2(void*)) parens balancing
    {
        doc->startEditing();
        KTextEditor::Cursor c( 2, 36 );
        v->setCursorPosition( c );

        CompletionItemTester complCtx( context, "connect( this, SIGNAL(", "", CursorInRevision( c.line(), c.column() ) );
        KSharedPtr<CompletionTreeItem> item;
        for( int i = 0; i < complCtx.items.length(); ++i ) {
            if( complCtx.itemData( i ).toString() == "signal1" ) {
                item = complCtx.items.at( i );
            }
        }
        QVERIFY( !item.isNull() );

        item->execute( doc, Range( c, 0 ) );
        QCOMPARE( doc->line( 2 ), QString("void test() { connect( this, SIGNAL(signal1(void*,char)), SLOT() ); } };") );

        doc->endEditing();
    }

    // Test 2: SLOT(<here>) parens balancing
    {
        doc->startEditing();
        KTextEditor::Cursor c( 2, 58 );
        v->setCursorPosition( c );

        CompletionItemTester complCtx( context, "connect( this, SIGNAL(signal1(void*,char)), SLOT(", "", CursorInRevision( c.line(), c.column() ) );
        KSharedPtr<CompletionTreeItem> item;
        for( int i = 0; i < complCtx.items.length(); ++i ) {
            if( complCtx.itemData( i ).toString() == "slot2" ) {
                item = complCtx.items.at( i );
            }
        }
        QVERIFY( !item.isNull() );

        item->execute( doc, Range( c, 0 ) );
        QCOMPARE( doc->line( 2 ), QString("void test() { connect( this, SIGNAL(signal1(void*,char)), SLOT(slot2(void*)) ); } };") );

        doc->endEditing();
    }
    // Test 3: Slot implementation helper: SLOT(<here>) parens balancing
    {
        doc->startEditing();
        KTextEditor::Cursor c( 2, 58 );
        v->setCursorPosition( c );

        lock.unlock();
        CompletionItemTester complCtx( context, "connect( this, SIGNAL(signal1(void*,char)), SLOT(", "slot3", CursorInRevision( c.line(), c.column() ) );
        qDebug() << "TEST3 names: " << complCtx.names;
        KSharedPtr<CompletionTreeItem> item;
        for( int i = 0; i < complCtx.items.length(); ++i ) {
            if( complCtx.itemData( i ).toString() == "slot3" ) {
                item = complCtx.items.at( i );
            }
        }
        QVERIFY( !item.isNull() );

        item->execute( doc, Range( c, 0 ) );
        lock.lock();
        QEXPECT_FAIL("", "Slot is not replaced because the engine fails to create the declaration.", Continue);
        QCOMPARE( doc->line( 2 ), QString("void test() { connect( this, SIGNAL(signal1(void*,char)), SLOT(slot3(void*)) ); } };") );

        doc->endEditing();
    }

    release(top);

    delete v;
    delete doc;
}

void TestCppCodeCompletion::testAssistant() {
  {
    QByteArray test = "#define A hallo(3) = 1\n void test() { A; bla = 5; }";

    TopDUContext* context = parse( test, DumpAll );
    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(context->problems().count(), 1);
    release(context);
  }
  {
    QByteArray test = "int n; class C { C() : member(n) {} }; }";

    TopDUContext* context = parse( test, DumpAll );
    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(context->problems().count(), 1);
    {
      KSharedPtr<Cpp::MissingDeclarationProblem> mdp( dynamic_cast<Cpp::MissingDeclarationProblem*>(context->problems()[0].data()) );
      QVERIFY(mdp);
      kDebug() << "problem:" << mdp->description();
      QCOMPARE(mdp->type->containerContext.data(), context->childContexts()[0]);
      QCOMPARE(mdp->type->identifier().toString(), QString("member"));
      QVERIFY(mdp->type->assigned.type.isValid());
      QCOMPARE(TypeUtils::removeConstants(mdp->type->assigned.type.abstractType(), context)->toString(), QString("int"));
    }
    release(context);
  }
  {
    QByteArray test = "class C {}; void test() {C c; c.value = 5; int i = c.value2; i = c.value3(); }";

    TopDUContext* context = parse( test, DumpAll );
    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(context->problems().count(), 3);
    {
      KSharedPtr<Cpp::MissingDeclarationProblem> mdp( dynamic_cast<Cpp::MissingDeclarationProblem*>(context->problems()[0].data()) );
      QVERIFY(mdp);
      kDebug() << "problem:" << mdp->description();
      QCOMPARE(mdp->type->containerContext.data(), context->childContexts()[0]);
      QCOMPARE(mdp->type->identifier().toString(), QString("value"));
      QVERIFY(mdp->type->assigned.type.isValid());
      QCOMPARE(TypeUtils::removeConstants(mdp->type->assigned.type.abstractType(), context)->toString(), QString("int"));
      QCOMPARE(context->childContexts().count(), 3);
    }
    {
      ///@todo Make this work as well
/*      KSharedPtr<Cpp::MissingDeclarationProblem> mdp( dynamic_cast<Cpp::MissingDeclarationProblem*>(context->problems()[1].data()) );
      QVERIFY(mdp);
      kDebug() << "problem:" << mdp->description();
      QCOMPARE(mdp->type->containerContext.data(), context->childContexts()[0]);
      QCOMPARE(mdp->type->identifier().toString(), QString("value2"));
      QVERIFY(!mdp->type->assigned.type.isValid());
      QVERIFY(mdp->type->convertedTo.type.isValid());
      QCOMPARE(TypeUtils::removeConstants(mdp->type->convertedTo.type.abstractType())->toString(), QString("int"));
      QCOMPARE(context->childContexts().count(), 3);*/
    }
    {
      KSharedPtr<Cpp::MissingDeclarationProblem> mdp( dynamic_cast<Cpp::MissingDeclarationProblem*>(context->problems()[2].data()) );
      QVERIFY(mdp);
      kDebug() << "problem:" << mdp->description();
      QCOMPARE(mdp->type->containerContext.data(), context->childContexts()[0]);
      QCOMPARE(mdp->type->identifier().toString(), QString("value3"));
      QVERIFY(!mdp->type->assigned.type.isValid());
      QVERIFY(mdp->type->convertedTo.type.isValid());
      QCOMPARE(TypeUtils::removeConstants(mdp->type->convertedTo.type.abstractType(), context)->toString(), QString("int"));
      QCOMPARE(context->childContexts().count(), 3);
    }
    release(context);
  }
  {
    QByteArray test = "class C {}; void test() {C c; int valueName; c.functionName(valueName); }";

    TopDUContext* context = parse( test, DumpAll );
    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(context->problems().count(), 1);
    {
      KSharedPtr<Cpp::MissingDeclarationProblem> mdp( dynamic_cast<Cpp::MissingDeclarationProblem*>(context->problems()[0].data()) );
      QVERIFY(mdp);
      kDebug() << "problem:" << mdp->description();
      QCOMPARE(mdp->type->containerContext.data(), context->childContexts()[0]);
      QCOMPARE(mdp->type->identifier().toString(), QString("functionName"));
      QVERIFY(!mdp->type->assigned.type.isValid());
      QCOMPARE(mdp->type->arguments.count(), 1);
      ///@todo Use the value-names of values given to the function
//       QCOMPARE(mdp->type->arguments[0].second, QString("valueName"));
      QCOMPARE(context->childContexts().count(), 3);
    }
    release(context);
  }  
}

void TestCppCodeCompletion::testImportTypedef() {
  {
    QByteArray test = "class Class { }; typedef Class Klass; class C : public Class { };";

    TopDUContext* context = parse( test, DumpNone /*DumpDUChain | DumpAST */);
    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(context->childContexts().count(), 2);
    QCOMPARE(context->childContexts()[1]->importedParentContexts().count(), 1);
    QCOMPARE(context->childContexts()[1]->importedParentContexts()[0].context(context->topContext()), context->childContexts()[0]);
  }
  {
    QByteArray test = "class A { public: int m; }; template<class Base> class C : public Base { };";

    TopDUContext* context = parse( test, DumpNone /*DumpDUChain | DumpAST */);
    DUChainWriteLocker lock(DUChain::lock());
    Declaration* CDecl = findDeclaration(context, QualifiedIdentifier("C<A>"));
    QVERIFY(CDecl);
    QVERIFY(CDecl->internalContext());
    QVector<DUContext::Import> imports = CDecl->internalContext()->importedParentContexts();
    QCOMPARE(imports.size(), 2);
    QVERIFY(imports[0].context(context));
    QVERIFY(imports[1].context(context));
    QCOMPARE(imports[0].context(context)->type(), DUContext::Template);
    QCOMPARE(imports[1].context(context)->type(), DUContext::Class);
    QCOMPARE(imports[1].context(context)->scopeIdentifier(true), QualifiedIdentifier("A"));
  }
  {
    QByteArray test = "class A { public: int m; }; template<class Base> class C : public Base { }; typedef C<A> TheBase; class B : public TheBase { }; class E : public B{ };";

    TopDUContext* context = parse( test, DumpNone /*DumpDUChain | DumpAST */);
    DUChainWriteLocker lock(DUChain::lock());
    Declaration* typeDef = findDeclaration(context, QualifiedIdentifier("TheBase"));
    QVERIFY(typeDef);
    QVERIFY(typeDef->isTypeAlias());
    QVERIFY(typeDef->type<KDevelop::TypeAliasType>());
    
    Declaration* BDecl = findDeclaration(context, QualifiedIdentifier("B"));
    QVERIFY(BDecl);
    QCOMPARE(BDecl->internalContext()->importedParentContexts().size(), 1);
    QVERIFY(BDecl->internalContext()->importedParentContexts()[0].context(context));
  }
  
}

void TestCppCodeCompletion::testPrivateVariableCompletion() {
  TEST_FILE_PARSE_ONLY
  QByteArray test = "class C {void test() {}; int i; };";

  DUContext* context = parse( test, DumpAll /*DumpDUChain | DumpAST */);
  DUChainWriteLocker lock(DUChain::lock());

  QVERIFY(context->type() != DUContext::Helper);
  QCOMPARE(context->childContexts().count(), 1);
  DUContext* CContext = context->childContexts()[0];
  QCOMPARE(CContext->type(), DUContext::Class);
  QCOMPARE(CContext->childContexts().count(), 2);
  QCOMPARE(CContext->localDeclarations().count(), 2);
  DUContext* testContext = CContext->childContexts()[1];
  QCOMPARE(testContext->type(), DUContext::Other );
  QVERIFY(testContext->owner());
  QCOMPARE(testContext->localScopeIdentifier(), QualifiedIdentifier("test"));
  lock.unlock();
  
  CompletionItemTester tester(testContext);
  kDebug() << "names:" << tester.names;
  QCOMPARE(tester.names.toSet(), (QStringList() << "C" << "i" << "test" << "this").toSet());

  lock.lock();
  release(context);
}

void TestCppCodeCompletion::testCompletionPrefix() {
  TEST_FILE_PARSE_ONLY
  {
    QByteArray method("struct Test {int m;}; Test t;Test* t2;void test() {}");
    TopDUContext* top = parse(method, DumpNone);

    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(top->childContexts().size(), 3);
    //Make sure the completion behind "for <" does not only show types
    QVERIFY(CompletionItemTester(top->childContexts()[2], ";for(int a = 0; a <  t2->").names.contains("m"));
    QVERIFY(CompletionItemTester(top->childContexts()[2], ";for(int a = 0; a <  ").names.contains("t2"));
    //Make sure that only types are shown as template parameters
    QVERIFY(!CompletionItemTester(top->childContexts()[2], "Test<").names.contains("t2"));
    
    QCOMPARE(CompletionItemTester(top->childContexts()[2], "if((t).").names, QStringList() << "m");
    QCOMPARE(CompletionItemTester(top->childContexts()[2], "Test t(&t2->").names, QStringList() << "Test(" << "m");

    QCOMPARE(CompletionItemTester(top->childContexts()[2], "Test(\"(\").").names, QStringList() << "m");
    
    QCOMPARE(CompletionItemTester(top->childContexts()[2], "Test(\" \\\" quotedText( \\\" \").").names, QStringList() << "m");
    
    QVERIFY(CompletionItemTester(top->childContexts()[2], ";int i = ").completionContext->parentContext());
    QVERIFY(CompletionItemTester(top->childContexts()[2], ";int i ( ").completionContext->parentContext());
    bool abort = false;
    QVERIFY(CompletionItemTester(top->childContexts()[2], ";int i = ").completionContext->parentContext()->completionItems(abort).size());
    QVERIFY(CompletionItemTester(top->childContexts()[2], ";int i ( ").completionContext->parentContext()->completionItems(abort).size());
    release(top);
  }
}

void TestCppCodeCompletion::testStringProblem() {
  TEST_FILE_PARSE_ONLY
  {
    QByteArray method("void test() {int i;};");
    TopDUContext* top = parse(method, DumpNone);
    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(top->childContexts().count(), 2);
    CompletionItemTester tester(top->childContexts()[1],QString("bla url('\\\"');"));
    
    QCOMPARE(tester.names.toSet(), (QStringList() << "i" << "test").toSet());;
    release(top);
  }
  {
    QByteArray method("void test() {int i;};");
    TopDUContext* top = parse(method, DumpNone);
    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(top->childContexts().count(), 2);
    CompletionItemTester tester(top->childContexts()[1],QString("bla url(\"http://wwww.bla.de/\");"));
    
    QCOMPARE(tester.names.toSet(), (QStringList() << "i" << "test").toSet());;
    release(top);
  }
}

void TestCppCodeCompletion::testInheritanceVisibility() {
  TEST_FILE_PARSE_ONLY
  QByteArray method("class A { public: class AMyClass {}; }; class B : protected A { public: class BMyClass {}; }; class C : private B{ public: class CMyClass {}; }; class D : public C { class DMyClass{}; }; ");
  TopDUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());

  QCOMPARE(top->childContexts().count(), 4);

  QCOMPARE(top->childContexts()[1]->type(), DUContext::Class);
  QVERIFY(top->childContexts()[1]->owner());
  QVERIFY(Cpp::localClassFromCodeContext(top->childContexts()[1]));
  //From within B, MyClass is visible, because of the protected inheritance
  QCOMPARE(top->childContexts()[1]->localDeclarations().size(), 1);
  QVERIFY(!Cpp::isAccessible(top, dynamic_cast<ClassMemberDeclaration*>(top->childContexts()[0]->localDeclarations()[0]), top, top->childContexts()[1]));
  QCOMPARE(CompletionItemTester(top->childContexts()[1], "A::").names, QStringList() << "AMyClass");
  QCOMPARE(CompletionItemTester(top->childContexts()[1]).names.toSet(), QSet<QString>() << "BMyClass" << "AMyClass" << "A" << "B" );
  QCOMPARE(CompletionItemTester(top, "A::").names, QStringList() << "AMyClass");
  kDebug() << "list:" << CompletionItemTester(top, "B::").names << CompletionItemTester(top, "A::").names.size();
  QCOMPARE(CompletionItemTester(top, "B::").names, QStringList() << "BMyClass");
  QCOMPARE(CompletionItemTester(top->childContexts()[2]).names.toSet(), QSet<QString>() << "CMyClass" << "BMyClass" << "AMyClass" << "C" << "B" << "A");
  QCOMPARE(CompletionItemTester(top, "C::").names.toSet(), QSet<QString>() << "CMyClass");
  QCOMPARE(CompletionItemTester(top->childContexts()[3]).names.toSet(), QSet<QString>() << "DMyClass" << "CMyClass" << "D" << "C" << "B" << "A");
  QCOMPARE(CompletionItemTester(top, "D::").names.toSet(), QSet<QString>() << "CMyClass" ); //DMyClass is private
}



void TestCppCodeCompletion::testConstVisibility() {
  TEST_FILE_PARSE_ONLY
  QByteArray method("struct test { void f(); void e() const; }; int main() { const test a; } void test::e() const { }");
  TopDUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());

  QCOMPARE(top->childContexts().count(), 5);

  kDebug() << "list:" << CompletionItemTester(top->childContexts()[2], "a.").names << CompletionItemTester(top->childContexts()[2], "a.").names.size();
  QCOMPARE(CompletionItemTester(top->childContexts()[2], "a.").names.toSet(), QSet<QString>() << "e");
  kDebug() << "list:" << CompletionItemTester(top->childContexts()[4], "").names << CompletionItemTester(top->childContexts()[4], "").names.size();
  QCOMPARE(CompletionItemTester(top->childContexts()[4], "").names.toSet(), QSet<QString>() << "e" << "test" << "main" << "this");
}

void TestCppCodeCompletion::testConstOverloadVisibility()
{
  // see also: https://bugs.kde.org/show_bug.cgi?id=267877
  TEST_FILE_PARSE_ONLY
  QByteArray method("struct test { test foo(); const test& foo() const; void bar() const; void asdf(); };\n"
                    "int main() { const test testConst; test testNonConst; }");
  TopDUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());

  QCOMPARE(top->childContexts().count(), 3);
  QCOMPARE(top->childContexts().first()->localDeclarations().size(), 4);

  CompletionItemTester tester = CompletionItemTester(top->childContexts()[2], "testConst.");
  QCOMPARE(tester.names.size(), 2);
  // non-const foo() should be hidden by overloaded const version
  QVERIFY(!tester.containsDeclaration(
    top->childContexts().first()->localDeclarations().at(0)
  ));
  // const& foo() const
  QVERIFY(tester.containsDeclaration(
    top->childContexts().first()->localDeclarations().at(1)
  ));
  // bar is always shown (not overloaded and const)
  QVERIFY(tester.containsDeclaration(
    top->childContexts().first()->localDeclarations().at(2)
  ));
  // foo is hidden since it's non-const
  QVERIFY(!tester.containsDeclaration(
    top->childContexts().first()->localDeclarations().at(3)
  ));

  tester = CompletionItemTester(top->childContexts()[2], "testNonConst.");
  QCOMPARE(tester.names.size(), 3);
  // non-const foo()
  QVERIFY(tester.containsDeclaration(
    top->childContexts().first()->localDeclarations().at(0)
  ));
  // const& foo() const hidden by overloaded non-const foo
  QVERIFY(!tester.containsDeclaration(
    top->childContexts().first()->localDeclarations().at(1)
  ));
  // bar is always shown (not overloaded and const)
  QVERIFY(tester.containsDeclaration(
    top->childContexts().first()->localDeclarations().at(2)
  ));
  // foo is shown here as well
  QVERIFY(tester.containsDeclaration(
    top->childContexts().first()->localDeclarations().at(3)
  ));
}

void TestCppCodeCompletion::testFriendVisibility() {
  TEST_FILE_PARSE_ONLY
  QByteArray method("class A { class PrivateClass {}; friend class B; }; class B{};");
  TopDUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());

  QCOMPARE(top->childContexts().count(), 2);

  //No type within A, so there should be no items
  QCOMPARE(CompletionItemTester(top->childContexts()[1], "A::").names, QStringList() << "PrivateClass");
}

void TestCppCodeCompletion::testLocalUsingNamespace() {
  TEST_FILE_PARSE_ONLY
  {
    QByteArray method("namespace Fuu { int test0(); }; namespace Foo { using namespace Fuu; int test() {} } void Bar() { using namespace Foo; int b = test(); }");
    TopDUContext* top = parse(method, DumpAll);

    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(top->childContexts().count(), 4);
    QCOMPARE(top->childContexts()[1]->localDeclarations().size(), 2);
    QCOMPARE(top->childContexts()[3]->localDeclarations().size(), 2);
    QVERIFY(top->childContexts()[1]->localDeclarations()[1]->uses().size());
    QVERIFY(top->childContexts()[3]->findLocalDeclarations(KDevelop::globalImportIdentifier(), KDevelop::CursorInRevision::invalid(), 0, KDevelop::AbstractType::Ptr(), KDevelop::DUContext::NoFiltering).size());
  //   QVERIFY(top->childContexts()[2]->findDeclarations(KDevelop::globalImportIdentifier).size());
    
    QVERIFY(CompletionItemTester(top->childContexts()[3]).names.contains("test"));
    QVERIFY(CompletionItemTester(top->childContexts()[3]).names.contains("test0"));
//     QVERIFY(CompletionItemTester(top->childContexts()[3], "Foo::").names.contains("test0"));
    release(top);
  }
}

void TestCppCodeCompletion::testTemplateFunction() {
    QByteArray method("template<class A> void test(A i); void t() { }");
    TopDUContext* top = parse(method, DumpNone);

    DUChainWriteLocker lock(DUChain::lock());

    QCOMPARE(top->childContexts().count(), 4);

    {
      CompletionItemTester tester1(top->childContexts()[3], "test<");
      QVERIFY(tester1.completionContext->parentContext());
      CompletionItemTester tester2 = tester1.parent();
      QCOMPARE(tester2.items.size(), 1);
      Cpp::NormalDeclarationCompletionItem* item = dynamic_cast<Cpp::NormalDeclarationCompletionItem*>(tester2.items[0].data());
      QVERIFY(item);
      QVERIFY(item->completingTemplateParameters());
    }
    {
      kDebug() << "second test";
      CompletionItemTester tester1(top->childContexts()[3], "test<int>(");
      QVERIFY(tester1.completionContext->parentContext());
      CompletionItemTester tester2 = tester1.parent();
      QCOMPARE(tester2.items.size(), 1);
      Cpp::NormalDeclarationCompletionItem* item = dynamic_cast<Cpp::NormalDeclarationCompletionItem*>(tester2.items[0].data());
      QVERIFY(item);
      QVERIFY(!item->completingTemplateParameters());
      QVERIFY(tester2.completionContext->matchTypes().size() == 1);
      QVERIFY(tester2.completionContext->matchTypes()[0].type<IntegralType>());
    }
    
    release(top);
}

void TestCppCodeCompletion::testTemplateArguments() {
    QByteArray method("template<class T> struct I; typedef I<int> II; template<class T> struct Test { T t; }; ");
    TopDUContext* top = parse(method, DumpNone);

    DUChainWriteLocker lock(DUChain::lock());

    QCOMPARE(top->childContexts().count(), 3);
    
    QVERIFY(findDeclaration(top, QualifiedIdentifier("II")));
    
    Declaration* decl = findDeclaration(top, QualifiedIdentifier("Test<II>::t"));
    QVERIFY(decl);
    QVERIFY(decl->abstractType());
    QVERIFY(decl->type<TypeAliasType>());
    
    //Since II is not template-dependent, the type should have stayed a TypeAliasType
    QCOMPARE(Identifier(decl->abstractType()->toString()), Identifier("II"));
    
    release(top);
}

void TestCppCodeCompletion::testCompletionBehindTypedeffedConstructor() {
    QByteArray method("template<class T> struct A { A(T); int m; }; typedef A<int> TInt; void test() {}");
    TopDUContext* top = parse(method, DumpAll);

    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(top->childContexts().count(), 4);
    QCOMPARE(top->childContexts()[1]->localDeclarations().size(), 2);

    //Member completion
    // NOTE: constructor A is not listed, as you can't call the constructor in this way
    QCOMPARE(CompletionItemTester(top->childContexts()[3], "A<int>().").names.toSet(), (QStringList() << QString("m")).toSet());
    QCOMPARE(CompletionItemTester(top->childContexts()[3], "TInt().").names.toSet(), (QStringList() << QString("m")).toSet());
    
    //Argument-hints
    kDebug() << CompletionItemTester(top->childContexts()[3], "TInt(").parent().names;
    QVERIFY(CompletionItemTester(top->childContexts()[3], "TInt(").parent().names.contains("A"));
    QVERIFY(CompletionItemTester(top->childContexts()[3], "TInt ti(").parent().names.contains("A"));
    
    release(top);
}

void TestCppCodeCompletion::testCompletionInExternalClassDefinition() {
    {
      QByteArray method("class A { class Q; class B; }; class A::B {class C;}; class A::B::C{void test(); }; void A::B::test() {}; void A::B::C::test() {}");
      TopDUContext* top = parse(method, DumpAll);

      DUChainWriteLocker lock(DUChain::lock());
      QCOMPARE(top->childContexts().count(), 7);
      QCOMPARE(top->childContexts()[1]->childContexts().count(), 1);
      QVERIFY(CompletionItemTester(top->childContexts()[1]->childContexts()[0]).names.contains("Q"));
      QVERIFY(CompletionItemTester(top->childContexts()[2]->childContexts()[0]).names.contains("Q"));
      QVERIFY(CompletionItemTester(top->childContexts()[3]).names.contains("Q"));
      QVERIFY(CompletionItemTester(top->childContexts()[5]).names.contains("Q"));
      release(top);
    }
}

void TestCppCodeCompletion::testTemplateMemberAccess() {
  {
    QByteArray method("template<class T> struct I; template<class T> class Test { public: typedef I<T> It; }; template<class T> struct I { }; Test<int>::It test;");
    TopDUContext* top = parse(method, DumpNone);

    DUChainWriteLocker lock(DUChain::lock());

    QCOMPARE(top->localDeclarations().count(), 4);
    AbstractType::Ptr type = TypeUtils::unAliasedType(top->localDeclarations()[3]->abstractType());
    QVERIFY(type);
    IdentifiedType* identified = dynamic_cast<IdentifiedType*>(type.unsafeData());
    QVERIFY(identified);
    QVERIFY(!identified->declarationId().isDirect());
    QString specializationString = IndexedInstantiationInformation(identified->declarationId().specialization()).information().toString();
    QCOMPARE(specializationString, QString("<int>"));
    QCOMPARE(top->localDeclarations()[3]->abstractType()->toString().remove(' '), QString("Test<int>::It"));
    QCOMPARE(TypeUtils::unAliasedType(top->localDeclarations()[3]->abstractType())->toString().remove(' '), QString("I<int>"));
    
    lock.unlock();
    parse(method, DumpNone, 0, QUrl(), top);
    lock.lock();

    QCOMPARE(top->localDeclarations().count(), 4);
    QVERIFY(top->localDeclarations()[3]->abstractType());
    QCOMPARE(TypeUtils::unAliasedType(top->localDeclarations()[3]->abstractType())->toString().remove(' '), QString("I<int>"));
    
    release(top);
  }
  {
    QByteArray method("template<class T> class Test { public: T member; typedef T Data; enum { Value = 3 }; }; typedef Test<int> IntTest; void test() { IntTest tv; int i = Test<int>::Value; }");
    TopDUContext* top = parse(method, DumpNone);

    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(CompletionItemTester(top->childContexts()[3], "Test<int>::").names.toSet(), QSet<QString>() << "Data" << "Value" << "member");

    lock.unlock();
    parse(method, DumpNone, 0, QUrl(), top);
    lock.lock();

    QCOMPARE(top->childContexts().count(), 4);
    QCOMPARE(top->childContexts()[3]->type(), DUContext::Other);
    QCOMPARE(CompletionItemTester(top->childContexts()[3], "IntTest::").names.toSet(), QSet<QString>() << "Data" << "Value" << "member");
    QCOMPARE(CompletionItemTester(top->childContexts()[3], "Test<int>::").names.toSet(), QSet<QString>() << "Data" << "Value" << "member");
    QCOMPARE(CompletionItemTester(top->childContexts()[3], "tv.").names.toSet(), QSet<QString>() << "member");
    release(top);
  }
}

void TestCppCodeCompletion::testNamespaceCompletion() {
  
  QByteArray method("namespace A { class m; namespace Q {}; }; namespace A { class n; int q; }");
  TopDUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());

  QCOMPARE(top->localDeclarations().count(), 2);
  QCOMPARE(top->childContexts().count(), 2);
  QCOMPARE(top->localDeclarations()[0]->identifier(), Identifier("A"));
  QCOMPARE(top->localDeclarations()[1]->identifier(), Identifier("A"));
  QCOMPARE(top->localDeclarations()[0]->kind(), Declaration::Namespace);
  QCOMPARE(top->localDeclarations()[1]->kind(), Declaration::Namespace);
  QVERIFY(!top->localDeclarations()[0]->abstractType());
  QVERIFY(!top->localDeclarations()[1]->abstractType());
  QCOMPARE(top->localDeclarations()[0]->internalContext(), top->childContexts()[0]);
  QCOMPARE(top->localDeclarations()[1]->internalContext(), top->childContexts()[1]);
  
  QCOMPARE(CompletionItemTester(top).names, QStringList() << "A");

  QCOMPARE(CompletionItemTester(top->childContexts()[1], "A::").names.toSet(), QSet<QString>() << "m" << "n" << "Q");
  QCOMPARE(CompletionItemTester(top).itemData("A", KTextEditor::CodeCompletionModel::Prefix).toString(), QString("namespace"));
  release(top);
}

void TestCppCodeCompletion::testNamespaceAliasCompletion() {
  
  QByteArray method("namespace A { class C_A1; class C_A2; namespace Q { class C_Q1; class C_Q2; }; }; "
                    "namespace B = A; " // direct import of a namespace
                    "namespace C = B; " // indirect import through another alias
                    );

  TopDUContext* top = parse(method, DumpNone);
  
  DUChainWriteLocker lock(DUChain::lock());
  
  QCOMPARE(top->localDeclarations().count(), 3);
  QCOMPARE(top->childContexts().count(), 1);
  QCOMPARE(top->localDeclarations()[0]->identifier(), Identifier("A"));
  QCOMPARE(top->localDeclarations()[1]->identifier(), Identifier("B"));
  QCOMPARE(top->localDeclarations()[2]->identifier(), Identifier("C"));
  QCOMPARE(top->localDeclarations()[0]->kind(), Declaration::Namespace);
  QCOMPARE(top->localDeclarations()[1]->kind(), Declaration::NamespaceAlias);
  QCOMPARE(top->localDeclarations()[2]->kind(), Declaration::NamespaceAlias);
  QVERIFY(!top->localDeclarations()[0]->abstractType());
  QVERIFY(!top->localDeclarations()[1]->abstractType());
  QVERIFY(!top->localDeclarations()[2]->abstractType());
  QCOMPARE(top->localDeclarations()[0]->internalContext(), top->childContexts()[0]);
  
  QCOMPARE(CompletionItemTester(top).names.toSet(), QSet<QString>() << "A" << "B" << "C");
  
  QCOMPARE(CompletionItemTester(top->childContexts()[0], "A::").names.toSet(), QSet<QString>() << "C_A1" << "C_A2" << "Q");
  QCOMPARE(CompletionItemTester(top->childContexts()[0], "B::").names.toSet(), QSet<QString>() << "C_A1" << "C_A2" << "Q");
  QCOMPARE(CompletionItemTester(top->childContexts()[0], "C::").names.toSet(), QSet<QString>() << "C_A1" << "C_A2" << "Q");
  QCOMPARE(CompletionItemTester(top).itemData("A", KTextEditor::CodeCompletionModel::Prefix).toString(), QString("namespace"));
  release(top);
}

void TestCppCodeCompletion::testNamespaceAliasCycleCompletion() {
  
  QByteArray method("namespace A { class C_A1; class C_A2; namespace Q { class C_Q1; class C_Q2; }; }; "
                    "namespace B = A; namespace A = B; ");
  
  TopDUContext* top = parse(method, DumpNone);
  
  DUChainWriteLocker lock(DUChain::lock());
  
  QCOMPARE(top->localDeclarations().count(), 3);
  QCOMPARE(top->childContexts().count(), 1);
  QCOMPARE(top->localDeclarations()[0]->identifier(), Identifier("A"));
  QCOMPARE(top->localDeclarations()[1]->identifier(), Identifier("B"));
  QCOMPARE(top->localDeclarations()[2]->identifier(), Identifier("A"));
  QCOMPARE(top->localDeclarations()[0]->kind(), Declaration::Namespace);
  QCOMPARE(top->localDeclarations()[1]->kind(), Declaration::NamespaceAlias);
  QCOMPARE(top->localDeclarations()[2]->kind(), Declaration::NamespaceAlias);
  QVERIFY(!top->localDeclarations()[0]->abstractType());
  QVERIFY(!top->localDeclarations()[1]->abstractType());
  QVERIFY(!top->localDeclarations()[2]->abstractType());
  QCOMPARE(top->localDeclarations()[0]->internalContext(), top->childContexts()[0]);
  
  QCOMPARE(CompletionItemTester(top).names.toSet(), QSet<QString>() << "A" << "B");
  
  QCOMPARE(CompletionItemTester(top->childContexts()[0], "A::").names.toSet(), QSet<QString>() << "C_A1" << "C_A2" << "Q");
  QCOMPARE(CompletionItemTester(top->childContexts()[0], "B::").names.toSet(), QSet<QString>() << "C_A1" << "C_A2" << "Q");
  QCOMPARE(CompletionItemTester(top).itemData("A", KTextEditor::CodeCompletionModel::Prefix).toString(), QString("namespace"));
  release(top);
}

void TestCppCodeCompletion::testAfterNamespace()
{
  QByteArray method("void foo(); namespace asdf { namespace foobar {} }");

  TopDUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock;

  CompletionItemTester tester(top, "namespace");
  QCOMPARE(tester.names, QStringList() << "asdf");
  release(top);
}

void TestCppCodeCompletion::testIndirectImports()
{
  {
    addInclude("testIndirectImportsHeader1.h", "class C {};");
    addInclude("testIndirectImportsHeader2.h", "template<class T> class D : public T {};");
    
    QByteArray method("#include \"testIndirectImportsHeader2.h\"\n#include \"testIndirectImportsHeader1.h\"\n typedef D<C> Base; class MyClass : public C, public Base {}; ");

    TopDUContext* top = parse(method, DumpNone);

    DUChainWriteLocker lock(DUChain::lock());

    QCOMPARE(top->importedParentContexts().size(), 2);
    QCOMPARE(top->childContexts().count(), 1);
    QCOMPARE(top->childContexts()[0]->importedParentContexts().count(), 2);
    QVERIFY(!top->childContexts()[0]->importedParentContexts()[0].isDirect()); //Should not be direct, since it crosses a header
    QVERIFY(!top->childContexts()[0]->importedParentContexts()[0].indirectDeclarationId().isDirect()); //Should not be direct, since it crosses a header
    QVERIFY(!top->childContexts()[0]->importedParentContexts()[1].isDirect()); //Should not be direct, since it crosses a header
    QVERIFY(!top->childContexts()[0]->importedParentContexts()[1].indirectDeclarationId().isDirect()); //Should not be direct, since it crosses a header
    DUContext* import1 = top->childContexts()[0]->importedParentContexts()[1].context(top);
    QVERIFY(import1);
    QCOMPARE(import1->importedParentContexts().count(), 2); //The template-context is also imported
    QVERIFY(!import1->importedParentContexts()[1].isDirect()); //Should not be direct, since it crosses a header
    QVERIFY(!import1->importedParentContexts()[1].indirectDeclarationId().isDirect()); //Should not be direct, since it crosses a header

    DUContext* import2 = import1->importedParentContexts()[0].context(top);
    QVERIFY(import2);
    QCOMPARE(import2->importedParentContexts().count(), 0);

    release(top);
  }
}

void TestCppCodeCompletion::testSameNamespace() {
  {
    addInclude("testSameNamespaceClassHeader.h", "namespace A {\n class B\n {\n \n};\n \n}");
    
    QByteArray method("#include \"testSameNamespaceClassHeader.h\"\n namespace A {\n namespace AA {\n};\n };\n");

    TopDUContext* top = parse(method, DumpNone);

    DUChainWriteLocker lock(DUChain::lock());

    QCOMPARE(top->importedParentContexts().size(), 1);
    QVERIFY(!top->parentContext());
    QCOMPARE(top->childContexts().count(), 1);
    QCOMPARE(top->childContexts()[0]->childContexts().count(), 1);
    {
      kDebug() << CompletionItemTester(top->childContexts()[0]).names;
      QCOMPARE(CompletionItemTester(top->childContexts()[0]).names.toSet(), QSet<QString>() << "B" << "A" << "AA");
      QCOMPARE(CompletionItemTester(top->childContexts()[0]->childContexts()[0]).names.toSet(), QSet<QString>() << "B" << "A" << "AA");
    }
    
    release(top);
  }

  {
    //                 0         1         2         3         4         5         6         7
    //                 0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
    QByteArray method("namespace A { class C { }; void test2() {} } namespace A { void test() { } class C {};}");

    TopDUContext* top = parse(method, DumpNone);

    DUChainWriteLocker lock(DUChain::lock());

    QVERIFY(!top->parentContext());
    QCOMPARE(top->childContexts().count(), 2);
    QCOMPARE(top->childContexts()[1]->childContexts().count(), 3);
    QCOMPARE(top->childContexts()[1]->localDeclarations().count(), 2);
    FunctionDefinition* funDef = dynamic_cast<KDevelop::FunctionDefinition*>(top->childContexts()[1]->localDeclarations()[0]);
    QVERIFY(!funDef->hasDeclaration());

  //   lock.unlock();
    {
      kDebug() << CompletionItemTester(top->childContexts()[1]->childContexts()[2]).names;
      QCOMPARE(CompletionItemTester(top->childContexts()[1]->childContexts()[2]).names.toSet(), QSet<QString>() << "C" << "A");
      QCOMPARE(CompletionItemTester(top->childContexts()[1]).names.toSet(), QSet<QString>() << "C" << "A");
      QCOMPARE(CompletionItemTester(top->childContexts()[1]->childContexts()[1]).names.toSet(), QSet<QString>() << "C" << "test2" << "test" << "A");
    }
    
    release(top);
  }
}

void TestCppCodeCompletion::testUnnamedNamespace() {
  TEST_FILE_PARSE_ONLY

  //                 0         1         2         3         4         5         6         7
  //                 0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
  QByteArray method("namespace {int a;} namespace { int b; }; void test() {a = 3;}");

  TopDUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());

  QVERIFY(!top->parentContext());
  QCOMPARE(top->childContexts().count(), 4);
  QCOMPARE(top->localDeclarations().count(), 3);
  kDebug() << top->localDeclarations()[0]->range().castToSimpleRange().textRange();
  QCOMPARE(top->localDeclarations()[0]->range().castToSimpleRange().textRange(), KTextEditor::Range(0, 10, 0, 10));
  QVERIFY(findDeclaration(top, QualifiedIdentifier("a")));
  QVERIFY(findDeclaration(top, QualifiedIdentifier("b")));
  QVERIFY(findDeclaration(top, QualifiedIdentifier("a"))->uses().size());
  PersistentSymbolTable::FilteredDeclarationIterator decls = KDevelop::PersistentSymbolTable::self().getFilteredDeclarations(QualifiedIdentifier(Cpp::unnamedNamespaceIdentifier().identifier()), top->recursiveImportIndices());
  QVERIFY(decls);
  QCOMPARE(top->findLocalDeclarations(Cpp::unnamedNamespaceIdentifier().identifier()).size(), 2);
  QCOMPARE(top->findDeclarations(QualifiedIdentifier(Cpp::unnamedNamespaceIdentifier().identifier())).size(), 2);

//   lock.unlock();
  {
    Cpp::CodeCompletionContext::Ptr cptr( new  Cpp::CodeCompletionContext(DUContextPointer(top), "; ", QString(), top->range().end) );
    bool abort = false;
    typedef KSharedPtr <KDevelop::CompletionTreeItem > Item;
    
    QList <Item > items = cptr->completionItems(abort);
    foreach(Item i, items) {
      Cpp::NormalDeclarationCompletionItem* decItem  = dynamic_cast<Cpp::NormalDeclarationCompletionItem*>(i.data());
      QVERIFY(decItem);
      kDebug() << decItem->declaration()->toString();
      kDebug() << i->data(fakeModel().index(0, KTextEditor::CodeCompletionModel::Name), Qt::DisplayRole, 0).toString();
    }
    
    //Have been filtered out, because only types are shown from the global scope
    QCOMPARE(items.count(), 0); //C, test, and i
  }
  {
    Cpp::CodeCompletionContext::Ptr cptr( new  Cpp::CodeCompletionContext(DUContextPointer(top->childContexts()[3]), "; ", QString(), top->range().end) );
    bool abort = false;
    typedef KSharedPtr <KDevelop::CompletionTreeItem > Item;
    
    QList <Item > items = cptr->completionItems(abort);
    foreach(Item i, items) {
      Cpp::NormalDeclarationCompletionItem* decItem  = dynamic_cast<Cpp::NormalDeclarationCompletionItem*>(i.data());
      QVERIFY(decItem);
      kDebug() << decItem->declaration()->toString();
      kDebug() << i->data(fakeModel().index(0, KTextEditor::CodeCompletionModel::Name), Qt::DisplayRole, 0).toString();
    }
    
    QCOMPARE(items.count(), 3); //b, a, and test
  }
  
//   lock.lock();
  release(top);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TestCppCodeCompletion::testCompletionContext() {
  TEST_FILE_PARSE_ONLY

  QByteArray test = "#include \"testFile1.h\"\n";
  test += "#include \"testFile2.h\"\n";
  test += "void test() { }";

  DUContext* context = parse( test, DumpNone /*DumpDUChain | DumpAST */);
  DUChainWriteLocker lock(DUChain::lock());

  QVERIFY(context->childContexts().count());
  DUContext* testContext = context->childContexts()[0];
  QCOMPARE( testContext->type(), DUContext::Function );

  lock.unlock();
  {
    ///Test whether a recursive function-call context is created correctly
    Cpp::CodeCompletionContext::Ptr cptr( new  Cpp::CodeCompletionContext(DUContextPointer(DUContextPointer(context)), "; globalFunction(globalFunction(globalHonk, ", QString(), CursorInRevision::invalid() ) );
    Cpp::CodeCompletionContext& c(*cptr);
    QVERIFY( c.isValid() );
    QVERIFY( c.accessType() == Cpp::CodeCompletionContext::NoMemberAccess );
    QVERIFY( !c.memberAccessContainer().isValid() );

    //globalHonk is of type Honk. Check whether all matching functions were rated correctly by the overload-resolution.
    //The preferred parent-function in the list should be "Honk globalFunction( const Honk&, const Heinz& h )", because the first argument maches globalHonk
    Cpp::CodeCompletionContext* function = c.parentContext();
    QVERIFY(function);
    QVERIFY(function->accessType() == Cpp::CodeCompletionContext::FunctionCallAccess);
    QVERIFY(!function->functions().isEmpty());

    lock.lock();
    for( Cpp::CodeCompletionContext::FunctionList::const_iterator it = function->functions().begin(); it != function->functions().end(); ++it )
      kDebug(9007) << (*it).function.declaration()->toString() << ((*it).function.isViable() ? QString("(viable)") : QString("(not viable)")) ;
    lock.unlock();

    QCOMPARE(function->functions().size(), 4);
    QVERIFY(function->functions()[0].function.isViable());
    //Because Honk has a conversion-function to int, globalFunction(int) is yet viable(although it can take only 1 parameter)
    QVERIFY(function->functions()[1].function.isViable());
    //Because Erna has a constructor that takes "const Honk&", globalFunction(Erna) is yet viable(although it can take only 1 parameter)
    QVERIFY(function->functions()[2].function.isViable());
    //Because a value of type Honk is given, 2 globalFunction's are not viable
    QVERIFY(!function->functions()[3].function.isViable());


    function = function->parentContext();
    QVERIFY(function);
    QVERIFY(function->accessType() == Cpp::CodeCompletionContext::FunctionCallAccess);
    QVERIFY(!function->functions().isEmpty());
    QVERIFY(!function->parentContext());
    QVERIFY(function->functions().size() == 4);
    //Because no arguments were given, all functions are viable
    QVERIFY(function->functions()[0].function.isViable());
    QVERIFY(function->functions()[1].function.isViable());
    QVERIFY(function->functions()[2].function.isViable());
    QVERIFY(function->functions()[3].function.isViable());
  }

  {
    ///The context is a function, and there is no prefix-expression, so it should be normal completion.
    DUContextPointer contPtr(context);
    Cpp::CodeCompletionContext c(contPtr, "{", QString(), CursorInRevision::invalid() );
    QVERIFY( c.isValid() );
    QVERIFY( c.accessType() == Cpp::CodeCompletionContext::NoMemberAccess );
    QVERIFY( !c.memberAccessContainer().isValid() );
  }

  lock.lock();
  release(context);
}


void TestCppCodeCompletion::testTypeConversion() {
  TEST_FILE_PARSE_ONLY

  QByteArray test = "#include \"testFile1.h\"\n";
  test += "#include \"testFile2.h\"\n";
  test += "#include \"testFile3.h\"\n";
  test += "int n;\n";
  test += "void test() { }\n";

  DUContext* context = parse( test, DumpNone /*DumpDUChain | DumpAST */);
  DUChainWriteLocker lock(DUChain::lock());

  DUContext* testContext = context->childContexts()[0];
  QCOMPARE( testContext->type(), DUContext::Function );

  QVERIFY(findDeclaration( testContext, QualifiedIdentifier("Heinz") ));
  QVERIFY(findDeclaration( testContext, QualifiedIdentifier("Erna") ));
  Declaration* decl = findDeclaration( testContext, QualifiedIdentifier("Erna") );
  QVERIFY(decl);
  QVERIFY(decl->logicalInternalContext(context->topContext()));
  QVERIFY(findDeclaration( testContext, QualifiedIdentifier("Honk") ));
  QVERIFY(findDeclaration( testContext, QualifiedIdentifier("A") ));
  QVERIFY(findDeclaration( testContext, QualifiedIdentifier("B") ));
  QVERIFY(findDeclaration( testContext, QualifiedIdentifier("test") ));
  QVERIFY(findDeclaration( testContext, QualifiedIdentifier("n") ));
  AbstractType::Ptr Heinz = findDeclaration( testContext, QualifiedIdentifier("Heinz") )->abstractType();
  AbstractType::Ptr Erna = findDeclaration( testContext, QualifiedIdentifier("Erna") )->abstractType();
  AbstractType::Ptr Honk = findDeclaration( testContext, QualifiedIdentifier("Honk") )->abstractType();
  AbstractType::Ptr A = findDeclaration( testContext, QualifiedIdentifier("A") )->abstractType();
  AbstractType::Ptr B = findDeclaration( testContext, QualifiedIdentifier("B") )->abstractType();
  AbstractType::Ptr n = findDeclaration( testContext, QualifiedIdentifier("n") )->abstractType();

  QVERIFY(n);

  {
    FunctionType::Ptr test = findDeclaration( testContext, QualifiedIdentifier("test") )->type<FunctionType>();
    QVERIFY(test);

    Cpp::TypeConversion conv(context->topContext());
    QVERIFY(!conv.implicitConversion(test->returnType()->indexed(), Heinz->indexed(), false));
    QVERIFY(!conv.implicitConversion(Heinz->indexed(), test->returnType()->indexed(), false));
    QVERIFY(!conv.implicitConversion(test->returnType()->indexed(), n->indexed(), false));
    QVERIFY(!conv.implicitConversion(n->indexed(), test->returnType()->indexed(), false));
  }
  //lock.unlock();
  {
    ///Test whether a recursive function-call context is created correctly
    Cpp::TypeConversion conv(context->topContext());
    QVERIFY( !conv.implicitConversion(Honk->indexed(), Heinz->indexed()) );
    QVERIFY( conv.implicitConversion(Honk->indexed(), typeInt->indexed()) ); //Honk has operator int()
    QVERIFY( conv.implicitConversion(Honk->indexed(), Erna->indexed()) ); //Erna has constructor that takes Honk
    QVERIFY( !conv.implicitConversion(Erna->indexed(), Heinz->indexed()) );

    ///@todo reenable once base-classes are parsed correctly
    //QVERIFY( conv.implicitConversion(B, A) ); //B is based on A, so there is an implicit copy-constructor that creates A from B
    //QVERIFY( conv.implicitConversion(Heinz, Erna) ); //Heinz is based on Erna, so there is an implicit copy-constructor that creates Erna from Heinz

  }

  //lock.lock();
  release(context);
}

KDevelop::IndexedType toReference(IndexedType t) {
  
  ReferenceType::Ptr refType( new ReferenceType);
  refType->setBaseType(t.abstractType());
  return refType->indexed();
}

KDevelop::IndexedType toPointer(IndexedType t) {
  
  PointerType::Ptr refType( new PointerType);
  refType->setBaseType(t.abstractType());
  return refType->indexed();
}

void TestCppCodeCompletion::testTypeConversion2() {
  {
    QByteArray test = "class A {}; class B {public: explicit B(const A&); explicit B(const int&){}; private: operator A() const {}; }; class C : public B{private: C(B) {}; };";
    TopDUContext* context = parse( test, DumpAll /*DumpDUChain | DumpAST */);
    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(context->localDeclarations().size(), 3);
    QVERIFY(context->localDeclarations()[0]->internalContext());
    QCOMPARE(context->localDeclarations()[1]->internalContext()->localDeclarations().count(), 3);
    ClassFunctionDeclaration* classFun = dynamic_cast<ClassFunctionDeclaration*>(context->localDeclarations()[1]->internalContext()->localDeclarations()[0]);
    QVERIFY(classFun);
    QVERIFY(classFun->isExplicit());
    classFun = dynamic_cast<ClassFunctionDeclaration*>(context->localDeclarations()[1]->internalContext()->localDeclarations()[1]);
    QVERIFY(classFun);
    QVERIFY(classFun->isExplicit());
    
    
    Cpp::TypeConversion conv(context);
    QVERIFY( !conv.implicitConversion(context->localDeclarations()[2]->indexedType(), context->localDeclarations()[0]->indexedType()) );
    QVERIFY( conv.implicitConversion(context->localDeclarations()[2]->indexedType(), context->localDeclarations()[1]->indexedType()) );
    QVERIFY( !conv.implicitConversion(context->localDeclarations()[1]->indexedType(), context->localDeclarations()[2]->indexedType()) );
    QVERIFY( !conv.implicitConversion(context->localDeclarations()[1]->indexedType(), context->localDeclarations()[0]->indexedType()) );
    QVERIFY( !conv.implicitConversion(context->localDeclarations()[0]->indexedType(), context->localDeclarations()[1]->indexedType()) );

    QVERIFY( !conv.implicitConversion(toReference(context->localDeclarations()[2]->indexedType()), toReference(context->localDeclarations()[0]->indexedType()) ));
    QVERIFY( conv.implicitConversion(toReference(context->localDeclarations()[2]->indexedType()), toReference(context->localDeclarations()[1]->indexedType()) ));
    QVERIFY( !conv.implicitConversion(toReference(context->localDeclarations()[1]->indexedType()), toReference(context->localDeclarations()[2]->indexedType()) ));
    QVERIFY( !conv.implicitConversion(toReference(context->localDeclarations()[1]->indexedType()), toReference(context->localDeclarations()[0]->indexedType()) ));
    QVERIFY( !conv.implicitConversion(toReference(context->localDeclarations()[0]->indexedType()), toReference(context->localDeclarations()[1]->indexedType()) ));

    QVERIFY( !conv.implicitConversion(toPointer(context->localDeclarations()[2]->indexedType()), toPointer(context->localDeclarations()[0]->indexedType()) ));
    QVERIFY( conv.implicitConversion(toPointer(context->localDeclarations()[2]->indexedType()), toPointer(context->localDeclarations()[1]->indexedType()) ));
    QVERIFY( !conv.implicitConversion(toPointer(context->localDeclarations()[1]->indexedType()), toPointer(context->localDeclarations()[2]->indexedType()) ));
    QVERIFY( !conv.implicitConversion(toPointer(context->localDeclarations()[1]->indexedType()), toPointer(context->localDeclarations()[0]->indexedType()) ));
    QVERIFY( !conv.implicitConversion(toPointer(context->localDeclarations()[0]->indexedType()), toPointer(context->localDeclarations()[1]->indexedType()) ));
    
    release(context);
  }
  {
    QByteArray test = "const char** b; char** c; char** const d; char* const * e; char f; const char q; ";
    TopDUContext* context = parse( test, DumpNone /*DumpDUChain | DumpAST */);
    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(context->localDeclarations().size(), 6);
    Cpp::TypeConversion conv(context);
    QVERIFY( !conv.implicitConversion(context->localDeclarations()[0]->indexedType(), context->localDeclarations()[1]->indexedType()) );
    PointerType::Ptr fromPointer = context->localDeclarations()[1]->indexedType().type< PointerType>();
    QVERIFY(fromPointer);
    QVERIFY( !(fromPointer->modifiers() & AbstractType::ConstModifier));
    QVERIFY( conv.implicitConversion(context->localDeclarations()[1]->indexedType(), context->localDeclarations()[0]->indexedType()) );
    QVERIFY( conv.implicitConversion(context->localDeclarations()[1]->indexedType(), context->localDeclarations()[2]->indexedType()) );
    QVERIFY( !conv.implicitConversion(context->localDeclarations()[0]->indexedType(), context->localDeclarations()[2]->indexedType()) );
    QVERIFY( conv.implicitConversion(context->localDeclarations()[2]->indexedType(), context->localDeclarations()[0]->indexedType()) );
    QVERIFY( conv.implicitConversion(context->localDeclarations()[2]->indexedType(), context->localDeclarations()[1]->indexedType()) );
    QVERIFY( !conv.implicitConversion(context->localDeclarations()[3]->indexedType(), context->localDeclarations()[0]->indexedType()) );
    QVERIFY( !conv.implicitConversion(context->localDeclarations()[3]->indexedType(), context->localDeclarations()[1]->indexedType()) );
    QVERIFY( !conv.implicitConversion(context->localDeclarations()[0]->indexedType(), context->localDeclarations()[3]->indexedType()) );
    QVERIFY( conv.implicitConversion(context->localDeclarations()[1]->indexedType(), context->localDeclarations()[3]->indexedType()) );
    QVERIFY( !conv.implicitConversion(context->localDeclarations()[3]->indexedType(), context->localDeclarations()[1]->indexedType()) );
    QVERIFY( conv.implicitConversion(context->localDeclarations()[4]->indexedType(), context->localDeclarations()[5]->indexedType()) );
    QVERIFY( conv.implicitConversion(context->localDeclarations()[5]->indexedType(), context->localDeclarations()[4]->indexedType()) );
    
    release(context);
  }
  
  {
    QByteArray test = "class A {}; class C {}; enum M { Em }; template<class T> class B{ public:B(T t); }; ";
    TopDUContext* context = parse( test, DumpNone /*DumpDUChain | DumpAST */);
    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(context->localDeclarations().size(), 4);
    QCOMPARE(context->childContexts().size(), 5);
    Cpp::TypeConversion conv(context);
    Declaration* decl = findDeclaration(context, QualifiedIdentifier("B<A>"));
    QVERIFY(decl);
    kDebug() << decl->toString();
    
    QVERIFY( conv.implicitConversion(context->localDeclarations()[0]->indexedType(), decl->indexedType()) );
    
    decl = findDeclaration(context, QualifiedIdentifier("B<M>"));
    QVERIFY(decl);
    kDebug() << decl->toString();
    QCOMPARE(context->childContexts()[2]->localDeclarations().size(), 1);
    QVERIFY( conv.implicitConversion(context->childContexts()[2]->localDeclarations()[0]->indexedType(), decl->indexedType()) );    
    
    release(context);
  }  
}

void TestCppCodeCompletion::testInclude() {
  TEST_FILE_PARSE_ONLY

  addInclude("file1.h", "#include \"testFile1.h\"\n#include \"testFile2.h\"\n");


  QByteArray test = "#include \"file1.h\"  \n  struct Cont { operator int() {}; }; void test( int c = 5 ) { this->test( Cont(), 1, 5.5, 6); }; HONK undefinedHonk;";
  DUContext* c = parse( test, DumpNone /*DumpDUChain | DumpAST */);
  DUChainWriteLocker lock(DUChain::lock());

  QVERIFY(c->topContext()->usingImportsCache());
  
  Declaration* decl = findDeclaration(c, QualifiedIdentifier("globalHeinz"));
  QVERIFY(decl);
  QVERIFY(decl->abstractType());
  QCOMPARE(decl->abstractType()->toString(), QString("Heinz"));

  decl = findDeclaration(c, QualifiedIdentifier("globalErna"));
  QVERIFY(decl);
  QVERIFY(decl->abstractType());
  QCOMPARE(decl->abstractType()->toString(), QString("Erna"));

  decl = findDeclaration(c, QualifiedIdentifier("globalInt"));
  QVERIFY(decl);
  QVERIFY(decl->abstractType());
  QCOMPARE(decl->abstractType()->toString(), QString("int"));

  decl = findDeclaration(c, QualifiedIdentifier("Honk"));
  QVERIFY(decl);
  QVERIFY(decl->abstractType());
  QCOMPARE(decl->abstractType()->toString(), QString("Honk"));

  decl = findDeclaration(c, QualifiedIdentifier("honky"));
  QVERIFY(decl);
  QVERIFY(decl->abstractType());
  QCOMPARE(decl->abstractType()->toString(), QString("Honk"));

  decl = findDeclaration(c, QualifiedIdentifier("globalHonk"));
  QVERIFY(decl);
  QVERIFY(decl->abstractType());
  QCOMPARE(decl->abstractType()->toString(), QString("Honk"));

  decl = findDeclaration(c, QualifiedIdentifier("globalMacroHonk"));
  QVERIFY(decl);
  QVERIFY(decl->abstractType());
  QCOMPARE(decl->abstractType()->toString(), QString("Honk"));

  ///HONK was #undef'ed in testFile2, so this must be unresolved.
  decl = findDeclaration(c, QualifiedIdentifier("undefinedHonk"));
  QVERIFY(decl);
  QVERIFY(decl->abstractType().cast<DelayedType>());


  Cpp::ExpressionParser parser;

  ///The following test tests the expression-parser, but it is here because the other tests depend on it
  lock.unlock();
  Cpp::ExpressionEvaluationResult result = parser.evaluateExpression( "globalHonk.erna", DUContextPointer( c ) );
  lock.lock();

  QVERIFY(result.isValid());
  QVERIFY(result.isInstance);
  QVERIFY(result.type);
  QCOMPARE(result.type.abstractType()->toString(), QString("Erna&"));


  ///Test overload-resolution
  lock.unlock();
  result = parser.evaluateExpression( "globalClass.function(globalHeinz)", DUContextPointer(c));
  lock.lock();

  QVERIFY(result.isValid());
  QVERIFY(result.isInstance);
  QVERIFY(result.type);
  QCOMPARE(result.type.abstractType()->toString(), QString("Heinz"));

  lock.unlock();
  result = parser.evaluateExpression( "globalClass.function(globalHonk.erna)", DUContextPointer(c));
  lock.lock();

  QVERIFY(result.isValid());
  QVERIFY(result.isInstance);
  QVERIFY(result.type);
  QCOMPARE(result.type.abstractType()->toString(), QString("Erna"));

  //No matching function for type Honk. Since the expression-parser is not set to "strict", it returns any found function with the right name.
  lock.unlock();
  result = parser.evaluateExpression( "globalClass.function(globalHonk)", DUContextPointer(c));
  lock.lock();

  QVERIFY(result.isValid());
  QVERIFY(result.isInstance);
  QVERIFY(result.type);
  //QCOMPARE(result.type.abstractType()->toString(), QString("Heinz"));


  lock.unlock();
  result = parser.evaluateExpression( "globalFunction(globalHeinz)", DUContextPointer(c));
  lock.lock();

  QVERIFY(result.isValid());
  QVERIFY(result.isInstance);
  QVERIFY(result.type);
  QCOMPARE(result.type.abstractType()->toString(), QString("Heinz"));
  lock.unlock();

  result = parser.evaluateExpression( "globalFunction(globalHonk.erna)", DUContextPointer(c));
  lock.lock();

  QVERIFY(result.isValid());
  QVERIFY(result.isInstance);
  QVERIFY(result.type);
  QCOMPARE(result.type.abstractType()->toString(), QString("Erna"));

  release(c);
}

void TestCppCodeCompletion::testUpdateChain() {
  TEST_FILE_PARSE_ONLY

{
    QByteArray text("#define Q_FOREACH(variable, container) for (QForeachContainer<__typeof__(container)> _container_(container); !_container_.brk && _container_.i != _container_.e; __extension__ ({ ++_container_.brk; ++_container_.i; })) for (variable = *_container_.i;; __extension__ ({--_container_.brk; break;})) \nvoid test() { Q_FOREACH(int a, b) { int i; } }");
    TopDUContext* top = parse( text, DumpAll );

    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(top->childContexts().count(), 2);
    QCOMPARE(top->childContexts()[1]->childContexts().count(), 2);
    QCOMPARE(top->childContexts()[1]->childContexts()[1]->childContexts().count(), 2);
    QCOMPARE(top->childContexts()[1]->childContexts()[1]->childContexts()[1]->localDeclarations().count(), 1);
    IndexedDeclaration decl(top->childContexts()[1]->childContexts()[1]->childContexts()[1]->localDeclarations()[0]);
    QVERIFY(decl.data());
    QCOMPARE(decl.data()->identifier().toString(), QString("i"));
    
    parse(text, DumpNone, 0, QUrl(), top);
    QVERIFY(decl.data()); //Make sure the declaration has been updated, and not deleted
    
    release(top);
}
}

void TestCppCodeCompletion::testHeaderSections() {
  TEST_FILE_PARSE_ONLY
  /**
   * Make sure that the ends of header-sections are recognized correctly
   * */

  addInclude( "someHeader.h", "\n" );
  addInclude( "otherHeader.h", "\n" );

  IncludeFileList includes;

  IndexedString turl("ths.h");

  QCOMPARE(preprocess(turl, "#include \"someHeader.h\"\nHello", includes, 0, true), QString("\n"));
  QCOMPARE(includes.count(), 1);
  includes.clear();

  QCOMPARE(preprocess(turl, "#include \"someHeader.h\"\nHello", includes, 0, false), QString("\nHello"));
  QCOMPARE(includes.count(), 1);
  includes.clear();

  QCOMPARE(preprocess(turl, "#include \"someHeader.h\"\n#include \"otherHeader.h\"\nHello", includes, 0, false), QString("\n\nHello"));
  QCOMPARE(includes.count(), 2);
  includes.clear();

  QCOMPARE(preprocess(turl, "#include \"someHeader.h\"\n#include \"otherHeader.h\"\nHello", includes, 0, true), QString("\n\n"));
  QCOMPARE(includes.count(), 2);
  includes.clear();

  QCOMPARE(preprocess(turl, "#ifndef GUARD\n#define GUARD\n#include \"someHeader.h\"\nHello\n#endif", includes, 0, true), QString("\n\n\n"));
  QCOMPARE(includes.count(), 1);
  includes.clear();

  QCOMPARE(preprocess(turl, "#ifndef GUARD\n#define GUARD\n#include \"someHeader.h\"\nHello\n#endif", includes, 0, false), QString("\n\n\nHello\n"));
  QCOMPARE(includes.count(), 1);
  includes.clear();
}

void TestCppCodeCompletion::testForwardDeclaration()
{
  addInclude( "testdeclaration.h", "class Test{ };" );
  QByteArray method("#include \"testdeclaration.h\"\n class Test; ");

  DUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());


  Declaration* decl = findDeclaration(top, Identifier("Test"), top->range().end);
  QVERIFY(decl);
  QVERIFY(decl->abstractType());
  AbstractType::Ptr t(decl->abstractType());
  QVERIFY(dynamic_cast<const IdentifiedType*>(t.unsafeData()));
  QVERIFY(!decl->isForwardDeclaration());

  release(top);
}

void TestCppCodeCompletion::testUsesThroughMacros() {
  {
    QByteArray method("int x;\n#define TEST(X) X\ny = TEST(x);");

    DUContext* top = parse(method, DumpNone);

    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(top->localDeclarations().count(), 1);
    QCOMPARE(top->localDeclarations()[0]->uses().count(), 1);
    QCOMPARE(top->localDeclarations()[0]->uses().begin()->count(), 1);
    QCOMPARE(top->localDeclarations()[0]->uses().begin()->at(0).start.column, 9);
    QCOMPARE(top->localDeclarations()[0]->uses().begin()->at(0).end.column, 10);
  }
  {
    ///2 uses of x, that go through the macro TEST(..), and effectively are in line 2 column 5.
    QByteArray method("int x;\n#define TEST(X) void test() { int z = X; int q = X; }\nTEST(x)");

    kDebug() << method;
    DUContext* top = parse(method, DumpNone);

    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(top->localDeclarations().count(), 2);
    QCOMPARE(top->localDeclarations()[0]->uses().count(), 1);
    //Since uses() returns unique uses, and both uses of x in TEST(x) have the same range,
    //only one use is returned.
    QCOMPARE(top->localDeclarations()[0]->uses().begin()->count(), 1);

    RangeInRevision range1(top->localDeclarations()[0]->uses().begin()->at(0));
    QCOMPARE(range1.start.line, 2);
    QCOMPARE(range1.end.line, 2);
    QCOMPARE(range1.start.column, 5);
    QCOMPARE(range1.end.column, 6);
  }
}

void TestCppCodeCompletion::testMacroIncludeDirectives()
{
  addInclude( "macroincludedirectivetest1.h", "class Test1{ };" );
  addInclude( "macro includedirectivetest2.h", "class Test1{ };" );

  {
    QByteArray method("#define TEST macroincludedirectivetest1.h \n #define TEST_HPP <TEST> \n #include TEST_HPP\n");

    DUContext* top = parse(method, DumpNone);

    DUChainWriteLocker lock(DUChain::lock());

    QCOMPARE(top->importedParentContexts().size(), 1);

    release(top);
  }
  
  {
    QByteArray method("#define TEST \"macroincludedirectivetest1.h\" \n #include TEST\n");

    DUContext* top = parse(method, DumpNone);

    DUChainWriteLocker lock(DUChain::lock());

    QCOMPARE(top->importedParentContexts().size(), 1);

    release(top);
  }

  {
    QByteArray method("#define TEST <macroincludedirectivetest1.h> \n #include TEST\n");

    DUContext* top = parse(method, DumpNone);

    DUChainWriteLocker lock(DUChain::lock());

    QCOMPARE(top->importedParentContexts().size(), 1);

    release(top);
  }

  {
    QByteArray method("#include \"macro includedirectivetest2.h\"\n");

    DUContext* top = parse(method, DumpNone);

    DUChainWriteLocker lock(DUChain::lock());

    QCOMPARE(top->importedParentContexts().size(), 1);

    release(top);
  }

  {
    QByteArray method("#define TEST \"macro includedirectivetest2.h\" \n #include TEST\n");

    DUContext* top = parse(method, DumpNone);

    DUChainWriteLocker lock(DUChain::lock());

    QCOMPARE(top->importedParentContexts().size(), 1);

    release(top);
  }
}

void TestCppCodeCompletion::testAcrossHeaderReferences()
{
  addInclude( "acrossheader1.h", "class Test{ };" );
  addInclude( "acrossheader2.h", "Test t;" );
  QByteArray method("#include \"acrossheader1.h\"\n#include \"acrossheader2.h\"\n");

  DUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());


  Declaration* decl = findDeclaration(top, Identifier("t"), top->range().end);
  QVERIFY(decl);
  QVERIFY(decl->abstractType());
  AbstractType::Ptr t(decl->abstractType());
  QVERIFY(dynamic_cast<const IdentifiedType*>(t.unsafeData()));

  release(top);
}

void TestCppCodeCompletion::testAcrossHeaderTemplateResolution() {
  addInclude("acrossheaderresolution1.h", "class C {}; namespace std { template<class T> class A {  }; }");
  addInclude("acrossheaderresolution2.h", "namespace std { template<class T> class B { typedef A<T> Type; }; }");
  
  QByteArray method("#include \"acrossheaderresolution1.h\"\n#include \"acrossheaderresolution2.h\"\n std::B<C>::Type t;");
  
  DUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());
  
  Declaration* decl = findDeclaration(top, QualifiedIdentifier("t"), top->range().end);
  QVERIFY(decl);
  QCOMPARE(QualifiedIdentifier(TypeUtils::unAliasedType(decl->abstractType())->toString()), QualifiedIdentifier("std::A<C>"));
  
  release(top);
}

void TestCppCodeCompletion::testAcrossHeaderTemplateReferences()
{
  addInclude( "acrossheader1.h", "class Dummy { }; template<class Q> class Test{ };" );
  addInclude( "acrossheader2.h", "template<class B, class B2 = Test<B> > class Test2 : public Test<B>{ Test<B> bm; };" );
  QByteArray method("#include \"acrossheader1.h\"\n#include \"acrossheader2.h\"\n ");

  DUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());


  {
    kDebug() << "top is" << top;
    Declaration* decl = findDeclaration(top, QualifiedIdentifier("Dummy"), top->range().end);
    QVERIFY(decl);
    QVERIFY(decl->abstractType());
    AbstractType::Ptr t(decl->abstractType());
    QVERIFY(dynamic_cast<const IdentifiedType*>(t.unsafeData()));
    QCOMPARE(decl->abstractType()->toString(), QString("Dummy"));
  }
  {
    Declaration* decl = findDeclaration(top, QualifiedIdentifier("Test2<Dummy>::B2"), top->range().end);
    QVERIFY(decl);
    QVERIFY(decl->abstractType());
    AbstractType::Ptr t(decl->abstractType());
    QVERIFY(dynamic_cast<const IdentifiedType*>(t.unsafeData()));
    QCOMPARE(decl->abstractType()->toString(), QString("Test< Dummy >"));
  }
  {
    Declaration* decl = findDeclaration(top, QualifiedIdentifier("Test2<Dummy>::bm"), top->range().end);
    QVERIFY(decl);
    QVERIFY(decl->abstractType());
    AbstractType::Ptr t(decl->abstractType());
    QVERIFY(dynamic_cast<const IdentifiedType*>(t.unsafeData()));
    QCOMPARE(decl->abstractType()->toString(), QString("Test< Dummy >"));
  }
  {
    ClassDeclaration* decl = dynamic_cast<ClassDeclaration*>(findDeclaration(top, QualifiedIdentifier("Test2<Dummy>"), top->range().end));
    QVERIFY(decl);
    QVERIFY(decl->abstractType());
    CppClassType::Ptr classType = decl->abstractType().cast<CppClassType>();
    QVERIFY(classType);
    QCOMPARE(decl->baseClassesSize(), 1u);
    QVERIFY(decl->baseClasses()[0].baseClass);
    CppClassType::Ptr parentClassType = decl->baseClasses()[0].baseClass.type<CppClassType>();
    QVERIFY(parentClassType);
    QCOMPARE(parentClassType->toString(), QString("Test< Dummy >"));
  }

  release(top);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TestCppCodeCompletion::release(DUContext* top)
{
  //EditorIntegrator::releaseTopRange(top->textRangePtr());
  if(dynamic_cast<TopDUContext*>(top))
    DUChain::self()->removeDocumentChain(static_cast<TopDUContext*>(top));
  //delete top;
}

void TestCppCodeCompletion::addInclude( const QString& identity, const QString& text ) {
  fakeIncludes[identity] = text;
}

//Only for debugging
QString print(const Cpp::ReferenceCountedStringSet& set) {
  QString ret;
  bool first = true;
  Cpp::ReferenceCountedStringSet::Iterator it(set.iterator());
  while(it) {
    if(!first)
      ret += ", ";
    first = false;

    ret += (*it).str();
    ++it;
  }
  return ret;
}

//Only for debugging
QStringList toStringList( const Cpp::ReferenceCountedStringSet& set ) {
  QStringList ret;
  Cpp::ReferenceCountedStringSet::Iterator it(set.iterator());
  while(it) {
    ret << (*it).str();
    ++it;
  }
  ret.sort();
  return ret;
}

QStringList splitSorted(const QString& str) {
  QStringList ret = str.split("\n");
  ret.sort();
  return ret;
}

void TestCppCodeCompletion::testEmptyMacroArguments() {
  QString test("#define merge(prefix, suffix) prefix ## suffix\n void merge(test1, ) () { } void merge(, test2) () { }");
  DUChainWriteLocker l(DUChain::lock());
  TopDUContext* ctx = parse(test.toUtf8());
  QCOMPARE(ctx->localDeclarations().count(), 2);
  QCOMPARE(ctx->localDeclarations()[0]->identifier().toString(), QString("test1"));
  QCOMPARE(ctx->localDeclarations()[1]->identifier().toString(), QString("test2"));
}

void TestCppCodeCompletion::testMacroExpansionRanges() {
{
  QString test("#define TEST(X) int allabamma; \nTEST(C)\n");
  DUChainWriteLocker l(DUChain::lock());
  TopDUContext* ctx = parse(test.toUtf8());
  QCOMPARE(ctx->localDeclarations().count(), 1);
  kDebug() << ctx->localDeclarations()[0]->range().castToSimpleRange().textRange();
  //kDebug() << ctx->localDeclarations()[1]->range().castToSimpleRange().textRange();
  QCOMPARE(ctx->localDeclarations()[0]->range().castToSimpleRange().textRange(), KTextEditor::Range(1, 7, 1, 7)); //Because the macro TEST was expanded out of its physical range, the Declaration is collapsed.
  //  QCOMPARE(ctx->localDeclarations()[1]->range().castToSimpleRange().textRange(), KTextEditor::Range(1, 10, 1, 11));
  //kDebug() << "Range:" << ctx->localDeclarations()[0]->range().castToSimpleRange().textRange();
}
{
  QString test("#define A(X) bbbbbb\nint A(0);\n");
  DUChainWriteLocker l(DUChain::lock());
  TopDUContext* ctx = parse(test.toUtf8());
  QCOMPARE(ctx->localDeclarations().count(), 1);
  kDebug() << ctx->localDeclarations()[0]->range().castToSimpleRange().textRange();
  QCOMPARE(ctx->localDeclarations()[0]->range().castToSimpleRange().textRange(), KTextEditor::Range(1, 8, 1, 8)); //Because the macro TEST was expanded out of its physical range, the Declaration is collapsed.
}
{
  QString test("#define TEST namespace NS{int a;int b;int c;int d;int q;} class A{}; \nTEST; int a; int b; int c; int d;int e;int f;int g;int h;\n");
  DUChainWriteLocker l(DUChain::lock());
  TopDUContext* ctx = parse(test.toUtf8());
  QCOMPARE(ctx->localDeclarations().count(), 10);
  QCOMPARE(ctx->localDeclarations()[1]->range().castToSimpleRange().textRange(), KTextEditor::Range(1, 4, 1, 4)); //Because the macro TEST was expanded out of its physical range, the Declaration is collapsed.
  QCOMPARE(ctx->localDeclarations()[2]->range().castToSimpleRange().textRange(), KTextEditor::Range(1, 10, 1, 11));
}
{
  //The range of the merged declaration name should be trimmed to the end of the macro invocation
  QString test("#define TEST(X) class X ## Class {};\nTEST(Hallo)\n");
  DUChainWriteLocker l(DUChain::lock());
  TopDUContext* ctx = parse(test.toUtf8());
  QCOMPARE(ctx->localDeclarations().count(), 1);
  QCOMPARE(ctx->localDeclarations()[0]->range().castToSimpleRange().textRange(), KTextEditor::Range(1, 5, 1, 11));
}
{
  //The range of the merged declaration name should be within macro invocation even when the order of merging is different from the order of formal parameters
  QString test("#define TEST(X, Y) class Y ## X {};\nTEST(Hello, World)\n");
  DUChainWriteLocker l(DUChain::lock());
  TopDUContext* ctx = parse(test.toUtf8());
  QCOMPARE(ctx->localDeclarations().count(), 1);
  QCOMPARE(ctx->localDeclarations()[0]->range().castToSimpleRange().textRange(), KTextEditor::Range(1, 12, 1, 18));
}
{
  //The range of the merged declaration name should be collapsed if it does not start with a macro parameter
  QString test("#define TEST(X) class Hallo ## X {};\nTEST(Class)\n");
  DUChainWriteLocker l(DUChain::lock());
  TopDUContext* ctx = parse(test.toUtf8());
  QCOMPARE(ctx->localDeclarations().count(), 1);
  QCOMPARE(ctx->localDeclarations()[0]->range().castToSimpleRange().textRange(), KTextEditor::Range(1, 11, 1, 11));
}
}

void TestCppCodeCompletion::testTimeMacro()
{
  QString test("const char* str = __TIME__;");
  DUChainWriteLocker l(DUChain::lock());
  TopDUContext* ctx = parse(test.toUtf8());
  QVERIFY(ctx->problems().isEmpty());
  QCOMPARE(ctx->localDeclarations().count(), 1);
}

void TestCppCodeCompletion::testDateMacro()
{
  QString test("const char* str = __DATE__;");
  DUChainWriteLocker l(DUChain::lock());
  TopDUContext* ctx = parse(test.toUtf8());
  QVERIFY(ctx->problems().isEmpty());
  QCOMPARE(ctx->localDeclarations().count(), 1);
}

void TestCppCodeCompletion::testFileMacro()
{
  QString test("const char* str = __FILE__;");
  DUChainWriteLocker l(DUChain::lock());
  TopDUContext* ctx = parse(test.toUtf8());
  QVERIFY(ctx->problems().isEmpty());
  QCOMPARE(ctx->localDeclarations().count(), 1);
}

void TestCppCodeCompletion::testNaiveMatching() {
  return;
    Cpp::EnvironmentManager::self()->setMatchingLevel(Cpp::EnvironmentManager::Naive);
    {
      addInclude("recursive_test_1.h", "#include \"recursive_test_2.h\"\nint i1;\n");
      addInclude("recursive_test_2.h", "#include \"recursive_test_1.h\"\nint i2;\n");
      
      TopDUContext* test1 = parse(QByteArray("#include \"recursive_test_1.h\"\n"), DumpNone);
      DUChainWriteLocker l(DUChain::lock());
      QCOMPARE(test1->recursiveImportIndices().count(), 3u);
      QCOMPARE(test1->importedParentContexts().count(), 1);
      QCOMPARE(test1->importedParentContexts()[0].indexedContext().context()->importedParentContexts().count(), 1);
      QCOMPARE(test1->importedParentContexts()[0].indexedContext().context()->importedParentContexts()[0].indexedContext().context()->importedParentContexts().count(), 1);
      QCOMPARE(test1->importedParentContexts()[0].indexedContext().context()->importedParentContexts()[0].indexedContext().context()->importedParentContexts()[0].indexedContext().context()->importedParentContexts().count(), 1);
      Cpp::EnvironmentFile* envFile1 = dynamic_cast<Cpp::EnvironmentFile*>(test1->parsingEnvironmentFile().data());
      QVERIFY(envFile1);
      QVERIFY(envFile1->headerGuard().isEmpty());
      release(test1);
    }

}

void TestCppCodeCompletion::testHeaderGuards() {
    {
      TopDUContext* test1 = parse(QByteArray("#ifndef GUARD\n#define GUARD\nint x = 5; \n#endif\n#define BLA\n"), DumpNone);
      DUChainWriteLocker l(DUChain::lock());
      Cpp::EnvironmentFile* envFile1 = dynamic_cast<Cpp::EnvironmentFile*>(test1->parsingEnvironmentFile().data());
      QVERIFY(envFile1);
      QVERIFY(envFile1->headerGuard().isEmpty());
      release(test1);
    }
    {
      TopDUContext* test1 = parse(QByteArray("#ifndef GUARD\n#define GUARD\nint x = 5;\n#ifndef GUARD\n#define GUARD\n#endif\n#if defined(TEST)\n int q = 4;#endif\n#endif\n"), DumpNone);
      DUChainWriteLocker l(DUChain::lock());
      Cpp::EnvironmentFile* envFile1 = dynamic_cast<Cpp::EnvironmentFile*>(test1->parsingEnvironmentFile().data());
      QVERIFY(envFile1);
      QCOMPARE(envFile1->headerGuard().str(), QString("GUARD"));
      release(test1);
    }
    {
      TopDUContext* test1 = parse(QByteArray("int x;\n#ifndef GUARD\n#define GUARD\nint x = 5; \n#endif\n"), DumpNone);
      DUChainWriteLocker l(DUChain::lock());
      Cpp::EnvironmentFile* envFile1 = dynamic_cast<Cpp::EnvironmentFile*>(test1->parsingEnvironmentFile().data());
      QVERIFY(envFile1);
      QVERIFY(envFile1->headerGuard().isEmpty());
      release(test1);
    }
    {
      TopDUContext* test1 = parse(QByteArray("#define X\n#ifndef GUARD\n#define GUARD\nint x = 5; \n#endif\n"), DumpNone);
      DUChainWriteLocker l(DUChain::lock());
      Cpp::EnvironmentFile* envFile1 = dynamic_cast<Cpp::EnvironmentFile*>(test1->parsingEnvironmentFile().data());
      QVERIFY(envFile1);
      QVERIFY(envFile1->headerGuard().isEmpty());
      release(test1);
    }
    {
      TopDUContext* test1 = parse(QByteArray("#ifndef GUARD\n#define GUARD\nint x = 5; \n#endif\nint o;\n"), DumpNone);
      DUChainWriteLocker l(DUChain::lock());
      Cpp::EnvironmentFile* envFile1 = dynamic_cast<Cpp::EnvironmentFile*>(test1->parsingEnvironmentFile().data());
      QVERIFY(envFile1);
      QVERIFY(envFile1->headerGuard().isEmpty());
      release(test1);
    }
}

void TestCppCodeCompletion::testEnvironmentMatching() {
    {
      CppPreprocessEnvironment::setRecordOnlyImportantString(false);
      
      addInclude("deep2.h", "#ifdef WANT_DEEP\nint x;\n#undef WANT_DEEP\n#endif\n");
      addInclude("deep1.h", "#define WANT_DEEP\n#include \"deep2.h\"\n");
      TopDUContext* test1 = parse(QByteArray("#include \"deep1.h\""), DumpNone);
      Cpp::EnvironmentFile* envFile1 = dynamic_cast<Cpp::EnvironmentFile*>(test1->parsingEnvironmentFile().data());
      DUChainWriteLocker lock(DUChain::lock());
      QVERIFY(envFile1);
      QCOMPARE(envFile1->definedMacroNames().set().count(), 0u);
      QCOMPARE(envFile1->definedMacros().set().count(), 0u);
      QCOMPARE(envFile1->usedMacros().set().count(), 0u);
    }

    addInclude("h1.h", "#ifndef H1_H  \n#define H1_H  \n  class H1 {};\n #else \n class H1_Already_Defined {}; \n#endif");
    addInclude("h1_user.h", "#ifndef H1_USER \n#define H1_USER \n#include \"h1.h\" \nclass H1User {}; \n#endif\n");

    {
      TopDUContext* test1 = parse(QByteArray("#include \"h1.h\" \n#include \"h1_user.h\"\n\nclass Honk {};"), DumpNone);
        //We test here, whether the environment-manager re-combines h1_user.h so it actually contains a definition of class H1.
        //In the version parsed in test1, H1_H was already defined, so the h1.h parsed into h1_user.h was parsed to contain H1_Already_Defined.
        TopDUContext* test2 = parse(QByteArray("#include \"h1_user.h\"\n"), DumpNone);
        DUChainWriteLocker lock(DUChain::lock());
        QVERIFY(test1->parsingEnvironmentFile());
        QVERIFY(test2->parsingEnvironmentFile());
        Cpp::EnvironmentFile* envFile1 = dynamic_cast<Cpp::EnvironmentFile*>(test1->parsingEnvironmentFile().data());
        Cpp::EnvironmentFile* envFile2 = dynamic_cast<Cpp::EnvironmentFile*>(test2->parsingEnvironmentFile().data());
        QVERIFY(envFile1);
        QVERIFY(envFile2);

        QCOMPARE(envFile1->usedMacros().set().count(), 0u);
        QCOMPARE(envFile2->usedMacros().set().count(), 0u);
        QVERIFY(findDeclaration( test1, Identifier("H1") ));

      QCOMPARE( envFile1->contentStartLine(), 3 );
    }

    { //Test shadowing of strings through #undefs
      addInclude("stringset_test1.h", "String1 s1;\n#undef String2\n String2 s2;");
      addInclude("stringset_test2.h", "String1 s1;\n#undef String2\n String2 s2;");

      {
        TopDUContext* top = parse(QByteArray("#include \"stringset_test1.h\"\n"), DumpNone);
        DUChainWriteLocker lock(DUChain::lock());
        QVERIFY(top->parsingEnvironmentFile());
        Cpp::EnvironmentFile* envFile = dynamic_cast<Cpp::EnvironmentFile*>(top->parsingEnvironmentFile().data());
        QVERIFY(envFile);
        kDebug() << "url" << envFile->url().str();
        QCOMPARE(envFile->usedMacros().set().count(), 0u);
        QCOMPARE(toStringList(envFile->strings()), splitSorted("String1\ns1\ns2")); //The #undef protects String2, so it cannot be affected from outside
      }
      {
        TopDUContext* top = parse(QByteArray("#define String1\n#include \"stringset_test1.h\"\nString2 String1;"), DumpNone); //Both String1 and String2 are shadowed. String1 by the macro, and String2 by the #undef in stringset_test1.h
        DUChainWriteLocker lock(DUChain::lock());
        QVERIFY(top->parsingEnvironmentFile());
        Cpp::EnvironmentFile* envFile = dynamic_cast<Cpp::EnvironmentFile*>(top->parsingEnvironmentFile().data());
        QVERIFY(envFile);
        //String1 is shadowed by the macro-definition, so it is not a string that can be affected from outside.
        QCOMPARE(toStringList(envFile->strings()), splitSorted("s1\ns2"));
        QCOMPARE(toStringList(envFile->usedMacroNames()), QStringList()); //No macros from outside were used

        QCOMPARE(envFile->definedMacros().set().count(), 1u);
        QCOMPARE(envFile->usedMacros().set().count(), 0u);

        QCOMPARE(top->importedParentContexts().count(), 1);
        TopDUContext* top2 = dynamic_cast<TopDUContext*>(top->importedParentContexts()[0].context(0));
        QVERIFY(top2);
        Cpp::EnvironmentFile* envFile2 = dynamic_cast<Cpp::EnvironmentFile*>(top2->parsingEnvironmentFile().data());
        QVERIFY(envFile2);

        QCOMPARE(envFile2->definedMacros().set().count(), 0u);

        QCOMPARE(toStringList(envFile2->usedMacroNames()), QStringList("String1")); //stringset_test1.h used the Macro String1 from outside
        QCOMPARE(toStringList(envFile2->strings()), splitSorted("String1\ns1\ns2"));
      }
      {
        TopDUContext* top = parse(QByteArray("#define String1\n#undef String1\n#include \"stringset_test1.h\""), DumpNone);
        DUChainWriteLocker lock(DUChain::lock());
        QVERIFY(top->parsingEnvironmentFile());
        Cpp::EnvironmentFile* envFile = dynamic_cast<Cpp::EnvironmentFile*>(top->parsingEnvironmentFile().data());
        QVERIFY(envFile);
        QCOMPARE(envFile->definedMacros().set().count(), 0u);
        QCOMPARE(envFile->usedMacros().set().count(), 0u);
        //String1 is shadowed by the macro-definition, so it is not a string that can be affected from outside.
        kDebug() << toStringList(envFile->strings()) << splitSorted("s1\ns2");
        QCOMPARE(toStringList(envFile->strings()), splitSorted("s1\ns2"));
        QCOMPARE(toStringList(envFile->usedMacroNames()), QStringList()); //No macros from outside were used

        QCOMPARE(top->importedParentContexts().count(), 1);
        TopDUContext* top2 = dynamic_cast<TopDUContext*>(top->importedParentContexts()[0].context(0));
        QVERIFY(top2);
        Cpp::EnvironmentFile* envFile2 = dynamic_cast<Cpp::EnvironmentFile*>(top2->parsingEnvironmentFile().data());
        QVERIFY(envFile2);
        QCOMPARE(envFile2->definedMacros().set().count(), 0u);

        QCOMPARE(toStringList(envFile2->usedMacroNames()), QStringList()); //stringset_test1.h used the Macro String1 from outside. However it is an undef-macro, so it does not appear in usedMacroNames() and usedMacros()
        QCOMPARE(envFile2->usedMacros().set().count(), (unsigned int)0);
        QCOMPARE(toStringList(envFile2->strings()), splitSorted("String1\ns1\ns2"));
      }
      {
        addInclude("usingtest1.h", "#define HONK\nMACRO m\n#undef HONK2\n");

        TopDUContext* top = parse(QByteArray("#define MACRO meh\nint MACRO;\n#include \"usingtest1.h\"\n"), DumpNone);
        DUChainWriteLocker lock(DUChain::lock());
        QVERIFY(top->parsingEnvironmentFile());
        Cpp::EnvironmentFile* envFile = dynamic_cast<Cpp::EnvironmentFile*>(top->parsingEnvironmentFile().data());
        QVERIFY(envFile);
        QCOMPARE(envFile->definedMacros().set().count(), 2u);
        QCOMPARE(envFile->unDefinedMacroNames().set().count(), 1u);
        QCOMPARE(envFile->usedMacros().set().count(), 0u);
        QCOMPARE(envFile->usedMacroNames().set().count(), 0u);

        kDebug() << toStringList(envFile->strings()) ;
        QCOMPARE(envFile->strings().count(), 3u); //meh, m, int

        QCOMPARE(top->importedParentContexts().count(), 1);
        TopDUContext* top2 = dynamic_cast<TopDUContext*>(top->importedParentContexts()[0].context(0));
        QVERIFY(top2);
        Cpp::EnvironmentFile* envFile2 = dynamic_cast<Cpp::EnvironmentFile*>(top2->parsingEnvironmentFile().data());
        QVERIFY(envFile2);
        QCOMPARE(envFile2->definedMacros().set().count(), 1u);
        QCOMPARE(envFile2->unDefinedMacroNames().set().count(), 1u);
        QCOMPARE(envFile2->usedMacros().set().count(), 1u);
        QCOMPARE(envFile2->usedMacroNames().set().count(), 1u);
        kDebug() << toStringList(envFile2->strings()) ;
        QCOMPARE(envFile2->strings().count(), 3u); //meh(from macro), MACRO, m
      }
    }

/*    addInclude( "envmatch_header1.h", "#include \"envmatch_header2.h\"\n class SomeName; #define SomeName SomeAlternativeName" );
    addInclude( "envmatch_header2.h", "#ifndef SOMEDEF\n #define SOMEDEF\n#endif\n" );
    QByteArray method1("#include \"envmatch_header1.h\"");
    QByteArray method2("#include \"envmatch_header1.h\"");
    QByteArray method3("#include \"envmatch_header1.h\"\n#include \"envmatch_header1.h\"");

    DUContext* top1 = parse(method1, DumpNone);
    DUContext* top2 = parse(method1, DumpNone);
    DUContext* top3 = parse(method1, DumpNone);

    DUChainWriteLocker lock(DUChain::lock());

    QCOMPARE(top1->importedParentContexts().count(), 1);
    QCOMPARE(top2->importedParentContexts().count(), 1);
//     QCOMPARE(top3->importedParentContexts().count(), 2);

    QCOMPARE(top1->importedParentContexts()[0], top2->importedParentContexts()[1]);*/
}

void TestCppCodeCompletion::testPreprocessor() {
  TEST_FILE_PARSE_ONLY
  
  IncludeFileList includes;

  {
    QString a = "#define Q(c) c; char* q = #c; \n Q(int i;\n char* c = \"a\";)\n";
    QString preprocessed = preprocess(IndexedString(), a, includes);
    kDebug() << "preprocessed:" << preprocessed;
    QVERIFY(preprocessed.contains("\"int i;\\n char* c = \\\"a\\\";")); //The newline must have been escaped correctly, and the string as well
    TopDUContext* top = parse(a.toLocal8Bit(), DumpNone);
    DUChainWriteLocker lock(DUChain::lock());
    kDebug() << top->localDeclarations()[0]->identifier().toString();
    QCOMPARE(top->localDeclarations().count(), 3);
    QCOMPARE(top->localDeclarations()[0]->range().start.line, 1);
    QCOMPARE(top->localDeclarations()[1]->range().start.line, 2);
    QCOMPARE(top->localDeclarations()[2]->range().start.line, 2);
  }
  {
    QString a = "#undef __attribute__\n__attribute__((visibility(\"default\")))";
    QString preprocessed = preprocess(IndexedString(), a, includes);
    kDebug() << "preprocessed:" << preprocessed;
    QVERIFY(!preprocessed.contains ("__attribute__"));
  }
  {
    QString a = "#ifdef __attribute__\npassed\n#else\nfailed\n#endif";
    QString preprocessed = preprocess(IndexedString(), a, includes);
    kDebug() << "preprocessed: " << preprocessed;
    QVERIFY(!preprocessed.contains("failed"));
    QVERIFY(preprocessed.contains("passed"));
  }
  {
    QString a = "#define Q(c) c ## ULL \n void test() {int i = Q(0x5);}";
    QString preprocessed = preprocess(IndexedString(), a, includes);
    kDebug() << "preprocessed:" << preprocessed;
    TopDUContext* top = parse(a.toLocal8Bit(), DumpNone);
    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(top->childContexts().count(), 2);
    QCOMPARE(top->childContexts()[1]->localDeclarations().count(), 1);
  }
  {
    QString a = "#define MA(x) T<x> a\n #define MB(x) T<x>\n #define MC(X) int\n #define MD(X) c\n template <typename P1> struct A {}; template <typename P2> struct T {}; int main(int argc, char ** argv) { MA(A<int>); A<MB(int)> b; MC(a)MD(b); MC(a)d; }";
    QString preprocessed = preprocess(IndexedString(), a, includes);
    kDebug() << "preprocessed:" << preprocessed;
    TopDUContext* top = parse(a.toUtf8(), DumpAll);
    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(top->childContexts().count(), 6);
    QCOMPARE(top->childContexts()[5]->localDeclarations().count(), 4);
  }
    #ifdef TEST_MACRO_EXPANSION_ORDER
    //Not working yet
  {//No macro-expansion should happen on the first layer of a macro-call
  QString preprocessed = preprocess(IndexedString(), "#define VAL_KIND A \n#define DO_CAT_I(a, b) a ## b \n#define DO_CAT(a, b) DO_CAT_I(a, b) \nint DO_CAT(Value_, VAL_KIND); \nint DO_CAT_I(Value_, VAL_KIND);\n int VAL_KIND;\nint DO_CAT(VAL_KIND, _Value);\nint DO_CAT(VAL_KIND, _Value);\n", includes);
  kDebug() << preprocessed;
    TopDUContext* top = parse(preprocessed.toUtf8(), DumpNone);
    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(top->localDeclarations().count(), 5);
    QCOMPARE(top->localDeclarations()[0]->identifier(), Identifier("Value_A"));
    QCOMPARE(top->localDeclarations()[1]->identifier(), Identifier("Value_VAL_KIND"));
    QCOMPARE(top->localDeclarations()[1]->identifier(), Identifier("A"));
    QCOMPARE(top->localDeclarations()[0]->identifier(), Identifier("A_Value"));
    QCOMPARE(top->localDeclarations()[0]->identifier(), Identifier("VAL_KIND_Value"));
  }
  #endif
  {//Test macro redirection
    QString test = preprocess(IndexedString(), "#define M1(X) X ## _m1 \n#define M2(X) M ## X\n#define M3 M2\n#define M4 M3 \nM4(1)(hallo)", includes);
    kDebug() << test;
    QCOMPARE(test.trimmed(), QString("hallo_m1"));
  }
  {//Test replacement of merged preprocessor function calls
    TopDUContext* top = parse(QByteArray("#define MACRO_1(X) X ## _fromMacro1 \n#define A(pred, n) MACRO_ ## n(pred) \n#define D(X,Y) A(X ## Y, 1) \nint D(a,ba);"), DumpNone);
    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(top->localDeclarations().count(), 1);
    QCOMPARE(top->localDeclarations()[0]->identifier(), Identifier("aba_fromMacro1"));
  }
  {//Test merging
    TopDUContext* top = parse(QByteArray("#define D(X,Y) X ## Y \nint D(a,ba);"), DumpNone);
    IncludeFileList includes;
    kDebug() << preprocess(IndexedString("somefile"), "#define D(X,Y) X ## Y \nint D(a,ba);", includes);
    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(top->localDeclarations().count(), 1);
    QCOMPARE(top->localDeclarations()[0]->identifier(), Identifier("aba"));
  }
  {
    TopDUContext* top = parse(QByteArray("#define MERGE(a, b) a ## b \n#define MERGE_WITH_PARENS(par) MERGE ## par \nint MERGE_WITH_PARENS((int, B));"), DumpNone);
    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(top->localDeclarations().count(), 1);
    QCOMPARE(top->localDeclarations()[0]->identifier(), Identifier("intB"));
  }
  {//Test simple #if
    TopDUContext* top = parse(QByteArray("#define X\n#if defined(X)\nint xDefined;\n#endif\n#if !defined(X)\nint xNotDefined;\n#endif\n#if (!defined(X))\nint xNotDefined2;\n#endif"), DumpNone);
    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(top->localDeclarations().count(), 1);
    QCOMPARE(top->localDeclarations()[0]->identifier(), Identifier("xDefined"));
  }
  {//Test simple #if
    TopDUContext* top = parse(QByteArray("#if defined(X)\nint xDefined;\n#endif\n#if !defined(X)\nint xNotDefined;\n#endif\n#if (!defined(X))\nint xNotDefined2;\n#endif"), DumpNone);
    DUChainWriteLocker lock(DUChain::lock());
    QVERIFY(top->localDeclarations().count() >= 1);
    QCOMPARE(top->localDeclarations()[0]->identifier(), Identifier("xNotDefined"));
    QCOMPARE(top->localDeclarations().count(), 2);
    QCOMPARE(top->localDeclarations()[1]->identifier(), Identifier("xNotDefined2"));
  }
  {//Test multi-line definitions
    TopDUContext* top = parse(QByteArray("#define X \\\nint i;\\\nint o;\nX;\n"), DumpNone);
    IncludeFileList includes;
    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(top->localDeclarations().count(), 2);
  }
  {//Test multi-line definitions
    TopDUContext* top = parse(QByteArray("#define THROUGH_DEFINE(X) X\nclass B {\nclass C{\n};\nC* cPcPcPcPcPcPcPcPcP;\n};\nB* bP;\nvoid test() {\nTHROUGH_DEFINE(bP->cPcPcPcPcPcPcPcPcP);\n}\n"), DumpNone);
    IncludeFileList includes;
    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(top->childContexts().count(), 3);
    QCOMPARE(top->childContexts()[0]->localDeclarations().count(), 2);
    QCOMPARE(top->childContexts()[0]->localDeclarations()[1]->uses().size(), 1);
    QCOMPARE(top->childContexts()[0]->localDeclarations()[1]->uses().begin()->count(), 1);
    QCOMPARE(top->childContexts()[0]->localDeclarations()[1]->uses().begin()->at(0).start.column, 19);
    QCOMPARE(top->childContexts()[0]->localDeclarations()[1]->uses().begin()->at(0).end.column, 37);
  }
  {//Test merging
    TopDUContext* top = parse(QByteArray("#define D(X,Y) X ## Y \nint D(a,ba);"), DumpNone);
    IncludeFileList includes;
    kDebug() << preprocess(IndexedString("somefile"), "#define D(X,Y) X ## Y \nint D(a,ba);", includes);
    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(top->localDeclarations().count(), 1);
    QCOMPARE(top->localDeclarations()[0]->identifier(), Identifier("aba"));
  }
  {//Test merging
    TopDUContext* top = parse(QByteArray("#define A(x) int x;\n#define B(name) A(bo ## name)\nB(Hallo)"), DumpNone);
    IncludeFileList includes;
    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(top->localDeclarations().count(), 1);
    QCOMPARE(top->localDeclarations()[0]->identifier(), Identifier("boHallo"));
  }
  {//Test __builtin_offsetof
    TopDUContext* top = parse(QByteArray("typedef struct a { int i; } t_a; int o = __builtin_offsetof(t_a, i);"), DumpNone);
    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(top->localDeclarations().count(), 3);
  }
  {//Test __builtin_offsetof with a struct type (make sure spaces are handled correctly by the macro)
    TopDUContext* top = parse(QByteArray("struct a { int i; }; int o = __builtin_offsetof(struct a, i);"), DumpNone);
    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(top->localDeclarations().count(), 2);
  }
  {
    QString parsed = preprocess(IndexedString("somefile"),
      "#define NC(...) __VA_ARGS__\nNC(bla,bla)\n", includes)
      .replace(QRegExp("[\n\t ]+"), "");
    QEXPECT_FAIL("", "Variadic macros unsupported", Continue);
    QCOMPARE(parsed, QString("bla,bla"));
    // fallback: at least don't fail parsing altogether, see https://bugs.kde.org/show_bug.cgi?id=308556
    QCOMPARE(parsed, QString());

    parsed = preprocess(IndexedString("somefile"),
      "#define PUT_BETWEEN(x,y) x y x\n#define NC(...) __VA_ARGS__\nPUT_BETWEEN(NC(pair<a,b>), c)\n", includes)
      .replace(QRegExp("[\n\t ]+"), " ").trimmed();
    QEXPECT_FAIL("", "Variadic macros unsupported", Continue);
    QCOMPARE(parsed, QString("pair<a,b> c pair<a,b>"));

  }
}

void TestCppCodeCompletion::testArgumentList()
{
  QMap<QByteArray, QString> codeToArgList;
  codeToArgList.insert("void foo(int arg[]){}", "(int arg[])");
  codeToArgList.insert("void foo(int arg[1]){}", "(int arg[1])");
  codeToArgList.insert("void foo(int arg[][1]){}", "(int arg[][1])");
  codeToArgList.insert("void foo(int arg[1][1]){}", "(int arg[1][1])");
  codeToArgList.insert("void foo(int arg[][1][1]){}", "(int arg[][1][1])");
  codeToArgList.insert("void foo(void){}", "(void)");
  codeToArgList.insert("void foo(int){}", "(int)");
  QMap< QByteArray, QString >::const_iterator it = codeToArgList.constBegin();
  while (it != codeToArgList.constEnd()){
    qDebug() << "input function is:" << it.key();
    TopDUContext* top = parse(it.key(), DumpNone);
    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(top->localDeclarations().size(), 1);

    CompletionItemTester complCtx(top, "");
    Cpp::NormalDeclarationCompletionItem item(DeclarationPointer(top->localDeclarations().first()), KSharedPtr<KDevelop::CodeCompletionContext>(complCtx.completionContext.data()));
    QString ret;
    Cpp::createArgumentList(item, ret, 0);
    QCOMPARE(ret, it.value());

    release(top);

    ++it;
  }
}

void TestCppCodeCompletion::testStaticMethods()
{
  QByteArray code("struct A {\n"
                  "  public: static void myPublicStatic() {}\n"
                  "  protected: static void myProtectedStatic() {}\n"
                  "  private: static void myPrivateStatic() {}\n"
                  "  public: void myPublicNonStatic() {}\n"
                  "  protected: void myProtectedNonStatic() {}\n"
                  "  private: void myPrivateNonStatic() {}\n"
                  "};\n"
                  "A a; int main() { return 0;}");
  TopDUContext* top = parse(code, DumpNone);
  DUChainWriteLocker lock(DUChain::lock());
  QVERIFY(top->problems().isEmpty());
  QCOMPARE(top->childContexts().size(), 3);
  {
    // in body of main
    CompletionItemTester complCtx(top->childContexts().last(), "a.");
    QVERIFY(complCtx.completionContext->isValid());
    QCOMPARE(complCtx.names, QStringList() << "myPublicStatic" << "myPublicNonStatic");
  }
  {
    // in body of main
    CompletionItemTester complCtx(top->childContexts().last(), "A::");
    QVERIFY(complCtx.completionContext->isValid());
    QEXPECT_FAIL("", "non-static functions don't get filtered. comment in context.cpp: ///@todo what NOT to show on static member choose? Actually we cannot hide all non-static functions, because of function-pointers", Continue);
    QCOMPARE(complCtx.names, QStringList() << "myPublicStatic"); // fails and gets skipped
    // this is a fallback to verify the current behavior
    QCOMPARE(complCtx.names, QStringList() << "myPublicStatic" << "myPublicNonStatic");
  }
  {
    // in body of myPrivate
    CompletionItemTester complCtx(top->childContexts().first()->childContexts().last(), "this->");
    QVERIFY(complCtx.completionContext->isValid());
    QCOMPARE(complCtx.names, QStringList() << "myPublicStatic" << "myProtectedStatic" << "myPrivateStatic" << "myPublicNonStatic" << "myProtectedNonStatic" << "myPrivateNonStatic");
  }
  {
    // in body of myPrivate
    CompletionItemTester complCtx(top->childContexts().first()->childContexts().last(), "A::");
    QVERIFY(complCtx.completionContext->isValid());
    QEXPECT_FAIL("", "non-static functions don't get filtered. comment in context.cpp: ///@todo what NOT to show on static member choose? Actually we cannot hide all non-static functions, because of function-pointers", Continue);
    QCOMPARE(complCtx.names, QStringList() << "myPublicStatic" << "myProtectedStatic" << "myPrivateStatic"); // fails and gets skipped
    // this is a fallback to verify the current behavior
    QCOMPARE(complCtx.names, QStringList() << "myPublicStatic" << "myProtectedStatic" << "myPrivateStatic" << "myPublicNonStatic" << "myProtectedNonStatic" << "myPrivateNonStatic");
  }
  release(top);
}

void TestCppCodeCompletion::testStringInComment_data()
{
  QTest::addColumn<QString>("prefix");

  QTest::newRow("cpp comment") << QString("/* \" */\n");
  QTest::newRow("c comment") << QString("// \"\n");
}

void TestCppCodeCompletion::testStringInComment()
{
  QByteArray code("");
  TopDUContext* top = parse(code, DumpNone);
  DUChainWriteLocker lock(DUChain::lock());
  QVERIFY(top->problems().isEmpty());

  QFETCH(QString, prefix);

  CompletionItemTester complCtx(top, prefix);
  QVERIFY(complCtx.completionContext->isValid());
}

void TestCppCodeCompletion::testProperties()
{
  QByteArray code(
    "class A{\n"
    "  __qt_property__(int myProp READ myProp WRITE setMyProp);\n"
    "public:\n"
    "  int myProp() const;\n" // actual getter
    "  void setMyProp(int);\n" // actual setter
    "};\n"
    " A aStack; A* aHeap = new A; int main() {}");
  TopDUContext* top = parse(code, DumpNone);
  DUChainWriteLocker lock;
  QVERIFY(top->problems().isEmpty());
  QVector<Declaration*> declarations = top->childContexts().first()->localDeclarations();
  QCOMPARE(declarations.count(), 2);
  Declaration* getter = declarations.first();
  QVERIFY(getter->isFunctionDeclaration());
  Declaration* setter = declarations.last();
  QVERIFY(getter->isFunctionDeclaration());

  DUContext* ctx = top->childContexts().last();
  QCOMPARE(ctx->type(), DUContext::Other);

  {
  CompletionItemTester complCtx(ctx, "aStack.");
  QVERIFY(complCtx.completionContext->isValid());
  QCOMPARE(complCtx.completionContext->accessType(), Cpp::CodeCompletionContext::MemberAccess);
  QCOMPARE(complCtx.completionContext->onlyShow(), Cpp::CodeCompletionContext::ShowAll);
  QVERIFY(complCtx.containsDeclaration(getter));
  QVERIFY(complCtx.containsDeclaration(setter));
  }
  {
  CompletionItemTester complCtx(ctx, "aHeap->");
  QVERIFY(complCtx.completionContext->isValid());
  QCOMPARE(complCtx.completionContext->accessType(), Cpp::CodeCompletionContext::ArrowMemberAccess);
  QCOMPARE(complCtx.completionContext->onlyShow(), Cpp::CodeCompletionContext::ShowAll);
  QVERIFY(complCtx.containsDeclaration(getter));
  QVERIFY(complCtx.containsDeclaration(setter));
  }

  release(top);
}

void TestCppCodeCompletion::testAnonStruct()
{
  QByteArray code("void foo() { struct { int a; } myStruct; }");
  TopDUContext* top = parse(code, DumpNone);
  DUChainWriteLocker lock;
  QVERIFY(top->problems().isEmpty());

  CompletionItemTester complCtx(top->childContexts().last(), "");
  QVERIFY(!complCtx.names.contains("<unknown>"));

  release(top);
}

void TestCppCodeCompletion::testOverrideCtor()
{
  QByteArray code("class A { A(int) {} }; class B : public A { B(float) {} }; class C : public B { C(float) {} };");
  TopDUContext* top = parse(code, DumpNone);
  DUChainWriteLocker lock;
  QVERIFY(top->problems().isEmpty());

  Declaration* aCtor = top->childContexts().at(0)->localDeclarations().first();
  Declaration* bCtor = top->childContexts().at(1)->localDeclarations().first();

  {
    // in B we should see A's ctor
    CompletionItemTester complCtx(top->childContexts().at(1), "");
    QVERIFY(complCtx.containsDeclaration(aCtor));
    QVERIFY(!complCtx.containsDeclaration(bCtor));
  }
  {
    // in C we should _only_ see B's ctor. Since it's already overridden, nothing should be shown.
    CompletionItemTester complCtx(top->childContexts().at(2), "");
    QVERIFY(!complCtx.containsDeclaration(aCtor));
    QVERIFY(!complCtx.containsDeclaration(bCtor));
  }

  release(top);
}

void TestCppCodeCompletion::testFilterVoid()
{
  QByteArray code("struct S{}; void foo() {}");
  TopDUContext* top = parse(code, DumpNone);
  DUChainWriteLocker lock;
  QVERIFY(top->problems().isEmpty());

  // filter void here
  foreach(const QString& s, QStringList() << "int i = " << "S* sPtr = ") {
    CompletionItemTester complCtx(top->childContexts().last(), s);
    QVERIFY(!complCtx.containsDeclaration(top->localDeclarations().last()));
  }

  // but not here
  CompletionItemTester complCtx(top->childContexts().last(), "void (*fPtr)() =");
  QVERIFY(complCtx.containsDeclaration(top->localDeclarations().last()));

  release(top);
}

void TestCppCodeCompletion::testExecuteKeepWord_data()
{
  QTest::addColumn<QString>("code");
  QTest::addColumn<QString>("expectedCode");

  QTest::newRow("replaceWord") << "\nstruct Foo { };\nFoo f;\nfbar();"
                               << "f();";

  QTest::newRow("structfunction") << "\nstruct Foo { int bar() { return 0; } };\nFoo f;\nfbar();"
                                  << "f.bar();";

  QTest::newRow("pointer") << "\nstruct Foo { int bar() { return 0; } };\nFoo *f;\nfbar();"
                           << "f->bar();";

  QTest::newRow("reference") << "\nstruct Foo { int bar() { return 0; } };\nFoo &f;\nfbar();"
                             << "f.bar();";

  QTest::newRow("smartPtr") << "template<class T> struct SmartPointer { T* operator->() {return 0;}};\nstruct Foo { int bar() { return 0; } };\nSmartPointer<Foo> f;\nfbar();"
                            << "f->bar();";

  QTest::newRow("enum") << "\nclass f { enum Foo { xy, xz }; };\n\nfFoo x;"
                            << "f::Foo x;";

  QTest::newRow("staticfunction") << "struct f { static void bar() {} };\n\n\nfbar();"
                                  << "f::bar();";
}

void TestCppCodeCompletion::testExecuteKeepWord()
{
  QFETCH(QString, code);
  TopDUContext* top = parse(code.toAscii(), DumpAll);

  KTextEditor::Editor* editor = KTextEditor::Editor::instance();
  QVERIFY(editor);

  KTextEditor::Document* doc = editor->createDocument(this);
  QVERIFY(doc);
  doc->setText(code);
  doc->startEditing();

  KTextEditor::View *v = doc->createView(0);
  v->setCursorPosition(KTextEditor::Cursor(3, 1));

  DUChainWriteLocker lock;
  QVERIFY(top->problems().isEmpty());

  CompletionItemTester complCtx(top->childContexts().last(), "");
  KSharedPtr<CompletionTreeItem> item;
  for(int i=0; i<complCtx.items.length(); ++i) {
    kDebug() << complCtx.itemData(i).toString();
    if (complCtx.itemData(i).toString()=="f") {
      item = complCtx.items.at(i);
    }
  }
  QVERIFY(!item.isNull());
  item->execute(doc, KTextEditor::Range(3, 0, 3, 4));

  QFETCH(QString, expectedCode);
  QCOMPARE(doc->line(3), expectedCode);

  doc->endEditing();

  release(top);

  delete v;
  delete doc;
}


//BEGIN: Helper

class TestPreprocessor : public rpp::Preprocessor
{
public:

  TestCppCodeCompletion* cc;
  IncludeFileList& included;
  rpp::pp* pp;
  bool stopAfterHeaders;
  Cpp::EnvironmentFilePointer environmentFile;

  TestPreprocessor( TestCppCodeCompletion* _cc, IncludeFileList& _included, bool _stopAfterHeaders ) : cc(_cc), included(_included), pp(0), stopAfterHeaders(_stopAfterHeaders) {
  }

  rpp::Stream* sourceNeeded(QString& fileName, rpp::Preprocessor::IncludeType /*type*/, int sourceLine, bool /*skipCurrentPath*/)
  {
    QMap<QString,QString>::const_iterator it = cc->fakeIncludes.constFind(fileName);
    if( it != cc->fakeIncludes.constEnd() || !pp ) {
      kDebug(9007) << "parsing included file \"" << fileName << "\"";
      included << LineContextPair( dynamic_cast<TopDUContext*>(cc->parse( (*it).toUtf8(), TestCppCodeCompletion::DumpNone, pp, KUrl(it.key()))), sourceLine );
    } else {
      kDebug(9007) << "could not find include-file \"" << fileName << "\"";
    }
    return 0;
  }

  void setPp( rpp::pp* _pp ) {
    pp = _pp;
  }

  virtual void headerSectionEnded(rpp::Stream& stream) {
    if( environmentFile )
      environmentFile->setContentStartLine( stream.originalInputPosition().line );
    if(stopAfterHeaders)
      stream.toEnd();
  }
  
  virtual void foundHeaderGuard(rpp::Stream& /*stream*/, KDevelop::IndexedString guardName) {
    environmentFile->setHeaderGuard(guardName);
  }
};

QString TestCppCodeCompletion::preprocess( const IndexedString& url, const QString& text, IncludeFileList& included, rpp::pp* parent, bool stopAfterHeaders, KSharedPtr<Cpp::EnvironmentFile>* paramEnvironmentFile, rpp::LocationTable** returnLocationTable, PreprocessedContents* targetContents ) {
  TestPreprocessor ppc( this, included, stopAfterHeaders );


    rpp::pp preprocessor(&ppc);
    ppc.setPp( &preprocessor );

    KSharedPtr<Cpp::EnvironmentFile> environmentFile;
    if( paramEnvironmentFile && *paramEnvironmentFile )
      environmentFile = *paramEnvironmentFile;
    else
      environmentFile = Cpp::EnvironmentFilePointer( new Cpp::EnvironmentFile( IndexedString(url.str()), 0 ) );

  ppc.environmentFile = environmentFile;

    if( paramEnvironmentFile )
      *paramEnvironmentFile = environmentFile;

    CppPreprocessEnvironment* currentEnvironment = new CppPreprocessEnvironment( environmentFile );
    preprocessor.setEnvironment( currentEnvironment );
    currentEnvironment->setEnvironmentFile( environmentFile );

    if( parent )
      preprocessor.environment()->swapMacros(parent->environment());
    else
      currentEnvironment->merge(CppUtils::standardMacros());

    PreprocessedContents contents = preprocessor.processFile(url.str(), text.toUtf8());
    if(targetContents)
      *targetContents = contents;

    QString result = QString::fromUtf8(stringFromContents(contents));

    if (returnLocationTable)
      *returnLocationTable = preprocessor.environment()->takeLocationTable();

    currentEnvironment->finishEnvironment();

    if( parent ) {
      preprocessor.environment()->swapMacros(parent->environment());
      static_cast<CppPreprocessEnvironment*>(parent->environment())->environmentFile()->merge(*environmentFile);
    }

    return result;
}

TopDUContext* TestCppCodeCompletion::parse(const QByteArray& unit, DumpAreas dump, rpp::pp* parent, KUrl _identity, TopDUContext* update)
{
  if (dump)
    kDebug(9007) << "==== Beginning new test case...:" << endl << unit;

  ParseSession* session = new ParseSession();
   ;

  static int testNumber = 0;
  IndexedString url(QString("file:///internal/%1").arg(testNumber++));
  if( !_identity.isEmpty() )
      url = IndexedString(_identity);

   IncludeFileList included;
   QList<DUContext*> temporaryIncluded;

  rpp::LocationTable* locationTable;

  Cpp::EnvironmentFilePointer file;

  PreprocessedContents contents;

  preprocess( url, QString::fromUtf8(unit), included, parent, false, &file, &locationTable, &contents );

  session->setContents( contents, locationTable );

    if( parent ) {
      //Temporarily insert all files parsed previously by the parent, so forward-declarations can be resolved etc.
      TestPreprocessor* testPreproc = dynamic_cast<TestPreprocessor*>(parent->preprocessor());
      if( testPreproc ) {
        foreach( LineContextPair include, testPreproc->included ) {
          if( !containsContext( included, include.context ) ) {
            included.push_front( include );
            temporaryIncluded << include.context;
          }
        }
      } else {
        kDebug(9007) << "PROBLEM";
      }
    }

  Parser parser(&control);
  TranslationUnitAST* ast = parser.parse(session);
  ast->session = session;

  if (dump & DumpAST) {
    kDebug(9007) << "===== AST:";
    cppDumper.dump(ast, session);
  }

  DeclarationBuilder definitionBuilder(session);

  TopDUContext* top = definitionBuilder.buildDeclarations(file, ast, &included, ReferencedTopDUContext(update));

  UseBuilder useBuilder(session);
  useBuilder.buildUses(ast);
  foreach(KDevelop::ProblemPointer problem, useBuilder.problems()) {
    DUChainWriteLocker lock(DUChain::lock());
    top->addProblem(problem);
  }

  if (dump & DumpDUChain) {
    kDebug(9007) << "===== DUChain:";

    DUChainWriteLocker lock(DUChain::lock());
    KDevelop::dumpDUContext(top);
  }

  if( parent ) {
    //Remove temporarily inserted files parsed previously by the parent
    DUChainWriteLocker lock(DUChain::lock());
    TestPreprocessor* testPreproc = dynamic_cast<TestPreprocessor*>(parent->preprocessor());
    if( testPreproc ) {
      foreach( DUContext* context, temporaryIncluded )
        top->removeImportedParentContext( context );
    } else {
      kDebug(9007) << "PROBLEM";
    }
  }


  if (dump)
    kDebug(9007) << "===== Finished test case.";

  delete session;

  return top;
}

void TestCppCodeCompletion::testCompletedIncludeFilePath()
{
  KTempDir tempDir;
  QDir dir(tempDir.name());
  QString innerDirName = "directoryabcde";
  dir.mkdir(innerDirName);
  QDir innerDir(tempDir.name() + innerDirName);
  QString filename = "xxxxx.h";
  QFile file(innerDir.absoluteFilePath(filename));
  QVERIFY(file.open(QIODevice::ReadWrite));
  QList<IncludeItem> includeItems = CppUtils::allFilesInIncludePath(QString(tempDir.name() + "source.cpp"), true, innerDirName, QStringList() << tempDir.name());
  QCOMPARE(includeItems.size(), 1);
  QCOMPARE(includeItems[0].basePath, KUrl(innerDir.absolutePath()));
}

/**
 * Check that there are no multiple include completion items referring to the same file
 */
void TestCppCodeCompletion::testMultipleIncludeCompletionItems()
{
  KTempDir tempDir;
  QDir dir(tempDir.name());
  QString innerDirName1 = "directory1abcde";
  dir.mkdir(innerDirName1);
  QString innerDirName2 = "directory2abcde";
  dir.mkdir(innerDirName2);
  QDir innerDir1(tempDir.name() + innerDirName1);
  QString filename = "xxxxx.h";
  QFile file(innerDir1.absoluteFilePath(filename));
  QVERIFY(file.open(QIODevice::ReadWrite));
  QList<IncludeItem> includeItems = CppUtils::allFilesInIncludePath(innerDir1.absoluteFilePath("source.cpp"), true, QString("../" + innerDirName1), QStringList() << QString(tempDir.name() + innerDirName1) << QString(tempDir.name() + innerDirName2));
  QCOMPARE(includeItems.size(), 1);
  QCOMPARE(includeItems[0].basePath, KUrl(innerDir1.absolutePath()));
}

void TestCppCodeCompletion::testAfterVisibility_data()
{
  QTest:: addColumn<QString>("vis");
  QTest::newRow("public") << "public:";
  QTest::newRow("protected") << "public:";
  QTest::newRow("private") << "public:";
  // happens e.g. after Q_OBJECT
  QTest::newRow("private-private") << "private:private:";
  QTest::newRow("private-public") << "private:public:";
  QTest::newRow("private-protected") << "private:protected:";
}

void TestCppCodeCompletion::testAfterVisibility()
{
  QByteArray code("struct b { virtual void foo(); }; struct c : public b {};");
  TopDUContext* top = parse(code, DumpNone);
  DUChainWriteLocker lock;
  QVERIFY(top->problems().isEmpty());

  QFETCH(QString, vis);
  CompletionItemTester complCtx(top->childContexts().last(), vis);
  QVERIFY(complCtx.completionContext->isValid());
  QVERIFY(complCtx.containsDeclaration(top->findDeclarations(QualifiedIdentifier("b::foo")).first()));

  release(top);
}

void TestCppCodeCompletion::testNoQuadrupleColon()
{
  QByteArray code("namespace Foobar { static int var; }\n main() { Foobar::var; }");
  TopDUContext* top = parse(code, DumpNone);
  DUChainWriteLocker lock;
  QVERIFY(top->problems().isEmpty());
  
  CompletionItemTester tester(top->childContexts().last());
  QVERIFY(tester.completionContext->isValid());
  KSharedPtr<CompletionTreeItem> item;
  for( int i = 0; i < tester.items.length(); ++i ) {
      if( tester.itemData( i ).toString() == "Foobar" ) {
          item = tester.items.at( i );
      }
  }
  QVERIFY( !item.isNull() );
  
  KTextEditor::Editor* editor = KTextEditor::Editor::instance();
  QVERIFY(editor);
  KTextEditor::Document* doc = editor->createDocument(this);
  QVERIFY(doc);

  // verify it adds the "::" when the doc is empty
  doc->setText("");
  KTextEditor::View* v = doc->createView(0);
  
  doc->startEditing();
  KTextEditor::Cursor c( 0, 0 );
  v->setCursorPosition( c );

  item->execute( doc, Range( c, 0 ) );
  QCOMPARE( doc->line( 0 ), QString("Foobar::") );

  // verify it doesn't when there's already a "::"
  doc->setText("::var;");
  v->setCursorPosition( c );

  item->execute( doc, Range( c, 0 ) );
  QCOMPARE( doc->line( 0 ), QString("Foobar::var;") );

  doc->endEditing();
}

void TestCppCodeCompletion::testLookaheadMatches_data()
{
  QTest::addColumn<QString>("insert");          // inserted code
  QTest::addColumn<QStringList>("completions"); // completions offered
  QStringList all;
  all << "m_one" << "m_two" << "m_smartOne" << "m_access" << "ThreeTwoOne" << "One"
      << "Two" << "OneSmartPointer" << "Access" << "OneTwoThree" << "this";

  QTest::newRow("Function Arg") << "m_smartOne.setOne("
    << (QStringList() << "m_two.hatPointer" << "m_smartOne.operator->" << "setOne" << all);
  QTest::newRow("Smart Pointer") << "int foo = "
    << (QStringList() << "int =" << "m_one.alsoRan" << "m_two.meToo" << "m_smartOne->alsoRan" << "m_access.publicMember" << all);
  QTest::newRow("Type Conversions") << "m_smartOne = "
    << (QStringList() << "OneSmartPointer m_smartOne =" << "m_two.hatPointer" << all );
  QTest::newRow("Assignment") << "m_one =  "
    << (QStringList() << "One m_one =" << "m_two.hat" << all );
  QTest::newRow("Equality") << "m_one == "
    << (QStringList() << "m_two.hat" << all );
  QTest::newRow("ReturnAccess") << "return"
    << (QStringList() << "return int" << "m_one.alsoRan" << "m_two.meToo" << "m_smartOne->alsoRan" << "m_access.publicMember" << all );
  QTest::newRow("No Lookahead") << "One::NoLookahead test = "
    << (QStringList() << "One::NoLookahead NoLookahead =" << "NO" << "CAN" << "SEE" << all );
}

void TestCppCodeCompletion::testLookaheadMatches()
{
  QByteArray test = "struct One { enum NoLookahead { NO, CAN, SEE, }; int alsoRan; typedef int myInt; };"
                    "struct Two { One hat; One *hatPointer; int meToo(); };"
                    "struct OneSmartPointer { OneSmartPointer(One*) {}; void setOne(One*); One* operator->() const {} };"
                    "class Access{ int privateMember; public: int publicMember; };"
                    "struct OneTwoThree { One m_one; Two m_two; OneSmartPointer m_smartOne; Access m_access; int ThreeTwoOne() { } };";
  QFETCH(QString, insert);
  QFETCH(QStringList, completions);
  TopDUContext* top = parse(test, DumpNone);
  DUChainWriteLocker lock(DUChain::lock());
  DUContext *testContext = top->childContexts()[4]->childContexts()[1];
  CompletionItemTester tester(testContext, insert);
  qDebug() << tester.names << "VERSUS" << completions;
  QCOMPARE(tester.names.toSet(), completions.toSet());
  release(top);
}

void TestCppCodeCompletion::testMemberAccessInstance()
{
  QByteArray test = "struct foo{}; int main() {}";
  TopDUContext* top = parse(test, DumpNone);
  DUChainWriteLocker lock(DUChain::lock());
  CompletionItemTester tester(top->childContexts()[2], "foo.");
  QCOMPARE(tester.names, QStringList());
  release(top);
}

void TestCppCodeCompletion::testNestedInlineNamespace()
{
  QByteArray test = "namespace a { inline namespace b { void foo(); } } int main() {}";
  TopDUContext* top = parse(test, DumpAll);
  DUChainWriteLocker lock;
  CompletionItemTester tester(top->childContexts()[2], "a::");
  QCOMPARE(tester.names, QStringList() << "b" << "foo" );
  release(top);
}

void TestCppCodeCompletion::testDuplicatedNamespace()
{
  // see also: https://bugs.kde.org/show_bug.cgi?id=328803
  QByteArray test( "namespace foo { const int bar = 1; }\n"
                   "namespace foo { const int asdf = 1; }\n"
                   "namespace foo { }\n"
                   "int main() {}\n" );
  TopDUContext* top = parse(test, DumpNone);
  DUChainWriteLocker lock;
  CompletionItemTester tester(top->childContexts().last(), "foo::");
  //TODO: the sort-order is apparently undefined...
  qSort(tester.names);
  QCOMPARE(tester.names, QStringList() << "asdf" << "bar" );
  release(top);
}

#include "test_cppcodecompletion.moc"
