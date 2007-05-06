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

#include "cmake_execprogramast_test.h"
#include "cmakeast.h"
#include "cmakelistsparser.h"

QTEST_MAIN( ExecProgramAstTest )

void ExecProgramAstTest::testGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    ExecProgramAst* ast = new ExecProgramAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void ExecProgramAstTest::testGoodParse_data()
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

void ExecProgramAstTest::testBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AddExecutableAst* ast = new AddExecutableAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void ExecProgramAstTest::testBadParse_data()
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

#include "cmake_execprogramast_test.moc"
