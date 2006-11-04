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

#include <QtCore/QTemporaryFile>
#include "cmListFileLexer.h"

QTEST_MAIN( CMakeParserTest )

CMakeParserTest::CMakeParserTest()
{
}


CMakeParserTest::~CMakeParserTest()
{
}

void CMakeParserTest::testLexerCreation()
{
    cmListFileLexer* lexer = cmListFileLexer_New();
    QVERIFY( lexer != 0 );
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
    QVERIFY( cmListFileLexer_SetFileName( lexer, qPrintable( tempName ) ) );
    cmListFileLexer_Delete( lexer );
    tempFile.remove();
}

#include "cmakeparsertest.moc"

