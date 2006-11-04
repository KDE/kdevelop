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

#include "cmakeasttest.h"
#include "cmakeast.h"
#include "cmakelistsparser.h"

QTEST_MAIN( CMakeAstTests )

void CMakeAstTests::testCustomCommandAstParsing()
{
//    QFETCH( CMakeFunctionDesc, func );

}

void CMakeAstTests::testCustomCommandAstParsing_data()
{
    QTest::addColumn<CMakeFunctionDesc>( "function" );
    CMakeFunctionDesc func1;
    func.name = "add_custom_command";
    QStringList argList;
    argList << "output" << "foo" << "command" << "bar";
    func.arguments = argList;
    func.filePath = QString();
    func.line = 0;

    CMakeFunctionDesc func1;
    func1.name = "add_custom_command";
    QStringList argList;
    argList << "output" << "foo" << "command" << "bar";
    argList << "main_dependency" << "dep1" << "depends" << "dep1" << "dep2";
    argList << "working_directory" << "dir1" << "comment" << "some comment";
    argList << "verbatim" << "append";
    func1.arguments = argList;

    CMakeFunctionDesc func2;
    func2.name = "ADD_CUSTOM_COMMAND";
    QStringList argList;
    argList << "OUTPUT" << "foo" << "COMMAND" << "bar";
    argList << "MAIN_DEPENDENCY" << "dep1" << "DEPENDS" << "dep1" << "dep2";
    argList << "WORKING_DIRECTORY" << "dir1" << "COMMENT" << "some comment";
    argList << "VERBATIM" << "APPEND";
    func2.arguments = argList;

    QTest::newRow( "no optional" ) << func;
    QTest::newRow( "all optional" ) << func1;
    QTest::newRow( "all optional uppercase" ) << func2;

}


void CMakeAstTests::testCustomTargetAstParsing()
{
}

void CMakeAstTests::testAddDefintionAstParsing()
{
}

void CMakeAstTests::testAddDependenciesAstParsing()
{
}

void CMakeAstTests::testAddExecutableAstParsing()
{
}

void CMakeAstTests::testAddLibraryAstParsing()
{
}

void CMakeAstTests::testAddSubdirectoryAstParsing()
{
}

void CMakeAstTests::testAddTestAstParsing()
{
}

void CMakeAstTests::testAuxSourceDirectoryAstParsing()
{
}

void CMakeAstTests::testBuildCommandAstParsing()
{
}

void CMakeAstTests::testBuildNameAstParsing()
{
}

void CMakeAstTests::testMinimumVersionAstParsing()
{
}

void CMakeAstTests::testConfigureFileAstParsing()
{
}

void CMakeAstTests::testCreateTestSourceListAstParsing()
{
}

void CMakeAstTests::testElseAstParsing()
{
}

void CMakeAstTests::testEnableLanguageAstParsing()
{
}

void CMakeAstTests::testEnableTestingAstParsing()
{

}

void CMakeAstTests::testEndForeachAstParsing()
{

}

void CMakeAstTests::testEndIfAstParsing()
{

}

void CMakeAstTests::testEndMacroAstParsing()
{

}

void CMakeAstTests::testEndWhileAstParsing()
{

}

void CMakeAstTests::testExecProgramAstParsing()
{

}

void CMakeAstTests::testExecuteProcessAstParsing()
{

}

void CMakeAstTests::testExportLibraryDepsAstParsing()
{

}

void CMakeAstTests::testFileAstParsing()
{

}

void CMakeAstTests::testFindFileAstParsing()
{

}

void CMakeAstTests::testFindLibraryAstParsing()
{

}

void CMakeAstTests::testFindPackageAstParsing()
{

}

void CMakeAstTests::testFindPathAstParsing()
{

}

void CMakeAstTests::testFindProgramAstParsing()
{

}

void CMakeAstTests::testFltkWrapUiAstParsing()
{

}

void CMakeAstTests::testForeachAstParsing()
{

}

void CMakeAstTests::testGetCmakePropertyAstParsing()
{

}

void CMakeAstTests::testGetDirPropertyAstParsing()
{

}

void CMakeAstTests::testGetFilenameComponentAstParsing()
{

}

void CMakeAstTests::testGetSourceFilePropAstParsing()
{

}

void CMakeAstTests::testGetTargetPropAstParsing()
{

}

void CMakeAstTests::testGetTestPropAstParsing()
{

}

void CMakeAstTests::testIfAstParsing()
{

}

void CMakeAstTests::testIncludeAstParsing()
{

}

void CMakeAstTests::testIncludeDirectoriesAstParsing()
{

}

void CMakeAstTests::testIncludeExtMSProjectAstParsing()
{

}

void CMakeAstTests::testIncludeRegexAstParsing()
{

}

void CMakeAstTests::testInstallAstParsing()
{

}

void CMakeAstTests::testInstallFilesAstParsing()
{

}

void CMakeAstTests::testInstallProgramsAstParsing()
{

}

void CMakeAstTests::testInstallTargetsAstParsing()
{

}

void CMakeAstTests::testLinkDirsAstParsing()
{

}

void CMakeAstTests::testLinkLibsAstParsing()
{

}

void CMakeAstTests::testListAstParsing()
{

}

void CMakeAstTests::testLoadCacheAstParsing()
{

}

void CMakeAstTests::testLoadCommandAstParsing()
{

}

void CMakeAstTests::testMacroAstParsing()
{

}

void CMakeAstTests::testMakeDirAstParsing()
{

}

void CMakeAstTests::testMarkAdvancedAstParsing()
{

}

void CMakeAstTests::testMathAstParsing()
{

}

void CMakeAstTests::testMessageAstParsing()
{

}

void CMakeAstTests::testOptionAstParsing()
{

}

void CMakeAstTests::testOutputReqFilesAstParsing()
{

}

void CMakeAstTests::testProjectAstParsing()
{

}

void CMakeAstTests::testQtWrapCppAstParsing()
{

}

void CMakeAstTests::testQtWrapUiAstParsing()
{

}

void CMakeAstTests::testRemoveAstParsing()
{

}

void CMakeAstTests::testRemoveDefinitionsAstParsing()
{

}

void CMakeAstTests::testSeparateArgsAstParsing()
{

}

void CMakeAstTests::testSetAstParsing()
{

}

void CMakeAstTests::testSetDirPropsAstParsing()
{

}

void CMakeAstTests::testSetSourceFilePropsAstParsing()
{

}

void CMakeAstTests::testSetTargetPropsAstParsing()
{

}

void CMakeAstTests::testSetTestsPropsAstParsing()
{

}

void CMakeAstTests::testSiteNameAstParsing()
{

}

void CMakeAstTests::testSourceGroupAstParsing()
{

}

void CMakeAstTests::testStringAstParsing()
{

}

void CMakeAstTests::testSubdirDependsAstParsing()
{

}

void CMakeAstTests::testSubdirsAstParsing()
{

}

void CMakeAstTests::testTargetLinkLibsAstParsing()
{

}

void CMakeAstTests::testTryCompileAstParsing()
{

}

void CMakeAstTests::testTryRunAstParsing()
{

}

void CMakeAstTests::testUseMangledMesaAstParsing()
{

}

void CMakeAstTests::testUtilitySourceAstParsing()
{

}

void CMakeAstTests::testVarRequiresAstParsing()
{

}

void CMakeAstTests::testVtkMakeAstParsing()
{

}

void CMakeAstTests::testVtkWrapJavaParsing()
{

}

void CMakeAstTests::testVtkWrapPythonParsing()
{

}

void CMakeAstTests::testVtkWrapTclParsing()
{

}

void CMakeAstTests::testWhileParsing()
{

}

void CMakeAstTests::testWriteFileParsing()
{

}

#include "cmakeasttest.moc"
