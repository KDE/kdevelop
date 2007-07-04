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

#include "cmake_vtkwrappythonast_test.h"
#include "cmakeast.h"
#include "cmakelistsparser.h"

QTEST_MAIN( VtkWrapPythonAstTest )

void VtkWrapPythonAstTest::testGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    VtkWrapPythonAst* ast = new VtkWrapPythonAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void VtkWrapPythonAstTest::testGoodParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>("function");

    const int NUM_TESTDATA = 2;
    CMakeFunctionDesc funcs[NUM_TESTDATA];
    QStringList args[NUM_TESTDATA];

    for ( int i = 0; i < NUM_TESTDATA; i++ )
        funcs[i].name = "VTK_WRAP_PYTHON";

    //test lowercase too
    funcs[NUM_TESTDATA - 1].name = funcs[NUM_TESTDATA - 1].name.toLower();

    args[0] << "myLibrary" << "MySourceList" << "source1";
    args[1] << "myLibrary" << "MySourceList" << "source1" << "source2";

    QTest::addColumn<CMakeFunctionDesc>( "function" );

    for ( int i = 0; i < NUM_TESTDATA; i++)
    {
        funcs[i].addArguments( args[i] );
        QTest::newRow( qPrintable(QString::number(i)) ) << funcs[i];
    }
}

void VtkWrapPythonAstTest::testBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AddExecutableAst* ast = new AddExecutableAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void VtkWrapPythonAstTest::testBadParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>("function");

    const int NUM_TESTDATA = 3;
    CMakeFunctionDesc funcs[NUM_TESTDATA];
    QStringList args[NUM_TESTDATA];

    for ( int i = 0; i < NUM_TESTDATA; i++ )
        funcs[i].name = "VTK_WRAP_PYTHON";

    funcs[NUM_TESTDATA - 1].name = "wrong_name";

    args[0] << "myLibrary"; //no source list and no sources
    args[1] << "myLibrary" << "MySourceList"; //source list but no sources
    args[NUM_TESTDATA - 1] << "myLibrary" << "MySourceList" << "source1";

    QTest::addColumn<CMakeFunctionDesc>( "function" );

    for ( int i = 0; i < NUM_TESTDATA; i++)
    {
        funcs[i].addArguments( args[i] );
        QTest::newRow( qPrintable(QString::number(i)) ) << funcs[i];
    }
}

#include "cmake_vtkwrappythonast_test.moc"
