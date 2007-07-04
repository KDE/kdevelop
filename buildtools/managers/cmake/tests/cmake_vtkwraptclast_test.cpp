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

#include "cmake_vtkwraptclast_test.h"
#include "cmakeast.h"
#include "cmakelistsparser.h"

QTEST_MAIN( VtkWrapTclAstTest )

void VtkWrapTclAstTest::testGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    VtkWrapTclAst* ast = new VtkWrapTclAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void VtkWrapTclAstTest::testGoodParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>("function");

    const int NUM_TESTDATA = 6;
    CMakeFunctionDesc funcs[NUM_TESTDATA];
    QStringList args[NUM_TESTDATA];

    for ( int i = 0; i < NUM_TESTDATA; i++ )
        funcs[i].name = "VTK_WRAP_TCL";

    //test lowercase too
    funcs[NUM_TESTDATA - 1].name = funcs[NUM_TESTDATA - 1].name.toLower();

    args[0] << "myLibrary" << "MySourceList" << "source1.tcl";
    
    //SOURCES is optional
    args[1] << "myLibrary" << "SOURCES" << "MySourceList" << "source1.tcl";
    
    //multiple sources
    args[2] << "myLibrary" << "SOURCES" << "MySourceList" << "source1.tcl"
            << "source2.tcl";

    //commands are optional
    args[3] << "myLibrary" << "MySourceList" << "source1.tcl"
            << "COMMANDS" << "com1";

    //multiple commands
    args[4] << "myLibrary" << "MySourceList" << "source1.tcl"
            << "COMMANDS" << "com1" << "com2";
            
    //line from the cmake example
    args[NUM_TESTDATA - 1] << "myLibrary" << "SOURCES"
            << "MySourceList" << "class1" << "class2"
            << "COMMANDS" << "first_command" << "second_command";
    
    QTest::addColumn<CMakeFunctionDesc>( "function" );

    for ( int i = 0; i < NUM_TESTDATA; i++)
    {
        funcs[i].addArguments( args[i] );
        QTest::newRow( qPrintable(QString::number(i)) ) << funcs[i];
    }
}

void VtkWrapTclAstTest::testBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AddExecutableAst* ast = new AddExecutableAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void VtkWrapTclAstTest::testBadParse_data()
{
    QTest::addColumn<CMakeFunctionDesc>("function");

    const int NUM_TESTDATA = 5;
    CMakeFunctionDesc funcs[NUM_TESTDATA];
    QStringList args[NUM_TESTDATA];

    for ( int i = 0; i < NUM_TESTDATA; i++ )
        funcs[i].name = "VTK_WRAP_TCL";

    funcs[NUM_TESTDATA - 1].name = "wrong_name";

    args[0] << "myLibrary"; //no source list and no sources
    args[1] << "myLibrary" << "MySourceList"; //source list but no sources
    args[2] << "myLibrary" << "SOURCES" << "MySourceList"; //source list but no sources
    args[3] << "myLibrary" << "SOURCES" << "MySourceList" << "foo.tcl";
    args[NUM_TESTDATA - 1] << "myLibrary" << "SOURCES"
            << "MySourceList" << "class1" << "class2"
            << "COMMANDS" << "first_command";
    
    QTest::addColumn<CMakeFunctionDesc>( "function" );

    for ( int i = 0; i < NUM_TESTDATA; i++)
    {
        funcs[i].addArguments( args[i] );
        QTest::newRow( qPrintable(QString::number(i)) ) << funcs[i];
    }
}

#include "cmake_vtkwraptclast_test.moc"
