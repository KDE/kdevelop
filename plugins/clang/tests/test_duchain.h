/*
    SPDX-FileCopyrightText: 2014 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef DUCHAINTEST_H
#define DUCHAINTEST_H

#include <QObject>

class TestEnvironmentProvider;

namespace KDevelop {
class TestProjectController;
}

class TestDUChain : public QObject
{
    Q_OBJECT
public:
    ~TestDUChain() override;

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void init();
    void cleanup();

    void testComments();
    void testComments_data();
    void testElaboratedType();
    void testElaboratedType_data();

    void testMacroDefinition();
    void testMacroDefinition_data();

    void testInclude();
    void testMissingInclude();
    void testIncludeLocking();
    void testReparse();
    void testReparseError();
    void testTemplate();
    void testNamespace();
    void testAutoTypeDeduction();
    void testTypeDeductionInTemplateInstantiation();
    void testVirtualMemberFunction();
    void testBaseClasses();
    void testReparseBaseClasses();
    void testReparseBaseClassesTemplates();
    void testGetInheriters();
    void testGetInheriters_data();
    void testGlobalFunctionDeclaration();
    void testFunctionDefinitionVsDeclaration();
    void testEnsureNoDoubleVisit();
    void testReparseWithAllDeclarationsContextsAndUses();
    void testReparseOnDocumentActivated();
    void testParsingEnvironment();
    void testSystemIncludes();
    void testReparseInclude();
    void testReparseChangeEnvironment();
    void testMacrosRanges();
    void testUseInMacroParameter();
    void testMacroUses();
    void testHeaderParsingOrder1();
    void testHeaderParsingOrder2();
    void testMacroDependentHeader();
    void testNestedImports();
    void testEnvironmentWithDifferentOrderOfElements();
    void testReparseMacro();
    void testMultiLineMacroRanges();
    void testNestedMacroRanges();
    void testGotoStatement();
    void testRangesOfOperatorsInsideMacro();
    void testActiveDocumentHasASTAttached();
    void testActiveDocumentsGetBestPriority();
    void testUsesCreatedForDeclarations();
    void testReparseIncludeGuard();
    void testIncludeGuardHeaderHeaderOnly();
    void testIncludeGuardHeaderHeaderOnly_data();
    void testIncludeGuardHeaderWarning();
    void testExternC();
    void testIncludeExternC();
    void testLambda();
    void testReparseUnchanged_data();
    void testReparseUnchanged();
    void testTypeAliasTemplate();
    void testDeclarationsInsideMacroExpansion();
    void testForwardTemplateTypeParameterContext();
    void testTemplateFunctionParameterName();
    void testFriendDeclaration();
    void testVariadicTemplateArguments();
    void testProblemRequestedHere();
    void testProblemRequestedHereSameFile();
    void testProblemRequestedHereChain();

    void testGccCompatibility_data();
    void testGccCompatibility();
    void testQtIntegration();
    void testHasInclude();

    void testSameFunctionDefinition();
    void testSizeAlignOf();
    void testSizeAlignOfUpdate();
    void testBitWidth();
    void testValueDependentBitWidth();
    void testBitWidthUpdate();

private:
    QScopedPointer<TestEnvironmentProvider> m_provider;
    KDevelop::TestProjectController* m_projectController;
};

#endif // DUCHAINTEST_H
