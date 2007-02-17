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

#include "cmakecustomtargetasttest.h"
#include "cmakeast.h"
#include "cmakelistsparser.h"

QTEST_MAIN( CustomTargetAstTests )

void CustomTargetAstTests::testGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CustomTargetAst* ast = new CustomTargetAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CustomTargetAstTests::testGoodParse_data()
{
    CMakeFunctionDesc func1;
    func1.name = "add_custom_target";
    QStringList argList1;
    argList1 << "MyName" << "ALL" << "foobar --test" << "COMMAND"
             << "barbaz --foo" << "DEPENDS" << "dep1" << "dep2" << "dep3"
             << "WORKING_DIRECTORY" << "/path/to/my/dir" << "COMMENT"
             << "this is my comment" << "VERBATIM";
    func1.addArguments( argList1 );

    CMakeFunctionDesc func2;
    func2.name = "ADD_CUSTOM_TARGET";
    QStringList argList2;
    argList2 << "MyName" << "my_command --test-param 1";
    func2.addArguments( argList2 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "all optional" ) << func1;
    QTest::newRow( "no optional" ) << func2;
}

void CustomTargetAstTests::testBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CustomTargetAst* ast = new CustomTargetAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CustomTargetAstTests::testBadParse_data()
{
    CMakeFunctionDesc func1;
    func1.name = "add_custom_target";
    QStringList argList1;
    argList1 << "IAm#1" << "ALL" << "foobar --test" << "COMMAND"
             << "barbaz --foo" << "DEPENDS" << "dep1" << "dep2" << "dep3"
             << "WORKING_DIRECTORY" << "/path/to/my/dir" << "COMMENT"
             << "this is my comment" << "VERBATIM";
    func1.addArguments( argList1 );

    CMakeFunctionDesc func2;
    func2.name = "ADD_CUSTOM_TARGET";
    QStringList argList2;
    argList2 << "ALL" << "my_command --test-param 1";
    func2.addArguments( argList2 );

    CMakeFunctionDesc func3;
    func3.name = "add_custom_target";

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "bad 1" ) << func1;
    QTest::newRow( "bad 2" ) << func2;
    QTest::newRow( "bad 3" ) << func3;
}

#include "cmakecustomtargetasttest.moc"
