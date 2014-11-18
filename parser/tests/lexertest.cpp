/* KDevelop
 *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>
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

#include "lexertest.h"
#include "qmakelexer.h"
#include "qmakeparser.h"

QTEST_MAIN( LexerTest )

LexerTest::LexerTest( QObject* parent )
    : QObject( parent )
{}

LexerTest::~LexerTest()
{}

void LexerTest::init()
{
}

void LexerTest::cleanup()
{
}

void LexerTest::varAssignment()
{
    QFETCH( QString, project );
    QFETCH( QList<QVariant>, expectedtokens );
    QFETCH( QList<QVariant>, expectedbegins );
    QFETCH( QList<QVariant>, expectedends );
    QMake::Lexer l(0, project);
    QList<QVariant> tokens;
    QList<QVariant> begins;
    QList<QVariant> ends;
    int token = 0;
    do
    {
        token = l.nextTokenKind();
        tokens << token;
        begins << qulonglong(l.tokenBegin());
        ends << qulonglong(l.tokenEnd());
    }while( token != 0 );
    tokens.removeLast();
    begins.removeLast();
    ends.removeLast();

//     qDebug() << project;
//     qDebug() << tokens << expectedtokens;
//     qDebug() << begins << expectedbegins;
//     qDebug() << ends << expectedends;
    QVERIFY( tokens == expectedtokens );
    QVERIFY( begins == expectedbegins );
    QVERIFY( ends == expectedends );
}

void LexerTest::varAssignment_data()
{
    QTest::addColumn<QString>( "project" );
    QTest::addColumn<QList<QVariant> >( "expectedtokens" );
    QTest::addColumn<QList<QVariant> >( "expectedbegins" );
    QTest::addColumn<QList<QVariant> >( "expectedends" );
    QList<QVariant> expectedtokens;
    QList<QVariant> expectedbegins;
    QList<QVariant> expectedends;
    expectedtokens.clear();
    expectedbegins.clear();
    expectedends.clear();
    expectedtokens << QMake::Parser::Token_IDENTIFIER;
    expectedtokens << QMake::Parser::Token_PLUSEQ;
    expectedtokens << QMake::Parser::Token_VALUE;
    expectedbegins << 0 << 4 << 7;
    expectedends << 2 << 5 << 9;
    QTest::newRow( "row1" )
            << "FOO += bar" << expectedtokens << expectedbegins << expectedends;
    expectedtokens.clear();
    expectedbegins.clear();
    expectedends.clear();
    expectedtokens << QMake::Parser::Token_IDENTIFIER;
    expectedtokens << QMake::Parser::Token_PLUSEQ;
    expectedtokens << QMake::Parser::Token_VALUE;
    expectedtokens << QMake::Parser::Token_CONT;
    expectedtokens << QMake::Parser::Token_NEWLINE;
    expectedtokens << QMake::Parser::Token_VALUE;
    expectedbegins << 0 << 4 << 7 << 10 << 11 << 12;
    expectedends << 2 << 5 << 9 << 10 << 11 << 14;
    QTest::newRow( "row2" )
            << "FOO += bar\\\nbaz" << expectedtokens << expectedbegins << expectedends;
    expectedtokens.clear();
    expectedbegins.clear();
    expectedends.clear();
    expectedtokens << QMake::Parser::Token_IDENTIFIER;
    expectedtokens << QMake::Parser::Token_PLUSEQ;
    expectedtokens << QMake::Parser::Token_VALUE;
    expectedtokens << QMake::Parser::Token_CONT;
    expectedtokens << QMake::Parser::Token_NEWLINE;
    expectedtokens << QMake::Parser::Token_CONT;
    expectedtokens << QMake::Parser::Token_NEWLINE;
    expectedtokens << QMake::Parser::Token_VALUE;
    expectedbegins << 0 << 4 << 7 << 14 << 15 << 21 << 22 << 23;
    expectedends << 2 << 5 << 9 << 14 << 15 << 21 << 22 << 25;
    QTest::newRow( "row3" )
            << "FOO += bar    \\\n     \\\nbaz" << expectedtokens << expectedbegins << expectedends;
    expectedtokens.clear();
    expectedbegins.clear();
    expectedends.clear();
    expectedtokens << QMake::Parser::Token_IDENTIFIER;
    expectedtokens << QMake::Parser::Token_PLUSEQ;
    expectedtokens << QMake::Parser::Token_VALUE;
    expectedtokens << QMake::Parser::Token_CONT;
    expectedtokens << QMake::Parser::Token_NEWLINE;
    expectedtokens << QMake::Parser::Token_VALUE;
    expectedbegins << 0 << 4 << 7 << 14 << 20 << 26;
    expectedends << 2 << 5 << 9 << 14 << 20 << 28;
    QTest::newRow( "row4" )
            << "FOO += bar    \\     \n     baz" << expectedtokens << expectedbegins << expectedends;
    expectedtokens.clear();
    expectedbegins.clear();
    expectedends.clear();
    expectedtokens << QMake::Parser::Token_IDENTIFIER;
    expectedtokens << QMake::Parser::Token_PLUSEQ;
    expectedtokens << QMake::Parser::Token_VALUE;
    expectedbegins << 4 << 8 << 11;
    expectedends << 6 << 9 << 13;
    QTest::newRow( "row5" )
            << "    FOO += bar" << expectedtokens << expectedbegins << expectedends;
    expectedtokens.clear();
    expectedbegins.clear();
    expectedends.clear();
    expectedtokens << QMake::Parser::Token_IDENTIFIER;
    expectedtokens << QMake::Parser::Token_PLUSEQ;
    expectedtokens << QMake::Parser::Token_VALUE;
    expectedbegins << 0 << 4 << 7;
    expectedends << 2 << 5 << 9;
    QTest::newRow( "row6" )
            << "FOO += bar#ffoo" << expectedtokens << expectedbegins << expectedends;
    expectedtokens.clear();
    expectedbegins.clear();
    expectedends.clear();
    expectedtokens << QMake::Parser::Token_IDENTIFIER;
    expectedtokens << QMake::Parser::Token_PLUSEQ;
    expectedtokens << QMake::Parser::Token_VALUE;
    expectedtokens << QMake::Parser::Token_CONT;
    expectedtokens << QMake::Parser::Token_NEWLINE;
    expectedbegins << 0 << 4 << 7 << 10 << 11;
    expectedends << 2 << 5 << 9 << 10 << 11;
    QTest::newRow( "row7" )
            << "FOO += bar\\\n#baz" << expectedtokens << expectedbegins << expectedends;
    expectedtokens.clear();
    expectedbegins.clear();
    expectedends.clear();
    expectedtokens << QMake::Parser::Token_IDENTIFIER;
    expectedtokens << QMake::Parser::Token_PLUSEQ;
    expectedtokens << QMake::Parser::Token_VALUE;
    expectedtokens << QMake::Parser::Token_CONT;
    expectedtokens << QMake::Parser::Token_NEWLINE;
    expectedtokens << QMake::Parser::Token_VALUE;
    expectedbegins << 0 << 4 << 7 << 10 << 11 << 17;
    expectedends << 2 << 5 << 9 << 10 << 11 << 19;
    QTest::newRow( "row8" )
            << "FOO += bar\\\n#baz\nbar" << expectedtokens << expectedbegins << expectedends;
    expectedtokens.clear();
    expectedbegins.clear();
    expectedends.clear();
    expectedtokens << QMake::Parser::Token_IDENTIFIER;
    expectedtokens << QMake::Parser::Token_PLUSEQ;
    expectedtokens << QMake::Parser::Token_VALUE;
    expectedtokens << QMake::Parser::Token_CONT;
    expectedtokens << QMake::Parser::Token_NEWLINE;
    expectedtokens << QMake::Parser::Token_VALUE;
    expectedbegins << 0 << 4 << 7 << 10 << 15 << 16;
    expectedends << 2 << 5 << 9 << 10 << 15 << 18;
    QTest::newRow( "row9" )
            << "FOO += bar\\#far\nbaz" << expectedtokens << expectedbegins << expectedends;
    expectedtokens.clear();
    expectedbegins.clear();
    expectedends.clear();
    expectedtokens << QMake::Parser::Token_IDENTIFIER;
    expectedtokens << QMake::Parser::Token_PLUSEQ;
    expectedtokens << QMake::Parser::Token_VALUE;
    expectedtokens << QMake::Parser::Token_VALUE;
    expectedbegins << 0 << 4 << 7 << 11;
    expectedends << 2 << 5 << 9 << 26;
    QTest::newRow( "row10" )
            << "FOO += bar \"fsfsd   sadfsd\"" << expectedtokens << expectedbegins << expectedends;
    expectedtokens.clear();
    expectedbegins.clear();
    expectedends.clear();
    expectedtokens << QMake::Parser::Token_IDENTIFIER;
    expectedtokens << QMake::Parser::Token_PLUSEQ;
    expectedtokens << QMake::Parser::Token_VALUE;
    expectedtokens << QMake::Parser::Token_VALUE;
    expectedbegins << 0 << 4 << 7 << 11;
    expectedends << 2 << 5 << 9 << 18;
    QTest::newRow( "row11" )
            << "FOO += bar \"fsfsd  # sadfsd\"" << expectedtokens << expectedbegins << expectedends;


}

void LexerTest::functions()
{
    QFETCH( QString, project );
    QFETCH( QList<QVariant>, expectedtokens );
    QFETCH( QList<QVariant>, expectedbegins );
    QFETCH( QList<QVariant>, expectedends );
    QMake::Lexer l(0, project);
    QList<QVariant> tokens;
    QList<QVariant> begins;
    QList<QVariant> ends;
    int token = 0;
    do
    {
        token = l.nextTokenKind();
        tokens << token;
        begins << qulonglong(l.tokenBegin());
        ends << qulonglong(l.tokenEnd());
    }while( token != 0 );
    tokens.removeLast();
    begins.removeLast();
    ends.removeLast();

//     qDebug() << project;
//     qDebug() << tokens << expectedtokens;
//     qDebug() << begins << expectedbegins;
//     qDebug() << ends << expectedends;
    QVERIFY( tokens == expectedtokens );
    QVERIFY( begins == expectedbegins );
    QVERIFY( ends == expectedends );
}

void LexerTest::functions_data()
{
    QTest::addColumn<QString>( "project" );
    QTest::addColumn<QList<QVariant> >( "expectedtokens" );
    QTest::addColumn<QList<QVariant> >( "expectedbegins" );
    QTest::addColumn<QList<QVariant> >( "expectedends" );
    QList<QVariant> expectedtokens;
    QList<QVariant> expectedbegins;
    QList<QVariant> expectedends;
    expectedtokens.clear();
    expectedbegins.clear();
    expectedends.clear();
    expectedtokens << QMake::Parser::Token_IDENTIFIER;
    expectedtokens << QMake::Parser::Token_LPAREN;
    expectedtokens << QMake::Parser::Token_RPAREN;
    expectedbegins << 0 << 3 << 4;
    expectedends << 2 << 3 << 4;
    QTest::newRow( "row1" )
            << "foo()" << expectedtokens << expectedbegins << expectedends;
    expectedtokens.clear();
    expectedbegins.clear();
    expectedends.clear();
    expectedtokens << QMake::Parser::Token_IDENTIFIER;
    expectedtokens << QMake::Parser::Token_LPAREN;
    expectedtokens << QMake::Parser::Token_VALUE;
    expectedtokens << QMake::Parser::Token_RPAREN;
    expectedbegins << 0 << 3 << 4 << 8;
    expectedends << 2 << 3 << 7 << 8 ;
    QTest::newRow( "row2" )
            << "foo(arg1)" << expectedtokens << expectedbegins << expectedends;
    expectedtokens.clear();
    expectedbegins.clear();
    expectedends.clear();
    expectedtokens << QMake::Parser::Token_IDENTIFIER;
    expectedtokens << QMake::Parser::Token_LPAREN;
    expectedtokens << QMake::Parser::Token_VALUE;
    expectedtokens << QMake::Parser::Token_COMMA;
    expectedtokens << QMake::Parser::Token_VALUE;
    expectedtokens << QMake::Parser::Token_RPAREN;
    expectedbegins << 0 << 3 << 4 << 8 << 9 << 13;
    expectedends << 2 << 3 << 7 << 8 << 12 << 13;
    QTest::newRow( "row3" )
            << "foo(arg1,arg3)" << expectedtokens << expectedbegins << expectedends;
    expectedtokens.clear();
    expectedbegins.clear();
    expectedends.clear();
    expectedtokens << QMake::Parser::Token_IDENTIFIER;
    expectedtokens << QMake::Parser::Token_LPAREN;
    expectedtokens << QMake::Parser::Token_VALUE;
    expectedtokens << QMake::Parser::Token_RPAREN;
    expectedbegins << 0 << 3 << 4 << 20;
    expectedends << 2 << 3 << 19 << 20;
    QTest::newRow( "row4" )
            << "foo(  arg1   arg2   )" << expectedtokens << expectedbegins << expectedends;
    expectedtokens.clear();
    expectedbegins.clear();
    expectedends.clear();
    expectedtokens << QMake::Parser::Token_IDENTIFIER;
    expectedtokens << QMake::Parser::Token_LPAREN;
    expectedtokens << QMake::Parser::Token_VALUE;
    expectedtokens << QMake::Parser::Token_RPAREN;
    expectedbegins << 0 << 3 << 4 << 22;
    expectedends << 2 << 3 << 21 << 22;
    QTest::newRow( "row5" )
            << "foo(  arg1  \\\n arg2   )" << expectedtokens << expectedbegins << expectedends;
    expectedtokens.clear();
    expectedbegins.clear();
    expectedends.clear();
    expectedtokens << QMake::Parser::Token_IDENTIFIER;
    expectedtokens << QMake::Parser::Token_LPAREN;
    expectedtokens << QMake::Parser::Token_VALUE;
    expectedtokens << QMake::Parser::Token_RPAREN;
    expectedtokens << QMake::Parser::Token_LBRACE;
    expectedbegins << 0 << 3 << 4 << 8 << 10;
    expectedends << 2 << 3 << 7 << 8 << 10;
    QTest::newRow( "row5" )
            << "foo(arg1) {" << expectedtokens << expectedbegins << expectedends;
}


void LexerTest::operators()
{
    QFETCH( QString, project );
    QFETCH( QList<QVariant>, expectedtokens );
    QFETCH( QList<QVariant>, expectedbegins );
    QFETCH( QList<QVariant>, expectedends );
    QMake::Lexer l(0, project);
    QList<QVariant> tokens;
    QList<QVariant> begins;
    QList<QVariant> ends;
    int token = 0;
    do
    {
        token = l.nextTokenKind();
        tokens << token;
        begins << qulonglong(l.tokenBegin());
        ends << qulonglong(l.tokenEnd());
    }while( token != 0 );
    tokens.removeLast();
    begins.removeLast();
    ends.removeLast();

    qDebug() << project;
    qDebug() << tokens << expectedtokens;
    qDebug() << begins << expectedbegins;
    qDebug() << ends << expectedends;
    QVERIFY( tokens == expectedtokens );
    QVERIFY( begins == expectedbegins );
    QVERIFY( ends == expectedends );
}

void LexerTest::operators_data()
{
    QTest::addColumn<QString>( "project" );
    QTest::addColumn<QList<QVariant> >( "expectedtokens" );
    QTest::addColumn<QList<QVariant> >( "expectedbegins" );
    QTest::addColumn<QList<QVariant> >( "expectedends" );
    QList<QVariant> expectedtokens;
    QList<QVariant> expectedbegins;
    QList<QVariant> expectedends;
    expectedtokens.clear();
    expectedbegins.clear();
    expectedends.clear();
    expectedtokens << QMake::Parser::Token_IDENTIFIER;
    expectedtokens << QMake::Parser::Token_EQUAL;
    expectedtokens << QMake::Parser::Token_VALUE;
    expectedbegins << 0 << 4 << 6;
    expectedends << 2 << 4 << 8;
    QTest::newRow( "row1" )
            << "foo = bar" << expectedtokens << expectedbegins << expectedends;
    expectedtokens.clear();
    expectedbegins.clear();
    expectedends.clear();
    expectedtokens << QMake::Parser::Token_IDENTIFIER;
    expectedtokens << QMake::Parser::Token_PLUSEQ;
    expectedtokens << QMake::Parser::Token_VALUE;
    expectedbegins << 0 << 4 << 7;
    expectedends << 2 << 5 << 9;
    QTest::newRow( "row2" )
            << "foo += bar" << expectedtokens << expectedbegins << expectedends;
    expectedtokens.clear();
    expectedbegins.clear();
    expectedends.clear();
    expectedtokens << QMake::Parser::Token_IDENTIFIER;
    expectedtokens << QMake::Parser::Token_MINUSEQ;
    expectedtokens << QMake::Parser::Token_VALUE;
    expectedbegins << 0 << 4 << 7;
    expectedends << 2 << 5 << 9;
    QTest::newRow( "row3" )
            << "foo -= bar" << expectedtokens << expectedbegins << expectedends;
    expectedtokens.clear();
    expectedbegins.clear();
    expectedends.clear();
    expectedtokens << QMake::Parser::Token_IDENTIFIER;
    expectedtokens << QMake::Parser::Token_TILDEEQ;
    expectedtokens << QMake::Parser::Token_VALUE;
    expectedbegins << 0 << 4 << 7;
    expectedends << 2 << 5 << 9;
    QTest::newRow( "row4" )
            << "foo ~= bar" << expectedtokens << expectedbegins << expectedends;
    expectedtokens.clear();
    expectedbegins.clear();
    expectedends.clear();
    expectedtokens << QMake::Parser::Token_IDENTIFIER;
    expectedtokens << QMake::Parser::Token_STAREQ;
    expectedtokens << QMake::Parser::Token_VALUE;
    expectedbegins << 0 << 4 << 7;
    expectedends << 2 << 5 << 9;
    QTest::newRow( "row5" )
            << "foo *= bar" << expectedtokens << expectedbegins << expectedends;

}

void LexerTest::scope()
{
    QFETCH( QString, project );
    QFETCH( QList<QVariant>, expectedtokens );
    QFETCH( QList<QVariant>, expectedbegins );
    QFETCH( QList<QVariant>, expectedends );
    QMake::Lexer l(0, project);
    QList<QVariant> tokens;
    QList<QVariant> begins;
    QList<QVariant> ends;
    int token = 0;
    do
    {
        token = l.nextTokenKind();
        tokens << token;
        begins << qulonglong(l.tokenBegin());
        ends << qulonglong(l.tokenEnd());
    }while( token != 0 );
    tokens.removeLast();
    begins.removeLast();
    ends.removeLast();

//     qDebug() << project;
//     qDebug() << tokens << expectedtokens;
//     qDebug() << begins << expectedbegins;
//     qDebug() << ends << expectedends;
    QVERIFY( tokens == expectedtokens );
    QVERIFY( begins == expectedbegins );
    QVERIFY( ends == expectedends );
}

void LexerTest::scope_data()
{
    QTest::addColumn<QString>( "project" );
    QTest::addColumn<QList<QVariant> >( "expectedtokens" );
    QTest::addColumn<QList<QVariant> >( "expectedbegins" );
    QTest::addColumn<QList<QVariant> >( "expectedends" );
    QList<QVariant> expectedtokens;
    QList<QVariant> expectedbegins;
    QList<QVariant> expectedends;
    expectedtokens.clear();
    expectedbegins.clear();
    expectedends.clear();
    expectedtokens << QMake::Parser::Token_IDENTIFIER;
    expectedtokens << QMake::Parser::Token_LBRACE;
    expectedbegins << 0 << 3;
    expectedends << 2 << 3;
    QTest::newRow( "row1" )
            << "foo{" << expectedtokens << expectedbegins << expectedends;
}
