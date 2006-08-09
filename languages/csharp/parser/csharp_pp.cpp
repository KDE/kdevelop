// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#include "csharp_pp.h"


#include "csharp.h"
#include "csharp_lexer.h"
#include "csharp_pp_handler_visitor.h"

#include <string>


namespace csharp_pp
  {

  void parser::tokenize(bool &encountered_eof)
  {
    encountered_eof = false;
    int kind = parser::Token_EOF;
    do
      {
        kind = _M_lexer->yylex();
        //std::cerr << "pp: " << lexer.YYText() << std::endl; //" "; // debug output

        parser::token_type &t = this->token_stream->next();
        t.kind = kind;
        t.begin = _M_lexer->token_begin();
        t.end = _M_lexer->token_end();
        t.text = _M_lexer->contents();

        if (kind == parser::Token_EOF)
          {
            t.kind = parser::Token_PP_NEW_LINE;
            encountered_eof = true;
            break;
          }
      }
    while (kind != parser::Token_PP_NEW_LINE);

    parser::token_type &t = this->token_stream->next();
    t.kind = parser::Token_EOF;
    t.begin = _M_lexer->token_begin();
    t.end = _M_lexer->token_end();
    t.text = _M_lexer->contents();

    this->yylex(); // produce the look ahead token
  }

  void parser::add_token(parser::token_type_enum token_kind)
  {
    //std::cerr << "pp: " << lexer.YYText() << std::endl; //" "; // debug output
    parser::token_type &t = this->token_stream->next();
    t.kind = token_kind;
    t.begin = _M_lexer->token_begin();
    t.end = _M_lexer->token_end();
    t.text = _M_lexer->contents();
  }


  parser::pp_parse_result parser::pp_parse_line(
    parser::token_type_enum first_token, scope* scope, csharp::Lexer *lexer )
  {
    // 0) setup
    if (scope == 0)
      return parser::result_invalid;

    _M_scope = scope;
    bool encountered_eof;

    // 1) tokenize
    _M_lexer = lexer;
    add_token(first_token);
    tokenize(encountered_eof);

    // 2) parse
    pp_directive_ast* pp_directive_node = 0;
    bool matched = parse_pp_directive(&pp_directive_node);

    if (matched)
      {
        handler_visitor v(this);
        v.set_scope(_M_scope);
        v.visit_node(pp_directive_node);
      }
    else
      {
        yy_expected_symbol(ast_node::Kind_pp_directive, "pp_directive"); // ### remove me
        return parser::result_invalid;
      }

    if (encountered_eof)
      return parser::result_eof;
    else
      return parser::result_ok;
  }


  // custom error recovery
  void parser::yy_expected_token(int /*expected*/, std::size_t /*where*/, char const *name)
  {
    //print_token_environment(this);
    if (_M_scope->csharp_parser() != 0)
      {
        _M_scope->csharp_parser()->report_problem(
          csharp::parser::error,
          std::string("Invalid pre-processor directive: Expected token ``") + name
          //+ "'' instead of ``" + current_token_text
          + "''"
        );
      }
  }

  void parser::yy_expected_symbol(int /*expected_symbol*/, char const *name)
  {
    //print_token_environment(this);
    if (_M_scope->csharp_parser() != 0)
      {
        _M_scope->csharp_parser()->report_problem(
          csharp::parser::error,
          std::string("Invalid pre-processor directive: Expected symbol ``") + name
          //+ "'' instead of ``" + current_token_text
          + "''"
        );
      }
  }

} // end of namespace csharp_pp


namespace csharp_pp
  {

  bool parser::parse_pp_and_expression(pp_and_expression_ast **yynode)
  {
    *yynode = create<pp_and_expression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_PP_CONDITIONAL_SYMBOL
        || yytoken == Token_PP_BANG
        || yytoken == Token_PP_LPAREN
        || yytoken == Token_PP_TRUE
        || yytoken == Token_PP_FALSE)
      {
        pp_equality_expression_ast *__node_0 = 0;
        if (!parse_pp_equality_expression(&__node_0))
          {
            yy_expected_symbol(ast_node::Kind_pp_equality_expression, "pp_equality_expression");
            return false;
          }
        (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_0, memory_pool);

        while (yytoken == Token_PP_LOG_AND)
          {
            if (yytoken != Token_PP_LOG_AND)
              {
                yy_expected_token(yytoken, Token_PP_LOG_AND, "&&");
                return false;
              }
            yylex();

            pp_equality_expression_ast *__node_1 = 0;
            if (!parse_pp_equality_expression(&__node_1))
              {
                yy_expected_symbol(ast_node::Kind_pp_equality_expression, "pp_equality_expression");
                return false;
              }
            (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_1, memory_pool);

          }
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_pp_declaration(pp_declaration_ast **yynode)
  {
    *yynode = create<pp_declaration_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_PP_DEFINE
        || yytoken == Token_PP_UNDEF)
      {
        if (yytoken == Token_PP_DEFINE)
          {
            if (yytoken != Token_PP_DEFINE)
              {
                yy_expected_token(yytoken, Token_PP_DEFINE, "#define");
                return false;
              }
            yylex();

            (*yynode)->type = pp_declaration::type_define;
          }
        else if (yytoken == Token_PP_UNDEF)
          {
            if (yytoken != Token_PP_UNDEF)
              {
                yy_expected_token(yytoken, Token_PP_UNDEF, "#undef");
                return false;
              }
            yylex();

            (*yynode)->type = pp_declaration::type_undef;
          }
        else
          {
            return false;
          }
        if (yytoken != Token_PP_CONDITIONAL_SYMBOL)
          {
            yy_expected_token(yytoken, Token_PP_CONDITIONAL_SYMBOL, "pre-processor symbol");
            return false;
          }
        (*yynode)->conditional_symbol = token_stream->index() - 1;
        yylex();

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_pp_diagnostic(pp_diagnostic_ast **yynode)
  {
    *yynode = create<pp_diagnostic_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_PP_ERROR
        || yytoken == Token_PP_WARNING)
      {
        if (yytoken == Token_PP_ERROR)
          {
            if (yytoken != Token_PP_ERROR)
              {
                yy_expected_token(yytoken, Token_PP_ERROR, "#error");
                return false;
              }
            yylex();

            (*yynode)->type = pp_diagnostic::type_error;
          }
        else if (yytoken == Token_PP_WARNING)
          {
            if (yytoken != Token_PP_WARNING)
              {
                yy_expected_token(yytoken, Token_PP_WARNING, "#warning");
                return false;
              }
            yylex();

            (*yynode)->type = pp_diagnostic::type_warning;
          }
        else
          {
            return false;
          }
        if (yytoken == Token_PP_MESSAGE)
          {
            if (yytoken != Token_PP_MESSAGE)
              {
                yy_expected_token(yytoken, Token_PP_MESSAGE, "single-line text");
                return false;
              }
            (*yynode)->message = token_stream->index() - 1;
            yylex();

          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_pp_directive(pp_directive_ast **yynode)
  {
    *yynode = create<pp_directive_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_PP_DEFINE
        || yytoken == Token_PP_UNDEF
        || yytoken == Token_PP_IF
        || yytoken == Token_PP_ELIF
        || yytoken == Token_PP_ELSE
        || yytoken == Token_PP_ENDIF
        || yytoken == Token_PP_LINE
        || yytoken == Token_PP_ERROR
        || yytoken == Token_PP_WARNING
        || yytoken == Token_PP_REGION
        || yytoken == Token_PP_ENDREGION
        || yytoken == Token_PP_PRAGMA)
      {
        if (yytoken == Token_PP_DEFINE
            || yytoken == Token_PP_UNDEF)
          {
            pp_declaration_ast *__node_2 = 0;
            if (!parse_pp_declaration(&__node_2))
              {
                yy_expected_symbol(ast_node::Kind_pp_declaration, "pp_declaration");
                return false;
              }
            (*yynode)->declaration = __node_2;

          }
        else if (yytoken == Token_PP_IF)
          {
            pp_if_clause_ast *__node_3 = 0;
            if (!parse_pp_if_clause(&__node_3))
              {
                yy_expected_symbol(ast_node::Kind_pp_if_clause, "pp_if_clause");
                return false;
              }
            (*yynode)->if_clause = __node_3;

          }
        else if (yytoken == Token_PP_ELIF)
          {
            pp_elif_clause_ast *__node_4 = 0;
            if (!parse_pp_elif_clause(&__node_4))
              {
                yy_expected_symbol(ast_node::Kind_pp_elif_clause, "pp_elif_clause");
                return false;
              }
            (*yynode)->elif_clause = __node_4;

          }
        else if (yytoken == Token_PP_ELSE)
          {
            pp_else_clause_ast *__node_5 = 0;
            if (!parse_pp_else_clause(&__node_5))
              {
                yy_expected_symbol(ast_node::Kind_pp_else_clause, "pp_else_clause");
                return false;
              }
            (*yynode)->else_clause = __node_5;

          }
        else if (yytoken == Token_PP_ENDIF)
          {
            pp_endif_clause_ast *__node_6 = 0;
            if (!parse_pp_endif_clause(&__node_6))
              {
                yy_expected_symbol(ast_node::Kind_pp_endif_clause, "pp_endif_clause");
                return false;
              }
            (*yynode)->endif_clause = __node_6;

          }
        else if (yytoken == Token_PP_ERROR
                 || yytoken == Token_PP_WARNING)
          {
            pp_diagnostic_ast *__node_7 = 0;
            if (!parse_pp_diagnostic(&__node_7))
              {
                yy_expected_symbol(ast_node::Kind_pp_diagnostic, "pp_diagnostic");
                return false;
              }
            (*yynode)->diagnostic = __node_7;

          }
        else if (yytoken == Token_PP_REGION
                 || yytoken == Token_PP_ENDREGION)
          {
            pp_region_ast *__node_8 = 0;
            if (!parse_pp_region(&__node_8))
              {
                yy_expected_symbol(ast_node::Kind_pp_region, "pp_region");
                return false;
              }
            (*yynode)->region = __node_8;

          }
        else if (yytoken == Token_PP_LINE)
          {
            pp_line_ast *__node_9 = 0;
            if (!parse_pp_line(&__node_9))
              {
                yy_expected_symbol(ast_node::Kind_pp_line, "pp_line");
                return false;
              }
            (*yynode)->line = __node_9;

          }
        else if (yytoken == Token_PP_PRAGMA)
          {
            pp_pragma_ast *__node_10 = 0;
            if (!parse_pp_pragma(&__node_10))
              {
                yy_expected_symbol(ast_node::Kind_pp_pragma, "pp_pragma");
                return false;
              }
            (*yynode)->pragma = __node_10;

          }
        else
          {
            return false;
          }
        if (yytoken != Token_PP_NEW_LINE)
          {
            yy_expected_token(yytoken, Token_PP_NEW_LINE, "line break");
            return false;
          }
        yylex();

        if (Token_EOF != yytoken)
          {
            return false;
          }
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_pp_elif_clause(pp_elif_clause_ast **yynode)
  {
    *yynode = create<pp_elif_clause_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_PP_ELIF)
      {
        if (yytoken != Token_PP_ELIF)
          {
            yy_expected_token(yytoken, Token_PP_ELIF, "#elif");
            return false;
          }
        yylex();

        pp_expression_ast *__node_11 = 0;
        if (!parse_pp_expression(&__node_11))
          {
            yy_expected_symbol(ast_node::Kind_pp_expression, "pp_expression");
            return false;
          }
        (*yynode)->expression = __node_11;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_pp_else_clause(pp_else_clause_ast **yynode)
  {
    *yynode = create<pp_else_clause_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_PP_ELSE)
      {
        if (yytoken != Token_PP_ELSE)
          {
            yy_expected_token(yytoken, Token_PP_ELSE, "#else");
            return false;
          }
        yylex();

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_pp_endif_clause(pp_endif_clause_ast **yynode)
  {
    *yynode = create<pp_endif_clause_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_PP_ENDIF)
      {
        if (yytoken != Token_PP_ENDIF)
          {
            yy_expected_token(yytoken, Token_PP_ENDIF, "#endif");
            return false;
          }
        yylex();

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_pp_equality_expression(pp_equality_expression_ast **yynode)
  {
    *yynode = create<pp_equality_expression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_PP_CONDITIONAL_SYMBOL
        || yytoken == Token_PP_BANG
        || yytoken == Token_PP_LPAREN
        || yytoken == Token_PP_TRUE
        || yytoken == Token_PP_FALSE)
      {
        pp_unary_expression_ast *__node_12 = 0;
        if (!parse_pp_unary_expression(&__node_12))
          {
            yy_expected_symbol(ast_node::Kind_pp_unary_expression, "pp_unary_expression");
            return false;
          }
        (*yynode)->expression = __node_12;

        while (yytoken == Token_PP_EQUAL
               || yytoken == Token_PP_NOT_EQUAL)
          {
            pp_equality_expression_rest_ast *__node_13 = 0;
            if (!parse_pp_equality_expression_rest(&__node_13))
              {
                yy_expected_symbol(ast_node::Kind_pp_equality_expression_rest, "pp_equality_expression_rest");
                return false;
              }
            (*yynode)->additional_expression_sequence = snoc((*yynode)->additional_expression_sequence, __node_13, memory_pool);

          }
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_pp_equality_expression_rest(pp_equality_expression_rest_ast **yynode)
  {
    *yynode = create<pp_equality_expression_rest_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_PP_EQUAL
        || yytoken == Token_PP_NOT_EQUAL)
      {
        if (yytoken == Token_PP_EQUAL)
          {
            if (yytoken != Token_PP_EQUAL)
              {
                yy_expected_token(yytoken, Token_PP_EQUAL, "==");
                return false;
              }
            yylex();

            (*yynode)->equality_operator = pp_equality_expression_rest::op_equal;
          }
        else if (yytoken == Token_PP_NOT_EQUAL)
          {
            if (yytoken != Token_PP_NOT_EQUAL)
              {
                yy_expected_token(yytoken, Token_PP_NOT_EQUAL, "!=");
                return false;
              }
            yylex();

            (*yynode)->equality_operator = pp_equality_expression_rest::op_not_equal;
          }
        else
          {
            return false;
          }
        pp_unary_expression_ast *__node_14 = 0;
        if (!parse_pp_unary_expression(&__node_14))
          {
            yy_expected_symbol(ast_node::Kind_pp_unary_expression, "pp_unary_expression");
            return false;
          }
        (*yynode)->expression = __node_14;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_pp_expression(pp_expression_ast **yynode)
  {
    *yynode = create<pp_expression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_PP_CONDITIONAL_SYMBOL
        || yytoken == Token_PP_BANG
        || yytoken == Token_PP_LPAREN
        || yytoken == Token_PP_TRUE
        || yytoken == Token_PP_FALSE)
      {
        pp_and_expression_ast *__node_15 = 0;
        if (!parse_pp_and_expression(&__node_15))
          {
            yy_expected_symbol(ast_node::Kind_pp_and_expression, "pp_and_expression");
            return false;
          }
        (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_15, memory_pool);

        while (yytoken == Token_PP_LOG_OR)
          {
            if (yytoken != Token_PP_LOG_OR)
              {
                yy_expected_token(yytoken, Token_PP_LOG_OR, "||");
                return false;
              }
            yylex();

            pp_and_expression_ast *__node_16 = 0;
            if (!parse_pp_and_expression(&__node_16))
              {
                yy_expected_symbol(ast_node::Kind_pp_and_expression, "pp_and_expression");
                return false;
              }
            (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_16, memory_pool);

          }
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_pp_if_clause(pp_if_clause_ast **yynode)
  {
    *yynode = create<pp_if_clause_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_PP_IF)
      {
        if (yytoken != Token_PP_IF)
          {
            yy_expected_token(yytoken, Token_PP_IF, "#if");
            return false;
          }
        yylex();

        pp_expression_ast *__node_17 = 0;
        if (!parse_pp_expression(&__node_17))
          {
            yy_expected_symbol(ast_node::Kind_pp_expression, "pp_expression");
            return false;
          }
        (*yynode)->expression = __node_17;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_pp_line(pp_line_ast **yynode)
  {
    *yynode = create<pp_line_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_PP_LINE)
      {
        if (yytoken != Token_PP_LINE)
          {
            yy_expected_token(yytoken, Token_PP_LINE, "#line");
            return false;
          }
        yylex();

        if (yytoken == Token_PP_LINE_NUMBER)
          {
            if (yytoken != Token_PP_LINE_NUMBER)
              {
                yy_expected_token(yytoken, Token_PP_LINE_NUMBER, "line number");
                return false;
              }
            (*yynode)->line_number = token_stream->index() - 1;
            yylex();

            if (yytoken == Token_PP_FILE_NAME)
              {
                if (yytoken != Token_PP_FILE_NAME)
                  {
                    yy_expected_token(yytoken, Token_PP_FILE_NAME, "file name (in double quotes)");
                    return false;
                  }
                (*yynode)->file_name = token_stream->index() - 1;
                yylex();

              }
            else if (true /*epsilon*/)
            {}
            else
              {
                return false;
              }
          }
        else if (yytoken == Token_PP_DEFAULT)
          {
            if (yytoken != Token_PP_DEFAULT)
              {
                yy_expected_token(yytoken, Token_PP_DEFAULT, "default");
                return false;
              }
            (*yynode)->token_default = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_PP_IDENTIFIER_OR_KEYWORD)
          {
            if (yytoken != Token_PP_IDENTIFIER_OR_KEYWORD)
              {
                yy_expected_token(yytoken, Token_PP_IDENTIFIER_OR_KEYWORD, "identifier or keyword");
                return false;
              }
            (*yynode)->identifier_or_keyword = token_stream->index() - 1;
            yylex();

          }
        else
          {
            return false;
          }
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_pp_pragma(pp_pragma_ast **yynode)
  {
    *yynode = create<pp_pragma_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_PP_PRAGMA)
      {
        if (yytoken != Token_PP_PRAGMA)
          {
            yy_expected_token(yytoken, Token_PP_PRAGMA, "#pragma");
            return false;
          }
        yylex();

        if (yytoken == Token_PP_PRAGMA_TEXT)
          {
            if (yytoken != Token_PP_PRAGMA_TEXT)
              {
                yy_expected_token(yytoken, Token_PP_PRAGMA_TEXT, "pragma text");
                return false;
              }
            (*yynode)->pragma_text = token_stream->index() - 1;
            yylex();

          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_pp_primary_expression(pp_primary_expression_ast **yynode)
  {
    *yynode = create<pp_primary_expression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_PP_CONDITIONAL_SYMBOL
        || yytoken == Token_PP_LPAREN
        || yytoken == Token_PP_TRUE
        || yytoken == Token_PP_FALSE)
      {
        if (yytoken == Token_PP_TRUE)
          {
            if (yytoken != Token_PP_TRUE)
              {
                yy_expected_token(yytoken, Token_PP_TRUE, "true");
                return false;
              }
            yylex();

            (*yynode)->type = pp_primary_expression::type_true;
          }
        else if (yytoken == Token_PP_FALSE)
          {
            if (yytoken != Token_PP_FALSE)
              {
                yy_expected_token(yytoken, Token_PP_FALSE, "false");
                return false;
              }
            yylex();

            (*yynode)->type = pp_primary_expression::type_false;
          }
        else if (yytoken == Token_PP_CONDITIONAL_SYMBOL)
          {
            if (yytoken != Token_PP_CONDITIONAL_SYMBOL)
              {
                yy_expected_token(yytoken, Token_PP_CONDITIONAL_SYMBOL, "pre-processor symbol");
                return false;
              }
            (*yynode)->conditional_symbol = token_stream->index() - 1;
            yylex();

            (*yynode)->type = pp_primary_expression::type_conditional_symbol;
          }
        else if (yytoken == Token_PP_LPAREN)
          {
            if (yytoken != Token_PP_LPAREN)
              {
                yy_expected_token(yytoken, Token_PP_LPAREN, "(");
                return false;
              }
            yylex();

            pp_expression_ast *__node_18 = 0;
            if (!parse_pp_expression(&__node_18))
              {
                yy_expected_symbol(ast_node::Kind_pp_expression, "pp_expression");
                return false;
              }
            (*yynode)->parenthesis_expression = __node_18;

            if (yytoken != Token_PP_RPAREN)
              {
                yy_expected_token(yytoken, Token_PP_RPAREN, ")");
                return false;
              }
            yylex();

            (*yynode)->type = pp_primary_expression::type_parenthesis_expression;
          }
        else
          {
            return false;
          }
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_pp_region(pp_region_ast **yynode)
  {
    *yynode = create<pp_region_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_PP_REGION
        || yytoken == Token_PP_ENDREGION)
      {
        if (yytoken == Token_PP_REGION)
          {
            if (yytoken != Token_PP_REGION)
              {
                yy_expected_token(yytoken, Token_PP_REGION, "#region");
                return false;
              }
            yylex();

            (*yynode)->type = pp_region::type_region;
          }
        else if (yytoken == Token_PP_ENDREGION)
          {
            if (yytoken != Token_PP_ENDREGION)
              {
                yy_expected_token(yytoken, Token_PP_ENDREGION, "#endregion");
                return false;
              }
            yylex();

            (*yynode)->type = pp_region::type_endregion;
          }
        else
          {
            return false;
          }
        if (yytoken == Token_PP_MESSAGE)
          {
            if (yytoken != Token_PP_MESSAGE)
              {
                yy_expected_token(yytoken, Token_PP_MESSAGE, "single-line text");
                return false;
              }
            (*yynode)->label = token_stream->index() - 1;
            yylex();

          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_pp_unary_expression(pp_unary_expression_ast **yynode)
  {
    *yynode = create<pp_unary_expression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_PP_CONDITIONAL_SYMBOL
        || yytoken == Token_PP_BANG
        || yytoken == Token_PP_LPAREN
        || yytoken == Token_PP_TRUE
        || yytoken == Token_PP_FALSE)
      {
        if (yytoken == Token_PP_BANG)
          {
            if (yytoken != Token_PP_BANG)
              {
                yy_expected_token(yytoken, Token_PP_BANG, "!");
                return false;
              }
            yylex();

            pp_primary_expression_ast *__node_19 = 0;
            if (!parse_pp_primary_expression(&__node_19))
              {
                yy_expected_symbol(ast_node::Kind_pp_primary_expression, "pp_primary_expression");
                return false;
              }
            (*yynode)->expression = __node_19;

            (*yynode)->negated = true;
          }
        else if (yytoken == Token_PP_CONDITIONAL_SYMBOL
                 || yytoken == Token_PP_LPAREN
                 || yytoken == Token_PP_TRUE
                 || yytoken == Token_PP_FALSE)
          {
            pp_primary_expression_ast *__node_20 = 0;
            if (!parse_pp_primary_expression(&__node_20))
              {
                yy_expected_symbol(ast_node::Kind_pp_primary_expression, "pp_primary_expression");
                return false;
              }
            (*yynode)->expression = __node_20;

            (*yynode)->negated = false;
          }
        else
          {
            return false;
          }
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  visitor::parser_fun_t visitor::_S_parser_table[] = {
        reinterpret_cast<parser_fun_t>(&visitor::visit_pp_and_expression),
        reinterpret_cast<parser_fun_t>(&visitor::visit_pp_declaration),
        reinterpret_cast<parser_fun_t>(&visitor::visit_pp_diagnostic),
        reinterpret_cast<parser_fun_t>(&visitor::visit_pp_directive),
        reinterpret_cast<parser_fun_t>(&visitor::visit_pp_elif_clause),
        reinterpret_cast<parser_fun_t>(&visitor::visit_pp_else_clause),
        reinterpret_cast<parser_fun_t>(&visitor::visit_pp_endif_clause),
        reinterpret_cast<parser_fun_t>(&visitor::visit_pp_equality_expression),
        reinterpret_cast<parser_fun_t>(&visitor::visit_pp_equality_expression_rest),
        reinterpret_cast<parser_fun_t>(&visitor::visit_pp_expression),
        reinterpret_cast<parser_fun_t>(&visitor::visit_pp_if_clause),
        reinterpret_cast<parser_fun_t>(&visitor::visit_pp_line),
        reinterpret_cast<parser_fun_t>(&visitor::visit_pp_pragma),
        reinterpret_cast<parser_fun_t>(&visitor::visit_pp_primary_expression),
        reinterpret_cast<parser_fun_t>(&visitor::visit_pp_region),
        reinterpret_cast<parser_fun_t>(&visitor::visit_pp_unary_expression)
      }; // _S_parser_table[]

} // end of namespace csharp_pp


