/* This file is part of KDevelop
    Copyright (C) 2006 Hamish Rodda<rodda@kde.org>

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

#include <duchain.h>
#include <duchainlock.h>
#include <topducontext.h>
#include <duchain/typesystem.h>
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

Declaration* TestCppCodeCompletion::findDeclaration(DUContext* context, const Identifier& id, const Cursor& position)
{
  QList<Declaration*> ret = context->findDeclarations(id, position);
  if (ret.count())
    return ret.first();
  return 0;
}

Declaration* TestCppCodeCompletion::findDeclaration(DUContext* context, const QualifiedIdentifier& id, const Cursor& position)
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
  
  DUContext* testContext = context->childContexts()[0];
  QCOMPARE( testContext->type(), DUContext::Function );

  lock.unlock();
  {
    ///Test whether a recursive function-call context is created correctly
    Cpp::CodeCompletionContext::Ptr cptr( new  Cpp::CodeCompletionContext(context, "; globalFunction(globalFunction(globalHonk, " ) );
    Cpp::CodeCompletionContext& c(*cptr);
    // this doesn't work, as c is not a pointer
    // QVERIFY( c );
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
      kDebug() << (*it).function.declaration()->toString() << ((*it).function.isViable() ? QString("(viable)") : QString("(not viable)"))  << endl;
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
    Cpp::CodeCompletionContext c( context, "{" );
    // this test doesn't work because c is not a pointer
    // QVERIFY( c );
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
  test += "void test() { }";
      
  DUContext* context = parse( test, DumpNone /*DumpDUChain | DumpAST */);
  DUChainWriteLocker lock(DUChain::lock());
  
  DUContext* testContext = context->childContexts()[0];
  QCOMPARE( testContext->type(), DUContext::Function );

  AbstractType::Ptr Heinz = findDeclaration( testContext, QualifiedIdentifier("Heinz") )->abstractType();
  AbstractType::Ptr Erna = findDeclaration( testContext, QualifiedIdentifier("Erna") )->abstractType();
  AbstractType::Ptr Honk = findDeclaration( testContext, QualifiedIdentifier("Honk") )->abstractType();
  AbstractType::Ptr A = findDeclaration( testContext, QualifiedIdentifier("A") )->abstractType();
  AbstractType::Ptr B = findDeclaration( testContext, QualifiedIdentifier("B") )->abstractType();
  
  //lock.unlock();
  {
    ///Test whether a recursive function-call context is created correctly
    Cpp::TypeConversion conv;
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
  QVERIFY(!decl->abstractType());
  

  Cpp::ExpressionParser parser;

  ///The following test tests the expression-parser, but it is here because the other tests depend on it
  lock.unlock();
  Cpp::ExpressionEvaluationResult result = parser.evaluateType( "globalHonk.erna", c );
  lock.lock();
  
  QVERIFY(result.isValid());
  QVERIFY(result.instance);
  QVERIFY(result.type);
  QCOMPARE(result.type->toString(), QString("Erna&"));

  
  ///Test overload-resolution 
  lock.unlock();
  result = parser.evaluateType( "globalClass.function(globalHeinz)", c );
  lock.lock();
  
  QVERIFY(result.isValid());
  QVERIFY(result.instance);
  QVERIFY(result.type);
  QCOMPARE(result.type->toString(), QString("Heinz"));
  
  lock.unlock();
  result = parser.evaluateType( "globalClass.function(globalHonk.erna)", c );
  lock.lock();
  
  QVERIFY(result.isValid());
  QVERIFY(result.instance);
  QVERIFY(result.type);
  QCOMPARE(result.type->toString(), QString("Erna"));

  //No matching function for type Honk. Since the expression-parser is not set to "strict", it returns any found function with the right name.
  lock.unlock();
  result = parser.evaluateType( "globalClass.function(globalHonk)", c );
  lock.lock();
  
  QVERIFY(result.isValid());
  QVERIFY(result.instance);
  QVERIFY(result.type);
  //QCOMPARE(result.type->toString(), QString("Heinz"));
  
  
  lock.unlock();
  result = parser.evaluateType( "globalFunction(globalHeinz)", c );
  lock.lock();
  
  QVERIFY(result.isValid());
  QVERIFY(result.instance);
  QVERIFY(result.type);
  QCOMPARE(result.type->toString(), QString("Heinz"));
  lock.unlock();
  
  result = parser.evaluateType( "globalFunction(globalHonk.erna)", c );
  lock.lock();
  
  QVERIFY(result.isValid());
  QVERIFY(result.instance);
  QVERIFY(result.type);
  QCOMPARE(result.type->toString(), QString("Erna"));
    
  release(c);
}

void TestCppCodeCompletion::testUpdateChain() {
  TEST_FILE_PARSE_ONLY
      
  DUContext* context = parse( testFile3.toUtf8(), static_cast<DumpAreas>(DumpDUChain | DumpAST |  DumpType), 0, KUrl("testIdentity") );
  parse( testFile3.toUtf8(), static_cast<DumpAreas>(DumpDUChain | DumpType), 0, KUrl("testIdentity") );
  parse( testFile3.toUtf8(), static_cast<DumpAreas>(DumpDUChain | DumpType), 0, KUrl("testIdentity") );
  parse( testFile3.toUtf8(), static_cast<DumpAreas>(DumpDUChain | DumpType), 0, KUrl("testIdentity") );
  parse( testFile3.toUtf8(), static_cast<DumpAreas>(DumpDUChain | DumpType), 0, KUrl("testIdentity") );
  parse( testFile3.toUtf8(), static_cast<DumpAreas>(DumpDUChain | DumpType), 0, KUrl("testIdentity") );
  parse( testFile3.toUtf8(), static_cast<DumpAreas>(DumpDUChain | DumpType), 0, KUrl("testIdentity") );
  
  
  DUChainWriteLocker lock(DUChain::lock());
  //lock.lock();
  release(context);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TestCppCodeCompletion::release(DUContext* top)
{
  KDevelop::EditorIntegrator::releaseTopRange(top->textRangePtr());
  delete top;
}

void TestCppCodeCompletion::addInclude( const QString& identity, const QString& text ) {
  fakeIncludes[identity] = text;
}

struct TestPreprocessor : public rpp::Preprocessor {

  TestCppCodeCompletion* cc;
  QList<DUContext*>& included;
  rpp::pp* pp;

  TestPreprocessor( TestCppCodeCompletion* _cc, QList<DUContext*>& _included ) : cc(_cc), included(_included), pp(0) {
  }
  
  rpp::Stream* sourceNeeded(QString& fileName, rpp::Preprocessor::IncludeType type, int sourceLine)
  {
    QMap<QString,QString>::const_iterator it = cc->fakeIncludes.find(fileName);
    if( it != cc->fakeIncludes.end() || !pp ) {
      kDebug() << "parsing included file \"" << fileName << "\"" << endl;
      included << cc->parse( (*it).toUtf8(), TestCppCodeCompletion::DumpNone, pp);
    } else {
      kDebug() << "could not find include-file \"" << fileName << "\"" << endl;
    }
    return 0;
  }

  void setPp( rpp::pp* _pp ) {
    pp = _pp;
  }
};

QString TestCppCodeCompletion::preprocess( const QString& text, QList<DUContext*>& included, rpp::pp* parent ) {
  TestPreprocessor ppc( this, included );

    rpp::pp preprocessor(&ppc);
    ppc.setPp( &preprocessor );
    rpp::MacroBlock* macros = new rpp::MacroBlock(0);

    preprocessor.environment()->enterBlock(macros);
    
    if( parent )
      preprocessor.environment()->swapMacros(parent->environment());
    
    QString result = preprocessor.processFile(text, rpp::pp::Data);

        //Merge include-file-set, defined macros, used macros, and string-set
//         parentPreprocessor->m_lexedFile->merge(*m_lexedFile);
  
    if( parent )
            preprocessor.environment()->swapMacros(parent->environment());
    return result;
}

DUContext* TestCppCodeCompletion::parse(const QByteArray& unit, DumpAreas dump, rpp::pp* parent, KUrl _identity)
{
  if (dump)
    kDebug() << "==== Beginning new test case...:" << endl << unit << endl << endl;

  ParseSession* session = new ParseSession();
   ;

   QList<DUContext*> included;
   QList<DUContext*> temporaryIncluded;

  session->setContents( preprocess( QString::fromUtf8(unit), included, parent ).toUtf8() );

    if( parent ) {
      //Temporarily insert all files parsed previously by the parent, so forward-declarations can be resolved etc.
      TestPreprocessor* testPreproc = dynamic_cast<TestPreprocessor*>(parent->preprocessor());
      if( testPreproc ) {
        foreach( DUContext* include, testPreproc->included ) {
          if( !included.contains( include ) ) {
            included.push_front( include );
            temporaryIncluded << include;
          }
        }
      } else {
        kDebug() << "PROBLEM" << endl;
      }
    }
  
  Parser parser(&control);
  TranslationUnitAST* ast = parser.parse(session);
  ast->session = session;

  if (dump & DumpAST) {
    kDebug() << "===== AST:" << endl;
    dumper.dump(ast, session);
  }

  static int testNumber = 0;
  KUrl url(QString("file:///internal/%1").arg(testNumber++));
  if( !_identity.isEmpty() )
        url = _identity;

  DeclarationBuilder definitionBuilder(session);

  //LexedFile is not built correctly by TestPreprocessor
  Cpp::LexedFilePointer file( new Cpp::LexedFile( url, 0 ) );
  TopDUContext* top = definitionBuilder.buildDeclarations(file, ast, &included);

  UseBuilder useBuilder(session);
  useBuilder.buildUses(ast);

  if (dump & DumpDUChain) {
    kDebug() << "===== DUChain:" << endl;

    DUChainWriteLocker lock(DUChain::lock());
    dumper.dump(top);
  }
  
  if (dump & DumpType) {
    kDebug() << "===== Types:" << endl;
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
      kDebug() << "PROBLEM" << endl;
    }
  }
  

  if (dump)
    kDebug() << "===== Finished test case." << endl;

  delete session;

  return top;
}

#include "test_cppcodecompletion.moc"
