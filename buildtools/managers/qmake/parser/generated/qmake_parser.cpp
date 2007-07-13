// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#include "qmake_parser.h"


#include "qmakelexer.h"
#include <kdebug.h>
#include <QtCore/QString>

namespace QMake
  {

  void parser::tokenize( const QString& contents )
  {
    m_contents =  contents;
    QMake::Lexer lexer( this,  contents );
    int kind =  parser::Token_EOF;

    do
      {
        kind =  lexer.getNextTokenKind();
        kDebug(9024) <<  kind <<  "(" <<  lexer.getTokenBegin() <<  "," <<  lexer.getTokenEnd() <<  ")::" <<  tokenText(lexer.getTokenBegin(),  lexer.getTokenEnd()) <<  endl; //" "; // debug output

        if  ( !kind ) // when the lexer returns 0, the end of file is reached
          kind =  parser::Token_EOF;

        parser::token_type &t =  this->token_stream->next();
        t.kind =  kind;
        t.begin =  lexer.getTokenBegin();
        t.end =  lexer.getTokenEnd();
      }

    while  ( kind !=  parser::Token_EOF );

    this->yylex(); // produce the look ahead token
  }

  QString parser::tokenText( std::size_t begin,  std::size_t end ) const
    {
      return  m_contents.mid((int)begin,  (int)end - begin);
    }

  void parser::reportProblem( parser::ProblemType type,  const QString& message )
  {
    if  (type ==  Error)
      kDebug(9024) <<  "** ERROR: " <<  message <<  endl;
    else if  (type ==  Warning)
      kDebug(9024) <<  "** WARNING: " <<  message <<  endl;
    else if  (type ==  Info)
      kDebug(9024) <<  "** Info: " <<  message <<  endl;
  }


  // custom error recovery
  void parser::yy_expected_token(int /*expected*/,  std::size_t /*where*/,  char const *name)
  {
    reportProblem(
      parser::Error,
      QString("Expected token \"%1\"").arg(name));
  }

  void parser::yy_expected_symbol(int /*expected_symbol*/,  char const *name)
  {
    std::size_t line;
    std::size_t col;
    size_t index =  token_stream->index();
    token_type &token =  token_stream->token(index);
    kDebug(9024) <<  "token starts at: " <<  token.begin <<  endl;
    kDebug(9024) <<  "index is: " <<  index <<  endl;
    token_stream->start_position(index,  &line,  &col);
    size_t tokenLength =  token.end -  token.begin;
    QString tokenValue =  tokenText(token.begin,  token.end);
    reportProblem(
      parser::Error,
      QString("Expected symbol \"%1\" (current token: \"%2\" [%3] at line: %4 col: %5)")
      .arg(name)
      .arg(token.kind !=  0 ?  tokenValue :  "EOF")
      .arg(token.kind)
      .arg(line)
      .arg(col));
  }


} // end of namespace QMake


namespace QMake
  {

  bool parser::parse_project(project_ast **yynode)
  {
    *yynode =  create<project_ast>();

    (*yynode)->start_token =  token_stream->index() -  1;

    if  (yytoken ==  Token_NEWLINE ||  yytoken ==  Token_EOF)
      {
        while  (yytoken ==  Token_NEWLINE)
          {
            if  (yytoken !=  Token_NEWLINE)
              {
                yy_expected_token(yytoken,  Token_NEWLINE,  "newline");
                return  false;
              }

            yylex();

          }

        if  (Token_EOF !=  yytoken)
          {
            return  false;
          }
      }

    else
      {
        return  false;
      }

    (*yynode)->end_token =  token_stream->index() -  1;

    return  true;
  }


} // end of namespace QMake


