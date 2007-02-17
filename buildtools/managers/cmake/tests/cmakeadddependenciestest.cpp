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

#include "cmakeadddependenciestest.h"
#include "cmakeast.h"
#include "cmakelistsparser.h"

QTEST_MAIN( AddDependenciesAstTest )

void AddDependenciesAstTest::testGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AddDependenciesAst* ast = new AddDependenciesAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;

}

void AddDependenciesAstTest::testGoodParse_data()
{
    CMakeFunctionDesc func;
    func.name = "add_dependencies";
    QStringList argList;
    argList << "target-name" << "dep1" << "dep2";
    func.addArguments( argList );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "simple" ) << func;
}

void AddDependenciesAstTest::testBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AddDependenciesAst* ast = new AddDependenciesAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void AddDependenciesAstTest::testBadParse_data()
{
    CMakeFunctionDesc func;
    func.name = "add_dependencies";
    QStringList argList;

    CMakeFunctionDesc func2;
    func2.name = "add_dependencies";
    QStringList argList2;
    argList2 << "target";
    func2.addArguments( argList2 );

    CMakeFunctionDesc func3;
    func3.name = "foobar";
    QStringList argList3;
    argList3 << "target" << "dep1" << "dep2";
    func3.addArguments( argList3 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "no args" ) << func;
    QTest::newRow( "one arg" ) << func2;
    QTest::newRow( "two args. wrong name" ) << func3;
}

#include "cmakeadddependenciestest.moc"
