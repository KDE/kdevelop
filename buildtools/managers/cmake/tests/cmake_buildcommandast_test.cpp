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

#include "cmake_buildcommandast_test.h"
#include "cmakeast.h"
#include "cmakelistsparser.h"

QTEST_MAIN( BuildCommandAstTest )

void BuildCommandAstTest::testGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    BuildCommandAst* ast = new BuildCommandAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void BuildCommandAstTest::testGoodParse_data()
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

void BuildCommandAstTest::testBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AddExecutableAst* ast = new AddExecutableAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void BuildCommandAstTest::testBadParse_data()
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

#include "cmake_buildcommandast_test.moc"
