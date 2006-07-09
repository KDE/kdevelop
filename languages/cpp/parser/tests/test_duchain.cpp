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

#include "duchain.h"
#include "ducontext.h"
#include "dubuilder.h"
#include "typesystem.h"
#include "definition.h"

class TestDUChain : public QObject
{
  Q_OBJECT

  TypeEnvironment types;
  const AbstractType* type1;
  const AbstractType* type2;
  const AbstractType* type3;
  Definition* definition1;
  Definition* definition2;
  Definition* definition3;
  Definition* definition4;
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

    definition1 = new Definition(type1, QString("lazy"), Definition::LocalScope);
    definition2 = new Definition(type2, QString("m_errorCode"), Definition::ClassScope);
    definition3 = new Definition(type3, QString("lazy"), Definition::GlobalScope);
    definition4 = new Definition(type2, QString("m_errorCode2"), Definition::ClassScope);

    file1 = "file:///opt/kde4/src/kdevelop/languages/cpp/parser/dutopContext->cpp";
    file2 = "file:///opt/kde4/src/kdevelop/languages/cpp/parser/dubuilder.cpp";

    topContext = new DUContext;
    topContext->setTextRange(KTextEditor::Range(0,0,25,0), file1);
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

  /*void testParentContexts()
  {
    DUContext topContext;
    (&chain);

    DUContext context(&chain);
    QCOMPARE(context.parentContexts().count(), 1);
    QCOMPARE(context.parentContexts().first(), &chain);

    DUContext otherParent;
    context.addParentContext(&otherParent);
    QCOMPARE(context.parentContexts().count(), 2);
    QCOMPARE(context.parentContexts().last(), &otherParent);

    DUContext thirdParent;
    context.insertParentContext(&otherParent, &thirdParent);
    QCOMPARE(context.parentContexts().count(), 3);
    QCOMPARE(context.parentContexts()[1], &thirdParent);

    context.removeParentContext(&chain);
    QCOMPARE(context.parentContexts().count(), 2);
    QCOMPARE(context.parentContexts()[0], &thirdParent);

    context.clearParentContexts();
    QVERIFY(context.parentContexts().isEmpty());
  }*/

  void testContextRelationships()
  {
    QCOMPARE(DUChain::self()->chainForDocument(file1), topContext);

    DUContext* firstChild = new DUContext;
    firstChild->setTextRange(KTextEditor::Range(4,4, 10,3), file1);
    topContext->addChildContext(firstChild);

    QCOMPARE(firstChild->parentContexts().count(), 1);
    QCOMPARE(firstChild->parentContexts().first(), topContext);
    QCOMPARE(firstChild->childContexts().count(), 0);
    QCOMPARE(topContext->childContexts().count(), 1);
    QCOMPARE(topContext->childContexts().last(), firstChild);

    DUContext* secondChild = new DUContext;
    secondChild->setTextRange(KTextEditor::Range(14,4, 19,3), file1);
    topContext->addChildContext(firstChild);

    QCOMPARE(topContext->childContexts().count(), 2);
    QCOMPARE(topContext->childContexts()[1], secondChild);

    DUContext* thirdChild = new DUContext;
    thirdChild->setTextRange(KTextEditor::Range(10,4, 14,3), file1);
    topContext->addChildContext(secondChild);

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

    kDebug() << k_funcinfo << "Warning expected here (bug if not present)." << endl;
    topContext->addDefinition(definition2);
    QCOMPARE(topContext->localDefinitions().count(), 2);

    topContext->clearLocalDefinitions();
    QVERIFY(topContext->localDefinitions().isEmpty());
  }

  void testDefinitionChain()
  {
    topContext->addDefinition(definition1);
    topContext->addDefinition(definition2);

    DUContext* context1 = new DUContext;
    context1->setTextRange(KTextEditor::Range(4,4, 14,3), file1);
    topContext->addChildContext(context1);
    TextPosition insideContext1(KTextEditor::Cursor(5,9), file1);

    QCOMPARE(topContext->findContext(insideContext1), context1);
    QCOMPARE(topContext->findDefinition(definition1->identifier(), insideContext1), definition1);

    DUContext* subContext1 = new DUContext;
    subContext1->setTextRange(KTextEditor::Range(5,4, 7,3), file1);
    topContext->addChildContext(subContext1);

    DUContext* subContext2 = new DUContext;
    subContext2->setTextRange(KTextEditor::Range(9,4, 12,3), file1);
    topContext->addChildContext(subContext2);

    subContext1->addDefinition(definition3);

    subContext2->addDefinition(definition4);

    topContext->deleteChildContextsRecursively();
  }

  /*void testControlContexts()
  {
    Control control;
    const NameSymbol *n1 = control.findOrInsertName("a", 1);
    int *type1 = new int(1); // don't care much about types
    control.declare(n1, (Type*)type1);

    control.pushContext();
    int *type2 = new int(2);
    const NameSymbol *n2 = control.findOrInsertName("b", 1);
    control.declare(n2, (Type*)type2);

    QCOMPARE(control.lookupType(n1), (Type*)type1);
    QCOMPARE(control.lookupType(n2), (Type*)type2);

    control.popContext();
    QCOMPARE(control.lookupType(n1), (Type*)type1);
    QCOMPARE(control.lookupType(n2), (Type*)0);
  }

  void testProblems()
  {
    Problem p;
    p.setColumn(1);
    p.setLine(1);
    p.setFileName("foo.cpp");
    p.setMessage("The Problem");
    Control c;
    c.reportProblem(p);
    QCOMPARE(p.message(), c.problem(0).message());
  }

  void testTokenTable()
  {
    QCOMPARE(token_name(Token_EOF), "eof");
    QCOMPARE(token_name('a'), "a");
    QCOMPARE(token_name(Token_delete), "delete");
  }

  void testLexer()
  {
    QByteArray code("#include <foo.h>");
    TokenStream token_stream;
    LocationTable location_table;
    LocationTable line_table;
    Control control;

    Lexer lexer(token_stream, location_table, line_table, &control);
    lexer.tokenize(code, code.size()+1);
    QCOMPARE(control.problem(0).message(), QString("expected end of line"));

    QByteArray code2("class Foo { int foo() {} }; ");
    lexer.tokenize(code2, code2.size()+1);
    QCOMPARE(control.problemCount(), 1);    //we still have the old problem in the list
  }

  void testParser()
  {
    QByteArray clazz("struct A { int i; A() : i(5) { } virtual void test() = 0; };");
    pool mem_pool;
    TranslationUnitAST* ast = parse(clazz, &mem_pool);
    QVERIFY(ast != 0);
    QVERIFY(ast->declarations != 0);
  }

  void testParserFail()
  {
    QByteArray stuff("foo bar !!! nothing that really looks like valid c++ code");
    pool mem_pool;
    TranslationUnitAST *ast = parse(stuff, &mem_pool);
    QVERIFY(ast->declarations == 0);
    QCOMPARE(control.problemCount(), 5);
  }

  void testParseMethod()
  {
    QByteArray method("void A::test() {  }");
    QVERIFY(!control.skipFunctionBody());
    pool mem_pool;
    TranslationUnitAST* ast = parse(method, &mem_pool);
    QVERIFY(ast != 0);
    QVERIFY(hasKind(ast, AST::Kind_FunctionDefinition));
  }

  void testMethodArgs()
  {
    QByteArray method("int A::test(int primitive, B* pointer) { return primitive; }");
    pool mem_pool;
    Parser parser(&control);
    TranslationUnitAST* ast = parser.parse(method.constData(),
					   method.size() + 1, &mem_pool);
    // return type
    SimpleTypeSpecifierAST* retType = static_cast<SimpleTypeSpecifierAST*>
      (getAST(ast, AST::Kind_SimpleTypeSpecifier));
    QCOMPARE((TOKEN_KIND)parser.token_stream.kind(retType->start_token),
	    Token_int);

    // first param
    ParameterDeclarationAST* param = static_cast<ParameterDeclarationAST*>
      (getAST(ast, AST::Kind_ParameterDeclaration));
    SimpleTypeSpecifierAST* paramType = static_cast<SimpleTypeSpecifierAST*>
      (getAST(param, AST::Kind_SimpleTypeSpecifier));
    QCOMPARE((TOKEN_KIND)parser.token_stream.kind(paramType->start_token),
	    Token_int);
    UnqualifiedNameAST* argName  = static_cast<UnqualifiedNameAST*>
      (getAST(param, AST::Kind_UnqualifiedName));
    QCOMPARE(parser.token_stream.symbol(argName->id)->as_string(),
	    QString("primitive"));

    // second param
    param = static_cast<ParameterDeclarationAST*>
      (getAST(ast, AST::Kind_ParameterDeclaration, 1));
    UnqualifiedNameAST* argType = static_cast<UnqualifiedNameAST*>
      (getAST(param, AST::Kind_UnqualifiedName));
    QCOMPARE(parser.token_stream.symbol(argType->id)->as_string(),
	    QString("B"));

    // pointer operator
    QVERIFY(hasKind(param, AST::Kind_PtrOperator));

    argName = static_cast<UnqualifiedNameAST*>
      (getAST(param, AST::Kind_UnqualifiedName, 1));
    QCOMPARE(parser.token_stream.symbol(argName->id)->as_string(),
	    QString("pointer"));

  }

  void testForStatements()
  {
    QByteArray method("void A::t() { for (int i = 0; i < 10; i++) { ; }}");
    pool mem_pool;
    TranslationUnitAST* ast = parse(method, &mem_pool);

    QVERIFY(ast != 0);
    QVERIFY(hasKind(ast, AST::Kind_ForStatement));
    QVERIFY(hasKind(ast, AST::Kind_Condition));
    QVERIFY(hasKind(ast, AST::Kind_IncrDecrExpression));
    QVERIFY(hasKind(ast, AST::Kind_SimpleDeclaration));

    QByteArray emptyFor("void A::t() { for (;;) { } }");
    ast = parse(emptyFor, &mem_pool);
    QVERIFY(ast != 0);
    QVERIFY(hasKind(ast, AST::Kind_ForStatement));
    QVERIFY(!hasKind(ast, AST::Kind_Condition));
    QVERIFY(!hasKind(ast, AST::Kind_SimpleDeclaration));
  }

  void testIfStatements()
  {
    QByteArray method("void A::t() { if (1 < 2) { } }");
    pool mem_pool;
    TranslationUnitAST* ast = parse(method, &mem_pool);
    QVERIFY(hasKind(ast, AST::Kind_Condition));
    QVERIFY(hasKind(ast, AST::Kind_BinaryExpression));
  }

  void testParseFile()
  {
     QFile file(TEST_FILE);
     QVERIFY(file.open(QFile::ReadOnly));
     QByteArray contents = file.readAll();
     file.close();
     pool mem_pool;
     Parser parser(&control);
     TranslationUnitAST* ast = parser.parse(contents.constData(),
					    contents.size(), &mem_pool);
     QVERIFY(ast != 0);
     QVERIFY(ast->declarations != 0);
   }

private:

  TranslationUnitAST* parse(const QByteArray& unit, pool* mem_pool)
  {
    Parser parser(&control);
    return  parser.parse(unit.constData(), unit.size() + 1, mem_pool);
  }


*/
};


#include "test_duchain.moc"

QTEST_MAIN(TestDUChain)
