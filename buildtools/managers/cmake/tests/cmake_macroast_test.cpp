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

#include "cmake_macroast_test.h"
#include "cmakeast.h"
#include "cmakelistsparser.h"

QTEST_MAIN( MacroAstTest )

void MacroAstTest::testGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    MacroAst* ast = new MacroAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void MacroAstTest::testGoodParse_data()
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

void MacroAstTest::testBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AddExecutableAst* ast = new AddExecutableAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void MacroAstTest::testBadParse_data()
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

#include "cmake_macroast_test.moc"
