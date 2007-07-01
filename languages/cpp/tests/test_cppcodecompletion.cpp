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

using namespace KTextEditor;

using namespace KDevelop;

QTEST_MAIN(TestCppCodeCompletion)

QString testFile1 = "struct Honk { int a,b; enum Enum { Number1, Number2 }; }; struct Pointer { Honk* operator ->() const {}; Honk& operator * () {}; }; Honk globalHonk; Honk honky; \n#define HONK Honk\n";
QString testFile2 = "class Honk; struct Erna {}; struct Heinz; Erna globalErna; Heinz globalHeinz; int globalInt; Heinz globalFunction(const Heinz& h ) {}; Erna globalFunction( const Erna& erna); HONK globalMacroHonk; \n#undef HONK\n";

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
      
  QByteArray test = "struct Cont { operator int() {}; }; void test( int c = 5 ) { this->test( Cont(), 1, 5.5, 6); }";
  DUContext* c = parse( test, DumpNone /*DumpDUChain | DumpAST */);
  DUChainWriteLocker lock(DUChain::lock());
  
  DUContext* testContext = c->childContexts()[1];
  QCOMPARE( testContext->type(), DUContext::Function );
  
  DUContext* contContext = c->childContexts()[0];
  Declaration* decl = contContext->localDeclarations()[0];

  QCOMPARE(decl->identifier(), Identifier("operator<...cast...>"));
  CppFunctionType* function = dynamic_cast<CppFunctionType*>(decl->abstractType().data());
  QCOMPARE(function->returnType()->toString(), QString("int"));

  Declaration* testDecl = c->localDeclarations()[1];
  ClassFunctionDeclaration* functionDecl = dynamic_cast<ClassFunctionDeclaration*>(testDecl);
  QVERIFY(functionDecl);

  QVERIFY(functionDecl->defaultParameters().size() == 1);
  QCOMPARE(functionDecl->defaultParameters()[0], QString("5"));
  
  //QVERIFY(0);
  //lock.lock();
  release(c);
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
  
  decl = findDeclaration(c, QualifiedIdentifier("undefinedHonk"));
  QVERIFY(decl);
  QVERIFY(!decl->abstractType());
  
  
/*  DUContext* testContext = c->childContexts()[1];
  QCOMPARE( testContext->type(), DUContext::Function );
  
  DUContext* contContext = c->childContexts()[0];
  Declaration* decl = contContext->localDeclarations()[0];

  QCOMPARE(decl->identifier(), Identifier("operator<...cast...>"));
  CppFunctionType* function = dynamic_cast<CppFunctionType*>(decl->abstractType().data());
  QCOMPARE(function->returnType()->toString(), QString("int"));

  Declaration* testDecl = c->localDeclarations()[1];
  ClassFunctionDeclaration* functionDecl = dynamic_cast<ClassFunctionDeclaration*>(testDecl);
  QVERIFY(functionDecl);

  QVERIFY(functionDecl->defaultParameters().size() == 1);
  QCOMPARE(functionDecl->defaultParameters()[0], QString("5"));*/
  
  //QVERIFY(0);
  //lock.lock();
  release(c);
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

DUContext* TestCppCodeCompletion::parse(const QByteArray& unit, DumpAreas dump, rpp::pp* parent)
{
  if (dump)
    kDebug() << "==== Beginning new test case...:" << endl << unit << endl << endl;

  ParseSession* session = new ParseSession();
   ;

   QList<DUContext*> included;

  session->setContents( preprocess( QString::fromUtf8(unit), included, parent ).toUtf8() );

  Parser parser(&control);
  TranslationUnitAST* ast = parser.parse(session);
  ast->session = session;

  if (dump & DumpAST) {
    kDebug() << "===== AST:" << endl;
    dumper.dump(ast, session);
  }

  static int testNumber = 0;
  KUrl url(QString("file:///internal/%1").arg(testNumber++));

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

  if (dump)
    kDebug() << "===== Finished test case." << endl;

  delete session;

  return top;
}

#include "test_cppcodecompletion.moc"
