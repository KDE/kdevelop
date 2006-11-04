/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef CMAKEASTTEST_H
#define CMAKEASTTEST_H

#include <QtTest/QtTest>

class CMakeAstTests : public QObject
{
    Q_OBJECT
public:
    CMakeAstTests() {}
    virtual ~CMakeAstTests() {}

private slots:
    void testCustomCommandAstParsing();
    void testCustomTargetAstParsing();
    void testAddDefintionAstParsing();
    void testAddDependenciesAstParsing();
    void testAddExecutableAstParsing();
    void testAddLibraryAstParsing();
    void testAddSubdirectoryAstParsing();
    void testAddTestAstParsing();
    void testAuxSourceDirectoryAstParsing();
    void testBuildCommandAstParsing();
    void testBuildNameAstParsing();
    void testMinimumVersionAstParsing();
    void testConfigureFileAstParsing();
    void testCreateTestSourceListAstParsing();
    void testElseAstParsing();
    void testEnableLanguageAstParsing();
    void testEnableTestingAstParsing();
    void testEndForeachAstParsing();
    void testEndIfAstParsing();
    void testEndMacroAstParsing();
    void testEndWhileAstParsing();
    void testExecProgramAstParsing();
    void testExecuteProcessAstParsing();
    void testExportLibraryDepsAstParsing();
    void testFileAstParsing();
    void testFindFileAstParsing();
    void testFindLibraryAstParsing();
    void testFindPackageAstParsing();
    void testFindPathAstParsing();
    void testFindProgramAstParsing();
    void testFltkWrapUiAstParsing();
    void testForeachAstParsing();
    void testGetCmakePropertyAstParsing();
    void testGetDirPropertyAstParsing();
    void testGetFilenameComponentAstParsing();
    void testGetSourceFilePropAstParsing();
    void testGetTargetPropAstParsing();
    void testGetTestPropAstParsing();
    void testIfAstParsing();
    void testIncludeAstParsing();
    void testIncludeDirectoriesAstParsing();
    void testIncludeExtMSProjectAstParsing();
    void testIncludeRegexAstParsing();
    void testInstallAstParsing();
    void testInstallFilesAstParsing();
    void testInstallProgramsAstParsing();
    void testInstallTargetsAstParsing();
    void testLinkDirsAstParsing();
    void testLinkLibsAstParsing();
    void testListAstParsing();
    void testLoadCacheAstParsing();
    void testLoadCommandAstParsing();
    void testMacroAstParsing();
    void testMakeDirAstParsing();
    void testMarkAdvancedAstParsing();
    void testMathAstParsing();
    void testMessageAstParsing();
    void testOptionAstParsing();
    void testOutputReqFilesAstParsing();
    void testProjectAstParsing();
    void testQtWrapCppAstParsing();
    void testQtWrapUiAstParsing();
    void testRemoveAstParsing();
    void testRemoveDefinitionsAstParsing();
    void testSeparateArgsAstParsing();
    void testSetAstParsing();
    void testSetDirPropsAstParsing();
    void testSetSourceFilePropsAstParsing();
    void testSetTargetPropsAstParsing();
    void testSetTestsPropsAstParsing();
    void testSiteNameAstParsing();
    void testSourceGroupAstParsing();
    void testStringAstParsing();
    void testSubdirDependsAstParsing();
    void testSubdirsAstParsing();
    void testTargetLinkLibsAstParsing();
    void testTryCompileAstParsing();
    void testTryRunAstParsing();
    void testUseMangledMesaAstParsing();
    void testUtilitySourceAstParsing();
    void testVarRequiresAstParsing();
    void testVtkMakeAstParsing();
    void testVtkWrapJavaParsing();
    void testVtkWrapPythonParsing();
    void testVtkWrapTclParsing();
    void testWhileParsing();
    void testWriteFileParsing();

};

#endif
