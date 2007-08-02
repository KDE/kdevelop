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

        if  ( !kind ) // when the lexer returns 0, the end of file is reached
          kind =  parser::Token_EOF;

        parser::token_type &t =  this->token_stream->next();
        t.kind =  kind;
        if ( t.kind ==  parser::Token_EOF )
          {
            t.begin =  m_contents.size();
            t.end =  m_contents.size();
          }

        else
          {
            t.begin =  lexer.getTokenBegin();
            t.end =  lexer.getTokenEnd();
          }

        if ( m_debug )
          {
            kDebug(9024) <<  kind <<  "(" <<  t.begin <<  "," <<  t.end <<  ")::" <<  tokenText(t.begin,  t.end) <<  "::"; //" "; // debug output
          }

      }

    while  ( kind !=  parser::Token_EOF );

    this->yylex(); // produce the look ahead token
  }

  QString parser::tokenText( std::size_t begin,  std::size_t end ) const
    {
      return  m_contents.mid((int)begin,  (int)end - begin + 1);
    }

  void parser::reportProblem( parser::ProblemType type,  const QString& message )
  {
    if  (type ==  Error)
      kDebug(9024) <<  "** ERROR:" <<  message;
    else if  (type ==  Warning)
      kDebug(9024) <<  "** WARNING:" <<  message;
    else if  (type ==  Info)
      kDebug(9024) <<  "** Info:" <<  message;
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
    size_t index =  token_stream->index() - 1;
    token_type &token =  token_stream->token(index);
    kDebug(9024) <<  "token starts at:" <<  token.begin;
    kDebug(9024) <<  "index is:" <<  index;
    token_stream->start_position(index,  &line,  &col);
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

  void parser::setDebug( bool debug )
  {
    m_debug =  debug;
  }

} // end of namespace QMake


namespace QMake
  {

  bool parser::parse_arg_list(arg_list_ast **yynode)
  {
    *yynode =  create<arg_list_ast>();

    (*yynode)->start_token =  token_stream->index() -  1;

    if  (yytoken ==  Token_CONT
         ||  yytoken ==  Token_VALUE ||  yytoken ==  Token_RPAREN)
      {
        if  (yytoken ==  Token_CONT
             ||  yytoken ==  Token_VALUE)
          {
            if  (yytoken ==  Token_VALUE)
              {
                value_ast *__node_0 =  0;

                if  (!parse_value(&__node_0))
                  {
                    yy_expected_symbol(ast_node::Kind_value,  "value");
                    return  false;
                  }

                (*yynode)->args_sequence =  snoc((*yynode)->args_sequence,  __node_0,  memory_pool);

              }

            else if  (yytoken ==  Token_CONT)
              {
                if  (yytoken !=  Token_CONT)
                  {
                    yy_expected_token(yytoken,  Token_CONT,  "cont");
                    return  false;
                  }

                yylex();

                if  (yytoken !=  Token_NEWLINE)
                  {
                    yy_expected_token(yytoken,  Token_NEWLINE,  "newline");
                    return  false;
                  }

                yylex();

              }

            else
              {
                return  false;
              }

            while  (yytoken ==  Token_COMMA
                    ||  yytoken ==  Token_CONT)
              {
                if  (yytoken ==  Token_COMMA)
                  {
                    if  (yytoken !=  Token_COMMA)
                      {
                        yy_expected_token(yytoken,  Token_COMMA,  "comma");
                        return  false;
                      }

                    yylex();

                  }

                else if  (yytoken ==  Token_CONT)
                  {
                    if  (yytoken !=  Token_CONT)
                      {
                        yy_expected_token(yytoken,  Token_CONT,  "cont");
                        return  false;
                      }

                    yylex();

                    if  (yytoken !=  Token_NEWLINE)
                      {
                        yy_expected_token(yytoken,  Token_NEWLINE,  "newline");
                        return  false;
                      }

                    yylex();

                  }

                else
                  {
                    return  false;
                  }

                value_ast *__node_1 =  0;

                if  (!parse_value(&__node_1))
                  {
                    yy_expected_symbol(ast_node::Kind_value,  "value");
                    return  false;
                  }

                (*yynode)->args_sequence =  snoc((*yynode)->args_sequence,  __node_1,  memory_pool);

              }
          }

        else if  (true /*epsilon*/)
        {}
        else
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

  bool parser::parse_function_args(function_args_ast **yynode)
  {
    *yynode =  create<function_args_ast>();

    (*yynode)->start_token =  token_stream->index() -  1;

    if  (yytoken ==  Token_LPAREN)
      {
        if  (yytoken !=  Token_LPAREN)
          {
            yy_expected_token(yytoken,  Token_LPAREN,  "lparen");
            return  false;
          }

        yylex();

        arg_list_ast *__node_2 =  0;

        if  (!parse_arg_list(&__node_2))
          {
            yy_expected_symbol(ast_node::Kind_arg_list,  "arg_list");
            return  false;
          }

        (*yynode)->args =  __node_2;

        if  (yytoken !=  Token_RPAREN)
          {
            yy_expected_token(yytoken,  Token_RPAREN,  "rparen");
            return  false;
          }

        yylex();

      }

    else
      {
        return  false;
      }

    (*yynode)->end_token =  token_stream->index() -  1;

    return  true;
  }

  bool parser::parse_item(item_ast **yynode)
  {
    *yynode =  create<item_ast>();

    (*yynode)->start_token =  token_stream->index() -  1;

    if  (yytoken ==  Token_IDENTIFIER)
      {
        if  (yytoken !=  Token_IDENTIFIER)
          {
            yy_expected_token(yytoken,  Token_IDENTIFIER,  "identifier");
            return  false;
          }

        (*yynode)->id =  token_stream->index() -  1;
        yylex();

        if  (yytoken ==  Token_LPAREN)
          {
            function_args_ast *__node_3 =  0;

            if  (!parse_function_args(&__node_3))
              {
                yy_expected_symbol(ast_node::Kind_function_args,  "function_args");
                return  false;
              }

            (*yynode)->func_args =  __node_3;

          }

        else if  (true /*epsilon*/)
        {}
        else
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

  bool parser::parse_op(op_ast **yynode)
  {
    *yynode =  create<op_ast>();

    (*yynode)->start_token =  token_stream->index() -  1;

    if  (yytoken ==  Token_PLUSEQ
         ||  yytoken ==  Token_EQUAL
         ||  yytoken ==  Token_MINUSEQ
         ||  yytoken ==  Token_STAREQ
         ||  yytoken ==  Token_TILDEEQ)
      {
        if  (yytoken ==  Token_PLUSEQ)
          {
            if  (yytoken !=  Token_PLUSEQ)
              {
                yy_expected_token(yytoken,  Token_PLUSEQ,  "pluseq");
                return  false;
              }

            (*yynode)->optoken =  token_stream->index() -  1;
            yylex();

          }

        else if  (yytoken ==  Token_MINUSEQ)
          {
            if  (yytoken !=  Token_MINUSEQ)
              {
                yy_expected_token(yytoken,  Token_MINUSEQ,  "minuseq");
                return  false;
              }

            (*yynode)->optoken =  token_stream->index() -  1;
            yylex();

          }

        else if  (yytoken ==  Token_STAREQ)
          {
            if  (yytoken !=  Token_STAREQ)
              {
                yy_expected_token(yytoken,  Token_STAREQ,  "stareq");
                return  false;
              }

            (*yynode)->optoken =  token_stream->index() -  1;
            yylex();

          }

        else if  (yytoken ==  Token_EQUAL)
          {
            if  (yytoken !=  Token_EQUAL)
              {
                yy_expected_token(yytoken,  Token_EQUAL,  "equal");
                return  false;
              }

            (*yynode)->optoken =  token_stream->index() -  1;
            yylex();

          }

        else if  (yytoken ==  Token_TILDEEQ)
          {
            if  (yytoken !=  Token_TILDEEQ)
              {
                yy_expected_token(yytoken,  Token_TILDEEQ,  "tildeeq");
                return  false;
              }

            (*yynode)->optoken =  token_stream->index() -  1;
            yylex();

          }

        else
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

  bool parser::parse_or_op(or_op_ast **yynode)
  {
    *yynode =  create<or_op_ast>();

    (*yynode)->start_token =  token_stream->index() -  1;

    if  (yytoken ==  Token_OR)
      {
        do
          {
            if  (yytoken !=  Token_OR)
              {
                yy_expected_token(yytoken,  Token_OR,  "or");
                return  false;
              }

            yylex();

            item_ast *__node_4 =  0;

            if  (!parse_item(&__node_4))
              {
                yy_expected_symbol(ast_node::Kind_item,  "item");
                return  false;
              }

            (*yynode)->item_sequence =  snoc((*yynode)->item_sequence,  __node_4,  memory_pool);

          }

        while  (yytoken ==  Token_OR);
      }

    else
      {
        return  false;
      }

    (*yynode)->end_token =  token_stream->index() -  1;

    return  true;
  }

  bool parser::parse_project(project_ast **yynode)
  {
    *yynode =  create<project_ast>();

    (*yynode)->start_token =  token_stream->index() -  1;

    if  (yytoken ==  Token_EXCLAM
         ||  yytoken ==  Token_NEWLINE
         ||  yytoken ==  Token_IDENTIFIER ||  yytoken ==  Token_EOF)
      {
        while  (yytoken ==  Token_EXCLAM
                ||  yytoken ==  Token_NEWLINE
                ||  yytoken ==  Token_IDENTIFIER)
          {
            stmt_ast *__node_5 =  0;

            if  (!parse_stmt(&__node_5))
              {
                yy_expected_symbol(ast_node::Kind_stmt,  "stmt");
                return  false;
              }

            (*yynode)->stmts_sequence =  snoc((*yynode)->stmts_sequence,  __node_5,  memory_pool);

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

  bool parser::parse_scope(scope_ast **yynode)
  {
    *yynode =  create<scope_ast>();

    (*yynode)->start_token =  token_stream->index() -  1;

    if  (yytoken ==  Token_LBRACE
         ||  yytoken ==  Token_LPAREN
         ||  yytoken ==  Token_COLON
         ||  yytoken ==  Token_OR)
      {
        if  (yytoken ==  Token_LPAREN)
          {
            function_args_ast *__node_6 =  0;

            if  (!parse_function_args(&__node_6))
              {
                yy_expected_symbol(ast_node::Kind_function_args,  "function_args");
                return  false;
              }

            (*yynode)->func_args =  __node_6;

            if  (yytoken ==  Token_LBRACE
                 ||  yytoken ==  Token_COLON)
              {
                scope_body_ast *__node_7 =  0;

                if  (!parse_scope_body(&__node_7))
                  {
                    yy_expected_symbol(ast_node::Kind_scope_body,  "scope_body");
                    return  false;
                  }

                (*yynode)->scope_body =  __node_7;

              }

            else if  (yytoken ==  Token_OR)
              {
                or_op_ast *__node_8 =  0;

                if  (!parse_or_op(&__node_8))
                  {
                    yy_expected_symbol(ast_node::Kind_or_op,  "or_op");
                    return  false;
                  }

                (*yynode)->or_op =  __node_8;

                scope_body_ast *__node_9 =  0;

                if  (!parse_scope_body(&__node_9))
                  {
                    yy_expected_symbol(ast_node::Kind_scope_body,  "scope_body");
                    return  false;
                  }

                (*yynode)->scope_body =  __node_9;

              }

            else if  (true /*epsilon*/)
            {}
            else
              {
                return  false;
              }
          }

        else if  (yytoken ==  Token_LBRACE
                  ||  yytoken ==  Token_COLON
                  ||  yytoken ==  Token_OR)
          {
            if  (yytoken ==  Token_OR)
              {
                or_op_ast *__node_10 =  0;

                if  (!parse_or_op(&__node_10))
                  {
                    yy_expected_symbol(ast_node::Kind_or_op,  "or_op");
                    return  false;
                  }

                (*yynode)->or_op =  __node_10;

              }

            else if  (true /*epsilon*/)
            {}
            else
              {
                return  false;
              }

            scope_body_ast *__node_11 =  0;

            if  (!parse_scope_body(&__node_11))
              {
                yy_expected_symbol(ast_node::Kind_scope_body,  "scope_body");
                return  false;
              }

            (*yynode)->scope_body =  __node_11;

          }

        else
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

  bool parser::parse_scope_body(scope_body_ast **yynode)
  {
    *yynode =  create<scope_body_ast>();

    (*yynode)->start_token =  token_stream->index() -  1;

    if  (yytoken ==  Token_LBRACE
         ||  yytoken ==  Token_COLON)
      {
        if  (yytoken ==  Token_LBRACE)
          {
            if  (yytoken !=  Token_LBRACE)
              {
                yy_expected_token(yytoken,  Token_LBRACE,  "lbrace");
                return  false;
              }

            yylex();

            if  (yytoken ==  Token_NEWLINE)
              {
                if  (yytoken !=  Token_NEWLINE)
                  {
                    yy_expected_token(yytoken,  Token_NEWLINE,  "newline");
                    return  false;
                  }

                yylex();

              }

            else if  (true /*epsilon*/)
            {}
            else
              {
                return  false;
              }

            while  (yytoken ==  Token_EXCLAM
                    ||  yytoken ==  Token_NEWLINE
                    ||  yytoken ==  Token_IDENTIFIER)
              {
                stmt_ast *__node_12 =  0;

                if  (!parse_stmt(&__node_12))
                  {
                    yy_expected_symbol(ast_node::Kind_stmt,  "stmt");
                    return  false;
                  }

                (*yynode)->stmts_sequence =  snoc((*yynode)->stmts_sequence,  __node_12,  memory_pool);

              }

            if  (yytoken !=  Token_RBRACE)
              {
                yy_expected_token(yytoken,  Token_RBRACE,  "rbrace");
                return  false;
              }

            yylex();

          }

        else if  (yytoken ==  Token_COLON)
          {
            if  (yytoken !=  Token_COLON)
              {
                yy_expected_token(yytoken,  Token_COLON,  "colon");
                return  false;
              }

            yylex();

            stmt_ast *__node_13 =  0;

            if  (!parse_stmt(&__node_13))
              {
                yy_expected_symbol(ast_node::Kind_stmt,  "stmt");
                return  false;
              }

            (*yynode)->stmts_sequence =  snoc((*yynode)->stmts_sequence,  __node_13,  memory_pool);

          }

        else
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

    if  (yytoken ==  Token_EXCLAM
         ||  yytoken ==  Token_NEWLINE
         ||  yytoken ==  Token_IDENTIFIER)
      {
        if  (yytoken ==  Token_IDENTIFIER)
          {
            if  (yytoken !=  Token_IDENTIFIER)
              {
                yy_expected_token(yytoken,  Token_IDENTIFIER,  "identifier");
                return  false;
              }

            (*yynode)->id =  token_stream->index() -  1;
            yylex();

            if  (yytoken ==  Token_PLUSEQ
                 ||  yytoken ==  Token_EQUAL
                 ||  yytoken ==  Token_MINUSEQ
                 ||  yytoken ==  Token_STAREQ
                 ||  yytoken ==  Token_TILDEEQ)
              {
                variable_assignment_ast *__node_14 =  0;

                if  (!parse_variable_assignment(&__node_14))
                  {
                    yy_expected_symbol(ast_node::Kind_variable_assignment,  "variable_assignment");
                    return  false;
                  }

                (*yynode)->var =  __node_14;

              }

            else if  (yytoken ==  Token_LBRACE
                      ||  yytoken ==  Token_LPAREN
                      ||  yytoken ==  Token_COLON
                      ||  yytoken ==  Token_OR)
              {
                scope_ast *__node_15 =  0;

                if  (!parse_scope(&__node_15))
                  {
                    yy_expected_symbol(ast_node::Kind_scope,  "scope");
                    return  false;
                  }

                (*yynode)->scope =  __node_15;

              }

            else
              {
                return  false;
              }

            (*yynode)->isNewline =  false;
            (*yynode)->isExclam =  false;
          }

        else if  (yytoken ==  Token_EXCLAM)
          {
            if  (yytoken !=  Token_EXCLAM)
              {
                yy_expected_token(yytoken,  Token_EXCLAM,  "exclam");
                return  false;
              }

            yylex();

            if  (yytoken !=  Token_IDENTIFIER)
              {
                yy_expected_token(yytoken,  Token_IDENTIFIER,  "identifier");
                return  false;
              }

            (*yynode)->id =  token_stream->index() -  1;
            yylex();

            scope_ast *__node_16 =  0;

            if  (!parse_scope(&__node_16))
              {
                yy_expected_symbol(ast_node::Kind_scope,  "scope");
                return  false;
              }

            (*yynode)->scope =  __node_16;


            (*yynode)->isNewline =  false;
            (*yynode)->isExclam =  true;
          }

        else if  (yytoken ==  Token_NEWLINE)
          {
            if  (yytoken !=  Token_NEWLINE)
              {
                yy_expected_token(yytoken,  Token_NEWLINE,  "newline");
                return  false;
              }

            yylex();


            (*yynode)->isNewline =  true;
            (*yynode)->isExclam =  false;
          }

        else
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

  bool parser::parse_value(value_ast **yynode)
  {
    *yynode =  create<value_ast>();

    (*yynode)->start_token =  token_stream->index() -  1;

    if  (yytoken ==  Token_VALUE)
      {
        if  (yytoken !=  Token_VALUE)
          {
            yy_expected_token(yytoken,  Token_VALUE,  "value");
            return  false;
          }

        (*yynode)->value =  token_stream->index() -  1;
        yylex();

      }

    else
      {
        return  false;
      }

    (*yynode)->end_token =  token_stream->index() -  1;

    return  true;
  }

  bool parser::parse_value_list(value_list_ast **yynode)
  {
    *yynode =  create<value_list_ast>();

    (*yynode)->start_token =  token_stream->index() -  1;

    if  (yytoken ==  Token_CONT
         ||  yytoken ==  Token_VALUE)
      {
        do
          {
            if  (yytoken ==  Token_VALUE)
              {
                value_ast *__node_17 =  0;

                if  (!parse_value(&__node_17))
                  {
                    yy_expected_symbol(ast_node::Kind_value,  "value");
                    return  false;
                  }

                (*yynode)->list_sequence =  snoc((*yynode)->list_sequence,  __node_17,  memory_pool);

              }

            else if  (yytoken ==  Token_CONT)
              {
                if  (yytoken !=  Token_CONT)
                  {
                    yy_expected_token(yytoken,  Token_CONT,  "cont");
                    return  false;
                  }

                yylex();

                if  (yytoken !=  Token_NEWLINE)
                  {
                    yy_expected_token(yytoken,  Token_NEWLINE,  "newline");
                    return  false;
                  }

                yylex();

              }

            else
              {
                return  false;
              }
          }

        while  (yytoken ==  Token_CONT
                ||  yytoken ==  Token_VALUE);
      }

    else
      {
        return  false;
      }

    (*yynode)->end_token =  token_stream->index() -  1;

    return  true;
  }

  bool parser::parse_variable_assignment(variable_assignment_ast **yynode)
  {
    *yynode =  create<variable_assignment_ast>();

    (*yynode)->start_token =  token_stream->index() -  1;

    if  (yytoken ==  Token_PLUSEQ
         ||  yytoken ==  Token_EQUAL
         ||  yytoken ==  Token_MINUSEQ
         ||  yytoken ==  Token_STAREQ
         ||  yytoken ==  Token_TILDEEQ)
      {
        op_ast *__node_18 =  0;

        if  (!parse_op(&__node_18))
          {
            yy_expected_symbol(ast_node::Kind_op,  "op");
            return  false;
          }

        (*yynode)->op =  __node_18;

        value_list_ast *__node_19 =  0;

        if  (!parse_value_list(&__node_19))
          {
            yy_expected_symbol(ast_node::Kind_value_list,  "value_list");
            return  false;
          }

        (*yynode)->values =  __node_19;

        if  (yytoken ==  Token_NEWLINE)
          {
            if  (yytoken !=  Token_NEWLINE)
              {
                yy_expected_token(yytoken,  Token_NEWLINE,  "newline");
                return  false;
              }

            yylex();

          }

        else if  (true /*epsilon*/)
        {}
        else
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


