/* KDevelop CMake Support
 *
 * Copyright 2006,2007 Matt Rogers <mattr@kde.org>
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

#include "cmake_createtestsourcelistast_test.h"
#include "cmakeast.h"
#include "cmakelistsparser.h"

QTEST_MAIN( CreateTestSourcelistAstTest )

void CreateTestSourcelistAstTest::testGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CreateTestSourcelistAst* ast = new CreateTestSourcelistAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CreateTestSourcelistAstTest::testGoodParse_data()
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

void CreateTestSourcelistAstTest::testBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AddExecutableAst* ast = new AddExecutableAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CreateTestSourcelistAstTest::testBadParse_data()
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

#include "cmake_createtestsourcelistast_test.moc"
