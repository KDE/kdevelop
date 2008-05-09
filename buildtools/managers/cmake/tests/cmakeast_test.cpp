#include "cmakeast_test.h"

#include "cmakeast.h"
#include "cmakelistsparser.h"

QTEST_MAIN( CMakeAstTest )

void CMakeAstTest::testAddExecutableGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AddExecutableAst* ast = new AddExecutableAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testAddExecutableGoodParse_data()
{
    CMakeFunctionDesc func;
    func.name = "add_executable";
    QStringList argList;
    argList << "foo" << "bar.c";
    func.addArguments( argList );

    CMakeFunctionDesc func2;
    func2.name = "add_executable";
    QStringList argList2;
    argList2 << "foo" << "WIN32" << "${mysrcs_SRCS}";
    func2.addArguments( argList2 );

    CMakeFunctionDesc func3;
    func3.name = "add_executable";
    QStringList argList3;
    argList3 << "foo" << "MACOSX_BUNDLE" << "${mysrcs_SRCS}";
    func3.addArguments( argList3 );

    CMakeFunctionDesc func4;
    func4.name = "add_executable";
    QStringList argList4;
    argList4 << "foo" << "EXCLUDE_FROM_ALL" << "${mysrcs_SRCS}";
    func4.addArguments( argList4 );

    CMakeFunctionDesc func5;
    func5.name = "add_executable";
    QStringList argList5;
    argList5 << "foo" << "WIN32" << "MACOSX_BUNDLE" << "EXCLUDE_FROM_ALL"
             << "${mysrcs_SRCS}";
    func5.addArguments( argList5 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "only sources" ) << func;
    QTest::newRow( "win 32 app" ) << func2;
    QTest::newRow( "mac os bundle" ) << func3;
    QTest::newRow( "exclude from all" ) << func4;
    QTest::newRow( "all" ) << func5;
}

void CMakeAstTest::testAddExecutableBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AddExecutableAst* ast = new AddExecutableAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testAddExecutableBadParse_data()
{
    CMakeFunctionDesc func;
    func.name = "add_executable";
    QStringList argList;
    argList << "foo";
    func.addArguments( argList );

    CMakeFunctionDesc func2;
    func2.name = "addexecutable";
    QStringList argList2;
    argList2 << "foo" << "${mysrcs_SRCS}";
    func2.addArguments( argList2 );

    CMakeFunctionDesc func3;
    func3.name = "add_executable";
    QStringList argList3;
    func3.addArguments( argList3 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "no sources" ) << func;
    QTest::newRow( "wrong name" ) << func2;
    QTest::newRow( "no arguments" ) << func3;

}










void CMakeAstTest::testAddLibraryGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AddLibraryAst* ast = new AddLibraryAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testAddLibraryGoodParse_data()
{
    CMakeFunctionDesc func, func2, func3, func4;
    QStringList argList, argList2, argList3, argList4;

    func.name = func2.name = func3.name = func4.name =  "add_library";
    argList << "foo" << "${SRCS}";
    func.addArguments( argList );

    argList2 << "foo2" << "SHARED" << "${SRCS}";
    func2.addArguments( argList2 );

    argList3 << "foo3" << "EXCLUDE_FROM_ALL" << "${SRCS}";
    func3.addArguments( argList3 );

    argList4 << "foo4" << "MODULE" << "EXCLUDE_FROM_ALL" << "foo.c"
             << "bar.c" << "baz.c";
    func4.addArguments( argList4 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "simple" ) << func;
    QTest::newRow( "shared" ) << func2;
    QTest::newRow( "exclude" ) << func3;
    QTest::newRow( "full" ) << func4;
}

void CMakeAstTest::testAddLibraryBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AddLibraryAst* ast = new AddLibraryAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testAddLibraryBadParse_data()
{
    CMakeFunctionDesc func, func2, func3;
    QStringList argList, argList2, argList3;

    func.name = func2.name = func3.name = "add_library";
    func.name = "wrong_name";
    argList << "foo" << "${SRCS}";
    func.addArguments( argList );

    func2.addArguments( argList2 );

    argList3 << "no-sources" << "EXCLUDE_FROM_ALL";
    func3.addArguments( argList3 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "wrong name" ) << func;
    QTest::newRow( "no args" ) << func2;
    QTest::newRow( "no sources" ) << func3;
}









void CMakeAstTest::testAddSubdirectoryGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AddSubdirectoryAst* ast = new AddSubdirectoryAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testAddSubdirectoryGoodParse_data()
{
    CMakeFunctionDesc func, func2, func3, func4;
    func.name = "ADD_SUBDIRECTORY";
    func.addArguments( QStringList( "foodir" ) );

    func2.name = "add_subdirectory";
    func2.addArguments( QStringList( "foodir" ) );

    func3.name = "add_subdirectory";
    QStringList argList3;
    argList3 << "foodir" << "binary_foo_dir" << "EXCLUDE_FROM_ALL";
    func3.addArguments( argList3 );

    func4.name = "add_subdirectory";
    QStringList argList4;
    argList4 << "foodri" << "binary_foo_dir";
    func4.addArguments( argList4 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "good lowercase" ) << func;
    QTest::newRow( "good uppercase" ) << func2;
    QTest::newRow( "good all args" ) << func3;
    QTest::newRow( "good binary dir only" ) << func4;
}

void CMakeAstTest::testAddSubdirectoryBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AddSubdirectoryAst* ast = new AddSubdirectoryAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testAddSubdirectoryBadParse_data()
{
    CMakeFunctionDesc func, func2;
    func.name = "ADD_SUBDIRECTORY";
    func2.name = "foobar";

    func2.addArguments( QStringList( "foodir" ) );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "good lowercase" ) << func;
    QTest::newRow( "good uppercase" ) << func2;
}

void CMakeAstTest::testAddTestGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AddTestAst* ast = new AddTestAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testAddTestGoodParse_data()
{
    CMakeFunctionDesc func1, func2;
    func1.name = "ADD_TEST";
    func2.name = "add_test";

    QStringList argList1, argList2;
    argList1 << "test_name" << "exec_name";
    argList2 << "test_name" << "exec_name" << "arg1";

    func1.addArguments( argList1 );
    func2.addArguments( argList2 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "good req args" ) << func1;
    QTest::newRow( "good opt args" ) << func2;
}

void CMakeAstTest::testAddTestBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AddTestAst* ast = new AddTestAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testAddTestBadParse_data()
{
    CMakeFunctionDesc func1, func2, func3;
    func1.name = "wrong_name";
    func2.name = func3.name = "add_test";

    QStringList argList1, argList2, argList3;
    argList1 << "some" << "args";
    argList2 << "one arg";

    func1.addArguments( argList1 );
    func2.addArguments( argList2 );
    func3.addArguments( argList3 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "wrong name" ) << func1;
    QTest::newRow( "not enough args" ) << func2;
    QTest::newRow( "no args" ) << func3;

}










void CMakeAstTest::testAuxSourceDirectoryGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AuxSourceDirectoryAst* ast = new AuxSourceDirectoryAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testAuxSourceDirectoryGoodParse_data()
{
    CMakeFunctionDesc func1, func2;
    func1.name = "AUX_SOURCE_DIRECTORY";
    func2.name = "aux_source_directory";

    QStringList argList;
    argList << "foo1" << "foo2";

    func1.addArguments( argList );
    func2.addArguments( argList );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "good uppercase" ) << func1;
    QTest::newRow( "good lowercase" ) << func2;
}

void CMakeAstTest::testAuxSourceDirectoryBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AuxSourceDirectoryAst* ast = new AuxSourceDirectoryAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testAuxSourceDirectoryBadParse_data()
{
    CMakeFunctionDesc func1, func2;
    func1.name = "AUX_SOURCE_DIRECTORY";
    func2.name = "wrong name";

    QStringList argList;
    argList << "foo1" << "foo2";

    func2.addArguments( argList );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "bad no args" ) << func1;
    QTest::newRow( "bad wrong name" ) << func2;
}

void CMakeAstTest::testBreakGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    BreakAst* ast = new BreakAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testBreakGoodParse_data()
{
    CMakeFunctionDesc func1, func2;
    func1.name = "break";

    func2.addArguments( QStringList() );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "good" ) << func1;
}

void CMakeAstTest::testBreakBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    BreakAst* ast = new BreakAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testBreakBadParse_data()
{
    CMakeFunctionDesc func1, func2;
    func1.name = "break";
    func2.name = "wrong name";

    QStringList argList;
    argList << "foo1" << "foo2";

    func1.addArguments( argList );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "bad with args" ) << func1;
    QTest::newRow( "bad wrong name" ) << func2;
}

void CMakeAstTest::testBuildCommandGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    BuildCommandAst* ast = new BuildCommandAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testBuildCommandGoodParse_data()
{
    CMakeFunctionDesc func1, func2;
    func1.name = "build_command";
    func2.name = func1.name.toUpper();

    QStringList argList;
    argList << "mybuildtool" << "my_cool_build";
    func1.addArguments( argList );
    func2.arguments = func1.arguments;

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "good lower" ) << func1;
    QTest::newRow( "good upper" ) << func2;
}

void CMakeAstTest::testBuildCommandBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    BuildCommandAst* ast = new BuildCommandAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testBuildCommandBadParse_data()
{
    CMakeFunctionDesc func1, func2, func3;
    func1.name = "build_command";
    func2.name = "some_other_foo";
    func3.name = func1.name;

    QStringList argList, argList2;
    argList << "mymake" << "unsermake";
    argList2 << "single argument";
    func2.addArguments( argList );
    func3.addArguments( argList2 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "bad no args" ) << func1;
    QTest::newRow( "bad wrong name" ) << func2;
    QTest::newRow( "bad not enough args" ) << func3;
}










void CMakeAstTest::testBuildNameGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    BuildNameAst* ast = new BuildNameAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testBuildNameGoodParse_data()
{
    CMakeFunctionDesc func1, func2;
    func1.name = "build_name";
    func2.name = func1.name.toUpper();

    QStringList argList;
    argList << "my_cool_build";
    func1.addArguments( argList );
    func2.arguments = func1.arguments;

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "good lower" ) << func1;
    QTest::newRow( "good upper" ) << func2;
}

void CMakeAstTest::testBuildNameBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    BuildNameAst* ast = new BuildNameAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testBuildNameBadParse_data()
{
    CMakeFunctionDesc func1, func2;
    func1.name = "build_name";
    func2.name = "wrong_func_name";

    QStringList argList;
    argList << "my_cool_build";
    func2.arguments = func1.arguments;

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "bad no args" ) << func1;
    QTest::newRow( "ban wrong name" ) << func2;
}

void CMakeAstTest::testCMakeMinimumRequiredGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeMinimumRequiredAst* ast = new CMakeMinimumRequiredAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testCMakeMinimumRequiredGoodParse_data()
{
    CMakeFunctionDesc func1, func2, func3;
    func1.name = "CMAKE_MINIMUM_REQUIRED";
    func2.name = func3.name = func1.name.toLower();
    QStringList argList1, argList2, argList3;

    argList1 << "VERSION" << "2.4";
    argList2 = argList1;
    argList2 << "FATAL_ERROR";

    func1.addArguments( argList1 );
    func2.addArguments( argList1 );
    func3.addArguments( argList2 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "good upper case" ) << func1;
    QTest::newRow( "good lower case" ) << func2;
    QTest::newRow( "good all args" ) << func3;
}

void CMakeAstTest::testCMakeMinimumRequiredBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeMinimumRequiredAst* ast = new CMakeMinimumRequiredAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testCMakeMinimumRequiredBadParse_data()
{
    CMakeFunctionDesc func1, func2, func3, func4;
    func1.name = "wrong_name";
    func2.name = func3.name = "cmake_required_version";
    func4.name = func3.name;
    QStringList argList1, argList2, argList3, argList4;

    argList1 << "VERSION" << "2.4";
    argList2 << "VERSION";
    argList3 << "VERSION" << "FATAL_ERROR";


    func1.addArguments( argList1 );
    func2.addArguments( argList1 );
    func3.addArguments( argList3 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "wrong name" ) << func1;
    QTest::newRow( "no version number 1" ) << func2;
    QTest::newRow( "no version number 2" ) << func3;
    QTest::newRow( "no arguments" ) << func4;
}

void CMakeAstTest::testCMakePolicyGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakePolicyAst* ast = new CMakePolicyAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testCMakePolicyGoodParse_data()
{
    CMakeFunctionDesc func[4];
    func[0].name = "CMAKE_POLICY";
    func[1].name = func[2].name = func[3].name = func[0].name.toLower();
    QStringList argList[4];

    argList[0] << "VERSION" << "2.4";
    argList[1] << "SET CMP3333 NEW";
    argList[2] << "POP";
    argList[3] << "PUSH";

    func[0].addArguments( argList[0] );
    func[1].addArguments( argList[1] );
    func[2].addArguments( argList[2] );
    func[3].addArguments( argList[3] );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "good version" ) << func[0];
    QTest::newRow( "good cmpset" ) << func[1];
    QTest::newRow( "good pop" ) << func[2];
    QTest::newRow( "good push" ) << func[3];
}

void CMakeAstTest::testCMakePolicyBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakePolicyAst* ast = new CMakePolicyAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testCMakePolicyBadParse_data()
{
    CMakeFunctionDesc func[6];
    func[0].name = "iamwrong";
    func[1].name = func[2].name = func[3].name = func[4].name = func[5].name = "cmake_policy";
    QStringList argList[5];

    argList[0] << "VERSION" << "24";
    argList[1] << "SET NEW";
    argList[2] << "POP" << "33";
    argList[3] << "PUSH" << "44";
    argList[4] << "BUTTERFLY" << "44";

    func[0].addArguments( argList[0] );
    func[1].addArguments( argList[0] );
    func[2].addArguments( argList[1] );
    func[3].addArguments( argList[2] );
    func[4].addArguments( argList[3] );
    func[5].addArguments( argList[4] );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "bad name" ) << func[0];
    QTest::newRow( "bad version" ) << func[1];
    QTest::newRow( "bad cmpset" ) << func[2];
    QTest::newRow( "bad pop" ) << func[3];
    QTest::newRow( "bad push" ) << func[4];
    QTest::newRow( "bad parameter" ) << func[4];

}

void CMakeAstTest::testConfigureFileGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    ConfigureFileAst* ast = new ConfigureFileAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testConfigureFileGoodParse_data()
{
    CMakeFunctionDesc func1, func2, func3, func4, func5;
    func1.name = "CONFIGURE_FILE";
    func2.name = func1.name.toLower();
    func3.name = func4.name = func5.name = func2.name;

    QStringList argList1, argList2, argList3, argList4;
    argList1 << "inputfile" << "outputfile";
    argList2 = argList1 << "COPYONLY";
    argList3 = argList2 << "ESCAPE_QUOTES";
    argList4 = argList3 << "@ONLY";

    func1.addArguments( argList1 );
    func2.addArguments( argList1 );
    func3.addArguments( argList2 );
    func4.addArguments( argList3 );
    func5.addArguments( argList4 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "good uppercase min args" ) << func1;
    QTest::newRow( "good lowercase min args" ) << func2;
    QTest::newRow( "good lowercase 3 args" ) << func3;
    QTest::newRow( "good lowercase 4 args" ) << func4;
    QTest::newRow( "good lowercase 5 args" ) << func5;
}

void CMakeAstTest::testConfigureFileBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    ConfigureFileAst* ast = new ConfigureFileAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testConfigureFileBadParse_data()
{
    CMakeFunctionDesc func1, func2, func3;
    func1.name = "wrong_func_name";
    func2.name = func3.name = "configure_file";

    QStringList argList1, argList2, argList3;
    argList1 << "sourcefile" << "outputfile";
    argList2 << "only_one";

    func1.addArguments( argList1 );
    func2.addArguments( argList2 );
    func2.addArguments( argList3 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "bad wrong name" ) << func1;
    QTest::newRow( "bad only one arg" ) << func2;
    QTest::newRow( "bad no args" ) << func3;
}

void CMakeAstTest::testCreateTestSourcelistGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CreateTestSourcelistAst* ast = new CreateTestSourcelistAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testCreateTestSourcelistGoodParse_data()
{
    CMakeFunctionDesc func1, func2, func3, func4;
    func1.name = "create_test_sourcelist";
    func2.name = func1.name;
    func3.name = func2.name.toUpper();
    func4.name = func3.name;

    QStringList argList1, argList2, argList3, argList4;
    argList1 << "$(TESTDRIVER_SRCS) test_driver $(TEST_SRCS)";
    argList2 = argList1;
    argList2 << "EXTRA_INCLUDE" << "include.h";
    argList3 = argList1;
    argList3 << "FUNCTION" << "function";
    argList4 = argList1;
    argList4 << "EXTRA_INCLUDE" << "include.h";
    argList4 << "FUNCTION" << "function";

    func1.addArguments( argList1 );
    func2.addArguments( argList2 );
    func3.addArguments( argList3 );
    func4.addArguments( argList4 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "good min lower case" ) << func1;
    QTest::newRow( "good lower case extra include" ) << func2;
    QTest::newRow( "good upper extra function" ) << func3;
    QTest::newRow( "good upper all args" ) << func4;
}

void CMakeAstTest::testCreateTestSourcelistBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CreateTestSourcelistAst* ast = new CreateTestSourcelistAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testCreateTestSourcelistBadParse_data()
{
    CMakeFunctionDesc func1, func2, func3, func4;
    func1.name = "create_test_sourcelists";
    func2.name = "create_test_sourcelist";
    func3.name = func2.name.toUpper();
    func4.name = func3.name;

    QStringList argList1, argList2, argList3, argList4;
    argList1 << "$(TESTDRIVER_SRCS) test_driver $(TEST_SRCS)";
    argList2 = argList1;
    argList2 << "EXTRA_INCLUDE";
    argList3 = argList1;
    argList3 << "FUNCTION";
    argList4 << "foo bar";

    func1.addArguments( argList1 );
    func2.addArguments( argList2 );
    func3.addArguments( argList3 );
    func4.addArguments( argList4 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "bad name lower case" ) << func1;
    QTest::newRow( "bad extra include not enough args" ) << func2;
    QTest::newRow( "bad extra function not enough args" ) << func3;
    QTest::newRow( "bad upper need at least 3 args" ) << func4;
}










void CMakeAstTest::testEnableLanguageGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    EnableLanguageAst* ast = new EnableLanguageAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testEnableLanguageGoodParse_data()
{
    CMakeFunctionDesc func1, func2;
    func1.name = "ENABLE_LANGUAGE";
    func2.name = "enable_language";

    QStringList argList1, argList2;
    argList1 << "C++";
    argList2 << "Java";

    func1.addArguments( argList1 );
    func2.addArguments( argList2 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "good uppercase" ) << func1;
    QTest::newRow( "good lowercase" ) << func2;
}

void CMakeAstTest::testEnableLanguageBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    EnableLanguageAst* ast = new EnableLanguageAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testEnableLanguageBadParse_data()
{
    CMakeFunctionDesc func1, func2;
    func1.name = "ENABLE_LANGUAGES";
    func2.name = "enable_language";

    QStringList argList1, argList2;
    argList1 << "C++";

    func1.addArguments( argList1 );
    func2.addArguments( argList2 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "bad uppercase" ) << func1;
    QTest::newRow( "bad lowercase. no param" ) << func2;
}










void CMakeAstTest::testEnableTestingGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    EnableTestingAst* ast = new EnableTestingAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testEnableTestingGoodParse_data()
{
    CMakeFunctionDesc func1, func2;
    func1.name = "ENABLE_TESTING";
    func2.name = "enable_testing";

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "good upper" ) << func1;
    QTest::newRow( "good lower" ) << func2;

}

void CMakeAstTest::testEnableTestingBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    EnableTestingAst* ast = new EnableTestingAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testEnableTestingBadParse_data()
{
    CMakeFunctionDesc func1, func2;
    func1.name = "bad_func_name";
    func2.name = "enable_testing";

    QStringList argList;
    argList << "foo";

    func2.addArguments( argList );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "bad wrong name" ) << func1;
    QTest::newRow( "bad has args" ) << func2;

}










void CMakeAstTest::testExecProgramGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    ExecProgramAst* ast = new ExecProgramAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testExecProgramGoodParse_data()
{
    CMakeFunctionDesc func1, func2, func3, func4;
    func1.name = "exec_program";
    func2.name = func1.name.toUpper();
    func3.name = func4.name = func1.name;

    QStringList argList1, argList2, argList3, argList4;
    argList1 << "myExec";
    argList2 = argList1;
    argList2 << "myRunDir";
    argList3 = argList1;
    argList3 << "ARGS" << "arg1" << "arg2";
    argList4 = argList1;
    argList4 << "RETURN_VALUE" << "myReturnVar";

    func1.addArguments( argList1 );
    func2.addArguments( argList2 );
    func3.addArguments( argList3 );
    func4.addArguments( argList4 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "good 1" ) << func1;
    QTest::newRow( "good 2" ) << func2;
    QTest::newRow( "good 3" ) << func3;
    QTest::newRow( "good 4" ) << func4;

}

void CMakeAstTest::testExecProgramBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    ExecProgramAst* ast = new ExecProgramAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testExecProgramBadParse_data()
{
    CMakeFunctionDesc func1, func2;
    func1.name = "wrong_name_here";
    func2.name = "exec_program";

    QStringList argList1;
    argList1 << "myExec";

    func1.addArguments( argList1 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "bad 1" ) << func1;
    QTest::newRow( "bad 2" ) << func2;
}










void CMakeAstTest::testExecuteProcessGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    ExecuteProcessAst* ast = new ExecuteProcessAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testExecuteProcessGoodParse_data()
{
}

void CMakeAstTest::testExecuteProcessBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    ExecuteProcessAst* ast = new ExecuteProcessAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testExecuteProcessBadParse_data()
{
}

void CMakeAstTest::testExportGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    ExportAst* ast = new ExportAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testExportGoodParse_data()
{
}

void CMakeAstTest::testExportBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    ExportAst* ast = new ExportAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testExportBadParse_data()
{
}










void CMakeAstTest::testExportLibraryDepsGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    ExportLibraryDepsAst* ast = new ExportLibraryDepsAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testExportLibraryDepsGoodParse_data()
{
    CMakeFunctionDesc func1, func2;
    func1.name = func2.name = "export_library_dependencies";

    QStringList argList1, argList2;
    argList1 << "dep_file";
    argList2 = argList1;
    argList2 << "APPEND";

    func1.addArguments( argList1 );
    func2.addArguments( argList2 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "good 1" ) << func1;
    QTest::newRow( "good 2" ) << func2;
 
}

void CMakeAstTest::testExportLibraryDepsBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    ExportLibraryDepsAst* ast = new ExportLibraryDepsAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testExportLibraryDepsBadParse_data()
{
    CMakeFunctionDesc func1, func2;
    func1.name = "export_library_dependencies";
    func2.name = "foo_foo_bar";

    QStringList argList1, argList2;
    argList1 << "dep_file";
    argList2 = argList1;
    argList2 << "APPEND";

    func1.addArguments( argList1 );
    func2.addArguments( argList2 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "bad 1" ) << func1;
    QTest::newRow( "bad 2" ) << func2;
}










void CMakeAstTest::testFileGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    FileAst* ast = new FileAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testFileGoodParse_data()
{
    const int NUM_TESTDATA = 13;
    CMakeFunctionDesc funcs[NUM_TESTDATA];
    QStringList args[NUM_TESTDATA];

    for ( int i = 0; i < NUM_TESTDATA; i++ )
        funcs[i].name = "FILE";

    //write file command
    args[0] << "WRITE" << "somefile.cpp" << "\"the things to write\"";

    //append file command
    args[1] << "APPEND" << "somefile.cpp" << "\"the things to append\"";

    //read file command
    args[2] << "READ" << "somefile.cpp" << "MY_VAR";

    //glob files command. does not search in files only for a pattern
    args[3] << "GLOB" << "MY_VAR" << "*.cpp";
    args[4] << "GLOB" << "MY_VAR" << "RELATIVE" << "/path/to/something"
            << "*.cpp"; //RELATIVE is optional

    //recursive glob
    args[5] << "GLOB_RECURSE" << "MY_VAR" << "*.cpp";
    args[6] << "GLOB_RECURSE" << "MY_VAR" << "RELATIVE" << "/path/to/something"
            << "*.cpp"; //RELATIVE is optional

    //remove command
    args[7] << "REMOVE" << "/path/to/file/to/remove.cpp";

    //remove recursive
    args[8] << "REMOVE_RECURSE" << "/path/to/dir/to/remove/files";

    //make a directory
    args[9] << "MAKE_DIRECTORY" << "/path/to/dir/to/create";

    //get a relative path
    args[10] << "RELATIVE_PATH" << "MY_VAR" << "/path/to/foo"
             << "/path/to/file/to/get/path/for.cpp";

    //get the cmake native path
    args[11] << "TO_CMAKE_PATH" << "/path/to/file.cpp" << "MY_CMAKE_PATH";

    //get the platform native path
    args[12] << "TO_NATIVE_PATH" << "/path/to/file.cpp" << "MY_NATIVE_PATH";


    QTest::addColumn<CMakeFunctionDesc>( "function" );

    for ( int i = 0; i < NUM_TESTDATA; i++)
    {
        funcs[i].addArguments( args[i] );
        QTest::newRow( qPrintable(QString::number(i)) ) << funcs[i];
    }

}

void CMakeAstTest::testFileBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    FileAst* ast = new FileAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testFileBadParse_data()
{

    const int NUM_TESTDATA = 35;
    CMakeFunctionDesc funcs[NUM_TESTDATA];
    QStringList args[NUM_TESTDATA];

    for ( int i = 0; i < NUM_TESTDATA; i++ )
        funcs[i].name = "FILE";

    funcs[NUM_TESTDATA - 1].name = "foo";
    
    //write file command
    args[0] << "WRITE" << "somefile.cpp"; //nothing to write
    args[1] << "WRITE"; //no file
    args[2] << "write" << "somefile.cpp" << "thing to write"; //uppercase required

    //append file command
    args[3] << "APPEND" << "somefile.cpp"; //nothing to append
    args[4] << "APPEND"; //no file
    args[5] << "append" << "somefile.cpp" << "thing to append"; //uppercase required

    //read file command
    args[6] << "READ" << "somefile.cpp"; //no variable
    args[7] << "READ"; //no file
    args[8] << "read" << "somefile.cpp" << "MY_VAR"; //uppercase required

    //glob files command. does not search in files only for a pattern
    args[9] << "GLOB"; //no variable
    args[10] << "GLOB" << "MY_VAR" << "RELATIVE"; //no path
    
    args[11] << "glob" << "MY_VAR" << "*.cpp"; //uppercase required
    args[12] << "GLOB" << "MY_VAR" << "relative" << "/path/to/something"
               << "*.cpp"; //uppercase required

    //recursive glob
    args[13] << "GLOB_RECURSE"; //no variable
    args[14] << "GLOB_RECURSE" << "MY_VAR" << "RELATIVE"; //no path

    args[15] << "glob_recurse" << "MY_VAR" << "*.cpp"; //uppercase required
    args[16] << "GLOB_RECURSE" << "MY_VAR" << "relative" << "/path/to/something"
            << "*.cpp"; //uppercase required

    //remove command
    args[17] << "REMOVE"; //nothing to remove
    args[18] << "remove" << "/path/to/file/to/remove.cpp";

    //remove recursive
    args[19] << "REMOVE_RECURSE"; //nothing to remove
    args[20] << "remove_recurse" << "/path/to/dir"; //uppercase required

    //make a directory
    args[21] << "MAKE_DIRECTORY"; //nothing to create
    args[22] << "make_directory" << "/path/to/dir"; //uppercase required

    //get a relative path
    args[23] << "RELATIVE_PATH" << "MY_VAR" << "/path/to/foo"; //no file
    args[24] << "RELATIVE_PATH" << "MY_VAR"; //no path and no file
    args[25] << "RELATIVE_PATH"; //no variable, path, or file
    args[26] << "relative_path" << "MY_VAR" << "/path/to/foo"
        << "/path/to/file/to/get/full/path/for.cpp"; // uppercase required

    //get the cmake native path
    args[27] << "TO_CMAKE_PATH" << "/path/to/file.cpp"; // no variable
    args[28] << "TO_CMAKE_PATH"; //no path or variable
    args[29] << "to_cmake_path" << "/path/to/file.cpp" << "MY_VAR"; //uppercase required
    
    //get the platform native path
    args[30] << "TO_NATIVE_PATH" << "/path/to/file.cpp"; //no variable
    args[31] << "TO_NATIVE_PATH"; //no path or variable
    args[32] << "to_native_path" << "/path/to/file.cpp" << "MY_VAR"; //uppercase required
    
    args[34] << "TO_NATIVE_PATH" << "/path/to/file.cpp" << "MY_VAR"; //correct args. wrong name
    
    QTest::addColumn<CMakeFunctionDesc>( "function" );

    for ( int i = 0; i < NUM_TESTDATA; i++)
    {
        funcs[i].addArguments( args[i] );
        QTest::newRow( qPrintable(QString::number(i)) ) << funcs[i];
    }

}










void CMakeAstTest::testFindFileGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    FindFileAst* ast = new FindFileAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testFindFileGoodParse_data()
{
}

void CMakeAstTest::testFindFileBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    FindFileAst* ast = new FindFileAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testFindFileBadParse_data()
{
}










void CMakeAstTest::testFindLibraryGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    FindLibraryAst* ast = new FindLibraryAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testFindLibraryGoodParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>("function");
    
    CMakeFunctionDesc l;
    l.name = "find_library";
    l.addArguments(QString("DEST_VAR name").split(' '));
    QTest::newRow("findlib with name") << l;
    
    l.arguments.clear();
    l.addArguments(QString("DEST_VAR name /path/to/lib").split(' '));
    QTest::newRow("findlib with name and path") << l;
}

void CMakeAstTest::testFindLibraryBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    FindLibraryAst* ast = new FindLibraryAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testFindLibraryBadParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>("function");
    
    CMakeFunctionDesc l;
    l.name = "lol";
    l.addArguments(QString("DEST_VAR name").split(' '));
    QTest::newRow("findlib with name") << l;
}










void CMakeAstTest::testFindPackageGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    FindPackageAst* ast = new FindPackageAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testFindPackageGoodParse_data()
{
}

void CMakeAstTest::testFindPackageBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    FindPackageAst* ast = new FindPackageAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testFindPackageBadParse_data()
{
}










void CMakeAstTest::testFindPathGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    FindPathAst* ast = new FindPathAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testFindPathGoodParse_data()
{
}

void CMakeAstTest::testFindPathBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    FindPathAst* ast = new FindPathAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testFindPathBadParse_data()
{
}










void CMakeAstTest::testFindProgramGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    FindProgramAst* ast = new FindProgramAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testFindProgramGoodParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>("function");

    CMakeFunctionDesc l;
    l.name = "find_program";
    l.addArguments(QStringList() << "MY_VAR" << "file" << "location");
    QTest::newRow("normal use") << l;

    l.arguments.clear();
    l.addArguments(QStringList() << "MY_VAR" << "NAMES" << "file1" << "file2" << "PATHS" << "location1" << "location2");
    QTest::newRow("advanced use") << l;

    l.arguments.clear();
    l.addArguments(QStringList() << "MY_VAR" << "NAMES" << "file1" << "file2"
            << "PATHS" << "location1" << "location2" << "DOC" << "I am documenting"
            << "PATH_SUFFIXES" << "modules" << "NO_CMAKE_PATH");
    QTest::newRow("strange use") << l;
}

void CMakeAstTest::testFindProgramBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    FindProgramAst* ast = new FindProgramAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testFindProgramBadParse_data()
{
    
    QTest::addColumn<CMakeFunctionDesc>("function");
    
    CMakeFunctionDesc l1;
    l1.name = "";
    l1.addArguments(QStringList() << "MY_VAR" << "file");
    QTest::newRow ("no function name") << l1;

    CMakeFunctionDesc l;
    l.name = "find_program";
    l.addArguments(QStringList() << "MY_VAR" << "file");
    QTest::newRow("not enough parameters") << l;

    l.arguments.clear();
    l.addArguments(QStringList() << "MY_VAR" << "NAMES" << "PATHS" << "location1" << "location2");
    QTest::newRow("no names") << l;

    l.arguments.clear();
    l.addArguments(QStringList() << "MY_VAR" << "NAMES" << "file1" << "file2"
            << "PATHS"
            << "PATH_SUFFIXES" << "modules" << "NO_CMAKE_PATH");
    QTest::newRow("no paths") << l;
}










void CMakeAstTest::testFltkWrapUiGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    FltkWrapUiAst* ast = new FltkWrapUiAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testFltkWrapUiGoodParse_data()
{
}

void CMakeAstTest::testFltkWrapUiBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    FltkWrapUiAst* ast = new FltkWrapUiAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testFltkWrapUiBadParse_data()
{
}










void CMakeAstTest::testForeachGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    ForeachAst* ast = new ForeachAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testForeachGoodParse_data()
{
}

void CMakeAstTest::testForeachBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    ForeachAst* ast = new ForeachAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testForeachBadParse_data()
{
}











void CMakeAstTest::testGetCMakePropertyGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    GetCMakePropertyAst* ast = new GetCMakePropertyAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testGetCMakePropertyGoodParse_data()
{
}

void CMakeAstTest::testGetCMakePropertyBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    GetCMakePropertyAst* ast = new GetCMakePropertyAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testGetCMakePropertyBadParse_data()
{
}










void CMakeAstTest::testGetDirPropertyGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    GetDirPropertyAst* ast = new GetDirPropertyAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testGetDirPropertyGoodParse_data()
{
}

void CMakeAstTest::testGetDirPropertyBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    GetDirPropertyAst* ast = new GetDirPropertyAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testGetDirPropertyBadParse_data()
{
}










void CMakeAstTest::testGetFilenameComponentGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    GetFilenameComponentAst* ast = new GetFilenameComponentAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testGetFilenameComponentGoodParse_data()
{
}

void CMakeAstTest::testGetFilenameComponentBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    GetFilenameComponentAst* ast = new GetFilenameComponentAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testGetFilenameComponentBadParse_data()
{
}










void CMakeAstTest::testGetSourceFilePropGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    GetSourceFilePropAst* ast = new GetSourceFilePropAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testGetSourceFilePropGoodParse_data()
{
}

void CMakeAstTest::testGetSourceFilePropBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    GetSourceFilePropAst* ast = new GetSourceFilePropAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testGetSourceFilePropBadParse_data()
{
}










void CMakeAstTest::testGetTargetPropGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    GetTargetPropAst* ast = new GetTargetPropAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testGetTargetPropGoodParse_data()
{
}

void CMakeAstTest::testGetTargetPropBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    GetTargetPropAst* ast = new GetTargetPropAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testGetTargetPropBadParse_data()
{
}










void CMakeAstTest::testGetTestPropGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    GetTestPropAst* ast = new GetTestPropAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testGetTestPropGoodParse_data()
{
}

void CMakeAstTest::testGetTestPropBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    GetTestPropAst* ast = new GetTestPropAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testGetTestPropBadParse_data()
{
}










void CMakeAstTest::testIfGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    IfAst* ast = new IfAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testIfGoodParse_data()
{
}

void CMakeAstTest::testIfBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    IfAst* ast = new IfAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testIfBadParse_data()
{
}










void CMakeAstTest::testIncludeGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    IncludeAst* ast = new IncludeAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testIncludeGoodParse_data()
{
    CMakeFunctionDesc func1, func2, func3;
    func1.name = "INCLUDE";
    func2.name = func3.name = func1.name.toLower();

    QStringList argList1, argList2;
    argList1 << "SomeFile";
    argList2 << "SomeFile" << "OPTIONAL";

    func1.addArguments( argList1 );
    func2.addArguments( argList1 );
    func3.addArguments( argList2 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "good upper" ) << func1;
    QTest::newRow( "good lower" ) << func2;
    QTest::newRow( "good all args" ) << func3;
}

void CMakeAstTest::testIncludeBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    IncludeAst* ast = new IncludeAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testIncludeBadParse_data()
{
    CMakeFunctionDesc func1, func2;
    func1.name = "WrongFuncName";
    func2.name = "include";

    QStringList argList1, argList2;
    argList1 << "SomeFile";

    func1.addArguments( argList1 );
    func2.addArguments( argList2 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "bad wrong name" ) << func1;
    QTest::newRow( "bad no args" ) << func2;

}










void CMakeAstTest::testIncludeDirectoriesGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    IncludeDirectoriesAst* ast = new IncludeDirectoriesAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testIncludeDirectoriesGoodParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>("function");
    
    CMakeFunctionDesc l;
    l.name = "include_directories";
    l.addArguments(QStringList() << "../g4u");
    QTest::newRow("a normal include_directories") << l;
    
    l.arguments.clear();
    l.addArguments(QStringList() << "AFTER" << "boost/");
    QTest::newRow("a include_directories with AFTER parameter") << l;
    
    l.arguments.clear();
    l.addArguments(QStringList() << "SYSTEM" << "~/kdelibs");
    QTest::newRow("a include_directories with SYSTEM paremeter") << l;
}

void CMakeAstTest::testIncludeDirectoriesBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    IncludeDirectoriesAst* ast = new IncludeDirectoriesAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testIncludeDirectoriesBadParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>("function");
    
    CMakeFunctionDesc l;
    l.name = "include_directories";
    l.addArguments(QStringList() << "AFTER" << "BEFORE" << "lol");
    QTest::newRow("can't have after and before in include_directories") << l;
}










void CMakeAstTest::testIncludeExternalMsProjectGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    IncludeExternalMsProjectAst* ast = new IncludeExternalMsProjectAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testIncludeExternalMsProjectGoodParse_data()
{
}

void CMakeAstTest::testIncludeExternalMsProjectBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    IncludeExternalMsProjectAst* ast = new IncludeExternalMsProjectAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testIncludeExternalMsProjectBadParse_data()
{
}










void CMakeAstTest::testIncludeRegularExpressionGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    IncludeRegularExpressionAst* ast = new IncludeRegularExpressionAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testIncludeRegularExpressionGoodParse_data()
{
}

void CMakeAstTest::testIncludeRegularExpressionBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    IncludeRegularExpressionAst* ast = new IncludeRegularExpressionAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testIncludeRegularExpressionBadParse_data()
{
}










void CMakeAstTest::testInstallGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    InstallAst* ast = new InstallAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testInstallGoodParse_data()
{
}

void CMakeAstTest::testInstallBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    InstallAst* ast = new InstallAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testInstallBadParse_data()
{
}










void CMakeAstTest::testInstallFilesGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    InstallFilesAst* ast = new InstallFilesAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testInstallFilesGoodParse_data()
{
}

void CMakeAstTest::testInstallFilesBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    InstallFilesAst* ast = new InstallFilesAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testInstallFilesBadParse_data()
{
}










void CMakeAstTest::testInstallProgramsGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    InstallProgramsAst* ast = new InstallProgramsAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testInstallProgramsGoodParse_data()
{
}

void CMakeAstTest::testInstallProgramsBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    InstallProgramsAst* ast = new InstallProgramsAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testInstallProgramsBadParse_data()
{
}










void CMakeAstTest::testInstallTargetsGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    InstallTargetsAst* ast = new InstallTargetsAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testInstallTargetsGoodParse_data()
{
}

void CMakeAstTest::testInstallTargetsBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    InstallTargetsAst* ast = new InstallTargetsAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testInstallTargetsBadParse_data()
{
}










void CMakeAstTest::testLinkDirectoriesGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    LinkDirectoriesAst* ast = new LinkDirectoriesAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testLinkDirectoriesGoodParse_data()
{
}

void CMakeAstTest::testLinkDirectoriesBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    LinkDirectoriesAst* ast = new LinkDirectoriesAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testLinkDirectoriesBadParse_data()
{
}










void CMakeAstTest::testLinkLibrariesGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    LinkLibrariesAst* ast = new LinkLibrariesAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testLinkLibrariesGoodParse_data()
{
}

void CMakeAstTest::testLinkLibrariesBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    LinkLibrariesAst* ast = new LinkLibrariesAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testLinkLibrariesBadParse_data()
{
}










void CMakeAstTest::testListGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    ListAst* ast = new ListAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testListGoodParse_data()
{
}

void CMakeAstTest::testListBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    ListAst* ast = new ListAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testListBadParse_data()
{
}










void CMakeAstTest::testLoadCacheGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    LoadCacheAst* ast = new LoadCacheAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testLoadCacheGoodParse_data()
{
}

void CMakeAstTest::testLoadCacheBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    LoadCacheAst* ast = new LoadCacheAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testLoadCacheBadParse_data()
{
}










void CMakeAstTest::testLoadCommandGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    LoadCommandAst* ast = new LoadCommandAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testLoadCommandGoodParse_data()
{
}

void CMakeAstTest::testLoadCommandBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    LoadCommandAst* ast = new LoadCommandAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testLoadCommandBadParse_data()
{
}










void CMakeAstTest::testMacroGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    MacroAst* ast = new MacroAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testMacroGoodParse_data()
{
    CMakeFunctionDesc func1, func2, func3;
    func1.name = "MACRO";
    func2.name = func3.name = func1.name.toLower();

    QStringList argList1, argList2;
    argList1 << "MY_NEATO_MACRO";
    argList2 << "MY_NEATO_MACRO" << "one_arg" << "second_arg";

    func1.addArguments( argList1 );
    func2.addArguments( argList1 );
    func3.addArguments( argList2 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "good upper" ) << func1;
    QTest::newRow( "good lower" ) << func2;
    QTest::newRow( "good with args" ) << func3;
}

void CMakeAstTest::testMacroBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    MacroAst* ast = new MacroAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testMacroBadParse_data()
{
    CMakeFunctionDesc func1, func2;
    func1.name = "MACRO";
    func2.name = "wrong_function";

    QStringList argList1, argList2;
    argList2 << "MY_NEATO_MACRO" << "one_arg" << "second_arg";

    func1.addArguments( argList1 );
    func2.addArguments( argList2 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "bad no args" ) << func1;
    QTest::newRow( "bad wrong name" ) << func2;
}










void CMakeAstTest::testMakeDirectoryGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    MakeDirectoryAst* ast = new MakeDirectoryAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testMakeDirectoryGoodParse_data()
{
}

void CMakeAstTest::testMakeDirectoryBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    MakeDirectoryAst* ast = new MakeDirectoryAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testMakeDirectoryBadParse_data()
{
}










void CMakeAstTest::testMarkAsAdvancedGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    MarkAsAdvancedAst* ast = new MarkAsAdvancedAst();
    QCOMPARE( ast->parseFunctionInfo( function ), true );
    delete ast;
}

void CMakeAstTest::testMarkAsAdvancedGoodParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>("function");
    
    CMakeFunctionDesc l;
    l.name = "mark_as_advanced";
    l.addArguments(QStringList() << "FORCE" << "My_LIBRARY" << "My_INCLUDES");
    QTest::newRow("a forced mark_as_advanced") << l;
    
    l.arguments.clear();
    l.addArguments(QStringList() << "CLEAR" << "My_LIB");
    QTest::newRow("a clear mark_as_advanced") << l;
    
    l.arguments.clear();
    l.addArguments(QStringList() << "My_LIB");
    QTest::newRow("a normal mark_as_advanced") << l;
}

void CMakeAstTest::testMarkAsAdvancedBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    MarkAsAdvancedAst* ast = new MarkAsAdvancedAst();
    QCOMPARE( ast->parseFunctionInfo( function ), false );
    delete ast;
}

void CMakeAstTest::testMarkAsAdvancedBadParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>("function");
    
    CMakeFunctionDesc l;
    l.name = "mark_as_advanced";
    QTest::newRow("a mark_as_advanced without parameters") << l;
    
    l.arguments.clear();
    l.addArguments(QStringList() << "CLEAR");
    QTest::newRow("a clear mark_as_advanced without parameters") << l;
}










void CMakeAstTest::testMathGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    MathAst* ast = new MathAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testMathGoodParse_data()
{
}

void CMakeAstTest::testMathBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    MathAst* ast = new MathAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testMathBadParse_data()
{
}











void CMakeAstTest::testMessageGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    MessageAst* ast = new MessageAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testMessageGoodParse_data()
{
}

void CMakeAstTest::testMessageBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    MessageAst* ast = new MessageAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testMessageBadParse_data()
{
}










void CMakeAstTest::testOptionGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    OptionAst* ast = new OptionAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testOptionGoodParse_data()
{
}

void CMakeAstTest::testOptionBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    OptionAst* ast = new OptionAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testOptionBadParse_data()
{
}










void CMakeAstTest::testOutputRequiredFilesGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    OutputRequiredFilesAst* ast = new OutputRequiredFilesAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testOutputRequiredFilesGoodParse_data()
{
}

void CMakeAstTest::testOutputRequiredFilesBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    OutputRequiredFilesAst* ast = new OutputRequiredFilesAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testOutputRequiredFilesBadParse_data()
{
}










void CMakeAstTest::testProjectGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    ProjectAst* ast = new ProjectAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testProjectGoodParse_data()
{
    CMakeFunctionDesc func1, func2, func3, func4, func5, func6;
    func1.name = "PROJECT";
    func2.name = func3.name = func4.name = func5.name = func6.name = func1.name.toLower();

    QStringList argList1, argList2, argList3, argList4, argList5;
    argList1 << "myproject";
    argList2 << "myproject" << "C";
    argList3 << "myproject" << "CXX";
    argList4 << "myproject" << "Java";
    argList5 << "myproject" << "C" << "CXX" << "Java";

    func1.addArguments( argList1 );
    func2.addArguments( argList1 );
    func3.addArguments( argList2 );
    func4.addArguments( argList3 );
    func5.addArguments( argList4 );
    func6.addArguments( argList5 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "good 1" ) << func1;
    QTest::newRow( "good 2" ) << func2;
    QTest::newRow( "good 3" ) << func3;
    QTest::newRow( "good 4" ) << func4;
    QTest::newRow( "good 5" ) << func5;
    QTest::newRow( "good 6" ) << func6;

}

void CMakeAstTest::testProjectBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    ProjectAst* ast = new ProjectAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testProjectBadParse_data()
{
    CMakeFunctionDesc func1, func2, func3;
    func1.name = "wrongname";
    func2.name = func3.name = "PROJECT";

    QStringList argList1, argList2;
    argList1 << "myproject" << "C" << "CXX" << "Java";
    argList2 << "myproject" << "C" << "CXX" << "Java" << "foo";

    func1.addArguments( argList1 );
    func3.addArguments( argList2 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow("wrong name") << func1;
    QTest::newRow("no args") << func2;
    QTest::newRow("wrong lang args") << func3;
}










void CMakeAstTest::testQtWrapCppGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    QtWrapCppAst* ast = new QtWrapCppAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testQtWrapCppGoodParse_data()
{
}

void CMakeAstTest::testQtWrapCppBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    QtWrapCppAst* ast = new QtWrapCppAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testQtWrapCppBadParse_data()
{
}










void CMakeAstTest::testQtWrapUiGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    QtWrapUiAst* ast = new QtWrapUiAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testQtWrapUiGoodParse_data()
{
}

void CMakeAstTest::testQtWrapUiBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    QtWrapUiAst* ast = new QtWrapUiAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testQtWrapUiBadParse_data()
{
}










void CMakeAstTest::testRemoveGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    RemoveAst* ast = new RemoveAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testRemoveGoodParse_data()
{
}

void CMakeAstTest::testRemoveBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AddExecutableAst* ast = new AddExecutableAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testRemoveBadParse_data()
{
}










void CMakeAstTest::testRemoveDefinitionsGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    RemoveDefinitionsAst* ast = new RemoveDefinitionsAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testRemoveDefinitionsGoodParse_data()
{
}

void CMakeAstTest::testRemoveDefinitionsBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    RemoveDefinitionsAst* ast = new RemoveDefinitionsAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testRemoveDefinitionsBadParse_data()
{
}










void CMakeAstTest::testReturnGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    ReturnAst* ast = new ReturnAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testReturnGoodParse_data()
{
}

void CMakeAstTest::testReturnBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    ReturnAst* ast = new ReturnAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testReturnBadParse_data()
{
}










void CMakeAstTest::testSeparateArgumentsGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    SeparateArgumentsAst* ast = new SeparateArgumentsAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testSeparateArgumentsGoodParse_data()
{
}

void CMakeAstTest::testSeparateArgumentsBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    SeparateArgumentsAst* ast = new SeparateArgumentsAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testSeparateArgumentsBadParse_data()
{
}










void CMakeAstTest::testSetGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    SetAst* ast = new SetAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testSetGoodParse_data()
{
    CMakeFunctionDesc func1, func2, func3, func4;
    func1.name = "SET";
    func2.name = func3.name = func4.name = func1.name.toLower();

    QStringList argList1, argList2, argList3, argList4;
    argList1 << "MYVAR";
    argList2 << "MYVAR" << "value1";
    argList3 << "MYVAR" << "CACHE" << "FILEPATH" << "docu";
    argList4 << "MYVAR" << "value1" << "CACHE" << "STRING" << "docu" << "FORCE";

    func1.addArguments( argList1 );
    func2.addArguments( argList2 );
    func3.addArguments( argList3 );
    func4.addArguments( argList4 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "good delete value" ) << func1;
    QTest::newRow( "good set value" ) << func2;
    QTest::newRow( "good set cache value" ) << func3;
    QTest::newRow( "good set cache value forced" ) << func4;

}

void CMakeAstTest::testSetBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    SetAst* ast = new SetAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testSetBadParse_data()
{
    CMakeFunctionDesc func1, func2, func3, func4, func5;
    func1.name = "foobar_set";
    func2.name = func3.name = func4.name = func5.name = "set";

    QStringList argList1, argList2, argList3, argList4, argList5;
    argList1 << "MYVAR" << "value1" << "CACHE" << "STRING" << "docu" << "FORCE";
    argList3 << "MYVAR" << "CACHE";
    argList4 << "MYVAR" << "FORCE";
    argList5 << "MYVAR" << "CACHE" << "STRING";

    func1.addArguments( argList1 );
    func2.addArguments( argList2 );
    func3.addArguments( argList3 );
    func4.addArguments( argList4 );
    func5.addArguments( argList5 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "bad wrong name" ) << func1;
    QTest::newRow( "bad no args" ) << func2;
    QTest::newRow( "bad wrong cache use" ) << func3;
    QTest::newRow( "bad wrong force use" ) << func4;
    QTest::newRow( "bad wrong cache use 2" ) << func5;

}










void CMakeAstTest::testSetDirectoryPropsGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    SetDirectoryPropsAst* ast = new SetDirectoryPropsAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testSetDirectoryPropsGoodParse_data()
{
}

void CMakeAstTest::testSetDirectoryPropsBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    SetDirectoryPropsAst* ast = new SetDirectoryPropsAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testSetDirectoryPropsBadParse_data()
{
}










void CMakeAstTest::testSetSourceFilesPropsGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    SetSourceFilesPropsAst* ast = new SetSourceFilesPropsAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testSetSourceFilesPropsGoodParse_data()
{
}

void CMakeAstTest::testSetSourceFilesPropsBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    SetSourceFilesPropsAst* ast = new SetSourceFilesPropsAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testSetSourceFilesPropsBadParse_data()
{
}










void CMakeAstTest::testSetTargetPropsGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    SetTargetPropsAst* ast = new SetTargetPropsAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testSetTargetPropsGoodParse_data()
{
}

void CMakeAstTest::testSetTargetPropsBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    SetTargetPropsAst* ast = new SetTargetPropsAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testSetTargetPropsBadParse_data()
{
}










void CMakeAstTest::testSetTestsPropsGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    SetTestsPropsAst* ast = new SetTestsPropsAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testSetTestsPropsGoodParse_data()
{
}

void CMakeAstTest::testSetTestsPropsBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    SetTestsPropsAst* ast = new SetTestsPropsAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testSetTestsPropsBadParse_data()
{
}










void CMakeAstTest::testSiteNameGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    SiteNameAst* ast = new SiteNameAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testSiteNameGoodParse_data()
{
}

void CMakeAstTest::testSiteNameBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    SiteNameAst* ast = new SiteNameAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testSiteNameBadParse_data()
{
}










void CMakeAstTest::testSourceGroupGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    SourceGroupAst* ast = new SourceGroupAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testSourceGroupGoodParse_data()
{
}

void CMakeAstTest::testSourceGroupBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    SourceGroupAst* ast = new SourceGroupAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testSourceGroupBadParse_data()
{
}










void CMakeAstTest::testStringGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    StringAst* ast = new StringAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testStringGoodParse_data()
{
}

void CMakeAstTest::testStringBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    StringAst* ast = new StringAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testStringBadParse_data()
{
}










void CMakeAstTest::testSubdirDependsGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    SubdirDependsAst* ast = new SubdirDependsAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testSubdirDependsGoodParse_data()
{
}

void CMakeAstTest::testSubdirDependsBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    SubdirDependsAst* ast = new SubdirDependsAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testSubdirDependsBadParse_data()
{
}










void CMakeAstTest::testSubdirsGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    SubdirsAst* ast = new SubdirsAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testSubdirsGoodParse_data()
{
}

void CMakeAstTest::testSubdirsBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    SubdirsAst* ast = new SubdirsAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testSubdirsBadParse_data()
{
}










void CMakeAstTest::testTargetLinkLibrariesGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    TargetLinkLibrariesAst* ast = new TargetLinkLibrariesAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testTargetLinkLibrariesGoodParse_data()
{
    CMakeFunctionDesc func1, func2, func3;
    func1.name = "TARGET_LINK_LIBRARIES";
    func2.name = func3.name = func1.name.toLower();

    QStringList argList1, argList2, argList3;

    argList1 << "MYTARGET" << "SOME_VAR";
    argList2 << "MYTARGET" << "debug" << "onlydebuglib";
    argList3 << "MYTARGET" << "optimized" << "onlyoptimizedlib";

    func1.addArguments( argList1 );
    func2.addArguments( argList2 );
    func3.addArguments( argList3 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "whatever" ) << func1;
    QTest::newRow( "whatever" ) << func2;
    QTest::newRow( "whatever" ) << func3;

}

void CMakeAstTest::testTargetLinkLibrariesBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    TargetLinkLibrariesAst* ast = new TargetLinkLibrariesAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testTargetLinkLibrariesBadParse_data()
{
    CMakeFunctionDesc func1, func2, func3, func4;
    func1.name = "wrong_func_name";
    func2.name = func3.name, func4.name = "target_link_libraries";

    QStringList argList1, argList2, argList3, argList4;

    argList1 << "MYTARGET" << "SOME_VAR";
    argList2 << "MYTARGET";
    argList3 << "MYTARGET" << "optimized";
    argList4 << "MYTARGET" << "debug";

    func1.addArguments( argList1 );
    func2.addArguments( argList2 );
    func3.addArguments( argList3 );
    func4.addArguments( argList4 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "whatever" ) << func1;
    QTest::newRow( "whatever" ) << func2;
    QTest::newRow( "whatever" ) << func3;
    QTest::newRow( "whatever" ) << func4;

}




void CMakeAstTest::testTryCompileGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    TryCompileAst* ast = new TryCompileAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testTryCompileGoodParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>("function");

    CMakeFunctionDesc l;
    l.name = "try_compile";
    l.addArguments(QStringList() << "RESULT" << "mySrcDir" << "myBinDir" << "myProjectName");
    QTest::newRow("cmake way") << l;

    l.arguments.clear();
    l.addArguments(QStringList() << "RESULT" << "mySrcDir" << "myBinDir");
    QTest::newRow("compile way") << l;

    l.arguments.clear();
    l.addArguments(QStringList() << "RESULT" << "mySrcDir" << "myBinDir" << "COMPILE_DEFINITIONS" << "-D LOL");
    QTest::newRow("more complex compile") << l;
}

void CMakeAstTest::testTryCompileBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    TryCompileAst* ast = new TryCompileAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testTryCompileBadParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>("function");

    CMakeFunctionDesc l;
    l.name = "try_compile";
    QTest::newRow("no parameters") << l;

}










void CMakeAstTest::testTryRunGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    TryRunAst* ast = new TryRunAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testTryRunGoodParse_data()
{
}

void CMakeAstTest::testTryRunBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    TryRunAst* ast = new TryRunAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testTryRunBadParse_data()
{
}










void CMakeAstTest::testUseMangledMesaGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    UseMangledMesaAst* ast = new UseMangledMesaAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testUseMangledMesaGoodParse_data()
{
}

void CMakeAstTest::testUseMangledMesaBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    UseMangledMesaAst* ast = new UseMangledMesaAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testUseMangledMesaBadParse_data()
{
}










void CMakeAstTest::testUtilitySourceGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    UtilitySourceAst* ast = new UtilitySourceAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testUtilitySourceGoodParse_data()
{
}

void CMakeAstTest::testUtilitySourceBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    UtilitySourceAst* ast = new UtilitySourceAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testUtilitySourceBadParse_data()
{
}










void CMakeAstTest::testVariableRequiresGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    VariableRequiresAst* ast = new VariableRequiresAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testVariableRequiresGoodParse_data()
{
}

void CMakeAstTest::testVariableRequiresBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    VariableRequiresAst* ast = new VariableRequiresAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testVariableRequiresBadParse_data()
{
}










void CMakeAstTest::testVtkMakeInstantiatorGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    VtkMakeInstantiatorAst* ast = new VtkMakeInstantiatorAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testVtkMakeInstantiatorGoodParse_data()
{
}

void CMakeAstTest::testVtkMakeInstantiatorBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    VtkMakeInstantiatorAst* ast = new VtkMakeInstantiatorAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testVtkMakeInstantiatorBadParse_data()
{
}










void CMakeAstTest::testVtkWrapJavaGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    VtkWrapJavaAst* ast = new VtkWrapJavaAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testVtkWrapJavaGoodParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>("function");

    const int NUM_TESTDATA = 2;
    CMakeFunctionDesc funcs[NUM_TESTDATA];
    QStringList args[NUM_TESTDATA];

    for ( int i = 0; i < NUM_TESTDATA; i++ )
        funcs[i].name = "VTK_WRAP_JAVA";

    //test lowercase too
    funcs[NUM_TESTDATA - 1].name = funcs[NUM_TESTDATA - 1].name.toLower();

    args[0] << "myLibrary" << "MySourceList" << "source1";
    args[1] << "myLibrary" << "MySourceList" << "source1" << "source2";

    QTest::addColumn<CMakeFunctionDesc>( "function" );

    for ( int i = 0; i < NUM_TESTDATA; i++)
    {
        funcs[i].addArguments( args[i] );
        QTest::newRow( qPrintable(QString::number(i)) ) << funcs[i];
    }
}

void CMakeAstTest::testVtkWrapJavaBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    VtkWrapJavaAst* ast = new VtkWrapJavaAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testVtkWrapJavaBadParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>("function");

    const int NUM_TESTDATA = 3;
    CMakeFunctionDesc funcs[NUM_TESTDATA];
    QStringList args[NUM_TESTDATA];

    for ( int i = 0; i < NUM_TESTDATA; i++ )
        funcs[i].name = "VTK_WRAP_JAVA";

    funcs[NUM_TESTDATA - 1].name = "wrong_name";

    args[0] << "myLibrary"; //no source list and no sources
    args[1] << "myLibrary" << "MySourceList"; //source list but no sources
    args[NUM_TESTDATA - 1] << "myLibrary" << "MySourceList" << "source1";

    QTest::addColumn<CMakeFunctionDesc>( "function" );

    for ( int i = 0; i < NUM_TESTDATA; i++)
    {
        funcs[i].addArguments( args[i] );
        QTest::newRow( qPrintable(QString::number(i)) ) << funcs[i];
    }
}










void CMakeAstTest::testVtkWrapPythonGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    VtkWrapPythonAst* ast = new VtkWrapPythonAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testVtkWrapPythonGoodParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>("function");

    const int NUM_TESTDATA = 2;
    CMakeFunctionDesc funcs[NUM_TESTDATA];
    QStringList args[NUM_TESTDATA];

    for ( int i = 0; i < NUM_TESTDATA; i++ )
        funcs[i].name = "VTK_WRAP_PYTHON";

    //test lowercase too
    funcs[NUM_TESTDATA - 1].name = funcs[NUM_TESTDATA - 1].name.toLower();

    args[0] << "myLibrary" << "MySourceList" << "source1";
    args[1] << "myLibrary" << "MySourceList" << "source1" << "source2";

    QTest::addColumn<CMakeFunctionDesc>( "function" );

    for ( int i = 0; i < NUM_TESTDATA; i++)
    {
        funcs[i].addArguments( args[i] );
        QTest::newRow( qPrintable(QString::number(i)) ) << funcs[i];
    }
}

void CMakeAstTest::testVtkWrapPythonBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    VtkWrapPythonAst* ast = new VtkWrapPythonAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testVtkWrapPythonBadParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>("function");

    const int NUM_TESTDATA = 3;
    CMakeFunctionDesc funcs[NUM_TESTDATA];
    QStringList args[NUM_TESTDATA];

    for ( int i = 0; i < NUM_TESTDATA; i++ )
        funcs[i].name = "VTK_WRAP_PYTHON";

    funcs[NUM_TESTDATA - 1].name = "wrong_name";

    args[0] << "myLibrary"; //no source list and no sources
    args[1] << "myLibrary" << "MySourceList"; //source list but no sources
    args[NUM_TESTDATA - 1] << "myLibrary" << "MySourceList" << "source1";

    QTest::addColumn<CMakeFunctionDesc>( "function" );

    for ( int i = 0; i < NUM_TESTDATA; i++)
    {
        funcs[i].addArguments( args[i] );
        QTest::newRow( qPrintable(QString::number(i)) ) << funcs[i];
    }
}










void CMakeAstTest::testVtkWrapTclGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    VtkWrapTclAst* ast = new VtkWrapTclAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testVtkWrapTclGoodParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>("function");

    const int NUM_TESTDATA = 6;
    CMakeFunctionDesc funcs[NUM_TESTDATA];
    QStringList args[NUM_TESTDATA];

    for ( int i = 0; i < NUM_TESTDATA; i++ )
        funcs[i].name = "VTK_WRAP_TCL";

    //test lowercase too
    funcs[NUM_TESTDATA - 1].name = funcs[NUM_TESTDATA - 1].name.toLower();

    args[0] << "myLibrary" << "MySourceList" << "source1.tcl";
    
    //SOURCES is optional
    args[1] << "myLibrary" << "SOURCES" << "MySourceList" << "source1.tcl";
    
    //multiple sources
    args[2] << "myLibrary" << "SOURCES" << "MySourceList" << "source1.tcl"
            << "source2.tcl";

    //commands are optional
    args[3] << "myLibrary" << "MySourceList" << "source1.tcl"
            << "COMMANDS" << "com1";

    //multiple commands
    args[4] << "myLibrary" << "MySourceList" << "source1.tcl"
            << "COMMANDS" << "com1" << "com2";
            
    //line from the cmake example
    args[NUM_TESTDATA - 1] << "myLibrary" << "SOURCES"
            << "MySourceList" << "class1" << "class2"
            << "COMMANDS" << "first_command" << "second_command";
    
    QTest::addColumn<CMakeFunctionDesc>( "function" );

    for ( int i = 0; i < NUM_TESTDATA; i++)
    {
        funcs[i].addArguments( args[i] );
        QTest::newRow( qPrintable(QString::number(i)) ) << funcs[i];
    }
}

void CMakeAstTest::testVtkWrapTclBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    VtkWrapTclAst* ast = new VtkWrapTclAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testVtkWrapTclBadParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>("function");

    const int NUM_TESTDATA = 5;
    CMakeFunctionDesc funcs[NUM_TESTDATA];
    QStringList args[NUM_TESTDATA];

    for ( int i = 0; i < NUM_TESTDATA; i++ )
        funcs[i].name = "VTK_WRAP_TCL";

    funcs[NUM_TESTDATA - 1].name = "wrong_name";

    args[0] << "myLibrary"; //no source list and no sources
    args[1] << "myLibrary" << "MySourceList"; //source list but no sources
    args[2] << "myLibrary" << "SOURCES" << "MySourceList"; //source list but no sources
    args[3] << "myLibrary" << "SOURCES" << "MySourceList" << "foo.tcl";
    args[NUM_TESTDATA - 1] << "myLibrary" << "SOURCES"
            << "MySourceList" << "class1" << "class2"
            << "COMMANDS" << "first_command";
    
    QTest::addColumn<CMakeFunctionDesc>( "function" );

    for ( int i = 0; i < NUM_TESTDATA; i++)
    {
        funcs[i].addArguments( args[i] );
        QTest::newRow( qPrintable(QString::number(i)) ) << funcs[i];
    }
}










void CMakeAstTest::testWhileGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    WhileAst* ast = new WhileAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testWhileGoodParse_data()
{
}

void CMakeAstTest::testWhileBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    WhileAst* ast = new WhileAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testWhileBadParse_data()
{
}


void CMakeAstTest::testWriteFileGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    WriteFileAst* ast = new WriteFileAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testWriteFileGoodParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>("function");

    const int NUM_TESTDATA = 2;
    CMakeFunctionDesc funcs[NUM_TESTDATA];
    QStringList args[NUM_TESTDATA];

    for ( int i = 0; i < NUM_TESTDATA; i++ )
    {
        funcs[i].name = "WRITE_FILE";
        if ( i % 2 == 0 )
            funcs[i].name.toLower();
    }
    
    args[0] << "myfile.txt" << "\"this is my message\"";
    args[1] << "myfile.txt" << "\"this is also my message\"" << "APPEND";
    
    QTest::addColumn<CMakeFunctionDesc>( "function" );

    for ( int i = 0; i < NUM_TESTDATA; i++)
    {
        funcs[i].addArguments( args[i] );
        QTest::newRow( qPrintable(QString::number(i)) ) << funcs[i];
    }
}

void CMakeAstTest::testWriteFileBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    WriteFileAst* ast = new WriteFileAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testWriteFileBadParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>("function");

    const int NUM_TESTDATA = 3;
    CMakeFunctionDesc funcs[NUM_TESTDATA];
    QStringList args[NUM_TESTDATA];

    for ( int i = 0; i < NUM_TESTDATA; i++ )
        funcs[i].name = "WRITE_FILE";

    funcs[NUM_TESTDATA - 1].name = "wrong_name";
    
    args[0] << "myfile.txt";
    args[1] << "myfile.txt" << "APPEND"; //append but no message
    args[NUM_TESTDATA - 1] << "myfile.txt" << "\"this is also my message\"" << "APPEND";
    
    QTest::addColumn<CMakeFunctionDesc>( "function" );

    for ( int i = 0; i < NUM_TESTDATA; i++)
    {
        funcs[i].addArguments( args[i] );
        QTest::newRow( qPrintable(QString::number(i)) ) << funcs[i];
    }
}


