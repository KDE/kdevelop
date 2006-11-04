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

#include "cmake_cmakeminimumrequiredast_test.h"
#include "cmakeast.h"
#include "cmakelistsparser.h"

QTEST_MAIN( CMakeMinimumRequiredAstTest )

void CMakeMinimumRequiredAstTest::testGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CMakeMinimumRequiredAst* ast = new CMakeMinimumRequiredAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CMakeMinimumRequiredAstTest::testGoodParse_data()
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

void CMakeMinimumRequiredAstTest::testBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AddExecutableAst* ast = new AddExecutableAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CMakeMinimumRequiredAstTest::testBadParse_data()
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

#include "cmake_cmakeminimumrequiredast_test.moc"
