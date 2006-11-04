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
    void testCustomCommandAstParsing() { QFAIL( "no magic" ); }
    void testCustomTargetAstParsing() { QFAIL( "no magic" ); }
    void testAddDefintionAstParsing() { QFAIL( "no magic" ); }
    void testAddDependenciesAstParsing() { QFAIL( "no magic" ); }
    void testAddExecutableAstParsing() { QFAIL( "no magic" ); }
    void testAddLibraryAstParsing() { QFAIL( "no magic" ); }
    void testAddSubdirectoryAstParsing() { QFAIL( "no magic" ); }
    void testAddTestAstParsing() { QFAIL( "no magic" ); }
    void testAuxSourceDirectoryAstParsing() { QFAIL( "no magic" ); }
    void testBuildCommandAstParsing() { QFAIL( "no magic" ); }
    void testBuildNameAstParsing() { QFAIL( "no magic" ); }
    void testMinimumVersionAstParsing() { QFAIL( "no magic" ); }
    void testConfigureFileAstParsing() { QFAIL( "no magic" ); }
    void testCreateTestSourceListAstParsing() { QFAIL( "no magic" ); }
    void testElseAstParsing() { QFAIL( "no magic" ); }
    void testEnableLanguageAstParsing() { QFAIL( "no magic" ); }
    void testEnableTestingAstParsing() { QFAIL( "no magic" ); }
    void testEndForeachAstParsing() { QFAIL( "no magic" ); }
    void testEndIfAstParsing() { QFAIL( "no magic" ); }
    void testEndMacroAstParsing() { QFAIL( "no magic" ); }
    void testEndWhileAstParsing() { QFAIL( "no magic" ); }
    void testExecProgramAstParsing() { QFAIL( "no magic" ); }
    void testExecuteProcessAstParsing() { QFAIL( "no magic" ); }
    void testExportLibraryDepsAstParsing() { QFAIL( "no magic" ); }
    void testFileAstParsing() { QFAIL( "no magic" ); }
    void testFindFileAstParsing() { QFAIL( "no magic" ); }
    void testFindLibraryAstParsing() { QFAIL( "no magic" ); }
    void testFindPackageAstParsing() { QFAIL( "no magic" ); }
    void testFindPathAstParsing() { QFAIL( "no magic" ); }
    void testFindProgramAstParsing() { QFAIL( "no magic" ); }
    void testFltkWrapUiAstParsing() { QFAIL( "no magic" ); }
    void testForeachAstParsing() { QFAIL( "no magic" ); }
    void testGetCmakePropertyAstParsing() { QFAIL( "no magic" ); }
    void testGetDirPropertyAstParsing() { QFAIL( "no magic" ); }
    void testGetFilenameComponentAstParsing() { QFAIL( "no magic" ); }
    void testGetSourceFilePropAstParsing() { QFAIL( "no magic" ); }
    void testGetTargetPropAstParsing() { QFAIL( "no magic" ); }
    void testGetTestPropAstParsing() { QFAIL( "no magic" ); }
    void testIfAstParsing() { QFAIL( "no magic" ); }
    void testIncludeAstParsing() { QFAIL( "no magic" ); }
    void testIncludeDirectoriesAstParsing() { QFAIL( "no magic" ); }
    void testIncludeExtMSProjectAstParsing() { QFAIL( "no magic" ); }
    void testIncludeRegexAstParsing() { QFAIL( "no magic" ); }
    void testInstallAstParsing() { QFAIL( "no magic" ); }
    void testInstallFilesAstParsing() { QFAIL( "no magic" ); }
    void testInstallProgramsAstParsing() { QFAIL( "no magic" ); }
    void testInstallTargetsAstParsing() { QFAIL( "no magic" ); }
    void testLinkDirsAstParsing() { QFAIL( "no magic" ); }
    void testLinkLibsAstParsing() { QFAIL( "no magic" ); }
    void testListAstParsing() { QFAIL( "no magic" ); }
    void testLoadCacheAstParsing() { QFAIL( "no magic" ); }
    void testLoadCommandAstParsing() { QFAIL( "no magic" ); }
    void testMacroAstParsing() { QFAIL( "no magic" ); }
    void testMakeDirAstParsing() { QFAIL( "no magic" ); }
    void testMarkAdvancedAstParsing() { QFAIL( "no magic" ); }
    void testMathAstParsing() { QFAIL( "no magic" ); }
    void testMessageAstParsing() { QFAIL( "no magic" ); }
    void testOptionAstParsing() { QFAIL( "no magic" ); }
    void testOutputReqFilesAstParsing() { QFAIL( "no magic" ); }
    void testProjectAstParsing() { QFAIL( "no magic" ); }
    void testQtWrapCppAstParsing() { QFAIL( "no magic" ); }
    void testQtWrapUiAstParsing() { QFAIL( "no magic" ); }
    void testRemoveAstParsing() { QFAIL( "no magic" ); }
    void testRemoveDefinitionsAstParsing() { QFAIL( "no magic" ); }
    void testSeparateArgsAstParsing() { QFAIL( "no magic" ); }
    void testSetAstParsing() { QFAIL( "no magic" ); }
    void testSetDirPropsAstParsing() { QFAIL( "no magic" ); }
    void testSetSourceFilePropsAstParsing() { QFAIL( "no magic" ); }
    void testSetTargetPropsAstParsing() { QFAIL( "no magic" ); }
    void testSetTestsPropsAstParsing() { QFAIL( "no magic" ); }
    void testSiteNameAstParsing() { QFAIL( "no magic" ); }
    void testSourceGroupAstParsing() { QFAIL( "no magic" ); }
    void testStringAstParsing() { QFAIL( "no magic" ); }
    void testSubdirDependsAstParsing() { QFAIL( "no magic" ); }
    void testSubdirsAstParsing() { QFAIL( "no magic" ); }
    void testTargetLinkLibsAstParsing() { QFAIL( "no magic" ); }
    void testTryCompileAstParsing() { QFAIL( "no magic" ); }
    void testTryRunAstParsing() { QFAIL( "no magic" ); }
    void testUseMangledMesaAstParsing() { QFAIL( "no magic" ); }
    void testUtilitySourceAstParsing() { QFAIL( "no magic" ); }
    void testVarRequiresAstParsing() { QFAIL( "no magic" ); }
    void testVtkMakeAstParsing() { QFAIL( "no magic" ); }
    void testVtkWrapJavaParsing() { QFAIL( "no magic" ); }
    void testVtkWrapPythonParsing() { QFAIL( "no magic" ); }
    void testVtkWrapTclParsing() { QFAIL( "no magic" ); }
    void testWhileParsing() { QFAIL( "no magic" ); }
    void testWriteFileParsing() { QFAIL( "no magic" ); }

};

#endif
