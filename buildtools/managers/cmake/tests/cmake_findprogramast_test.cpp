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

#include "cmake_findprogramast_test.h"
#include "cmakeast.h"
#include "cmakelistsparser.h"

QTEST_MAIN( FindProgramAstTest )

void FindProgramAstTest::testGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    FindProgramAst* ast = new FindProgramAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void FindProgramAstTest::testGoodParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>("function");

    CMakeFunctionDesc l;
    l.name = "find_program";
    l.addArguments(QStringList() << "MY_VAR" << "file" << "location");
    QTest::newRow("normal use") << l;

    l.arguments.clear();
    l.addArguments(QStringList() << "MY_VAR" << "NAMES" << "file1" << "file2" << "PATHS" << "location1" << "location2");
    QTest::newRow("advanced use") << l;

    l.arguments.clear();
    l.addArguments(QStringList() << "MY_VAR" << "NAMES" << "file1" << "file2"
            << "PATHS" << "location1" << "location2" << "DOC" << "I am documenting"
            << "PATH_SUFFIXES" << "modules" << "NO_CMAKE_PATH");
    QTest::newRow("strange use") << l;
}

void FindProgramAstTest::testBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    FindProgramAst* ast = new FindProgramAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void FindProgramAstTest::testBadParse_data()
{
    
    QTest::addColumn<CMakeFunctionDesc>("function");
    
    CMakeFunctionDesc l1;
    l1.name = "";
    l1.addArguments(QStringList() << "MY_VAR" << "file");
    QTest::newRow ("no function name") << l1;

    CMakeFunctionDesc l;
    l.name = "find_program";
    l.addArguments(QStringList() << "MY_VAR" << "file");
    QTest::newRow("not enough parameters") << l;

    l.arguments.clear();
    l.addArguments(QStringList() << "MY_VAR" << "NAMES" << "PATHS" << "location1" << "location2");
    QTest::newRow("no names") << l;

    l.arguments.clear();
    l.addArguments(QStringList() << "MY_VAR" << "NAMES" << "file1" << "file2"
            << "PATHS"
            << "PATH_SUFFIXES" << "modules" << "NO_CMAKE_PATH");
    QTest::newRow("no paths") << l;
}

#include "cmake_findprogramast_test.moc"
