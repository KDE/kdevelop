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

#include "cmake_configurefileast_test.h"
#include "cmakeast.h"
#include "cmakelistsparser.h"

QTEST_MAIN( ConfigureFileAstTest )

void ConfigureFileAstTest::testGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    ConfigureFileAst* ast = new ConfigureFileAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void ConfigureFileAstTest::testGoodParse_data()
{
    CMakeFunctionDesc func1, func2, func3, func4, func5;
    func1.name = "CONFIGURE_FILE";
    func2.name = func1.name.toLower();
    func3.name = func4.name = func5.name = func2.name;

    QStringList argList1, argList2, argList3, argList4;
    argList1 << "inputfile" << "outputfile";
    argList2 = argList1 << "COPYONLY";
    argList3 = argList2 << "ESCAPE_QUOTES";
    argList4 = argList3 << "@ONLY";

    func1.addArguments( argList1 );
    func2.addArguments( argList1 );
    func3.addArguments( argList2 );
    func4.addArguments( argList3 );
    func5.addArguments( argList4 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "good uppercase min args" ) << func1;
    QTest::newRow( "good lowercase min args" ) << func2;
    QTest::newRow( "good lowercase 3 args" ) << func3;
    QTest::newRow( "good lowercase 4 args" ) << func4;
    QTest::newRow( "good lowercase 5 args" ) << func5;
}

void ConfigureFileAstTest::testBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AddExecutableAst* ast = new AddExecutableAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void ConfigureFileAstTest::testBadParse_data()
{
    CMakeFunctionDesc func1, func2, func3;
    func1.name = "wrong_func_name";
    func2.name = func3.name = "configure_file";

    QStringList argList1, argList2, argList3;
    argList1 << "sourcefile" << "outputfile";
    argList2 << "only_one";

    func1.addArguments( argList1 );
    func2.addArguments( argList2 );
    func2.addArguments( argList3 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "bad wrong name" ) << func1;
    QTest::newRow( "bad only one arg" ) << func2;
    QTest::newRow( "bad no args" ) << func3;
}

#include "cmake_configurefileast_test.moc"
