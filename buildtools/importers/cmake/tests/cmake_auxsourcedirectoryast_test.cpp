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

#include "cmake_auxsourcedirectoryast_test.h"
#include "cmakeast.h"
#include "cmakelistsparser.h"

QTEST_MAIN( AuxSourceDirectoryAstTest )

void AuxSourceDirectoryAstTest::testGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AuxSourceDirectoryAst* ast = new AuxSourceDirectoryAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void AuxSourceDirectoryAstTest::testGoodParse_data()
{
    CMakeFunctionDesc func1, func2;
    func1.name = "AUX_SOURCE_DIRECTORY";
    func2.name = "aux_source_directory";

    QStringList argList;
    argList << "foo1" << "foo2";

    func1.addArguments( argList );
    func2.addArguments( argList );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "good uppercase" ) << func1;
    QTest::newRow( "good lowercase" ) << func2;
}

void AuxSourceDirectoryAstTest::testBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AddExecutableAst* ast = new AddExecutableAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void AuxSourceDirectoryAstTest::testBadParse_data()
{
    CMakeFunctionDesc func1, func2;
    func1.name = "AUX_SOURCE_DIRECTORY";
    func2.name = "wrong name";

    QStringList argList;
    argList << "foo1" << "foo2";

    func2.addArguments( argList );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "bad no args" ) << func1;
    QTest::newRow( "bad wrong name" ) << func2;
}

#include "cmake_auxsourcedirectoryast_test.moc"
