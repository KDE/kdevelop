/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
 * Copyright 2007 Aleix Pol <aleixpol@gmail.com>
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

#include "cmake_trycompileast_test.h"
#include "cmakeast.h"
#include "cmakelistsparser.h"

QTEST_MAIN( TryCompileAstTest )

void TryCompileAstTest::testGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    TryCompileAst* ast = new TryCompileAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void TryCompileAstTest::testGoodParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>("function");

    CMakeFunctionDesc l;
    l.name = "try_compile";
    l.addArguments(QStringList() << "RESULT" << "mySrcDir" << "myBinDir" << "myProjectName");
    QTest::newRow("cmake way") << l;

    l.arguments.clear();
    l.addArguments(QStringList() << "RESULT" << "mySrcDir" << "myBinDir");
    QTest::newRow("compile way") << l;

    l.arguments.clear();
    l.addArguments(QStringList() << "RESULT" << "mySrcDir" << "myBinDir" << "COMPILE_DEFINITIONS" << "-D LOL");
    QTest::newRow("more complex compile") << l;
}

void TryCompileAstTest::testBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    TryCompileAst* ast = new TryCompileAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void TryCompileAstTest::testBadParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>("function");

    CMakeFunctionDesc l;
    l.name = "try_compile";
    QTest::newRow("no parameters") << l;

}

#include "cmake_trycompileast_test.moc"
