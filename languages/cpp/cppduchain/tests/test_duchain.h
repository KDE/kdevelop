/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>
    Copyright 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef TESTDUCHAIN_H
#define TESTDUCHAIN_H

#include <QObject>
#include <QByteArray>

#include <kurl.h>

#include <ktexteditor/cursor.h>

#include "parser.h"
#include "control.h"
#include "dumpchain.h"

#include <language/duchain/identifier.h>
#include <language/duchain/dumpchain.h>
#include <language/duchain/types/indexedtype.h>
#include <language/editor/simplecursor.h>

namespace KDevelop
{
class Declaration;
class TopDUContext;
class SimpleCursor;
class ClassFunctionDeclaration;
}

class TestDUChain : public QObject
{
  Q_OBJECT

public:
  TestDUChain();

  KDevelop::Declaration* findDeclaration(KDevelop::DUContext* context, const KDevelop::Identifier& id, const KDevelop::SimpleCursor& position = KDevelop::SimpleCursor::invalid());
  KDevelop::Declaration* findDeclaration(KDevelop::DUContext* context, const KDevelop::QualifiedIdentifier& id, const KDevelop::SimpleCursor& position = KDevelop::SimpleCursor::invalid());


  //Move to slots again once it should be tested
  void testFileParse();

private slots:
  void initTestCase();
  void cleanupTestCase();
  void testTypeof();
  void testIdentifiers();
  void testContextRelationships();
  void testDeclareInt();
  void testIntegralTypes();
  void testConversionReturn();
  void testArrayType();
  void testDeclareFor();
  void testVariableDeclaration();
  void testDeclareStruct();
  void testDeclareStructInNamespace();
  void testDeclareClass();
  void testDeclareFriend();
  void testDeclareNamespace();
  void testDeclareNamespace2();
  void testLocalNamespaceAlias();
  void testDeclareUsingNamespace();
  void testUsingDeclaration();
  void testUsingDeclarationInTemplate();
  void testDeclareUsingNamespace2();
  void testSearchAcrossNamespace();
  void testSearchAcrossNamespace2();
  void testSearchAcrossNamespace3();
  void testTemplateEnums();
  void testIntegralTemplates();
  void testTypedef();
  void testTypedefUnsignedInt();
  void testTemplateDependentClass();
  void testTemplateFunctions();
  void testTypedefUses();
  void testTemplateReference();
  void testSimplifiedTypeString();
  void testSourceCodeInsertion();
  void testTemplates();
  void testTemplates2();
  void testTemplates3();
  void testTemplates4();
  void testTemplatesRebind();
  void testTemplatesRebind2();
  void testTemplateDefaultParameters();
  void testTemplateParameters();
  void testFunctionTemplates();
  void testContextAssignment();
  void testSpecializedTemplates();
  void testFunctionDefinition();
  void testFunctionDefinition2();
  void testFunctionDefinition3();
  void testFunctionDefinition4();
  void testFunctionDefinition5();
  void testFunctionDefinition6();
  void testMetaProgramming();
  void testMetaProgramming2();
  void testMetaProgramming3();
  void testSignalSlotDeclaration();
  void testSignalSlotUse();
  void testBaseClasses();
  void testForwardDeclaration();
  void testForwardDeclaration2();
  void testForwardDeclaration3();
  void testForwardDeclaration4();
  void testTemplateForwardDeclaration();
  void testTemplateForwardDeclaration2();
  void testTemplateRecursiveInstantiation();
  void testTemplateInternalSearch();
  void testTemplateImplicitInstantiations();
  void testStringSets();
  void testAssignedContexts();
  void testTryCatch();
  void testIndexedStrings();
  void testEnum();
  void testCaseUse();
  void testConstructorOperatorUses();
  void testSizeofUse();
  void testDefinitionUse();
  void testOperatorUses();
  void testImportStructure();
  void testImportCache();
  void testDeclareSubClass();
  void testDeclarationId();
  void testConst();
  void testEnumOverride();
  void testDoWhile();
  void testCodeModel();
  void testLoopNamespaceImport();
  void testConstructorUses();

  void testSymbolTableValid();
  
  void testSeparateVariableDefinition();
  
  void testBaseUses();
  void testProblematicUses();

  void testCStruct();
  void testCStruct2();

  void testVirtualMemberFunction();
  void testNonVirtualMemberFunction();
  void testMultipleVirtual();
  void testMixedVirtualNormal();

  void testMemberFunctionModifiers();

  void testContextSearch();
  
  void testFinalCleanup();
  void testEllipsis();

  void testMultiByteCStrings();

  void testUses();

private:
  void assertNoMemberFunctionModifiers(KDevelop::ClassFunctionDeclaration* memberFun);

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
  KDevelop::TopDUContext* parse(const QByteArray& unit, DumpAreas dump = static_cast<DumpAreas>(DumpAST | DumpDUChain | DumpType),
                                 KDevelop::TopDUContext* update = 0, bool keepAst = false);

  // Parser
  Control control;
  Cpp::DumpChain cppDumper;
  KDevelop::DumpChain dumper;

  // Declaration - use chain
  KDevelop::Declaration* noDef;
  KUrl file1, file2;
  KDevelop::TopDUContext* topContext;

  KDevelop::IndexedType typeVoid;
  KDevelop::IndexedType typeInt;
  KDevelop::IndexedType typeShort;

  bool testFileParseOnly;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TestDUChain::DumpAreas)

#endif
