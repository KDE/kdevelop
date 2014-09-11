/* This file is part of KDevelop
    Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef TEST_EXPRESSIONPARSER_H
#define TEST_EXPRESSIONPARSER_H

#include <QObject>
#include <QByteArray>

#include <QUrl>

#include "parser.h"
#include "control.h"
#include "dumpchain.h"

#include <language/duchain/identifier.h>
#include <language/duchain/types/abstracttype.h>
#include <language/duchain/types/indexedtype.h>

namespace KDevelop
{
class Declaration;
class TopDUContext;
}

class TestExpressionParser : public QObject
{
  Q_OBJECT

public:
  TestExpressionParser();

  KDevelop::Declaration* findDeclaration(KDevelop::DUContext* context, const KDevelop::Identifier& id, const KDevelop::CursorInRevision& position = KDevelop::CursorInRevision::invalid());
  KDevelop::Declaration* findDeclaration(KDevelop::DUContext* context, const KDevelop::QualifiedIdentifier& id, const KDevelop::CursorInRevision& position = KDevelop::CursorInRevision::invalid());

private slots:
  void initTestCase();
  void testTemplateSpecialization();
  void testIntegralType();
  void testSimpleExpression();
  void testAutoTemplate();
  void testTypeConversion();
  void testTypeConversion2();
  void testTypeConversionWithTypedefs();
  void testSmartPointer();
  void testCasts();
  void testEnum();
  void testBaseClasses();
  void testTemplatesSimple();
  void testTemplates();
  void testTemplates2();
  void cleanupTestCase();
  void testOperators();
  void testTemplateFunctions();
  void testThis();
  void testArray();
  void testDynamicArray();
  void testTypeID();
  void testConstness();
  void testConstnessOverload();
  void testConstnessOverloadSubscript();
  void testReference();

  void testCharacterTypes_data();
  void testCharacterTypes();

  void benchEvaluateType();
  void benchEvaluateType_data();

public:
  enum DumpArea {
    DumpNone = 0,
    DumpAST = 1,
    DumpDUChain = 2,
    DumpType = 4,
    DumpAll = 7
  };
  Q_DECLARE_FLAGS(DumpAreas, DumpArea)

private:
  KDevelop::TopDUContext* parse(const QByteArray& unit, DumpAreas dump = static_cast<DumpAreas>(DumpAST | DumpDUChain | DumpType));

  void release(KDevelop::DUContext* top);

  // Parser
  Control control;
  Cpp::DumpChain cppDumper;

  KDevelop::AbstractType::Ptr typeVoid;
  KDevelop::AbstractType::Ptr typeInt;

  bool testFileParseOnly;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TestExpressionParser::DumpAreas)

#endif
