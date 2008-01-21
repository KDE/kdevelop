/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda<rodda@kde.org>

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

#include <duchain.h>
#include <duchainlock.h>
#include <topducontext.h>
#include <duchain/typesystem.h>
#include <duchain/forwarddeclaration.h>
#include "declarationbuilder.h"
#include "usebuilder.h"
#include <declaration.h>
#include <documentrange.h>
#include "cppeditorintegrator.h"
#include "dumptypes.h"
#include "environmentmanager.h"

#include "tokens.h"
#include "parsesession.h"
#include <symboltable.h>

#include "rpp/preprocessor.h"
#include "rpp/pp-engine.h"
#include "rpp/pp-environment.h"
#include "expressionvisitor.h"
#include "expressionparser.h"
#include "classfunctiondeclaration.h"
#include "codecompletioncontext.h"
#include "cpppreprocessenvironment.h"

using namespace KTextEditor;

using namespace KDevelop;

QTEST_MAIN(TestCppCodeCompletion)

///@todo make forward-declarations work correctly across headers

QString testFile1 = "class Erna; struct Honk { int a,b; enum Enum { Number1, Number2 }; Erna& erna; operator int() {}; }; struct Pointer { Honk* operator ->() const {}; Honk& operator * () {}; }; Honk globalHonk; Honk honky; \n#define HONK Honk\n";

QString testFile2 = "struct Honk { int a,b; enum Enum { Number1, Number2 }; Erna& erna; operator int() {}; }; struct Erna { Erna( const Honk& honk ) {} }; struct Heinz : public Erna {}; Erna globalErna; Heinz globalHeinz; int globalInt; Heinz globalFunction(const Heinz& h ) {}; Erna globalFunction( const Erna& erna); Honk globalFunction( const Honk&, const Heinz& h ) {}; int globalFunction(int ) {}; HONK globalMacroHonk; struct GlobalClass { Heinz function(const Heinz& h ) {}; Erna function( const Erna& erna);  }; GlobalClass globalClass;\n#undef HONK\n";

QString testFile3 = "struct A {}; struct B : public A {};";

QString testFile4 = "void test1() {}; class TestClass() { TestClass() {} };";

namespace QTest {
  template<>
  char* toString(const Cursor& cursor)
  {
    QByteArray ba = "Cursor(";
    ba += QByteArray::number(cursor.line()) + ", " + QByteArray::number(cursor.column());
    ba += ')';
    return qstrdup(ba.data());
  }
  template<>
  char* toString(const QualifiedIdentifier& id)
  {
    QByteArray arr = id.toString().toLatin1();
    return qstrdup(arr.data());
  }
  template<>
  char* toString(const Identifier& id)
  {
    QByteArray arr = id.toString().toLatin1();
    return qstrdup(arr.data());
  }
  /*template<>
  char* toString(QualifiedIdentifier::MatchTypes t)
  {
    QString ret;
    switch (t) {
      case QualifiedIdentifier::NoMatch:
        ret = "No Match";
        break;
      case QualifiedIdentifier::Contains:
        ret = "Contains";
        break;
      case QualifiedIdentifier::ContainedBy:
        ret = "Contained By";
        break;
      case QualifiedIdentifier::ExactMatch:
        ret = "Exact Match";
        break;
    }
    QByteArray arr = ret.toString().toLatin1();
    return qstrdup(arr.data());
  }*/
  template<>
  char* toString(const Declaration& def)
  {
    QString s = QString("Declaration %1 (%2): %3").arg(def.identifier().toString()).arg(def.qualifiedIdentifier().toString()).arg(reinterpret_cast<long>(&def));
    return qstrdup(s.toLatin1().constData());
  }
  template<>
  char* toString(const KSharedPtr<AbstractType>& type)
  {
    QString s = QString("Type: %1").arg(type ? type->toString() : QString("<null>"));
    return qstrdup(s.toLatin1().constData());
  }
}

#define TEST_FILE_PARSE_ONLY if (testFileParseOnly) QSKIP("Skip", SkipSingle);
TestCppCodeCompletion::TestCppCodeCompletion()
{
  testFileParseOnly = false;
}

void TestCppCodeCompletion::initTestCase()
{
  typeVoid = AbstractType::Ptr::staticCast(TypeRepository::self()->integral(CppIntegralType::TypeVoid));
  typeInt = AbstractType::Ptr::staticCast(TypeRepository::self()->integral(CppIntegralType::TypeInt));

  addInclude( "testFile1.h", testFile1 );
  addInclude( "testFile2.h", testFile2 );
  addInclude( "testFile3.h", testFile3 );
}

void TestCppCodeCompletion::cleanupTestCase()
{
}

Declaration* TestCppCodeCompletion::findDeclaration(DUContext* context, const Identifier& id, const SimpleCursor& position)
{
  QList<Declaration*> ret = context->findDeclarations(id, position);
  if (ret.count())
    return ret.first();
  return 0;
}

Declaration* TestCppCodeCompletion::findDeclaration(DUContext* context, const QualifiedIdentifier& id, const SimpleCursor& position)
{
  QList<Declaration*> ret = context->findDeclarations(id, position);
  if (ret.count())
    return ret.first();
  return 0;
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
    Cpp::CodeCompletionContext::Ptr cptr( new  Cpp::CodeCompletionContext(DUContextPointer(DUContextPointer(context)), "; globalFunction(globalFunction(globalHonk, " ) );
    Cpp::CodeCompletionContext& c(*cptr);
    QVERIFY( c.isValid() );
    QVERIFY( c.memberAccessOperation() == Cpp::CodeCompletionContext::NoMemberAccess );
    QVERIFY( !c.memberAccessContainer().isValid() );

    //globalHonk is of type Honk. Check whether all matching functions were rated correctly by the overload-resolution.
    //The preferred parent-function in the list should be "Honk globalFunction( const Honk&, const Heinz& h )", because the first argument maches globalHonk
    Cpp::CodeCompletionContext* function = c.parentContext();
    QVERIFY(function);
    QVERIFY(function->memberAccessOperation() == Cpp::CodeCompletionContext::FunctionCallAccess);
    QVERIFY(!function->functions().isEmpty());
    
    lock.lock();
    for( Cpp::CodeCompletionContext::FunctionList::const_iterator it = function->functions().begin(); it != function->functions().end(); ++it )
      kDebug(9007) << (*it).function.declaration()->toString() << ((*it).function.isViable() ? QString("(viable)") : QString("(not viable)")) ;
    lock.unlock();
    
    QVERIFY(function->functions().size() == 4);
    QVERIFY(function->functions()[0].function.isViable());
    //Because Honk has a conversion-function to int, globalFunction(int) is yet viable(although it can take only 1 parameter)
    QVERIFY(function->functions()[1].function.isViable());
    //Because Erna has a constructor that takes "const Honk&", globalFunction(Erna) is yet viable(although it can take only 1 parameter)
    QVERIFY(function->functions()[2].function.isViable());
    //Because a value of type Honk is given, 2 globalFunction's are not viable
    QVERIFY(!function->functions()[3].function.isViable());
    
    
    function = function->parentContext();
    QVERIFY(function);
    QVERIFY(function->memberAccessOperation() == Cpp::CodeCompletionContext::FunctionCallAccess);
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
    Cpp::CodeCompletionContext c(contPtr, "{" );
    QVERIFY( c.isValid() );
    QVERIFY( c.memberAccessOperation() == Cpp::CodeCompletionContext::NoMemberAccess );
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
    CppFunctionType::Ptr test = findDeclaration( testContext, QualifiedIdentifier("test") )->type<CppFunctionType>();
    QVERIFY(test);
  
    Cpp::TypeConversion conv(context->topContext());
    QVERIFY(!conv.implicitConversion(test->returnType(), Heinz, false));
    QVERIFY(!conv.implicitConversion(Heinz, test->returnType(), false));
    QVERIFY(!conv.implicitConversion(test->returnType(), n, false));
    QVERIFY(!conv.implicitConversion(n, test->returnType(), false));
  }
  //lock.unlock();
  {
    ///Test whether a recursive function-call context is created correctly
    Cpp::TypeConversion conv(context->topContext());
    QVERIFY( !conv.implicitConversion(Honk, Heinz) );
    QVERIFY( conv.implicitConversion(Honk, typeInt) ); //Honk has operator int()
    QVERIFY( conv.implicitConversion(Honk, Erna) ); //Erna has constructor that takes Honk
    QVERIFY( !conv.implicitConversion(Erna, Heinz) );

    ///@todo reenable once base-classes are parsed correctly
    //QVERIFY( conv.implicitConversion(B, A) ); //B is based on A, so there is an implicit copy-constructor that creates A from B
    //QVERIFY( conv.implicitConversion(Heinz, Erna) ); //Heinz is based on Erna, so there is an implicit copy-constructor that creates Erna from Heinz
    
  }
  
  //lock.lock();
  release(context);
}

void TestCppCodeCompletion::testInclude() {
  TEST_FILE_PARSE_ONLY

  addInclude("file1.h", "#include \"testFile1.h\"\n#include \"testFile2.h\"\n");
  
  
  QByteArray test = "#include \"file1.h\"  \n  struct Cont { operator int() {}; }; void test( int c = 5 ) { this->test( Cont(), 1, 5.5, 6); }; HONK undefinedHonk;";
  DUContext* c = parse( test, DumpNone /*DumpDUChain | DumpAST */);
  DUChainWriteLocker lock(DUChain::lock());

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
  QVERIFY(dynamic_cast<DelayedType*>(decl->abstractType().data()));
  

  Cpp::ExpressionParser parser;

  ///The following test tests the expression-parser, but it is here because the other tests depend on it
  lock.unlock();
  Cpp::ExpressionEvaluationResult result = parser.evaluateExpression( "globalHonk.erna", DUContextPointer( c ) );
  lock.lock();
  
  QVERIFY(result.isValid());
  QVERIFY(result.instance);
  QVERIFY(result.type);
  QCOMPARE(result.type->toString(), QString("Erna&"));

  
  ///Test overload-resolution 
  lock.unlock();
  result = parser.evaluateExpression( "globalClass.function(globalHeinz)", DUContextPointer(c));
  lock.lock();
  
  QVERIFY(result.isValid());
  QVERIFY(result.instance);
  QVERIFY(result.type);
  QCOMPARE(result.type->toString(), QString("Heinz"));
  
  lock.unlock();
  result = parser.evaluateExpression( "globalClass.function(globalHonk.erna)", DUContextPointer(c));
  lock.lock();
  
  QVERIFY(result.isValid());
  QVERIFY(result.instance);
  QVERIFY(result.type);
  QCOMPARE(result.type->toString(), QString("Erna"));

  //No matching function for type Honk. Since the expression-parser is not set to "strict", it returns any found function with the right name.
  lock.unlock();
  result = parser.evaluateExpression( "globalClass.function(globalHonk)", DUContextPointer(c));
  lock.lock();
  
  QVERIFY(result.isValid());
  QVERIFY(result.instance);
  QVERIFY(result.type);
  //QCOMPARE(result.type->toString(), QString("Heinz"));
  
  
  lock.unlock();
  result = parser.evaluateExpression( "globalFunction(globalHeinz)", DUContextPointer(c));
  lock.lock();
  
  QVERIFY(result.isValid());
  QVERIFY(result.instance);
  QVERIFY(result.type);
  QCOMPARE(result.type->toString(), QString("Heinz"));
  lock.unlock();
  
  result = parser.evaluateExpression( "globalFunction(globalHonk.erna)", DUContextPointer(c));
  lock.lock();
  
  QVERIFY(result.isValid());
  QVERIFY(result.instance);
  QVERIFY(result.type);
  QCOMPARE(result.type->toString(), QString("Erna"));
    
  release(c);
}

void TestCppCodeCompletion::testUpdateChain() {
  TEST_FILE_PARSE_ONLY
      
  DUContext* context = parse( testFile3.toUtf8(), DumpNone, 0, KUrl("testIdentity") );
  parse( testFile3.toUtf8(), DumpNone, 0, KUrl("testIdentity") );
  parse( testFile3.toUtf8(), DumpNone, 0, KUrl("testIdentity") );
  parse( testFile3.toUtf8(), DumpNone, 0, KUrl("testIdentity") );
  parse( testFile3.toUtf8(), DumpNone, 0, KUrl("testIdentity") );
  parse( testFile3.toUtf8(), DumpNone, 0, KUrl("testIdentity") );
  parse( testFile3.toUtf8(), DumpNone, 0, KUrl("testIdentity") );
  
  
  DUChainWriteLocker lock(DUChain::lock());
  //lock.lock();
  release(context);
}

void TestCppCodeCompletion::testHeaderSections() {
  TEST_FILE_PARSE_ONLY
  /**
   * Make sure that the ends of header-sections are recognized correctly
   * */

  addInclude( "someHeader.h", "\n" );
  addInclude( "otherHeader.h", "\n" );
      
  IncludeFileList includes;

  HashedString turl("ths.h");
  
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

  DUContext* top = parse(method, DumpAll);

  DUChainWriteLocker lock(DUChain::lock());


  Declaration* decl = findDeclaration(top, Identifier("Test"), top->range().end);
  QVERIFY(decl);
  QVERIFY(decl->abstractType());
  QVERIFY(dynamic_cast<const IdentifiedType*>(decl->abstractType().data()));
  QVERIFY(!decl->isForwardDeclaration());
  
  release(top);
}

void TestCppCodeCompletion::testUsesThroughMacros() {
  {
    QByteArray method("int x;\n#define TEST(X) X\ny = TEST(x);");

    DUContext* top = parse(method, DumpAll);

    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(top->localDeclarations().count(), 1);
    QCOMPARE(top->localDeclarations()[0]->uses().count(), 1);
    top->localDeclarations()[0]->uses()[0]->range().start.column;
    QCOMPARE(top->localDeclarations()[0]->uses()[0]->range().start.column, 9);
    QCOMPARE(top->localDeclarations()[0]->uses()[0]->range().end.column, 10);
  }
  {
    ///2 uses of x, that go through the macro TEST(..), and effectively are in line 2 column 5.
    QByteArray method("int x;\n#define TEST(X) void test() { int z = X; int q = X; }\nTEST(x)");

    kDebug() << method;
    DUContext* top = parse(method, DumpAll);

    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(top->localDeclarations().count(), 2);
    QCOMPARE(top->localDeclarations()[0]->uses().count(), 2);

    const SimpleRange& range1(top->localDeclarations()[0]->uses()[0]->range());
    QCOMPARE(range1.start.line, 2);
    QCOMPARE(range1.end.line, 2);
    QCOMPARE(range1.start.column, 5);
    QCOMPARE(range1.end.column, 6);
    
    const SimpleRange& range2(top->localDeclarations()[0]->uses()[1]->range());
    QCOMPARE(range2.start.line, 2);
    QCOMPARE(range2.end.line, 2);
    
    QCOMPARE(range2.start.column, 5);
    QCOMPARE(range2.end.column, 6);
  }
}

void TestCppCodeCompletion::testAcrossHeaderReferences()
{
  addInclude( "acrossheader1.h", "class Test{ };" );
  addInclude( "acrossheader2.h", "Test t;" );
  QByteArray method("#include \"acrossheader1.h\"\n#include \"acrossheader2.h\"\n");

  DUContext* top = parse(method, DumpAll);

  DUChainWriteLocker lock(DUChain::lock());


  Declaration* decl = findDeclaration(top, Identifier("t"), top->range().end);
  QVERIFY(decl);
  QVERIFY(decl->abstractType());
  QVERIFY(dynamic_cast<const IdentifiedType*>(decl->abstractType().data()));
  
  release(top);
}

void TestCppCodeCompletion::testAcrossHeaderTemplateReferences()
{
  addInclude( "acrossheader1.h", "class Dummy { }; template<class Q> class Test{ };" );
  addInclude( "acrossheader2.h", "template<class B, class B2 = Test<B> > class Test2 : public Test<B>{ Test<B> bm; };" );
  QByteArray method("#include \"acrossheader1.h\"\n#include \"acrossheader2.h\"\n ");

  DUContext* top = parse(method, DumpAll);

  DUChainWriteLocker lock(DUChain::lock());


  {
    kDebug() << "top is" << top;
    Declaration* decl = findDeclaration(top, QualifiedIdentifier("Dummy"), top->range().end);
    QVERIFY(decl);
    QVERIFY(decl->abstractType());
    QVERIFY(dynamic_cast<const IdentifiedType*>(decl->abstractType().data()));
    QCOMPARE(decl->abstractType()->toString(), QString("Dummy"));
  }
  {
    Declaration* decl = findDeclaration(top, QualifiedIdentifier("Test2<Dummy>::B2"), top->range().end);
    QVERIFY(decl);
    QVERIFY(decl->abstractType());
    QVERIFY(dynamic_cast<const IdentifiedType*>(decl->abstractType().data()));
    QCOMPARE(decl->abstractType()->toString(), QString("Test< Dummy >"));
  }
  {
    Declaration* decl = findDeclaration(top, QualifiedIdentifier("Test2<Dummy>::bm"), top->range().end);
    QVERIFY(decl);
    QVERIFY(decl->abstractType());
    QVERIFY(dynamic_cast<const IdentifiedType*>(decl->abstractType().data()));
    QCOMPARE(decl->abstractType()->toString(), QString("Test< Dummy >"));
  }
  {
    Declaration* decl = findDeclaration(top, QualifiedIdentifier("Test2<Dummy>"), top->range().end);
    QVERIFY(decl);
    QVERIFY(decl->abstractType());
    CppClassType* classType = dynamic_cast<CppClassType*>(decl->abstractType().data());
    QVERIFY(classType);
    QCOMPARE(classType->baseClasses().count(), 1);
    QVERIFY(classType->baseClasses()[0].baseClass);
    kDebug() << typeid(*classType->baseClasses()[0].baseClass.data()).name();
    const CppClassType* parentClassType = dynamic_cast<const CppClassType*>(classType->baseClasses()[0].baseClass.data());
    QVERIFY(parentClassType);
    QCOMPARE(parentClassType->toString(), QString("Test< Dummy >"));
  }
  
  release(top);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TestCppCodeCompletion::release(DUContext* top)
{
  //KDevelop::EditorIntegrator::releaseTopRange(top->textRangePtr());
  if(dynamic_cast<TopDUContext*>(top))
    DUChain::self()->removeDocumentChain(static_cast<TopDUContext*>(top)->identity());
  //delete top;
}

void TestCppCodeCompletion::addInclude( const QString& identity, const QString& text ) {
  fakeIncludes[identity] = text;
}

//Only for debugging
QString print(const Cpp::StringSetRepository::LazySet& set) {
  Utils::Set s = set.set();
  QString ret;
  bool first = true;
  Cpp::StringSetRepository::Iterator it(&Cpp::EnvironmentManager::m_stringRepository, s.iterator());
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
QStringList toStringList( const Utils::Set& set ) {
  QStringList ret;
  Cpp::StringSetRepository::Iterator it(&Cpp::EnvironmentManager::m_stringRepository, set.iterator());
  while(it) {
    ret << (*it).str();
    ++it;
  }
  return ret;
}

void TestCppCodeCompletion::testEnvironmentMatching() {
    Cpp::EnvironmentManager* environmentManager = new Cpp::EnvironmentManager;
    {
        DUChainWriteLocker l(DUChain::lock());
        DUChain::self()->addParsingEnvironmentManager(environmentManager);
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

        QVERIFY(findDeclaration( test1, Identifier("H1") ));
        
      QCOMPARE( envFile1->contentStartLine(), 3 );
    }

    {
      addInclude("stringset_test1.h", "String1 s1;\n#undef String2\n String2 s2;");
      addInclude("stringset_test2.h", "String1 s1;\n#undef String2\n String2 s2;");
      
      {
        TopDUContext* top = parse(QByteArray("#include \"stringset_test1.h\"\n"), DumpNone);
        DUChainWriteLocker lock(DUChain::lock());
        QVERIFY(top->parsingEnvironmentFile());
        Cpp::EnvironmentFile* envFile = dynamic_cast<Cpp::EnvironmentFile*>(top->parsingEnvironmentFile().data());
        QVERIFY(envFile);
        QCOMPARE(toStringList(envFile->strings()), QString("String1\ns1\ns2").split("\n")); //The #undef protects String2, so it cannot be affected from outside
      }
      {
        TopDUContext* top = parse(QByteArray("#define String1\n#include \"stringset_test1.h\"\nString2 String1;"), DumpNone); //Both String1 and String2 are shadowed. String1 by the macro, and String2 by the #undef in stringset_test1.h
        DUChainWriteLocker lock(DUChain::lock());
        QVERIFY(top->parsingEnvironmentFile());
        Cpp::EnvironmentFile* envFile = dynamic_cast<Cpp::EnvironmentFile*>(top->parsingEnvironmentFile().data());
        QVERIFY(envFile);
        //String1 is shadowed by the macro-definition, so it is not a string that can be affected from outside.
        QCOMPARE(toStringList(envFile->strings()), QString("s1\ns2").split("\n"));
        QCOMPARE(toStringList(envFile->usedMacroNames().set()), QStringList()); //No macros from outside were used

        QCOMPARE(envFile->definedMacros().set().count(), 2u);
        
        QCOMPARE(top->importedParentContexts().count(), 1);
        TopDUContext* top2 = dynamic_cast<TopDUContext*>(top->importedParentContexts()[0].data());
        QVERIFY(top2);
        Cpp::EnvironmentFile* envFile2 = dynamic_cast<Cpp::EnvironmentFile*>(top2->parsingEnvironmentFile().data());
        QVERIFY(envFile2);
        
        QCOMPARE(envFile2->definedMacros().set().count(), 1u);
        
        QCOMPARE(toStringList(envFile2->usedMacroNames().set()), QStringList("String1")); //stringset_test1.h used the Macro String1 from outside
        QCOMPARE(toStringList(envFile2->strings()), QString("String1\ns1\ns2").split("\n"));
      }
      {
        TopDUContext* top = parse(QByteArray("#define String1\n#undef String1\n#include \"stringset_test1.h\""), DumpNone);
        DUChainWriteLocker lock(DUChain::lock());
        QVERIFY(top->parsingEnvironmentFile());
        Cpp::EnvironmentFile* envFile = dynamic_cast<Cpp::EnvironmentFile*>(top->parsingEnvironmentFile().data());
        QVERIFY(envFile);
        QCOMPARE(envFile->definedMacros().set().count(), 2u);
        //String1 is shadowed by the macro-definition, so it is not a string that can be affected from outside.
        QCOMPARE(toStringList(envFile->strings()), QString("s1\ns2").split("\n"));
        QCOMPARE(toStringList(envFile->usedMacroNames().set()), QStringList()); //No macros from outside were used

        QCOMPARE(top->importedParentContexts().count(), 1);
        TopDUContext* top2 = dynamic_cast<TopDUContext*>(top->importedParentContexts()[0].data());
        QVERIFY(top2);
        Cpp::EnvironmentFile* envFile2 = dynamic_cast<Cpp::EnvironmentFile*>(top2->parsingEnvironmentFile().data());
        QVERIFY(envFile2);
        QCOMPARE(envFile2->definedMacros().set().count(), 1u);
        
        QCOMPARE(toStringList(envFile2->usedMacroNames().set()), QStringList()); //stringset_test1.h used the Macro String1 from outside. However it is an undef-macro, so it does not appear in usedMacroNames() and usedMacros()
        QCOMPARE(envFile2->usedMacros().set().count(), (unsigned int)0);
        QCOMPARE(toStringList(envFile2->strings()), QString("String1\ns1\ns2").split("\n"));
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
    
    DUChainWriteLocker lock(DUChain::lock());
    DUChain::self()->removeParsingEnvironmentManager(environmentManager);
}

struct TestPreprocessor : public rpp::Preprocessor {

  TestCppCodeCompletion* cc;
  IncludeFileList& included;
  rpp::pp* pp;
  bool stopAfterHeaders;
  Cpp::EnvironmentFilePointer environmentFile;

  TestPreprocessor( TestCppCodeCompletion* _cc, IncludeFileList& _included, bool _stopAfterHeaders ) : cc(_cc), included(_included), pp(0), stopAfterHeaders(_stopAfterHeaders) {
  }
  
  rpp::Stream* sourceNeeded(QString& fileName, rpp::Preprocessor::IncludeType type, int sourceLine, bool skipCurrentPath)
  {
    QMap<QString,QString>::const_iterator it = cc->fakeIncludes.find(fileName);
    if( it != cc->fakeIncludes.end() || !pp ) {
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
};

QString TestCppCodeCompletion::preprocess( const HashedString& url, const QString& text, IncludeFileList& included, rpp::pp* parent, bool stopAfterHeaders, KSharedPtr<Cpp::EnvironmentFile>* paramEnvironmentFile, rpp::LocationTable** returnLocationTable ) {
  TestPreprocessor ppc( this, included, stopAfterHeaders );

    
    rpp::pp preprocessor(&ppc);
    ppc.setPp( &preprocessor );

    KSharedPtr<Cpp::EnvironmentFile> environmentFile;
    if( paramEnvironmentFile && *paramEnvironmentFile )
      environmentFile = *paramEnvironmentFile;
    else
      environmentFile = Cpp::EnvironmentFilePointer( new Cpp::EnvironmentFile( url, 0 ) );

  ppc.environmentFile = environmentFile;
  
    if( paramEnvironmentFile )
      *paramEnvironmentFile = environmentFile;
    
    CppPreprocessEnvironment* currentEnvironment = new CppPreprocessEnvironment( &preprocessor, environmentFile );
    preprocessor.setEnvironment( currentEnvironment );
    currentEnvironment->setEnvironmentFile( environmentFile );
    
    rpp::MacroBlock* macros = new rpp::MacroBlock(0);

    preprocessor.environment()->enterBlock(macros);
    
    if( parent )
      preprocessor.environment()->swapMacros(parent->environment());
    
    QString result = preprocessor.processFile("<test>", rpp::pp::Data, text);

    if (returnLocationTable)
      *returnLocationTable = preprocessor.environment()->takeLocationTable();
    
    currentEnvironment->finish();

    if( parent ) {
      preprocessor.environment()->swapMacros(parent->environment());
      static_cast<CppPreprocessEnvironment*>(parent->environment())->environmentFile()->merge(*environmentFile);
    }

    return result;
}

TopDUContext* TestCppCodeCompletion::parse(const QByteArray& unit, DumpAreas dump, rpp::pp* parent, KUrl _identity)
{
  if (dump)
    kDebug(9007) << "==== Beginning new test case...:" << endl << unit;

  ParseSession* session = new ParseSession();
   ;
  
  static int testNumber = 0;
  HashedString url(QString("file:///internal/%1").arg(testNumber++));
  if( !_identity.isEmpty() )
      url = _identity.prettyUrl();

   IncludeFileList included;
   QList<DUContext*> temporaryIncluded;

  rpp::LocationTable* locationTable;
  
  Cpp::EnvironmentFilePointer file;
   
  QByteArray source = preprocess( url, QString::fromUtf8(unit), included, parent, false, &file, &locationTable ).toUtf8();

  session->setContents( source, locationTable );

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
    dumper.dump(ast, session);
  }

  DeclarationBuilder definitionBuilder(session);

  TopDUContext* top = definitionBuilder.buildDeclarations(file, ast, &included);

  UseBuilder useBuilder(session);
  useBuilder.buildUses(ast);

  if (dump & DumpDUChain) {
    kDebug(9007) << "===== DUChain:";

    DUChainWriteLocker lock(DUChain::lock());
    dumper.dump(top);
  }
  
  if (dump & DumpType) {
    kDebug(9007) << "===== Types:";
    DumpTypes dt;
    DUChainWriteLocker lock(DUChain::lock());
    foreach (const AbstractType::Ptr& type, definitionBuilder.topTypes())
      dt.dump(type.data());
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

#include "test_cppcodecompletion.moc"
