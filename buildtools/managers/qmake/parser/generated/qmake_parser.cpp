// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#include "qmake_parser.h"


#include "qmakelexer.h"
#include <kdebug.h>

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
        kDebug(9024) <<  kind <<  "::" <<  tokenText(lexer.getTokenBegin(),  lexer.getTokenEnd()) <<  endl; //" "; // debug output

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
    int line;
    int col;
    size_t index =  token_stream->index();
    token_type &token =  token_stream->token(index);
    token_stream->start_position(index,  &line,  &col);
    size_t tokenLength =  token.end -  token.begin;
    QString tokenValue =  tokenText(token.begin,  token.end);
    reportProblem(
      parser::Error,
      QString("Expected symbol \"%1\" (current token: \"%2\" [%3] at line: %4 col: %5)")
      .arg(name)
      .arg(token.kind !=  0 ?  tokenValue :  "EOF")
      .arg(token.kind)
      .arg(line + 1)
      .arg(col + 1 ) );
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
            stmt_ast *__node_0 =  0;

            if  (!parse_stmt(&__node_0))
              {
                yy_expected_symbol(ast_node::Kind_stmt,  "stmt");
                return  false;
              }

            (*yynode)->stmt_sequence =  snoc((*yynode)->stmt_sequence,  __node_0,  memory_pool);

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

  bool parser::parse_stmt(stmt_ast **yynode)
  {
    *yynode =  create<stmt_ast>();

    (*yynode)->start_token =  token_stream->index() -  1;

    if  (true /*epsilon*/ ||  yytoken ==  Token_NEWLINE)
    {}
    else
      {
        return  false;
      }

    (*yynode)->end_token =  token_stream->index() -  1;

    return  true;
  }


} // end of namespace QMake


