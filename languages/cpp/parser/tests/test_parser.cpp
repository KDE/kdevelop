#include <QtTest/QtTest>

#define private public
#include "ast.h"
#undef private

#include "parser.h"
#include "rpp/preprocessor.h"
#include "control.h"
#include "dumptree.h"
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

bool hasKind(AST*, AST::NODE_KIND);
AST* getAST(AST*, AST::NODE_KIND, int num = 0);

class TestParser : public QObject
{
  Q_OBJECT

  Control control;
  DumpTree dumper;

public:
  TestParser()
  {
  }

private slots:

  void initTestCase()
  {
    KDevelop::AutoTestShell::init();
    KDevelop::TestCore* core = new KDevelop::TestCore();
    core->initialize(KDevelop::Core::NoUi);
  }

  void testSymbolTable()
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
//   void testControlContexts()
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

  void testTokenTable()
  {
    QCOMPARE(token_name(Token_EOF), "eof");
    QCOMPARE(token_name('a'), "a");
    QCOMPARE(token_name(Token_delete), "delete");
  }

///@todo reenable
//   void testLexer()
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

  void testParser()
  {
    QByteArray clazz("struct A { int i; A() : i(5) { } virtual void test() = 0; };");
    pool mem_pool;
    TranslationUnitAST* ast = parse(clazz, &mem_pool);
    QVERIFY(ast != 0);
    QVERIFY(ast->declarations != 0);
  }
  
  void testTemplateArguments()
  {
    QByteArray templatetest("template <int N, int M> struct SeriesAdder{ enum { value = N + SeriesAdder< 0 >::value }; };");
    pool mem_pool;
    TranslationUnitAST* ast = parse(templatetest, &mem_pool);
    QVERIFY(ast != 0);
    QVERIFY(ast->declarations != 0);
    QVERIFY(control.problems().isEmpty());
  }
  
  void testManyComparisons()
  {
    //Should not crash
    {
      QByteArray clazz("void test() { if(val < f && val < val1 && val < val2 && val < val3 ){ } }");
      pool mem_pool;
      TranslationUnitAST* ast = parse(clazz, &mem_pool);
      QVERIFY(ast != 0);
      QVERIFY(ast->declarations != 0);
      dumper.dump(ast, lastSession->token_stream);
    }
    {
      QByteArray clazz("void test() { if(val < f && val < val1 && val < val2 && val < val3 && val < val4 && val < val5 && val < val6 && val < val7 && val < val8 && val < val9 && val < val10 && val < val11 && val < val12 && val < val13 && val < val14 && val < val15 && val < val16 && val < val17 && val < val18 && val < val19 && val < val20 && val < val21 && val < val22 && val < val23 && val < val24 && val < val25 && val < val26){ } }");
      pool mem_pool;
      TranslationUnitAST* ast = parse(clazz, &mem_pool);
      QVERIFY(ast != 0);
      QVERIFY(ast->declarations != 0);
    }
  }
  
  void testParserFail()
  {
    QByteArray stuff("foo bar !!! nothing that really looks like valid c++ code");
    pool mem_pool;
    TranslationUnitAST *ast = parse(stuff, &mem_pool);
    QVERIFY(ast->declarations == 0);
    QVERIFY(control.problems().count() > 3);
  }

  void testPartialParseFail() {
    {
    QByteArray method("struct C { Something invalid is here };");
    pool mem_pool;
    TranslationUnitAST* ast = parse(method, &mem_pool);
    QVERIFY(ast != 0);
    QVERIFY(hasKind(ast, AST::Kind_ClassSpecifier));
    }
    {
    QByteArray method("void test() { Something invalid is here };");
    pool mem_pool;
    TranslationUnitAST* ast = parse(method, &mem_pool);
    QVERIFY(ast != 0);
    QVERIFY(hasKind(ast, AST::Kind_FunctionDefinition));
    }
    {
    QByteArray method("void test() { {Something invalid is here };");
    pool mem_pool;
    TranslationUnitAST* ast = parse(method, &mem_pool);
    QVERIFY(ast != 0);
    QVERIFY(hasKind(ast, AST::Kind_FunctionDefinition));
    QVERIFY(ast->hadMissingCompoundTokens);
    }
    {
    QByteArray method("void test() { case:{};");
    pool mem_pool;
    TranslationUnitAST* ast = parse(method, &mem_pool);
    QVERIFY(ast != 0);
    QVERIFY(hasKind(ast, AST::Kind_FunctionDefinition));
    QVERIFY(ast->hadMissingCompoundTokens);
    }
  }

  void testParseMethod()
  {
    QByteArray method("void A::test() {  }");
    pool mem_pool;
    TranslationUnitAST* ast = parse(method, &mem_pool);
    QVERIFY(ast != 0);
    QVERIFY(hasKind(ast, AST::Kind_FunctionDefinition));
  }

///@todo reenable
//   void testMethodArgs()
//   {
//     QByteArray method("int A::test(int primitive, B* pointer) { return primitive; }");
//     pool mem_pool;
//     Parser parser(&control);
//     TranslationUnitAST* ast = parser.parse(method.constData(),
// 					   method.size() + 1, &mem_pool);
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

  void testForStatements()
  {
    QByteArray method("void A::t() { for (int i = 0; i < 10; i++) { ; }}");
    pool mem_pool;
    TranslationUnitAST* ast = parse(method, &mem_pool);

    QVERIFY(control.problems().isEmpty());
    
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

    QVERIFY(control.problems().isEmpty());
    QVERIFY(hasKind(ast, AST::Kind_Condition));
    QVERIFY(hasKind(ast, AST::Kind_BinaryExpression));
  }

  void testComments()
  {
    QByteArray method("//TranslationUnitComment\n//Hello\nint A; //behind\n /*between*/\n /*Hello2*/\n class B{}; //behind\n//Hello3\n //beforeTest\nvoid test(); //testBehind");
    pool mem_pool;
    TranslationUnitAST* ast = parse(method, &mem_pool);

    CommentFormatter formatter;
    
    QCOMPARE(formatter.formatComment(ast->comments, lastSession), QByteArray("TranslationUnitComment")); //The comments were merged

    const ListNode<DeclarationAST*>* it = ast->declarations;
    QVERIFY(it);
    it = it->next;
    QVERIFY(it);
    QCOMPARE(formatter.formatComment(it->element->comments, lastSession), QByteArray("Hello\n(behind)"));

    it = it->next;
    QVERIFY(it);
    QCOMPARE(formatter.formatComment(it->element->comments, lastSession), QByteArray("between\nHello2\n(behind)"));

    it = it->next;
    QVERIFY(it);
    QCOMPARE(formatter.formatComment(it->element->comments, lastSession), QByteArray("Hello3\nbeforeTest\n(testBehind)"));
  }

  void testComments2()
  {
    CommentFormatter formatter;
    QByteArray method("enum Enum\n {//enumerator1Comment\nenumerator1, //enumerator1BehindComment\n /*enumerator2Comment*/ enumerator2 /*enumerator2BehindComment*/};");
    pool mem_pool;
    TranslationUnitAST* ast = parse(method, &mem_pool);

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

  void testComments3()
  {
    CommentFormatter formatter;
    QByteArray method("class Class{\n//Comment\n int val;};");
    pool mem_pool;
    TranslationUnitAST* ast = parse(method, &mem_pool);

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

  void testComments4()
  {
    CommentFormatter formatter;
    QByteArray method("//TranslationUnitComment\n//Comment\ntemplate<class C> class Class{};");
    pool mem_pool;
    TranslationUnitAST* ast = parse(method, &mem_pool);

    const ListNode<DeclarationAST*>* it = ast->declarations;
    QVERIFY(it);
    it = it->next;
    QVERIFY(it);
    const TemplateDeclarationAST* templDecl = static_cast<const TemplateDeclarationAST*>(it->element);
    QVERIFY(templDecl);
    QVERIFY(templDecl->declaration);

    //QCOMPARE(formatter.formatComment(templDecl->declaration->comments, lastSession), QString("Comment"));
  }
  
  void testComments5()
  {
    CommentFormatter formatter;
    QByteArray method("//TranslationUnitComment\n  //FIXME comment\n //this is TODO\n /* TODO: comment */\n  int i;  // another TODO \n // Just a simple comment\nint j;\n int main(void) {\n // TODO COMMENT\n}\n");
    pool mem_pool;
    int initial_size = control.problems().size();  // Remember existing number of problems
    TranslationUnitAST* ast = parse(method, &mem_pool);

    const ListNode<DeclarationAST*>* it = ast->declarations;
    QVERIFY(it);
    it = it->next;
    QVERIFY(it);
    QCOMPARE(formatter.formatComment(it->element->comments, lastSession), QByteArray("FIXME comment\nthis is TODO\n TODO: comment\n(another TODO)"));
    it = it->next;
    QVERIFY(it);
    QCOMPARE(formatter.formatComment(it->element->comments, lastSession), QByteArray("Just a simple comment"));

    QList<KDevelop::ProblemPointer> problem_list = control.problems();
    QCOMPARE(problem_list.size(), initial_size + 5); // 5 to-dos
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
  }

  void testComments6() {
    QByteArray module("//TranslationUnitComment\n/**\n * foo\n **/\nint i;\n");
    pool mem_pool;
    TranslationUnitAST* ast = parse(module, &mem_pool);
    const ListNode<DeclarationAST*>* it = ast->declarations;
    QVERIFY(it);
    it = it->next;
    QVERIFY(it);
    QCOMPARE(CommentFormatter().formatComment(it->element->comments, lastSession), QByteArray("foo"));
  }

  QString preprocess(const QString& contents) {
    rpp::Preprocessor preprocessor;
    rpp::pp pp(&preprocessor);
    QByteArray qba = stringFromContents(pp.processFile("anonymous", contents.toUtf8()));
    if(pp.problems().empty())
      return QString::fromUtf8(qba);
    else
      return "*ERROR*";
  }

  void testPreprocessor() {
    rpp::Preprocessor preprocessor;
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
    
    QEXPECT_FAIL("", "No problems reported for missmatching macro-parameter-lists", Continue);
    QCOMPARE(preprocess("#define bla(x,y)\nbla(1,2,3)\n"), QString("*ERROR*"));
    
    QEXPECT_FAIL("", "No problems reported for missmatching macro-parameter-lists", Continue);
    QCOMPARE(preprocess("#define PUT_BETWEEN(x,y) x y x\nPUT_BETWEEN(pair<a,b>, c)\n"), QString("*ERROR*"));
    
    QEXPECT_FAIL("", "No problems reported for macro-redefinition", Continue);
    QCOMPARE(preprocess("#define A B\n#define A C\n"), QString("*ERROR*"));
  }

  void testPreprocessorStringify() {
    QCOMPARE(preprocess("#define STR(s) #s\n#define MACRO string\nSTR(MACRO)").trimmed(), QString("\"MACRO\""));
    QCOMPARE(preprocess("#define STR(s) #s\n#define XSTR(s) STR(s)\n#define MACRO string\nXSTR(MACRO)").simplified(), QString("\"string\""));
    
    QEXPECT_FAIL("", "# incorrectly handled", Continue);
    QCOMPARE(preprocess("#define CONCAT(x,y) x ## y\n#define test #CONCAT(1,2)\ntest\n").trimmed(), QString("#12"));
  }

  void testStringConcatenation()
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

  void testEmptyInclude()
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

  void testCondition()
  {
    QByteArray method("bool i = (small < big || big > small);");
    pool mem_pool;
    TranslationUnitAST* ast = parse(method, &mem_pool);
    dumper.dump(ast, lastSession->token_stream);
    ///@todo make this work, it should yield something like TranslationUnit -> SimpleDeclaration -> InitDeclarator -> BinaryExpression
  }

  void testNonTemplateDeclaration()
  {
    /*{
      QByteArray templateMethod("template <int> class a {}; int main() { const int b = 1; const int c = 2; a<b|c> d; }");
      pool mem_pool;
      TranslationUnitAST* ast = parse(templateMethod, &mem_pool);
      dumper.dump(ast, lastSession->token_stream);
    }*/

    //int a, b, c, d; bool e;
    QByteArray declaration("void expression() { if (a < b || c > d) {} }");
    pool mem_pool;
    TranslationUnitAST* ast = parse(declaration, &mem_pool);
    dumper.dump(ast, lastSession->token_stream);
  }

  void testInitListTrailingComma()
  {
    //see bug https://bugs.kde.org/show_bug.cgi?id=233328

    QByteArray code("const int foo [] = {1,};");
    pool memPool;
    TranslationUnitAST* ast = parse(code, &memPool);
    dumper.dump(ast, lastSession->token_stream);

    QCOMPARE(ast->declarations->count(), 1);
    SimpleDeclarationAST* simpleDecl = reinterpret_cast<SimpleDeclarationAST*>(ast->declarations->at(0)->element);
    QVERIFY(simpleDecl);

    QCOMPARE(simpleDecl->init_declarators->count(), 1);
    
  }

  void testAsmVolatile()
  {
    //see bug https://bugs.kde.org/show_bug.cgi?id=238772
    QByteArray code("__asm__ __volatile__ (\"cld; rep; \" \"stosq\" : \"=c\""
                    "  (__d0), \"=D\" (__d1) : \"a\" (0), \"0\" (sizeof (fd_set) / sizeof (__fd_mask)),"
                    "  \"1\""
                    "  (&((&rfds)->__fds_bits)[0]) : \"memory\");");
    pool memPool;
    TranslationUnitAST* ast = parse(code, &memPool);
    dumper.dump(ast, lastSession->token_stream);

    QCOMPARE(ast->declarations->count(), 1);
    AsmDefinitionAST* asmDecl = reinterpret_cast<AsmDefinitionAST*>(ast->declarations->at(0)->element);
    QVERIFY(asmDecl);

    QCOMPARE(asmDecl->cv->count(), 1);
    QVERIFY(lastSession->token_stream->kind(asmDecl->cv->at(0)->element) == Token_volatile);
  }

  void testIncrIdentifier()
  {
    //see bug https://bugs.kde.org/show_bug.cgi?id=238772
    QByteArray code("void incr();");
    pool memPool;
    TranslationUnitAST* ast = parse(code, &memPool);
    dumper.dump(ast, lastSession->token_stream);

    QCOMPARE(ast->declarations->count(), 1);
    FunctionDefinitionAST* funcDecl = reinterpret_cast<FunctionDefinitionAST*>(ast->declarations->at(0)->element);
    QVERIFY(funcDecl);
  }

  /*void testParseFile()
  {
     QFile file(TEST_FILE);
     QVERIFY(file.open(QFile::ReadOnly));
     QByteArray contents = file.readAll();
     file.close();
     pool mem_pool;
     Parser parser(&control);
     ParseSession session;
     session.setContents(contents);
     TranslationUnitAST* ast = parser.parse(&session);
     QVERIFY(ast != 0);
     QVERIFY(ast->declarations != 0);
   }*/

  void testQProperty_data()
  {
    QTest::addColumn<QByteArray>("code");
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

    QTest::newRow("read") << QByteArray("class Class{\n__qt_property__(bool myProp READ prop)\n};")
                          << true << false << false << false << false << false
                          << true << true << true << false << false << false;
    QTest::newRow("write") << QByteArray("class Class{\n__qt_property__(bool myProp READ prop WRITE prop)\n};")
                           << true << true << false << false << false << false
                           << true << true << true << false << false << false;
    QTest::newRow("reset") << QByteArray("class Class{\n__qt_property__(bool myProp READ prop RESET prop)\n};")
                           << true << false << true << false << false << false
                           << true << true << true << false << false << false;
    QTest::newRow("notify") << QByteArray("class Class{\n__qt_property__(bool myProp READ prop NOTIFY prop)\n};")
                            << true << false << false << true << false << false
                            << true << true << true << false << false << false;
    QTest::newRow("desable") << QByteArray("class Class{\n__qt_property__(bool myProp READ prop DESIGNABLE prop)\n};")
                             << true << false << false << false << true << false
                             << true << true << true << false << false << false;
    QTest::newRow("scpable") << QByteArray("class Class{\n__qt_property__(bool myProp READ prop SCRIPTABLE prop)\n};")
                             << true << false << false << false << false << true
                             << true << true << true << false << false << false;
    QTest::newRow("desvalue") << QByteArray("class Class{\n__qt_property__(bool myProp READ prop DESIGNABLE false)\n};")
                              << true << false << false << false << false << false
                              << false << true << true << false << false << false;
    QTest::newRow("scpvalue") << QByteArray("class Class{\n__qt_property__(bool myProp READ prop SCRIPTABLE false)\n};")
                              << true << false << false << false << false << false
                              << true << false << true << false << false << false;
    QTest::newRow("stored") << QByteArray("class Class{\n__qt_property__(bool myProp READ prop STORED false)\n};")
                            << true << false << false << false << false << false
                            << true << true << false << false << false << false;
    QTest::newRow("user") << QByteArray("class Class{\n__qt_property__(bool myProp READ prop USER true)\n};")
                          << true << false << false << false << false << false
                          << true << true << true << true << false << false;
    QTest::newRow("constant") << QByteArray("class Class{\n__qt_property__(bool myProp READ prop CONSTANT)\n};")
                              << true << false << false << false << false << false
                              << true << true << true << false << true << false;
    QTest::newRow("final") << QByteArray("class Class{\n__qt_property__(bool myProp READ prop FINAL)\n};")
                           << true << false << false << false << false << false
                           << true << true << true << false << false << true;
  }

  void testQProperty()
  {
    QFETCH(QByteArray, code);
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

    pool memPool;
    TranslationUnitAST* ast = parse(code, &memPool);

    QVERIFY(ast != 0);
    QVERIFY(hasKind(ast, AST::Kind_QPropertyDeclaration));

    QPropertyDeclarationAST* propAst = static_cast<QPropertyDeclarationAST*>
                                                  (getAST(ast, AST::Kind_QPropertyDeclaration));

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

  void testCommentAfterFunctionCall() {
    //this is ambigous
    pool memPool;
    TranslationUnitAST* ast = parse("void setView() {\n"
                                    "  setView(m_view); //\n"
                                    "}\n", &memPool);

    QVERIFY(ast != 0);

    DumpTree dumper;
    dumper.dump(ast, lastSession->token_stream);

    QCOMPARE(ast->declarations->count(), 1);
    QVERIFY(hasKind(ast, AST::Kind_FunctionDefinition));
    FunctionDefinitionAST* funcAst = static_cast<FunctionDefinitionAST*>(getAST(ast, AST::Kind_FunctionDefinition));
    QVERIFY(hasKind(funcAst, AST::Kind_ExpressionOrDeclarationStatement));
    ExpressionOrDeclarationStatementAST* ambAst = static_cast<ExpressionOrDeclarationStatementAST*>(getAST(funcAst, AST::Kind_ExpressionOrDeclarationStatement));
    QVERIFY(hasKind(funcAst, AST::Kind_FunctionCall));
    QVERIFY(hasKind(funcAst, AST::Kind_InitDeclarator));
  }

  void testPtrToMemberAst() {
    pool memPool;
    TranslationUnitAST* ast = parse("\nstruct AA {"
                                    "\n  int j;"
                                    "\n};"
                                    "\nstruct BB{"
                                    "\n  int AA::* pj;"
                                    "\n};"
                                    "\nvoid f(){"
                                    "\n  int AA::* BB::* ppj=&BB::pj;"
                                    "\n}"
                                    , &memPool);
    QVERIFY(ast!=0);
    QCOMPARE(ast->declarations->count(), 3);
    QVERIFY(hasKind(ast,AST::Kind_PtrToMember));
    FunctionDefinitionAST* f_ast=static_cast<FunctionDefinitionAST*>(getAST(ast,AST::Kind_FunctionDefinition));
    QVERIFY(hasKind(f_ast,AST::Kind_PtrToMember));
    DeclaratorAST* d_ast=static_cast<DeclaratorAST*>(getAST(f_ast->function_body,AST::Kind_Declarator));
    QCOMPARE(d_ast-> ptr_ops->count(),2);
  }

  void testSwitchStatement()
  {
    int problemCount = control.problems().count();
    pool mem_pool;

    QByteArray switchTest("int main() { switch(0); }");
    parse(switchTest, &mem_pool);
    QCOMPARE(control.problems().count(), problemCount);
    QByteArray switchTest2("int main() { switch (0) case 0: if (true) ; else return 1; }");
    parse(switchTest2, &mem_pool);
    QCOMPARE(control.problems().count(), problemCount);
    QByteArray switchTest3("int main() { switch (0) { case 0: if (true) ; else return 1; } }");
    parse(switchTest3, &mem_pool);
    QCOMPARE(control.problems().count(), problemCount);
    QByteArray switchTest4("int main() { switch (0) while(true) return false; }");
    parse(switchTest4, &mem_pool);
    QCOMPARE(control.problems().count(), problemCount);
    QByteArray switchTest5("int main() { switch (0) { case 0: return 0; } }");
    parse(switchTest5, &mem_pool);
    QCOMPARE(control.problems().count(), problemCount);
  }

private:
  ParseSession* lastSession;

  TranslationUnitAST* parse(const QByteArray& unit, pool* mem_pool)
  {
    control = Control(); // Clear the problems
    Parser parser(&control);
    lastSession = new ParseSession();
    lastSession->setContentsAndGenerateLocationTable(tokenizeFromByteArray(unit));
    return  parser.parse(lastSession);
  }



};

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

bool hasKind(AST* ast, AST::NODE_KIND kind)
{
  HasKindVisitor visitor(kind);
  visitor.visit(ast);
  return visitor.hasKind();
}

AST* getAST(AST* ast, AST::NODE_KIND kind, int num)
{
  HasKindVisitor visitor(kind, num);
  visitor.visit(ast);
  return visitor.ast;
}


#include "test_parser.moc"

QTEST_MAIN(TestParser)
