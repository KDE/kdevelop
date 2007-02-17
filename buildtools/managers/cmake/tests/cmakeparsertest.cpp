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

#include "cmakeparsertest.h"

#include <KTemporaryFile>
#include "cmListFileLexer.h"
#include "cmakelistsparser.h"
#include "cmakeast.h"

QTEST_MAIN( CMakeParserTest )

CMakeParserTest::CMakeParserTest()
{
    fakeRoot = new CMakeAst;
}


CMakeParserTest::~CMakeParserTest()
{
    delete fakeRoot;
}

void CMakeParserTest::testLexerCreation()
{
    cmListFileLexer* lexer = cmListFileLexer_New();
    QVERIFY( lexer != 0 );
    cmListFileLexer_Delete( lexer );
}

void CMakeParserTest::testLexerWithFile()
{
    KTemporaryFile tempFile;
    tempFile.setAutoRemove( false );
    tempFile.open();
    if ( !QFile::exists( tempFile.fileName() ) )
         QFAIL( "Unable to open temporary file" );

    QString tempName = tempFile.fileName();
    tempFile.close(); //hacks to the get name of the file

    cmListFileLexer* lexer = cmListFileLexer_New();
    if ( !lexer )
        QFAIL( "unable to create lexer" );
    QVERIFY( cmListFileLexer_SetFileName( lexer, qPrintable( tempName ) ) );
    cmListFileLexer_Delete( lexer );
    tempFile.remove();
}

void CMakeParserTest::testParserWithGoodData()
{
//    QFAIL( "the magic is missing" );
    QFETCH( QString, text );
    QTemporaryFile tempFile;
    tempFile.setAutoRemove( false );
    tempFile.open();
    if ( !QFile::exists( tempFile.fileName() ) )
        QFAIL( "Unable to open temporary file" );

    tempFile.write( text.toUtf8() );
    QString tempName = tempFile.fileName();
    tempFile.close(); //hacks to the get name of the file
    CMakeAst* ast = new CMakeAst;
    bool parseError = CMakeListsParser::parseCMakeFile( ast, qPrintable( tempName ) );
    delete ast;
    QVERIFY( parseError == false );

}

void CMakeParserTest::testParserWithGoodData_data()
{
    QTest::addColumn<QString>( "text" );
    QTest::newRow( "good data1" ) << "project(foo)\nset(foobar_SRCS foo.h foo.c)";
    QTest::newRow( "good data2" ) << "set(foobar_SRCS foo.h foo.c)\n"
                                     "add_executable( foo ${foobar_SRCS})";
}

void CMakeParserTest::testParserWithBadData()
{
    QFETCH( QString, text );
    QTemporaryFile tempFile;
    tempFile.setAutoRemove( false );
    tempFile.open();
    if ( !QFile::exists( tempFile.fileName() ) )
        QFAIL( "Unable to open temporary file" );

    tempFile.write( text.toUtf8() );
    QString tempName = tempFile.fileName();
    tempFile.close(); //hacks to the get name of the file
    CMakeAst* ast = new CMakeAst;
    bool parseError = CMakeListsParser::parseCMakeFile( ast, qPrintable( tempName ) );
    delete ast;
    QVERIFY( parseError == true );
}

void CMakeParserTest::testParserWithBadData_data()
{
    QTest::addColumn<QString>( "text" );

    //just plain wrong. :)
    QTest::newRow( "bad data 1" ) << "foo bar baz zippedy do dah";

    //missing right parenthesis
    QTest::newRow( "bad data 2" ) << "set(mysrcs_SRCS foo.c\n\n\n";

    //extra identifiers between functions. cmake doesn't allow plain identifiers
    QTest::newRow( "bad data 3" ) << "the(quick) brown fox jumped(over) the lazy dog";

    //invalid due to no newline before next command
    QTest::newRow( "bad data 4" ) << "project(foo) set(mysrcs_SRCS foo.c)";
}

// void CMakeParserTest::testAstCreation()
// {

// }

#include "cmakeparsertest.moc"

