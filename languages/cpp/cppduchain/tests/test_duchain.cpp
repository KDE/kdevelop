/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>

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

#include <QtTest/QtTest>

#include <duchain.h>
#include <duchainlock.h>
#include <topducontext.h>
#include <forwarddeclaration.h>
#include "declarationbuilder.h"
#include "usebuilder.h"
#include <declaration.h>
#include <definition.h>
#include <dumpdotgraph.h>
#include <time.h>
#include <set>
#include <algorithm>
#include <iterator>
#include <time.h>
#include "cpptypes.h"
#include "templateparameterdeclaration.h"
#include <documentrange.h>
#include "cppeditorintegrator.h"
#include "dumptypes.h"
#include "environmentmanager.h"
#include "setrepository.h"
#include "hashedstring.h"
#include "typeutils.h"
#include "templatedeclaration.h"

#include "tokens.h"
#include "parsesession.h"
#include <symboltable.h>

#include "rpp/preprocessor.h"

//Uncomment the following line to get additional output from the string-repository test
//#define DEBUG_STRINGREPOSITORY

using namespace KTextEditor;
using namespace TypeUtils;
using namespace KDevelop;
using namespace Cpp;
using namespace Utils;

QTEST_MAIN(TestDUChain)

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

Declaration* getDeclaration( AbstractType::Ptr base ) {
  if( !base ) return 0;

  IdentifiedType* idType = dynamic_cast<IdentifiedType*>(base.data());
  if( idType ) {
    return idType->declaration();
  } else {
    return 0;
  }
}


#define TEST_FILE_PARSE_ONLY if (testFileParseOnly) QSKIP("Skip", SkipSingle);
TestDUChain::TestDUChain()
{
  testFileParseOnly = false;
}

void TestDUChain::initTestCase()
{
  noDef = 0;

  file1 = "file:///media/data/kdedev/4.0/kdevelop/languages/cpp/parser/duchain.cpp";
  file2 = "file:///media/data/kdedev/4.0/kdevelop/languages/cpp/parser/dubuilder.cpp";

  topContext = new TopDUContext(file1.prettyUrl(), new KDevelop::DocumentRange(file1.prettyUrl(), Range(0,0,25,0)));
  DUChainWriteLocker lock(DUChain::lock());
  
  DUChain::self()->addDocumentChain(IdentifiedFile(file1.prettyUrl()), topContext);

  typeVoid = AbstractType::Ptr::staticCast(TypeRepository::self()->integral(CppIntegralType::TypeVoid));
  typeInt = AbstractType::Ptr::staticCast(TypeRepository::self()->integral(CppIntegralType::TypeInt));
  typeShort = AbstractType::Ptr::staticCast(TypeRepository::self()->integral(CppIntegralType::TypeInt, CppIntegralType::ModifierShort));
}

void TestDUChain::cleanupTestCase()
{
  /*delete type1;
  delete type2;
  delete type3;*/

  DUChainWriteLocker lock(DUChain::lock());

  KDevelop::EditorIntegrator::releaseTopRange(topContext->textRangePtr());
  delete topContext;
}

Declaration* TestDUChain::findDeclaration(DUContext* context, const Identifier& id, const Cursor& position)
{
  QList<Declaration*> ret = context->findDeclarations(id, position);
  if (ret.count())
    return ret.first();
  return 0;
}

Declaration* TestDUChain::findDeclaration(DUContext* context, const QualifiedIdentifier& id, const Cursor& position)
{
  QList<Declaration*> ret = context->findDeclarations(id, position);
  if (ret.count())
    return ret.first();
  return 0;
}

void TestDUChain::testIdentifiers()
{
  TEST_FILE_PARSE_ONLY

  QualifiedIdentifier aj("::Area::jump");
  QCOMPARE(aj.count(), 2);
  QCOMPARE(aj.explicitlyGlobal(), true);
  QCOMPARE(aj.at(0), Identifier("Area"));
  QCOMPARE(aj.at(1), Identifier("jump"));

  QualifiedIdentifier aj2 = QualifiedIdentifier("Area::jump");
  QCOMPARE(aj2.count(), 2);
  QCOMPARE(aj2.explicitlyGlobal(), false);
  QCOMPARE(aj2.at(0), Identifier("Area"));
  QCOMPARE(aj2.at(1), Identifier("jump"));

  QCOMPARE(aj == aj2, true);

  QCOMPARE(aj.match(aj2), QualifiedIdentifier::ExactMatch);

  QualifiedIdentifier ajt("Area::jump::test");
  QualifiedIdentifier jt("jump::test");
  QualifiedIdentifier ajt2("Area::jump::tes");

  QCOMPARE(aj2.match(ajt), QualifiedIdentifier::NoMatch);
  QCOMPARE(ajt.match(aj2), QualifiedIdentifier::NoMatch);
  //QCOMPARE(jt.match(aj2), QualifiedIdentifier::ContainedBy); ///@todo reenable this(but it fails)
  QCOMPARE(ajt.match(jt), QualifiedIdentifier::EndsWith);

  QCOMPARE(aj2.match(ajt2), QualifiedIdentifier::NoMatch);
  QCOMPARE(ajt2.match(aj2), QualifiedIdentifier::NoMatch);
  QualifiedIdentifier t("Area<A,B>::jump<F>::tes<C>");
  QCOMPARE(t.count(), 3);
  QCOMPARE(t.at(0).templateIdentifiers().count(), 2);
  QCOMPARE(t.at(1).templateIdentifiers().count(), 1);
  QCOMPARE(t.at(2).templateIdentifiers().count(), 1);
  QCOMPARE(t.at(0).identifier(), QString("Area"));
  QCOMPARE(t.at(1).identifier(), QString("jump"));
  QCOMPARE(t.at(2).identifier(), QString("tes"));

  QualifiedIdentifier op1("operator<");
  QualifiedIdentifier op2("operator<=");
  QualifiedIdentifier op3("operator>");
  QualifiedIdentifier op4("operator>=");
  QualifiedIdentifier op5("operator()");
  QualifiedIdentifier op6("operator( )");
  QCOMPARE(op1.count(), 1);
  QCOMPARE(op2.count(), 1);
  QCOMPARE(op3.count(), 1);
  QCOMPARE(op4.count(), 1);
  QCOMPARE(op5.count(), 1);
  QCOMPARE(op6.count(), 1);
  QCOMPARE(op4.toString(), QString("operator>="));
  QCOMPARE(op3.toString(), QString("operator>"));
  QCOMPARE(op1.toString(), QString("operator<"));
  QCOMPARE(op2.toString(), QString("operator<="));
  QCOMPARE(op5.toString(), QString("operator()"));
  QCOMPARE(op6.toString(), QString("operator( )"));
}

void TestDUChain::testContextRelationships()
{
  TEST_FILE_PARSE_ONLY

  QCOMPARE(DUChain::self()->chainForDocument(file1), topContext);

  DUChainWriteLocker lock(DUChain::lock());

  DUContext* firstChild = new DUContext(file1.prettyUrl(), new KDevelop::DocumentRange(file1.prettyUrl(), Range(4,4, 10,3)), topContext);

  QCOMPARE(firstChild->parentContext(), topContext);
  QCOMPARE(firstChild->childContexts().count(), 0);
  QCOMPARE(topContext->childContexts().count(), 1);
  QCOMPARE(topContext->childContexts().last(), firstChild);

  DUContext* secondChild = new DUContext(file1.prettyUrl(), new KDevelop::DocumentRange(file1.prettyUrl(), Range(14,4, 19,3)), topContext);

  QCOMPARE(topContext->childContexts().count(), 2);
  QCOMPARE(topContext->childContexts()[1], secondChild);

  DUContext* thirdChild = new DUContext(file1.prettyUrl(), new KDevelop::DocumentRange(file1.prettyUrl(), Range(10,4, 14,3)), topContext);

  QCOMPARE(topContext->childContexts().count(), 3);
  QCOMPARE(topContext->childContexts()[1], thirdChild);

  topContext->deleteChildContextsRecursively();
  QVERIFY(topContext->childContexts().isEmpty());
}

void TestDUChain::testDeclareInt()
{
  TEST_FILE_PARSE_ONLY

  QByteArray method("int i;");

  DUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());

  QVERIFY(!top->parentContext());
  QCOMPARE(top->childContexts().count(), 0);
  QCOMPARE(top->localDeclarations().count(), 1);
  QVERIFY(top->localScopeIdentifier().isEmpty());

  Declaration* def = top->localDeclarations().first();
  QCOMPARE(def->identifier(), Identifier("i"));
  QCOMPARE(findDeclaration(top, def->identifier()), def);

  release(top);
}

void TestDUChain::testIntegralTypes()
{
  TEST_FILE_PARSE_ONLY

  QByteArray method("const unsigned int i, k; volatile long double j; int* l; double * const * m; const int& n = l;");

  DUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());

  QVERIFY(!top->parentContext());
  QCOMPARE(top->childContexts().count(), 0);
  QCOMPARE(top->localDeclarations().count(), 6);
  QVERIFY(top->localScopeIdentifier().isEmpty());

  Declaration* defI = top->localDeclarations().first();
  QCOMPARE(defI->identifier(), Identifier("i"));
  QCOMPARE(findDeclaration(top, defI->identifier()), defI);
  QVERIFY(defI->type<CppIntegralType>());
  QCOMPARE(defI->type<CppIntegralType>()->integralType(), CppIntegralType::TypeInt);
  QCOMPARE(defI->type<CppIntegralType>()->typeModifiers(), CppIntegralType::ModifierUnsigned);
  QVERIFY(defI->type<CppIntegralType>()->isConstant());
  QVERIFY(!defI->type<CppIntegralType>()->isVolatile());

  Declaration* defK = top->localDeclarations()[1];
  QCOMPARE(defK->identifier(), Identifier("k"));
  QCOMPARE(defK->type<CppIntegralType>(), defI->type<CppIntegralType>());

  Declaration* defJ = top->localDeclarations()[2];
  QCOMPARE(defJ->identifier(), Identifier("j"));
  QCOMPARE(findDeclaration(top, defJ->identifier()), defJ);
  QVERIFY(defJ->type<CppIntegralType>());
  QCOMPARE(defJ->type<CppIntegralType>()->integralType(), CppIntegralType::TypeDouble);
  QCOMPARE(defJ->type<CppIntegralType>()->typeModifiers(), CppIntegralType::ModifierLong);
  QVERIFY(!defJ->type<CppIntegralType>()->isConstant());
  QVERIFY(defJ->type<CppIntegralType>()->isVolatile());

  Declaration* defL = top->localDeclarations()[3];
  QCOMPARE(defL->identifier(), Identifier("l"));
  QVERIFY(defL->type<CppPointerType>());
  QCOMPARE(defL->type<CppPointerType>()->baseType(), typeInt);
  QVERIFY(!defL->type<CppPointerType>()->isConstant());
  QVERIFY(!defL->type<CppPointerType>()->isVolatile());

  Declaration* defM = top->localDeclarations()[4];
  QCOMPARE(defM->identifier(), Identifier("m"));
  CppPointerType::Ptr firstpointer = defM->type<CppPointerType>();
  QVERIFY(firstpointer);
  QVERIFY(!firstpointer->isConstant());
  QVERIFY(!firstpointer->isVolatile());
  CppPointerType::Ptr secondpointer = CppPointerType::Ptr::dynamicCast(firstpointer->baseType());
  QVERIFY(secondpointer);
  QVERIFY(secondpointer->isConstant());
  QVERIFY(!secondpointer->isVolatile());
  CppIntegralType::Ptr base = CppIntegralType::Ptr::dynamicCast(secondpointer->baseType());
  QVERIFY(base);
  QCOMPARE(base->integralType(), CppIntegralType::TypeDouble);
  QVERIFY(!base->isConstant());
  QVERIFY(!base->isVolatile());

  Declaration* defN = top->localDeclarations()[5];
  QCOMPARE(defN->identifier(), Identifier("n"));
  QVERIFY(defN->type<CppReferenceType>());
  base = CppIntegralType::Ptr::dynamicCast(defN->type<CppReferenceType>()->baseType());
  QVERIFY(base);
  QCOMPARE(base->integralType(), CppIntegralType::TypeInt);
  QVERIFY(base->isConstant());
  QVERIFY(!base->isVolatile());

  release(top);
}

void TestDUChain::testArrayType()
{
  TEST_FILE_PARSE_ONLY

  QByteArray method("int i[3];");

  DUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());

  QVERIFY(!top->parentContext());
  QCOMPARE(top->childContexts().count(), 0);
  QCOMPARE(top->localDeclarations().count(), 1);
  QVERIFY(top->localScopeIdentifier().isEmpty());

  Declaration* defI = top->localDeclarations().first();
  QCOMPARE(defI->identifier(), Identifier("i"));
  QCOMPARE(findDeclaration(top, defI->identifier()), defI);

  ArrayType::Ptr array = defI->type<ArrayType>();
  QVERIFY(array);
  CppIntegralType::Ptr element = CppIntegralType::Ptr::dynamicCast(array->elementType());
  QVERIFY(element);
  QCOMPARE(element->integralType(), CppIntegralType::TypeInt);
  QCOMPARE(array->dimension(), 3);

  release(top);
}

void TestDUChain::testTypedefUses()
{
  TEST_FILE_PARSE_ONLY

  //                 0         1         2         3         4         5
  //                 012345678901234567890123456789012345678901234567890123456789
  QByteArray method("class A{}; typedef A B; A c; B d;");

  DUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());

  QCOMPARE(top->localDeclarations().count(), 4);
  QCOMPARE(top->uses().count(), 3);
  QCOMPARE(top->localDeclarations()[0]->uses().count(), 2); //Typedef and "A c;"
  QCOMPARE(top->localDeclarations()[1]->uses().count(), 1); //"B d;"
  
}

void TestDUChain::testDeclareFor()
{
  TEST_FILE_PARSE_ONLY

  //                 0         1         2         3         4         5
  //                 012345678901234567890123456789012345678901234567890123456789
  QByteArray method("int main() { for (int i = 0; i < 10; i++) { if (i == 4) return; int i5[5]; i5[i] = 1; } }");

  DUContext* top = parse(method, DumpAll);

  DUChainWriteLocker lock(DUChain::lock());

  QVERIFY(!top->parentContext());
  QCOMPARE(top->childContexts().count(), 2);
  QCOMPARE(top->localDeclarations().count(), 1);
  QVERIFY(top->localScopeIdentifier().isEmpty());

  Declaration* defMain = top->localDeclarations().first();
  QCOMPARE(defMain->identifier(), Identifier("main"));
  QCOMPARE(findDeclaration(top, defMain->identifier()), defMain);
  QVERIFY(defMain->type<CppFunctionType>());
  QCOMPARE(defMain->type<CppFunctionType>()->returnType(), typeInt);
  QCOMPARE(defMain->type<CppFunctionType>()->arguments().count(), 0);
  QVERIFY(!defMain->type<CppFunctionType>()->isConstant());
  QVERIFY(!defMain->type<CppFunctionType>()->isVolatile());

  QCOMPARE(findDeclaration(top, Identifier("i")), noDef);

  DUContext* main = top->childContexts()[1];
  QVERIFY(main->parentContext());
  QCOMPARE(main->importedParentContexts().count(), 1);
  QCOMPARE(main->childContexts().count(), 2);
  QCOMPARE(main->localDeclarations().count(), 0);
  QVERIFY(main->localScopeIdentifier().isEmpty());

  QCOMPARE(findDeclaration(main, Identifier("i")), noDef);

  DUContext* forCtx = main->childContexts()[1];
  QVERIFY(forCtx->parentContext());
  QCOMPARE(forCtx->importedParentContexts().count(), 1);
  QCOMPARE(forCtx->childContexts().count(), 2);
  QCOMPARE(forCtx->localDeclarations().count(), 1);
  QVERIFY(forCtx->localScopeIdentifier().isEmpty());

  DUContext* forParamCtx = forCtx->importedParentContexts().first().data();
  QVERIFY(forParamCtx->parentContext());
  QCOMPARE(forParamCtx->importedParentContexts().count(), 0);
  QCOMPARE(forParamCtx->childContexts().count(), 0);
  QCOMPARE(forParamCtx->localDeclarations().count(), 1);
  QVERIFY(forParamCtx->localScopeIdentifier().isEmpty());

  Declaration* defI = forParamCtx->localDeclarations().first();
  QCOMPARE(defI->identifier(), Identifier("i"));
  QCOMPARE(defI->uses().count(), 4);

  QCOMPARE(findDeclaration(forCtx, defI->identifier()), defI);

  DUContext* ifCtx = forCtx->childContexts()[1];
  QVERIFY(ifCtx->parentContext());
  QCOMPARE(ifCtx->importedParentContexts().count(), 1);
  QCOMPARE(ifCtx->childContexts().count(), 0);
  QCOMPARE(ifCtx->localDeclarations().count(), 0);
  QVERIFY(ifCtx->localScopeIdentifier().isEmpty());

  QCOMPARE(findDeclaration(ifCtx,  defI->identifier()), defI);

  release(top);
}

void TestDUChain::testEnum()
{
  TEST_FILE_PARSE_ONLY

  //                 0         1         2         3         4         5         6         7
  //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
  QByteArray method("enum Enum { Value1 = 5, value2 }; enum Enum2 { Value21, value22 = 0x02 };");

  DUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());

  QCOMPARE(top->localDeclarations().count(), 6);
  
  Declaration* decl = findDeclaration(top, Identifier("Enum"));
  QVERIFY(decl);
  QVERIFY(dynamic_cast<CppEnumerationType*>(decl->abstractType().data()));
  CppEnumerationType* en = static_cast<CppEnumerationType*>(decl->abstractType().data());
  QVERIFY(en->declaration());
  QCOMPARE(en->identifier(), QualifiedIdentifier("Enum"));

  {
    decl = findDeclaration(top, QualifiedIdentifier("Value1"));
    QVERIFY(decl);
    QVERIFY(dynamic_cast<CppEnumeratorType*>(decl->abstractType().data()));
    CppEnumeratorType* en = static_cast<CppEnumeratorType*>(decl->abstractType().data());
    QCOMPARE((int)en->value<qint64>(), 5);
  
    decl = findDeclaration(top, Identifier("value2"));
    QVERIFY(decl);
    QVERIFY(dynamic_cast<CppEnumeratorType*>(decl->abstractType().data()));
    en = static_cast<CppEnumeratorType*>(decl->abstractType().data());
    QCOMPARE((int)en->value<qint64>(), 6);
  }
  decl = findDeclaration(top, Identifier("Enum2"));
  QVERIFY(decl);
  QVERIFY(dynamic_cast<CppEnumerationType*>(decl->abstractType().data()));
  en = static_cast<CppEnumerationType*>(decl->abstractType().data());
  QVERIFY(en->declaration());
  QCOMPARE(en->identifier(), QualifiedIdentifier("Enum2"));
  
  {
    decl = findDeclaration(top, Identifier("Value21"));
    QVERIFY(decl);
    QVERIFY(dynamic_cast<CppEnumeratorType*>(decl->abstractType().data()));
    CppEnumeratorType* en = static_cast<CppEnumeratorType*>(decl->abstractType().data());
    QCOMPARE((int)en->value<qint64>(), 0);
  
    decl = findDeclaration(top, Identifier("value22"));
    QVERIFY(decl);
    QVERIFY(dynamic_cast<CppEnumeratorType*>(decl->abstractType().data()));
    en = static_cast<CppEnumeratorType*>(decl->abstractType().data());
    QCOMPARE((int)en->value<qint64>(), 2);
  }
  release(top);
}

void TestDUChain::testDeclareStruct()
{
  TEST_FILE_PARSE_ONLY

  //                 0         1         2         3         4         5         6         7
  //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
  QByteArray method("struct A { short i; A(int b, int c) : i(c) { } virtual void test(int j) = 0; }; A instance;");

  DUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());

  QVERIFY(!top->parentContext());
  QCOMPARE(top->childContexts().count(), 1);
  QCOMPARE(top->localDeclarations().count(), 2);
  QVERIFY(top->localScopeIdentifier().isEmpty());

  IdentifiedType* idType = dynamic_cast<IdentifiedType*>(top->localDeclarations()[1]->abstractType().data());
  QVERIFY(idType);
  QCOMPARE( idType->identifier(), QualifiedIdentifier("A") );
  
  Declaration* defStructA = top->localDeclarations().first();
  QCOMPARE(defStructA->identifier(), Identifier("A"));
  QCOMPARE(defStructA->uses().count(), 1);
  QVERIFY(defStructA->type<CppClassType>());
  QCOMPARE(defStructA->type<CppClassType>()->elements().count(), 3);
  QCOMPARE(defStructA->type<CppClassType>()->elements().first(), typeShort);
  QVERIFY(CppFunctionType::Ptr::dynamicCast(defStructA->type<CppClassType>()->elements()[1]));
  QCOMPARE(defStructA->type<CppClassType>()->classType(), CppClassType::Struct);

  DUContext* structA = top->childContexts().first();
  QVERIFY(structA->parentContext());
  QCOMPARE(structA->importedParentContexts().count(), 0);
  QCOMPARE(structA->childContexts().count(), 3);
  QCOMPARE(structA->localDeclarations().count(), 3);
  QCOMPARE(structA->localScopeIdentifier(), QualifiedIdentifier("A"));

  Declaration* defI = structA->localDeclarations().first();
  QCOMPARE(defI->identifier(), Identifier("i"));
  QCOMPARE(defI->uses().count(), 1);

  QCOMPARE(findDeclaration(structA,  Identifier("i")), defI);
  QCOMPARE(findDeclaration(structA,  Identifier("b")), noDef);
  QCOMPARE(findDeclaration(structA,  Identifier("c")), noDef);

  DUContext* ctorImplCtx = structA->childContexts()[1];
  QVERIFY(ctorImplCtx->parentContext());
  QCOMPARE(ctorImplCtx->importedParentContexts().count(), 1);
  QCOMPARE(ctorImplCtx->childContexts().count(), 1);
  QCOMPARE(ctorImplCtx->localDeclarations().count(), 0);
  QVERIFY(ctorImplCtx->localScopeIdentifier().isEmpty());

  DUContext* ctorCtx = ctorImplCtx->importedParentContexts().first().data();
  QVERIFY(ctorCtx->parentContext());
  QCOMPARE(ctorCtx->childContexts().count(), 0);
  QCOMPARE(ctorCtx->localDeclarations().count(), 2);
  QCOMPARE(ctorCtx->localScopeIdentifier(), QualifiedIdentifier("A")); ///@todo check if it should really be this way

  Declaration* defB = ctorCtx->localDeclarations().first();
  QCOMPARE(defB->identifier(), Identifier("b"));
  QCOMPARE(defB->uses().count(), 0);

  Declaration* defC = ctorCtx->localDeclarations()[1];
  QCOMPARE(defC->identifier(), Identifier("c"));
  QCOMPARE(defC->uses().count(), 1);

  QCOMPARE(findDeclaration(ctorCtx,  Identifier("i")), defI);
  QCOMPARE(findDeclaration(ctorCtx,  Identifier("b")), defB);
  QCOMPARE(findDeclaration(ctorCtx,  Identifier("c")), defC);

  DUContext* testCtx = structA->childContexts().last();
  QCOMPARE(testCtx->childContexts().count(), 0);
  QCOMPARE(testCtx->localDeclarations().count(), 1);
  QCOMPARE(testCtx->localScopeIdentifier(), QualifiedIdentifier("test")); ///@todo check if it should really be this way

  Declaration* defJ = testCtx->localDeclarations().first();
  QCOMPARE(defJ->identifier(), Identifier("j"));
  QCOMPARE(defJ->uses().count(), 0);

  /*DUContext* insideCtorCtx = ctorCtx->childContexts().first();
  QCOMPARE(insideCtorCtx->childContexts().count(), 0);
  QCOMPARE(insideCtorCtx->localDeclarations().count(), 0);
  QVERIFY(insideCtorCtx->localScopeIdentifier().isEmpty());*/

  release(top);
}

void TestDUChain::testCStruct()
{
  TEST_FILE_PARSE_ONLY

  //                 0         1         2         3         4         5         6         7
  //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
  QByteArray method("struct A {int i; }; struct A instance; typedef struct { int a; } B, *BPointer;");

  DUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());

  QVERIFY(!top->parentContext());
  QCOMPARE(top->childContexts().count(), 2);
  QCOMPARE(top->localDeclarations().count(), 5);

  QVERIFY(top->localDeclarations()[0]->kind() == Declaration::Type);
  QVERIFY(top->localDeclarations()[1]->kind() == Declaration::Instance);
  
  release(top);
}

void TestDUChain::testCStruct2()
{
  TEST_FILE_PARSE_ONLY

  //                 0         1         2         3         4         5         6         7
  //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
   
  QByteArray method("struct A {struct C c; }; struct C { int v; };");

  /* Expected result: the elaborated type-specifier "struct C" within the declaration "struct A" should create a new global declaration.
  */

  DUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());

  QVERIFY(!top->parentContext());
  QCOMPARE(top->childContexts().count(), 2);
  QCOMPARE(top->localDeclarations().count(), 3); //3 declarations because the elaborated type-specifier "struct C" creates a forward-declaration on the global scope
  QCOMPARE(top->childContexts()[0]->localDeclarations().count(), 1);
  QCOMPARE(top->childContexts()[0]->localDeclarations()[0]->abstractType(), top->localDeclarations()[1]->abstractType());
           
  QCOMPARE(top->childContexts()[0]->localDeclarations()[0]->kind(), Declaration::Instance); /*QVERIFY(top->localDeclarations()[0]->kind() == Declaration::Type);
  QVERIFY(top->localDeclarations()[1]->kind() == Declaration::Instance);*/
  
  release(top);
}

void TestDUChain::testVariableDeclaration()
{
  TEST_FILE_PARSE_ONLY

  //                 0         1         2         3         4         5         6         7
  //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("A instance(b); A instance(2);");

  DUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());

  QVERIFY(!top->parentContext());
  QCOMPARE(top->childContexts().count(), 0);
  QCOMPARE(top->localDeclarations().count(), 2);
  QVERIFY(top->localScopeIdentifier().isEmpty());

//   IdentifiedType* idType = dynamic_cast<IdentifiedType*>(top->localDeclarations()[1]->abstractType().data());
//   QVERIFY(idType);
//   QCOMPARE( idType->identifier(), QualifiedIdentifier("A") );
//   
//   Declaration* defStructA = top->localDeclarations().first();
//   QCOMPARE(defStructA->identifier(), Identifier("A"));

  release(top);
}

void TestDUChain::testDeclareClass()
{
  TEST_FILE_PARSE_ONLY

  //                 0         1         2         3         4         5         6         7
  //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
  QByteArray method("class A { A() {}; A rec; void test(int); }; void A::test(int j) {}");

  DUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());

  KDevelop::DumpDotGraph dump;
/*  kDebug() << "dot-graph: \n" << dump.dotGraph(top, false);

  kDebug() << "ENDE ENDE ENDE";
  kDebug() << "dot-graph: \n" << dump.dotGraph(top, false);*/
  
  QVERIFY(!top->parentContext());
  QCOMPARE(top->childContexts().count(), 3);
  QCOMPARE(top->localDeclarations().count(), 1);
  QVERIFY(top->localScopeIdentifier().isEmpty());

  Declaration* defClassA = top->localDeclarations().first();
  QCOMPARE(defClassA->identifier(), Identifier("A"));
  QCOMPARE(defClassA->uses().count(), 1);
  QVERIFY(defClassA->type<CppClassType>());
  QCOMPARE(defClassA->type<CppClassType>()->elements().count(), 3);
  CppFunctionType::Ptr function = CppFunctionType::Ptr::dynamicCast(defClassA->type<CppClassType>()->elements()[2]);
  QVERIFY(function);
  QCOMPARE(function->returnType(), typeVoid);
  QCOMPARE(function->arguments().count(), 1);
  QCOMPARE(function->arguments().first(), typeInt);

  QVERIFY(defClassA->internalContext());

  QCOMPARE(defClassA->internalContext()->localDeclarations().count(), 3);
  QCOMPARE(defClassA->internalContext()->localDeclarations()[1]->abstractType(), defClassA->abstractType());

  DUContext* classA = top->childContexts().first();
  QVERIFY(classA->parentContext());
  QCOMPARE(classA->importedParentContexts().count(), 0);
  QCOMPARE(classA->childContexts().count(), 3);
  QCOMPARE(classA->localDeclarations().count(), 3);
  QCOMPARE(classA->localScopeIdentifier(), QualifiedIdentifier("A"));

  Declaration* defRec = classA->localDeclarations()[1];
  QVERIFY(defRec->abstractType());
  QCOMPARE(defRec->abstractType(), defClassA->abstractType());

  release(top);
}

void TestDUChain::testDeclareFriend()
{
  TEST_FILE_PARSE_ONLY

  //                 0         1         2         3         4         5         6         7
  //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
  QByteArray method("class A { friend class F; }; ");

  DUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());

  KDevelop::DumpDotGraph dump;
/*  kDebug() << "dot-graph: \n" << dump.dotGraph(top, false);

  kDebug() << "ENDE ENDE ENDE";
  kDebug() << "dot-graph: \n" << dump.dotGraph(top, false);*/
  
  QVERIFY(!top->parentContext());
  QCOMPARE(top->childContexts().count(), 1);
  QCOMPARE(top->localDeclarations().count(), 1);
  QVERIFY(top->localScopeIdentifier().isEmpty());

  Declaration* defClassA = top->localDeclarations().first();
  QCOMPARE(defClassA->identifier(), Identifier("A"));
  QCOMPARE(defClassA->uses().count(), 0);
  QVERIFY(defClassA->type<CppClassType>());
  QVERIFY(defClassA->internalContext());
  QCOMPARE(defClassA->internalContext()->localDeclarations().count(), 0);

  release(top);
}

void TestDUChain::testDeclareNamespace()
{
  TEST_FILE_PARSE_ONLY

  //                 0         1         2         3         4         5         6         7
  //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
  QByteArray method("namespace foo { int bar; } int bar; int test() { return foo::bar; }");

  DUContext* top = parse(method, DumpNone);


  DUChainWriteLocker lock(DUChain::lock());
  
  QVERIFY(!top->parentContext());
  QCOMPARE(top->childContexts().count(), 3);
  QCOMPARE(top->localDeclarations().count(), 2);
  QVERIFY(top->localScopeIdentifier().isEmpty());
  QCOMPARE(findDeclaration(top, Identifier("foo")), noDef);

  DUContext* fooCtx = top->childContexts().first();
  QCOMPARE(fooCtx->childContexts().count(), 0);
  QCOMPARE(fooCtx->localDeclarations().count(), 1);
  QCOMPARE(fooCtx->localScopeIdentifier(), QualifiedIdentifier("foo"));
  QCOMPARE(fooCtx->scopeIdentifier(), QualifiedIdentifier("foo"));

  DUContext* testCtx = top->childContexts()[2];
  QCOMPARE(testCtx->childContexts().count(), 0);
  QCOMPARE(testCtx->localDeclarations().count(), 0);
  QCOMPARE(testCtx->localScopeIdentifier(), QualifiedIdentifier());
  QCOMPARE(testCtx->scopeIdentifier(), QualifiedIdentifier());

  Declaration* bar2 = top->localDeclarations().first();
  QCOMPARE(bar2->identifier(), Identifier("bar"));
  QCOMPARE(bar2->qualifiedIdentifier(), QualifiedIdentifier("bar"));
  QCOMPARE(bar2->uses().count(), 0);

  Declaration* bar = fooCtx->localDeclarations().first();
  QCOMPARE(bar->identifier(), Identifier("bar"));
  QCOMPARE(bar->qualifiedIdentifier(), QualifiedIdentifier("foo::bar"));
  QCOMPARE(findDeclaration(testCtx,  QualifiedIdentifier("foo::bar")), bar);
  QCOMPARE(bar->uses().count(), 1);
  QCOMPARE(findDeclaration(top, bar->identifier()), bar2);
  QCOMPARE(findDeclaration(top, bar->qualifiedIdentifier()), bar);

  QCOMPARE(findDeclaration(top, QualifiedIdentifier("bar")), bar2);
  QCOMPARE(findDeclaration(top, QualifiedIdentifier("::bar")), bar2);
  QCOMPARE(findDeclaration(top, QualifiedIdentifier("foo::bar")), bar);
  QCOMPARE(findDeclaration(top, QualifiedIdentifier("::foo::bar")), bar);
  
  release(top);
}

void TestDUChain::testSearchAcrossNamespace()
{
  TEST_FILE_PARSE_ONLY

  //                 0         1         2         3         4         5         6         7
  //                 0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
  QByteArray method("namespace A { class B{}; } namespace Q{} namespace A{ B bla; }");

  DUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());
  
  QVERIFY(!top->parentContext());
  QCOMPARE(top->childContexts().count(), 3);
  QCOMPARE(top->childContexts()[2]->localDeclarations().count(), 1);
  QVERIFY(top->childContexts()[2]->localDeclarations()[0]->abstractType());
  QVERIFY(!dynamic_cast<DelayedType*>(top->childContexts()[2]->localDeclarations()[0]->abstractType().data()));
  
  release(top);
}

void TestDUChain::testDeclareUsingNamespace2()
{
  TEST_FILE_PARSE_ONLY

  //                 0         1         2         3         4         5         6         7
  //                 0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
  QByteArray method("namespace foo2 {int bar2; namespace SubFoo { int subBar2; } }; namespace foo { int bar; using namespace foo2; } namespace GFoo{ namespace renamedFoo2 = foo2; using namespace renamedFoo2; using namespace SubFoo; int gf; } using namespace GFoo; int test() { return bar; }");

  DUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());
  
  QVERIFY(!top->parentContext());
  QCOMPARE(top->childContexts().count(), 5);
  QCOMPARE(top->localDeclarations().count(), 2);
  QVERIFY(top->localScopeIdentifier().isEmpty());
  QCOMPARE(findDeclaration(top, Identifier("foo")), noDef);

  QCOMPARE( top->childContexts().first()->localDeclarations().count(), 1);
  Declaration* bar2 = top->childContexts().first()->localDeclarations()[0];
  QVERIFY(bar2);
  QCOMPARE(bar2->identifier(), Identifier("bar2"));
  
  QCOMPARE( top->childContexts()[1]->localDeclarations().count(), 2);
  Declaration* bar = top->childContexts()[1]->localDeclarations()[0];
  QVERIFY(bar);
  QCOMPARE(bar->identifier(), Identifier("bar"));
  
  QCOMPARE( top->childContexts()[2]->localDeclarations().count(), 4);
  Declaration* gf = top->childContexts()[2]->localDeclarations()[3];
  QVERIFY(gf);
  QCOMPARE(gf->identifier(), Identifier("gf"));
  
  //QCOMPARE(top->namespaceAliases().count(), 1);
  //QCOMPARE(top->namespaceAliases().first()->nsIdentifier, QualifiedIdentifier("foo"));

/*  QCOMPARE(findDeclaration(top, QualifiedIdentifier("bar2")), bar2);*/
  QCOMPARE(findDeclaration(top, QualifiedIdentifier("::bar")), noDef);
  QCOMPARE(findDeclaration(top, QualifiedIdentifier("foo::bar2")), bar2);
  QCOMPARE(findDeclaration(top, QualifiedIdentifier("foo2::bar2")), bar2);
  QCOMPARE(findDeclaration(top, QualifiedIdentifier("::foo::bar2")), bar2);

  //bar2 can be found from GFoo
  QCOMPARE(findDeclaration(top->childContexts()[2], QualifiedIdentifier("bar2")), bar2);
  QCOMPARE(findDeclaration(top->childContexts()[2], QualifiedIdentifier("bar")), noDef);
  QCOMPARE(findDeclaration(top, QualifiedIdentifier("gf")), gf);
  QCOMPARE(findDeclaration(top, QualifiedIdentifier("bar2")), bar2);
  QCOMPARE(findDeclaration(top, QualifiedIdentifier("::GFoo::renamedFoo2::bar2")), bar2);
  QVERIFY(findDeclaration(top, QualifiedIdentifier("subBar2")) != noDef);
  
  release(top);
}


void TestDUChain::testDeclareUsingNamespace()
{
  TEST_FILE_PARSE_ONLY

  //                 0         1         2         3         4         5         6         7
  //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
  QByteArray method("namespace foo { int bar; } using namespace foo; namespace alternativeFoo = foo; int test() { return bar; }");

  DUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());
  
  QVERIFY(!top->parentContext());
  QCOMPARE(top->childContexts().count(), 3);
  QCOMPARE(top->localDeclarations().count(), 3);
  QVERIFY(top->localScopeIdentifier().isEmpty());
  QCOMPARE(findDeclaration(top, Identifier("foo")), noDef);

//   QCOMPARE(top->namespaceAliases().count(), 1);
//   QCOMPARE(top->namespaceAliases().first()->nsIdentifier, QualifiedIdentifier("foo"));
//   QCOMPARE(top->namespaceAliases().first()->textCursor(), Cursor(0, 47));

  DUContext* fooCtx = top->childContexts().first();
  QCOMPARE(fooCtx->childContexts().count(), 0);
  QCOMPARE(fooCtx->localDeclarations().count(), 1);
  QCOMPARE(fooCtx->localScopeIdentifier(), QualifiedIdentifier("foo"));
  QCOMPARE(fooCtx->scopeIdentifier(), QualifiedIdentifier("foo"));

  Declaration* bar = fooCtx->localDeclarations().first();
  QCOMPARE(bar->identifier(), Identifier("bar"));
  QCOMPARE(bar->qualifiedIdentifier(), QualifiedIdentifier("foo::bar"));
  QCOMPARE(bar->uses().count(), 1);
  QCOMPARE(findDeclaration(top, bar->identifier(), top->textRange().start()), noDef);
  QCOMPARE(findDeclaration(top, bar->identifier()), bar);
  QCOMPARE(findDeclaration(top, bar->qualifiedIdentifier()), bar);

  QCOMPARE(findDeclaration(top, QualifiedIdentifier("bar")), bar);
  QCOMPARE(findDeclaration(top, QualifiedIdentifier("::bar")), bar); //iso c++ 3.4.3 says this must work
  QCOMPARE(findDeclaration(top, QualifiedIdentifier("foo::bar")), bar);
  QCOMPARE(findDeclaration(top, QualifiedIdentifier("::foo::bar")), bar);
  QCOMPARE(findDeclaration(top, QualifiedIdentifier("alternativeFoo::bar")), bar);
  QCOMPARE(findDeclaration(top, QualifiedIdentifier("::alternativeFoo::bar")), bar);

  DUContext* testCtx = top->childContexts()[2];
  QVERIFY(testCtx->parentContext());
  QCOMPARE(testCtx->importedParentContexts().count(), 1);
  QCOMPARE(testCtx->childContexts().count(), 0);
  QCOMPARE(testCtx->localDeclarations().count(), 0);
  QCOMPARE(testCtx->localScopeIdentifier(), QualifiedIdentifier());
  QCOMPARE(testCtx->scopeIdentifier(), QualifiedIdentifier());
  
  release(top);
}

void TestDUChain::testFunctionDefinition() {
  QByteArray text("class B{}; class A { char at(B* b); A(); ~A(); }; \n char A::at(B* b) {B* b; at(b); }; A::A() {}; A::~A() {}; ");

  DUContext* top = parse(text, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());

  QCOMPARE(top->childContexts().count(), 8);
  QCOMPARE(top->childContexts()[1]->childContexts().count(), 3);
  QCOMPARE(top->childContexts()[1]->localDeclarations().count(), 3);

  Declaration* atInA = top->childContexts()[1]->localDeclarations()[0];
  
  QVERIFY(dynamic_cast<AbstractFunctionDeclaration*>(atInA));

  foreach( Definition* def, top->localDefinitions() )
    kDebug() << "definition: " << def->declaration()->toString();
  QCOMPARE(top->localDeclarations().count(), 2);
  QCOMPARE(top->localDefinitions().count(), 3);

  
  QCOMPARE(top->localDefinitions()[0]->declaration(), top->childContexts()[1]->localDeclarations()[0]);
  QCOMPARE(top->localDefinitions()[0], top->childContexts()[1]->localDeclarations()[0]->definition());

  QCOMPARE(top->localDefinitions()[1]->declaration(), top->childContexts()[1]->localDeclarations()[1]);
  QCOMPARE(top->localDefinitions()[1], top->childContexts()[1]->localDeclarations()[1]->definition());

  QCOMPARE(top->localDefinitions()[2]->declaration(), top->childContexts()[1]->localDeclarations()[2]);
  QCOMPARE(top->localDefinitions()[2], top->childContexts()[1]->localDeclarations()[2]->definition());
  
  QCOMPARE(top->childContexts()[3]->owner()->asDefinition(), top->localDefinitions()[0]);
  QCOMPARE(top->childContexts()[3]->importedParentContexts().count(), 1);
  QCOMPARE(top->childContexts()[3]->importedParentContexts()[0].data(), top->childContexts()[2]);
  
  QCOMPARE(top->childContexts()[2]->importedParentContexts().count(), 1);
  QCOMPARE(top->childContexts()[2]->importedParentContexts()[0].data(), top->childContexts()[1]);
  
  
  QCOMPARE(findDeclaration(top, QualifiedIdentifier("at")), noDef);

  QVERIFY(top->localDefinitions()[0]->internalContext());
  QCOMPARE(top->localDefinitions()[0]->internalContext()->localDeclarations().count(), 1);
  
  ///How exactly should this look now?
/*  QCOMPARE(top->localDeclarations().count(), 2);
  QVERIFY(top->localDeclarations()[1]->isDefinition());*/
  
  release(top);
}

void TestDUChain::testBaseClasses() {
  QByteArray text("class A{int aValue; }; class B{int bValue;}; class C : public A{int cValue;}; class D : public A, B {int dValue;}; template<class Base> class F : public Base { int fValue;};");

  DUContext* top = parse(text, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());

  QCOMPARE(top->localDeclarations().count(), 5);
  Declaration* defClassA = top->localDeclarations().first();
  QCOMPARE(defClassA->identifier(), Identifier("A"));
  QVERIFY(defClassA->type<CppClassType>());
  
  Declaration* defClassB = top->localDeclarations()[1];
  QCOMPARE(defClassB->identifier(), Identifier("B"));
  QVERIFY(defClassB->type<CppClassType>());

  Declaration* defClassC = top->localDeclarations()[2];
  QCOMPARE(defClassC->identifier(), Identifier("C"));
  QVERIFY(defClassC->type<CppClassType>());
  QCOMPARE( defClassC->type<CppClassType>()->baseClasses().size(), 1 );
  
  Declaration* defClassD = top->localDeclarations()[3];
  QCOMPARE(defClassD->identifier(), Identifier("D"));
  QVERIFY(defClassD->type<CppClassType>());
  QCOMPARE( defClassD->type<CppClassType>()->baseClasses().size(), 2 );

  QVERIFY( findDeclaration( defClassD->internalContext(), Identifier("dValue") ) );
  QVERIFY( !findDeclaration( defClassD->internalContext(), Identifier("cValue") ) );
  QVERIFY( findDeclaration( defClassD->internalContext(), Identifier("aValue") ) );
  QVERIFY( findDeclaration( defClassD->internalContext(), Identifier("bValue") ) );
  
  QVERIFY( !findDeclaration( defClassC->internalContext(), Identifier("dValue") ) );
  QVERIFY( findDeclaration( defClassC->internalContext(), Identifier("cValue") ) );
  QVERIFY( !findDeclaration( defClassC->internalContext(), Identifier("bValue") ) );
  QVERIFY( findDeclaration( defClassC->internalContext(), Identifier("aValue") ) );

  QVERIFY( !findDeclaration( defClassB->internalContext(), Identifier("dValue") ) );
  QVERIFY( !findDeclaration( defClassB->internalContext(), Identifier("cValue") ) );
  QVERIFY( findDeclaration( defClassB->internalContext(), Identifier("bValue") ) );
  QVERIFY( !findDeclaration( defClassB->internalContext(), Identifier("aValue") ) );

  QVERIFY( !findDeclaration( defClassA->internalContext(), Identifier("dValue") ) );
  QVERIFY( !findDeclaration( defClassA->internalContext(), Identifier("cValue") ) );
  QVERIFY( !findDeclaration( defClassA->internalContext(), Identifier("bValue") ) );
  QVERIFY( findDeclaration( defClassA->internalContext(), Identifier("aValue") ) );

  ///Now test a template-class as base-class
  Declaration* defClassF = top->localDeclarations()[4];
  QCOMPARE(defClassF->identifier(), Identifier("F"));
  QVERIFY(defClassF->type<CppClassType>());
  QCOMPARE( defClassF->type<CppClassType>()->baseClasses().size(), 1 );
  QVERIFY( dynamic_cast<const DelayedType*>( defClassF->type<CppClassType>()->baseClasses().front().baseClass.data() ) );

  Declaration* FDDecl = findDeclaration(top, QualifiedIdentifier("F<D>") );
  QVERIFY(FDDecl);
  QVERIFY(FDDecl->internalContext() != defClassF->internalContext());
  
  QVERIFY( findDeclaration( FDDecl->internalContext(), Identifier("dValue") ) );
  QVERIFY( !findDeclaration( FDDecl->internalContext(), Identifier("cValue") ) );
  QVERIFY( findDeclaration( FDDecl->internalContext(), Identifier("aValue") ) );
  QVERIFY( findDeclaration( FDDecl->internalContext(), Identifier("bValue") ) );
  QVERIFY( findDeclaration( FDDecl->internalContext(), Identifier("fValue") ) );
  
  
  release(top);
}

void TestDUChain::testTypedef() {
  QByteArray method("/*This is A translation-unit*/ \n/*This is class A*/class A { }; \ntypedef A B;//This is a typedef\nvoid test() { }");

  DUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());

  Declaration* defClassA = top->localDeclarations().first();
  QCOMPARE(defClassA->identifier(), Identifier("A"));
  QVERIFY(defClassA->type<CppClassType>());
  QCOMPARE(defClassA->comment(), QString("This is class A"));

  DUContext* classA = top->childContexts().first();
  QVERIFY(classA->parentContext());
  QCOMPARE(classA->importedParentContexts().count(), 0);
  QCOMPARE(classA->localScopeIdentifier(), QualifiedIdentifier("A"));

  Declaration* defB = findDeclaration(top,  Identifier("B"));
  QCOMPARE(defB->abstractType(), defClassA->abstractType());
  QVERIFY(defB->isTypeAlias());
  QCOMPARE(defB->kind(), Declaration::Type);
  QCOMPARE(defB->comment(), QString("This is a typedef"));
  
  QVERIFY( !dynamic_cast<CppTypeAliasType*>(defB->abstractType().data()) ); //Just verify that CppTypeAliasType is not used, because it isn't(maybe remove that class?)
  
  release(top);
}

void TestDUChain::testContextAssignment() {
  QByteArray text("template<class A>class Class { enum{ Bla = A::a }; }; ");
  DUContext* top = parse(text, DumpNone);
  DUChainWriteLocker lock(DUChain::lock());

  QCOMPARE(top->localDeclarations().count(), 1);

  QCOMPARE(top->childContexts().count(), 2);
  QCOMPARE(top->childContexts()[1]->owner(), top->localDeclarations()[0]);

  QCOMPARE(top->childContexts()[1]->localDeclarations().count(), 2);
  
  QCOMPARE((void*)top->childContexts()[0]->owner(), (void*)0);
  QCOMPARE((void*)top->childContexts()[1]->localDeclarations()[0]->internalContext(), (void*)0);

  release(top);
}

void TestDUChain::testSpecializedTemplates() {
  QByteArray text("class A{}; class B{}; class C{}; template<class T,class T2> class E{typedef A Type1;}; template<class T2> class E<A,T2> { typedef B Type2;}; template<class T2> class E<T2,A> { typedef C Type3; };");
  DUContext* top = parse(text, DumpNone);
  DUChainWriteLocker lock(DUChain::lock());

  QCOMPARE(top->localDeclarations().count(), 6);
  Declaration* ADecl = top->localDeclarations()[0];
  Declaration* BDecl = top->localDeclarations()[1];
  Declaration* CDecl = top->localDeclarations()[2];
  Declaration* EDecl = top->localDeclarations()[3];
  Declaration* E1Decl = top->localDeclarations()[4];
  Declaration* E2Decl = top->localDeclarations()[5];

  TemplateDeclaration* templateEDecl = dynamic_cast<TemplateDeclaration*>(EDecl);
  TemplateDeclaration* templateE1Decl = dynamic_cast<TemplateDeclaration*>(E1Decl);
  TemplateDeclaration* templateE2Decl = dynamic_cast<TemplateDeclaration*>(E2Decl);
  QVERIFY(templateEDecl);
  QVERIFY(templateE1Decl);
  QVERIFY(templateE2Decl);

  QCOMPARE(templateE1Decl->specializedFrom(), templateEDecl);
  QCOMPARE(templateE2Decl->specializedFrom(), templateEDecl);
  
  release(top);
}

int value( const AbstractType::Ptr& type ) {
  const CppConstantIntegralType* integral = dynamic_cast<const CppConstantIntegralType*>(type.data());
  if( integral )
    return (int)integral->value<qint64>();
  else
    return 0;
}

void TestDUChain::testTemplateEnums()
{
  QByteArray text("class A {enum { Val = 5}; }; class B { enum{ Val = 7 }; }; template<class C, int i> class Test { enum { TempVal = C::Val, Num = i, Sum = TempVal + i }; };");
  DUContext* top = parse(text, DumpNone);
  DUChainWriteLocker lock(DUChain::lock());

  QCOMPARE(top->localDeclarations().count(), 3);
  Declaration* ADecl = top->localDeclarations()[0];
  Declaration* BDecl = top->localDeclarations()[1];
  Declaration* testDecl = top->localDeclarations()[2];

  TemplateDeclaration* templateTestDecl = dynamic_cast<TemplateDeclaration*>(testDecl);
  QVERIFY(templateTestDecl);

  Declaration* tempDecl = findDeclaration( top, QualifiedIdentifier("Test<A, 3>::TempVal") );
  QVERIFY(tempDecl);
  QCOMPARE(value(tempDecl->abstractType()), 5);
  
  tempDecl = findDeclaration( top, QualifiedIdentifier("Test<A, 3>::Num") );
  QVERIFY(tempDecl);
  QCOMPARE(value(tempDecl->abstractType()), 3);

  tempDecl = findDeclaration( top, QualifiedIdentifier("Test<A, 3>::Sum") );
  QVERIFY(tempDecl->abstractType());
  QCOMPARE(value(tempDecl->abstractType()), 8);
  
  tempDecl = findDeclaration( top, QualifiedIdentifier("Test<B, 9>::TempVal") );
  QVERIFY(tempDecl->abstractType());
  QCOMPARE(value(tempDecl->abstractType()), 7);

  tempDecl = findDeclaration( top, QualifiedIdentifier("Test<B, 9>::Num") );
  QVERIFY(tempDecl->abstractType());
  QCOMPARE(value(tempDecl->abstractType()), 9);

  tempDecl = findDeclaration( top, QualifiedIdentifier("Test<B, 9>::Sum") );
  QVERIFY(tempDecl->abstractType());
  QCOMPARE(value(tempDecl->abstractType()), 16);
  
  release(top);
}

void TestDUChain::testIntegralTemplates()
{
  QByteArray text("template<class T> class A { T i; }; ");
  DUContext* top = parse(text, DumpNone);
  DUChainWriteLocker lock(DUChain::lock());

  QCOMPARE(top->localDeclarations().count(), 1);
  QCOMPARE(top->childContexts().count(), 2);
  Declaration* ADecl = top->localDeclarations()[0];

  QCOMPARE(top->childContexts()[1]->localDeclarations().count(), 1);
  Declaration* dec = findDeclaration( top, QualifiedIdentifier( "A<int>::i") );
  QVERIFY(dec);
  CppIntegralType* integral = dynamic_cast<CppIntegralType*>( dec->abstractType().data() );
  QVERIFY( integral );
  QCOMPARE( integral->integralType(), CppIntegralType::TypeInt );
  
  QCOMPARE(top->childContexts()[1]->localDeclarations().count(), 1);
  dec = findDeclaration( top, QualifiedIdentifier( "A<unsigned int>::i") );
  integral = dynamic_cast<CppIntegralType*>( dec->abstractType().data() );
  QVERIFY( integral );
  QCOMPARE( integral->integralType(), CppIntegralType::TypeInt );
  QCOMPARE( integral->typeModifiers(), CppIntegralType::ModifierUnsigned );
  
  QCOMPARE(top->childContexts()[1]->localDeclarations().count(), 1);
  dec = findDeclaration( top, QualifiedIdentifier( "A<long double>::i") );
  integral = dynamic_cast<CppIntegralType*>( dec->abstractType().data() );
  QVERIFY( integral );
  QCOMPARE( integral->integralType(), CppIntegralType::TypeDouble );
  QCOMPARE( integral->typeModifiers(), CppIntegralType::ModifierLong );
  
  release(top);
}

void TestDUChain::testFunctionTemplates() {
  QByteArray method("template<class T> T test(const T& t) {};");

  DUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());

  Declaration* defTest = top->localDeclarations()[0];
  QCOMPARE(defTest->identifier(), Identifier("test"));
  QVERIFY(defTest->type<FunctionType>());
  QVERIFY( isTemplateDeclaration(defTest) );

  QCOMPARE( defTest->type<FunctionType>()->arguments().count(), 1 );
  QVERIFY( dynamic_cast<const DelayedType*>( realType(defTest->type<FunctionType>()->arguments()[0]) ) );
  
  release(top);
}

void TestDUChain::testTemplateFunctions() {
  QByteArray method("class A {}; template<class T> T a(T& q) {};");

  DUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());
  QCOMPARE(top->localDeclarations().count(), 2);
  Declaration* d = findDeclaration(top, QualifiedIdentifier("a<A>"));
  QVERIFY(d);
  CppFunctionType* cppFunction = dynamic_cast<CppFunctionType*>(d->abstractType().data());
  QVERIFY(cppFunction);
  QCOMPARE(cppFunction->arguments().count(), 1);
  QCOMPARE(cppFunction->returnType(), top->localDeclarations()[0]->abstractType());
  QCOMPARE(cppFunction->arguments()[0]->toString(), QString("A&"));
}

void TestDUChain::testTemplateDependentClass() {
  QByteArray method("class A {}; template<class T> class B { class Q{ typedef T Type; }; }; B<A>::Q::Type t;");

  DUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());
  Declaration* d = findDeclaration(top, QualifiedIdentifier("t"));
  QVERIFY(d);
  kDebug() << d->toString();
  QCOMPARE(d->abstractType().data(), top->localDeclarations()[0]->abstractType().data());
}

void TestDUChain::testTemplates() {
  QByteArray method("template<class T> T test(const T& t) {}; template<class T, class T2> class A {T2 a; typedef T Template1; }; class B{int b;}; class C{int c;}; template<class T>class A<B,T>{};  typedef A<B,C> D;");

  DUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());

  Declaration* defTest = top->localDeclarations()[0];
  QCOMPARE(defTest->identifier(), Identifier("test"));
  QVERIFY(defTest->type<FunctionType>());
  QVERIFY( isTemplateDeclaration(defTest) );
  
  Declaration* defClassA = top->localDeclarations()[1];
  QCOMPARE(defClassA->identifier(), Identifier("A"));
  QVERIFY(defClassA->type<CppClassType>());
  QVERIFY( isTemplateDeclaration(defClassA) );

  Declaration* defClassB = top->localDeclarations()[2];
  QCOMPARE(defClassB->identifier(), Identifier("B"));
  QVERIFY(defClassB->type<CppClassType>());
  QVERIFY( !isTemplateDeclaration(defClassB) );
  
  Declaration* defClassC = top->localDeclarations()[3];
  QCOMPARE(defClassC->identifier(), Identifier("C"));
  QVERIFY(defClassC->type<CppClassType>());
  QVERIFY( !isTemplateDeclaration(defClassC) );
  
  DUContext* classA = defClassA->internalContext();
  QVERIFY(classA);
  QVERIFY(classA->parentContext());
  QCOMPARE(classA->importedParentContexts().count(), 1); //The template-parameter context is imported
  QCOMPARE(classA->localScopeIdentifier(), QualifiedIdentifier("A"));

  DUContext* classB = defClassB->internalContext();
  QVERIFY(classB);
  QVERIFY(classB->parentContext());
  QCOMPARE(classB->importedParentContexts().count(), 0);
  QCOMPARE(classB->localScopeIdentifier(), QualifiedIdentifier("B"));
  
  DUContext* classC = defClassC->internalContext();
  QVERIFY(classC);
  QVERIFY(classC->parentContext());
  QCOMPARE(classC->importedParentContexts().count(), 0);
  QCOMPARE(classC->localScopeIdentifier(), QualifiedIdentifier("C"));

  ///Test getting the typedef for the unset template
  {
    Declaration* typedefDecl = findDeclaration(classA, Identifier("Template1"));
    QVERIFY(typedefDecl);
    QVERIFY(typedefDecl->isTypeAlias());
    QVERIFY(typedefDecl->abstractType());
    DelayedType* delayed = dynamic_cast<DelayedType*>(typedefDecl->abstractType().data());
    QVERIFY(delayed);
    QCOMPARE(delayed->qualifiedIdentifier(), QualifiedIdentifier("T"));
  }

  ///Test creating a template instance of class A<B,C>
  {
    Identifier ident("A");
    ident.appendTemplateIdentifier(QualifiedIdentifier("B"));
    ident.appendTemplateIdentifier(QualifiedIdentifier("C"));
    Declaration* instanceDefClassA = findDeclaration(top, ident);
    Declaration* instanceTypedefD = findDeclaration(top, Identifier("D"));
    QVERIFY(instanceTypedefD);
    QVERIFY(instanceDefClassA);
    QCOMPARE(instanceTypedefD->abstractType()->toString(), instanceDefClassA->abstractType()->toString() );
    //QCOMPARE(instanceTypedefD->abstractType().data(), instanceDefClassA->abstractType().data() ); Re-enable once specializations are re-used
    Cpp::TemplateDeclaration* templateDecl = dynamic_cast<Cpp::TemplateDeclaration*>(instanceDefClassA);
    QVERIFY(templateDecl);
    QVERIFY(instanceDefClassA != defClassA);
    QVERIFY(instanceDefClassA->context() == defClassA->context());
    QVERIFY(instanceDefClassA->internalContext() != defClassA->internalContext());
    QCOMPARE(instanceDefClassA->identifier(), Identifier("A<B,C>"));
    QCOMPARE(instanceDefClassA->identifier().toString(), QString("A< B, C >"));
    QVERIFY(instanceDefClassA->abstractType());
    IdentifiedType* identifiedType = dynamic_cast<IdentifiedType*>(instanceDefClassA->abstractType().data());
    QVERIFY(identifiedType);
    QCOMPARE(identifiedType->declaration(), instanceDefClassA);
    QCOMPARE(identifiedType->identifier().toString(), Identifier("A<B,C>").toString());
    QVERIFY(instanceDefClassA->internalContext());
    QVERIFY(instanceDefClassA->internalContext() != defClassA->internalContext());
    QVERIFY(instanceDefClassA->context() == defClassA->context());
    QVERIFY(instanceDefClassA->internalContext()->importedParentContexts().size() == 1);
    QVERIFY(defClassA->internalContext()->importedParentContexts().size() == 1);
    QCOMPARE(instanceDefClassA->internalContext()->importedParentContexts().front()->type(), DUContext::Template);
    QVERIFY(defClassA->internalContext()->importedParentContexts().front() != instanceDefClassA->internalContext()->importedParentContexts().front()); //The template-context has been instantiated

    //Make sure the first template-parameter has been resolved to class B
    QCOMPARE( instanceDefClassA->internalContext()->importedParentContexts()[0]->localDeclarations()[0]->abstractType().data(), defClassB->abstractType().data() );
    //Make sure the second template-parameter has been resolved to class C
    QCOMPARE( instanceDefClassA->internalContext()->importedParentContexts()[0]->localDeclarations()[1]->abstractType().data(), defClassC->abstractType().data() );

    QualifiedIdentifier ident2(ident);
    ident2.push(Identifier("Template1"));
    
    Declaration* template1InstanceDecl1 = findDeclaration(instanceDefClassA->internalContext(), Identifier("Template1"));
    QVERIFY(template1InstanceDecl1);
    QCOMPARE(template1InstanceDecl1->abstractType()->toString(), QString("B"));
    
    Declaration* template1InstanceDecl2 = findDeclaration(instanceDefClassA->internalContext(), Identifier("a"));
    QVERIFY(template1InstanceDecl2);
    QCOMPARE(template1InstanceDecl2->abstractType()->toString(), QString("C"));
    
/*    kDebug(9007) << "searching for" << ident2.toString();
    kDebug(9007) << "Part 1:" << ident2.at(0).toString() << "templates:" << ident2.at(0).templateIdentifiers().count();
    kDebug(9007) << "Part 2:" << ident2.at(1).toString();*/
    Declaration* template1InstanceDecl = findDeclaration(top, ident2);
    QVERIFY(template1InstanceDecl);
/*    kDebug(9007) << "found:" << template1InstanceDecl->toString();*/
    QCOMPARE(template1InstanceDecl->abstractType()->toString(), QString("B"));
  }
  
/*  QCOMPARE(findDeclaration(top,  Identifier("B"))->abstractType(), defClassA->abstractType());
  QVERIFY(findDeclaration(top,  Identifier("B"))->isTypeAlias());
  QCOMPARE(findDeclaration(top,  Identifier("B"))->kind(), Declaration::Type);*/
  release(top);
}

void TestDUChain::testTemplateDefaultParameters() {
  QByteArray method("struct S {} ; namespace std { template<class T> class Template1 { }; } template<class _TT, typename TT2 = std::Template1<_TT> > class Template2 { typedef TT2 T1; };");

  DUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());

  Declaration* memberDecl = findDeclaration(top, QualifiedIdentifier("Template2<S>::T1"));
  QVERIFY(memberDecl);
  QVERIFY(memberDecl->abstractType());
  QCOMPARE(memberDecl->abstractType()->toString(), QString("Template1< S >"));
  
  release(top);
}

void TestDUChain::testTemplates2() {
  QByteArray method("struct S {} ; template<class TT> class Base { struct Alloc { typedef TT& referenceType; }; }; template<class T> struct Class : public Base<T> { typedef typename Base<T>::Alloc Alloc; typedef typename Alloc::referenceType reference; reference member; }; Class<S*> instance;");

  DUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());

  Declaration* memberDecl = findDeclaration(top, QualifiedIdentifier("instance"));
  QVERIFY(memberDecl);
  QVERIFY(memberDecl->abstractType());
  QCOMPARE(memberDecl->abstractType()->toString(), QString("Class< S* >"));
  
/*  memberDecl = findDeclaration(top, QualifiedIdentifier("Class<S>::Alloc<S>::referenceType"));
  QVERIFY(memberDecl);
  QVERIFY(memberDecl->abstractType());
  QCOMPARE(memberDecl->abstractType()->toString(), QString("S&"));*/
  
  memberDecl = findDeclaration(top, QualifiedIdentifier("Class<S>::member"));
  QVERIFY(memberDecl);
  QVERIFY(memberDecl->abstractType());
  QCOMPARE(memberDecl->abstractType()->toString(), QString("S&"));
  
  
  memberDecl = findDeclaration(top, QualifiedIdentifier("Class<S*>::member"));
  QVERIFY(memberDecl);
  QVERIFY(memberDecl->abstractType());
  QCOMPARE(memberDecl->abstractType()->toString(), QString("S*&"));

  release(top);
}

void TestDUChain::testTemplatesRebind() {
  QByteArray method("struct A {}; struct S {typedef A Value;} ; template<class TT> class Base { template<class T> struct rebind { typedef Base<T> other; }; typedef TT Type; }; template<class T> class Class { typedef Base<T>::rebind<T>::other::Type MemberType; MemberType member; Base<T>::template rebind<T>::other::Type member2; T::Value value; }; };");

  DUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());

  QCOMPARE(top->childContexts().count(), 6);
  QCOMPARE(top->childContexts()[3]->childContexts().count(), 2);
  QCOMPARE(top->childContexts()[3]->childContexts()[1]->localDeclarations().count(), 1);
  {
  Declaration* memberDecl = findDeclaration(top, QualifiedIdentifier("Base<S>::rebind<A>::other::Type"));
  QVERIFY(memberDecl);
  QVERIFY(memberDecl->abstractType());
  QCOMPARE(memberDecl->abstractType()->toString(), QString("A"));
  }
  
  Declaration* memberDecl = findDeclaration(top, QualifiedIdentifier("Class<S>::member"));
  QVERIFY(memberDecl);
  QVERIFY(memberDecl->abstractType());
  QCOMPARE(memberDecl->abstractType()->toString(), QString("S"));
  
  Declaration* member3Decl = findDeclaration(top, QualifiedIdentifier("Class<S>::value"));
  QVERIFY(member3Decl);
  QVERIFY(member3Decl->abstractType());
  QCOMPARE(member3Decl->abstractType()->toString(), QString("A"));
  
  Declaration* member2Decl = findDeclaration(top, QualifiedIdentifier("Class<S>::member2"));
  QVERIFY(member2Decl);
  QVERIFY(member2Decl->abstractType());
  QCOMPARE(member2Decl->abstractType()->toString(), QString("S"));
  
  release(top);
}

void TestDUChain::testTemplatesRebind2() {
  QByteArray method("struct A {}; struct S {typedef A Value;} ;template<class T> class Test { }; template<class T> class Class { typedef T::Value Value; T::Value value; typedef Test<Value> ValueClass; Test<const Value> ValueClass2;}; };");

  DUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());

  Declaration* member5Decl = findDeclaration(top, QualifiedIdentifier("Class<S>::ValueClass2"));
  QVERIFY(member5Decl);
  QVERIFY(member5Decl->abstractType());
  QCOMPARE(member5Decl->abstractType()->toString(), QString("Test< A >")); ///@todo This will fail once we parse "const" correctly, change it to "Test< const A >" then
  
  Declaration* member4Decl = findDeclaration(top, QualifiedIdentifier("Class<S>::ValueClass"));
  QVERIFY(member4Decl);
  QVERIFY(member4Decl->abstractType());
  QCOMPARE(member4Decl->abstractType()->toString(), QString("Test< A >"));
  
  Declaration* member3Decl = findDeclaration(top, QualifiedIdentifier("Class<S>::value"));
  QVERIFY(member3Decl);
  QVERIFY(member3Decl->abstractType());
  QCOMPARE(member3Decl->abstractType()->toString(), QString("A"));
  
  release(top);
}

void TestDUChain::testForwardDeclaration()
{
  QByteArray method("class Test; Test t; class Test {int i; class SubTest; }; Test::SubTest t2; class Test::SubTest{ int i;};");

  DUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());

  QCOMPARE(top->localDeclarations().count(), 5);

  QVERIFY(dynamic_cast<ForwardDeclaration*>(top->localDeclarations()[0]));
  
  CppClassType* type1 = top->localDeclarations()[0]->type<CppClassType>().data();
  CppClassType* type2 = top->localDeclarations()[1]->type<CppClassType>().data();
  CppClassType* type3 = top->localDeclarations()[2]->type<CppClassType>().data();
  CppClassType* type4 = top->localDeclarations()[3]->type<CppClassType>().data();
  CppClassType* type5 = top->localDeclarations()[4]->type<CppClassType>().data();


  QCOMPARE(top->localDeclarations()[0]->kind(), Declaration::Type);
  QCOMPARE(top->localDeclarations()[1]->kind(), Declaration::Instance);
  QCOMPARE(top->localDeclarations()[2]->kind(), Declaration::Type);
  QCOMPARE(top->localDeclarations()[3]->kind(), Declaration::Instance);
  QCOMPARE(top->localDeclarations()[4]->kind(), Declaration::Type);
  QVERIFY(type1);
  QVERIFY(type2);
  QVERIFY(type3);
  QVERIFY(type4);
  QVERIFY(type5);

  Declaration* TestDecl = top->localDeclarations()[2];
  QVERIFY(TestDecl->internalContext());
  QCOMPARE(TestDecl->internalContext()->localDeclarations().count(), 2);
  CppClassType* subType = TestDecl->internalContext()->localDeclarations()[1]->type<CppClassType>().data();

  QCOMPARE(type1, type2);
  QCOMPARE(type2, type3);
  QCOMPARE(subType, type4);
  QCOMPARE(type4, type5);
  
  release(top);
}

void TestDUChain::testForwardDeclaration2()
{
  QByteArray method("class Test; Test t; class Test {int i; class SubTest; }; class Test; Test::SubTest t2; class Test::SubTest{ int i;};");

  DUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());

  QCOMPARE(top->localDeclarations().count(), 6);

  QVERIFY(dynamic_cast<ForwardDeclaration*>(top->localDeclarations()[0]));
  
  CppClassType* type1 = top->localDeclarations()[0]->type<CppClassType>().data();
  CppClassType* type2 = top->localDeclarations()[1]->type<CppClassType>().data();
  CppClassType* type3 = top->localDeclarations()[2]->type<CppClassType>().data();
  CppClassType* type4 = top->localDeclarations()[4]->type<CppClassType>().data();
  CppClassType* type5 = top->localDeclarations()[5]->type<CppClassType>().data();
  CppClassType* type12 = top->localDeclarations()[3]->type<CppClassType>().data();

  QCOMPARE(top->localDeclarations()[0]->kind(), Declaration::Type);
  QCOMPARE(top->localDeclarations()[1]->kind(), Declaration::Instance);
  QCOMPARE(top->localDeclarations()[2]->kind(), Declaration::Type);
  QCOMPARE(top->localDeclarations()[3]->kind(), Declaration::Type);
  QCOMPARE(top->localDeclarations()[4]->kind(), Declaration::Instance);
  QCOMPARE(top->localDeclarations()[5]->kind(), Declaration::Type);
  
  QVERIFY(type1);
  QVERIFY(type12);
  QVERIFY(type2);
  QVERIFY(type3);
  QVERIFY(type4);
  QVERIFY(type5);

  Declaration* TestDecl = top->localDeclarations()[2];
  QVERIFY(TestDecl->internalContext());
  QCOMPARE(TestDecl->internalContext()->localDeclarations().count(), 2);
  CppClassType* subType = TestDecl->internalContext()->localDeclarations()[1]->type<CppClassType>().data();

  QCOMPARE(type1, type2);
  QCOMPARE(type2, type3);
  QCOMPARE(type1, type12);
  QCOMPARE(subType, type4);
  QCOMPARE(type4, type5);
  
  release(top);
}

void TestDUChain::testForwardDeclaration3()
{
  QByteArray method("namespace B {class Test;} B::Test t; namespace B { class Test {int i; class SubTest; };} B::Test::SubTest t2; namespace B {class Test::SubTest{ int i;};}");

  DUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());

  QCOMPARE(top->localDeclarations().count(), 2);
  QCOMPARE(top->childContexts().count(), 3);

  QVERIFY(dynamic_cast<ForwardDeclaration*>(top->childContexts()[0]->localDeclarations()[0]));

  ForwardDeclaration* forwardDecl = static_cast<ForwardDeclaration*>(top->childContexts()[0]->localDeclarations()[0]);
  QVERIFY(forwardDecl->resolved());
  QCOMPARE(forwardDecl->resolved(), top->childContexts()[1]->localDeclarations()[0]);
  
  CppClassType* type1 = top->childContexts()[0]->localDeclarations()[0]->type<CppClassType>().data();
  CppClassType* type2 = top->localDeclarations()[0]->type<CppClassType>().data();
  CppClassType* type3 = top->childContexts()[1]->localDeclarations()[0]->type<CppClassType>().data();
  CppClassType* type4 = top->localDeclarations()[1]->type<CppClassType>().data();
  CppClassType* type5 = top->childContexts()[2]->localDeclarations()[0]->type<CppClassType>().data();

  
  QVERIFY(type1);
  QVERIFY(type2);
  QVERIFY(type3);
  QVERIFY(type4);
  QVERIFY(type5);

  Declaration* TestDecl = top->childContexts()[1]->localDeclarations()[0];
  QVERIFY(TestDecl->internalContext());
  QCOMPARE(TestDecl->internalContext()->localDeclarations().count(), 2);
  CppClassType* subType = TestDecl->internalContext()->localDeclarations()[1]->type<CppClassType>().data();

  QCOMPARE(type1, type2);
  QCOMPARE(type2, type3);
  QCOMPARE(subType, type4);
  QCOMPARE(type4, type5);
  
  release(top);
}

void TestDUChain::testTemplateForwardDeclaration()
{
  QByteArray method("class B{}; template<class T = B>class Test; Test<B> t; template<class T = B>class Test {}; ");

  DUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());

  QCOMPARE(top->localDeclarations().count(), 4);

  QVERIFY(top->localDeclarations()[1]->internalContext());
  
  QVERIFY(dynamic_cast<ForwardDeclaration*>(top->localDeclarations()[1]));
  
  CppClassType* type1 = top->localDeclarations()[1]->type<CppClassType>().data();
  CppClassType* type2 = top->localDeclarations()[2]->type<CppClassType>().data();
  CppClassType* type3 = top->localDeclarations()[3]->type<CppClassType>().data();

  QVERIFY(type1);
  QVERIFY(type2);
  QVERIFY(type3);

  TemplateDeclaration* temp2Decl = dynamic_cast<TemplateDeclaration*>(type2->declaration());
  QVERIFY(temp2Decl);
  TemplateDeclaration* temp3Decl = dynamic_cast<TemplateDeclaration*>(type3->declaration());
  QVERIFY(temp3Decl);
  QCOMPARE(temp2Decl->instantiatedFrom(), temp3Decl);

  Declaration* decl = findDeclaration(top, QualifiedIdentifier("Test<>"));
  QVERIFY(decl);
  kDebug() << decl->toString();
  
  release(top);
}

void TestDUChain::testTemplateForwardDeclaration2()
{
  QByteArray method("class B{}; template<class T = B, class Q = B, class R = B>class Test; Test<B> t; template<class T, class Q, class R>class Test { T a; Q b; R c;}; ");

  DUContext* top = parse(method, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());

  QCOMPARE(top->localDeclarations().count(), 4);

  QVERIFY(top->localDeclarations()[1]->internalContext());
  kDebug() << top->localDeclarations()[1]->internalContext();
  
  QVERIFY(dynamic_cast<ForwardDeclaration*>(top->localDeclarations()[1]));
  
  CppClassType* type1 = top->localDeclarations()[1]->type<CppClassType>().data();
  CppClassType* type2 = top->localDeclarations()[2]->type<CppClassType>().data();
  CppClassType* type3 = top->localDeclarations()[3]->type<CppClassType>().data();

  QVERIFY(type1);
  QVERIFY(type2);
  QVERIFY(type3);

  TemplateDeclaration* temp2Decl = dynamic_cast<TemplateDeclaration*>(type2->declaration());
  QVERIFY(temp2Decl);
  TemplateDeclaration* temp3Decl = dynamic_cast<TemplateDeclaration*>(type3->declaration());
  QVERIFY(temp3Decl);
  QCOMPARE(temp2Decl->instantiatedFrom(), temp3Decl);

  Declaration* decl = findDeclaration(top, QualifiedIdentifier("Test<>"));
  QVERIFY(decl);
  QVERIFY(decl->internalContext());
  Declaration* aDecl = findDeclaration(decl->internalContext(), QualifiedIdentifier("a"));
  QVERIFY(aDecl);
  Declaration* bDecl = findDeclaration(decl->internalContext(), QualifiedIdentifier("b"));
  QVERIFY(bDecl);
  Declaration* cDecl = findDeclaration(decl->internalContext(), QualifiedIdentifier("c"));
  QVERIFY(cDecl);
  QCOMPARE(bDecl->abstractType().data(), top->localDeclarations()[0]->abstractType().data());
  QCOMPARE(cDecl->abstractType().data(), top->localDeclarations()[0]->abstractType().data());
  QCOMPARE(aDecl->abstractType().data(), top->localDeclarations()[0]->abstractType().data());
  
  release(top);
}

void TestDUChain::testFileParse()
{
  //QSKIP("Unwanted", SkipSingle);

  //QFile file("/opt/kde4/src/kdevelop/languages/cpp/duchain/tests/files/membervariable.cpp");
  QFile file("/opt/kde4/src/kdevelop/languages/csharp/parser/csharp_parser.cpp");
  //QFile file("/opt/kde4/src/kdevelop/plugins/outputviews/makewidget.cpp");
  //QFile file("/opt/kde4/src/kdelibs/kate/part/katecompletionmodel.h");
  //QFile file("/opt/kde4/src/kdevelop/lib/kdevbackgroundparser.cpp");
  //QFile file("/opt/qt-copy/src/gui/kernel/qwidget.cpp");
  QVERIFY( file.open( QIODevice::ReadOnly ) );

  QByteArray fileData = file.readAll();
  file.close();
  QString contents = QString::fromUtf8( fileData.constData() );
  rpp::Preprocessor preprocessor;
  QString ppd = preprocessor.processString( contents );
  QByteArray preprocessed = ppd.toUtf8();

  DUContext* top = parse(preprocessed, DumpNone);

  DUChainWriteLocker lock(DUChain::lock());

  SymbolTable::self()->dumpStatistics();

  release(top);
}

typedef BasicSetRepository::Index Index;

void TestDUChain::testHashedStringRepository() {

  //Create 3 random sets with each 10 of 20 items
  const unsigned int setCount = 10;
  const unsigned int choiceCount = 30;
  const unsigned int itemCount = 100;
  
  BasicSetRepository rep;
  
  for(Index a = 0; a < itemCount; a++)
    Q_ASSERT( a+1 == rep.appendIndices(1) );

  kDebug() << "Start repository-layout: \n" << rep.dumpDotGraph();

  clock_t repositoryClockTime = 0; //Time spent on repository-operations
  clock_t genericClockTime = 0; //Time spend on equivalent operations with generic sets
  
  clock_t repositoryIntersectionClockTime = 0; //Time spent on repository-operations
  clock_t genericIntersectionClockTime = 0; //Time spend on equivalent operations with generic sets
  clock_t qsetIntersectionClockTime = 0; //Time spend on equivalent operations with generic sets
  
  clock_t repositoryUnionClockTime = 0; //Time spent on repository-operations
  clock_t genericUnionClockTime = 0; //Time spend on equivalent operations with generic sets
  
  clock_t repositoryDifferenceClockTime = 0; //Time spent on repository-operations
  clock_t genericDifferenceClockTime = 0; //Time spend on equivalent operations with generic sets
  
  Set sets[setCount];
  std::set<Index> realSets[setCount];
  for(unsigned int a = 0; a < setCount; a++)
  {
    std::set<Index> chosenIndices;
    unsigned int thisCount = rand() % choiceCount;
    if(thisCount == 0)
      thisCount = 1;
    
    for(unsigned int b = 0; b < thisCount; b++)
    {
      Index choose = (rand() % itemCount) + 1;
      while(chosenIndices.find(choose) != chosenIndices.end()) {
        choose = (rand() % itemCount) + 1;
      }
      
      clock_t c = clock();
      chosenIndices.insert(chosenIndices.end(), choose);
      genericClockTime += clock() - c;
    }

    clock_t c = clock();
    sets[a] = rep.createSet(chosenIndices);
    repositoryClockTime += clock() - c;
    
    realSets[a] = chosenIndices;

    std::set<Index> tempSet = sets[a].stdSet();

    if(tempSet != realSets[a]) {
      QString dbg = "created set: ";
      for(std::set<Index>::const_iterator it = realSets[a].begin(); it != realSets[a].end(); ++it)
        dbg += QString("%1 ").arg(*it);
      kDebug() << dbg;

      dbg = "repo.   set: ";
      for(std::set<Index>::const_iterator it = tempSet.begin(); it != tempSet.end(); ++it)
        dbg += QString("%1 ").arg(*it);
      kDebug() << dbg;

      kDebug() << "DOT-Graph:\n\n" << sets[a].dumpDotGraph() << "\n\n";
    }
    Q_ASSERT(tempSet == realSets[a]);
  }

  for(int cycle = 0; cycle < 100; ++cycle) {
    for(unsigned int a = 0; a < setCount; a++) {
      for(unsigned int b = 0; b < setCount; b++) {
        /// ----- SUBTRACTION/DIFFERENCE
        std::set<Index> _realDifference;
        clock_t c = clock();
        std::set_difference(realSets[a].begin(), realSets[a].end(), realSets[b].begin(), realSets[b].end(), std::insert_iterator<std::set<Index> >(_realDifference, _realDifference.begin()));
        genericDifferenceClockTime += clock() - c;

        c = clock();
        Set _difference = sets[a] - sets[b];
        repositoryDifferenceClockTime += clock() - c;

        if(_difference.stdSet() != _realDifference)
        {
          {
            kDebug() << "SET a:";
            QString dbg = "";
            for(std::set<Index>::const_iterator it = realSets[a].begin(); it != realSets[a].end(); ++it)
              dbg += QString("%1 ").arg(*it);
            kDebug() << dbg;

            kDebug() << "DOT-Graph:\n\n" << sets[a].dumpDotGraph() << "\n\n";
          }
          {
            kDebug() << "SET b:";
            QString dbg = "";
            for(std::set<Index>::const_iterator it = realSets[b].begin(); it != realSets[b].end(); ++it)
              dbg += QString("%1 ").arg(*it);
            kDebug() << dbg;

            kDebug() << "DOT-Graph:\n\n" << sets[b].dumpDotGraph() << "\n\n";
          }

          {
            std::set<Index> tempSet = _difference.stdSet();
            
            kDebug() << "SET difference:";
            QString dbg = "real    set: ";
            for(std::set<Index>::const_iterator it = _realDifference.begin(); it != _realDifference.end(); ++it)
              dbg += QString("%1 ").arg(*it);
            kDebug() << dbg;

            dbg = "repo.   set: ";
            for(std::set<Index>::const_iterator it = tempSet.begin(); it != tempSet.end(); ++it)
              dbg += QString("%1 ").arg(*it);
            kDebug() << dbg;
            
            kDebug() << "DOT-Graph:\n\n" << _difference.dumpDotGraph() << "\n\n";
          }
        }        
        Q_ASSERT(_difference.stdSet() == _realDifference);


        /// ------ UNION
        
        std::set<Index> _realUnion;
        c = clock();
        std::set_union(realSets[a].begin(), realSets[a].end(), realSets[b].begin(), realSets[b].end(), std::insert_iterator<std::set<Index> >(_realUnion, _realUnion.begin()));
        genericUnionClockTime += clock() - c;

        c = clock();
        Set _union = sets[a] + sets[b];
        repositoryUnionClockTime += clock() - c;

        if(_union.stdSet() != _realUnion)
        {
          {
            kDebug() << "SET a:";
            QString dbg = "";
            for(std::set<Index>::const_iterator it = realSets[a].begin(); it != realSets[a].end(); ++it)
              dbg += QString("%1 ").arg(*it);
            kDebug() << dbg;

            kDebug() << "DOT-Graph:\n\n" << sets[a].dumpDotGraph() << "\n\n";
          }
          {
            kDebug() << "SET b:";
            QString dbg = "";
            for(std::set<Index>::const_iterator it = realSets[b].begin(); it != realSets[b].end(); ++it)
              dbg += QString("%1 ").arg(*it);
            kDebug() << dbg;

            kDebug() << "DOT-Graph:\n\n" << sets[b].dumpDotGraph() << "\n\n";
          }

          {
            std::set<Index> tempSet = _union.stdSet();
            
            kDebug() << "SET union:";
            QString dbg = "real    set: ";
            for(std::set<Index>::const_iterator it = _realUnion.begin(); it != _realUnion.end(); ++it)
              dbg += QString("%1 ").arg(*it);
            kDebug() << dbg;

            dbg = "repo.   set: ";
            for(std::set<Index>::const_iterator it = tempSet.begin(); it != tempSet.end(); ++it)
              dbg += QString("%1 ").arg(*it);
            kDebug() << dbg;
            
            kDebug() << "DOT-Graph:\n\n" << _union.dumpDotGraph() << "\n\n";
          }
        }        
        Q_ASSERT(_union.stdSet() == _realUnion);
      
        std::set<Index> _realIntersection;

        /// -------- INTERSECTION
        c = clock();
        std::set_intersection(realSets[a].begin(), realSets[a].end(), realSets[b].begin(), realSets[b].end(), std::insert_iterator<std::set<Index> >(_realIntersection, _realIntersection.begin()));
        genericIntersectionClockTime += clock() - c;

        //Just for fun: Test how fast QSet intersections are
        QSet<Index> first, second;
        for(std::set<Index>::const_iterator it = realSets[a].begin(); it != realSets[a].end(); ++it)
          first.insert(*it);
        for(std::set<Index>::const_iterator it = realSets[b].begin(); it != realSets[b].end(); ++it)
          second.insert(*it);
        c = clock();
        QSet<Index> i = first.intersect(second);
        qsetIntersectionClockTime += clock() - c;

        c = clock();
        Set _intersection = sets[a] & sets[b];
        repositoryIntersectionClockTime += clock() - c;

        
        if(_intersection.stdSet() != _realIntersection)
        {
          {
            kDebug() << "SET a:";
            QString dbg = "";
            for(std::set<Index>::const_iterator it = realSets[a].begin(); it != realSets[a].end(); ++it)
              dbg += QString("%1 ").arg(*it);
            kDebug() << dbg;

            kDebug() << "DOT-Graph:\n\n" << sets[a].dumpDotGraph() << "\n\n";
          }
          {
            kDebug() << "SET b:";
            QString dbg = "";
            for(std::set<Index>::const_iterator it = realSets[b].begin(); it != realSets[b].end(); ++it)
              dbg += QString("%1 ").arg(*it);
            kDebug() << dbg;

            kDebug() << "DOT-Graph:\n\n" << sets[b].dumpDotGraph() << "\n\n";
          }

          {
            std::set<Index> tempSet = _intersection.stdSet();
            
            kDebug() << "SET intersection:";
            QString dbg = "real    set: ";
            for(std::set<Index>::const_iterator it = _realIntersection.begin(); it != _realIntersection.end(); ++it)
              dbg += QString("%1 ").arg(*it);
            kDebug() << dbg;

            dbg = "repo.   set: ";
            for(std::set<Index>::const_iterator it = tempSet.begin(); it != tempSet.end(); ++it)
              dbg += QString("%1 ").arg(*it);
            kDebug() << dbg;
            
            kDebug() << "DOT-Graph:\n\n" << _intersection.dumpDotGraph() << "\n\n";
          }
        }
        Q_ASSERT(_intersection.stdSet() == _realIntersection);
      }
    }
#ifdef DEBUG_STRINGREPOSITORY
    kDebug() << "cycle " << cycle;
  kDebug() << "Clock-cycles needed for set-building: repository-set: " << repositoryClockTime << " generic-set: " << genericClockTime;
  kDebug() << "Clock-cycles needed for intersection: repository-sets: " << repositoryIntersectionClockTime << " generic-set: " << genericIntersectionClockTime << " QSet: " << qsetIntersectionClockTime;
  kDebug() << "Clock-cycles needed for union: repository-sets: " << repositoryUnionClockTime << " generic-set: " << genericUnionClockTime;
  kDebug() << "Clock-cycles needed for difference: repository-sets: " << repositoryDifferenceClockTime << " generic-set: " << genericDifferenceClockTime;
#endif
  }

}

void TestDUChain::release(DUContext* top)
{
  KDevelop::EditorIntegrator::releaseTopRange(top->textRangePtr());
  if(dynamic_cast<TopDUContext*>(top))
    DUChain::self()->removeDocumentChain(static_cast<TopDUContext*>(top)->identity());
  //delete top;
}

DUContext* TestDUChain::parse(const QByteArray& unit, DumpAreas dump)
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
  HashedString url(QString("file:///internal/%1").arg(testNumber++));

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
    DUChainWriteLocker lock(DUChain::lock());
    DumpTypes dt;
    foreach (const AbstractType::Ptr& type, definitionBuilder.topTypes())
      dt.dump(type.data());
  }

  if (dump)
    kDebug(9007) << "===== Finished test case.";

  delete session;

  return top;
}

#include "test_duchain.moc"
