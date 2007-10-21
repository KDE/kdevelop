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
    args[5] << "GLOB_RECURSE" << "MY_VAR" << "*.cpp";
    args[6] << "GLOB_RECURSE" << "MY_VAR" << "RELATIVE" << "/path/to/something"
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
    args[11] << "TO_CMAKE_PATH" << "/path/to/file.cpp" << "MY_CMAKE_PATH";

    //get the platform native path
    args[12] << "TO_NATIVE_PATH" << "/path/to/file.cpp" << "MY_NATIVE_PATH";


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

    const int NUM_TESTDATA = 35;
    CMakeFunctionDesc funcs[NUM_TESTDATA];
    QStringList args[NUM_TESTDATA];

    for ( int i = 0; i < NUM_TESTDATA; i++ )
        funcs[i].name = "FILE";

    funcs[NUM_TESTDATA - 1].name = "foo";
    
    //write file command
    args[0] << "WRITE" << "somefile.cpp"; //nothing to write
    args[1] << "WRITE"; //no file
    args[2] << "write" << "somefile.cpp" << "thing to write"; //uppercase required

    //append file command
    args[3] << "APPEND" << "somefile.cpp"; //nothing to append
    args[4] << "APPEND"; //no file
    args[5] << "append" << "somefile.cpp" << "thing to append"; //uppercase required

    //read file command
    args[6] << "READ" << "somefile.cpp"; //no variable
    args[7] << "READ"; //no file
    args[8] << "read" << "somefile.cpp" << "MY_VAR"; //uppercase required

    //glob files command. does not search in files only for a pattern
    args[9] << "GLOB"; //no variable
    args[10] << "GLOB" << "MY_VAR" << "RELATIVE"; //no path
    
    args[11] << "glob" << "MY_VAR" << "*.cpp"; //uppercase required
    args[12] << "GLOB" << "MY_VAR" << "relative" << "/path/to/something"
               << "*.cpp"; //uppercase required

    //recursive glob
    args[13] << "GLOB_RECURSE"; //no variable
    args[14] << "GLOB_RECURSE" << "MY_VAR" << "RELATIVE"; //no path

    args[15] << "glob_recurse" << "MY_VAR" << "*.cpp"; //uppercase required
    args[16] << "GLOB_RECURSE" << "MY_VAR" << "relative" << "/path/to/something"
            << "*.cpp"; //uppercase required

    //remove command
    args[17] << "REMOVE"; //nothing to remove
    args[18] << "remove" << "/path/to/file/to/remove.cpp";

    //remove recursive
    args[19] << "REMOVE_RECURSE"; //nothing to remove
    args[20] << "remove_recurse" << "/path/to/dir"; //uppercase required

    //make a directory
    args[21] << "MAKE_DIRECTORY"; //nothing to create
    args[22] << "make_directory" << "/path/to/dir"; //uppercase required

    //get a relative path
    args[23] << "RELATIVE_PATH" << "MY_VAR" << "/path/to/foo"; //no file
    args[24] << "RELATIVE_PATH" << "MY_VAR"; //no path and no file
    args[25] << "RELATIVE_PATH"; //no variable, path, or file
    args[26] << "relative_path" << "MY_VAR" << "/path/to/foo"
        << "/path/to/file/to/get/full/path/for.cpp"; // uppercase required

    //get the cmake native path
    args[27] << "TO_CMAKE_PATH" << "/path/to/file.cpp"; // no variable
    args[28] << "TO_CMAKE_PATH"; //no path or variable
    args[29] << "to_cmake_path" << "/path/to/file.cpp" << "MY_VAR"; //uppercase required
    
    //get the platform native path
    args[30] << "TO_NATIVE_PATH" << "/path/to/file.cpp"; //no variable
    args[31] << "TO_NATIVE_PATH"; //no path or variable
    args[32] << "to_native_path" << "/path/to/file.cpp" << "MY_VAR"; //uppercase required
    
    args[34] << "TO_NATIVE_PATH" << "/path/to/file.cpp" << "MY_VAR"; //correct args. wrong name
    
    QTest::addColumn<CMakeFunctionDesc>( "function" );

    for ( int i = 0; i < NUM_TESTDATA; i++)
    {
        funcs[i].addArguments( args[i] );
        QTest::newRow( qPrintable(QString::number(i)) ) << funcs[i];
    }

}

#include "cmake_fileast_test.moc"
