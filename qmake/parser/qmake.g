-------------------------------------------------------------------------------
-- This file is part of the QMake parser in KDevelop4
-- Copyright (c) 2007 Andreas Pakulat <apaku@gmx.de>
--
-- This program is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License
-- as published by the Free Software Foundation; either version 2
-- of the License, or (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program; if not, write to the Free Software
-- Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
-- 02110-1301, USA.
-------------------------------------------------------------------------------

-----------------------------------------------------------
-- Global  declarations
-----------------------------------------------------------


[:

#include <QtCore/QString>

namespace QMake
{
    class Lexer;
}

:]

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
    void reportProblem( parser::ProblemType type, const QString& message );

    QString tokenText(std::size_t begin, std::size_t end) const;

:]

%parserclass (private declaration)
[:
    QString m_contents;
:]
-----------------------------------------------------------
-- List of defined tokens
-----------------------------------------------------------

%token LBRACE("lbrace"), RBRACE("rbrace"), LPAREN("lparen"),RPAREN("rparen") ;;

%token PLUSEQ("pluseq"),EQUAL("equal"),MINUSEQ("minuseq"),STAREQ("stareq"),
       TILDEEQ("tildeeq") ;;

%token COLON("colon"), COMMA("comma"), CONT("cont"), EXCLAM("exclam"),
       NEWLINE("newline"), OR("or") ;;

%token IDENTIFIER("identifier"), VALUE("value"),
       QUOTEDVALUE("quotedvalue") ;;

-- token that makes the parser fail in any case:
%token INVALID ("invalid token") ;;

-- The actual grammar starts here.

   ( #stmts=stmt )*
-> project ;;

   ( id=IDENTIFIER ( var=variable_assignment | scope=scope )
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
-> stmt [ member variable isNewline: bool;
          member variable isExclam: bool; ] ;;

   func_args=function_args ( scope_body=scope_body | or_op=or_op scope_body=scope_body | 0 )
   | ( or_op=or_op | 0 ) scope_body=scope_body
-> scope ;;

   ( OR #item=item )+
-> or_op ;;

   id=IDENTIFIER ( func_args=function_args | 0 )
-> item ;;

   op=op ( values=value_list | 0 ) ( NEWLINE | EOF )
-> variable_assignment ;;

   optoken=PLUSEQ | optoken=MINUSEQ | optoken=STAREQ | optoken=EQUAL | optoken=TILDEEQ
-> op ;;

   ( #list=value | CONT NEWLINE )+
-> value_list ;;

   value=VALUE | value=QUOTEDVALUE
-> value ;;

   LPAREN args=arg_list RPAREN
-> function_args ;;

   ( ( #args=value | CONT NEWLINE ) ( ( COMMA | CONT NEWLINE ) #args=value )* | 0 )
-> arg_list ;;

    LBRACE ( NEWLINE | 0 ) ( #stmts=stmt )* RBRACE | COLON #stmts=stmt
-> scope_body ;;

-----------------------------------------------------------------
-- Code segments copied to the implementation (.cpp) file.
-- If existent, kdevelop-pg's current syntax requires this block
-- to occur at the end of the file.
-----------------------------------------------------------------

[:
#include "qmakelexer.h"
#include <kdebug.h>
#include <QtCore/QString>

namespace QMake
{

void parser::tokenize( const QString& contents )
{
    m_contents = contents;
    QMake::Lexer lexer( this, contents );
    int kind = parser::Token_EOF;

    do
    {
        kind = lexer.getNextTokenKind();
        kDebug(9024) << kind << "(" << lexer.getTokenBegin() << "," << lexer.getTokenEnd() << ")::" << tokenText(lexer.getTokenBegin(), lexer.getTokenEnd()) << "::"<< endl; //" "; // debug output

        if ( !kind ) // when the lexer returns 0, the end of file is reached
            kind = parser::Token_EOF;

        parser::token_type &t = this->token_stream->next();
        t.kind = kind;
        t.begin = lexer.getTokenBegin();
        t.end = lexer.getTokenEnd();
    }
    while ( kind != parser::Token_EOF );

    this->yylex(); // produce the look ahead token
}

QString parser::tokenText( std::size_t begin, std::size_t end ) const
{
    return m_contents.mid((int)begin, (int)end-begin+1);
}

void parser::reportProblem( parser::ProblemType type, const QString& message )
{
    if (type == Error)
        kDebug(9024) << "** ERROR: " << message << endl;
    else if (type == Warning)
        kDebug(9024) << "** WARNING: " << message << endl;
    else if (type == Info)
        kDebug(9024) << "** Info: " << message << endl;
}


// custom error recovery
void parser::yy_expected_token(int /*expected*/, std::size_t /*where*/, char const *name)
{
    reportProblem(
        parser::Error,
        QString("Expected token \"%1\"").arg(name));
}

void parser::yy_expected_symbol(int /*expected_symbol*/, char const *name)
{
    std::size_t line;
    std::size_t col;
    size_t index = token_stream->index()-1;
    token_type &token = token_stream->token(index);
    kDebug(9024) << "token starts at: " << token.begin << endl;
    kDebug(9024) << "index is: " << index << endl;
    token_stream->start_position(index, &line, &col);
    QString tokenValue = tokenText(token.begin, token.end);
    reportProblem(
        parser::Error,
        QString("Expected symbol \"%1\" (current token: \"%2\" [%3] at line: %4 col: %5)")
            .arg(name)
            .arg(token.kind != 0 ? tokenValue : "EOF")
            .arg(token.kind)
            .arg(line)
            .arg(col));
}


} // end of namespace QMake

:]

-- kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on

