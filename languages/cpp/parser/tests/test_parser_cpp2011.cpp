/* This file is part of KDevelop

   Copyright 2010 Milian Wolff <mail@milianw.de>

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

#include <QTest>

#include "parsesession.h"

void TestParser::testRangeBasedFor()
{
  QByteArray code("int main() {\n"
                  "  int array[5] = { 1, 2, 3, 4, 5 };\n"
                  "  for (int& x : array) {\n"
                  "    x *= 2;\n"
                  "  }\n"
                  " }\n");
  TranslationUnitAST* ast = parse(code);

  QVERIFY(ast);
  QVERIFY(ast->declarations);
  QVERIFY(control.problems().isEmpty());
}

void TestParser::testRValueReference()
{
  QByteArray code("int&& a = 1;");

  TranslationUnitAST* ast = parse(code);
  QVERIFY(control.problems().isEmpty());

  QVERIFY(ast);
  QVERIFY(ast->declarations);
}

void TestParser::testDefaultDeletedFunctions()
{
  QByteArray code("class A{\n"
                  "  A() = default;\n"
                  "  A(const A&) = delete;\n"
                  "};\n"
                  "bool operator==(const A&, const A&) = default;\n"
                  "bool operator!=(const A&, const A&) = delete;\n");
  TranslationUnitAST* ast = parse(code);
  QVERIFY(control.problems().isEmpty());

  QVERIFY(ast);
  QVERIFY(ast->declarations);
}

void TestParser::testVariadicTemplates_data()
{
  QTest::addColumn<QString>("code");

  QTest::newRow("template-pack-class") << "template<class ... Arg> class A {};\n";
  QTest::newRow("template-pack-typename") << "template<typename ... Arg> class A {};\n";
  QTest::newRow("pack-expansion-baseclasses") << "template<class ... Arg> class A : public Arg... {};\n";
  QTest::newRow("pack-expansion-baseclass") << "template<class ... Arg> class A : public B<Arg...> {};\n";
  QTest::newRow("pack-expansion-tplarg") << "template<class ... Arg> class A { A() { A<Arg...>(); } };\n";
  QTest::newRow("pack-expansion-params") << "template<typename ... Arg> void A(Arg ... params) {};\n";
  QTest::newRow("pack-expansion-params-call") << "template<typename ... Arg> void A(Arg ... params) { A(params...); };\n";
  QTest::newRow("pack-expansion-mem-initlist") << "template<class ... Mixins> class A : public Mixins... { A(Mixins... args) : Mixins(args)... {} };\n";
  QTest::newRow("pack-expansion-mem-initlist-arg") << "template<class ... Args> class A : public B<Args...> { A(Args... args) : B<Args...>(args...) {} };\n";
  QTest::newRow("pack-expansion-initlist") << "template<typename ... Arg> void A(Arg ... params) { SomeList list = { params... }; };\n";
  QTest::newRow("pack-expansion-initlist2") << "template<typename ... Arg> void A(Arg ... params) { int a[] = { params... }; };\n";
  QTest::newRow("pack-expansion-initlist3") << "template<typename ... Arg> void A(Arg ... params) { int a[] = { (params+10)... }; };\n";
  QTest::newRow("pack-expansion-throw") << "template<typename ... Arg> void A() throw(Arg...) {};\n";
  ///TODO: attribute-list?
  ///TODO: alignment-specifier?
  ///TODO: capture-list?
}

void TestParser::testVariadicTemplates()
{
  QFETCH(QString, code);
  TranslationUnitAST* ast = parse(code.toUtf8());
  dumper.dump(ast, lastSession->token_stream);
  QEXPECT_FAIL("pack-expansion-initlist", "not implemented", Abort);
  QEXPECT_FAIL("pack-expansion-initlist2", "not implemented", Abort);
  QEXPECT_FAIL("pack-expansion-initlist3", "not implemented", Abort);
  QEXPECT_FAIL("pack-expansion-throw", "not implemented", Abort);
  if (!control.problems().isEmpty()) {
    foreach(const KDevelop::ProblemPointer&p, control.problems()) {
      qDebug() << p->description() << p->explanation() << p->finalLocation().textRange();
    }
  }
  QVERIFY(control.problems().isEmpty());

  QVERIFY(ast);
  QVERIFY(ast->declarations);
}
