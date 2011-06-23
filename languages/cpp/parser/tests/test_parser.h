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

#ifndef TEST_PARSER_H
#define TEST_PARSER_H

#include <QObject>

#include "control.h"
#include "dumptree.h"

class pool;

class TestParser : public QObject {
  Q_OBJECT

public:
  TestParser() { }

private slots:
  void initTestCase();
  void testSymbolTable();
  void testTokenTable();
  void testParser();
  void testTemplateArguments();
  void testManyComparisons();
  void testParserFail();
  void testPartialParseFail();
  void testParseMethod();
  void testForStatements();
  void testIfStatements();

  void testComments();
  void testComments2();
  void testComments3();
  void testComments4();
  void testComments5();
  void testComments6();

  void testPreprocessor();
  void testPreprocessorStringify();
  void testStringConcatenation();
  void testEmptyInclude();

  void testCondition();
  void testNonTemplateDeclaration();
  void testInitListTrailingComma();
  void testAsmVolatile();
  void testIncrIdentifier();
  void testParseFile();

  void testQProperty_data();
  void testQProperty();

  void testCommentAfterFunctionCall();
  void testPtrToMemberAst();
  void testSwitchStatement();

  void testNamedOperators_data();
  void testNamedOperators();
  void testOperators_data();
  void testOperators();

  //BEGIN C++2011 support
  void testRangeBasedFor();
  void testRValueReference();
  void testDefaultDeletedFunctions_data();
  void testDefaultDeletedFunctions();
  void testVariadicTemplates_data();
  void testVariadicTemplates();
  void testStaticAssert_data();
  void testStaticAssert();
  void testConstExpr_data();
  void testConstExpr();
  void testEnumClass_data();
  void testEnumClass();
  void testRightAngleBrackets_data();
  void testRightAngleBrackets();

private:
  Control control;
  DumpTree dumper;

  ParseSession* lastSession;

  TranslationUnitAST* parse(const QByteArray& unit);
};

#endif // TEST_PARSER_H
