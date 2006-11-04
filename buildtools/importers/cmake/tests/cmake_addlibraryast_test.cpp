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

#include "cmake_addlibraryast_test.h"
#include "cmakeast.h"
#include "cmakelistsparser.h"

QTEST_MAIN( AddLibraryAstTest )

void AddLibraryAstTest::testGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AddLibraryAst* ast = new AddLibraryAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void AddLibraryAstTest::testGoodParse_data()
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

void AddLibraryAstTest::testBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AddExecutableAst* ast = new AddExecutableAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void AddLibraryAstTest::testBadParse_data()
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

#include "cmake_addlibraryast_test.moc"
