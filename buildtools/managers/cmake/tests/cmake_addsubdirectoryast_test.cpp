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

#include "cmake_addsubdirectoryast_test.h"
#include "cmakeast.h"
#include "cmakelistsparser.h"

QTEST_MAIN( AddSubdirectoryAstTest )

void AddSubdirectoryAstTest::testGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AddSubdirectoryAst* ast = new AddSubdirectoryAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void AddSubdirectoryAstTest::testGoodParse_data()
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

void AddSubdirectoryAstTest::testBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AddExecutableAst* ast = new AddExecutableAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void AddSubdirectoryAstTest::testBadParse_data()
{
    CMakeFunctionDesc func, func2;
    func.name = "ADD_SUBDIRECTORY";
    func2.name = "foobar";

    func2.addArguments( QStringList( "foodir" ) );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "good lowercase" ) << func;
    QTest::newRow( "good uppercase" ) << func2;
}

#include "cmake_addsubdirectoryast_test.moc"
