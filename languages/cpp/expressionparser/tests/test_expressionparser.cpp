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

#include "test_expressionparser.h"

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

#include "tokens.h"
#include "parsesession.h"
#include <symboltable.h>

#include "rpp/preprocessor.h"
#include "expressionvisitor.h"
#include "expressionparser.h"
#include "classfunctiondeclaration.h"

using namespace KTextEditor;

using namespace KDevelop;

class MyExpressionVisitor : public Cpp::ExpressionVisitor {
  public:
  MyExpressionVisitor( ParseSession* session ) : ExpressionVisitor(session)  {
  }
  protected:
  virtual void expressionType( AST* node, const AbstractType::Ptr& type, Instance instance ) {
    DumpChain d;
    kDebug() << "expression-result for " << endl;
    DUChainReadLocker lock( DUChain::lock() );
    d.dump( node, session() );
    kDebug() << "is: " << type->toString() << endl;
  }
};

QTEST_MAIN(TestExpressionParser)

char* debugString( const QString& str ) {
  char* ret = new char[str.length()+1];
  QByteArray b = str.toAscii();
  strcpy( ret, b.data() );
  return ret;
}

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
TestExpressionParser::TestExpressionParser()
{
  testFileParseOnly = false;
}

void TestExpressionParser::initTestCase()
{
  typeVoid = AbstractType::Ptr::staticCast(TypeRepository::self()->integral(CppIntegralType::TypeVoid));
  typeInt = AbstractType::Ptr::staticCast(TypeRepository::self()->integral(CppIntegralType::TypeInt));
}

void TestExpressionParser::cleanupTestCase()
{
}

Declaration* TestExpressionParser::findDeclaration(DUContext* context, const Identifier& id, const Cursor& position)
{
  QList<Declaration*> ret = context->findDeclarations(id, position);
  if (ret.count())
    return ret.first();
  return 0;
}

Declaration* TestExpressionParser::findDeclaration(DUContext* context, const QualifiedIdentifier& id, const Cursor& position)
{
  QList<Declaration*> ret = context->findDeclarations(id, position);
  if (ret.count())
    return ret.first();
  return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TestExpressionParser::testSimpleExpression() {
  TEST_FILE_PARSE_ONLY
      
  QByteArray test = "struct Cont { int& a; Cont* operator -> () {}; double operator*(); }; Cont c; Cont* d = &c; void test() { c.a = 5; d->a = 5; (*d).a = 5; c.a(5, 1, c); c.b<Fulli>(); }";
  DUContext* c = parse( test, DumpDUChain | DumpAST );
  DUChainWriteLocker lock(DUChain::lock());
  
  DUContext* testContext = c->childContexts()[1];
  QCOMPARE( testContext->type(), DUContext::Function );

  //Make sure the declaration of "c" is found correctly
  Declaration* d = findDeclaration( testContext, QualifiedIdentifier("c") );
  QVERIFY(d);
  QVERIFY( dynamic_cast<IdentifiedType*>( d->abstractType().data() ) );
  QVERIFY( dynamic_cast<IdentifiedType*>( d->abstractType().data() )->identifier().toString() == "Cont" );

  kDebug() << "test-Context: " << testContext << endl;
  lock.unlock();

  Cpp::ExpressionParser parser;

  Cpp::ExpressionEvaluationResult::Ptr result = parser.evaluateType( "c.a", testContext );
  lock.lock();
  QVERIFY(result);   
  QVERIFY(result->instance);
  QVERIFY(result->type);
  lock.unlock();

  result = parser.evaluateType( "d", testContext );
  lock.lock();
  QVERIFY(result);
  QVERIFY(result->instance);
  QCOMPARE(result->type->toString(), QString("Cont*"));
  QCOMPARE(dynamic_cast<CppPointerType*>(result->type.data())->baseType()->toString(), QString("Cont"));
  lock.unlock();
  
  //Test pointer-referencing
  result = parser.evaluateType( "&c.a", testContext );
  lock.lock();
  QVERIFY(result);
  QCOMPARE(result->type->toString(), QString("int*"));
  QVERIFY(result->instance);
  lock.unlock();

  //Test pointer-referencing and dereferencing
  result = parser.evaluateType( "*(&c.a)", testContext );
  lock.lock();
  QVERIFY(result);
  QCOMPARE(result->type->toString(), QString("int"));
  QVERIFY(result->instance);
  lock.unlock();
  
  //Test overloaded "operator*"
  result = parser.evaluateType( "*c", testContext );
  lock.lock();
  QVERIFY(result);
  QCOMPARE(result->type->toString(), QString("double"));
  QVERIFY(result->instance);
  lock.unlock();

  //Test overloaded "operator->"
  result = parser.evaluateType( "c->a", testContext );
  lock.lock();
  QVERIFY(result);
  QCOMPARE(result->type->toString(), QString("int&"));
  QVERIFY(result->instance);
  lock.unlock();

  //Test normal pointer-access + assign expression
  result = parser.evaluateType( "d->a = 5", testContext );
  lock.lock();
  QVERIFY(result);
  QCOMPARE(result->type->toString(), QString("int&"));
  QVERIFY(result->instance);
  lock.unlock();
  
  //Test double * (one real, one overloaded)
  result = parser.evaluateType( "**d", testContext );
  lock.lock();
  QVERIFY(result);
  QCOMPARE(result->type->toString(), QString("double"));
  QVERIFY(result->instance);
  lock.unlock();

  //Test double &
  result = parser.evaluateType( "&d", testContext );
  lock.lock();
  QVERIFY(result);
  QCOMPARE(result->type->toString(), QString("Cont**"));
  QVERIFY(result->instance);
  lock.unlock();
  
  //Test type-expression
  result = parser.evaluateType( "Cont", testContext );
  lock.lock();
  QVERIFY(result);
  QVERIFY(!result->instance);
  QVERIFY(result->type);
  lock.unlock();

  //Test conditional expression
  result = parser.evaluateType( "a ? c.a : c.a", testContext );
  lock.lock();
  QVERIFY(result);
  QCOMPARE(result->type->toString(), QString("int&"));
  QVERIFY(result->instance);
  lock.unlock();
  

  lock.lock();
  release(c);
}

void TestExpressionParser::testTypeConversion() {
  TEST_FILE_PARSE_ONLY
      
  QByteArray test = "struct Cont { operator int() {}; }; void test( int c = 5 ) { this->test( Cont(), 1, 5.5, 6); }";
  DUContext* c = parse( test, DumpDUChain | DumpAST );
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

void TestExpressionParser::testCasts() {
  TEST_FILE_PARSE_ONLY
      
  QByteArray test = "struct Cont2 {}; struct Cont { int& a; Cont* operator -> () {}; double operator*(); }; Cont c; Cont* d = &c; void test() { c.a = 5; d->a = 5; (*d).a = 5; c.a(5, 1, c); c.b<Fulli>(); c.a = dynamic_cast<const Cont2*>(d); }";
  DUContext* c = parse( test, DumpDUChain | DumpAST );
  DUChainWriteLocker lock(DUChain::lock());
  
  DUContext* testContext = c->childContexts()[2];
  QCOMPARE( testContext->type(), DUContext::Function );

  //Make sure the declaration of "c" is found correctly
  Declaration* d = findDeclaration( testContext, QualifiedIdentifier("c") );
  QVERIFY(d);
  QVERIFY( dynamic_cast<IdentifiedType*>( d->abstractType().data() ) );
  QVERIFY( dynamic_cast<IdentifiedType*>( d->abstractType().data() )->identifier().toString() == "Cont" );

  lock.unlock();

  Cpp::ExpressionParser parser;

  //Reenable this once the type-parsing system etc. is fixed
  /*
  Cpp::ExpressionEvaluationResult::Ptr result = parser.evaluateType( "dynamic_cast<Cont2*>(d)", testContext );
  lock.lock();
  QVERIFY(result);   
  QVERIFY(result->instance);
  QVERIFY(result->type);
  QCOMPARE(result->type->toString(), QString("Cont2*"));
  lock.unlock();

  result = parser.evaluateType( "static_cast<Cont2*>(d)", testContext );
  lock.lock();
  QVERIFY(result);   
  QVERIFY(result->instance);
  QVERIFY(result->type);
  QCOMPARE(result->type->toString(), QString("Cont2*"));
  lock.unlock();
  
  result = parser.evaluateType( "reinterpret_cast<Cont2*>(d)", testContext );
  lock.lock();
  QVERIFY(result);   
  QVERIFY(result->instance);
  QVERIFY(result->type);
  QCOMPARE(result->type->toString(), QString("Cont2*"));
  lock.unlock();*/
  
  lock.lock();
  release(c);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TestExpressionParser::release(DUContext* top)
{
  KDevelop::EditorIntegrator::releaseTopRange(top->textRangePtr());
  delete top;
}

DUContext* TestExpressionParser::parse(const QByteArray& unit, DumpAreas dump)
{
  if (dump)
    kDebug() << "==== Beginning new test case...:" << endl << unit << endl << endl;

  ParseSession* session = new ParseSession();
  session->setContents(unit);

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

  TopDUContext* top = definitionBuilder.buildDeclarations(url, ast);

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

  MyExpressionVisitor v(session);
  v.visit(ast);
  
  if (dump)
    kDebug() << "===== Finished test case." << endl;

  delete session;

  return top;
}

#include "test_expressionparser.moc"
