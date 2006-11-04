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

#include "cmake_addtestast_test.h"
#include "cmakeast.h"
#include "cmakelistsparser.h"

QTEST_MAIN( AddTestAstTest )

void AddTestAstTest::testGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AddTestAst* ast = new AddTestAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void AddTestAstTest::testGoodParse_data()
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

void AddTestAstTest::testBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AddExecutableAst* ast = new AddExecutableAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void AddTestAstTest::testBadParse_data()
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

#include "cmake_addtestast_test.moc"
