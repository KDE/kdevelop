/* This file is part of KDevelop
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

#include <QtTest/QtTest>
#include <QFlags>

#include "duchain.h"
#include "ducontext.h"
#include "dubuilder.h"
#include "typesystem.h"
#include "definition.h"
#include "documentrange.h"
#include "editorintegrator.h"
#include "dubuilder.h"

#include "parser.h"
#include "control.h"
#include "dumptree.h"
#include "tokens.h"
#include "testconfig.h"

using namespace KTextEditor;

class TestDUChain : public QObject
{
  Q_OBJECT

  // Parser
  Control control;
  DumpTree dumper;

  // Definition - use chain
  TypeEnvironment types;
  const AbstractType* type1;
  const AbstractType* type2;
  const AbstractType* type3;
  Definition* definition1;
  Definition* definition2;
  Definition* definition3;
  Definition* definition4;
  Definition* noDef;
  KUrl file1, file2;
  DUContext* topContext;

public:
  TestDUChain()
  {
  }

private slots:

  void initTestCase()
  {
    type1 = types.integralType(types.intern("definitionChain"));
    type2 = types.referenceType(type1);
    type3 = types.pointerType(type1);

    definition1 = new Definition(new DocumentRange(file1, Range(4,4,4,16)), Definition::LocalScope);
    definition1->setType(type1);
    definition1->setIdentifier("lazy");

    definition2 = new Definition(new DocumentRange(file1, Range(5,4,5,16)), Definition::ClassScope);
    definition2->setType(type2);
    definition2->setIdentifier("m_errorCode");

    definition3 = new Definition(new DocumentRange(file1, Range(6,4,6,16)), Definition::GlobalScope);
    definition3->setType(type3);
    definition3->setIdentifier("lazy");

    definition4 = new Definition(new DocumentRange(file1, Range(7,4,7,16)), Definition::ClassScope);
    definition4->setType(type2);
    definition4->setIdentifier("m_errorCode2");

    noDef = 0;

    file1 = "file:///opt/kde4/src/kdevelop/languages/cpp/parser/duchain.cpp";
    file2 = "file:///opt/kde4/src/kdevelop/languages/cpp/parser/dubuilder.cpp";

    topContext = new DUContext(new DocumentRange(file1, Range(0,0,25,0)));
    DUChain::self()->addDocumentChain(file1, topContext);
  }

  void cleanupTestCase()
  {
    /*delete type1;
    delete type2;
    delete type3;*/

    delete definition1;
    delete definition2;
    delete definition3;
    delete definition4;

    delete topContext;
  }

  void testContextRelationships()
  {
    QCOMPARE(DUChain::self()->chainForDocument(file1), topContext);

    DUContext* firstChild = new DUContext(new DocumentRange(file1, Range(4,4, 10,3)));
    topContext->addChildContext(firstChild);

    QCOMPARE(firstChild->parentContexts().count(), 1);
    QCOMPARE(firstChild->parentContexts().first(), topContext);
    QCOMPARE(firstChild->childContexts().count(), 0);
    QCOMPARE(topContext->childContexts().count(), 1);
    QCOMPARE(topContext->childContexts().last(), firstChild);

    DUContext* secondChild = new DUContext(new DocumentRange(file1, Range(14,4, 19,3)));
    topContext->addChildContext(secondChild);

    QCOMPARE(topContext->childContexts().count(), 2);
    QCOMPARE(topContext->childContexts()[1], secondChild);

    DUContext* thirdChild = new DUContext(new DocumentRange(file1, Range(10,4, 14,3)));
    topContext->addChildContext(thirdChild);

    QCOMPARE(topContext->childContexts().count(), 3);
    QCOMPARE(topContext->childContexts()[1], thirdChild);

    delete topContext->takeChildContext(firstChild);
    QCOMPARE(topContext->childContexts().count(), 2);
    QCOMPARE(topContext->childContexts(), QList<DUContext*>() << thirdChild << secondChild);

    topContext->deleteChildContextsRecursively();
    QVERIFY(topContext->childContexts().isEmpty());
  }

  void testLocalDefinitions()
  {
    QCOMPARE(definition1->type(), type1);
    QCOMPARE(definition1->identifier(), QString("lazy"));
    QCOMPARE(definition1->scope(), Definition::LocalScope);
    QCOMPARE(topContext->localDefinitions().count(), 0);

    topContext->addDefinition(definition1);
    QCOMPARE(topContext->localDefinitions().count(), 1);
    QCOMPARE(topContext->localDefinitions()[0], definition1);

    topContext->addDefinition(definition2);
    QCOMPARE(topContext->localDefinitions().count(), 2);
    QCOMPARE(topContext->localDefinitions()[1], definition2);

    /*kDebug() << k_funcinfo << "Warning expected here (bug if not present)." << endl;
    topContext->addDefinition(definition2);
    QCOMPARE(topContext->localDefinitions().count(), 2);*/

    topContext->clearLocalDefinitions();
    QVERIFY(topContext->localDefinitions().isEmpty());
  }

  void testDefinitionChain()
  {
    topContext->addDefinition(definition1);
    topContext->addDefinition(definition2);

    DUContext* context1 = new DUContext(new DocumentRange(file1, Range(4,4, 14,3)));
    topContext->addChildContext(context1);
    DocumentCursor insideContext1(file1, KTextEditor::Cursor(5,9));

    QCOMPARE(topContext->findContext(insideContext1), context1);
    QCOMPARE(topContext->findDefinition(definition1->identifier(), insideContext1), definition1);
    QCOMPARE(context1->findDefinition(definition1->identifier(), insideContext1), definition1);

    context1->addDefinition(definition3);
    QCOMPARE(topContext->findDefinition(definition1->identifier(), insideContext1), definition1);
    QCOMPARE(context1->findDefinition(definition1->identifier(), insideContext1), definition3);

    context1->takeDefinition(definition3);
    QCOMPARE(topContext->findDefinition(definition1->identifier(), insideContext1), definition1);
    QCOMPARE(context1->findDefinition(definition1->identifier(), insideContext1), definition1);

    DUContext* subContext1 = new DUContext(new DocumentRange(file1, Range(5,4, 7,3)));
    topContext->addChildContext(subContext1);

    DUContext* subContext2 = new DUContext(new DocumentRange(file1, Range(9,4, 12,3)));
    topContext->addChildContext(subContext2);

    subContext1->addDefinition(definition3);

    subContext2->addDefinition(definition4);

    subContext1->takeDefinition(definition3);
    subContext2->takeDefinition(definition4);
    topContext->takeDefinition(definition1);
    topContext->takeDefinition(definition2);

    topContext->deleteChildContextsRecursively();
  }

  void testDeclareInt()
  {
    QByteArray method("int i;");

    DUContext* top = parse(method, DumpNone);

    QCOMPARE(top->parentContexts().count(), 0);
    QCOMPARE(top->childContexts().count(), 0);
    QCOMPARE(top->localDefinitions().count(), 1);
    QVERIFY(top->localScopeIdentifier().isEmpty());

    Definition* def = top->localDefinitions().first();
    QCOMPARE(def->identifier(), QString("i"));
    QCOMPARE(top->findDefinition("i"), def);

    QCOMPARE(top->findDefinition("i"), def);

    release(top);
  }

  void testDeclareFunction()
  {
    //                 0         1         2
    //                 012345678901234567890123456789
    QByteArray method("void A::t(int i) { i = i + 3; }");

    DUContext* top = parse(method, DumpNone);

    QCOMPARE(top->parentContexts().count(), 0);
    QCOMPARE(top->childContexts().count(), 1);
    QCOMPARE(top->localDefinitions().count(), 1);
    QVERIFY(top->localScopeIdentifier().isEmpty());

    Definition* defAT = top->localDefinitions().first();
    QCOMPARE(defAT->identifier(), QString("A::t"));

    QCOMPARE(top->findDefinition("A::t"), defAT);
    QCOMPARE(top->findDefinition("i"), noDef);

    DUContext* fn = top->childContexts().first();
    QCOMPARE(fn->childContexts().count(), 1);
    QCOMPARE(fn->localDefinitions().count(), 1);
    QCOMPARE(fn->localScopeIdentifier(), QString("A::t"));

    Definition* defI = fn->localDefinitions().first();
    QCOMPARE(defI->identifier(), QString("i"));
    QCOMPARE(defI->uses().count(), 2);

    QCOMPARE(fn->findDefinition("A::t"), defAT);
    QCOMPARE(fn->findDefinition("i"), defI);

    DUContext* insideFn = fn->childContexts().first();
    QCOMPARE(insideFn->childContexts().count(), 0);
    QCOMPARE(insideFn->localDefinitions().count(), 0);
    QVERIFY(insideFn->localScopeIdentifier().isEmpty());

    QCOMPARE(insideFn->findDefinition("A::t"), defAT);
    QCOMPARE(insideFn->findDefinition("i"), defI);

    release(top);
  }

  void testDeclareFor()
  {
    //                 0         1         2         3         4         5
    //                 012345678901234567890123456789012345678901234567890123456789
    QByteArray method("int main() { for (int i = 0; i < 10; i++) {} }");

    DUContext* top = parse(method, DumpNone);

    QCOMPARE(top->parentContexts().count(), 0);
    QCOMPARE(top->childContexts().count(), 1);
    QCOMPARE(top->localDefinitions().count(), 1);
    QVERIFY(top->localScopeIdentifier().isEmpty());

    Definition* defMain = top->localDefinitions().first();
    QCOMPARE(defMain->identifier(), QString("main"));

    QCOMPARE(top->findDefinition("main"), defMain);
    QCOMPARE(top->findDefinition("i"), noDef);

    DUContext* main = top->childContexts().first();
    QCOMPARE(main->childContexts().count(), 1);
    QCOMPARE(main->localDefinitions().count(), 0);
    QCOMPARE(main->localScopeIdentifier(), QString("main"));

    QCOMPARE(main->findDefinition("main"), defMain);
    QCOMPARE(main->findDefinition("i"), noDef);

    DUContext* forCtx = main->childContexts().first();
    QCOMPARE(forCtx->childContexts().count(), 1);
    QCOMPARE(forCtx->localDefinitions().count(), 1);
    QVERIFY(forCtx->localScopeIdentifier().isEmpty());

    Definition* defI = forCtx->localDefinitions().first();
    QCOMPARE(defI->identifier(), QString("i"));
    QCOMPARE(defI->uses().count(), 2);

    QCOMPARE(forCtx->findDefinition("main"), defMain);
    QCOMPARE(forCtx->findDefinition("i"), defI);

    DUContext* insideFor = forCtx->childContexts().first();
    QCOMPARE(insideFor->childContexts().count(), 0);
    QCOMPARE(insideFor->localDefinitions().count(), 0);
    QVERIFY(insideFor->localScopeIdentifier().isEmpty());

    QCOMPARE(insideFor->findDefinition("main"), defMain);
    QCOMPARE(insideFor->findDefinition("i"), defI);

    release(top);
  }

  void testDeclareStruct()
  {
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("struct A { int i; A(int b, int c) : i(5) { } virtual void test(int j) = 0; };");

    // FIXME test scope indentifiers... they're wrong currently

    DUContext* top = parse(method, DumpNone);

    QCOMPARE(top->parentContexts().count(), 0);
    QCOMPARE(top->childContexts().count(), 1);
    QCOMPARE(top->localDefinitions().count(), 1);
    QVERIFY(top->localScopeIdentifier().isEmpty());

    Definition* defA = top->localDefinitions().first();
    QCOMPARE(defA->identifier(), QString("A"));

    QCOMPARE(top->findDefinition("A"), defA);
    QCOMPARE(top->findDefinition("i"), noDef);
    QCOMPARE(top->findDefinition("b"), noDef);
    QCOMPARE(top->findDefinition("c"), noDef);

    DUContext* structA = top->childContexts().first();
    QCOMPARE(structA->childContexts().count(), 1);
    // FIXME AST limitation... pure virtual functions are not shown as functions, thus we can't properly scope int j
    QCOMPARE(structA->localDefinitions().count(), 4);//3);
    QCOMPARE(structA->localScopeIdentifier(), QString("A"));

    Definition* defI = structA->localDefinitions().first();
    QCOMPARE(defI->identifier(), QString("i"));
    QCOMPARE(defI->uses().count(), 1);

    Definition* defACtor = structA->localDefinitions()[1];
    QCOMPARE(defACtor->identifier(), QString("A"));
    QCOMPARE(defACtor->uses().count(), 0);

    Definition* defTest = structA->localDefinitions()[2];
    QCOMPARE(defTest->identifier(), QString("test"));
    QCOMPARE(defTest->uses().count(), 0);

    QCOMPARE(structA->findDefinition("A"), defACtor);
    QCOMPARE(structA->findDefinition("i"), defI);
    QCOMPARE(structA->findDefinition("b"), noDef);
    QCOMPARE(structA->findDefinition("c"), noDef);

    DUContext* ctorCtx = structA->childContexts().first();
    QCOMPARE(ctorCtx->childContexts().count(), 1);
    QCOMPARE(ctorCtx->localDefinitions().count(), 2);
    QCOMPARE(ctorCtx->localScopeIdentifier(), QString("A"));

    QCOMPARE(ctorCtx->findDefinition("A"), defACtor);
    QCOMPARE(ctorCtx->findDefinition("i"), defI);
    QCOMPARE(ctorCtx->findDefinition("b"), ctorCtx->localDefinitions().first());
    QCOMPARE(ctorCtx->findDefinition("c"), ctorCtx->localDefinitions().last());

    DUContext* insideCtorCtx = ctorCtx->childContexts().first();
    QCOMPARE(insideCtorCtx->childContexts().count(), 0);
    QCOMPARE(insideCtorCtx->localDefinitions().count(), 0);
    QVERIFY(insideCtorCtx->localScopeIdentifier().isEmpty());

    release(top);
  }

  void testDeclareNamespace()
  {
    QByteArray method("namespace foo { int bar; }");

    DUContext* top = parse(method);//, DumpNone);

    QCOMPARE(top->parentContexts().count(), 0);
    QCOMPARE(top->childContexts().count(), 1);
    QCOMPARE(top->localDefinitions().count(), 0);
    QVERIFY(top->localScopeIdentifier().isEmpty());
    QCOMPARE(top->findDefinition("foo"), noDef);

    DUContext* fooNS = top->childContexts().first();
    QCOMPARE(fooNS->childContexts().count(), 0);
    QCOMPARE(fooNS->localDefinitions().count(), 1);
    QCOMPARE(fooNS->localScopeIdentifier(), QString("foo"));

    Definition* bar = fooNS->localDefinitions().first();
    QCOMPARE(bar->identifier(), QString("bar"));
    QCOMPARE(bar->uses().count(), 0);
    QCOMPARE(fooNS->findDefinition(bar->identifier()), bar);

    release(top);
  }

public:
  enum DumpType {
    DumpNone = 0,
    DumpAST = 1,
    DumpDUChain = 2
  };
  Q_DECLARE_FLAGS(DumpTypes, DumpType)

private:
  DUContext* parse(const QByteArray& unit, DumpTypes dump = static_cast<DumpTypes>(DumpAST | DumpDUChain));

  void release(DUContext* top)
  {
    EditorIntegrator::deleteTopRange(top->takeTextRange());
  }
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TestDUChain::DumpTypes)

DUContext* TestDUChain::parse(const QByteArray& unit, DumpTypes dump)
{
  if (dump)
    kDebug() << "==== Beginning new test case...:" << endl << unit << endl << endl;

  pool mem_pool;

  Parser parser(&control);
  TranslationUnitAST* ast = parser.parse(unit.constData(), unit.size() + 1, &mem_pool);

  EditorIntegrator::addParsedSource(&parser.lexer, &parser.token_stream);

  if (dump & DumpAST) {
    kDebug() << "===== AST:" << endl;
    dumper.dump(ast, &parser.token_stream);
  }

  DUBuilder dubuilder(&parser.token_stream);
  static int testNumber = 0;
  DUContext* top = dubuilder.build(KUrl(QString("file:///internal/%1").arg(testNumber++)), ast);

  if (dump & DumpDUChain) {
    kDebug() << "===== DUChain:" << endl;
    dumper.dump(top);
  }

  if (dump)
    kDebug() << "===== Finished test case." << endl;

  return top;
}

#include "test_duchain.moc"

QTEST_MAIN(TestDUChain)
