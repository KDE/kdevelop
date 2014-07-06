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

#ifndef TestCppCodeCompletion_H
#define TestCppCodeCompletion_H

#include <QObject>
#include <QByteArray>
#include <QMap>

#include <kurl.h>

#include <language/editor/cursorinrevision.h>
#include <language/duchain/identifier.h>

#include "contextbuilder.h"
#include "parser.h"
#include "control.h"

#include "dumpchain.h"
#include "rpp/chartools.h"

namespace KDevelop
{
class Declaration;
class TopDUContext;
class DUContext;
}

namespace rpp {
  class pp;
  class LocationTable;
}

using namespace KDevelop;

class TestPreprocessor;

class TestCppCodeCompletion : public QObject
{
  Q_OBJECT

public:
  TestCppCodeCompletion();

  KDevelop::Declaration* findDeclaration(KDevelop::DUContext* context, const KDevelop::Identifier& id, const KDevelop::CursorInRevision& position = KDevelop::CursorInRevision::invalid());
  KDevelop::Declaration* findDeclaration(KDevelop::DUContext* context, const KDevelop::QualifiedIdentifier& id, const KDevelop::CursorInRevision& position = KDevelop::CursorInRevision::invalid());

private slots:
  void initTestCase();
  void testCommentClearing();
  void testExpressionBefore();
  void testSpecialItems();
  void testOnlyShow();
  void testFriends();
  void testInvalidContexts();
  void testMemberAccess();
  void testParentContexts();
  void testCaseContext();
  void testCaseContextComplexExpression();
  void testCaseContextDifferentScope();
  void testCaseContextConstants();
  void testUnaryOperators();
  void testBinaryOperators();
  void testDeclarationIsInitialization();
  void testNoMemberAccess();
  void testFunctionImplementation();
  void testAliasDeclarationAccessPolicy();
  void testKeywords();
  void testInclude();
  void testTypeConversion();
  void testTypeConversion2();
  void testCompletionContext();
  void testPrivateVariableCompletion();
  void testUnnamedNamespace();
  void testIndirectImports();
  void testSameNamespace();
  void testUpdateChain();
  void testHeaderSections();
  void testCompletionPrefix();
  void cleanupTestCase();
  void testForwardDeclaration();
  void testArgumentMatching();
  void testAcrossHeaderReferences();
  void testMacroIncludeDirectives();
  void testAcrossHeaderTemplateReferences();
  void testAcrossHeaderTemplateResolution();
  void testHeaderGuards();
  void testEnvironmentMatching();
  void testUsesThroughMacros();
  void testMacroExpansionRanges();
  void testEmptyMacroArguments();
  void testMacrosInCodeCompletion();
  void testTimeMacro();
  void testDateMacro();
  void testFileMacro();
  void testPreprocessor();
  void testNaiveMatching();
  void testFriendVisibility();
  void testLocalUsingNamespace();
  void testInheritanceVisibility();
  void testConstVisibility();
  void testConstOverloadVisibility();
  void testNamespaceCompletion();
  void testNamespaceAliasCompletion();
  void testNamespaceAliasCycleCompletion();
  void testAfterNamespace();
  void testTemplateMemberAccess();
  void testTemplateArguments();
  void testTemplateFunction();
  void testImportTypedef();
  void testConstructorCompletion();
  void testConstructorUsageCompletion_data();
  void testConstructorUsageCompletion();
  void testAssistant();
  void testCompletionInExternalClassDefinition();
  void testCompletionBehindTypedeffedConstructor();
  void testSubClassVisibility();
  void testSignalSlotCompletion();
  void testSignalSlotExecution();
  void testStringProblem();
  void testArgumentList();
  void testStaticMethods();
  void testStringInComment_data();
  void testStringInComment();
  void testProperties();
  void testAnonStruct();
  void testOverrideCtor();
  void testFilterVoid();
  void testCompletedIncludeFilePath();
  void testMultipleIncludeCompletionItems();
  void testParentConstructor_data();
  void testParentConstructor();
  void testOverride_data();
  void testOverride();
  void testOverrideDeleted();
  void testExecuteKeepWord_data();
  void testExecuteKeepWord();
  void testAfterVisibility_data();
  void testAfterVisibility();
  void testNoQuadrupleColon();
  void testLookaheadMatches_data();
  void testLookaheadMatches();
  void testMemberAccessInstance();
  void testNestedInlineNamespace();
  void testDuplicatedNamespace();
public:
  enum DumpArea {
    DumpNone = 0,
    DumpAST = 1,
    DumpDUChain = 2,
    DumpType = 4,
    DumpAll = 1+2+4
  };
  Q_DECLARE_FLAGS(DumpAreas, DumpArea)

private:
  friend class TestPreprocessor;

  //Preprocesses the text, and parses all included strings within the correct context. Only strings that were added using addInclude(..) can be parsed. The url is only neede for the EnvironmentFile.
  QString preprocess( const IndexedString& url, const QString& text, IncludeFileList& included, rpp::pp* parent = 0, bool stopAfterHeaders = false, QExplicitlySharedDataPointer<Cpp::EnvironmentFile>* = 0, rpp::LocationTable** returnLocationTable = 0L, PreprocessedContents* contents = 0L );

  KDevelop::TopDUContext* parse(const QByteArray& unit, DumpAreas dump = static_cast<DumpAreas>(DumpAST | DumpDUChain | DumpType), rpp::pp* parent = 0, KUrl identity = QUrl(), TopDUContext* update = 0);

  void release(KDevelop::DUContext* top);

  //Add the text for a fake include-file
  void addInclude( const QString& identity, const QString& text );

  // Parser
  Control control;
  Cpp::DumpChain cppDumper;

  KDevelop::AbstractType::Ptr typeVoid;
  KDevelop::AbstractType::Ptr typeInt;

  QMap<QString, QString> fakeIncludes;

  bool testFileParseOnly;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TestCppCodeCompletion::DumpAreas)

#endif
