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

#include "cmakeadddefinitionasttest.h"
#include "cmakeast.h"
#include "cmakelistsparser.h"

QTEST_MAIN( AddDefinitionsAstTest )

void AddDefinitionsAstTest::testGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AddDefinitionsAst* ast = new AddDefinitionsAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void AddDefinitionsAstTest::testGoodParse_data()
{
    CMakeFunctionDesc func;
    func.name = "add_definitions";
    QStringList argList;
    argList << "-DFOOBAR" << "-DQT_NO_STL";
    func.addArguments( argList );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "simple" ) << func;
}

void AddDefinitionsAstTest::testBadParse_data()
{
    CMakeFunctionDesc func;
    func.name = "add_definition";
    QStringList argList;
    argList << "-DFOOBAR" << "-DQT_NO_STL";
    func.addArguments( argList );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "simple - bad" ) << func;

}

void AddDefinitionsAstTest::testBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AddDefinitionsAst* ast = new AddDefinitionsAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

#include "cmakeadddefinitionasttest.moc"

