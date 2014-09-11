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

#ifndef TEST_DUCHAIN_H
#define TEST_DUCHAIN_H

#include <QObject>
#include <QByteArray>

#include <QUrl>

#include <language/duchain/identifier.h>
#include <language/duchain/types/indexedtype.h>
#include <language/editor/cursorinrevision.h>

#include "test_helper.h"

namespace KDevelop
{
class Declaration;
class TopDUContext;
class ClassFunctionDeclaration;
}

class TestDUChain : public QObject, public Cpp::TestHelper
{
  Q_OBJECT

public:
  TestDUChain();

  KDevelop::Declaration* findDeclaration(KDevelop::DUContext* context, const KDevelop::Identifier& id, const KDevelop::CursorInRevision& position = KDevelop::CursorInRevision::invalid());
  KDevelop::Declaration* findDeclaration(KDevelop::DUContext* context, const KDevelop::QualifiedIdentifier& id, const KDevelop::CursorInRevision& position = KDevelop::CursorInRevision::invalid());


  //Move to slots again once it should be tested
  void testFileParse();

private slots:
  void initTestCase();
  void cleanupTestCase();
  void testTypeof();
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
  void testGlobalNamespaceAlias();
  void testGlobalNamespaceAliasCycle();
  void testLocalNamespaceAlias();
  void testDeclareUsingNamespace();
  void testUsingDeclaration();
  void testUsingDeclarationInTemplate();
  void testDeclareUsingNamespace2();
  void testUsingGlobalNamespaceAlias();
  void testUsingGlobalNamespaceAliasCycle();
  void testSearchAcrossNamespace();
  void testSearchAcrossNamespace2();
  void testSearchAcrossNamespace3();
  void testADL();
  void testADLConstness();
  void testADLConstness2();
  void testADLClassType();
  void testADLFunctionType();
  void testADLFunctionByName();
  void testADLEnumerationType();
  void testADLClassMembers();
  void testADLMemberFunction();
  void testADLMemberFunctionByName();
  void testADLOperators();
  void testADLNameAlias();
  void testADLTemplateArguments();
  void testADLTemplateTemplateArguments();
  void testADLEllipsis();
  void testAssignmentOperators();
  void testTemplateEnums();
  void testIntegralTemplates();
  void testTypedef();
  void testTypedefUnsignedInt();
  void testTypedefFuncptr();
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
  void testSpecializationSelection();
  void testSpecializationSelection2();
  void testTemplatesRebind();
  void testTemplatesRebind2();
  void testTemplateDefaultParameters();
  void testTemplateParameters();
  void testFunctionTemplates();
  void testContextAssignment();
  void testSpecializedTemplates();
  void testTemplatesSuper();
  void testFunctionDefinition();
  void testFunctionDefinition2();
  void testFunctionDefinition3();
  void testFunctionDefinition4();
  void testFunctionDefinition5();
  void testFunctionDefinition6();
  void testFunctionDefinition7();
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
  void testAssignedContexts();
  void testTryCatch();
  void testEnum();
  void testCaseUse();
  void testConstructorOperatorUses();
  void testSizeofUse();
  void testDefinitionUse();
  void testOperatorUses();
  void testDeclareSubClass();
  void testDeclarationId();
  void testConst();
  void testEnumOverride();
  void testDoWhile();
  void testCodeModel();
  void testLoopNamespaceImport();
  void testConstructorUses();
  void testExternalMemberDeclaration();

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

  void testEllipsis();
  void testEllipsisVexing();

  void testMultiByteCStrings();

  void testUses();
  void testCtorTypes();

  void testAutoTypeIntegral_data();
  void testAutoTypeIntegral();
  void testAutoTypes();

  void testCommentAfterFunctionCall();
  void testPointerToMember();
  void testMemberPtrCrash();
  void testNestedNamespace();
  
  void testDeclarationHasUses();
  void testBug269352();
  void testRenameClass();

  void testQProperty();

  //BEGIN C++2011
  void testRangeBasedFor();
  void testRangeBasedForClass();
  void testRangeBasedForClass2();
  void testRValueReference();
  void testDefaultDelete();
  void testDelete_Bug278781();
  void testEnum2011_data();
  void testEnum2011();
  void testDecltype();
  void testDecltypeTypedef();
  void testDecltypeUses();
  void testTrailingReturnType();
  void testConstexpr();
  void testInitListRegressions();
  void testBug284536();
  void testBug285004();
  void testLambda();
  void testLambdaReturn();
  void testLambdaCapture();
  void testTemplateSpecializeArray();
  void testTemplateSpecializeRValue();
  void testTemplateSpecializeVolatile();
  void testAliasDeclaration();
  void testAuto();
  void testNoexcept();
  void testInlineNamespace();
  //END C++2011

private:
  void assertNoMemberFunctionModifiers(KDevelop::ClassFunctionDeclaration* memberFun);

public:

private:
  // Declaration - use chain
  KDevelop::Declaration* noDef;
  KDevelop::IndexedString file;
  KDevelop::TopDUContext* topContext;

  KDevelop::IndexedType typeVoid;
  KDevelop::IndexedType typeInt;
  KDevelop::IndexedType typeShort;

  bool testFileParseOnly;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TestDUChain::DumpAreas)

#endif
