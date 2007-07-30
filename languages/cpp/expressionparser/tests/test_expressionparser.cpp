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

#include "test_expressionparser.h"

#include <QtTest/QtTest>

#include <duchain.h>
#include <duchainlock.h>
#include <topducontext.h>
#include <duchain/typesystem.h>
#include "typeutils.h"
#include "declarationbuilder.h"
#include "usebuilder.h"
#include <declaration.h>
#include <documentrange.h>
#include "cppeditorintegrator.h"
#include "dumptypes.h"
#include "environmentmanager.h"
#include "templatedeclaration.h"
#include "cppducontext.h"


#include "tokens.h"
#include "parsesession.h"
#include <symboltable.h>

#include "rpp/preprocessor.h"
#include "expressionvisitor.h"
#include "expressionparser.h"
#include "classfunctiondeclaration.h"

using namespace KTextEditor;
using namespace KDevelop;
using namespace TypeUtils;
using namespace Cpp;

class MyExpressionVisitor : public Cpp::ExpressionVisitor {
  public:
  MyExpressionVisitor( ParseSession* session ) : ExpressionVisitor(session)  {
  }
  protected:
  virtual void expressionType( AST* node, const AbstractType::Ptr& type, Instance /*instance */) {
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

void TestExpressionParser::testTemplates() {
  QByteArray method("template<class T> T test(const T& t) {}; template<class T, class T2> class A { }; class B{}; class C{}; typedef A<B,C> B;");

  DUContext* top = parse(method, DumpAll);

  DUChainWriteLocker lock(DUChain::lock());

  Declaration* defClassA = top->localDeclarations()[1];
  QCOMPARE(defClassA->identifier(), Identifier("A"));
  QVERIFY(defClassA->type<CppClassType>());

  Declaration* defClassB = top->localDeclarations()[2];
  QCOMPARE(defClassB->identifier(), Identifier("B"));
  QVERIFY(defClassB->type<CppClassType>());
  
  Declaration* defClassC = top->localDeclarations()[3];
  QCOMPARE(defClassC->identifier(), Identifier("C"));
  QVERIFY(defClassC->type<CppClassType>());
  
  DUContext* classA = TypeUtils::getInternalContext(defClassA);
  QVERIFY(classA->parentContext());
  QCOMPARE(classA->importedParentContexts().count(), 1); //The template-parameter context is imported
  QCOMPARE(classA->localScopeIdentifier(), QualifiedIdentifier("A"));

  DUContext* classB = TypeUtils::getInternalContext(defClassB);
  QVERIFY(classB->parentContext());
  QCOMPARE(classB->importedParentContexts().count(), 0);
  QCOMPARE(classB->localScopeIdentifier(), QualifiedIdentifier("B"));
  
  DUContext* classC = TypeUtils::getInternalContext(defClassC);
  QVERIFY(classC->parentContext());
  QCOMPARE(classC->importedParentContexts().count(), 0);
  QCOMPARE(classC->localScopeIdentifier(), QualifiedIdentifier("C"));

/*  QCOMPARE(findDeclaration(top,  Identifier("B"))->abstractType(), defClassA->abstractType());
  QVERIFY(findDeclaration(top,  Identifier("B"))->isTypeAlias());
  QCOMPARE(findDeclaration(top,  Identifier("B"))->kind(), Declaration::Type);*/
  release(top);
}

void TestExpressionParser::testSmartPointer() {
  QByteArray method("template<class T> struct SmartPointer { T* operator ->() const {} T& operator*() {}  } ; class B{int i;}; class C{}; SmartPointer<B> bPointer;");

  DUContext* top = parse(method, DumpAll);
  
  DUChainWriteLocker lock(DUChain::lock());

  Cpp::ExpressionParser parser;

  IdentifiedType* idType = dynamic_cast<IdentifiedType*>(top->localDeclarations()[3]->abstractType().data());
  QVERIFY(idType);
  QCOMPARE(idType->declaration()->context(), top);
  QVERIFY(idType->declaration()->internalContext() != top->localDeclarations()[0]->internalContext());

  Declaration* baseDecl = top->localDeclarations()[0];
  Declaration* specialDecl = idType->declaration();
  TemplateDeclaration* baseTemplateDecl = dynamic_cast<TemplateDeclaration*>(baseDecl);
  TemplateDeclaration* specialTemplateDecl = dynamic_cast<TemplateDeclaration*>(specialDecl);

  QVERIFY(baseTemplateDecl);
  QVERIFY(specialTemplateDecl);
  QVERIFY(specialTemplateDecl->isInstantiatedFrom(baseTemplateDecl));

  QVERIFY(specialDecl->internalContext() != baseDecl->internalContext());
  QCOMPARE(specialDecl->internalContext()->importedParentContexts().count(), 1); //Only the template-contexts are imported
  QCOMPARE(baseDecl->internalContext()->importedParentContexts().count(), 1);

  DUContext* specialTemplateContext = specialDecl->internalContext()->importedParentContexts().first().data();
  DUContext* baseTemplateContext = baseDecl->internalContext()->importedParentContexts().first().data();
  QVERIFY(specialTemplateContext != baseTemplateContext);
  QCOMPARE(specialTemplateContext->type(), DUContext::Template);
  QCOMPARE(baseTemplateContext->type(), DUContext::Template);
  kDebug() << typeid(baseTemplateContext).name() << endl;
  kDebug() << typeid(specialTemplateContext).name() << endl;
/*  CppDUContext<DUContext>* baseTemplateCtx = dynamic_cast<CppDUContext<DUContext>*>(baseTemplateContext);
  CppDUContext<DUContext>* specialTemplateCtx = dynamic_cast<CppDUContext<DUContext>*>(specialTemplateContext);
  QVERIFY(baseTemplateCtx);
  QVERIFY(specialTemplateCtx);
  QCOMPARE(specialTemplateCtx->instantiatedFrom(), baseTemplateContext);*/
  
  QCOMPARE(specialTemplateContext->localDeclarations().count(), 1);
  QCOMPARE(baseTemplateContext->localDeclarations().count(), 1);
  QVERIFY(specialTemplateContext->localDeclarations()[0] != baseTemplateContext->localDeclarations()[0]);
  
  kDebug() << top->localDeclarations()[3]->abstractType()->toString() << endl;
  Cpp::ExpressionEvaluationResult result = parser.evaluateType( "*bPointer", top );
  QVERIFY(result.instance);
  QCOMPARE(result.type->toString(), QString("B&"));

  result = parser.evaluateType( "bPointer->i", top );
  QVERIFY(result.instance);
  QCOMPARE(result.type->toString(), QString("int"));
  
  release(top);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TestExpressionParser::testSimpleExpression() {
  TEST_FILE_PARSE_ONLY
      
  QByteArray test = "struct Cont { int& a; Cont* operator -> () {}; double operator*(); }; Cont c; Cont* d = &c; void test() { c.a = 5; d->a = 5; (*d).a = 5; c.a(5, 1, c); c.b<Fulli>(); }";
  DUContext* c = parse( test, DumpNone /*DumpDUChain | DumpAST */);
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

  Cpp::ExpressionEvaluationResult result = parser.evaluateType( "c.a", testContext );
  lock.lock();
  QVERIFY(result.isValid());   
  QVERIFY(result.instance);
  QVERIFY(result.type);
  lock.unlock();

  result = parser.evaluateType( "d", testContext );
  lock.lock();
  QVERIFY(result.isValid());
  QVERIFY(result.instance);
  QCOMPARE(result.type->toString(), QString("Cont*"));
  QCOMPARE(dynamic_cast<CppPointerType*>(result.type.data())->baseType()->toString(), QString("Cont"));
  lock.unlock();
  
  //Test pointer-referencing
  result = parser.evaluateType( "&c.a", testContext );
  lock.lock();
  QVERIFY(result.isValid());
  QCOMPARE(result.type->toString(), QString("int*"));
  QVERIFY(result.instance);
  lock.unlock();

  //Test pointer-referencing and dereferencing
  result = parser.evaluateType( "*(&c.a)", testContext );
  lock.lock();
  QVERIFY(result.isValid());
  QCOMPARE(result.type->toString(), QString("int"));
  QVERIFY(result.instance);
  lock.unlock();
  
  //Test overloaded "operator*"
  result = parser.evaluateType( "*c", testContext );
  lock.lock();
  QVERIFY(result.isValid());
  QCOMPARE(result.type->toString(), QString("double"));
  QVERIFY(result.instance);
  lock.unlock();

  //Test overloaded "operator->"
  result = parser.evaluateType( "c->a", testContext );
  lock.lock();
  QVERIFY(result.isValid());
  QCOMPARE(result.type->toString(), QString("int&"));
  QVERIFY(result.instance);
  lock.unlock();

  //Test normal pointer-access + assign expression
  result = parser.evaluateType( "d->a = 5", testContext );
  lock.lock();
  QVERIFY(result.isValid());
  QCOMPARE(result.type->toString(), QString("int&"));
  QVERIFY(result.instance);
  lock.unlock();
  
  //Test double * (one real, one overloaded)
  result = parser.evaluateType( "**d", testContext );
  lock.lock();
  QVERIFY(result.isValid());
  QCOMPARE(result.type->toString(), QString("double"));
  QVERIFY(result.instance);
  lock.unlock();

  //Test double &
  result = parser.evaluateType( "&d", testContext );
  lock.lock();
  QVERIFY(result.isValid());
  QCOMPARE(result.type->toString(), QString("Cont**"));
  QVERIFY(result.instance);
  lock.unlock();
  
  //Test type-expression
  result = parser.evaluateType( "Cont", testContext );
  lock.lock();
  QVERIFY(result.isValid());
  QVERIFY(!result.instance);
  QVERIFY(result.type);
  lock.unlock();

  //Test conditional expression
  result = parser.evaluateType( "a ? c.a : c.a", testContext );
  lock.lock();
  QVERIFY(result.isValid());
  QCOMPARE(result.type->toString(), QString("int&"));
  QVERIFY(result.instance);
  lock.unlock();
  

  lock.lock();
  release(c);
}

void TestExpressionParser::testBaseClasses() {
  TEST_FILE_PARSE_ONLY
}

void TestExpressionParser::testTypeConversion() {
  TEST_FILE_PARSE_ONLY
      
  QByteArray test = "struct Cont { operator int() {}; }; void test( int c = 5 ) { this->test( Cont(), 1, 5.5, 6); }";
  DUContext* c = parse( test, DumpNone /*DumpDUChain | DumpAST */);
  DUChainWriteLocker lock(DUChain::lock());
  
  DUContext* testContext = c->childContexts()[1];
  QCOMPARE( testContext->type(), DUContext::Function );
  
  DUContext* contContext = c->childContexts()[0];
  Declaration* decl = contContext->localDeclarations()[0];

  QCOMPARE(decl->identifier(), Identifier("operator{...cast...}"));
  CppFunctionType* function = dynamic_cast<CppFunctionType*>(decl->abstractType().data());
  QCOMPARE(function->returnType()->toString(), QString("int"));

  Declaration* testDecl = c->localDeclarations()[1];
  AbstractFunctionDeclaration* functionDecl = dynamic_cast<AbstractFunctionDeclaration*>(testDecl);
  QVERIFY(functionDecl);

  QVERIFY(functionDecl->defaultParameters().size() == 1);
  QCOMPARE(functionDecl->defaultParameters()[0], QString("5"));
  
  //QVERIFY(0);
  //lock.lock();
  release(c);
}

void TestExpressionParser::testCasts() {
  TEST_FILE_PARSE_ONLY
      
  QByteArray test = "struct Cont2 {}; struct Cont { int& a; Cont* operator -> () {}; double operator*(); }; Cont c; Cont* d = &c; void test() { c.a = 5; d->a = 5; (*d).a = 5; c.a(5, 1, c); c(); c.a = dynamic_cast<const Cont2*>(d); }";
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
  Cpp::ExpressionEvaluationResult result = parser.evaluateType( "dynamic_cast<Cont2*>(d)", testContext );
  lock.lock();
  QVERIFY(result.isValid());
  QVERIFY(result.instance);
  QVERIFY(result.type);
  QCOMPARE(result.type->toString(), QString("Cont2*"));
  lock.unlock();

  result = parser.evaluateType( "static_cast<Cont2*>(d)", testContext );
  lock.lock();
  QVERIFY(result.isValid());
  QVERIFY(result.instance);
  QVERIFY(result.type);
  QCOMPARE(result.type->toString(), QString("Cont2*"));
  lock.unlock();
  
  result = parser.evaluateType( "reinterpret_cast<Cont2*>(d)", testContext );
  lock.lock();
  QVERIFY(result.isValid());
  QVERIFY(result.instance);
  QVERIFY(result.type);
  QCOMPARE(result.type->toString(), QString("Cont2*"));
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

  Cpp::EnvironmentFilePointer file( new Cpp::EnvironmentFile( url, 0 ) );
  TopDUContext* top = definitionBuilder.buildDeclarations(file, ast);

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

#include "test_expressionparser.moc"
