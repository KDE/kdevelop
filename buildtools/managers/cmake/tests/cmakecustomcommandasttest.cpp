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

#include "cmakecustomcommandasttest.h"
#include "cmakeast.h"
#include "cmakelistsparser.h"

QTEST_MAIN( CustomCommandAstTests )

void CustomCommandAstTests::testGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CustomCommandAst* ast = new CustomCommandAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void CustomCommandAstTests::testGoodParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>( "function" );
    CMakeFunctionDesc func;
    func.name = "add_custom_command";
    QStringList argList;
    argList << "OUTPUT" << "foo" << "COMMAND" << "bar";
    func.addArguments( argList );
    func.filePath = QString();
    func.line = 0;

    CMakeFunctionDesc func1;
    func1.name = "add_custom_command";
    QStringList argList1;
    argList1 << "OUTPUT" << "foo" << "COMMAND" << "bar";
    argList1 << "MAIN_DEPENDENCY" << "dep1" << "DEPENDS" << "dep1" << "dep2";
    argList1 << "WORKING_DIRECTORY" << "dir1" << "COMMENT" << "some comment";
    argList1 << "VERBATIM" << "APPEND";
    func1.addArguments( argList1 );

    CMakeFunctionDesc func2;
    func2.name = "ADD_CUSTOM_COMMAND";
    QStringList argList2;
    argList2 << "OUTPUT" << "foo" << "COMMAND" << "bar" << "ARGS" << "baz";
    argList2 << "MAIN_DEPENDENCY" << "dep1" << "DEPENDS" << "dep1" << "dep2";
    argList2 << "WORKING_DIRECTORY" << "dir1" << "COMMENT" << "some comment";
    argList2 << "VERBATIM" << "APPEND";
    func2.addArguments( argList2 );

    CMakeFunctionDesc func3;
    func3.name = "ADD_CUSTOM_COMMAND";
    QStringList argList3;
    argList3 << "TARGET" << "foo" << "PRE_BUILD" << "COMMAND" << "bar";
    argList3 << "MAIN_DEPENDENCY" << "dep1" << "DEPENDS" << "dep1" << "dep2";
    argList3 << "WORKING_DIRECTORY" << "dir1" << "COMMENT" << "some comment";
    func3.addArguments( argList3 );

    QTest::newRow( "no optional" ) << func;
    QTest::newRow( "all optional" ) << func1;
    QTest::newRow( "optional with arg" ) << func2;
    QTest::newRow( "second form all optional uppercase" ) << func3;

}

void CustomCommandAstTests::testBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    CustomCommandAst* ast = new CustomCommandAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void CustomCommandAstTests::testBadParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>( "function" );
    CMakeFunctionDesc func;
    func.name = "foo";
    func.filePath = QString();
    func.line = 0;

    CMakeFunctionDesc func_noargs;
    func_noargs.name = "add_custom_command";

    CMakeFunctionDesc func2;
    func2.name = "add_custom_command";
    QStringList argList2;
    argList2 << "nottarget" << "foo" << "notcommand" << "foo1";
    func2.addArguments( argList2 );
    func2.filePath = QString();
    func2.line = 0;

    CMakeFunctionDesc func3;
    func3.name = "add_custom_command";
    QStringList argList3;
    argList3 << "target" << "foo" << "no_pre_build" << "foo1";
    func3.addArguments( argList3 );
    func3.filePath = QString();
    func3.line = 0;

    CMakeFunctionDesc func4;
    func4.name = "add_custom_command";
    QStringList argList4;
    argList4 << "output" << "foo1" << "no_command" << "foo2";
    func4.addArguments( argList4 );
    func4.filePath = QString();
    func4.line = 0;

    CMakeFunctionDesc func5;
    func3.name = "add_custom_command";
    QStringList argList5;
    argList5 << "target" << "foo" << "PRE_BUILD" << "no_command";
    func5.addArguments( argList5 );
    func5.filePath = QString();
    func5.line = 0;

    QTest::newRow( "wrong function" ) << func;
    QTest::newRow( "right function. no args" ) << func_noargs;
    QTest::newRow( "wrong params 1" ) << func2;
    QTest::newRow( "wrong params 2" ) << func3;
    QTest::newRow( "wrong params 3" ) << func4;
    QTest::newRow( "wrong params 4" ) << func5;

}



#include "cmakecustomcommandasttest.moc"
