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
#include "kdevdocumentrange.h"
#include "cppeditorintegrator.h"
#include "dubuilder.h"

#include "parser.h"
#include "control.h"
#include "dumptree.h"
#include "tokens.h"
#include "testconfig.h"

#include "rpp/preprocessor.h"

using namespace KTextEditor;

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
}

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

    definition1 = new Definition(new KDevDocumentRange(file1, Range(4,4,4,16)), Definition::LocalScope);
    definition1->setType(type1);
    definition1->setIdentifier(Identifier("lazy"));

    definition2 = new Definition(new KDevDocumentRange(file1, Range(5,4,5,16)), Definition::ClassScope);
    definition2->setType(type2);
    definition2->setIdentifier(Identifier("m_errorCode"));

    definition3 = new Definition(new KDevDocumentRange(file1, Range(6,4,6,16)), Definition::GlobalScope);
    definition3->setType(type3);
    definition3->setIdentifier(Identifier("lazy"));

    definition4 = new Definition(new KDevDocumentRange(file1, Range(7,4,7,16)), Definition::ClassScope);
    definition4->setType(type2);
    definition4->setIdentifier(Identifier("m_errorCode2"));

    noDef = 0;

    file1 = "file:///opt/kde4/src/kdevelop/languages/cpp/parser/duchain.cpp";
    file2 = "file:///opt/kde4/src/kdevelop/languages/cpp/parser/dubuilder.cpp";

    topContext = new DUContext(new KDevDocumentRange(file1, Range(0,0,25,0)));
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

  void testIdentifiers()
  {
    QualifiedIdentifier at("A::t");
    QualifiedIdentifier at2;
    at2.push(Identifier("A"));
    at2.push(Identifier("t"));
    QCOMPARE(at, at2);
    QVERIFY(at.isQualified());

    QualifiedIdentifier global("::test::foo()");
    QVERIFY(global.explicitlyGlobal());
    QVERIFY(global.isQualified());

    QualifiedIdentifier unqualified("unqualified");
    QVERIFY(!unqualified.isQualified());
  }

  void testContextRelationships()
  {
    QCOMPARE(DUChain::self()->chainForDocument(file1), topContext);

    DUContext* firstChild = new DUContext(new KDevDocumentRange(file1, Range(4,4, 10,3)));
    topContext->addChildContext(firstChild);

    QCOMPARE(firstChild->parentContexts().count(), 1);
    QCOMPARE(firstChild->parentContexts().first(), topContext);
    QCOMPARE(firstChild->childContexts().count(), 0);
    QCOMPARE(topContext->childContexts().count(), 1);
    QCOMPARE(topContext->childContexts().last(), firstChild);

    DUContext* secondChild = new DUContext(new KDevDocumentRange(file1, Range(14,4, 19,3)));
    topContext->addChildContext(secondChild);

    QCOMPARE(topContext->childContexts().count(), 2);
    QCOMPARE(topContext->childContexts()[1], secondChild);

    DUContext* thirdChild = new DUContext(new KDevDocumentRange(file1, Range(10,4, 14,3)));
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
    QCOMPARE(definition1->identifier(), Identifier("lazy"));
    QCOMPARE(definition1->scope(), Definition::LocalScope);
    QCOMPARE(topContext->localDefinitions().count(), 0);

    topContext->addDefinition(definition1);
    QCOMPARE(topContext->localDefinitions().count(), 1);
    QCOMPARE(topContext->localDefinitions()[0], definition1);

    topContext->addDefinition(definition2);
    QCOMPARE(topContext->localDefinitions().count(), 2);
    QCOMPARE(topContext->localDefinitions()[1], definition2);

    //kDebug() << k_funcinfo << "Warning expected here (bug if not present)." << endl;
    //topContext->addDefinition(definition2);
    //QCOMPARE(topContext->localDefinitions().count(), 2);

    topContext->clearLocalDefinitions();
    QVERIFY(topContext->localDefinitions().isEmpty());
  }

  void testDefinitionChain()
  {
    topContext->addDefinition(definition1);
    topContext->addDefinition(definition2);

    DUContext* context1 = new DUContext(new KDevDocumentRange(file1, Range(4,4, 14,3)));
    topContext->addChildContext(context1);
    KDevDocumentCursor insideContext1(file1, Cursor(5,9));

    QCOMPARE(topContext->findContext(insideContext1), context1);
    QCOMPARE(topContext->findDefinition(definition1->identifier(), insideContext1), definition1);
    QCOMPARE(context1->findDefinition(definition1->identifier(), insideContext1), definition1);

    context1->addDefinition(definition3);
    QCOMPARE(topContext->findDefinition(definition1->identifier(), insideContext1), definition1);
    QCOMPARE(context1->findDefinition(definition1->identifier(), insideContext1), definition3);

    context1->takeDefinition(definition3);
    QCOMPARE(topContext->findDefinition(definition1->identifier(), insideContext1), definition1);
    QCOMPARE(context1->findDefinition(definition1->identifier(), insideContext1), definition1);

    DUContext* subContext1 = new DUContext(new KDevDocumentRange(file1, Range(5,4, 7,3)));
    topContext->addChildContext(subContext1);

    DUContext* subContext2 = new DUContext(new KDevDocumentRange(file1, Range(9,4, 12,3)));
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
    QCOMPARE(def->identifier(), Identifier("i"));
    QCOMPARE(top->findDefinition(def->identifier()), def);

    release(top);
  }

  void testDeclareFunction()
  {
    QSKIP("need to hook up function definition and declaration first", SkipSingle);

    //                 0         1         2
    //                 012345678901234567890123456789
    QByteArray method("class A { void t(int i); }; void A::t(int i) { i = i + 3; }");

    DUContext* top = parse(method, DumpNone);

    QCOMPARE(top->parentContexts().count(), 0);
    QCOMPARE(top->childContexts().count(), 1);
    QCOMPARE(top->localDefinitions().count(), 1);
    QVERIFY(top->localScopeIdentifier().isEmpty());

    Definition* defAT = top->localDefinitions().first();
    QCOMPARE(defAT->identifier(), Identifier("t"));

    QCOMPARE(top->findDefinition(defAT->identifier()), defAT);
    QCOMPARE(top->findDefinition(Identifier("i")), noDef);

    DUContext* fn = top->childContexts().first();
    QCOMPARE(fn->childContexts().count(), 1);
    QCOMPARE(fn->localDefinitions().count(), 1);
    QVERIFY(fn->localScopeIdentifier().isEmpty());

    Definition* defI = fn->localDefinitions().first();
    QCOMPARE(defI->identifier(), Identifier("i"));
    QCOMPARE(defI->uses().count(), 2);

    QCOMPARE(fn->findDefinition(defAT->identifier()), defAT);
    QCOMPARE(fn->findDefinition(defI->identifier()), defI);

    DUContext* insideFn = fn->childContexts().first();
    QCOMPARE(insideFn->childContexts().count(), 0);
    QCOMPARE(insideFn->localDefinitions().count(), 0);
    QVERIFY(insideFn->localScopeIdentifier().isEmpty());

    QCOMPARE(insideFn->findDefinition(defAT->identifier()), defAT);
    QCOMPARE(insideFn->findDefinition(defI->identifier()), defI);

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
    QCOMPARE(defMain->identifier(), Identifier("main"));

    QCOMPARE(top->findDefinition(defMain->identifier()), defMain);
    QCOMPARE(top->findDefinition(Identifier("i")), noDef);

    DUContext* main = top->childContexts().first();
    QCOMPARE(main->childContexts().count(), 1);
    QCOMPARE(main->localDefinitions().count(), 0);
    QVERIFY(main->localScopeIdentifier().isEmpty());

    QCOMPARE(main->findDefinition(defMain->identifier()), defMain);
    QCOMPARE(main->findDefinition(Identifier("i")), noDef);

    DUContext* forCtx = main->childContexts().first();
    QCOMPARE(forCtx->childContexts().count(), 1);
    QCOMPARE(forCtx->localDefinitions().count(), 1);
    QVERIFY(forCtx->localScopeIdentifier().isEmpty());

    Definition* defI = forCtx->localDefinitions().first();
    QCOMPARE(defI->identifier(), Identifier("i"));
    QCOMPARE(defI->uses().count(), 2);

    QCOMPARE(forCtx->findDefinition(defMain->identifier()), defMain);
    QCOMPARE(forCtx->findDefinition(defI->identifier()), defI);

    DUContext* insideFor = forCtx->childContexts().first();
    QCOMPARE(insideFor->childContexts().count(), 0);
    QCOMPARE(insideFor->localDefinitions().count(), 0);
    QVERIFY(insideFor->localScopeIdentifier().isEmpty());

    QCOMPARE(insideFor->findDefinition(defMain->identifier()), defMain);
    QCOMPARE(insideFor->findDefinition(defI->identifier()), defI);

    release(top);
  }

  void testDeclareStruct()
  {
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("struct A { int i; A(int b, int c) : i(5) { } virtual void test(int j) = 0; };");

    DUContext* top = parse(method, DumpNone);

    QCOMPARE(top->parentContexts().count(), 0);
    QCOMPARE(top->childContexts().count(), 1);
    QCOMPARE(top->localDefinitions().count(), 1);
    QVERIFY(top->localScopeIdentifier().isEmpty());

    Definition* defA = top->localDefinitions().first();
    QCOMPARE(defA->identifier(), Identifier("A"));

    QCOMPARE(top->findDefinition(Identifier("A")), defA);
    QCOMPARE(top->findDefinition(Identifier("i")), noDef);
    QCOMPARE(top->findDefinition(Identifier("b")), noDef);
    QCOMPARE(top->findDefinition(Identifier("c")), noDef);

    DUContext* structA = top->childContexts().first();
    QCOMPARE(structA->childContexts().count(), 2);
    QCOMPARE(structA->localDefinitions().count(), 3);
    QCOMPARE(structA->localScopeIdentifier(), QualifiedIdentifier("A"));

    Definition* defI = structA->localDefinitions().first();
    QCOMPARE(defI->identifier(), Identifier("i"));
    QCOMPARE(defI->uses().count(), 1);

    Definition* defACtor = structA->localDefinitions()[1];
    QCOMPARE(defACtor->identifier(), Identifier("A"));
    QCOMPARE(defACtor->uses().count(), 0);

    Definition* defTest = structA->localDefinitions()[2];
    QCOMPARE(defTest->identifier(), Identifier("test"));
    QCOMPARE(defTest->uses().count(), 0);

    QCOMPARE(structA->findDefinition(QualifiedIdentifier("::A")), defA);
    QCOMPARE(structA->findDefinition(Identifier("A")), defACtor);
    QCOMPARE(structA->findDefinition(Identifier("i")), defI);
    QCOMPARE(structA->findDefinition(Identifier("b")), noDef);
    QCOMPARE(structA->findDefinition(Identifier("c")), noDef);

    DUContext* ctorCtx = structA->childContexts().first();
    QCOMPARE(ctorCtx->childContexts().count(), 1);
    QCOMPARE(ctorCtx->localDefinitions().count(), 2);
    QVERIFY(ctorCtx->localScopeIdentifier().isEmpty());

    QCOMPARE(ctorCtx->findDefinition(QualifiedIdentifier("A")), defACtor);
    QCOMPARE(ctorCtx->findDefinition(QualifiedIdentifier("i")), defI);
    QCOMPARE(ctorCtx->findDefinition(QualifiedIdentifier("b")), ctorCtx->localDefinitions().first());
    QCOMPARE(ctorCtx->findDefinition(QualifiedIdentifier("c")), ctorCtx->localDefinitions().last());

    DUContext* testCtx = structA->childContexts().last();
    QCOMPARE(testCtx->childContexts().count(), 0);
    QCOMPARE(testCtx->localDefinitions().count(), 1);
    QVERIFY(testCtx->localScopeIdentifier().isEmpty());

    Definition* defJ = testCtx->localDefinitions().first();
    QCOMPARE(defJ->identifier(), Identifier("j"));
    QCOMPARE(defJ->uses().count(), 0);

    DUContext* insideCtorCtx = ctorCtx->childContexts().first();
    QCOMPARE(insideCtorCtx->childContexts().count(), 0);
    QCOMPARE(insideCtorCtx->localDefinitions().count(), 0);
    QVERIFY(insideCtorCtx->localScopeIdentifier().isEmpty());

    release(top);
  }

  void testDeclareNamespace()
  {
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("namespace foo { int bar; int test() { return bar; } } int test() { return foo::bar; }");

    DUContext* top = parse(method, DumpNone);

    QCOMPARE(top->parentContexts().count(), 0);
    QCOMPARE(top->childContexts().count(), 2);
    QCOMPARE(top->localDefinitions().count(), 1);
    QVERIFY(top->localScopeIdentifier().isEmpty());
    QCOMPARE(top->findDefinition(Identifier("foo")), noDef);

    QCOMPARE(top->usingNamespaces().count(), 0);

    DUContext* fooCtx = top->childContexts().first();
    QCOMPARE(fooCtx->childContexts().count(), 1);
    QCOMPARE(fooCtx->localDefinitions().count(), 2);
    QCOMPARE(fooCtx->localScopeIdentifier(), QualifiedIdentifier("foo"));
    QCOMPARE(fooCtx->scopeIdentifier(), QualifiedIdentifier("foo"));

    Definition* bar = fooCtx->localDefinitions().first();
    QCOMPARE(bar->identifier(), Identifier("bar"));
    QCOMPARE(bar->qualifiedIdentifier(), QualifiedIdentifier("foo::bar"));
    QCOMPARE(bar->uses().count(), 2);
    QCOMPARE(top->findDefinition(bar->identifier()), noDef);
    QCOMPARE(top->findDefinition(bar->qualifiedIdentifier()), bar);

    Definition* test = fooCtx->localDefinitions()[1];
    QCOMPARE(test->identifier(), Identifier("test"));
    QCOMPARE(test->qualifiedIdentifier(), QualifiedIdentifier("foo::test"));
    QCOMPARE(test->uses().count(), 0);

    Definition* test2 = top->localDefinitions().first();
    QCOMPARE(test2->identifier(), Identifier("test"));
    QCOMPARE(test2->qualifiedIdentifier(), QualifiedIdentifier("test"));
    QCOMPARE(test2->uses().count(), 0);

    QCOMPARE(top->findDefinition(QualifiedIdentifier("test")), test2);
    QCOMPARE(top->findDefinition(QualifiedIdentifier("::test")), test2);
    QCOMPARE(top->findDefinition(QualifiedIdentifier("foo::test")), test);
    QCOMPARE(top->findDefinition(QualifiedIdentifier("::foo::test")), test);

    DUContext* testCtx = fooCtx->childContexts().first();
    QCOMPARE(testCtx->childContexts().count(), 1);
    QCOMPARE(testCtx->localDefinitions().count(), 0);
    QVERIFY(testCtx->localScopeIdentifier().isEmpty());

    DUContext* testfnCtx = testCtx->childContexts().first();
    QCOMPARE(testfnCtx->childContexts().count(), 0);
    QCOMPARE(testfnCtx->localDefinitions().count(), 0);
    QVERIFY(testfnCtx->localScopeIdentifier().isEmpty());

    release(top);
  }

  void testDeclareUsingNamespace()
  {
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("namespace foo { int bar; int test(); } using namespace foo; int test() { return bar; }");

    DUContext* top = parse(method, DumpNone);

    QCOMPARE(top->parentContexts().count(), 0);
    QCOMPARE(top->childContexts().count(), 2);
    // Needs to wait for type system to recognise forward declarations
    QCOMPARE(top->localDefinitions().count(), 1);//0);
    QVERIFY(top->localScopeIdentifier().isEmpty());
    QCOMPARE(top->findDefinition(Identifier("foo")), noDef);

    QCOMPARE(top->usingNamespaces().count(), 1);
    QCOMPARE(top->usingNamespaces().first()->nsIdentifier, QualifiedIdentifier("foo"));
    QCOMPARE(*top->usingNamespaces().first()->origin, Cursor(0, 59));

    DUContext* fooCtx = top->childContexts().first();
    QCOMPARE(fooCtx->childContexts().count(), 1);
    QCOMPARE(fooCtx->localDefinitions().count(), 2);
    QCOMPARE(fooCtx->localScopeIdentifier(), QualifiedIdentifier("foo"));
    QCOMPARE(fooCtx->scopeIdentifier(), QualifiedIdentifier("foo"));

    Definition* bar = fooCtx->localDefinitions().first();
    QCOMPARE(bar->identifier(), Identifier("bar"));
    QCOMPARE(bar->qualifiedIdentifier(), QualifiedIdentifier("foo::bar"));
    QCOMPARE(bar->uses().count(), 1);
    QCOMPARE(top->findDefinition(bar->identifier(), KDevDocumentCursor(top->textRangePtr(), KDevDocumentCursor::Start)), noDef);
    QCOMPARE(top->findDefinition(bar->identifier()), noDef);
    QCOMPARE(top->findDefinition(bar->qualifiedIdentifier()), bar);

    Definition* test = fooCtx->localDefinitions()[1];
    QCOMPARE(test->identifier(), Identifier("test"));
    QCOMPARE(test->qualifiedIdentifier(), QualifiedIdentifier("foo::test"));
    QCOMPARE(test->uses().count(), 0);

    Definition* test2 = top->localDefinitions().first();
    QCOMPARE(test2->identifier(), Identifier("test"));
    // Needs to wait for type system to recognise forward declarations
    QCOMPARE(test2->qualifiedIdentifier(), QualifiedIdentifier("test"));//"foo::test"));
    QCOMPARE(test2->uses().count(), 0);

    QCOMPARE(top->findDefinition(QualifiedIdentifier("test")), test2);
    QCOMPARE(top->findDefinition(QualifiedIdentifier("::test")), test2);
    QCOMPARE(top->findDefinition(QualifiedIdentifier("foo::test")), test);
    QCOMPARE(top->findDefinition(QualifiedIdentifier("::foo::test")), test);

    DUContext* testCtx = fooCtx->childContexts().first();
    QCOMPARE(testCtx->childContexts().count(), 0);
    QCOMPARE(testCtx->localDefinitions().count(), 0);
    QVERIFY(testCtx->localScopeIdentifier().isEmpty());

    release(top);
  }

  void testFileParse()
  {
    QSKIP("Unwanted", SkipSingle);

    QFile file("/opt/kde4/src/kdevelop/languages/csharp/parser/csharp_parser.cpp");
    QVERIFY( file.open( QIODevice::ReadOnly ) );

    QByteArray fileData = file.readAll();
    file.close();
    QString contents = QString::fromUtf8( fileData.constData() );
    Preprocessor preprocessor;
    QString ppd = preprocessor.processString( contents );
    QByteArray preprocessed = ppd.toUtf8();

    DUContext* top = parse(preprocessed, DumpNone);

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
    CppEditorIntegrator::deleteTopRange(top->takeTextRange());
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

  CppEditorIntegrator::addParsedSource(&parser.lexer, &parser.token_stream);

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
