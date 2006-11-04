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

#include "cmake_targetlinklibrariesast_test.h"
#include "cmakeast.h"
#include "cmakelistsparser.h"

QTEST_MAIN( TargetLinkLibrariesAstTest )

void TargetLinkLibrariesAstTest::testGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    TargetLinkLibrariesAst* ast = new TargetLinkLibrariesAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void TargetLinkLibrariesAstTest::testGoodParse_data()
{
    CMakeFunctionDesc func1, func2, func3;
    func1.name = "TARGET_LINK_LIBRARIES";
    func2.name = func3.name = func1.name.toLower();

    QStringList argList1, argList2, argList3;

    argList1 << "MYTARGET" << "SOME_VAR";
    argList2 << "MYTARGET" << "debug" << "onlydebuglib";
    argList3 << "MYTARGET" << "optimized" << "onlyoptimizedlib";

    func1.addArguments( argList1 );
    func2.addArguments( argList2 );
    func3.addArguments( argList3 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "whatever" ) << func1;
    QTest::newRow( "whatever" ) << func2;
    QTest::newRow( "whatever" ) << func3;

}

void TargetLinkLibrariesAstTest::testBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AddExecutableAst* ast = new AddExecutableAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void TargetLinkLibrariesAstTest::testBadParse_data()
{
    CMakeFunctionDesc func1, func2, func3, func4;
    func1.name = "wrong_func_name";
    func2.name = func3.name, func4.name = "target_link_libraries";

    QStringList argList1, argList2, argList3, argList4;

    argList1 << "MYTARGET" << "SOME_VAR";
    argList2 << "MYTARGET";
    argList3 << "MYTARGET" << "optimized";
    argList4 << "MYTARGET" << "debug";

    func1.addArguments( argList1 );
    func2.addArguments( argList2 );
    func3.addArguments( argList3 );
    func4.addArguments( argList4 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "whatever" ) << func1;
    QTest::newRow( "whatever" ) << func2;
    QTest::newRow( "whatever" ) << func3;
    QTest::newRow( "whatever" ) << func4;

}

#include "cmake_targetlinklibrariesast_test.moc"
