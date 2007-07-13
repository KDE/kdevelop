-------------------------------------------------------------------------------
-- This file is part of the QMake parser in KDevelop4
-- Copyright (c) 2007 Andreas Pakulat <apaku@gmx.de>
--
-- Permission is hereby granted, free of charge, to any person obtaining
-- a copy of this software and associated documentation files (the
-- "Software"), to deal in the Software without restriction, including
-- without limitation the rights to use, copy, modify, merge, publish,
-- distribute, sublicense, and/or sell copies of the Software, and to
-- permit persons to whom the Software is furnished to do so, subject to
-- the following conditions:
--
-- The above copyright notice and this permission notice shall be
-- included in all copies or substantial portions of the Software.
--
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
-- EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
-- MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
-- NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
-- LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
-- OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
-- WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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

%token LBRACKET("lbracket"),RBRACKET("rbracket"),
       LBRACE("lbrace"),RBRACE("rbrace"),LPAREN("lparen"),RPAREN("rparen") ;;

%token PLUSEQ("pluseq"),EQUAL("equal"),MINUSEQ("minuseq"),STAREQ("stareq"),
       TILDEEQ("tildeeq") ;;

%token COLON("colon"), COMMA("comma"), CONT("cont"),
       NEWLINE("newline"), DOUBLEDOLLAR("doubledollar"),
       SINGLEDOLLAR("singledollar") ;;

%token IDENTIFIER("identifier"), VALUE("value"),
       QUOTEDSPACE("quotedspace") ;;

%token ERROR("error") ;;


-- token that makes the parser fail in any case:
%token INVALID ("invalid token") ;;

-- The actual grammar starts here.

   ( NEWLINE )*
-> project ;;


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
        kDebug(9024) << kind << "(" << lexer.getTokenBegin() << "," << lexer.getTokenEnd() << ")::" << tokenText(lexer.getTokenBegin(), lexer.getTokenEnd()) << endl; //" "; // debug output

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
    return m_contents.mid((int)begin, (int)end-begin);
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
    size_t tokenLength = token.end - token.begin;
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

