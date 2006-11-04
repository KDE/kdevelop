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

#include "cmake_projectast_test.h"
#include "cmakeast.h"
#include "cmakelistsparser.h"

QTEST_MAIN( ProjectAstTest )

void ProjectAstTest::testGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    ProjectAst* ast = new ProjectAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void ProjectAstTest::testGoodParse_data()
{
    CMakeFunctionDesc func1, func2, func3, func4, func5, func6;
    func1.name = "PROJECT";
    func2.name = func3.name = func4.name = func5.name = func6.name = func1.name.toLower();

    QStringList argList1, argList2, argList3, argList4, argList5;
    argList1 << "myproject";
    argList2 << "myproject" << "C";
    argList3 << "myproject" << "CXX";
    argList4 << "myproject" << "Java";
    argList5 << "myproject" << "C" << "CXX" << "Java";

    func1.addArguments( argList1 );
    func2.addArguments( argList1 );
    func3.addArguments( argList2 );
    func4.addArguments( argList3 );
    func5.addArguments( argList4 );
    func6.addArguments( argList5 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "good 1" ) << func1;
    QTest::newRow( "good 2" ) << func2;
    QTest::newRow( "good 3" ) << func3;
    QTest::newRow( "good 4" ) << func4;
    QTest::newRow( "good 5" ) << func5;
    QTest::newRow( "good 6" ) << func6;

}

void ProjectAstTest::testBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AddExecutableAst* ast = new AddExecutableAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void ProjectAstTest::testBadParse_data()
{
    CMakeFunctionDesc func1, func2, func3;
    func1.name = "wrongname";
    func2.name = func3.name = "PROJECT";

    QStringList argList1, argList2;
    argList1 << "myproject" << "C" << "CXX" << "Java";
    argList2 << "myproject" << "C" << "CXX" << "Java" << "foo";

    func1.addArguments( argList1 );
    func3.addArguments( argList2 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow("wrong name") << func1;
    QTest::newRow("no args") << func2;
    QTest::newRow("wrong lang args") << func3;
}

#include "cmake_projectast_test.moc"
