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

#include "cmake_findlibraryast_test.h"
#include "cmakeast.h"
#include "cmakelistsparser.h"

QTEST_MAIN( FindLibraryAstTest )

void FindLibraryAstTest::testGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    FindLibraryAst* ast = new FindLibraryAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void FindLibraryAstTest::testGoodParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>("function");
    
    CMakeFunctionDesc l;
    l.name = "find_library";
    l.addArguments(QString("DEST_VAR name").split(' '));
    QTest::newRow("findlib with name") << l;
    
    l.arguments.clear();
    l.addArguments(QString("DEST_VAR name /path/to/lib").split(' '));
    QTest::newRow("findlib with name and path") << l;
}

void FindLibraryAstTest::testBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    FindLibraryAst* ast = new FindLibraryAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void FindLibraryAstTest::testBadParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>("function");
    
    CMakeFunctionDesc l;
    l.name = "lol";
    l.addArguments(QString("DEST_VAR name").split(' '));
    QTest::newRow("findlib with name") << l;
}

#include "cmake_findlibraryast_test.moc"
