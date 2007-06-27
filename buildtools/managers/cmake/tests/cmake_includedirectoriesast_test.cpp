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

#include "cmake_includedirectoriesast_test.h"
#include "cmakeast.h"
#include "cmakelistsparser.h"

QTEST_MAIN( IncludeDirectoriesAstTest )

void IncludeDirectoriesAstTest::testGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    IncludeDirectoriesAst* ast = new IncludeDirectoriesAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void IncludeDirectoriesAstTest::testGoodParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>("function");
    
    CMakeFunctionDesc l;
    l.name = "include_directories";
    l.addArguments(QStringList() << "../g4u");
    QTest::newRow("a normal include_directories") << l;
    
    l.arguments.clear();
    l.addArguments(QStringList() << "AFTER" << "boost/");
    QTest::newRow("a include_directories with AFTER parameter") << l;
    
    l.arguments.clear();
    l.addArguments(QStringList() << "SYSTEM" << "~/kdelibs");
    QTest::newRow("a include_directories with SYSTEM paremeter") << l;
}

void IncludeDirectoriesAstTest::testBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    IncludeDirectoriesAst* ast = new IncludeDirectoriesAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void IncludeDirectoriesAstTest::testBadParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>("function");
    
    CMakeFunctionDesc l;
    l.name = "include_directories";
    l.addArguments(QStringList() << "AFTER" << "BEFORE" << "lol");
    QTest::newRow("can't have after and before in include_directories") << l;
}

#include "cmake_includedirectoriesast_test.moc"
