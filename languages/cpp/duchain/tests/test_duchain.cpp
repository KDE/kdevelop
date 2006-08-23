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
#include "topducontext.h"
#include "dubuilder.h"
#include "typesystem.h"
#include "definition.h"
#include "kdevdocumentrange.h"
#include "cppeditorintegrator.h"
#include "dubuilder.h"

#include "parser.h"
#include "control.h"
#include "dumpchain.h"
#include "tokens.h"
#include "parsesession.h"

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
  template<>
  char* toString(const Definition& def)
  {
    QString s = QString("Definition %1 (%2): %3").arg(def.identifier().toString()).arg(def.qualifiedIdentifier().toString()).arg(reinterpret_cast<long>(&def));
    return qstrdup(s.toLatin1().constData());
  }
}

class TestDUChain : public QObject
{
  Q_OBJECT

  // Parser
  Control control;
  DumpChain dumper;

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
  TopDUContext* topContext;

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

    topContext = new TopDUContext(new KDevDocumentRange(file1, Range(0,0,25,0)));
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

    QCOMPARE(firstChild->parentContext(), topContext);
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
    QCOMPARE(*topContext->findDefinition(definition1->identifier(), insideContext1), *definition1);
    QCOMPARE(*context1->findDefinition(definition1->identifier(), insideContext1), *definition1);

    context1->addDefinition(definition3);
    QCOMPARE(*topContext->findDefinition(definition1->identifier(), insideContext1), *definition1);
    QCOMPARE(*context1->findDefinition(definition1->identifier(), insideContext1), *definition1);
    QCOMPARE(*context1->findDefinition(definition1->identifier(), KDevDocumentCursor(definition3->textRangePtr(), KDevDocumentCursor::Start)), *definition3);

    context1->takeDefinition(definition3);
    QCOMPARE(*topContext->findDefinition(definition1->identifier(), insideContext1), *definition1);
    QCOMPARE(*context1->findDefinition(definition1->identifier(), insideContext1), *definition1);

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

    QVERIFY(!top->parentContext());
    QCOMPARE(top->childContexts().count(), 0);
    QCOMPARE(top->localDefinitions().count(), 1);
    QVERIFY(top->localScopeIdentifier().isEmpty());

    Definition* def = top->localDefinitions().first();
    QCOMPARE(def->identifier(), Identifier("i"));
    QCOMPARE(top->findDefinition(def->identifier()), def);

    release(top);
  }

  void testDeclareFor()
  {
    //                 0         1         2         3         4         5
    //                 012345678901234567890123456789012345678901234567890123456789
    QByteArray method("int main() { for (int i = 0; i < 10; i++) { if (i == 4) return; } }");

    DUContext* top = parse(method, DumpNone);

    QVERIFY(!top->parentContext());
    QCOMPARE(top->childContexts().count(), 2);
    QCOMPARE(top->localDefinitions().count(), 0);
    QVERIFY(top->localScopeIdentifier().isEmpty());

    QCOMPARE(top->findDefinition(Identifier("i")), noDef);

    DUContext* main = top->childContexts()[1];
    QVERIFY(main->parentContext());
    QCOMPARE(main->importedParentContexts().count(), 1);
    QCOMPARE(main->childContexts().count(), 2);
    QCOMPARE(main->localDefinitions().count(), 0);
    QVERIFY(main->localScopeIdentifier().isEmpty());

    QCOMPARE(main->findDefinition(Identifier("i")), noDef);

    DUContext* forCtx = main->childContexts()[1];
    QVERIFY(forCtx->parentContext());
    QCOMPARE(forCtx->importedParentContexts().count(), 1);
    QCOMPARE(forCtx->childContexts().count(), 2);
    QCOMPARE(forCtx->localDefinitions().count(), 0);
    QVERIFY(forCtx->localScopeIdentifier().isEmpty());

    DUContext* forParamCtx = forCtx->importedParentContexts().first();
    QVERIFY(forParamCtx->parentContext());
    QCOMPARE(forParamCtx->importedParentContexts().count(), 0);
    QCOMPARE(forParamCtx->childContexts().count(), 0);
    QCOMPARE(forParamCtx->localDefinitions().count(), 1);
    QVERIFY(forParamCtx->localScopeIdentifier().isEmpty());

    Definition* defI = forParamCtx->localDefinitions().first();
    QCOMPARE(defI->identifier(), Identifier("i"));
    QCOMPARE(defI->uses().count(), 3);

    QCOMPARE(forCtx->findDefinition(defI->identifier()), defI);

    DUContext* ifCtx = forCtx->childContexts()[1];
    QVERIFY(ifCtx->parentContext());
    QCOMPARE(ifCtx->importedParentContexts().count(), 1);
    QCOMPARE(ifCtx->childContexts().count(), 0);
    QCOMPARE(ifCtx->localDefinitions().count(), 0);
    QVERIFY(ifCtx->localScopeIdentifier().isEmpty());

    QCOMPARE(ifCtx->findDefinition(defI->identifier()), defI);

    release(top);
  }

  void testDeclareStruct()
  {
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("struct A { int i; A(int b, int c) : i(c) { } virtual void test(int j) = 0; };");

    DUContext* top = parse(method, DumpNone);

    QVERIFY(!top->parentContext());
    QCOMPARE(top->childContexts().count(), 1);
    QCOMPARE(top->localDefinitions().count(), 0);
    QVERIFY(top->localScopeIdentifier().isEmpty());

    DUContext* structA = top->childContexts().first();
    QVERIFY(structA->parentContext());
    QCOMPARE(structA->importedParentContexts().count(), 0);
    QCOMPARE(structA->childContexts().count(), 3);
    QCOMPARE(structA->localDefinitions().count(), 1);
    QCOMPARE(structA->localScopeIdentifier(), QualifiedIdentifier("A"));

    Definition* defI = structA->localDefinitions().first();
    QCOMPARE(defI->identifier(), Identifier("i"));
    QCOMPARE(defI->uses().count(), 1);

    QCOMPARE(structA->findDefinition(Identifier("i")), defI);
    QCOMPARE(structA->findDefinition(Identifier("b")), noDef);
    QCOMPARE(structA->findDefinition(Identifier("c")), noDef);

    DUContext* ctorImplCtx = structA->childContexts()[1];
    QVERIFY(ctorImplCtx->parentContext());
    QCOMPARE(ctorImplCtx->importedParentContexts().count(), 1);
    QCOMPARE(ctorImplCtx->childContexts().count(), 1);
    QCOMPARE(ctorImplCtx->localDefinitions().count(), 0);
    QVERIFY(ctorImplCtx->localScopeIdentifier().isEmpty());

    DUContext* ctorCtx = ctorImplCtx->importedParentContexts().first();
    QVERIFY(ctorCtx->parentContext());
    QCOMPARE(ctorCtx->childContexts().count(), 0);
    QCOMPARE(ctorCtx->localDefinitions().count(), 2);
    QVERIFY(ctorCtx->localScopeIdentifier().isEmpty());

    Definition* defB = ctorCtx->localDefinitions().first();
    QCOMPARE(defB->identifier(), Identifier("b"));
    QCOMPARE(defB->uses().count(), 0);

    Definition* defC = ctorCtx->localDefinitions()[1];
    QCOMPARE(defC->identifier(), Identifier("c"));
    QCOMPARE(defC->uses().count(), 1);

    QCOMPARE(ctorCtx->findDefinition(Identifier("i")), defI);
    QCOMPARE(ctorCtx->findDefinition(Identifier("b")), defB);
    QCOMPARE(ctorCtx->findDefinition(Identifier("c")), defC);

    DUContext* testCtx = structA->childContexts().last();
    QCOMPARE(testCtx->childContexts().count(), 0);
    QCOMPARE(testCtx->localDefinitions().count(), 1);
    QVERIFY(testCtx->localScopeIdentifier().isEmpty());

    Definition* defJ = testCtx->localDefinitions().first();
    QCOMPARE(defJ->identifier(), Identifier("j"));
    QCOMPARE(defJ->uses().count(), 0);

    /*DUContext* insideCtorCtx = ctorCtx->childContexts().first();
    QCOMPARE(insideCtorCtx->childContexts().count(), 0);
    QCOMPARE(insideCtorCtx->localDefinitions().count(), 0);
    QVERIFY(insideCtorCtx->localScopeIdentifier().isEmpty());*/

    release(top);
  }

  void testDeclareNamespace()
  {
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("namespace foo { int bar; } int bar; int test() { return foo::bar; }");

    DUContext* top = parse(method, DumpNone);

    QVERIFY(!top->parentContext());
    QCOMPARE(top->childContexts().count(), 3);
    QCOMPARE(top->localDefinitions().count(), 1);
    QVERIFY(top->localScopeIdentifier().isEmpty());
    QCOMPARE(top->findDefinition(Identifier("foo")), noDef);

    QCOMPARE(top->usingNamespaces().count(), 0);

    DUContext* fooCtx = top->childContexts().first();
    QCOMPARE(fooCtx->childContexts().count(), 0);
    QCOMPARE(fooCtx->localDefinitions().count(), 1);
    QCOMPARE(fooCtx->localScopeIdentifier(), QualifiedIdentifier("foo"));
    QCOMPARE(fooCtx->scopeIdentifier(), QualifiedIdentifier("foo"));

    DUContext* testCtx = top->childContexts()[2];
    QCOMPARE(testCtx->childContexts().count(), 0);
    QCOMPARE(testCtx->localDefinitions().count(), 0);
    QCOMPARE(testCtx->localScopeIdentifier(), QualifiedIdentifier());
    QCOMPARE(testCtx->scopeIdentifier(), QualifiedIdentifier());

    Definition* bar2 = top->localDefinitions().first();
    QCOMPARE(bar2->identifier(), Identifier("bar"));
    QCOMPARE(bar2->qualifiedIdentifier(), QualifiedIdentifier("bar"));
    QCOMPARE(bar2->uses().count(), 0);

    Definition* bar = fooCtx->localDefinitions().first();
    QCOMPARE(bar->identifier(), Identifier("bar"));
    QCOMPARE(bar->qualifiedIdentifier(), QualifiedIdentifier("foo::bar"));
    QCOMPARE(testCtx->findDefinition(QualifiedIdentifier("foo::bar")), bar);
    QCOMPARE(bar->uses().count(), 1);
    QCOMPARE(top->findDefinition(bar->identifier()), bar2);
    QCOMPARE(top->findDefinition(bar->qualifiedIdentifier()), bar);

    QCOMPARE(top->findDefinition(QualifiedIdentifier("bar")), bar2);
    QCOMPARE(top->findDefinition(QualifiedIdentifier("::bar")), bar2);
    QCOMPARE(top->findDefinition(QualifiedIdentifier("foo::bar")), bar);
    QCOMPARE(top->findDefinition(QualifiedIdentifier("::foo::bar")), bar);

    release(top);
  }

  void testDeclareUsingNamespace()
  {
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("namespace foo { int bar; } using namespace foo; int test() { return bar; }");

    DUContext* top = parse(method, DumpNone);

    QVERIFY(!top->parentContext());
    QCOMPARE(top->childContexts().count(), 3);
    QCOMPARE(top->localDefinitions().count(), 0);
    QVERIFY(top->localScopeIdentifier().isEmpty());
    QCOMPARE(top->findDefinition(Identifier("foo")), noDef);

    QCOMPARE(top->usingNamespaces().count(), 1);
    QCOMPARE(top->usingNamespaces().first()->nsIdentifier, QualifiedIdentifier("foo"));
    QCOMPARE(top->usingNamespaces().first()->textCursor(), Cursor(0, 47));

    DUContext* fooCtx = top->childContexts().first();
    QCOMPARE(fooCtx->childContexts().count(), 0);
    QCOMPARE(fooCtx->localDefinitions().count(), 1);
    QCOMPARE(fooCtx->localScopeIdentifier(), QualifiedIdentifier("foo"));
    QCOMPARE(fooCtx->scopeIdentifier(), QualifiedIdentifier("foo"));

    Definition* bar = fooCtx->localDefinitions().first();
    QCOMPARE(bar->identifier(), Identifier("bar"));
    QCOMPARE(bar->qualifiedIdentifier(), QualifiedIdentifier("foo::bar"));
    QCOMPARE(bar->uses().count(), 1);
    QCOMPARE(top->findDefinition(bar->identifier(), KDevDocumentCursor(top->textRangePtr(), KDevDocumentCursor::Start)), noDef);
    QCOMPARE(top->findDefinition(bar->identifier()), bar);
    QCOMPARE(top->findDefinition(bar->qualifiedIdentifier()), bar);

    QCOMPARE(top->findDefinition(QualifiedIdentifier("bar")), bar);
    QCOMPARE(top->findDefinition(QualifiedIdentifier("::bar")), noDef);
    QCOMPARE(top->findDefinition(QualifiedIdentifier("foo::bar")), bar);
    QCOMPARE(top->findDefinition(QualifiedIdentifier("::foo::bar")), bar);

    DUContext* testCtx = top->childContexts()[2];
    QVERIFY(testCtx->parentContext());
    QCOMPARE(testCtx->importedParentContexts().count(), 1);
    QCOMPARE(testCtx->childContexts().count(), 0);
    QCOMPARE(testCtx->localDefinitions().count(), 0);
    QCOMPARE(testCtx->localScopeIdentifier(), QualifiedIdentifier());
    QCOMPARE(testCtx->scopeIdentifier(), QualifiedIdentifier());

    release(top);
  }

  void testFileParse()
  {
    //QSKIP("Unwanted", SkipSingle);

    QFile file("/opt/kde4/src/kdelibs/kate/part/katecodecompletion.h");
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
    delete top;
  }
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TestDUChain::DumpTypes)

DUContext* TestDUChain::parse(const QByteArray& unit, DumpTypes dump)
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

  DUBuilder dubuilder(session);
  static int testNumber = 0;
  KUrl url(QString("file:///internal/%1").arg(testNumber++));
  TopDUContext* top = dubuilder.build(url, ast, DUBuilder::CompileDefinitions);
  dubuilder.build(url, ast, DUBuilder::CompileUses);

  if (dump & DumpDUChain) {
    kDebug() << "===== DUChain:" << endl;
    dumper.dump(top);
  }

  if (dump)
    kDebug() << "===== Finished test case." << endl;

  delete session;

  return top;
}

#include "test_duchain.moc"

QTEST_MAIN(TestDUChain)
