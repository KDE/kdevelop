#include "cmakeasttest.h"

#include "cmakeast.h"
#include "cmakelistsparser.h"
#include <astfactory.h>

QTEST_MAIN( CMakeAstTest )

void CMakeAstTest::testAddDefinitionsGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("add_definitions");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testAddDefinitionsGoodParse_data()
{
    CMakeFunctionDesc func;
    func.name = "add_definitions";
    QStringList argList;
    argList << "-DFOOBAR" << "-DQT_NO_STL";
    func.addArguments( argList );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "simple" ) << func;
}

void CMakeAstTest::testAddDefinitionsBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("add_definitions");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testAddDefinitionsBadParse_data()
{
    CMakeFunctionDesc func;
    func.name = "add_definition";
    QStringList argList;
    argList << "-DFOOBAR" << "-DQT_NO_STL";
    func.addArguments( argList );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "simple - bad" ) << func;

}

void CMakeAstTest::testAddDependenciesGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("add_dependencies");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;

}

void CMakeAstTest::testAddDependenciesGoodParse_data()
{
    CMakeFunctionDesc func;
    func.name = "add_dependencies";
    QStringList argList;
    argList << "target-name" << "dep1" << "dep2";
    func.addArguments( argList );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "simple" ) << func;
}

void CMakeAstTest::testAddDependenciesBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("add_dependencies");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testAddDependenciesBadParse_data()
{
    CMakeFunctionDesc func;
    func.name = "add_dependencies";
    QStringList argList;

    CMakeFunctionDesc func2;
    func2.name = "add_dependencies";
    QStringList argList2;
    argList2 << "target";
    func2.addArguments( argList2 );

    CMakeFunctionDesc func3;
    func3.name = "foobar";
    QStringList argList3;
    argList3 << "target" << "dep1" << "dep2";
    func3.addArguments( argList3 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "no args" ) << func;
    QTest::newRow( "one arg" ) << func2;
    QTest::newRow( "two args. wrong name" ) << func3;
}

void CMakeAstTest::testAddExecutableGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("add_executable");
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
    
    CMakeFunctionDesc func6;
    func6.name = "add_executable";
    QStringList argList6=QString("KDE4__kconfig_compiler IMPORTED").split(' ');
    func6.addArguments( argList6 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "only sources" ) << func;
    QTest::newRow( "win 32 app" ) << func2;
    QTest::newRow( "mac os bundle" ) << func3;
    QTest::newRow( "exclude from all" ) << func4;
    QTest::newRow( "all" ) << func5;
    QTest::newRow( "imported" ) << func6;
}

void CMakeAstTest::testAddExecutableBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("add_executable");
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

    CMakeFunctionDesc func4;
    func4.name = "add_executable";
    QStringList argList4;
    argList4 << "foo" << "WIN32";
    func4.addArguments( argList4 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "no sources" ) << func;
    QTest::newRow( "wrong name" ) << func2;
    QTest::newRow( "no arguments" ) << func3;
    QTest::newRow( "flags but no sources" ) << func4;

}

void CMakeAstTest::testAddLibraryGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("add_library");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testAddLibraryGoodParse_data()
{
    CMakeFunctionDesc func, func2, func3, func4, func5;
    QStringList argList, argList2, argList3, argList4;

    func.name = func2.name = func3.name = func4.name = func5.name =  "add_library";
    argList << "foo" << "a.cpp";
    func.addArguments( argList );

    argList2 << "foo2" << "SHARED" << "a.cpp";
    func2.addArguments( argList2 );

    argList3 << "foo3" << "EXCLUDE_FROM_ALL" << "a.cpp";
    func3.addArguments( argList3 );

    argList4 << "foo4" << "MODULE" << "EXCLUDE_FROM_ALL" << "foo.c" << "bar.c" << "baz.c";
    func4.addArguments( argList4 );
    
    func5.addArguments( QString("krossui SHARED IMPORTED").split(' ') );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "simple" ) << func;
    QTest::newRow( "shared" ) << func2;
    QTest::newRow( "exclude" ) << func3;
    QTest::newRow( "full" ) << func4;
    QTest::newRow( "imported" ) << func5;
    QTest::newRow( "alias" ) << CMakeFunctionDesc("add_library", QString("A ALIAS B").split(' '));
}

void CMakeAstTest::testAddLibraryBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("add_library");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testAddLibraryBadParse_data()
{
    CMakeFunctionDesc func, func2, func3;
    QStringList argList, argList2, argList3;

    func.name = func2.name = func3.name = "add_library";
    func.name = "wrong_name";
    argList << "foo" << "a.cpp";
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
    CMakeAst* ast = AstFactory::self()->createAst("add_subdirectory");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testAddSubdirectoryGoodParse_data()
{
    CMakeFunctionDesc func, func2, func3, func4;
    func.name = "add_subdirectory";
    func.addArguments( QStringList( "foodir" ) );

    func2.name = "add_subdirectory";
    func2.addArguments( QStringList( "foodir" ) );

    func3.name = "add_subdirectory";
    QStringList argList3;
    argList3 << "foodir" << "binary_foo_dir" << "EXCLUDE_FROM_ALL";
    func3.addArguments( argList3 );

    func4.name = "add_subdirectory";
    QStringList argList4;
    argList4 << "foodir" << "binary_foo_dir";
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
    CMakeAst* ast = AstFactory::self()->createAst("add_subdirectory");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testAddSubdirectoryBadParse_data()
{
    CMakeFunctionDesc func, func2, func3;
    func.name = func3.name = "ADD_SUBDIRECTORY";
    func2.name = "foobar";

    func2.addArguments( QStringList() << "foodir" );
    func3.addArguments( QStringList() << "srcdir" << "bindir" << "spuriousdir" );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "no arguments" ) << func;
    QTest::newRow( "bad func name" ) << func2;
    QTest::newRow( "too many arguments" ) << func3;
}

void CMakeAstTest::testAddTestGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    QFETCH( QStringList, args );
    CMakeAst* ast = AstFactory::self()->createAst("add_test");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    QCOMPARE( ((AddTestAst*)ast)->testName(), QString("test_name") );
    QCOMPARE( ((AddTestAst*)ast)->exeName(), QString("exec_name") );
    QCOMPARE( ((AddTestAst*)ast)->testArgs(), args );
    delete ast;
}

void CMakeAstTest::testAddTestGoodParse_data()
{
    CMakeFunctionDesc func1, func2, func3, func4;
    func1.name = "add_test";
    func2.name = "add_test";
    func3.name = "add_test";
    func4.name = "add_test";

    QStringList argList1, argList2, argList3, argList4;
    argList1 << "test_name" << "exec_name";
    argList2 << "test_name" << "exec_name" << "arg1";
    argList3 << "NAME" << "test_name" << "COMMAND" << "exec_name";
    argList4 << "NAME" << "test_name" << "CONFIGURATIONS" << "Debug" << "WORKING_DIRECTORY" << "/home/user" << "COMMAND" << "exec_name" << "arg1";

    func1.addArguments( argList1 );
    func2.addArguments( argList2 );
    func3.addArguments( argList3 );
    func4.addArguments( argList4 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::addColumn<QStringList>( "args" );
    QTest::newRow( "good req args" ) << func1 << QStringList();
    QTest::newRow( "good opt args" ) << func2 << (QStringList() << "arg1");
    QTest::newRow( "good extended req args" ) << func3 << QStringList();
    QTest::newRow( "good extended opt args" ) << func4 << (QStringList() << "arg1");
}

void CMakeAstTest::testAddTestBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("add_test");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testAddTestBadParse_data()
{
    CMakeFunctionDesc func1, func2, func3, func4;
    func1.name = "wrong_name";
    func2.name = func3.name = func4.name = "add_test";

    QStringList argList1, argList2, argList3, argList4;
    argList1 << "some" << "args";
    argList2 << "one arg";
    argList4 << "NAME" << "test_name" << "CONFIGURATIONS" << "Debug" << "WORKING_DIRECTORY" << "/home/user";

    func1.addArguments( argList1 );
    func2.addArguments( argList2 );
    func3.addArguments( argList3 );
    func4.addArguments( argList4 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "wrong name" ) << func1;
    QTest::newRow( "not enough args" ) << func2;
    QTest::newRow( "no args" ) << func3;
    QTest::newRow( "no command" ) << func4;

}

void CMakeAstTest::testAuxSourceDirectoryGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("aux_source_directory");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testAuxSourceDirectoryGoodParse_data()
{
    CMakeFunctionDesc func1;
    func1.name = "aux_source_directory";

    QStringList argList;
    argList << "foo1" << "foo2";

    func1.addArguments( argList );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "good uppercase" ) << func1;
}

void CMakeAstTest::testAuxSourceDirectoryBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("aux_source_directory");
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
    CMakeAst* ast = AstFactory::self()->createAst("break");
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
    CMakeAst* ast = AstFactory::self()->createAst("break");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testBreakBadParse_data()
{
    CMakeFunctionDesc func1;
    func1.name = "wrong name";

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "bad wrong name" ) << func1;
}

void CMakeAstTest::testBuildCommandGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("build_command");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testBuildCommandGoodParse_data()
{
    CMakeFunctionDesc func1, func2;
    func2.name = func1.name = "build_command";

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
    CMakeAst* ast = AstFactory::self()->createAst("build_command");
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
    CMakeAst* ast = AstFactory::self()->createAst("build_name");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testBuildNameGoodParse_data()
{
    CMakeFunctionDesc func1, func2;
    func1.name = "build_name";
    func2.name = func1.name;

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
    CMakeAst* ast = AstFactory::self()->createAst("build_name");
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

Q_DECLARE_METATYPE(QList<int>);
void CMakeAstTest::testCMakeMinimumRequiredGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("cmake_minimum_required");
    QVERIFY( ast->parseFunctionInfo( function ) == true );

    CMakeMinimumRequiredAst* minimumRequiredAst = static_cast<CMakeMinimumRequiredAst*>(ast);

    QFETCH(QList<int>, version);
    QFETCH(bool, fatal);

    QCOMPARE(minimumRequiredAst->version(), version);
    QCOMPARE(minimumRequiredAst->wrongVersionIsFatal(), fatal);

    delete ast;
}

void CMakeAstTest::testCMakeMinimumRequiredGoodParse_data()
{
    CMakeFunctionDesc func1, func2, func3, func4;
    func2.name = func3.name = func4.name = func1.name = "cmake_minimum_required";
    QStringList argList1, argList2, argList3, argList4;

    argList1 << "VERSION" << "2.4";
    argList2 = argList1;
    argList2 << "FATAL_ERROR";
    argList4 << "VERSION" << "2.6.3";

    func1.addArguments( argList1 );
    func2.addArguments( argList1 );
    func3.addArguments( argList2 );
    func4.addArguments( argList4 );

    QTest::addColumn<CMakeFunctionDesc>("function");
    QTest::addColumn<QList<int> >("version");
    QTest::addColumn<bool>("fatal");
    QTest::newRow( "good upper case" ) << func1 << (QList<int>() << 2 << 4) << false;
    QTest::newRow( "good lower case" ) << func2 << (QList<int>() << 2 << 4) << false;
    QTest::newRow( "good all args" ) << func3 << (QList<int>() << 2 << 4) << true;
    QTest::newRow( "good three components" ) << func4 << (QList<int>() << 2 << 6 << 3) << false;
}

void CMakeAstTest::testCMakeMinimumRequiredBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("cmake_minimum_required");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testCMakeMinimumRequiredBadParse_data()
{
    CMakeFunctionDesc func1, func2, func3, func4, func5;
    func1.name = "wrong_name";
    func2.name = func3.name = func4.name = func5.name = "cmake_minimum_required";
    QStringList argList1, argList2, argList3, argList5;

    argList1 << "VERSION" << "2.4";
    argList2 << "VERSION";
    argList3 << "VERSION" << "FATAL_ERROR";
    argList5 << "VERSION" << "2.4" << "JUNK";

    func1.addArguments( argList1 );
    func2.addArguments( argList2 );
    func3.addArguments( argList3 );
    func5.addArguments( argList5 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "wrong name" ) << func1;
    QTest::newRow( "no version number 1" ) << func2;
    QTest::newRow( "no version number 2" ) << func3;
    QTest::newRow( "no arguments" ) << func4;
    QTest::newRow( "invalid third arg" ) << func5;
}

void CMakeAstTest::testCMakePolicyGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("cmake_policy");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testCMakePolicyGoodParse_data()
{
    CMakeFunctionDesc func[4];
    func[1].name = func[2].name = func[3].name = func[0].name = "cmake_policy";
    QStringList argList[4];

    argList[0] << "VERSION" << "2.4";
    argList[1] << "SET" << "CMP333" << "NEW";
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
    CMakeAst* ast = AstFactory::self()->createAst("cmake_policy");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testCMakePolicyBadParse_data()
{
    CMakeFunctionDesc func[6];
    func[0].name = "iamwrong";
    func[1].name = func[2].name = func[3].name = func[4].name = func[5].name = "cmake_policy";
    QStringList argList[5];

    argList[0] << "VERSION" << "AB";
    argList[1] << "SET" << "CMP123" << "NOTNEW";
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
    CMakeAst* ast = AstFactory::self()->createAst("configure_file");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testConfigureFileGoodParse_data()
{
    CMakeFunctionDesc func1, func2, func3, func4, func5;
    func1.name = func2.name = "configure_file";
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
    CMakeAst* ast = AstFactory::self()->createAst("configure_file");
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
    func3.addArguments( argList3 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "bad wrong name" ) << func1;
    QTest::newRow( "bad only one arg" ) << func2;
    QTest::newRow( "bad no args" ) << func3;
}

void CMakeAstTest::testCustomCommandGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("add_custom_command");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testCustomCommandGoodParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>( "function" );
    CMakeFunctionDesc func;
    func.name = "add_custom_command";
    QStringList argList;
    argList << "OUTPUT" << "foo" << "COMMAND" << "bar";
    func.addArguments( argList );
    func.line = 0;

    CMakeFunctionDesc func1;
    func1.name = "add_custom_command";
    QStringList argList1;
    argList1 << "OUTPUT" << "foo" << "COMMAND" << "bar";
    argList1 << "MAIN_DEPENDENCY" << "dep1" << "DEPENDS" << "dep1" << "dep2";
    argList1 << "WORKING_DIRECTORY" << "dir1" << "COMMENT" << "some comment";
    argList1 << "VERBATIM" << "APPEND";
    func1.addArguments( argList1 );

    CMakeFunctionDesc func2;
    func2.name = "ADD_CUSTOM_COMMAND";
    QStringList argList2;
    argList2 << "OUTPUT" << "foo" << "COMMAND" << "bar" << "ARGS" << "baz";
    argList2 << "MAIN_DEPENDENCY" << "dep1" << "DEPENDS" << "dep1" << "dep2";
    argList2 << "WORKING_DIRECTORY" << "dir1" << "COMMENT" << "some comment";
    argList2 << "VERBATIM" << "APPEND";
    func2.addArguments( argList2 );

    CMakeFunctionDesc func3;
    func3.name = "ADD_CUSTOM_COMMAND";
    QStringList argList3;
    argList3 << "TARGET" << "foo" << "PRE_BUILD" << "COMMAND" << "bar";
    argList3 << "MAIN_DEPENDENCY" << "dep1" << "DEPENDS" << "dep1" << "dep2";
    argList3 << "WORKING_DIRECTORY" << "dir1" << "COMMENT" << "some comment";
    func3.addArguments( argList3 );

    QTest::newRow( "no optional" ) << func;
    QTest::newRow( "all optional" ) << func1;
    QTest::newRow( "optional with arg" ) << func2;
    QTest::newRow( "second form all optional uppercase" ) << func3;

}

void CMakeAstTest::testCustomCommandBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("add_custom_command");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testCustomCommandBadParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>( "function" );
    CMakeFunctionDesc func;
    func.name = "foo";
    func.line = 0;

    CMakeFunctionDesc func_noargs;
    func_noargs.name = "add_custom_command";

    CMakeFunctionDesc func2;
    func2.name = "add_custom_command";
    QStringList argList2;
    argList2 << "nottarget" << "foo" << "notcommand" << "foo1";
    func2.addArguments( argList2 );
    func2.line = 0;

    CMakeFunctionDesc func3("add_custom_command", QStringList("target") << "foo" << "no_pre_build" << "foo1");
    CMakeFunctionDesc func4("add_custom_command", QStringList("output") << "foo1" << "no_command" << "foo2");
    CMakeFunctionDesc func5("add_custom_command", QStringList("target") << "foo" << "PRE_BUILD" << "no_command");

    QTest::newRow( "wrong function" ) << func;
    QTest::newRow( "right function. no args" ) << func_noargs;
    QTest::newRow( "wrong params 1" ) << func2;
    QTest::newRow( "wrong params 2" ) << func3;
    QTest::newRow( "wrong params 3" ) << func4;
    QTest::newRow( "wrong params 4" ) << func5;

}

void CMakeAstTest::testCustomTargetGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("add_custom_target");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testCustomTargetGoodParse_data()
{
    CMakeFunctionDesc func1;
    func1.name = "add_custom_target";
    QStringList argList1;
    argList1 << "MyName" << "ALL" << "foobar --test" << "COMMAND"
             << "barbaz --foo" << "DEPENDS" << "dep1" << "dep2" << "dep3"
             << "WORKING_DIRECTORY" << "/path/to/my/dir" << "COMMENT"
             << "this is my comment" << "VERBATIM";
    func1.addArguments( argList1 );

    CMakeFunctionDesc func2;
    func2.name = "ADD_CUSTOM_TARGET";
    QStringList argList2;
    argList2 << "MyName" << "my_command --test-param 1";
    func2.addArguments( argList2 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "all optional" ) << func1;
    QTest::newRow( "no optional" ) << func2;
}

void CMakeAstTest::testCustomTargetBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("add_custom_target");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testCustomTargetBadParse_data()
{
    CMakeFunctionDesc func1;
    func1.name = "add_custom_target";
    QStringList argList1;
    argList1 << "IAm#1" << "ALL" << "foobar --test" << "COMMAND"
             << "barbaz --foo" << "DEPENDS" << "dep1" << "dep2" << "dep3"
             << "WORKING_DIRECTORY" << "/path/to/my/dir" << "COMMENT"
             << "this is my comment" << "VERBATIM";
    func1.addArguments( argList1 );

    CMakeFunctionDesc func2;
    func2.name = "ADD_CUSTOM_TARGET";
    QStringList argList2;
    argList2 << "ALL" << "my_command --test-param 1";
    func2.addArguments( argList2 );

    CMakeFunctionDesc func3;
    func3.name = "add_custom_target";

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "bad 1" ) << func1;
    QTest::newRow( "bad 2" ) << func2;
    QTest::newRow( "bad 3" ) << func3;
}

void CMakeAstTest::testCreateTestSourcelistGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("create_test_sourcelist");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testCreateTestSourcelistGoodParse_data()
{
    CMakeFunctionDesc func1, func2, func3, func4;
    func4.name = func3.name = func2.name = func1.name = "create_test_sourcelist";

    QStringList argList1, argList2, argList3, argList4;
    argList1 << "$(TESTDRIVER_SRCS)" << "test_driver" << "$(TEST_SRCS)";
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
    CMakeAst* ast = AstFactory::self()->createAst("create_test_sourcelist");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testCreateTestSourcelistBadParse_data()
{
    CMakeFunctionDesc func1, func2, func3, func4;
    func1.name = "create_test_sourcelists";
    func2.name = "create_test_sourcelist";
    func3.name = func2.name;
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
    CMakeAst* ast = AstFactory::self()->createAst("enable_language");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testEnableLanguageGoodParse_data()
{
    CMakeFunctionDesc func1, func2;
    func1.name = "enable_language";
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
    CMakeAst* ast = AstFactory::self()->createAst("enable_language");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testEnableLanguageBadParse_data()
{
    CMakeFunctionDesc func1, func2, func3;
    func1.name = "ENABLE_LANGUAGES";
    func2.name = func3.name = "enable_language";

    QStringList argList1, argList2, argList3;
    argList1 << "C++";
    argList3 << "C++" << "Java";

    func1.addArguments( argList1 );
    func2.addArguments( argList2 );
    func3.addArguments( argList3 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "bad func name" ) << func1;
    QTest::newRow( "no arguments" ) << func2;
    QTest::newRow( "too many arguments" ) << func3;
}

void CMakeAstTest::testEnableTestingGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("enable_testing");
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
    CMakeAst* ast = AstFactory::self()->createAst("enable_testing");
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
    CMakeAst* ast = AstFactory::self()->createAst("exec_program");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testExecProgramGoodParse_data()
{
    CMakeFunctionDesc func1, func2, func3, func4;
    func1.name = "exec_program";
    func2.name = func1.name;
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
    CMakeAst* ast = AstFactory::self()->createAst("exec_program");
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

#define TDD_TODO QSKIP("No data available yet", SkipSingle)
#define TDD_TOIMPL QSKIP("Command not implemented yet", SkipSingle)

void CMakeAstTest::testExecuteProcessGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("execute_process");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testExecuteProcessGoodParse_data()
{
}

void CMakeAstTest::testExecuteProcessBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("execute_process");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testExecuteProcessBadParse_data()
{
}

void CMakeAstTest::testExportGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("export");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}
/// @todo Test EXPORT(PACKAGE name), introduced in CMake 2.8
void CMakeAstTest::testExportGoodParse_data()
{
    const int NUM_TESTDATA = 8;
    CMakeFunctionDesc funcs[NUM_TESTDATA];

    QString args[NUM_TESTDATA];

    args[0] = "TARGETS main FILE main.cmake";
    args[1] = "TARGETS main foo FILE main.cmake";
    args[2] = "TARGETS FILE main.cmake"; // tested in cmake 2.8.1, having no targets really does work
    args[3] = "TARGETS main NAMESPACE ns FILE main.cmake";
    args[4] = "TARGETS main APPEND FILE main.cmake";
    args[5] = "TARGETS main APPEND NAMESPACE ns FILE main.cmake";
    args[6] = "TARGETS main NAMESPACE ns APPEND FILE main.cmake";
    args[7] = "TARGETS TARGETS FILE main.cmake";
    
    for (int i = 0; i < NUM_TESTDATA; ++i) {
        funcs[i].name = "EXPORT";
        funcs[i].addArguments(args[i].split(' '));
    }

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "single target" ) << funcs[0];
    QTest::newRow( "two targets" ) << funcs[1];
    QTest::newRow( "no targets" ) << funcs[2];
    QTest::newRow( "namespace" ) << funcs[3];
    QTest::newRow( "append" ) << funcs[4];
    QTest::newRow( "append and namespace" ) << funcs[5];
    QTest::newRow( "namespace and append" ) << funcs[6];
    QTest::newRow( "target called TARGETS" ) << funcs[7];
}

void CMakeAstTest::testExportBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("export");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testExportBadParse_data()
{
    const int NUM_TESTDATA = 5;
    CMakeFunctionDesc funcs[NUM_TESTDATA];

    QString args[NUM_TESTDATA];

    args[0] = "TARGETS main FILE main.cmake";
    args[1] = "CAKES main FILE main.cmake";
    args[2] = "TARGETS main";
    args[3] = "TARGETS main FILE";
    args[4] = "";

    for (int i = 0; i < NUM_TESTDATA; ++i) {
        funcs[i].name = "EXPORT";
        funcs[i].addArguments(args[i].split(' '));
    }
    funcs[0].name="exprt";

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow("bad func name") << funcs[0];
    QTest::newRow("bad subcommand") << funcs[1];
    QTest::newRow("no FILE") << funcs[2];
    QTest::newRow("nothing after FILE") << funcs[3];
    QTest::newRow("no args") << funcs[4];
}

void CMakeAstTest::testExportLibraryDepsGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("export_library_dependencies");
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
    CMakeAst* ast = AstFactory::self()->createAst("export_library_dependencies");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testExportLibraryDepsBadParse_data()
{
    CMakeFunctionDesc func1, func2;
    func1.name = "export_library_dependencies";
    func2.name = "foo_foo_bar";

    QStringList argList1, argList2;
    argList1 << "dep_file" << "lalala";
    argList2 << "dep_file";

    func1.addArguments( argList1 );
    func2.addArguments( argList2 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "bad 1" ) << func1;
    QTest::newRow( "bad 2" ) << func2;
}

void CMakeAstTest::testFileGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("file");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testFileGoodParse_data()
{
    const int NUM_TESTDATA = 13;
    CMakeFunctionDesc funcs[NUM_TESTDATA];
    QStringList args[NUM_TESTDATA];

    for ( int i = 0; i < NUM_TESTDATA; i++ )
        funcs[i].name = "file";

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
    CMakeAst* ast = AstFactory::self()->createAst("file");
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

    //recursive glob
    args[13] << "GLOB_RECURSE"; //no variable
    args[14] << "GLOB_RECURSE" << "MY_VAR" << "RELATIVE"; //no path

    args[15] << "glob_recurse" << "MY_VAR" << "*.cpp"; //uppercase required

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
        QTest::newRow( qPrintable(QString("%1. %2").arg(i).arg(args[i].isEmpty() ? "?" : args[i].first())) ) << funcs[i];
    }

}

void CMakeAstTest::testFindFileGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("find_file");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testFindFileGoodParse_data()
{

    QTest::addColumn<CMakeFunctionDesc>("function");
    
    CMakeFunctionDesc l;
    l.name = "find_file";
    l.addArguments(QString("_SOPRANO_MACRO_FILE NAMES SopranoAddOntology.cmake HINTS /home/kde-devel/kde/share/soprano/cmake").split(' '));
    QTest::newRow("find file") << l;
}

void CMakeAstTest::testFindFileBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("find_file");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testFindFileBadParse_data()
{
}

void CMakeAstTest::testFindLibraryGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("find_library");
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
    CMakeAst* ast = AstFactory::self()->createAst("find_library");
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
    CMakeAst* ast = AstFactory::self()->createAst("find_package");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testFindPackageGoodParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>("function");
    
    QTest::newRow( "complex" ) << CMakeFunctionDesc("find_package", QString("PolkitQt-1 0.99.0 QUIET NO_MODULE PATHS /home/kde-devel/kde/lib/PolkitQt-1/cmake").split(' '));
    QTest::newRow( "components" ) << CMakeFunctionDesc("find_package", QString("Qt5 5.2 CONFIG REQUIRED Concurrent Test").split(' '));
}

void CMakeAstTest::testFindPackageBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("find_package");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testFindPackageBadParse_data()
{
}

void CMakeAstTest::testFindPathGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("find_path");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testFindPathGoodParse_data()
{
}

void CMakeAstTest::testFindPathBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("find_path");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testFindPathBadParse_data()
{
}

void CMakeAstTest::testFindProgramGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("find_program");
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
    CMakeAst* ast = AstFactory::self()->createAst("find_program");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testFindProgramBadParse_data()
{
    
    QTest::addColumn<CMakeFunctionDesc>("function");
    
    CMakeFunctionDesc l;
    l.name = "";
    l.addArguments(QStringList() << "MY_VAR" << "file");
    QTest::newRow ("no function name") << l;

    l.arguments.clear();
    l.name = "find_program";
    l.addArguments(QStringList() << "MY_VAR" << "NAMES" << "PATHS" << "location1" << "location2");
    QTest::newRow("no names") << l;
}

void CMakeAstTest::testFltkWrapUiGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("fltk_wrap_ui");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testFltkWrapUiGoodParse_data()
{
}

void CMakeAstTest::testFltkWrapUiBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("fltk_wrap_ui");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testFltkWrapUiBadParse_data()
{
}

void CMakeAstTest::testForeachGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("foreach");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testForeachGoodParse_data()
{
}

void CMakeAstTest::testForeachBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("foreach");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testForeachBadParse_data()
{
}

void CMakeAstTest::testGetCMakePropertyGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("get_cmake_property");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testGetCMakePropertyGoodParse_data()
{
}

void CMakeAstTest::testGetCMakePropertyBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("get_cmake_property");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testGetCMakePropertyBadParse_data()
{
}

void CMakeAstTest::testGetDirPropertyGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("get_directory_property");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testGetDirPropertyGoodParse_data()
{
}

void CMakeAstTest::testGetDirPropertyBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("get_directory_property");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testGetDirPropertyBadParse_data()
{
}

void CMakeAstTest::testGetFilenameComponentGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("get_filename_component");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testGetFilenameComponentGoodParse_data()
{
}

void CMakeAstTest::testGetFilenameComponentBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("get_filename_component");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testGetFilenameComponentBadParse_data()
{
}

void CMakeAstTest::testGetSourceFilePropGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("get_sourcefile_property");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testGetSourceFilePropGoodParse_data()
{
}

void CMakeAstTest::testGetSourceFilePropBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("get_sourcefile_property");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testGetSourceFilePropBadParse_data()
{
}

void CMakeAstTest::testGetTargetPropGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("get_target_property");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testGetTargetPropGoodParse_data()
{
}

void CMakeAstTest::testGetTargetPropBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("get_target_property");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testGetTargetPropBadParse_data()
{
}

void CMakeAstTest::testGetTestPropGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("get_test_property");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testGetTestPropGoodParse_data()
{
}

void CMakeAstTest::testGetTestPropBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("get_test_property");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testGetTestPropBadParse_data()
{
}

void CMakeAstTest::testIfGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("if");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testIfGoodParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>("function");
    CMakeFunctionDesc if1, if2, if3;
    if1.name = if2.name = "if";
    if3.name = "IF";

    if1.addArguments(QStringList() << "TRUE");
    if2.addArguments(QStringList() << "myvar");
    if3.addArguments(QStringList() << "myvar" << "STREQUAL" << "\"foo\"");

    QTest::newRow("if true constant") << if1;
    QTest::newRow("if variable alone") << if2;
    QTest::newRow("if strequal") << if3;

    CMakeFunctionDesc else1;
    else1.name = "else";
    QTest::newRow("else no args") << else1;

    CMakeFunctionDesc elif1, elif2, elif3;
    elif1.name = elif2.name = "elseif";
    elif3.name = "ELSEIF";

    elif1.addArguments(QStringList() << "TRUE");
    elif2.addArguments(QStringList() << "myvar");
    elif3.addArguments(QStringList() << "myvar" << "STREQUAL" << "\"foo\"");

    QTest::newRow("elseif constant") << elif1;
    QTest::newRow("elseif variable alone") << elif2;
    QTest::newRow("elseif strequal") << elif3;
}

void CMakeAstTest::testIfBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("if");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testIfBadParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>("function");

    CMakeFunctionDesc badFuncName;
    badFuncName.name = "iif";
    badFuncName.addArguments(QStringList() << "myvar" << "STREQUAL" << "\"foo\"");
    QTest::newRow("bad function name") << badFuncName;

    //This is currently disabled because the parser doesn't fail on IF() with no args,
    //but official CMake doesn't either, so I don't know if it's *supposed* to fail.
    //Then again, CMake doesn't fail when you do pass arguments to ELSE() either...
    if(0) {
    CMakeFunctionDesc ifEmptyArgs;
    ifEmptyArgs.name = "if";
    QTest::newRow("if empty arguments") << ifEmptyArgs;

    CMakeFunctionDesc elifEmptyArgs;
    elifEmptyArgs.name = "elseif";
    QTest::newRow("elseif empty arguments") << elifEmptyArgs;
    }

    CMakeFunctionDesc elseWithArgs;
    elseWithArgs.name = "else";
    elseWithArgs.addArguments(QStringList() << "foo");
    QTest::newRow("else with arguments") << elseWithArgs;
}

void CMakeAstTest::testIncludeGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("include");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testIncludeGoodParse_data()
{
    CMakeFunctionDesc func1, func2, func3, func4, func5;
    func2.name = func3.name = func4.name = func5.name = func1.name = "include";

    QStringList argList1, argList2, argList3, argList4;
    argList1 << "SomeFile";
    argList2 << "SomeFile" << "OPTIONAL";
    argList3 << "SomeFile" << "RESULT_VARIABLE" << "output";
    argList4 << "SomeFile" << "OPTIONAL" << "RESULT_VARIABLE" << "output";

    func1.addArguments( argList1 );
    func2.addArguments( argList1 );
    func3.addArguments( argList2 );
    func4.addArguments( argList3 );
    func5.addArguments( argList4 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "good upper" ) << func1;
    QTest::newRow( "good lower" ) << func2;
    QTest::newRow( "good optional" ) << func3;
    QTest::newRow( "good result" ) << func4;
    QTest::newRow( "good all args" ) << func5;
}

void CMakeAstTest::testIncludeBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("include");
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
    CMakeAst* ast = AstFactory::self()->createAst("include_directories");
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
    TDD_TODO;
    
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("include_directories");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testIncludeDirectoriesBadParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>("function");
    
}

void CMakeAstTest::testIncludeExternalMsProjectGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("include_external_msproject");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testIncludeExternalMsProjectGoodParse_data()
{
}

void CMakeAstTest::testIncludeExternalMsProjectBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("include_external_msproject");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testIncludeExternalMsProjectBadParse_data()
{
}

void CMakeAstTest::testIncludeRegularExpressionGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("include_regular_expression");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testIncludeRegularExpressionGoodParse_data()
{
}

void CMakeAstTest::testIncludeRegularExpressionBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("include_regular_expression");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testIncludeRegularExpressionBadParse_data()
{
}

void CMakeAstTest::testInstallGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("install");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testInstallGoodParse_data()
{
}

void CMakeAstTest::testInstallBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("install");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testInstallBadParse_data()
{
}

void CMakeAstTest::testInstallFilesGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("install_files");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testInstallFilesGoodParse_data()
{
}

void CMakeAstTest::testInstallFilesBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("install_files");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testInstallFilesBadParse_data()
{
}

void CMakeAstTest::testInstallProgramsGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("install_programs");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testInstallProgramsGoodParse_data()
{
}

void CMakeAstTest::testInstallProgramsBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("install_programs");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testInstallProgramsBadParse_data()
{
}

void CMakeAstTest::testInstallTargetsGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("install_targets");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testInstallTargetsGoodParse_data()
{
}

void CMakeAstTest::testInstallTargetsBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("install_targets");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testInstallTargetsBadParse_data()
{
}

void CMakeAstTest::testLinkDirectoriesGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("link_directories");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testLinkDirectoriesGoodParse_data()
{
}

void CMakeAstTest::testLinkDirectoriesBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("link_directories");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testLinkDirectoriesBadParse_data()
{
}

void CMakeAstTest::testLinkLibrariesGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("link_libraries");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testLinkLibrariesGoodParse_data()
{
}

void CMakeAstTest::testLinkLibrariesBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("link_libraries");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testLinkLibrariesBadParse_data()
{
}

void CMakeAstTest::testListGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("list");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testListGoodParse_data()
{
    CMakeFunctionDesc func1;
    func1.name = "list";
    func1.addArguments(QString("APPEND _boost_TEST_VERSIONS 1.39").split(' '));

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "append" ) << func1;
}

void CMakeAstTest::testListBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("list");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testListBadParse_data()
{
}

void CMakeAstTest::testLoadCacheGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("load_cache");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testLoadCacheGoodParse_data()
{
}

void CMakeAstTest::testLoadCacheBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("load_cache");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testLoadCacheBadParse_data()
{
}

void CMakeAstTest::testLoadCommandGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("load_command");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testLoadCommandGoodParse_data()
{
}

void CMakeAstTest::testLoadCommandBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("load_command");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testLoadCommandBadParse_data()
{
}

void CMakeAstTest::testMacroGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("macro");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testMacroGoodParse_data()
{
    CMakeFunctionDesc func1, func2, func3;
    func2.name = func3.name = func1.name = "macro";

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
    CMakeAst* ast = AstFactory::self()->createAst("macro");
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

void CMakeAstTest::testFunctionGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("function");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testFunctionGoodParse_data()
{
    CMakeFunctionDesc func1, func2, func3;
    func2.name = func3.name = func1.name = "function";

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

void CMakeAstTest::testFunctionBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("function");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testFunctionBadParse_data()
{
    CMakeFunctionDesc func1, func2;
    func1.name = "FUNCTION";
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
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("make_directory");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testMakeDirectoryGoodParse_data()
{
}

void CMakeAstTest::testMakeDirectoryBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("make_directory");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testMakeDirectoryBadParse_data()
{
}

void CMakeAstTest::testMarkAsAdvancedGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("mark_as_advanced");
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
    CMakeAst* ast = AstFactory::self()->createAst("mark_as_advanced");
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
    CMakeAst* ast = AstFactory::self()->createAst("math");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testMathGoodParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>("function");

    CMakeFunctionDesc func;
    func.name = "MATH";
    func.addArguments(QStringList() << "EXPR" << "myvar" << "2+2");
    QTest::newRow("simple sum") << func;

    func.arguments.clear();
    // in a CMakeLists.txt, this would be MATH(EXPR myvar "2 + 2")
    // (with quotes around the expression)
    func.addArguments(QStringList() << "EXPR" << "myvar" << "2 +  2");
    QTest::newRow("spaces around op") << func;

    func.arguments.clear();
    func.addArguments(QStringList() << "EXPR" << "myvar" << " 2 + 2 ");
    QTest::newRow("spaces around expr") << func;

    func.name = "math";
    QTest::newRow("lowercase command") << func;
}

void CMakeAstTest::testMathBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("math");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testMathBadParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>("function");

    CMakeFunctionDesc f1;
    f1.name = "math";
    f1.addArguments(QStringList());
    QTest::newRow("no arguments") << f1;

    f1.arguments.clear();
    f1.addArguments(QStringList() << "EPXR" << "myvar" << "2+2");
    QTest::newRow("bad EXPR") << f1;

    f1.arguments.clear();
    f1.addArguments(QStringList() << "expr" << "myvar" << "2+2");
    QTest::newRow("lowercase expr") << f1;

    f1.arguments.clear();
    f1.addArguments(QStringList() << "EXPR");
    QTest::newRow("missing output var") << f1;

    f1.arguments.clear();
    f1.addArguments(QStringList() << "EXPR" << "myvar");
    QTest::newRow("missing expression") << f1;

    // in a CMakeLists.txt, this would be MATH(EXPR myvar 2 + 2)
    // (without quotes around the expression)
    f1.arguments.clear();
    f1.addArguments(QStringList() << "EXPR" << "myvar" << "2" << "+" << "2");
    QTest::newRow("multiarg expression") << f1;
}

void CMakeAstTest::testMessageGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("message");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testMessageGoodParse_data()
{
}

void CMakeAstTest::testMessageBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("message");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testMessageBadParse_data()
{
}

void CMakeAstTest::testOptionGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("option");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testOptionGoodParse_data()
{
}

void CMakeAstTest::testOptionBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("option");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testOptionBadParse_data()
{
}

void CMakeAstTest::testOutputRequiredFilesGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("output_required_files");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testOutputRequiredFilesGoodParse_data()
{
}

void CMakeAstTest::testOutputRequiredFilesBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("output_required_files");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testOutputRequiredFilesBadParse_data()
{
}

void CMakeAstTest::testProjectGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("project");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testProjectGoodParse_data()
{
    CMakeFunctionDesc func1, func2, func3, func4, func5, func6;
    func2.name = func3.name = func4.name = func5.name = func6.name = func1.name = "project";

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
    CMakeAst* ast = AstFactory::self()->createAst("project");
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
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("qt_wrap_cpp");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testQtWrapCppGoodParse_data()
{
}

void CMakeAstTest::testQtWrapCppBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("qt_wrap_cpp");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testQtWrapCppBadParse_data()
{
}

void CMakeAstTest::testQtWrapUiGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("qt_wrap_ui");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testQtWrapUiGoodParse_data()
{
}

void CMakeAstTest::testQtWrapUiBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("qt_wrap_ui");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testQtWrapUiBadParse_data()
{
}

void CMakeAstTest::testRemoveGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("remove");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testRemoveGoodParse_data()
{
}

void CMakeAstTest::testRemoveBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("remove");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testRemoveBadParse_data()
{
}

void CMakeAstTest::testRemoveDefinitionsGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("remove_definitions");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testRemoveDefinitionsGoodParse_data()
{
}

void CMakeAstTest::testRemoveDefinitionsBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("remove_definitions");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testRemoveDefinitionsBadParse_data()
{
}

void CMakeAstTest::testReturnGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("return");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testReturnGoodParse_data()
{
}

void CMakeAstTest::testReturnBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("return");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testReturnBadParse_data()
{
}

void CMakeAstTest::testSeparateArgumentsGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("separate_arguments");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testSeparateArgumentsGoodParse_data()
{
}

void CMakeAstTest::testSeparateArgumentsBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("separate_arguments");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testSeparateArgumentsBadParse_data()
{
}

void CMakeAstTest::testSetGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("set");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testSetGoodParse_data()
{
    CMakeFunctionDesc func1, func2, func3, func4, func5;
    func2.name = func3.name = func4.name = func5.name = func1.name = "set";

    QStringList argList1, argList2, argList3, argList4, argList5;
    argList1 << "MYVAR";
    argList2 << "MYVAR" << "value1";
    argList3 << "MYVAR" << "CACHE" << "FILEPATH" << "docu";
    argList4 << "MYVAR" << "value1" << "CACHE" << "STRING" << "docu" << "FORCE";
    argList5 << "MYVAR" << "value1" << "PARENT_SCOPE";

    func1.addArguments( argList1 );
    func2.addArguments( argList2 );
    func3.addArguments( argList3 );
    func4.addArguments( argList4 );
    func5.addArguments( argList5 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "good delete value" ) << func1;
    QTest::newRow( "good set value" ) << func2;
    QTest::newRow( "good set cache value" ) << func3;
    QTest::newRow( "good set cache value forced" ) << func4;
    QTest::newRow( "good set parent_scope" ) << func5;

}

void CMakeAstTest::testSetBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("set");
    QCOMPARE( ast->parseFunctionInfo( function ), false );
    delete ast;
}

void CMakeAstTest::testSetBadParse_data()
{
    CMakeFunctionDesc func1, func2, func3, func4, func5, func6;
    func1.name = "foobar_set";
    func2.name = func3.name = func4.name = func5.name = func6.name = "set";

    QStringList argList1, argList2, argList3, argList4, argList5, argList6;
    argList1 << "MYVAR" << "value1" << "CACHE" << "STRING" << "docu" << "FORCE";
    argList3 << "MYVAR" << "CACHE";
    argList4 << "MYVAR" << "FORCE";
    argList5 << "MYVAR" << "CACHE" << "STRING";
    argList6 << "MYVAR" << "value1" << "CACHE" << "STRING" << "docu" << "PARENT_SCOPE";
    
    func1.addArguments( argList1 );
//     func2.addArguments( argList2 );
    func3.addArguments( argList3 );
    func4.addArguments( argList4 );
    func5.addArguments( argList5 );
    func6.addArguments( argList6 );
    
    qDebug() << func2.arguments.count();

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "bad wrong name" ) << func1;
    QTest::newRow( "bad no args" ) << func2;
    QTest::newRow( "bad wrong cache use" ) << func3;
    QTest::newRow( "bad wrong force use" ) << func4;
    QTest::newRow( "bad wrong cache use 2" ) << func5;
    QTest::newRow( "bad cache + parent_scope" ) << func6;

}

void CMakeAstTest::testSetPropertyGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("set_property");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testSetPropertyGoodParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>( "function" );

    {
        CMakeFunctionDesc func;
        func.name = "set_property";
        func.addArguments( QString("GLOBAL PROPERTY SIMPLE_PROPERTY_NOVALUE").split(" ") );
        QTest::newRow( "no value" ) << func;
    }

    {
        CMakeFunctionDesc func;
        func.name = "set_property";
        func.addArguments( QString("GLOBAL APPEND foo bar").split(" ") );
        QTest::newRow( "append" ) << func;
    }

    {
        CMakeFunctionDesc func;
        func.name = "set_property";
        func.addArguments( QString("GLOBAL APPEND_STRING foo bar").split(" ") );
        QTest::newRow( "append_string" ) << func;
    }
}

void CMakeAstTest::testSetPropertyBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("set_property");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testSetPropertyBadParse_data()
{
}

void CMakeAstTest::testGetPropertyGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("get_property");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testGetPropertyGoodParse_data()
{
    CMakeFunctionDesc func1;
    func1.name = "get_property";
    func1.addArguments( QString("_CTEST_TARGETS_ADDED GLOBAL PROPERTY CTEST_TARGETS_ADDED").split(" ") );

    CMakeFunctionDesc funcCache;
    funcCache.name = "get_property";
    funcCache.addArguments(QString("_type CACHE BOOSTROOT PROPERTY TYPE").split(" "));

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "good get" ) << func1;
    QTest::newRow( "CACHE" ) << funcCache;
}

void CMakeAstTest::testGetPropertyBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("get_property");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testGetPropertyBadParse_data()
{
}

void CMakeAstTest::testSetDirectoryPropsGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("set_directory_properties");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testSetDirectoryPropsGoodParse_data()
{
}

void CMakeAstTest::testSetDirectoryPropsBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("set_directory_properties");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testSetDirectoryPropsBadParse_data()
{
}

void CMakeAstTest::testSetSourceFilesPropsGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("set_source_files_properties");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testSetSourceFilesPropsGoodParse_data()
{
}

void CMakeAstTest::testSetSourceFilesPropsBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("set_source_files_properties");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testSetSourceFilesPropsBadParse_data()
{
}

void CMakeAstTest::testSetTargetPropsGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("set_target_properties");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testSetTargetPropsGoodParse_data()
{
}

void CMakeAstTest::testSetTargetPropsBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("set_target_properties");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testSetTargetPropsBadParse_data()
{
}

void CMakeAstTest::testSetTestsPropsGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("set_tests_properties");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testSetTestsPropsGoodParse_data()
{
}

void CMakeAstTest::testSetTestsPropsBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("set_tests_properties");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testSetTestsPropsBadParse_data()
{
}

void CMakeAstTest::testSiteNameGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("site_name");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testSiteNameGoodParse_data()
{
}

void CMakeAstTest::testSiteNameBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("site_name");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testSiteNameBadParse_data()
{
}

void CMakeAstTest::testSourceGroupGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("source_group");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testSourceGroupGoodParse_data()
{
}

void CMakeAstTest::testSourceGroupBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("source_group");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testSourceGroupBadParse_data()
{
}

void CMakeAstTest::testStringGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("string");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testStringGoodParse_data()
{
    QStringList cases;
    cases << "string STRIP string output";
    cases << "string RANDOM output";
    cases << "stRing RANDOM LENGTH 3 output";
    cases << "string RANDOM ALPHABET abcd output";
    cases << "string RANDOM output LENGTH 3 ALPHABET abcd";
    QTest::addColumn<CMakeFunctionDesc>( "function" );
    
    foreach(const QString& acase, cases)
    {
        QStringList splitted = acase.split(" ");
        CMakeFunctionDesc func(splitted.takeFirst(), splitted);
        QTest::newRow( qPrintable(acase) ) << func;
    }
}

void CMakeAstTest::testStringBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("string");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testStringBadParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "few arguments" ) << CMakeFunctionDesc("string", QString("TOLOWER PROJECT_PRODUCT_DIR").split(" "));
}

void CMakeAstTest::testSubdirDependsGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("subdir_depends");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testSubdirDependsGoodParse_data()
{
}

void CMakeAstTest::testSubdirDependsBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("subdir_depends");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testSubdirDependsBadParse_data()
{
}

void CMakeAstTest::testSubdirsGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("subdirs");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testSubdirsGoodParse_data()
{
}

void CMakeAstTest::testSubdirsBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("subdirs");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testSubdirsBadParse_data()
{
}

void CMakeAstTest::testTargetLinkLibrariesGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("target_link_libraries");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testTargetLinkLibrariesGoodParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "simple" ) << CMakeFunctionDesc("target_link_libraries", QStringList("MYTARGET") << "somelib");
    QTest::newRow( "debug only" ) << CMakeFunctionDesc("target_link_libraries", QStringList("MYTARGET") << "debug" << "onlydebuglib");
    QTest::newRow( "optimized only" ) << CMakeFunctionDesc("target_link_libraries", QStringList("MYTARGET") << "optimized" << "onlyoptimizedlib");

    QTest::newRow( "public" ) << CMakeFunctionDesc("target_link_libraries", QStringList("MYTARGET") << "LINK_PUBLIC" << "onlyoptimizedlib");
    QTest::newRow( "private" ) << CMakeFunctionDesc("target_link_libraries", QStringList("MYTARGET") << "LINK_PUBLIC" << "onlyoptimizedlib");

}

void CMakeAstTest::testTargetLinkLibrariesBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("target_link_libraries");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testTargetLinkLibrariesBadParse_data()
{
    CMakeFunctionDesc func1, func2, func3, func4;
    func1.name = "wrong_func_name";
    func2.name = func3.name = func4.name = "target_link_libraries";

    QStringList argList1, argList2, argList3, argList4;

    argList1 << "MYTARGET" << "somelib";
    argList2 << "MYTARGET";
    argList3 << "MYTARGET" << "optimized";
    argList4 << "MYTARGET" << "debug";

    func1.addArguments( argList1 );
    func2.addArguments( argList2 );
    func3.addArguments( argList3 );
    func4.addArguments( argList4 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "wrong func name" ) << func1;
    QTest::newRow( "missing libs" ) << func2;
    QTest::newRow( "missing libs optimized" ) << func3;
    QTest::newRow( "missing libs debug" ) << func4;

}
void CMakeAstTest::testTargetLinkLibrariesMembers()
{
    CMakeAst* ast = AstFactory::self()->createAst("target_link_libraries");
    CMakeFunctionDesc func;
    func.name = "target_link_libraries";
    QStringList argList;
    argList << "mytarget" << "mylibrary";
    func.addArguments(argList);
    QVERIFY( ast->parseFunctionInfo( func ) == true );

    TargetLinkLibrariesAst* targetLinkAst = static_cast<TargetLinkLibrariesAst*>(ast);

    QCOMPARE(targetLinkAst->target(), QString("mytarget"));
    QCOMPARE(targetLinkAst->publicDependencies().other, QStringList("mylibrary"));
    delete ast;
}

void CMakeAstTest::testTryCompileGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("try_compile");
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
    CMakeAst* ast = AstFactory::self()->createAst("try_compile");
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
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("try_run");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testTryRunGoodParse_data()
{
}

void CMakeAstTest::testTryRunBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("try_run");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testTryRunBadParse_data()
{
}

void CMakeAstTest::testUnsetGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("unset");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testUnsetGoodParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>("function");

    CMakeFunctionDesc l;
    l.name = "unset";
    l.addArguments(QStringList("HOLA"));
    QTest::newRow("a") << l;

    l.arguments.clear();
    l.addArguments(QStringList() << "HOLA" << "CACHE");
    QTest::newRow("a cache") << l;

    l.arguments.clear();
    l.addArguments(QStringList() << "ENV{HOLA}");
    QTest::newRow("env{a}") << l;
}

void CMakeAstTest::testUnsetBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    UnsetAst* ast = (UnsetAst*) AstFactory::self()->createAst("unset");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    QVERIFY( QRegExp("[A-Z]*").exactMatch(ast->variableName()) );
    delete ast;
}

void CMakeAstTest::testUnsetBadParse_data()
{
}

void CMakeAstTest::testUseMangledMesaGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("use_mangled_mesa");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testUseMangledMesaGoodParse_data()
{
}

void CMakeAstTest::testUseMangledMesaBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("use_mangled_mesa");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testUseMangledMesaBadParse_data()
{
}

void CMakeAstTest::testUtilitySourceGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("utility_source");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testUtilitySourceGoodParse_data()
{
}

void CMakeAstTest::testUtilitySourceBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("utility_source");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testUtilitySourceBadParse_data()
{
}

void CMakeAstTest::testVariableRequiresGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("variable_requires");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testVariableRequiresGoodParse_data()
{
}

void CMakeAstTest::testVariableRequiresBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("variable_requires");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testVariableRequiresBadParse_data()
{
}

void CMakeAstTest::testVtkMakeInstantiatorGoodParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("vtk_make_instantiator");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testVtkMakeInstantiatorGoodParse_data()
{
}

void CMakeAstTest::testVtkMakeInstantiatorBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("vtk_make_instantiator");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testVtkMakeInstantiatorBadParse_data()
{
}

void CMakeAstTest::testVtkWrapJavaGoodParse()
{
    TDD_TOIMPL;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("vtk_wrap_java");
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
    TDD_TOIMPL;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("vtk_wrap_java");
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
    TDD_TOIMPL;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("vtk_wrap_python");
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
    TDD_TOIMPL;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("vtk_wrap_python");
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
    TDD_TOIMPL;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("vtk_wrap_tcl");
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
    TDD_TOIMPL;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("vtk_wrap_tcl");
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
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("while");
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeAstTest::testWhileGoodParse_data()
{
}

void CMakeAstTest::testWhileBadParse()
{
    TDD_TODO;
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("while");
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeAstTest::testWhileBadParse_data()
{
}

void CMakeAstTest::testWriteFileGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeAst* ast = AstFactory::self()->createAst("write_file");
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
            funcs[i].name = funcs[i].name.toLower();
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
    CMakeAst* ast = AstFactory::self()->createAst("write_file");
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
//     args[1] << "myfile.txt" << "APPEND"; //append but no message. Should not crash, "append" is the message.
    args[NUM_TESTDATA - 1] << "myfile.txt" << "\"this is also my message\"" << "APPEND";
    
    QTest::addColumn<CMakeFunctionDesc>( "function" );

    for ( int i = 0; i < NUM_TESTDATA; i++)
    {
        funcs[i].addArguments( args[i] );
        QTest::newRow( qPrintable(QString("%1. %2").arg(i).arg(args[i].join(" "))) ) << funcs[i];
    }
}
