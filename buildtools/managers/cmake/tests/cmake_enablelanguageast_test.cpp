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

#include "cmake_enablelanguageast_test.h"
#include "cmakeast.h"
#include "cmakelistsparser.h"

QTEST_MAIN( EnableLanguageAstTest )

void EnableLanguageAstTest::testGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    EnableLanguageAst* ast = new EnableLanguageAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void EnableLanguageAstTest::testGoodParse_data()
{
    CMakeFunctionDesc func1, func2;
    func1.name = "ENABLE_LANGUAGE";
    func2.name = "enable_language";

    QStringList argList1, argList2;
    argList1 << "C++";
    argList2 << "Java";

    func1.addArguments( argList1 );
    func2.addArguments( argList2 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "good uppercase" ) << func1;
    QTest::newRow( "good lowercase" ) << func2;
}

void EnableLanguageAstTest::testBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    EnableLanguageAst* ast = new EnableLanguageAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void EnableLanguageAstTest::testBadParse_data()
{
    CMakeFunctionDesc func1, func2;
    func1.name = "ENABLE_LANGUAGES";
    func2.name = "enable_language";

    QStringList argList1, argList2;
    argList1 << "C++";

    func1.addArguments( argList1 );
    func2.addArguments( argList2 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "bad uppercase" ) << func1;
    QTest::newRow( "bad lowercase. no param" ) << func2;
}

#include "cmake_enablelanguageast_test.moc"
