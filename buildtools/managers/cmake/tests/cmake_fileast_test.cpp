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

#include "cmake_fileast_test.h"
#include "cmakeast.h"
#include "cmakelistsparser.h"

QTEST_MAIN( FileAstTest )

void FileAstTest::testGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    FileAst* ast = new FileAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void FileAstTest::testGoodParse_data()
{
    const int NUM_TESTDATA = 13;
    CMakeFunctionDesc funcs[NUM_TESTDATA];
    QStringList args[NUM_TESTDATA];

    for ( int i = 0; i < NUM_TESTDATA; i++ )
        funcs[i].name = "FILE";

    //write file command
    args[0] << "WRITE" << "somefile.cpp" << "\"the things to write\"";

    //append file command
    args[1] << "APPEND" << "somefile.cpp" << "\"the things to append\"";

    //read file command
    args[2] << "READ" << "somefile.cpp" << "MY_VAR";

    //glob files command. does not search in files only for a pattern
    args[3] << "GLOB" << "MY_VAR" << "*.cpp";
    args[4] << "GLOB" << "MY_VAR" << "RELATIVE" << "/path/to/something"
            << "*.cpp"; //RELATIVE is optional

    //recursive glob
    args[5] << "GLOB" << "MY_VAR" << "*.cpp";
    args[6] << "GLOB" << "MY_VAR" << "RELATIVE" << "/path/to/something"
            << "*.cpp"; //RELATIVE is optional

    //remove command
    args[7] << "REMOVE" << "/path/to/file/to/remove.cpp";

    //remove recursive
    args[8] << "REMOVE_RECURSE" << "/path/to/dir/to/remove/files";

    //make a directory
    args[9] << "MAKE_DIRECTORY" << "/path/to/dir/to/create";

    //get a relative path
    args[10] << "RELATIVE_PATH" << "MY_VAR" << "/path/to/foo"
             << "/path/to/file/to/get/path/for.cpp";

    //get the cmake native path
    args[11] << "TO_CMAKE_PATH" << "/path/to/file.cpp" << "CMAKE_PATH";

    //get the platform native path
    args[12] << "TO_NATIVE_PATH" << "/path/to/file.cpp" << "NATIVE_PATH";


    QTest::addColumn<CMakeFunctionDesc>( "function" );

    for ( int i = 0; i < NUM_TESTDATA; i++)
    {
        funcs[i].addArguments( args[i] );
        QTest::newRow( qPrintable(QString::number(i)) ) << funcs[i];
    }

}

void FileAstTest::testBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AddExecutableAst* ast = new AddExecutableAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void FileAstTest::testBadParse_data()
{
/*
    const int NUM_TESTDATA = 13;
    CMakeFunctionDesc funcs[NUM_TESTDATA];
    QStringList args[NUM_TESTDATA];

    for ( int i = 0; i < NUM_TESTDATA; i++ )
        funcs[i].name = "FILE";

    //write file command
    args[0] << "WRITE" << "somefile.cpp" << "\"the things to write\"";

    //append file command
    args[1] << "APPEND" << "somefile.cpp" << "\"the things to append\"";

    //read file command
    args[2] << "READ" << "somefile.cpp" << "MY_VAR";

    //glob files command. does not search in files only for a pattern
    args[3] << "GLOB" << "MY_VAR" << "*.cpp";
    args[4] << "GLOB" << "MY_VAR" << "RELATIVE" << "/path/to/something"
            << "*.cpp"; //RELATIVE is optional

    //recursive glob
    args[5] << "GLOB" << "MY_VAR" << "*.cpp";
    args[6] << "GLOB" << "MY_VAR" << "RELATIVE" << "/path/to/something"
            << "*.cpp"; //RELATIVE is optional

    //remove command
    args[7] << "REMOVE" << "/path/to/file/to/remove.cpp";

    //remove recursive
    args[8] << "REMOVE_RECURSE" << "/path/to/dir/to/remove/files";

    //make a directory
    args[9] << "MAKE_DIRECTORY" << "/path/to/dir/to/create";

    //get a relative path
    args[10] << "RELATIVE_PATH" << "MY_VAR" << "/path/to/foo"
             << "/path/to/file/to/get/path/for.cpp";

    //get the cmake native path
    args[11] << "TO_CMAKE_PATH" << "/path/to/file.cpp";

    //get the platform native path
    args[12] << "TO_NATIVE_PATH" << "/path/to/file.cpp";


    QTest::addColumn<CMakeFunctionDesc>( "function" );

    for ( int i = 0; i < NUM_TESTDATA; i++)
    {
        funcs[i].addArguments( args[i] );
        QTest::newRow( qPrintable(QString::number(i)) ) << funcs[i];
    }
*/
}

#include "cmake_fileast_test.moc"
//kate: space-indent on; indent-width 4; replace-tabs on; tab-width 4; auto-insert-doxygen: on; indent-mode: csands;