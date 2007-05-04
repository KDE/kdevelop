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

#include "cmake_markasadvancedast_test.h"
#include "cmakeast.h"
#include "cmakelistsparser.h"

QTEST_MAIN( MarkAsAdvancedAstTest )

void MarkAsAdvancedAstTest::testGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    MarkAsAdvancedAst* ast = new MarkAsAdvancedAst();
    QCOMPARE( ast->parseFunctionInfo( function ), true );
    delete ast;
}

void MarkAsAdvancedAstTest::testGoodParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>("function");
    
    CMakeFunctionDesc l;
    l.name = "mark_as_advanced";
    l.addArguments(QStringList() << "FORCE" << "My_LIBRARY" << "My_INCLUDES");
    QTest::newRow("a forced mark_as_advanced") << l;
    
    l.arguments.clear();
    l.addArguments(QStringList() << "CLEAR" << "My_LIB");
    QTest::newRow("a clear mark_as_advanced") << l;
    
    l.arguments.clear();
    l.addArguments(QStringList() << "My_LIB");
    QTest::newRow("a normal mark_as_advanced") << l;
}

void MarkAsAdvancedAstTest::testBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    MarkAsAdvancedAst* ast = new MarkAsAdvancedAst();
    QCOMPARE( ast->parseFunctionInfo( function ), false );
    delete ast;
}

void MarkAsAdvancedAstTest::testBadParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>("function");
    
    CMakeFunctionDesc l;
    l.name = "mark_as_advanced";
    QTest::newRow("a mark_as_advanced without parameters") << l;
    
    l.arguments.clear();
    l.addArguments(QStringList() << "CLEAR");
    QTest::newRow("a clear mark_as_advanced without parameters") << l;
}

#include "cmake_markasadvancedast_test.moc"
