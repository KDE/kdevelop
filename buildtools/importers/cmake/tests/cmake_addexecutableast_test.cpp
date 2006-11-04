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

#include "cmake_addexecutableast_test.h"
#include "cmakeast.h"
#include "cmakelistsparser.h"

QTEST_MAIN( AddExecutableAstTest )

void AddExecutableAstTest::testGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AddExecutableAst* ast = new AddExecutableAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void AddExecutableAstTest::testGoodParse_data()
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

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "only sources" ) << func;
    QTest::newRow( "win 32 app" ) << func2;
    QTest::newRow( "mac os bundle" ) << func3;
    QTest::newRow( "exclude from all" ) << func4;
    QTest::newRow( "all" ) << func5;
}

void AddExecutableAstTest::testBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AddExecutableAst* ast = new AddExecutableAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void AddExecutableAstTest::testBadParse_data()
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

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "no sources" ) << func;
    QTest::newRow( "wrong name" ) << func2;
    QTest::newRow( "no arguments" ) << func3;

}

#include "cmake_addexecutableast_test.moc"
