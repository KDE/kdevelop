-------------------------------------------------------------------------------
-- This file is part of the QMake parser in KDevelop4
-- SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
--
-- SPDX-License-Identifier: GPL-2.0-or-later
-------------------------------------------------------------------------------

-----------------------------------------------------------
-- Global  declarations
-----------------------------------------------------------


[:

namespace QMake
{
    class Lexer;
}

#include <QString>

:]

%parser_declaration_header "QtCore/QString"


------------------------------------------------------------
-- Parser class members
------------------------------------------------------------

%parserclass (public declaration)
[:
    /**
     * Transform the raw input into tokens.
     * When this method returns, the parser's token stream has been filled
     * and any parse_*() method can be called.
     */
    void tokenize( const QString& contents );

    enum ProblemType {
        Error,
        Warning,
        Info
    };
    void reportProblem( Parser::ProblemType type, const QString& message );

    QString tokenText(qint64 begin, qint64 end) const;

    void setDebug( bool debug );

:]

%parserclass (private declaration)
[:
    QString m_contents;
    bool m_debug;
:]
-----------------------------------------------------------
-- List of defined tokens
-----------------------------------------------------------

%token LBRACE("lbrace"), RBRACE("rbrace"), LPAREN("lparen"),RPAREN("rparen") ;;

%token PLUSEQ("pluseq"),EQUAL("equal"),MINUSEQ("minuseq"),STAREQ("stareq"),
       TILDEEQ("tildeeq") ;;

%token COLON("colon"), COMMA("comma"), CONT("cont"), EXCLAM("exclam"),
       NEWLINE("newline"), OR("or") ;;

%token IDENTIFIER("identifier"), VALUE("value") ;;

-- token that makes the parser fail in any case:
%token INVALID ("invalid token") ;;

-- The actual grammar starts here.

   ( #statements=statement )*
-> project ;;

   ( id=IDENTIFIER ( var=variableAssignment | scope=scope )
            [:
                (*yynode)->isNewline = false;
                (*yynode)->isExclam = false;
            :]
   ) | ( EXCLAM id=IDENTIFIER scope=scope
            [:
               (*yynode)->isNewline = false;
               (*yynode)->isExclam = true;
            :]
    ) | NEWLINE
            [:
                (*yynode)->isNewline = true;
                (*yynode)->isExclam = false;
            :]
-> statement [ member variable isNewline: bool;
          member variable isExclam: bool; ] ;;

   functionArguments=functionArguments ( scopeBody=scopeBody | orOperator=orOperator scopeBody=scopeBody | 0 )
   | ( orOperator=orOperator | 0 ) scopeBody=scopeBody
-> scope ;;

   ( OR #item=item )+
-> orOperator ;;

   id=IDENTIFIER ( functionArguments=functionArguments | 0 )
-> item ;;

   op=op ( values=valueList ( NEWLINE | 0 ) | NEWLINE | 0 )
-> variableAssignment ;;

   optoken=PLUSEQ | optoken=MINUSEQ | optoken=STAREQ | optoken=EQUAL | optoken=TILDEEQ
-> op ;;

   ( #list=value | CONT NEWLINE )+
-> valueList ;;

   value=VALUE
-> value ;;

   LPAREN args=argumentList RPAREN
-> functionArguments ;;

   ( ( #args=value | CONT NEWLINE ) ( ( COMMA | CONT NEWLINE ) #args=value )* | 0 )
-> argumentList ;;

    LBRACE ( NEWLINE | 0 ) ( #statements=statement )* RBRACE | COLON #statements=statement
-> scopeBody ;;

-----------------------------------------------------------------
-- Code segments copied to the implementation (.cpp) file.
-- If existent, kdevelop-pg's current syntax requires this block
-- to occur at the end of the file.
-----------------------------------------------------------------

[:
#include "qmakelexer.h"
#include <QString>
#include <debug.h>

namespace QMake
{

void Parser::tokenize( const QString& contents )
{
    m_contents = contents;
    QMake::Lexer lexer( this, contents );
    int kind = Parser::Token_EOF;

    do
    {
        kind = lexer.nextTokenKind();

        if ( !kind ) // when the lexer returns 0, the end of file is reached
            kind = Parser::Token_EOF;

        Parser::Token &t = this->tokenStream->push();
        t.kind = kind;
        if( t.kind == Parser::Token_EOF )
        {
            t.begin = -1;
            t.end = -1;
        }else
        {
            t.begin = lexer.tokenBegin();
            t.end = lexer.tokenEnd();
        }

        if( m_debug )
        {
            qCDebug(KDEV_QMAKE) << kind << "(" << t.begin << "," << t.end << ")::" << tokenText(t.begin, t.end) << (tokenStream->size()-1);
        }

    }
    while ( kind != Parser::Token_EOF );

    this->yylex(); // produce the look ahead token
}

QString Parser::tokenText( qint64 begin, qint64 end ) const
{
    return m_contents.mid((int)begin, (int)end-begin+1);
}

void Parser::reportProblem( Parser::ProblemType type, const QString& message )
{
    if (type == Error)
        qCDebug(KDEV_QMAKE) << "** ERROR:" << message;
    else if (type == Warning)
        qCDebug(KDEV_QMAKE) << "** WARNING:" << message;
    else if (type == Info)
        qCDebug(KDEV_QMAKE) << "** Info:" << message;
}


// custom error recovery
void Parser::expectedToken(int actualToken, qint64 expectedToken, const QString& name)
{
    qint64 line;
    qint64 col;
    size_t index = tokenStream->index()-1;
    tokenStream->startPosition(index, &line, &col);

    reportProblem(
        Parser::Error,
        QStringLiteral("Expected token \"%1\" (%2) instead of %3 at line: %4 col: %5, token index %6")
            .arg(name, QString::number(expectedToken), QString::number(actualToken),
                 QString::number(line), QString::number(col), QString::number(index)));
}

void Parser::expectedSymbol(int /*expected_symbol*/, const QString& name)
{
    qint64 line;
    qint64 col;
    size_t index = tokenStream->index()-1;
    Token &token = tokenStream->at(index);
    qCDebug(KDEV_QMAKE) << "token starts at:" << token.begin;
    qCDebug(KDEV_QMAKE) << "index is:" << index;
    tokenStream->startPosition(index, &line, &col);
    QString tokenValue = tokenText(token.begin, token.end);
    reportProblem(
        Parser::Error,
        QStringLiteral("Expected symbol \"%1\" (current token: \"%2\" [%3] at line: %4 col: %5)")
            .arg(name, token.kind != 0 ? tokenValue : QStringLiteral("EOF"))
            .arg(token.kind)
            .arg(line)
            .arg(col));
}

void Parser::setDebug( bool debug )
{
    m_debug = debug;
}

} // end of namespace QMake

:]
