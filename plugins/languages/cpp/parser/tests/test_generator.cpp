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

bool operator==(const Token& t1, const Token& t2)
{
  return t1.kind == t2.kind && t1.symbolString() == t2.symbolString();
}

namespace QTest {
  template<>
  char* toString(const Token& t)
  {
    return qstrdup(QString("%1 [ %2 ]").arg(token_name(t.kind)).arg(t.symbolString()).toUtf8());
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
      kDebug() << unit;
      kDebug() << cg.output();
    }

    parseGenerated( cg.output().toUtf8() );

    compareTokenStreams();
  }

private slots:
  void initTestCase()
  {
  }

  void testIf()
  {
    QByteArray method("void test() { if (i == 0) { foo(); } else { foo2(); } }");
    parse(method);
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

private:
  ParseSession* lastSession;
  ParseSession* lastGeneratedSession;

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
    std::size_t cursor = 1;
    forever {
      QVERIFY(cursor < lastSession->token_stream->size());
      QVERIFY(cursor < lastGeneratedSession->token_stream->size());

      const Token& t1 = lastSession->token_stream->token( cursor );
      const Token& t2 = lastGeneratedSession->token_stream->token( cursor );

      QCOMPARE(t1, t2);
      if (t1.kind == Token_EOF)
        break;

      ++cursor;
    }
  }
};

#include "test_generator.moc"

QTEST_MAIN(TestGenerator)
