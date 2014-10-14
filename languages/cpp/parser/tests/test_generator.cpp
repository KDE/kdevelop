#include <QtTest/QtTest>

#include "ast.h"
#include "parser.h"
#include "rpp/preprocessor.h"
#include "control.h"
#include "dumptree.h"
#include "tokens.h"
#include "parsesession.h"
#include "commentformatter.h"
#include "codegenerator.h"

#include "testconfig.h"

#include <QByteArray>
#include <QDataStream>
#include <QFile>

#include <iostream>
#include <rpp/chartools.h>
#include <rpp/pp-engine.h>

#include <tests/autotestshell.h>
#include <tests/testcore.h>

static ParseSession* lastSession = 0;
static ParseSession* lastGeneratedSession = 0;

struct ComparableToken
{
  ComparableToken(const Token& token, ParseSession* session)
  : m_token(token)
  , m_session(session)
  {
  }
  inline QString toString() const
  {
    return m_session->token_stream->symbolString(m_token);
  }
  inline bool operator==(const ComparableToken& o) const
  {
    return o.m_token.kind == m_token.kind && toString() == o.toString();
  }
  Token m_token;
  ParseSession* m_session;
};

namespace QTest {
  template<>
  char* toString(const ComparableToken& t)
  {
    return qstrdup(
      QString("%1 [%2, %3]")
        .arg(t.toString())
        .arg(token_name(t.m_token.kind))
        .arg(t.m_token.position).toUtf8().constData());
  }
}

class TestGenerator : public QObject
{
  Q_OBJECT

  Control control;
  DumpTree dumper;

public:
  TestGenerator()
  {
  }

  enum ParseFlag {
    FlagNone = 0,
    DumpAST = 1,
    PrintCode = 2,
    FlagAll = 3
  };
  Q_DECLARE_FLAGS(ParseFlags, ParseFlag)

  void parse(const QByteArray& unit, ParseFlags flags = static_cast<ParseFlags>(FlagNone))
  {
    TranslationUnitAST* ast = parseOriginal(unit);
    if (flags & DumpAST)
      dumper.dump(ast, lastSession->token_stream);

    CodeGenerator cg(lastSession);
    cg.visit(ast);
    if (flags & PrintCode) {
      qDebug() << unit;
      qDebug() << cg.output();
    }

    parseGenerated( cg.output().toUtf8() );

    compareTokenStreams();
  }

private slots:
  void initTestCase()
  {
    KDevelop::AutoTestShell::init(QStringList() << "kdevcppsupport");
    KDevelop::TestCore* core = new KDevelop::TestCore();
    core->initialize(KDevelop::Core::NoUi);
  }

  void cleanupTestCase()
  {
    KDevelop::TestCore::shutdown();
  }

  void testIf()
  {
    parse(QByteArray("void test() { if (i == 0) { foo(); } else { foo2(); } }"));
  }

  void testFor()
  {
    QByteArray method("void test() { for (int i = 0; i < 4; ++i) { break; } for (j; j < 4; ) {return;} }");
    parse(method);
  }

  void testDo()
  {
    QByteArray method("void test() { do { foo(); } while (i < 0); }");
    parse(method);
  }

  void testWhile()
  {
    QByteArray method("void test() { while (i & 3) { foo(); } }");
    parse(method);
  }

  void testSwitch()
  {
    QByteArray method("void test() { switch (i) { case 1: break; case 2: return; default: goto foo; } foo: return; }");
    parse(method);
  }

  void testClass()
  {
    QByteArray method("struct A : public B, virtual private C { int i; A() : i(5) { } virtual void test() = 0; };");
    parse(method);
  }

  void testTemplateClass()
  {
    QByteArray method("template <typename B> struct A : private C { B i; A() : i(5) { } virtual void test() = 0; };");
    parse(method);
  }

  void testMethod()
  {
    QByteArray method("int A::test(int primitive, B* pointer) { return primitive; }");
    parse(method);
  }

  void testIntegralTypes()
  {
    parse(QByteArray("const unsigned int i, k; volatile long double j; int* l; double * const * m; const int& n = l;"));
  }

  void testArrayType()
  {
    parse(QByteArray("const unsigned int ArraySize = 3; int i[ArraySize];"));
  }

  void testEnum()
  {
    parse(QByteArray("enum Enum { Value1 = 5, value2 }; enum Enum2 { Value21, value22 = 2 }; union { int u1; float u2; };"));
  }

  void testPublicFlags()
  {
    parse(QByteArray("class Foo { public: virtual void bar(); private: void baz(); protected: };"));
  }

  void testDeclareStruct()
  {
    parse(QByteArray("struct { short i; } instance;"));
  }

  void testVariableDeclaration()
  {
    parse(QByteArray("int c; A instance(c); A instance(2, 3); A instance(q); bla() {int* i = new A(c); delete i; }"));
  }

  void testFriendDeclaration()
  {
    parse(QByteArray("class A { friend class F; }; "));
  }

  void testUsingDeclarationInTemplate()
  {
    parse(QByteArray("template<class T> class A { T i; }; template<class Q> struct B: private A<Q> { using A<Q>::i; };"));
  }

  void testDeclareUsingNamespace2()
  {
    parse(QByteArray("namespace foo2 {int bar2; namespace SubFoo { int subBar2; } } namespace foo { int bar; using namespace foo2; } namespace GFoo{ namespace renamedFoo2 = foo2; using namespace renamedFoo2; using namespace SubFoo; int gf; } using namespace GFoo; int test() { return bar; }"));
  }

  void testFunctionDefinition3()
  {
    parse(QByteArray("class B{template<class T> void test(T t); B(int i); int test(int a); int test(char a); template<class T2, class T3> void test(T2 t, T3 t3); int test(Unknown k); int test(Unknown2 k); }; template<class T> void B::test(T t) {} B::B(int) {} int B::test(int a){} int B::test(char a){} template<class T2, class T3> void B::test(T2 t, T3 t3) {} int B::test(Unknown k){} int B::test( Unknown2 k) {} "));
  }

  void testTemplateEnums()
  {
    parse(QByteArray("template<bool num> struct No {};  No<true> n;"));
    parse(QByteArray("template<int num=5> struct No {};  No n;"));
    parse(QByteArray("template<int num> struct No {};  No<9> n;"));
  }

  void testDynamicArray()
  {
    parse(QByteArray("struct Bla { int val; } blaArray[] = { {5} };"));
  }

  void testSmartPointer()
  {
    parse(QByteArray("template<class T> struct SmartPointer { T* operator ->() const {} template<class Target> SmartPointer<Target> cast() {} T& operator*() {}  } ; class B{int i;}; class C{}; SmartPointer<B> bPointer;"));
  }

  void testSimpleExpression()
  {
    parse(QByteArray("struct Cont { int& a; Cont* operator -> () {} double operator*(); }; Cont c; Cont* d = &c; void test() { c.a = 5; d->a = 5; (*d).a = 5; c.a(5, 1, c); c.b<Fulli>(); }"));
  }

  void testThis()
  {
    parse(QByteArray("struct Cont { operator int() {} }; void test( int c = 5 ) { this->test( Cont(), 1, 5.5, 6); }"));
  }

  void testCasts()
  {
    parse(QByteArray("struct Cont2 {}; struct Cont { int& a; Cont* operator -> () {} double operator*(); }; Cont c; Cont* d = &c; void test() { c.a = 5; d->a = 5; (*d).a = 5; c.a(5, 1, c); c(); c.a = dynamic_cast<const Cont2*>(d); }"));
  }

  void testOperators()
  {
    parse(QByteArray("struct Cont2 {int operator[]() {} operator()() {}}; struct Cont3{}; struct Cont { Cont3 operator[](int i) {} Cont3 operator()() {} Cont3 operator+(const Cont3& c3 ) {} }; Cont c; Cont2 operator+( const Cont& c, const Cont& c2){} Cont3 c3;"));
  }

  void testEmptyFor()
  {
    parse(QByteArray("void test() { for (;;) {} }"));
  }

private:
  TranslationUnitAST* parseOriginal(const QByteArray& unit)
  {
    Parser parser(&control);
    lastSession = new ParseSession();
    lastSession->setContentsAndGenerateLocationTable(tokenizeFromByteArray(unit));
    return  parser.parse(lastSession);
  }

  TranslationUnitAST* parseGenerated(const QByteArray& unit)
  {
    Parser parser(&control);
    lastGeneratedSession = new ParseSession();
    lastGeneratedSession->setContentsAndGenerateLocationTable(tokenizeFromByteArray(unit));
    return  parser.parse(lastGeneratedSession);
  }

  void compareTokenStreams()
  {
    int cursor = 1;
    forever {
      QVERIFY(cursor < lastSession->token_stream->size());
      QVERIFY(cursor < lastGeneratedSession->token_stream->size());

      const Token& t1 = lastSession->token_stream->token( cursor );
      const Token& t2 = lastGeneratedSession->token_stream->token( cursor );

      QCOMPARE(ComparableToken(t1, lastSession), ComparableToken(t2, lastGeneratedSession));
      if (t1.kind == Token_EOF)
        break;

      ++cursor;
    }
  }
};

#include "test_generator.moc"

QTEST_MAIN(TestGenerator)
