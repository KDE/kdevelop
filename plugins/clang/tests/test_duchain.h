/*
 * Copyright 2014  Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
    void testExternC();
    void testLambda();
    void testReparseUnchanged_data();
    void testReparseUnchanged();
    void testTypeAliasTemplate();
    void testDeclarationsInsideMacroExpansion();
    void testForwardTemplateTypeParameterContext();
    void testTemplateFunctionParameterName();
    void testFriendDeclaration();
    void testVariadicTemplateArguments();

    void benchDUChainBuilder();
    void testGccCompatibility();
    void testQtIntegration();
    void testHasInclude();

private:
    QScopedPointer<TestEnvironmentProvider> m_provider;
    KDevelop::TestProjectController* m_projectController;
};

#endif // DUCHAINTEST_H
