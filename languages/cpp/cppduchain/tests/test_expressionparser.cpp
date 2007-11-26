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

#include <typeinfo>

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
    kDebug(9007) << "expression-result for";
    DUChainReadLocker lock( DUChain::lock() );
    d.dump( node, session() );
    kDebug(9007) << "is:" << type->toString();
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

void TestExpressionParser::testTemplatesSimple() {
  QByteArray method("template<class T> T test(const T& t) {}; template<class T, class T2> class A { }; class B{}; class C{}; typedef A<B,C> B;");

  DUContext* top = parse(method, DumpNone);

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

  release(top);
}

void TestExpressionParser::testTemplates() {
  QByteArray method("class A{}; template<class T, int val> struct Container {typedef Container<T> SelfType; enum{ Value = val }; T member; T operator*() const {}; }; Container<A> c;");

  DUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());

  Declaration* defClassA = top->localDeclarations()[0];
  QCOMPARE(defClassA->identifier(), Identifier("A"));
  QVERIFY(defClassA->type<CppClassType>());

  Declaration* container = top->localDeclarations()[1];
  QCOMPARE(container->identifier(), Identifier("Container"));
  QVERIFY(container->type<CppClassType>());

  Cpp::TemplateDeclaration* templateDecl = dynamic_cast<Cpp::TemplateDeclaration*>(container);
  QVERIFY(templateDecl);

  
  Cpp::ExpressionParser parser;
  
  {
    Cpp::ExpressionEvaluationResult result = parser.evaluateExpression("c.member", KDevelop::DUContextPointer(top));

    QVERIFY(result.isValid());
    QVERIFY(dynamic_cast<CppClassType*>(result.type.data()));

    QVERIFY(!result.allDeclarations.isEmpty());
    QVERIFY(dynamic_cast<IdentifiedType*>(result.type.data()));
    QCOMPARE(dynamic_cast<IdentifiedType*>(result.type.data())->declaration(), defClassA);
  }
  
  {
    Cpp::ExpressionEvaluationResult result = parser.evaluateExpression("*c", KDevelop::DUContextPointer(top));

    QVERIFY(result.isValid());
    QVERIFY(dynamic_cast<CppClassType*>(result.type.data()));

    QVERIFY(!result.allDeclarations.isEmpty());
    QVERIFY(dynamic_cast<IdentifiedType*>(result.type.data()));
    QCOMPARE(dynamic_cast<IdentifiedType*>(result.type.data())->declaration(), defClassA);
  }
  
  release(top);
}

void TestExpressionParser::testArray() {
  QByteArray method("struct Bla { int val; } blaArray[5+3];");

  DUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());

  Cpp::ExpressionParser parser;
  
  {
    Cpp::ExpressionEvaluationResult result = parser.evaluateExpression("Bla", KDevelop::DUContextPointer(top));

    QVERIFY(result.isValid());
    QVERIFY(dynamic_cast<CppClassType*>(result.type.data()));

    QCOMPARE(result.toString(), QString("Bla"));
  }
  
  {
    Cpp::ExpressionEvaluationResult result = parser.evaluateExpression("blaArray", KDevelop::DUContextPointer(top));

    QVERIFY(result.isValid());
    QVERIFY(result.instance);
    kDebug() << result.toString();
    kDebug() << typeid(*result.type.data()).name();
    QVERIFY(dynamic_cast<CppArrayType*>(result.type.data()));
    QCOMPARE(static_cast<CppArrayType*>(result.type.data())->dimension(), 8);
  }
  
  {
    Cpp::ExpressionEvaluationResult result = parser.evaluateExpression("blaArray[5].val", KDevelop::DUContextPointer(top));

    QVERIFY(result.isValid());
    QVERIFY(dynamic_cast<CppIntegralType*>(result.type.data()));
  }

  release(top);
}

void TestExpressionParser::testDynamicArray() {
  QByteArray method("struct Bla { int val; } blaArray[] = { {5} };");

  DUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());

  Cpp::ExpressionParser parser;
  
  {
    Cpp::ExpressionEvaluationResult result = parser.evaluateExpression("Bla", KDevelop::DUContextPointer(top));

    QVERIFY(result.isValid());
    QVERIFY(dynamic_cast<CppClassType*>(result.type.data()));

    QCOMPARE(result.toString(), QString("Bla"));
  }
  
  {
    Cpp::ExpressionEvaluationResult result = parser.evaluateExpression("blaArray", KDevelop::DUContextPointer(top));

    QVERIFY(result.isValid());
    QVERIFY(result.instance);
    kDebug() << result.toString();
    kDebug() << typeid(*result.type.data()).name();
    QVERIFY(dynamic_cast<CppArrayType*>(result.type.data()));
  }
  
  {
    Cpp::ExpressionEvaluationResult result = parser.evaluateExpression("blaArray[5].val", KDevelop::DUContextPointer(top));

    QVERIFY(result.isValid());
    QVERIFY(dynamic_cast<CppIntegralType*>(result.type.data()));
  }

  release(top);
}

void TestExpressionParser::testSmartPointer() {
  QByteArray method("template<class T> struct SmartPointer { T* operator ->() const {}; template<class Target> SmartPointer<Target> cast() {}; T& operator*() {};  } ; class B{int i;}; class C{}; SmartPointer<B> bPointer;");
  //QByteArray method("template<class T> struct SmartPointer { template<class Target> void cast() {}; } ; ");

  DUContext* top = parse(method, DumpNone);
  
  DUChainWriteLocker lock(DUChain::lock());

  Cpp::ExpressionParser parser;

  QVERIFY(top->localDeclarations()[0]->internalContext());
  
  IdentifiedType* idType = dynamic_cast<IdentifiedType*>(top->localDeclarations()[3]->abstractType().data());
  QVERIFY(idType);
  QCOMPARE(idType->declaration()->context(), KDevelop::DUContextPointer(top).data());
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
  kDebug(9007) << typeid(baseTemplateContext).name();
  kDebug(9007) << typeid(specialTemplateContext).name();
/*  CppDUContext<DUContext>* baseTemplateCtx = dynamic_cast<CppDUContext<DUContext>*>(baseTemplateContext);
  CppDUContext<DUContext>* specialTemplateCtx = dynamic_cast<CppDUContext<DUContext>*>(specialTemplateContext);
  QVERIFY(baseTemplateCtx);
  QVERIFY(specialTemplateCtx);
  QCOMPARE(specialTemplateCtx->instantiatedFrom(), baseTemplateContext);*/
  
  QCOMPARE(specialTemplateContext->localDeclarations().count(), 1);
  QCOMPARE(baseTemplateContext->localDeclarations().count(), 1);
  QVERIFY(specialTemplateContext->localDeclarations()[0] != baseTemplateContext->localDeclarations()[0]);
  
  kDebug(9007) << top->localDeclarations()[3]->abstractType()->toString();
  Cpp::ExpressionEvaluationResult result = parser.evaluateExpression( "*bPointer", KDevelop::DUContextPointer(top));
  QVERIFY(result.instance);
  QCOMPARE(result.type->toString(), QString("B&"));

  result = parser.evaluateExpression( "bPointer->i", KDevelop::DUContextPointer(top));
  QVERIFY(result.instance);
  QCOMPARE(result.type->toString(), QString("int"));
  
/*  result = parser.evaluateExpression( "bPointer->cast<B>()->i", KDevelop::DUContextPointer(top));
  QVERIFY(result.isValid());
  QVERIFY(result.instance);
  QCOMPARE(result.type->toString(), QString("int"));*/
  
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

  kDebug(9007) << "test-Context:" << testContext;
  lock.unlock();

  Cpp::ExpressionParser parser;
  Cpp::ExpressionEvaluationResult result;

  result = parser.evaluateType( "const Cont", KDevelop::DUContextPointer(testContext));
  lock.lock();
  QVERIFY(result.isValid());
  QCOMPARE(result.type->toString(), QString("Cont")); ///@todo Change this to "const Cont" once we parse const correctly
  QVERIFY(!result.instance);
  lock.unlock();
  
  result = parser.evaluateExpression( "Cont", KDevelop::DUContextPointer(testContext));
  lock.lock();
  QVERIFY(result.isValid());
  QCOMPARE(result.type->toString(), QString("Cont"));
  QVERIFY(!result.instance);
  lock.unlock();

  result = parser.evaluateExpression( "c.a", KDevelop::DUContextPointer(testContext));
  lock.lock();
  QVERIFY(result.isValid());   
  QVERIFY(result.instance);
  QVERIFY(result.type);
  lock.unlock();

  result = parser.evaluateExpression( "d", KDevelop::DUContextPointer(testContext));
  lock.lock();
  QVERIFY(result.isValid());
  QVERIFY(result.instance);
  QCOMPARE(result.type->toString(), QString("Cont*"));
  QCOMPARE(dynamic_cast<CppPointerType*>(result.type.data())->baseType()->toString(), QString("Cont"));
  lock.unlock();
  
  //Test pointer-referencing
  result = parser.evaluateExpression( "&c.a", KDevelop::DUContextPointer(testContext));
  lock.lock();
  QVERIFY(result.isValid());
  QCOMPARE(result.type->toString(), QString("int*"));
  QVERIFY(result.instance);
  lock.unlock();

  //Test pointer-referencing and dereferencing
  result = parser.evaluateExpression( "*(&c.a)", KDevelop::DUContextPointer(testContext));
  lock.lock();
  QVERIFY(result.isValid());
  QCOMPARE(result.type->toString(), QString("int"));
  QVERIFY(result.instance);
  lock.unlock();
  
  //Test overloaded "operator*"
  result = parser.evaluateExpression( "*c", KDevelop::DUContextPointer(testContext));
  lock.lock();
  QVERIFY(result.isValid());
  QCOMPARE(result.type->toString(), QString("double"));
  QVERIFY(result.instance);
  lock.unlock();

  //Test overloaded "operator->"
  result = parser.evaluateExpression( "c->a", KDevelop::DUContextPointer(testContext));
  lock.lock();
  QVERIFY(result.isValid());
  QCOMPARE(result.type->toString(), QString("int&"));
  QVERIFY(result.instance);
  lock.unlock();

  //Test normal pointer-access + assign expression
  result = parser.evaluateExpression( "d->a = 5", KDevelop::DUContextPointer(testContext));
  lock.lock();
  QVERIFY(result.isValid());
  QCOMPARE(result.type->toString(), QString("int&"));
  QVERIFY(result.instance);
  lock.unlock();
  
  //Test double * (one real, one overloaded)
  result = parser.evaluateExpression( "**d", KDevelop::DUContextPointer(testContext));
  lock.lock();
  QVERIFY(result.isValid());
  QCOMPARE(result.type->toString(), QString("double"));
  QVERIFY(result.instance);
  lock.unlock();

  //Test double &
  result = parser.evaluateExpression( "&d", KDevelop::DUContextPointer(testContext));
  lock.lock();
  QVERIFY(result.isValid());
  QCOMPARE(result.type->toString(), QString("Cont**"));
  QVERIFY(result.instance);
  lock.unlock();
  
  //Test type-expression
  result = parser.evaluateExpression( "Cont", KDevelop::DUContextPointer(testContext));
  lock.lock();
  QVERIFY(result.isValid());
  QVERIFY(!result.instance);
  QVERIFY(result.type);
  lock.unlock();

  //Test conditional expression
  result = parser.evaluateExpression( "d ? c.a : c.a", KDevelop::DUContextPointer(testContext));
  lock.lock();
  QVERIFY(result.isValid());
  QCOMPARE(result.type->toString(), QString("int&"));
  QVERIFY(result.instance);
  lock.unlock();
  
  result = parser.evaluateExpression( "\"hello\"", KDevelop::DUContextPointer(testContext));
  lock.lock();
  QVERIFY(result.isValid());
  QCOMPARE(result.type->toString().trimmed(), QString("char* const").trimmed());
  QVERIFY(result.instance);
  lock.unlock();

  result = parser.evaluateExpression( "sizeof(Cont)", KDevelop::DUContextPointer(testContext));
  lock.lock();
  QVERIFY(result.isValid());
  QCOMPARE(result.type->toString(), QString("int"));
  QVERIFY(result.instance);
  lock.unlock();
  
  result = parser.evaluateExpression( "new Cont()", KDevelop::DUContextPointer(testContext));
  lock.lock();
  QVERIFY(result.isValid());
  QCOMPARE(result.type->toString(), QString("Cont*"));
  QVERIFY(result.instance);
  lock.unlock();
  
  result = parser.evaluateExpression( "5", KDevelop::DUContextPointer(testContext));
  lock.lock();
  QVERIFY(result.isValid());
  QCOMPARE(result.type->toString(), QString("5"));
  QVERIFY(result.instance);
  QVERIFY(dynamic_cast<CppConstantIntegralType*>(result.type.data()));
  lock.unlock();
  QVERIFY(!TypeUtils::isNullType(result.type.data()));
  
  result = parser.evaluateExpression( "5.5", KDevelop::DUContextPointer(testContext));
  lock.lock();
  QVERIFY(result.isValid());
  QCOMPARE(result.type->toString(), QString("5.5"));
  QVERIFY(result.instance);
  QVERIFY(dynamic_cast<CppConstantIntegralType*>(result.type.data()));
  lock.unlock();
  QVERIFY(!TypeUtils::isNullType(result.type.data()));
  
  result = parser.evaluateExpression( "0", KDevelop::DUContextPointer(testContext));
  lock.lock();
  QVERIFY(result.isValid());
  QCOMPARE(result.type->toString(), QString("0"));
  QVERIFY(result.instance);
  QVERIFY(dynamic_cast<CppConstantIntegralType*>(result.type.data()));
  lock.unlock();
  QVERIFY(TypeUtils::isNullType(result.type.data()));
  
  lock.lock();
  release(c);
}

void TestExpressionParser::testThis() {
  TEST_FILE_PARSE_ONLY
  
  QByteArray text("class A{ void test() { } void test2() const { }; void extTest(); }; void A::extTest() {}");
  DUContext* top = parse( text, DumpNone);//DumpNone /*DumpDUChain | DumpAST */);
  DUChainWriteLocker lock(DUChain::lock());
  
  QCOMPARE(top->childContexts().count(), 3);
  QCOMPARE(top->childContexts()[0]->type(), DUContext::Class);
  QCOMPARE(top->childContexts()[0]->childContexts().count(), 5);
  DUContext* testContext = top->childContexts()[0]->childContexts()[1];
  QCOMPARE(testContext->type(), DUContext::Other);
  QVERIFY(testContext->owner());
  
  DUContext* test2Context = top->childContexts()[0]->childContexts()[3];
  QCOMPARE(test2Context->type(), DUContext::Other);
  QVERIFY(test2Context->owner());

  Cpp::ExpressionParser parser;
  Cpp::ExpressionEvaluationResult result1 = parser.evaluateExpression( "this", KDevelop::DUContextPointer(testContext));
  QVERIFY(result1.isValid());
  QVERIFY(result1.type);
  QVERIFY(result1.instance);
  QCOMPARE(result1.type->toString(), QString("A*"));

  
  Cpp::ExpressionEvaluationResult result2 = parser.evaluateExpression( "this", KDevelop::DUContextPointer(test2Context));
  QVERIFY(result2.isValid());
  QVERIFY(result2.type);
  QVERIFY(result2.instance);
  QCOMPARE(result2.type->toString().trimmed(), QString("A* const"));

  DUContext* extTestCtx = top->childContexts()[2];
  QCOMPARE(extTestCtx->type(), DUContext::Other);
  QVERIFY(extTestCtx->owner());
  QVERIFY(extTestCtx->owner()->asDefinition());
  
  Cpp::ExpressionEvaluationResult result3 = parser.evaluateExpression( "this", KDevelop::DUContextPointer(extTestCtx));
  QVERIFY(result3.isValid());
  QVERIFY(result3.type);
  QVERIFY(result3.instance);
  QCOMPARE(result3.type->toString(), QString("A*"));
  
  release(top);
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
  DUContext* c = parse( test, DumpNone );
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
  
  Cpp::ExpressionEvaluationResult result = parser.evaluateExpression( "(Cont2*)(d)", KDevelop::DUContextPointer(testContext));
  lock.lock();
  QVERIFY(result.isValid());
  QVERIFY(result.instance);
  QVERIFY(result.type);
  QCOMPARE(result.type->toString(), QString("Cont2*"));
  lock.unlock();
  
  result = parser.evaluateExpression( "dynamic_cast<Cont2*>(d)", KDevelop::DUContextPointer(testContext));
  lock.lock();
  QVERIFY(result.isValid());
  QVERIFY(result.instance);
  QVERIFY(result.type);
  QCOMPARE(result.type->toString(), QString("Cont2*"));
  lock.unlock();

  result = parser.evaluateExpression( "static_cast<Cont2*>(d)", KDevelop::DUContextPointer(testContext));
  lock.lock();
  QVERIFY(result.isValid());
  QVERIFY(result.instance);
  QVERIFY(result.type);
  QCOMPARE(result.type->toString(), QString("Cont2*"));
  lock.unlock();
  
  result = parser.evaluateExpression( "reinterpret_cast<Cont2*>(d)", KDevelop::DUContextPointer(testContext));
  lock.lock();
  QVERIFY(result.isValid());
  QVERIFY(result.instance);
  QVERIFY(result.type);
  QCOMPARE(result.type->toString(), QString("Cont2*"));
  lock.unlock();
  
  result = parser.evaluateExpression( "new Cont2*", KDevelop::DUContextPointer(testContext));
  lock.lock();
  QVERIFY(result.isValid());
  QVERIFY(result.instance);
  QVERIFY(result.type);
  QCOMPARE(result.type->toString(), QString("Cont2**"));
  lock.unlock();
  
  result = parser.evaluateExpression( "new Cont2", KDevelop::DUContextPointer(testContext));
  lock.lock();
  QVERIFY(result.isValid());
  QVERIFY(result.instance);
  QVERIFY(result.type);
  QCOMPARE(result.type->toString(), QString("Cont2*"));
  lock.unlock();
  
  result = parser.evaluateExpression( "new Cont2[5]", KDevelop::DUContextPointer(testContext));
  lock.lock();
  QVERIFY(result.isValid());
  QVERIFY(result.instance);
  QVERIFY(result.type);
  QCOMPARE(result.type->toString(), QString("Cont2*"));
  lock.unlock();
  
  result = parser.evaluateExpression( "(int*)new Cont2[5]", KDevelop::DUContextPointer(testContext));
  lock.lock();
  QVERIFY(result.isValid());
  QVERIFY(result.instance);
  QVERIFY(result.type);
  QCOMPARE(result.type->toString(), QString("int*"));
  lock.unlock();
  
  lock.lock();
  release(c);
}

void TestExpressionParser::testOperators() {
  TEST_FILE_PARSE_ONLY
      
  QByteArray test = "struct Cont2 {operator[] {}; operator()() {};}; struct Cont3{}; struct Cont { Cont3 operator[](int i) {} Cont3 operator()() {} Cont3 operator+(const Cont3& c3 ) {} }; Cont c; Cont2 operator+( const Cont& c, const Cont& c2){} Cont3 c3;";
  DUContext* ctx = parse( test, DumpNone );
  DUChainWriteLocker lock(DUChain::lock());
  
  QCOMPARE(ctx->childContexts().count(), 5);
  QCOMPARE(ctx->localDeclarations().count(), 6);

  Declaration* cont2Dec = ctx->localDeclarations()[0];
  Declaration* cont3Dec = ctx->localDeclarations()[1];
  Declaration* contDec = ctx->localDeclarations()[2];
  Declaration* cDec = ctx->localDeclarations()[3];
  Declaration* plusDec = ctx->localDeclarations()[4];

  QVERIFY(cont2Dec->internalContext());
  QVERIFY(cont3Dec->internalContext());
  QVERIFY(contDec->internalContext());
  QVERIFY(!cDec->internalContext());
  QVERIFY(plusDec->internalContext());

  QVERIFY(findDeclaration(ctx, QualifiedIdentifier("Cont::operator()")));
  QVERIFY(findDeclaration(ctx, QualifiedIdentifier("Cont::operator()"))->type<CppFunctionType>());
  
  CppClassType::Ptr cont2 = cont2Dec->type<CppClassType>();
  CppClassType::Ptr cont3 = cont3Dec->type<CppClassType>();
  CppClassType::Ptr cont = contDec->type<CppClassType>();
  QVERIFY(cont2);
  QVERIFY(cont3);
  QVERIFY(cont);

  lock.unlock();

  Cpp::ExpressionParser parser;

  //Reenable this once the type-parsing system etc. is fixed
  
  Cpp::ExpressionEvaluationResult result = parser.evaluateExpression( "c+c", KDevelop::DUContextPointer(ctx));
  lock.lock();
  QVERIFY(result.isValid());
  QVERIFY(result.instance);
  QVERIFY(result.type);
  QCOMPARE(result.type->toString(), QString("Cont2"));
  lock.unlock();
  
  result = parser.evaluateExpression( "c+c3", KDevelop::DUContextPointer(ctx));
  lock.lock();
  QVERIFY(result.isValid());
  QVERIFY(result.instance);
  QVERIFY(result.type);
  QCOMPARE(result.type->toString(), QString("Cont3"));
  lock.unlock();
  
  result = parser.evaluateExpression( "c()", KDevelop::DUContextPointer(ctx));
  lock.lock();
  QVERIFY(result.isValid());
  QVERIFY(result.instance);
  QVERIFY(result.type);
  QCOMPARE(result.type->toString(), QString("Cont3"));
  lock.unlock();
  
  result = parser.evaluateExpression( "c[5]", KDevelop::DUContextPointer(ctx));
  lock.lock();
  QVERIFY(result.isValid());
  QVERIFY(result.instance);
  QVERIFY(result.type);
  QCOMPARE(result.type->toString(), QString("Cont3"));
  lock.unlock();

  //A simple test: Constructing a type should always result in the type, no matter whether there is a constructor.
  result = parser.evaluateExpression( "Cont(5)", KDevelop::DUContextPointer(ctx));
  lock.lock();
  QVERIFY(result.isValid());
  QVERIFY(result.instance);
  QVERIFY(result.type);
  QCOMPARE(result.type->toString(), QString("Cont"));
  lock.unlock();
  
  lock.lock();
  release(ctx);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TestExpressionParser::release(DUContext* top)
{
  KDevelop::EditorIntegrator::releaseTopRange(top->textRangePtr());
  if(dynamic_cast<TopDUContext*>(top))
    DUChain::self()->removeDocumentChain(static_cast<TopDUContext*>(top)->identity());
  //delete top;
}

DUContext* TestExpressionParser::parse(const QByteArray& unit, DumpAreas dump)
{
  if (dump)
    kDebug(9007) << "==== Beginning new test case...:" << endl << unit;

  ParseSession* session = new ParseSession();
  session->setContentsAndGenerateLocationTable(unit);

  Parser parser(&control);
  TranslationUnitAST* ast = parser.parse(session);
  ast->session = session;

  if (dump & DumpAST) {
    kDebug(9007) << "===== AST:";
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

  if (dump)
    kDebug(9007) << "===== Finished test case.";

  delete session;

  return top;
}

#include "test_expressionparser.moc"
