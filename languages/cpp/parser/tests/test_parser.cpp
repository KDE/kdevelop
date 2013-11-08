/* This file is part of KDevelop

   Copyright 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>
   Copyright 2011 Milian Wolff <mail@milianw.de>

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

#include "test_parser.h"

#include <QtTest/QtTest>

#define private public
#include "ast.h"
#undef private

#include "parser.h"
#include "rpp/preprocessor.h"
#include "tokens.h"
#include "parsesession.h"
#include "commentformatter.h"

#include "testconfig.h"

#include <QByteArray>
#include <QDataStream>
#include <QFile>

#include <iostream>
#include <rpp/chartools.h>
#include <rpp/pp-engine.h>

#include <tests/autotestshell.h>
#include <tests/testcore.h>

QString preprocess(const QString& contents) {
  rpp::Preprocessor preprocessor;
  rpp::pp pp(&preprocessor);
  QByteArray qba = stringFromContents(pp.processFile("anonymous", contents.toUtf8()));
  if(pp.problems().empty())
    return QString::fromUtf8(qba);
  else
    return "*ERROR*";
}

void TestParser::initTestCase()
{
  KDevelop::AutoTestShell::init(QStringList() << "kdevcppsupport");
  KDevelop::TestCore* core = new KDevelop::TestCore();
  core->initialize(KDevelop::Core::NoUi);
}

void TestParser::cleanupTestCase()
{
  KDevelop::TestCore::shutdown();
}

void TestParser::testSymbolTable()
{
  NameTable table;
  table.findOrInsert("Ideal::MainWindow", sizeof("Ideal::MainWindow"));
  table.findOrInsert("QMainWindow", sizeof("QMainWindow"));
  table.findOrInsert("KXmlGuiWindow", sizeof("KXmlGuiWindow"));
  QCOMPARE(table.count(), size_t(3));
  const NameSymbol *s = table.findOrInsert("QMainWindow", sizeof("QMainWindow"));
  QCOMPARE(QString(s->data), QString("QMainWindow"));
  QCOMPARE(table.count(), size_t(3));
}

  ///@todo reenable
//   void TestParser::testControlContexts()
//   {
//     Control control;
//     const NameSymbol *n1 = control.findOrInsertName("a", 1);
//     int *type1 = new int(1); // don't care much about types
//     control.declare(n1, (Type*)type1);
//
//     control.pushContext();
//     int *type2 = new int(2);
//     const NameSymbol *n2 = control.findOrInsertName("b", 1);
//     control.declare(n2, (Type*)type2);
//
//     QCOMPARE(control.lookupType(n1), (Type*)type1);
//     QCOMPARE(control.lookupType(n2), (Type*)type2);
//
//     control.popContext();
//     QCOMPARE(control.lookupType(n1), (Type*)type1);
//     QCOMPARE(control.lookupType(n2), (Type*)0);
//   }

void TestParser::testTokenTable()
{
  QCOMPARE(token_name(Token_EOF), "eof");
  QCOMPARE(token_name('a'), "a");
  QCOMPARE(token_name(Token_delete), "delete");
}

///@todo reenable
//   void TestParser::testLexer()
//   {
//     QByteArray code("#include <foo.h>");
//     TokenStream token_stream;
//     LocationTable location_table;
//     LocationTable line_table;
//     Control control;
//
//     Lexer lexer(token_stream, location_table, line_table, &control);
//     lexer.tokenize(code, code.size()+1);
//     QCOMPARE(control.problem(0).message(), QString("expected end of line"));
//
//     QByteArray code2("class Foo { int foo() {} }; ");
//     lexer.tokenize(code2, code2.size()+1);
//     QCOMPARE(control.problemCount(), 1);    //we still have the old problem in the list
//   }

void TestParser::testParser()
{
  QByteArray clazz("struct A { int i; A() : i(5) { } virtual void test() = 0; };");
  TranslationUnitAST* ast = parse(clazz);
  dump(ast);
  QVERIFY(control.problems().isEmpty());
  QVERIFY(ast != 0);
  QVERIFY(ast->declarations != 0);
}

void TestParser::testTemplateArguments()
{
  QByteArray templatetest("template <int N, int M> struct SeriesAdder{ enum { value = N + SeriesAdder< 0 >::value }; };");
  TranslationUnitAST* ast = parse(templatetest);
    QVERIFY(control.problems().isEmpty());
  QVERIFY(ast != 0);
  QVERIFY(ast->declarations != 0);
  QVERIFY(control.problems().isEmpty());
}

void TestParser::testTemplatedDTor()
{
  // see also: https://bugs.kde.org/show_bug.cgi?id=253618
  QByteArray templatetest("template <typename T> struct A{ ~A<T>(); };");
  TranslationUnitAST* ast = parse(templatetest);
  QVERIFY(ast != 0);
  QVERIFY(ast->declarations != 0);
  QVERIFY(control.problems().isEmpty());
}

void TestParser::testManyComparisons()
{
  //Should not crash
  {
    QByteArray clazz("void TestParser::test() { if(val < f && val < val1 && val < val2 && val < val3 ){ } }");
    TranslationUnitAST* ast = parse(clazz);
    dump(ast);
    QVERIFY(control.problems().isEmpty());
    QVERIFY(ast != 0);
    QVERIFY(ast->declarations != 0);
  }
  {
    QByteArray clazz("void TestParser::test() { if(val < f && val < val1 && val < val2 && val < val3 && val < val4 && val < val5 && val < val6 && val < val7 && val < val8 && val < val9 && val < val10 && val < val11 && val < val12 && val < val13 && val < val14 && val < val15 && val < val16 && val < val17 && val < val18 && val < val19 && val < val20 && val < val21 && val < val22 && val < val23 && val < val24 && val < val25 && val < val26){ } }");
    TranslationUnitAST* ast = parse(clazz);
    dump(ast);
    QVERIFY(control.problems().isEmpty());
    QVERIFY(ast != 0);
    QVERIFY(ast->declarations != 0);
  }
}

void TestParser::testParserFail()
{
  QByteArray stuff("foo bar !!! nothing that really looks like valid c++ code");
  TranslationUnitAST *ast = parse(stuff);
  QVERIFY(ast->declarations == 0);
  QVERIFY(control.problems().count() > 3);
}

void TestParser::testPartialParseFail() {
  {
  QByteArray method("struct C { Something invalid is here };");
  TranslationUnitAST* ast = parse(method);
  QVERIFY(ast != 0);
  QVERIFY(hasKind(ast, AST::Kind_ClassSpecifier));
  }
  {
  QByteArray method("void TestParser::test() { Something invalid is here };");
  TranslationUnitAST* ast = parse(method);
  QVERIFY(ast != 0);
  QVERIFY(hasKind(ast, AST::Kind_FunctionDefinition));
  }
  {
  QByteArray method("void TestParser::test() { {Something invalid is here };");
  TranslationUnitAST* ast = parse(method);
  QVERIFY(ast != 0);
  QVERIFY(hasKind(ast, AST::Kind_FunctionDefinition));
  QVERIFY(ast->hadMissingCompoundTokens);
  }
  {
  QByteArray method("void TestParser::test() { case:{};");
  TranslationUnitAST* ast = parse(method);
  QVERIFY(ast != 0);
  QVERIFY(hasKind(ast, AST::Kind_FunctionDefinition));
  QVERIFY(ast->hadMissingCompoundTokens);
  }
}

void TestParser::testParseMethod()
{
  QByteArray method("void TestParser::A::test() {  }");
  TranslationUnitAST* ast = parse(method);
  dump(ast);
  QVERIFY(control.problems().isEmpty());
  QVERIFY(ast != 0);
  QVERIFY(hasKind(ast, AST::Kind_FunctionDefinition));
}

///@todo reenable
//   void TestParser::testMethodArgs()
//   {
//     QByteArray method("int A::test(int primitive, B* pointer) { return primitive; }");
//     Parser parser(&control);
//     TranslationUnitAST* ast = parser.parse(method.constData(),
// 					   method.size() + 1);
//     // return type
//     SimpleTypeSpecifierAST* retType = static_cast<SimpleTypeSpecifierAST*>
//       (getAST(ast, AST::Kind_SimpleTypeSpecifier));
//     QCOMPARE((TOKEN_KIND)parser.token_stream.kind(retType->start_token),
// 	    Token_int);
//
//     // first param
//     ParameterDeclarationAST* param = static_cast<ParameterDeclarationAST*>
//       (getAST(ast, AST::Kind_ParameterDeclaration));
//     SimpleTypeSpecifierAST* paramType = static_cast<SimpleTypeSpecifierAST*>
//       (getAST(param, AST::Kind_SimpleTypeSpecifier));
//     QCOMPARE((TOKEN_KIND)parser.token_stream.kind(paramType->start_token),
// 	    Token_int);
//     UnqualifiedNameAST* argName  = static_cast<UnqualifiedNameAST*>
//       (getAST(param, AST::Kind_UnqualifiedName));
//     QCOMPARE(parser.token_stream.symbol(argName->id)->as_string(),
// 	    QString("primitive"));
//
//     // second param
//     param = static_cast<ParameterDeclarationAST*>
//       (getAST(ast, AST::Kind_ParameterDeclaration, 1));
//     UnqualifiedNameAST* argType = static_cast<UnqualifiedNameAST*>
//       (getAST(param, AST::Kind_UnqualifiedName));
//     QCOMPARE(parser.token_stream.symbol(argType->id)->as_string(),
// 	    QString("B"));
//
//     // pointer operator
//     QVERIFY(hasKind(param, AST::Kind_PtrOperator));
//
//     argName = static_cast<UnqualifiedNameAST*>
//       (getAST(param, AST::Kind_UnqualifiedName, 1));
//     QCOMPARE(parser.token_stream.symbol(argName->id)->as_string(),
// 	    QString("pointer"));
//
//   }

void TestParser::testForStatements()
{
  QByteArray method("void TestParser::A::t() { for (int i = 0; i < 10; i++) { ; }}");
  TranslationUnitAST* ast = parse(method);

  QVERIFY(control.problems().isEmpty());
  
  QVERIFY(ast != 0);
  QVERIFY(hasKind(ast, AST::Kind_ForStatement));
  QVERIFY(hasKind(ast, AST::Kind_Condition));
  QVERIFY(hasKind(ast, AST::Kind_IncrDecrExpression));
  QVERIFY(hasKind(ast, AST::Kind_SimpleDeclaration));

  QByteArray emptyFor("void TestParser::A::t() { for (;;) { } }");
  ast = parse(emptyFor);
  QVERIFY(ast != 0);
  QVERIFY(hasKind(ast, AST::Kind_ForStatement));
  QVERIFY(!hasKind(ast, AST::Kind_Condition));
  QVERIFY(!hasKind(ast, AST::Kind_SimpleDeclaration));
}

void TestParser::testIfStatements()
{
  QByteArray method("void TestParser::A::t() { if (1 < 2) { } }");
  TranslationUnitAST* ast = parse(method);

  QVERIFY(control.problems().isEmpty());
  QVERIFY(hasKind(ast, AST::Kind_Condition));
  QVERIFY(hasKind(ast, AST::Kind_BinaryExpression));
}

void TestParser::testComments()
{
  QByteArray method("//TranslationUnitComment\n"
                    "//Hello\n"
                    "int A; //behind\n"
                    " /*between*/\n"
                    " /*Hello2*/\n"
                    " class B{}; //behind\n"
                    "//Hello3\n"
                    " //beforeTest\n"
                    "void TestParser::test(); //testBehind");
  TranslationUnitAST* ast = parse(method);
  QVERIFY(control.problems().isEmpty());

  CommentFormatter formatter;
  
  QCOMPARE(formatter.formatComment(ast->comments, lastSession), QByteArray("TranslationUnitComment")); //The comments were merged

  const ListNode<DeclarationAST*>* it = ast->declarations;
  QVERIFY(it);
  QCOMPARE(it->count(), 3);
  it = it->next;
  QVERIFY(it);
  QVERIFY(it->element);
  QCOMPARE(formatter.formatComment(it->element->comments, lastSession), QByteArray("Hello\n(behind)"));

  it = it->next;
  QVERIFY(it);
  QVERIFY(it->element);
  QCOMPARE(formatter.formatComment(it->element->comments, lastSession), QByteArray("between\nHello2\n(behind)"));

  it = it->next;
  QVERIFY(it);
  QVERIFY(it->element);
  QCOMPARE(formatter.formatComment(it->element->comments, lastSession), QByteArray("Hello3\nbeforeTest\n(testBehind)"));
}

void TestParser::testComments2()
{
  CommentFormatter formatter;
  QByteArray method("enum Enum\n {//enumerator1Comment\nenumerator1, //enumerator1BehindComment\n /*enumerator2Comment*/ enumerator2 /*enumerator2BehindComment*/};");
  TranslationUnitAST* ast = parse(method);
  QVERIFY(control.problems().isEmpty());

  const ListNode<DeclarationAST*>* it = ast->declarations;
  QVERIFY(it);
  it = it->next;
  QVERIFY(it);
  const SimpleDeclarationAST* simpleDecl = static_cast<const SimpleDeclarationAST*>(it->element);
  QVERIFY(simpleDecl);

  const EnumSpecifierAST* enumSpec = (const EnumSpecifierAST*)simpleDecl->type_specifier;
  QVERIFY(enumSpec);

  const ListNode<EnumeratorAST*> *enumerator = enumSpec->enumerators;
  QVERIFY(enumerator);
  enumerator = enumerator->next;
  QVERIFY(enumerator);

  QCOMPARE(formatter.formatComment(enumerator->element->comments, lastSession), QByteArray("enumerator1Comment\n(enumerator1BehindComment)"));

  enumerator = enumerator->next;
  QVERIFY(enumerator);

  QCOMPARE(formatter.formatComment(enumerator->element->comments, lastSession), QByteArray("enumerator2Comment\n(enumerator2BehindComment)"));
}

void TestParser::testComments3()
{
  CommentFormatter formatter;
  QByteArray method("class Class{\n//Comment\n int val;};");
  TranslationUnitAST* ast = parse(method);
  QVERIFY(control.problems().isEmpty());

  const ListNode<DeclarationAST*>* it = ast->declarations;
  QVERIFY(it);
  it = it->next;
  QVERIFY(it);
  const SimpleDeclarationAST* simpleDecl = static_cast<const SimpleDeclarationAST*>(it->element);
  QVERIFY(simpleDecl);

  const ClassSpecifierAST* classSpec = (const ClassSpecifierAST*)simpleDecl->type_specifier;
  QVERIFY(classSpec);

  const ListNode<DeclarationAST*> *members = classSpec->member_specs;
  QVERIFY(members);
  members = members->next;
  QVERIFY(members);

  QCOMPARE(formatter.formatComment(members->element->comments, lastSession), QByteArray("Comment"));
}

void TestParser::testComments4()
{
  CommentFormatter formatter;
  QByteArray method("//TranslationUnitComment\n//Comment\ntemplate<class C> class Class{};");
  TranslationUnitAST* ast = parse(method);

  const ListNode<DeclarationAST*>* it = ast->declarations;
  QVERIFY(it);
  it = it->next;
  QVERIFY(it);
  const TemplateDeclarationAST* templDecl = static_cast<const TemplateDeclarationAST*>(it->element);
  QVERIFY(templDecl);
  QVERIFY(templDecl->declaration);

  //QCOMPARE(formatter.formatComment(templDecl->declaration->comments, lastSession), QString("Comment"));
}

void TestParser::testComments5()
{
  CommentFormatter formatter;
  QByteArray method("//TranslationUnitComment\n  //FIXME comment\n //this is TODO\n /* TODO: comment */\n  int i;  // another TODO \n // Just a simple comment\nint j;\n int main(void) {\n // TODO COMMENT\n}\n// Non-ascii TODO 例えば\n");
  int initial_size = control.problems().size();  // Remember existing number of problems
  TranslationUnitAST* ast = parse(method);

  const ListNode<DeclarationAST*>* it = ast->declarations;
  QVERIFY(it);
  it = it->next;
  QVERIFY(it);
  QCOMPARE(formatter.formatComment(it->element->comments, lastSession), QByteArray("FIXME comment\nthis is TODO\n TODO: comment\n(another TODO)"));
  it = it->next;
  QVERIFY(it);
  QCOMPARE(formatter.formatComment(it->element->comments, lastSession), QByteArray("Just a simple comment"));

  QList<KDevelop::ProblemPointer> problem_list = control.problems();
  QCOMPARE(problem_list.size(), initial_size + 6); // 6 to-dos
  KDevelop::ProblemPointer problem = problem_list[initial_size];
  QCOMPARE(problem->description(), QString("FIXME comment"));
  QCOMPARE(problem->source(), KDevelop::ProblemData::ToDo);
  QCOMPARE(problem->finalLocation().start, KDevelop::SimpleCursor(1, 4));
  QCOMPARE(problem->finalLocation().end, KDevelop::SimpleCursor(1, 17));

  problem = problem_list[initial_size + 1];
  QCOMPARE(problem->description(), QString("this is TODO"));
  QCOMPARE(problem->source(), KDevelop::ProblemData::ToDo);
  QCOMPARE(problem->finalLocation().start, KDevelop::SimpleCursor(2, 3));
  QCOMPARE(problem->finalLocation().end, KDevelop::SimpleCursor(2, 15));

  problem = problem_list[initial_size + 2];
  QCOMPARE(problem->description(), QString("TODO: comment"));
  QCOMPARE(problem->source(), KDevelop::ProblemData::ToDo);
  QCOMPARE(problem->finalLocation().start, KDevelop::SimpleCursor(3, 4));
  QCOMPARE(problem->finalLocation().end, KDevelop::SimpleCursor(3, 17));

  problem = problem_list[initial_size + 3];
  QCOMPARE(problem->description(), QString("another TODO"));
  QCOMPARE(problem->source(), KDevelop::ProblemData::ToDo);
  QCOMPARE(problem->finalLocation().start, KDevelop::SimpleCursor(4, 13));
  QCOMPARE(problem->finalLocation().end, KDevelop::SimpleCursor(4, 25));

  problem = problem_list[initial_size + 4];
  QCOMPARE(problem->description(), QString("TODO COMMENT"));
  QCOMPARE(problem->source(), KDevelop::ProblemData::ToDo);
  QCOMPARE(problem->finalLocation().start, KDevelop::SimpleCursor(8, 4));
  QCOMPARE(problem->finalLocation().end, KDevelop::SimpleCursor(8, 16));

  problem = problem_list[initial_size + 5];
  QCOMPARE(problem->description(), QString::fromUtf8("Non-ascii TODO 例えば"));
  QCOMPARE(problem->source(), KDevelop::ProblemData::ToDo);
  QCOMPARE(problem->finalLocation().start, KDevelop::SimpleCursor(10, 3));
  QCOMPARE(problem->finalLocation().end, KDevelop::SimpleCursor(10, 27));
}

void TestParser::testComments6() {
  QByteArray module("//TranslationUnitComment\n/**\n * foo\n **/\nint i;\n");
  TranslationUnitAST* ast = parse(module);
  const ListNode<DeclarationAST*>* it = ast->declarations;
  QVERIFY(it);
  it = it->next;
  QVERIFY(it);
  QCOMPARE(CommentFormatter().formatComment(it->element->comments, lastSession), QByteArray("foo"));
}

void TestParser::testComments7()
{
  QByteArray module("//TranslationUnitComment\n\n//Foo\\\nbar\nint i;\n");
  TranslationUnitAST* ast = parse(module);
  const ListNode<DeclarationAST*>* it = ast->declarations;
  QVERIFY(control.problems().isEmpty());
  QVERIFY(it);
  it = it->next;
  QVERIFY(it);
  QCOMPARE(QString::fromUtf8(CommentFormatter().formatComment(it->element->comments, lastSession)), QString("Foo bar"));
}

void TestParser::testEscapedNewline_data()
{
  QTest::addColumn<QByteArray>("module");
  QTest::newRow("simple") << QByteArray("\\\nint i;\n");
  QTest::newRow("after-decl") << QByteArray("int a;\\\nint b;\n");
  QTest::newRow("after-decl-white") << QByteArray("int a; \\\nint b;\n");
  QTest::newRow("member-initializer") << QByteArray("struct s {\n  int a, b;\n  s():\\\n    a(0),\\\n    b(0)\n  {}\n};\n");
}

void TestParser::testEscapedNewline()
{
  QFETCH(QByteArray, module);
  TranslationUnitAST* ast = parse(module);
  QVERIFY(control.problems().isEmpty());
}

void TestParser::testPreprocessor() {
  QCOMPARE(preprocess("#define TEST (1L<<10)\nTEST").trimmed(), QString("(1L<<10)"));
  QCOMPARE(preprocess("#define SELF OTHER\n#define OTHER SELF\nSELF").trimmed(), QString("SELF"));
  QCOMPARE(preprocess("#define TEST //Comment\nTEST 1").trimmed(), QString("1")); //Comments are not included in macros
  QCOMPARE(preprocess("#define TEST /*Comment\n*/\nTEST 1").trimmed(), QString("1")); //Comments are not included in macros
  QCOMPARE(preprocess("#define TEST_URL \"http://foobar.com\"\nTEST_URL").trimmed(), QString("\"http://foobar.com\""));
  QCOMPARE(preprocess("#define TEST_STR \"//\\\"//\"\nTEST_STR").trimmed(), QString("\"//\\\"//\""));
  QCOMPARE(preprocess("#if ~1\n#define NUMBER 10\n#else\n#define NUMBER 20\n#endif\nNUMBER").trimmed(), QString("10"));
  QCOMPARE(preprocess("#define MACRO(a, b) ab\nMACRO\n(aa, bb)").trimmed(), QString("ab"));
  QCOMPARE(preprocess("#define MACRO(a, b) ab\nMACRO(aa,\n bb)").trimmed(), QString("ab"));
  QCOMPARE(preprocess("#if 0x1\n #define NUMBER 10\n#else\n#define NUMBER 20\n#endif\nNUMBER\n").trimmed(), QString("10"));
  QCOMPARE(preprocess("#define AA BB\n#define BB CC\nAA\n").trimmed(), QString("CC"));
  QCOMPARE(preprocess("#define bla(x) bla(x)\n#define foo(x) foo##x\n#define choose(x) x(a) x(b)\nchoose(bla)\n").replace(QRegExp("[\n\t ]+"), ""), QString("bla(a)bla(b)"));
  QCOMPARE(preprocess("#define bla(x) bla(x)\n#define foo(x) foo##x\n#define choose(x) x(a) x(b)\nchoose(foo)\n").replace(QRegExp("[\n\t ]+"), " ").trimmed(), QString("fooa foob"));
  QCOMPARE(preprocess("#define XX YY\n#define YY(x) z x\nXX(x)\n").replace(QRegExp("[\n\t ]+"), " ").trimmed(), QString("z x"));
  QCOMPARE(preprocess("#define PP(x) Q##x\n#define QQ 12\nPP(Q)\n").trimmed(), QString("12"));
  QCOMPARE(preprocess("#define MM(x) NN\n#define OO(NN) MM(NN)\nOO(2)\n").trimmed(), QString("NN"));
  QCOMPARE(preprocess("#define OOO(x) x x x\n#define OOOO(x) O##x(2)\nOOOO(OO)\n").replace(QRegExp("[\n\t ]+"), " ").trimmed(), QString("2 2 2"));
  QCOMPARE(preprocess("#define OOO(x) x x x\n#define OOOO(x) O##x(2)\nOOOO(OOO)\n").replace(QRegExp("[\n\t ]+"), ""), QString("OOOO(2)"));

  QCOMPARE(preprocess("#if 1\n #define N 10\n#else\n#define N 20\n#endif\nN\n").trimmed(), QString("10"));
  QCOMPARE(preprocess("#if 1u\n #define N 10\n#else\n#define N 20\n#endif\nN\n").trimmed(), QString("10"));
  QCOMPARE(preprocess("#if 1l\n #define N 10\n#else\n#define N 20\n#endif\nN\n").trimmed(), QString("10"));
  QCOMPARE(preprocess("#if 1lu\n #define N 10\n#else\n#define N 20\n#endif\nN\n").trimmed(), QString("10"));
  QCOMPARE(preprocess("#if 1ul\n #define N 10\n#else\n#define N 20\n#endif\nN\n").trimmed(), QString("10"));
  QCOMPARE(preprocess("#if 1ll\n #define N 10\n#else\n#define N 20\n#endif\nN\n").trimmed(), QString("10"));
  QCOMPARE(preprocess("#if 1llu\n #define N 10\n#else\n#define N 20\n#endif\nN\n").trimmed(), QString("10"));

  QCOMPARE(preprocess("#if ~0ull == 0u + ~0u\n 10\n #endif\n").trimmed(), QString("10"));

  QCOMPARE(preprocess("#if 1Ul\n 10\n #endif\n").trimmed(), QString("10"));
  QCOMPARE(preprocess("#if 1Lu\n 10\n #endif\n").trimmed(), QString("10"));
  QCOMPARE(preprocess("#if 1LlU\n 10\n #endif\n").trimmed(), QString("10"));

  QCOMPARE(preprocess("#if 1_u\n #endif\n").trimmed(), QString("*ERROR*"));
  QCOMPARE(preprocess("#if 1u_\n #endif\n").trimmed(), QString("*ERROR*"));

  QCOMPARE(preprocess("#if 1uu\n #endif\n").trimmed(), QString("*ERROR*"));
  QCOMPARE(preprocess("#if 1lll\n #endif\n").trimmed(), QString("*ERROR*"));
  QCOMPARE(preprocess("#if 1lul\n #endif\n").trimmed(), QString("*ERROR*"));

  QCOMPARE(preprocess("#if (2147483647L + 10L) > 0\n 10\n #endif\n").trimmed(), QString("10"));

  QCOMPARE(preprocess("#ifdef\n"), QString("*ERROR*"));
  
  QEXPECT_FAIL("", "Backslash incorrectly handled", Continue);
  QCOMPARE(preprocess("bla \\\n#define foobar oc\nfoobar\n").replace(QRegExp("[\n\t ]+"), " ").trimmed(), QString("bla #define foobar oc foobar"));
  
  QEXPECT_FAIL("", "Backslash incorrectly handled", Continue);
  QCOMPARE(preprocess("#define foo(x) foo##x\n#define bla fo\\\no(2)\n").trimmed(), QString("foo2"));
  
  QEXPECT_FAIL("", "Empty expansions incorrectly handled", Continue);
  QCOMPARE(preprocess("#define foo(x) foo##x\n#define _A\n\n#define CALL(X, Y) X _A (Y)\nCALL(foo, 13)\n").replace(QRegExp("[\n\t ]+"), ""), QString("foo(13)"));
  
  QEXPECT_FAIL("", "Variadic macros unsupported", Continue);
  QCOMPARE(preprocess("#define NC(...) __VA_ARGS__\nNC(bla,bla)\n").replace(QRegExp("[\n\t ]+"), ""), QString("bla,bla"));
  
  QEXPECT_FAIL("", "Variadic macros unsupported", Continue);
  QCOMPARE(preprocess("#define PUT_BETWEEN(x,y) x y x\n#define NC(...) __VA_ARGS__\nPUT_BETWEEN(NC(pair<a,b>), c)\n").replace(QRegExp("[\n\t ]+"), " ").trimmed(), QString("pair<a,b> c pair<a,b>"));

  QEXPECT_FAIL("", "Variadic arguments cannot be left empty (GCC extension)", Continue);
  QCOMPARE(preprocess("#define NC(x,y...) x y\nNC(kde,ve)\nNC(lop)").replace(QRegExp("[\n\t ]+"), ""), QString("kdevelop"));
  
  QEXPECT_FAIL("", "No problems reported for missmatching macro-parameter-lists", Continue);
  QCOMPARE(preprocess("#define bla(x,y)\nbla(1,2,3)\n"), QString("*ERROR*"));
  
  QEXPECT_FAIL("", "No problems reported for missmatching macro-parameter-lists", Continue);
  QCOMPARE(preprocess("#define PUT_BETWEEN(x,y) x y x\nPUT_BETWEEN(pair<a,b>, c)\n"), QString("*ERROR*"));
  
  QEXPECT_FAIL("", "No problems reported for macro-redefinition", Continue);
  QCOMPARE(preprocess("#define A B\n#define A C\n"), QString("*ERROR*"));
}

void TestParser::testPreprocessorStringify() {
  QCOMPARE(preprocess("#define STR(s) #s\n#define MACRO string\nSTR(MACRO)").trimmed(), QString("\"MACRO\""));
  QCOMPARE(preprocess("#define STR(s) #s\n#define XSTR(s) STR(s)\n#define MACRO string\nXSTR(MACRO)").simplified(), QString("\"string\""));
  
  QEXPECT_FAIL("", "# incorrectly handled", Continue);
  QCOMPARE(preprocess("#define CONCAT(x,y) x ## y\n#define test #CONCAT(1,2)\ntest\n").trimmed(), QString("#12"));
}

void TestParser::testStringConcatenation()
{
  QCOMPARE(preprocess("Hello##You"), QString("HelloYou"));
  QCOMPARE(preprocess("#define CONCAT(Var1, Var2) Var1##Var2\nCONCAT(var1, )").trimmed(), QString("var1"));
  QCOMPARE(preprocess("#define CONCAT(Var1, Var2) Var1 ## Var2\nCONCAT(, var2)").trimmed(), QString("var2"));
  QCOMPARE(preprocess("#define CONCAT(Var1, Var2) Var1##Var2 Var2##Var1\nCONCAT(      Hello      ,      You     )").simplified(), QString("\nHelloYou YouHello").simplified());

  QCOMPARE(preprocess("#define GLUE(a, b) a ## b\n#define HIGHLOW hello\nGLUE(HIGH, LOW)").trimmed(), QString("hello"));
  QCOMPARE(preprocess("#define GLUE(a, b) a ## b\n#define HIGHLOW hello\n#define LOW LOW world\nGLUE(HIGH, LOW)").trimmed(), QString("hello"));
  QCOMPARE(preprocess("#define GLUE(a, b) a ##b\n#define XGLUE(a, b) GLUE(a, b)\n#define HIGHLOW hello\n#define LOW LOW world\nXGLUE(HIGH, LOW)").simplified(), QString("hello world")); // TODO: simplified -> trimmed
  QCOMPARE(preprocess("#define GLUE(a, b, c) k ## l ## m\nGLUE(a, b, c)").trimmed(), QString("klm"));
  
  QCOMPARE(preprocess("#define foo(x) foo##x\nint foo\n(13)\n").replace(QRegExp("[\n\t ]+"), " ").trimmed(), QString("int foo13"));
}

void TestParser::testEmptyInclude()
{
  // testcase for https://bugs.kde.org/show_bug.cgi?id=258972
  rpp::Preprocessor preprocessor;
  rpp::pp pp(&preprocessor);
  pp.processFile("anonymous", QByteArray("#include\n\nint main(){\n    ;\n}\n"));
  QCOMPARE(pp.problems().size(), 1);
  qDebug() << pp.problems().first()->description();
  QCOMPARE(pp.problems().first()->finalLocation().start, KDevelop::SimpleCursor(0, 8));
  QCOMPARE(pp.problems().first()->finalLocation().end, KDevelop::SimpleCursor(0, 8));
}

void TestParser::testCondition()
{
  QByteArray method("bool i = (small < big || big > small);");
  TranslationUnitAST* ast = parse(method);
  dumper.dump(ast, lastSession->token_stream);
  ///@todo make this work, it should yield something like TranslationUnit -> SimpleDeclaration -> InitDeclarator -> BinaryExpression
}

void TestParser::testNonTemplateDeclaration()
{
  /*{
    QByteArray templateMethod("template <int> class a {}; int main() { const int b = 1; const int c = 2; a<b|c> d; }");
    TranslationUnitAST* ast = parse(templateMethod);
    dumper.dump(ast, lastSession->token_stream);
  }*/

  //int a, b, c, d; bool e;
  QByteArray declaration("void TestParser::expression() { if (a < b || c > d) {} }");
  TranslationUnitAST* ast = parse(declaration);
  dumper.dump(ast, lastSession->token_stream);
}

void TestParser::testInitListTrailingComma()
{
  //see bug https://bugs.kde.org/show_bug.cgi?id=233328

  QByteArray code("const int foo [] = {1,};");
  TranslationUnitAST* ast = parse(code);
  dump(ast);
  QVERIFY(control.problems().isEmpty());

  QCOMPARE(ast->declarations->count(), 1);
  SimpleDeclarationAST* simpleDecl = reinterpret_cast<SimpleDeclarationAST*>(ast->declarations->at(0)->element);
  QVERIFY(simpleDecl);

  QCOMPARE(simpleDecl->init_declarators->count(), 1);
}

void TestParser::testAsmVolatile()
{
  //see bug https://bugs.kde.org/show_bug.cgi?id=238772
  QByteArray code("__asm__ __volatile__ (\"cld; rep; \" \"stosq\" : \"=c\""
                  "  (__d0), \"=D\" (__d1) : \"a\" (0), \"0\" (sizeof (fd_set) / sizeof (__fd_mask)),"
                  "  \"1\""
                  "  (&((&rfds)->__fds_bits)[0]) : \"memory\");");
  TranslationUnitAST* ast = parse(code);
  dumper.dump(ast, lastSession->token_stream);

  QCOMPARE(ast->declarations->count(), 1);
  AsmDefinitionAST* asmDecl = reinterpret_cast<AsmDefinitionAST*>(ast->declarations->at(0)->element);
  QVERIFY(asmDecl);

  QCOMPARE(asmDecl->cv->count(), 1);
  QVERIFY(lastSession->token_stream->kind(asmDecl->cv->at(0)->element) == Token_volatile);
}

void TestParser::testIncrIdentifier()
{
  //see bug https://bugs.kde.org/show_bug.cgi?id=238772
  QByteArray code("void TestParser::incr();");
  TranslationUnitAST* ast = parse(code);
  dumper.dump(ast, lastSession->token_stream);

  QCOMPARE(ast->declarations->count(), 1);
  FunctionDefinitionAST* funcDecl = reinterpret_cast<FunctionDefinitionAST*>(ast->declarations->at(0)->element);
  QVERIFY(funcDecl);
}

void TestParser::testParseFile()
{
  QFile file(TEST_FILE);
  QVERIFY(file.open(QFile::ReadOnly));
  QByteArray contents = file.readAll();
  file.close();
  TranslationUnitAST* ast =parse(contents);
  QVERIFY(ast != 0);
  QVERIFY(ast->declarations != 0);
}

void TestParser::testQProperty_data()
{
  QTest::addColumn<QByteArray>("code");
  QTest::addColumn<bool>("hasMember");
  QTest::addColumn<bool>("hasGetterMethod");
  QTest::addColumn<bool>("hasSetterMethod");
  QTest::addColumn<bool>("hasResetterMethod");
  QTest::addColumn<bool>("hasNotifierMethod");
  QTest::addColumn<bool>("hasDesignableMethod");
  QTest::addColumn<bool>("hasScriptableMethod");
  QTest::addColumn<bool>("isDesignable");
  QTest::addColumn<bool>("isScriptable");
  QTest::addColumn<bool>("isStored");
  QTest::addColumn<bool>("isUser");
  QTest::addColumn<bool>("isConstant");
  QTest::addColumn<bool>("isFinal");

  QTest::newRow("member") << QByteArray("class Class{\n__qt_property__(bool myProp MEMBER m_prop)\n};")
                          << true << false << false << false << false << false << false
                          << true << true << true << false << false << false;
  QTest::newRow("read") << QByteArray("class Class{\n__qt_property__(bool myProp READ prop)\n};")
                        << false << true << false << false << false << false << false
                        << true << true << true << false << false << false;
  QTest::newRow("write") << QByteArray("class Class{\n__qt_property__(bool myProp READ prop WRITE prop)\n};")
                          << false << true << true << false << false << false << false
                          << true << true << true << false << false << false;
  QTest::newRow("reset") << QByteArray("class Class{\n__qt_property__(bool myProp READ prop RESET prop)\n};")
                          << false << true << false << true << false << false << false
                          << true << true << true << false << false << false;
  QTest::newRow("notify") << QByteArray("class Class{\n__qt_property__(bool myProp READ prop NOTIFY prop)\n};")
                          << false << true << false << false << true << false << false
                          << true << true << true << false << false << false;
  QTest::newRow("desable") << QByteArray("class Class{\n__qt_property__(bool myProp READ prop DESIGNABLE prop)\n};")
                            << false << true << false << false << false << true << false
                            << true << true << true << false << false << false;
  QTest::newRow("scpable") << QByteArray("class Class{\n__qt_property__(bool myProp READ prop SCRIPTABLE prop)\n};")
                            << false << true << false << false << false << false << true
                            << true << true << true << false << false << false;
  QTest::newRow("desvalue") << QByteArray("class Class{\n__qt_property__(bool myProp READ prop DESIGNABLE false)\n};")
                            << false << true << false << false << false << false << false
                            << false << true << true << false << false << false;
  QTest::newRow("scpvalue") << QByteArray("class Class{\n__qt_property__(bool myProp READ prop SCRIPTABLE false)\n};")
                            << false << true << false << false << false << false << false
                            << true << false << true << false << false << false;
  QTest::newRow("stored") << QByteArray("class Class{\n__qt_property__(bool myProp READ prop STORED false)\n};")
                          << false << true << false << false << false << false << false
                          << true << true << false << false << false << false;
  QTest::newRow("user") << QByteArray("class Class{\n__qt_property__(bool myProp READ prop USER true)\n};")
                        << false << true << false << false << false << false << false
                        << true << true << true << true << false << false;
  QTest::newRow("constant") << QByteArray("class Class{\n__qt_property__(bool myProp READ prop CONSTANT)\n};")
                            << false << true << false << false << false << false << false
                            << true << true << true << false << true << false;
  QTest::newRow("final") << QByteArray("class Class{\n__qt_property__(bool myProp READ prop FINAL)\n};")
                          << false << true << false << false << false << false << false
                          << true << true << true << false << false << true;
}

void TestParser::testQProperty()
{
  QFETCH(QByteArray, code);
  QFETCH(bool, hasMember);
  QFETCH(bool, hasGetterMethod);
  QFETCH(bool, hasSetterMethod);
  QFETCH(bool, hasResetterMethod);
  QFETCH(bool, hasNotifierMethod);
  QFETCH(bool, hasDesignableMethod);
  QFETCH(bool, hasScriptableMethod);
  QFETCH(bool, isDesignable);
  QFETCH(bool, isScriptable);
  QFETCH(bool, isStored);
  QFETCH(bool, isUser);
  QFETCH(bool, isConstant);
  QFETCH(bool, isFinal);

  TranslationUnitAST* ast = parse(code);

  QVERIFY(ast != 0);
  QVERIFY(hasKind(ast, AST::Kind_QPropertyDeclaration));

  QPropertyDeclarationAST* propAst = static_cast<QPropertyDeclarationAST*>
                                                (getAST(ast, AST::Kind_QPropertyDeclaration));

  QVERIFY((propAst->member != 0) == hasMember);
  QVERIFY((propAst->getter != 0) == hasGetterMethod);
  QVERIFY((propAst->setter != 0) == hasSetterMethod);
  QVERIFY((propAst->resetter != 0) == hasResetterMethod);
  QVERIFY((propAst->notifier != 0) == hasNotifierMethod);
  QVERIFY((propAst->designableMethod != 0) == hasDesignableMethod);
  QVERIFY((propAst->scriptableMethod != 0) == hasScriptableMethod);
  QCOMPARE(propAst->designableValue, isDesignable);
  QCOMPARE(propAst->scriptableValue, isScriptable);
  QCOMPARE(propAst->stored, isStored);
  QCOMPARE(propAst->user, isUser);
  QCOMPARE(propAst->constant, isConstant);
  QCOMPARE(propAst->final, isFinal);
}

void TestParser::testDesignatedInitializers()
{
  TranslationUnitAST* ast;
  InitializerListAST* listAst;
  //DumpTree dumper;

  ast = parse("\nA a = {"
              "\n  .b = {"
              "\n    .a = 10,"
              "\n  },"
              "\n  .x = 10,"
              "\n  .y = SOME_CONST,"
              "\n  .z = 10,"
              "\n};");

  QVERIFY(ast != 0);
  QVERIFY(control.problems().isEmpty());
  QCOMPARE(ast->declarations->count(), 1);
  QVERIFY(hasKind(ast, AST::Kind_InitializerList));
  listAst = static_cast<InitializerListAST*>(getAST(ast, AST::Kind_InitializerList));
  QVERIFY(hasKind(listAst, AST::Kind_ClassMemberAccess));
  //dumper.dump(ast, lastSession->token_stream);

  ast = parse("\nint ia[10][5] = {"
              "\n  [1] = 10,"
              "\n  [2][B] = SOME_CONST,"
              "\n};");

  QVERIFY(ast != 0);
  QVERIFY(control.problems().isEmpty());
  QCOMPARE(ast->declarations->count(), 1);
  QVERIFY(hasKind(ast, AST::Kind_InitializerList));
  listAst = static_cast<InitializerListAST*>(getAST(ast, AST::Kind_InitializerList));
  QVERIFY(hasKind(listAst, AST::Kind_SubscriptExpression));
  //dumper.dump(ast, lastSession->token_stream);

}

void TestParser::testCommentAfterFunctionCall() {
  //this is ambigous
  TranslationUnitAST* ast = parse("void TestParser::setView() {\n"
                                  "  setView(m_view); //\n"
                                  "}\n");

  QVERIFY(ast != 0);

  DumpTree dumper;
  dumper.dump(ast, lastSession->token_stream);

  QCOMPARE(ast->declarations->count(), 1);
  QVERIFY(hasKind(ast, AST::Kind_FunctionDefinition));
  FunctionDefinitionAST* funcAst = static_cast<FunctionDefinitionAST*>(getAST(ast, AST::Kind_FunctionDefinition));
  QVERIFY(hasKind(funcAst, AST::Kind_ExpressionOrDeclarationStatement));
  QVERIFY(hasKind(funcAst, AST::Kind_FunctionCall));
  QVERIFY(hasKind(funcAst, AST::Kind_InitDeclarator));
  ExpressionOrDeclarationStatementAST* ambAst = static_cast<ExpressionOrDeclarationStatementAST*>(getAST(funcAst, AST::Kind_ExpressionOrDeclarationStatement));
  QVERIFY(ambAst);
}

void TestParser::testPtrToMemberAst() {
  TranslationUnitAST* ast = parse("\nstruct AA {"
                                  "\n  int j;"
                                  "\n};"
                                  "\nstruct BB{"
                                  "\n  int AA::* pj;"
                                  "\n};"
                                  "\nvoid TestParser::f(){"
                                  "\n  int AA::* BB::* ppj=&BB::pj;"
                                  "\n}"
                                  );
  QVERIFY(ast!=0);
  QCOMPARE(ast->declarations->count(), 3);
  QVERIFY(hasKind(ast,AST::Kind_PtrToMember));
  FunctionDefinitionAST* f_ast=static_cast<FunctionDefinitionAST*>(getAST(ast,AST::Kind_FunctionDefinition));
  QVERIFY(hasKind(f_ast,AST::Kind_PtrToMember));
  DeclaratorAST* d_ast=static_cast<DeclaratorAST*>(getAST(f_ast->function_body,AST::Kind_Declarator));
  QCOMPARE(d_ast-> ptr_ops->count(),2);
}

void TestParser::testSwitchStatement()
{
  int problemCount = control.problems().count();

  QByteArray switchTest("int main() { switch(0); }");
  parse(switchTest);
  QCOMPARE(control.problems().count(), problemCount);
  QByteArray switchTest2("int main() { switch (0) case 0: if (true) ; else return 1; }");
  parse(switchTest2);
  QCOMPARE(control.problems().count(), problemCount);
  QByteArray switchTest3("int main() { switch (0) { case 0: if (true) ; else return 1; } }");
  parse(switchTest3);
  QCOMPARE(control.problems().count(), problemCount);
  QByteArray switchTest4("int main() { switch (0) while(true) return false; }");
  parse(switchTest4);
  QCOMPARE(control.problems().count(), problemCount);
  QByteArray switchTest5("int main() { switch (0) { case 0: return 0; } }");
  parse(switchTest5);
  QCOMPARE(control.problems().count(), problemCount);
}

void TestParser::testNamedOperators_data()
{
  QTest::addColumn<QString>("code");
  QTest::newRow("xor") << "int i = 1 xor 2;";
  QTest::newRow("bitand") << "int i = 1 bitand 2;";
  QTest::newRow("bitor") << "int i = 1 bitor 2;";
  QTest::newRow("or") << "int i = 1 or 2;";
  QTest::newRow("and") << "int i = 1 and 2;";
  QTest::newRow("compl") << "int i = compl 2;";
  QTest::newRow("not") << "int i = not 2;";
  QTest::newRow("xor_eq") << "int i = 1; i xor_eq 2;";
  QTest::newRow("and_eq") << "int i = 1; i and_eq 2;";
  QTest::newRow("or_eq") << "int i = 1; i or_eq 2;";
  QTest::newRow("not_eq") << "int i = 1 not_eq 2;";
}

void TestParser::testNamedOperators()
{
  QFETCH(QString, code);
  code = "int main() { " + code + " }\n";
  parse(code.toLocal8Bit());
  QVERIFY(control.problems().isEmpty());
}

void TestParser::testOperators_data()
{
  QTest::addColumn<QString>("code");
  QTest::newRow("xor") << "int i = 1 ^ 2;";
  QTest::newRow("bitand") << "int i = 1 & 2;";
  QTest::newRow("bitor") << "int i = 1 | 2;";
  QTest::newRow("or") << "int i = 1 || 2;";
  QTest::newRow("and") << "int i = 1 && 2;";
  QTest::newRow("compl") << "int i = ~2;";
  QTest::newRow("not") << "int i = !2;";
  QTest::newRow("xor_eq") << "int i = 1; i ^= 2;";
  QTest::newRow("and_eq") << "int i = 1; i &= 2;";
  QTest::newRow("or_eq") << "int i = 1; i |= 2;";
  QTest::newRow("not_eq") << "int i = 1 != 2;";
}

void TestParser::testOperators()
{
  QFETCH(QString, code);
  code = "int main() { " + code + " }\n";
  parse(code.toLocal8Bit());
  QVERIFY(control.problems().isEmpty());
}

void TestParser::testTypeID_data()
{
  QTest::addColumn<QString>("code");
  QTest::newRow("typeid-type") << "typeid(int);";
  QTest::newRow("typeid-value") << "typeid(5);";
  QTest::newRow("typeid-var") << "float f; typeid(f);";
  QTest::newRow("type_info-name") << "typeid(1).name();";
  QTest::newRow("type_info-name-stream") << "cout << typeid(1).name() << endl;";
  QTest::newRow("type_info-op") << "bool b = typeid(1) == typeid(int);";
}

void TestParser::testTypeID()
{
  QFETCH(QString, code);
  code = "int main() { " + code + " }\n";
  TranslationUnitAST* ast = parse(code.toLocal8Bit());
  dumper.dump(ast, lastSession->token_stream);
  QVERIFY(control.problems().isEmpty());
}

void TestParser::testRegister()
{
  // see also: http://bugsfiles.kde.org/attachment.cgi?id=61647
  QString code = "void foo() { register int i; int register j; }\n";
  TranslationUnitAST* ast = parse(code.toLocal8Bit());
  dumper.dump(ast, lastSession->token_stream);
  QVERIFY(control.problems().isEmpty());
}

void TestParser::inlineTemplate()
{
  QByteArray code = "template <typename T> inline void a() {}\n";
  TranslationUnitAST* ast = parse(code);
  dumper.dump(ast, lastSession->token_stream);
  QVERIFY(control.problems().isEmpty());
}

void TestParser::testMultiByteCStrings()
{
  //                 0         1         2          3          4
  //                 01234567890123456789012345678 90 1234567890123456789
  QByteArray code = "int main() { const char* a = \"ä\"; a = 0; }\n";
  TranslationUnitAST* ast = parse(code);
  dumper.dump(ast, lastSession->token_stream);
  QVERIFY(control.problems().isEmpty());
  AST* str = getAST(ast, AST::Kind_StringLiteral);
  QVERIFY(str);
  QCOMPARE(lastSession->stringForNode(str, true), QString::fromUtf8("\"ä\""));
  Token token = lastSession->token_stream->token(str->start_token);
  QEXPECT_FAIL("", "the wide ä-char takes two indizes in a QByteArray, which breaks our lexer", Abort);
  QCOMPARE(token.size, 3u);
  QCOMPARE(lastSession->token_stream->symbolLength(token), 3u);
  Token endToken = lastSession->token_stream->token(str->end_token);
  rpp::Anchor pos = lastSession->positionAt(endToken.position);
  // should end just before the semicolon
  QVERIFY(pos == KDevelop::CursorInRevision(0, 32));
}

void TestParser::testMultiByteComments()
{
  //                 0         1         2         3          4
  //                 01234567890123456789012345678901234567890123456789
  QByteArray code = "int a = 1;/* ä */int b = 0;";
  TranslationUnitAST* ast = parse(code);
  dumper.dump(ast, lastSession->token_stream);
  QVERIFY(control.problems().isEmpty());
  AST* b = lastSession->topAstNode()->declarations->toBack()->element;
  Token token = lastSession->token_stream->token(b->start_token);
  rpp::Anchor pos = lastSession->positionAt(token.position);
  // should start just after the comment
  QEXPECT_FAIL("", "the wide ä-char takes two indizes in a QByteArray, which breaks our lexer", Abort);
  QVERIFY(pos == KDevelop::CursorInRevision(0, 17));
}

void TestParser::testTernaryEmptyExpression()
{
  // see also: https://bugs.kde.org/show_bug.cgi?id=292357
  // mostly GCC compatibility
  //                 0         1         2         3          4
  //                 01234567890123456789012345678901234567890123456789
  QByteArray code = "int a = false ?: 0;";
  TranslationUnitAST* ast = parse(code);
  dumper.dump(ast, lastSession->token_stream);
  QCOMPARE(control.problems().count(), 1);
  QCOMPARE(control.problems().first()->severity(), KDevelop::ProblemData::Warning);
  QVERIFY(ast);
}

TranslationUnitAST* TestParser::parse(const QByteArray& unit)
{
  control = Control(); // Clear the problems
  Parser parser(&control);
  lastSession = new ParseSession();

  rpp::Preprocessor preprocessor;
  rpp::pp pp(&preprocessor);
  lastSession->setContentsAndGenerateLocationTable(pp.processFile("anonymous", unit));
  return  parser.parse(lastSession);
}

void TestParser::dump(AST* node)
{
  dumper.dump(node, lastSession->token_stream);
  if (!control.problems().isEmpty()) {
    foreach(const KDevelop::ProblemPointer&p, control.problems()) {
      qDebug() << p->description() << p->explanation() << p->finalLocation().textRange();
    }
  }
}

struct HasKindVisitor : protected DefaultVisitor
{

  AST::NODE_KIND kind;
  AST* ast;
  int num;

  HasKindVisitor(AST::NODE_KIND kind, int num = 0)
    : kind(kind), ast(0), num(num)
  {
  }

  bool hasKind() const
  {
    return ast != 0;
  }

  void visit(AST* node)
  {
    if (!ast && node) {
      if (node->kind == kind && --num < 0) {
	ast = node;
      }
      else {
	DefaultVisitor::visit(node);
      }
    }
  }
};

bool TestParser::hasKind(AST* ast, AST::NODE_KIND kind)
{
  HasKindVisitor visitor(kind);
  visitor.visit(ast);
  return visitor.hasKind();
}

AST* TestParser::getAST(AST* ast, AST::NODE_KIND kind, int num)
{
  HasKindVisitor visitor(kind, num);
  visitor.visit(ast);
  return visitor.ast;
}

#include "test_parser.moc"

QTEST_MAIN(TestParser)
