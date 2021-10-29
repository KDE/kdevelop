/*
    SPDX-FileCopyrightText: 2006 Matt Rogers <mattr@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "cmakeparsertest.h"

#include <QTemporaryFile>
#include "cmListFileLexer.h"
#include "cmakelistsparser.h"

CMakeParserTest::CMakeParserTest()
{}

CMakeParserTest::~CMakeParserTest()
{}

void CMakeParserTest::testLexerCreation()
{
    cmListFileLexer* lexer = cmListFileLexer_New();
    QVERIFY( lexer != nullptr );
    cmListFileLexer_Delete( lexer );
}

void CMakeParserTest::testLexerWithFile()
{
    QTemporaryFile tempFile;
    tempFile.setAutoRemove( false );
    tempFile.open();
    if ( !QFile::exists( tempFile.fileName() ) )
         QFAIL( "Unable to open temporary file" );

    QString tempName = tempFile.fileName();
    tempFile.close(); //hacks to the get name of the file

    cmListFileLexer* lexer = cmListFileLexer_New();
    if ( !lexer )
        QFAIL( "unable to create lexer" );
    QVERIFY( cmListFileLexer_SetFileName( lexer, qPrintable( tempName ), nullptr ) );
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
    const bool parseError = CMakeListsParser::readCMakeFile( tempName ).isEmpty();
    QVERIFY( parseError == false );
    tempFile.remove();
}

void CMakeParserTest::testParserWithGoodData_data()
{
    QTest::addColumn<QString>( "text" );
    QTest::newRow( "good data1" ) << "project(foo)\nset(foobar_SRCS foo.h foo.c)";
    QTest::newRow( "good data2" ) << "set(foobar_SRCS foo.h foo.c)\n"
                                     "add_executable( foo ${foobar_SRCS})";
    QTest::newRow( "test data" ) << "add_test(mytest \"mytest\")\n";
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
    tempFile.close(); //hacks to the get name of the file
//    CMakeAst* ast = new CMakeAst;
//    QString tempName = tempFile.fileName();
//    bool parseError = CMakeListsParser::parseCMakeFile( ast, qPrintable( tempName ) );
//    delete ast;
//    QVERIFY( parseError == true );
    tempFile.remove();
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

QTEST_GUILESS_MAIN( CMakeParserTest )
