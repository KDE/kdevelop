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

#include "cmake_setast_test.h"
#include "cmakeast.h"
#include "cmakelistsparser.h"

QTEST_MAIN( SetAstTest )

void SetAstTest::testGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    SetAst* ast = new SetAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void SetAstTest::testGoodParse_data()
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

void SetAstTest::testBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AddExecutableAst* ast = new AddExecutableAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void SetAstTest::testBadParse_data()
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

#include "cmake_setast_test.moc"
