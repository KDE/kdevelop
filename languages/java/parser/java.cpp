// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#include "java.h"


#include "java_lexer.h"


namespace java
  {

  void parser::tokenize(char *contents)
  {
    Lexer lexer(this, contents);

    int kind = parser::Token_EOF;
    do
      {
        kind = lexer.yylex();
        //std::cerr << lexer.YYText() << std::endl; //" "; // debug output

        if (!kind) // when the lexer returns 0, the end of file is reached
          kind = parser::Token_EOF;

        parser::token_type &t = this->token_stream->next();
        t.kind = kind;
        t.begin = lexer.token_begin();
        t.end = lexer.token_end();
        t.text = contents;
      }
    while (kind != parser::Token_EOF);

    this->yylex(); // produce the look ahead token
  }

  parser::java_compatibility_mode parser::compatibility_mode()
  {
    return _M_compatibility_mode;
  }
  void parser::set_compatibility_mode( parser::java_compatibility_mode mode )
  {
    _M_compatibility_mode = mode;
  }


  parser::parser_state *parser::copy_current_state()
  {
    parser_state *state = new parser_state();
    state->ltCounter = _M_state.ltCounter;
    return state;
  }

  void parser::restore_state(parser::parser_state *state)
  {
    _M_state.ltCounter = state->ltCounter;
  }

} // end of namespace java


namespace java
  {

  bool parser::parse_additive_expression(additive_expression_ast **yynode)
  {
    *yynode = create<additive_expression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOOLEAN
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_SHORT
        || yytoken == Token_SUPER
        || yytoken == Token_THIS
        || yytoken == Token_VOID
        || yytoken == Token_LPAREN
        || yytoken == Token_LESS_THAN
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_FLOATING_POINT_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        multiplicative_expression_ast *__node_0 = 0;
        if (!parse_multiplicative_expression(&__node_0))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_multiplicative_expression, "multiplicative_expression");
              }
            return false;
          }
        (*yynode)->expression = __node_0;

        while (yytoken == Token_PLUS
               || yytoken == Token_MINUS)
          {
            additive_expression_rest_ast *__node_1 = 0;
            if (!parse_additive_expression_rest(&__node_1))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_additive_expression_rest, "additive_expression_rest");
                  }
                return false;
              }
            (*yynode)->additional_expression_sequence = snoc((*yynode)->additional_expression_sequence, __node_1, memory_pool);

          }
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_additive_expression_rest(additive_expression_rest_ast **yynode)
  {
    *yynode = create<additive_expression_rest_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_PLUS
        || yytoken == Token_MINUS)
      {
        if (yytoken == Token_PLUS)
          {
            if (yytoken != Token_PLUS)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_PLUS, "+");
                  }
                return false;
              }
            yylex();

            (*yynode)->additive_operator = additive_expression_rest::op_plus;
          }
        else if (yytoken == Token_MINUS)
          {
            if (yytoken != Token_MINUS)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_MINUS, "-");
                  }
                return false;
              }
            yylex();

            (*yynode)->additive_operator = additive_expression_rest::op_minus;
          }
        else
          {
            return false;
          }
        multiplicative_expression_ast *__node_2 = 0;
        if (!parse_multiplicative_expression(&__node_2))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_multiplicative_expression, "multiplicative_expression");
              }
            return false;
          }
        (*yynode)->expression = __node_2;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_annotation(annotation_ast **yynode)
  {
    *yynode = create<annotation_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_AT)
      {
        if (yytoken != Token_AT)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_AT, "@");
              }
            return false;
          }
        yylex();

        qualified_identifier_ast *__node_3 = 0;
        if (!parse_qualified_identifier(&__node_3))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_qualified_identifier, "qualified_identifier");
              }
            return false;
          }
        (*yynode)->type_name = __node_3;

        if (yytoken == Token_LPAREN)
          {
            if (yytoken != Token_LPAREN)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LPAREN, "(");
                  }
                return false;
              }
            yylex();

            if (yytoken == Token_BOOLEAN
                || yytoken == Token_BYTE
                || yytoken == Token_CHAR
                || yytoken == Token_DOUBLE
                || yytoken == Token_FLOAT
                || yytoken == Token_INT
                || yytoken == Token_LONG
                || yytoken == Token_NEW
                || yytoken == Token_SHORT
                || yytoken == Token_SUPER
                || yytoken == Token_THIS
                || yytoken == Token_VOID
                || yytoken == Token_LPAREN
                || yytoken == Token_LBRACE
                || yytoken == Token_AT
                || yytoken == Token_LESS_THAN
                || yytoken == Token_BANG
                || yytoken == Token_TILDE
                || yytoken == Token_INCREMENT
                || yytoken == Token_DECREMENT
                || yytoken == Token_PLUS
                || yytoken == Token_MINUS
                || yytoken == Token_TRUE
                || yytoken == Token_FALSE
                || yytoken == Token_NULL
                || yytoken == Token_INTEGER_LITERAL
                || yytoken == Token_FLOATING_POINT_LITERAL
                || yytoken == Token_CHARACTER_LITERAL
                || yytoken == Token_STRING_LITERAL
                || yytoken == Token_IDENTIFIER)
              {
                annotation_arguments_ast *__node_4 = 0;
                if (!parse_annotation_arguments(&__node_4))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_annotation_arguments, "annotation_arguments");
                      }
                    return false;
                  }
                (*yynode)->args = __node_4;

              }
            else if (true /*epsilon*/)
            {}
            else
              {
                return false;
              }
            if (yytoken != Token_RPAREN)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_RPAREN, ")");
                  }
                return false;
              }
            yylex();

            (*yynode)->has_parentheses = true;
          }
        else if (true /*epsilon*/)
          {
            (*yynode)->has_parentheses = false;
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

  bool parser::parse_annotation_arguments(annotation_arguments_ast **yynode)
  {
    *yynode = create<annotation_arguments_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOOLEAN
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_SHORT
        || yytoken == Token_SUPER
        || yytoken == Token_THIS
        || yytoken == Token_VOID
        || yytoken == Token_LPAREN
        || yytoken == Token_LBRACE
        || yytoken == Token_AT
        || yytoken == Token_LESS_THAN
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_FLOATING_POINT_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        if ((yytoken == Token_IDENTIFIER) && ( LA(2).kind == Token_ASSIGN ))
          {
            annotation_element_value_pair_ast *__node_5 = 0;
            if (!parse_annotation_element_value_pair(&__node_5))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_annotation_element_value_pair, "annotation_element_value_pair");
                  }
                return false;
              }
            (*yynode)->value_pair_sequence = snoc((*yynode)->value_pair_sequence, __node_5, memory_pool);

            while (yytoken == Token_COMMA)
              {
                if (yytoken != Token_COMMA)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_COMMA, ",");
                      }
                    return false;
                  }
                yylex();

                annotation_element_value_pair_ast *__node_6 = 0;
                if (!parse_annotation_element_value_pair(&__node_6))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_annotation_element_value_pair, "annotation_element_value_pair");
                      }
                    return false;
                  }
                (*yynode)->value_pair_sequence = snoc((*yynode)->value_pair_sequence, __node_6, memory_pool);

              }
          }
        else if (yytoken == Token_BOOLEAN
                 || yytoken == Token_BYTE
                 || yytoken == Token_CHAR
                 || yytoken == Token_DOUBLE
                 || yytoken == Token_FLOAT
                 || yytoken == Token_INT
                 || yytoken == Token_LONG
                 || yytoken == Token_NEW
                 || yytoken == Token_SHORT
                 || yytoken == Token_SUPER
                 || yytoken == Token_THIS
                 || yytoken == Token_VOID
                 || yytoken == Token_LPAREN
                 || yytoken == Token_LBRACE
                 || yytoken == Token_AT
                 || yytoken == Token_LESS_THAN
                 || yytoken == Token_BANG
                 || yytoken == Token_TILDE
                 || yytoken == Token_INCREMENT
                 || yytoken == Token_DECREMENT
                 || yytoken == Token_PLUS
                 || yytoken == Token_MINUS
                 || yytoken == Token_TRUE
                 || yytoken == Token_FALSE
                 || yytoken == Token_NULL
                 || yytoken == Token_INTEGER_LITERAL
                 || yytoken == Token_FLOATING_POINT_LITERAL
                 || yytoken == Token_CHARACTER_LITERAL
                 || yytoken == Token_STRING_LITERAL
                 || yytoken == Token_IDENTIFIER)
          {
            annotation_element_value_ast *__node_7 = 0;
            if (!parse_annotation_element_value(&__node_7))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_annotation_element_value, "annotation_element_value");
                  }
                return false;
              }
            (*yynode)->element_value = __node_7;

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

  bool parser::parse_annotation_element_array_initializer(annotation_element_array_initializer_ast **yynode)
  {
    *yynode = create<annotation_element_array_initializer_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_LBRACE)
      {
        if (yytoken != Token_LBRACE)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_LBRACE, "{");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_BOOLEAN
            || yytoken == Token_BYTE
            || yytoken == Token_CHAR
            || yytoken == Token_DOUBLE
            || yytoken == Token_FLOAT
            || yytoken == Token_INT
            || yytoken == Token_LONG
            || yytoken == Token_NEW
            || yytoken == Token_SHORT
            || yytoken == Token_SUPER
            || yytoken == Token_THIS
            || yytoken == Token_VOID
            || yytoken == Token_LPAREN
            || yytoken == Token_AT
            || yytoken == Token_LESS_THAN
            || yytoken == Token_BANG
            || yytoken == Token_TILDE
            || yytoken == Token_INCREMENT
            || yytoken == Token_DECREMENT
            || yytoken == Token_PLUS
            || yytoken == Token_MINUS
            || yytoken == Token_TRUE
            || yytoken == Token_FALSE
            || yytoken == Token_NULL
            || yytoken == Token_INTEGER_LITERAL
            || yytoken == Token_FLOATING_POINT_LITERAL
            || yytoken == Token_CHARACTER_LITERAL
            || yytoken == Token_STRING_LITERAL
            || yytoken == Token_IDENTIFIER)
          {
            annotation_element_array_value_ast *__node_8 = 0;
            if (!parse_annotation_element_array_value(&__node_8))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_annotation_element_array_value, "annotation_element_array_value");
                  }
                return false;
              }
            (*yynode)->element_value_sequence = snoc((*yynode)->element_value_sequence, __node_8, memory_pool);

            while (yytoken == Token_COMMA)
              {
                if (LA(2).kind == Token_RBRACE)
                  {
                    break;
                  }
                if (yytoken != Token_COMMA)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_COMMA, ",");
                      }
                    return false;
                  }
                yylex();

                annotation_element_array_value_ast *__node_9 = 0;
                if (!parse_annotation_element_array_value(&__node_9))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_annotation_element_array_value, "annotation_element_array_value");
                      }
                    return false;
                  }
                (*yynode)->element_value_sequence = snoc((*yynode)->element_value_sequence, __node_9, memory_pool);

              }
          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        if (yytoken == Token_COMMA)
          {
            if (yytoken != Token_COMMA)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_COMMA, ",");
                  }
                return false;
              }
            yylex();

          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        if (yytoken != Token_RBRACE)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_RBRACE, "}");
              }
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

  bool parser::parse_annotation_element_array_value(annotation_element_array_value_ast **yynode)
  {
    *yynode = create<annotation_element_array_value_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOOLEAN
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_SHORT
        || yytoken == Token_SUPER
        || yytoken == Token_THIS
        || yytoken == Token_VOID
        || yytoken == Token_LPAREN
        || yytoken == Token_AT
        || yytoken == Token_LESS_THAN
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_FLOATING_POINT_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        if (yytoken == Token_BOOLEAN
            || yytoken == Token_BYTE
            || yytoken == Token_CHAR
            || yytoken == Token_DOUBLE
            || yytoken == Token_FLOAT
            || yytoken == Token_INT
            || yytoken == Token_LONG
            || yytoken == Token_NEW
            || yytoken == Token_SHORT
            || yytoken == Token_SUPER
            || yytoken == Token_THIS
            || yytoken == Token_VOID
            || yytoken == Token_LPAREN
            || yytoken == Token_LESS_THAN
            || yytoken == Token_BANG
            || yytoken == Token_TILDE
            || yytoken == Token_INCREMENT
            || yytoken == Token_DECREMENT
            || yytoken == Token_PLUS
            || yytoken == Token_MINUS
            || yytoken == Token_TRUE
            || yytoken == Token_FALSE
            || yytoken == Token_NULL
            || yytoken == Token_INTEGER_LITERAL
            || yytoken == Token_FLOATING_POINT_LITERAL
            || yytoken == Token_CHARACTER_LITERAL
            || yytoken == Token_STRING_LITERAL
            || yytoken == Token_IDENTIFIER)
          {
            conditional_expression_ast *__node_10 = 0;
            if (!parse_conditional_expression(&__node_10))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_conditional_expression, "conditional_expression");
                  }
                return false;
              }
            (*yynode)->cond_expression = __node_10;

          }
        else if (yytoken == Token_AT)
          {
            annotation_ast *__node_11 = 0;
            if (!parse_annotation(&__node_11))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_annotation, "annotation");
                  }
                return false;
              }
            (*yynode)->annotation = __node_11;

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

  bool parser::parse_annotation_element_value(annotation_element_value_ast **yynode)
  {
    *yynode = create<annotation_element_value_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOOLEAN
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_SHORT
        || yytoken == Token_SUPER
        || yytoken == Token_THIS
        || yytoken == Token_VOID
        || yytoken == Token_LPAREN
        || yytoken == Token_LBRACE
        || yytoken == Token_AT
        || yytoken == Token_LESS_THAN
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_FLOATING_POINT_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        if (yytoken == Token_BOOLEAN
            || yytoken == Token_BYTE
            || yytoken == Token_CHAR
            || yytoken == Token_DOUBLE
            || yytoken == Token_FLOAT
            || yytoken == Token_INT
            || yytoken == Token_LONG
            || yytoken == Token_NEW
            || yytoken == Token_SHORT
            || yytoken == Token_SUPER
            || yytoken == Token_THIS
            || yytoken == Token_VOID
            || yytoken == Token_LPAREN
            || yytoken == Token_LESS_THAN
            || yytoken == Token_BANG
            || yytoken == Token_TILDE
            || yytoken == Token_INCREMENT
            || yytoken == Token_DECREMENT
            || yytoken == Token_PLUS
            || yytoken == Token_MINUS
            || yytoken == Token_TRUE
            || yytoken == Token_FALSE
            || yytoken == Token_NULL
            || yytoken == Token_INTEGER_LITERAL
            || yytoken == Token_FLOATING_POINT_LITERAL
            || yytoken == Token_CHARACTER_LITERAL
            || yytoken == Token_STRING_LITERAL
            || yytoken == Token_IDENTIFIER)
          {
            conditional_expression_ast *__node_12 = 0;
            if (!parse_conditional_expression(&__node_12))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_conditional_expression, "conditional_expression");
                  }
                return false;
              }
            (*yynode)->cond_expression = __node_12;

          }
        else if (yytoken == Token_AT)
          {
            annotation_ast *__node_13 = 0;
            if (!parse_annotation(&__node_13))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_annotation, "annotation");
                  }
                return false;
              }
            (*yynode)->annotation = __node_13;

          }
        else if (yytoken == Token_LBRACE)
          {
            annotation_element_array_initializer_ast *__node_14 = 0;
            if (!parse_annotation_element_array_initializer(&__node_14))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_annotation_element_array_initializer, "annotation_element_array_initializer");
                  }
                return false;
              }
            (*yynode)->element_array_initializer = __node_14;

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

  bool parser::parse_annotation_element_value_pair(annotation_element_value_pair_ast **yynode)
  {
    *yynode = create<annotation_element_value_pair_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_IDENTIFIER)
      {
        identifier_ast *__node_15 = 0;
        if (!parse_identifier(&__node_15))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_identifier, "identifier");
              }
            return false;
          }
        (*yynode)->element_name = __node_15;

        if (yytoken != Token_ASSIGN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_ASSIGN, "=");
              }
            return false;
          }
        yylex();

        annotation_element_value_ast *__node_16 = 0;
        if (!parse_annotation_element_value(&__node_16))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_annotation_element_value, "annotation_element_value");
              }
            return false;
          }
        (*yynode)->element_value = __node_16;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_annotation_method_declaration(annotation_method_declaration_ast **yynode, optional_modifiers_ast *modifiers, type_ast *return_type)
  {
    *yynode = create<annotation_method_declaration_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    (*yynode)->modifiers = modifiers;
    if (modifiers && modifiers->start_token < (*yynode)->start_token)
      (*yynode)->start_token = modifiers->start_token;

    (*yynode)->return_type = return_type;
    if (return_type && return_type->start_token < (*yynode)->start_token)
      (*yynode)->start_token = return_type->start_token;

    if (yytoken == Token_IDENTIFIER)
      {
        identifier_ast *__node_17 = 0;
        if (!parse_identifier(&__node_17))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_identifier, "identifier");
              }
            return false;
          }
        (*yynode)->annotation_name = __node_17;

        if (yytoken != Token_LPAREN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_LPAREN, "(");
              }
            return false;
          }
        yylex();

        if (yytoken != Token_RPAREN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_RPAREN, ")");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_DEFAULT)
          {
            if (yytoken != Token_DEFAULT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_DEFAULT, "default");
                  }
                return false;
              }
            yylex();

            annotation_element_value_ast *__node_18 = 0;
            if (!parse_annotation_element_value(&__node_18))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_annotation_element_value, "annotation_element_value");
                  }
                return false;
              }
            (*yynode)->annotation_element_value = __node_18;

          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        if (yytoken != Token_SEMICOLON)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_SEMICOLON, ";");
              }
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

  bool parser::parse_annotation_type_body(annotation_type_body_ast **yynode)
  {
    *yynode = create<annotation_type_body_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_LBRACE)
      {
        if (yytoken != Token_LBRACE)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_LBRACE, "{");
              }
            return false;
          }
        yylex();

        while (yytoken == Token_ABSTRACT
               || yytoken == Token_BOOLEAN
               || yytoken == Token_BYTE
               || yytoken == Token_CHAR
               || yytoken == Token_CLASS
               || yytoken == Token_DOUBLE
               || yytoken == Token_ENUM
               || yytoken == Token_FINAL
               || yytoken == Token_FLOAT
               || yytoken == Token_INT
               || yytoken == Token_INTERFACE
               || yytoken == Token_LONG
               || yytoken == Token_NATIVE
               || yytoken == Token_PRIVATE
               || yytoken == Token_PROTECTED
               || yytoken == Token_PUBLIC
               || yytoken == Token_SHORT
               || yytoken == Token_STATIC
               || yytoken == Token_STRICTFP
               || yytoken == Token_SYNCHRONIZED
               || yytoken == Token_TRANSIENT
               || yytoken == Token_VOID
               || yytoken == Token_VOLATILE
               || yytoken == Token_SEMICOLON
               || yytoken == Token_AT
               || yytoken == Token_IDENTIFIER)
          {
            std::size_t try_start_token_1 = token_stream->index() - 1;
            parser_state *try_start_state_1 = copy_current_state();
            {
              annotation_type_field_ast *__node_19 = 0;
              if (!parse_annotation_type_field(&__node_19))
                {
                  goto __catch_1;
                }
              (*yynode)->annotation_type_field_sequence = snoc((*yynode)->annotation_type_field_sequence, __node_19, memory_pool);

            }
            if (try_start_state_1)
              delete try_start_state_1;

            if (false) // the only way to enter here is using goto
              {
              __catch_1:
                if (try_start_state_1)
                  {
                    restore_state(try_start_state_1);
                    delete try_start_state_1;
                  }
                if (try_start_token_1 == token_stream->index() - 1)
                  yylex();

                while (yytoken != Token_EOF
                       && yytoken != Token_ABSTRACT
                       && yytoken != Token_BOOLEAN
                       && yytoken != Token_BYTE
                       && yytoken != Token_CHAR
                       && yytoken != Token_CLASS
                       && yytoken != Token_DOUBLE
                       && yytoken != Token_ENUM
                       && yytoken != Token_FINAL
                       && yytoken != Token_FLOAT
                       && yytoken != Token_INT
                       && yytoken != Token_INTERFACE
                       && yytoken != Token_LONG
                       && yytoken != Token_NATIVE
                       && yytoken != Token_PRIVATE
                       && yytoken != Token_PROTECTED
                       && yytoken != Token_PUBLIC
                       && yytoken != Token_SHORT
                       && yytoken != Token_STATIC
                       && yytoken != Token_STRICTFP
                       && yytoken != Token_SYNCHRONIZED
                       && yytoken != Token_TRANSIENT
                       && yytoken != Token_VOID
                       && yytoken != Token_VOLATILE
                       && yytoken != Token_RBRACE
                       && yytoken != Token_SEMICOLON
                       && yytoken != Token_AT
                       && yytoken != Token_IDENTIFIER)
                  {
                    yylex();
                  }
              }

          }
        if (yytoken != Token_RBRACE)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_RBRACE, "}");
              }
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

  bool parser::parse_annotation_type_declaration(annotation_type_declaration_ast **yynode, optional_modifiers_ast *modifiers)
  {
    *yynode = create<annotation_type_declaration_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    (*yynode)->modifiers = modifiers;
    if (modifiers && modifiers->start_token < (*yynode)->start_token)
      (*yynode)->start_token = modifiers->start_token;

    if (yytoken == Token_AT)
      {
        if (yytoken != Token_AT)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_AT, "@");
              }
            return false;
          }
        yylex();

        if (yytoken != Token_INTERFACE)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_INTERFACE, "interface");
              }
            return false;
          }
        yylex();

        identifier_ast *__node_20 = 0;
        if (!parse_identifier(&__node_20))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_identifier, "identifier");
              }
            return false;
          }
        (*yynode)->annotation_type_name = __node_20;

        annotation_type_body_ast *__node_21 = 0;
        if (!parse_annotation_type_body(&__node_21))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_annotation_type_body, "annotation_type_body");
              }
            return false;
          }
        (*yynode)->body = __node_21;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_annotation_type_field(annotation_type_field_ast **yynode)
  {
    *yynode = create<annotation_type_field_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_ABSTRACT
        || yytoken == Token_BOOLEAN
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_CLASS
        || yytoken == Token_DOUBLE
        || yytoken == Token_ENUM
        || yytoken == Token_FINAL
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_INTERFACE
        || yytoken == Token_LONG
        || yytoken == Token_NATIVE
        || yytoken == Token_PRIVATE
        || yytoken == Token_PROTECTED
        || yytoken == Token_PUBLIC
        || yytoken == Token_SHORT
        || yytoken == Token_STATIC
        || yytoken == Token_STRICTFP
        || yytoken == Token_SYNCHRONIZED
        || yytoken == Token_TRANSIENT
        || yytoken == Token_VOID
        || yytoken == Token_VOLATILE
        || yytoken == Token_SEMICOLON
        || yytoken == Token_AT
        || yytoken == Token_IDENTIFIER)
      {
        optional_modifiers_ast *modifiers = 0;

        type_ast *type = 0;

        const list_node<variable_declarator_ast *> *variable_declarator_sequence = 0;

        if (yytoken == Token_ABSTRACT
            || yytoken == Token_BOOLEAN
            || yytoken == Token_BYTE
            || yytoken == Token_CHAR
            || yytoken == Token_CLASS
            || yytoken == Token_DOUBLE
            || yytoken == Token_ENUM
            || yytoken == Token_FINAL
            || yytoken == Token_FLOAT
            || yytoken == Token_INT
            || yytoken == Token_INTERFACE
            || yytoken == Token_LONG
            || yytoken == Token_NATIVE
            || yytoken == Token_PRIVATE
            || yytoken == Token_PROTECTED
            || yytoken == Token_PUBLIC
            || yytoken == Token_SHORT
            || yytoken == Token_STATIC
            || yytoken == Token_STRICTFP
            || yytoken == Token_SYNCHRONIZED
            || yytoken == Token_TRANSIENT
            || yytoken == Token_VOID
            || yytoken == Token_VOLATILE
            || yytoken == Token_AT
            || yytoken == Token_IDENTIFIER)
          {
            optional_modifiers_ast *__node_22 = 0;
            if (!parse_optional_modifiers(&__node_22))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_optional_modifiers, "optional_modifiers");
                  }
                return false;
              }
            modifiers = __node_22;

            if (yytoken == Token_CLASS)
              {
                class_declaration_ast *__node_23 = 0;
                if (!parse_class_declaration(&__node_23, modifiers))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_class_declaration, "class_declaration");
                      }
                    return false;
                  }
                (*yynode)->class_declaration = __node_23;

              }
            else if (yytoken == Token_ENUM)
              {
                enum_declaration_ast *__node_24 = 0;
                if (!parse_enum_declaration(&__node_24, modifiers))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_enum_declaration, "enum_declaration");
                      }
                    return false;
                  }
                (*yynode)->enum_declaration = __node_24;

              }
            else if (yytoken == Token_INTERFACE)
              {
                interface_declaration_ast *__node_25 = 0;
                if (!parse_interface_declaration(&__node_25, modifiers))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_interface_declaration, "interface_declaration");
                      }
                    return false;
                  }
                (*yynode)->interface_declaration = __node_25;

              }
            else if (yytoken == Token_AT)
              {
                annotation_type_declaration_ast *__node_26 = 0;
                if (!parse_annotation_type_declaration(&__node_26, modifiers))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_annotation_type_declaration, "annotation_type_declaration");
                      }
                    return false;
                  }
                (*yynode)->annotation_type_declaration = __node_26;

              }
            else if (yytoken == Token_BOOLEAN
                     || yytoken == Token_BYTE
                     || yytoken == Token_CHAR
                     || yytoken == Token_DOUBLE
                     || yytoken == Token_FLOAT
                     || yytoken == Token_INT
                     || yytoken == Token_LONG
                     || yytoken == Token_SHORT
                     || yytoken == Token_VOID
                     || yytoken == Token_IDENTIFIER)
              {
                type_ast *__node_27 = 0;
                if (!parse_type(&__node_27))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_type, "type");
                      }
                    return false;
                  }
                type = __node_27;

                if ((yytoken == Token_IDENTIFIER) && ( LA(2).kind == Token_LPAREN ))
                  {
                    annotation_method_declaration_ast *__node_28 = 0;
                    if (!parse_annotation_method_declaration(&__node_28, modifiers, type ))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_annotation_method_declaration, "annotation_method_declaration");
                          }
                        return false;
                      }
                    (*yynode)->method_declaration = __node_28;

                  }
                else if (yytoken == Token_IDENTIFIER)
                  {
                    variable_declarator_ast *__node_29 = 0;
                    if (!parse_variable_declarator(&__node_29))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_variable_declarator, "variable_declarator");
                          }
                        return false;
                      }
                    variable_declarator_sequence = snoc(variable_declarator_sequence, __node_29, memory_pool);

                    while (yytoken == Token_COMMA)
                      {
                        if (yytoken != Token_COMMA)
                          {
                            if (!yy_block_errors)
                              {
                                yy_expected_token(yytoken, Token_COMMA, ",");
                              }
                            return false;
                          }
                        yylex();

                        variable_declarator_ast *__node_30 = 0;
                        if (!parse_variable_declarator(&__node_30))
                          {
                            if (!yy_block_errors)
                              {
                                yy_expected_symbol(ast_node::Kind_variable_declarator, "variable_declarator");
                              }
                            return false;
                          }
                        variable_declarator_sequence = snoc(variable_declarator_sequence, __node_30, memory_pool);

                      }
                    if (yytoken != Token_SEMICOLON)
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_token(yytoken, Token_SEMICOLON, ";");
                          }
                        return false;
                      }
                    yylex();

                    variable_declaration_data_ast *__node_31 = 0;
                    if (!parse_variable_declaration_data(&__node_31,
                                                         modifiers, type, variable_declarator_sequence
                                                        ))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_variable_declaration_data, "variable_declaration_data");
                          }
                        return false;
                      }
                    (*yynode)->constant_declaration = __node_31;

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
          }
        else if (yytoken == Token_SEMICOLON)
          {
            if (yytoken != Token_SEMICOLON)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_SEMICOLON, ";");
                  }
                return false;
              }
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

  bool parser::parse_array_access(array_access_ast **yynode)
  {
    *yynode = create<array_access_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_LBRACKET)
      {
        if (yytoken != Token_LBRACKET)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_LBRACKET, "[");
              }
            return false;
          }
        yylex();

        expression_ast *__node_32 = 0;
        if (!parse_expression(&__node_32))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_expression, "expression");
              }
            return false;
          }
        (*yynode)->array_index_expression = __node_32;

        if (yytoken != Token_RBRACKET)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_RBRACKET, "]");
              }
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

  bool parser::parse_array_creator_rest(array_creator_rest_ast **yynode)
  {
    *yynode = create<array_creator_rest_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_LBRACKET)
      {
        if ((yytoken == Token_LBRACKET) && ( LA(2).kind == Token_RBRACKET ))
          {
            mandatory_declarator_brackets_ast *__node_33 = 0;
            if (!parse_mandatory_declarator_brackets(&__node_33))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_mandatory_declarator_brackets, "mandatory_declarator_brackets");
                  }
                return false;
              }
            (*yynode)->mandatory_declarator_brackets = __node_33;

            variable_array_initializer_ast *__node_34 = 0;
            if (!parse_variable_array_initializer(&__node_34))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_variable_array_initializer, "variable_array_initializer");
                  }
                return false;
              }
            (*yynode)->array_initializer = __node_34;

          }
        else if (yytoken == Token_LBRACKET)
          {
            if (yytoken != Token_LBRACKET)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LBRACKET, "[");
                  }
                return false;
              }
            yylex();

            expression_ast *__node_35 = 0;
            if (!parse_expression(&__node_35))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_expression, "expression");
                  }
                return false;
              }
            (*yynode)->index_expression_sequence = snoc((*yynode)->index_expression_sequence, __node_35, memory_pool);

            if (yytoken != Token_RBRACKET)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_RBRACKET, "]");
                  }
                return false;
              }
            yylex();

            while (yytoken == Token_LBRACKET)
              {
                if (LA(2).kind == Token_RBRACKET)
                  {
                    break;
                  }
                if (yytoken != Token_LBRACKET)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_LBRACKET, "[");
                      }
                    return false;
                  }
                yylex();

                expression_ast *__node_36 = 0;
                if (!parse_expression(&__node_36))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_expression, "expression");
                      }
                    return false;
                  }
                (*yynode)->index_expression_sequence = snoc((*yynode)->index_expression_sequence, __node_36, memory_pool);

                if (yytoken != Token_RBRACKET)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_RBRACKET, "]");
                      }
                    return false;
                  }
                yylex();

              }
            optional_declarator_brackets_ast *__node_37 = 0;
            if (!parse_optional_declarator_brackets(&__node_37))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_optional_declarator_brackets, "optional_declarator_brackets");
                  }
                return false;
              }
            (*yynode)->optional_declarator_brackets = __node_37;

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

  bool parser::parse_array_type_dot_class(array_type_dot_class_ast **yynode)
  {
    *yynode = create<array_type_dot_class_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_IDENTIFIER)
      {
        qualified_identifier_ast *__node_38 = 0;
        if (!parse_qualified_identifier(&__node_38))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_qualified_identifier, "qualified_identifier");
              }
            return false;
          }
        (*yynode)->qualified_identifier = __node_38;

        mandatory_declarator_brackets_ast *__node_39 = 0;
        if (!parse_mandatory_declarator_brackets(&__node_39))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_mandatory_declarator_brackets, "mandatory_declarator_brackets");
              }
            return false;
          }
        (*yynode)->declarator_brackets = __node_39;

        if (yytoken != Token_DOT)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_DOT, ".");
              }
            return false;
          }
        yylex();

        if (yytoken != Token_CLASS)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_CLASS, "class");
              }
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

  bool parser::parse_assert_statement(assert_statement_ast **yynode)
  {
    *yynode = create<assert_statement_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_ASSERT)
      {
        if (yytoken != Token_ASSERT)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_ASSERT, "assert");
              }
            return false;
          }
        yylex();

        expression_ast *__node_40 = 0;
        if (!parse_expression(&__node_40))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_expression, "expression");
              }
            return false;
          }
        (*yynode)->condition = __node_40;

        if (yytoken == Token_COLON)
          {
            if (yytoken != Token_COLON)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_COLON, ":");
                  }
                return false;
              }
            yylex();

            expression_ast *__node_41 = 0;
            if (!parse_expression(&__node_41))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_expression, "expression");
                  }
                return false;
              }
            (*yynode)->message = __node_41;

          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        if (yytoken != Token_SEMICOLON)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_SEMICOLON, ";");
              }
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

  bool parser::parse_bit_and_expression(bit_and_expression_ast **yynode)
  {
    *yynode = create<bit_and_expression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOOLEAN
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_SHORT
        || yytoken == Token_SUPER
        || yytoken == Token_THIS
        || yytoken == Token_VOID
        || yytoken == Token_LPAREN
        || yytoken == Token_LESS_THAN
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_FLOATING_POINT_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        equality_expression_ast *__node_42 = 0;
        if (!parse_equality_expression(&__node_42))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_equality_expression, "equality_expression");
              }
            return false;
          }
        (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_42, memory_pool);

        while (yytoken == Token_BIT_AND)
          {
            if (yytoken != Token_BIT_AND)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_BIT_AND, "&");
                  }
                return false;
              }
            yylex();

            equality_expression_ast *__node_43 = 0;
            if (!parse_equality_expression(&__node_43))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_equality_expression, "equality_expression");
                  }
                return false;
              }
            (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_43, memory_pool);

          }
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_bit_or_expression(bit_or_expression_ast **yynode)
  {
    *yynode = create<bit_or_expression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOOLEAN
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_SHORT
        || yytoken == Token_SUPER
        || yytoken == Token_THIS
        || yytoken == Token_VOID
        || yytoken == Token_LPAREN
        || yytoken == Token_LESS_THAN
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_FLOATING_POINT_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        bit_xor_expression_ast *__node_44 = 0;
        if (!parse_bit_xor_expression(&__node_44))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_bit_xor_expression, "bit_xor_expression");
              }
            return false;
          }
        (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_44, memory_pool);

        while (yytoken == Token_BIT_OR)
          {
            if (yytoken != Token_BIT_OR)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_BIT_OR, "|");
                  }
                return false;
              }
            yylex();

            bit_xor_expression_ast *__node_45 = 0;
            if (!parse_bit_xor_expression(&__node_45))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_bit_xor_expression, "bit_xor_expression");
                  }
                return false;
              }
            (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_45, memory_pool);

          }
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_bit_xor_expression(bit_xor_expression_ast **yynode)
  {
    *yynode = create<bit_xor_expression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOOLEAN
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_SHORT
        || yytoken == Token_SUPER
        || yytoken == Token_THIS
        || yytoken == Token_VOID
        || yytoken == Token_LPAREN
        || yytoken == Token_LESS_THAN
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_FLOATING_POINT_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        bit_and_expression_ast *__node_46 = 0;
        if (!parse_bit_and_expression(&__node_46))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_bit_and_expression, "bit_and_expression");
              }
            return false;
          }
        (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_46, memory_pool);

        while (yytoken == Token_BIT_XOR)
          {
            if (yytoken != Token_BIT_XOR)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_BIT_XOR, "^");
                  }
                return false;
              }
            yylex();

            bit_and_expression_ast *__node_47 = 0;
            if (!parse_bit_and_expression(&__node_47))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_bit_and_expression, "bit_and_expression");
                  }
                return false;
              }
            (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_47, memory_pool);

          }
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_block(block_ast **yynode)
  {
    *yynode = create<block_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_LBRACE)
      {
        if (yytoken != Token_LBRACE)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_LBRACE, "{");
              }
            return false;
          }
        yylex();

        while (yytoken == Token_ABSTRACT
               || yytoken == Token_ASSERT
               || yytoken == Token_BOOLEAN
               || yytoken == Token_BREAK
               || yytoken == Token_BYTE
               || yytoken == Token_CHAR
               || yytoken == Token_CLASS
               || yytoken == Token_CONTINUE
               || yytoken == Token_DO
               || yytoken == Token_DOUBLE
               || yytoken == Token_ENUM
               || yytoken == Token_FINAL
               || yytoken == Token_FLOAT
               || yytoken == Token_FOR
               || yytoken == Token_IF
               || yytoken == Token_INT
               || yytoken == Token_INTERFACE
               || yytoken == Token_LONG
               || yytoken == Token_NATIVE
               || yytoken == Token_NEW
               || yytoken == Token_PRIVATE
               || yytoken == Token_PROTECTED
               || yytoken == Token_PUBLIC
               || yytoken == Token_RETURN
               || yytoken == Token_SHORT
               || yytoken == Token_STATIC
               || yytoken == Token_STRICTFP
               || yytoken == Token_SUPER
               || yytoken == Token_SWITCH
               || yytoken == Token_SYNCHRONIZED
               || yytoken == Token_THIS
               || yytoken == Token_THROW
               || yytoken == Token_TRANSIENT
               || yytoken == Token_TRY
               || yytoken == Token_VOID
               || yytoken == Token_VOLATILE
               || yytoken == Token_WHILE
               || yytoken == Token_LPAREN
               || yytoken == Token_LBRACE
               || yytoken == Token_SEMICOLON
               || yytoken == Token_AT
               || yytoken == Token_LESS_THAN
               || yytoken == Token_BANG
               || yytoken == Token_TILDE
               || yytoken == Token_INCREMENT
               || yytoken == Token_DECREMENT
               || yytoken == Token_PLUS
               || yytoken == Token_MINUS
               || yytoken == Token_TRUE
               || yytoken == Token_FALSE
               || yytoken == Token_NULL
               || yytoken == Token_INTEGER_LITERAL
               || yytoken == Token_FLOATING_POINT_LITERAL
               || yytoken == Token_CHARACTER_LITERAL
               || yytoken == Token_STRING_LITERAL
               || yytoken == Token_IDENTIFIER)
          {
            std::size_t try_start_token_2 = token_stream->index() - 1;
            parser_state *try_start_state_2 = copy_current_state();
            {
              block_statement_ast *__node_48 = 0;
              if (!parse_block_statement(&__node_48))
                {
                  goto __catch_2;
                }
              (*yynode)->statement_sequence = snoc((*yynode)->statement_sequence, __node_48, memory_pool);

            }
            if (try_start_state_2)
              delete try_start_state_2;

            if (false) // the only way to enter here is using goto
              {
              __catch_2:
                if (try_start_state_2)
                  {
                    restore_state(try_start_state_2);
                    delete try_start_state_2;
                  }
                if (try_start_token_2 == token_stream->index() - 1)
                  yylex();

                while (yytoken != Token_EOF
                       && yytoken != Token_ABSTRACT
                       && yytoken != Token_ASSERT
                       && yytoken != Token_BOOLEAN
                       && yytoken != Token_BREAK
                       && yytoken != Token_BYTE
                       && yytoken != Token_CHAR
                       && yytoken != Token_CLASS
                       && yytoken != Token_CONTINUE
                       && yytoken != Token_DO
                       && yytoken != Token_DOUBLE
                       && yytoken != Token_ENUM
                       && yytoken != Token_FINAL
                       && yytoken != Token_FLOAT
                       && yytoken != Token_FOR
                       && yytoken != Token_IF
                       && yytoken != Token_INT
                       && yytoken != Token_INTERFACE
                       && yytoken != Token_LONG
                       && yytoken != Token_NATIVE
                       && yytoken != Token_NEW
                       && yytoken != Token_PRIVATE
                       && yytoken != Token_PROTECTED
                       && yytoken != Token_PUBLIC
                       && yytoken != Token_RETURN
                       && yytoken != Token_SHORT
                       && yytoken != Token_STATIC
                       && yytoken != Token_STRICTFP
                       && yytoken != Token_SUPER
                       && yytoken != Token_SWITCH
                       && yytoken != Token_SYNCHRONIZED
                       && yytoken != Token_THIS
                       && yytoken != Token_THROW
                       && yytoken != Token_TRANSIENT
                       && yytoken != Token_TRY
                       && yytoken != Token_VOID
                       && yytoken != Token_VOLATILE
                       && yytoken != Token_WHILE
                       && yytoken != Token_LPAREN
                       && yytoken != Token_LBRACE
                       && yytoken != Token_RBRACE
                       && yytoken != Token_SEMICOLON
                       && yytoken != Token_AT
                       && yytoken != Token_LESS_THAN
                       && yytoken != Token_BANG
                       && yytoken != Token_TILDE
                       && yytoken != Token_INCREMENT
                       && yytoken != Token_DECREMENT
                       && yytoken != Token_PLUS
                       && yytoken != Token_MINUS
                       && yytoken != Token_TRUE
                       && yytoken != Token_FALSE
                       && yytoken != Token_NULL
                       && yytoken != Token_INTEGER_LITERAL
                       && yytoken != Token_FLOATING_POINT_LITERAL
                       && yytoken != Token_CHARACTER_LITERAL
                       && yytoken != Token_STRING_LITERAL
                       && yytoken != Token_IDENTIFIER)
                  {
                    yylex();
                  }
              }

          }
        if (yytoken != Token_RBRACE)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_RBRACE, "}");
              }
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

  bool parser::parse_block_statement(block_statement_ast **yynode)
  {
    *yynode = create<block_statement_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_ABSTRACT
        || yytoken == Token_ASSERT
        || yytoken == Token_BOOLEAN
        || yytoken == Token_BREAK
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_CLASS
        || yytoken == Token_CONTINUE
        || yytoken == Token_DO
        || yytoken == Token_DOUBLE
        || yytoken == Token_ENUM
        || yytoken == Token_FINAL
        || yytoken == Token_FLOAT
        || yytoken == Token_FOR
        || yytoken == Token_IF
        || yytoken == Token_INT
        || yytoken == Token_INTERFACE
        || yytoken == Token_LONG
        || yytoken == Token_NATIVE
        || yytoken == Token_NEW
        || yytoken == Token_PRIVATE
        || yytoken == Token_PROTECTED
        || yytoken == Token_PUBLIC
        || yytoken == Token_RETURN
        || yytoken == Token_SHORT
        || yytoken == Token_STATIC
        || yytoken == Token_STRICTFP
        || yytoken == Token_SUPER
        || yytoken == Token_SWITCH
        || yytoken == Token_SYNCHRONIZED
        || yytoken == Token_THIS
        || yytoken == Token_THROW
        || yytoken == Token_TRANSIENT
        || yytoken == Token_TRY
        || yytoken == Token_VOID
        || yytoken == Token_VOLATILE
        || yytoken == Token_WHILE
        || yytoken == Token_LPAREN
        || yytoken == Token_LBRACE
        || yytoken == Token_SEMICOLON
        || yytoken == Token_AT
        || yytoken == Token_LESS_THAN
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_FLOATING_POINT_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        optional_modifiers_ast *modifiers = 0;

        bool block_errors_3 = block_errors(true);
        std::size_t try_start_token_3 = token_stream->index() - 1;
        parser_state *try_start_state_3 = copy_current_state();
        {
          variable_declaration_statement_ast *__node_49 = 0;
          if (!parse_variable_declaration_statement(&__node_49))
            {
              goto __catch_3;
            }
          (*yynode)->variable_declaration_statement = __node_49;

        }
        block_errors(block_errors_3);
        if (try_start_state_3)
          delete try_start_state_3;

        if (false) // the only way to enter here is using goto
          {
          __catch_3:
            if (try_start_state_3)
              {
                restore_state(try_start_state_3);
                delete try_start_state_3;
              }
            block_errors(block_errors_3);
            rewind(try_start_token_3);

            if ((yytoken == Token_ASSERT
                 || yytoken == Token_BOOLEAN
                 || yytoken == Token_BREAK
                 || yytoken == Token_BYTE
                 || yytoken == Token_CHAR
                 || yytoken == Token_CONTINUE
                 || yytoken == Token_DO
                 || yytoken == Token_DOUBLE
                 || yytoken == Token_FLOAT
                 || yytoken == Token_FOR
                 || yytoken == Token_IF
                 || yytoken == Token_INT
                 || yytoken == Token_LONG
                 || yytoken == Token_NEW
                 || yytoken == Token_RETURN
                 || yytoken == Token_SHORT
                 || yytoken == Token_SUPER
                 || yytoken == Token_SWITCH
                 || yytoken == Token_SYNCHRONIZED
                 || yytoken == Token_THIS
                 || yytoken == Token_THROW
                 || yytoken == Token_TRY
                 || yytoken == Token_VOID
                 || yytoken == Token_WHILE
                 || yytoken == Token_LPAREN
                 || yytoken == Token_LBRACE
                 || yytoken == Token_SEMICOLON
                 || yytoken == Token_LESS_THAN
                 || yytoken == Token_BANG
                 || yytoken == Token_TILDE
                 || yytoken == Token_INCREMENT
                 || yytoken == Token_DECREMENT
                 || yytoken == Token_PLUS
                 || yytoken == Token_MINUS
                 || yytoken == Token_TRUE
                 || yytoken == Token_FALSE
                 || yytoken == Token_NULL
                 || yytoken == Token_INTEGER_LITERAL
                 || yytoken == Token_FLOATING_POINT_LITERAL
                 || yytoken == Token_CHARACTER_LITERAL
                 || yytoken == Token_STRING_LITERAL
                 || yytoken == Token_IDENTIFIER) && ( (yytoken != Token_SYNCHRONIZED) ||
                                                      (yytoken == Token_SYNCHRONIZED && LA(2).kind == Token_LPAREN)
                                                    ))
              {
                embedded_statement_ast *__node_50 = 0;
                if (!parse_embedded_statement(&__node_50))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_embedded_statement, "embedded_statement");
                      }
                    return false;
                  }
                (*yynode)->statement = __node_50;

              }
            else if (yytoken == Token_ABSTRACT
                     || yytoken == Token_CLASS
                     || yytoken == Token_ENUM
                     || yytoken == Token_FINAL
                     || yytoken == Token_INTERFACE
                     || yytoken == Token_NATIVE
                     || yytoken == Token_PRIVATE
                     || yytoken == Token_PROTECTED
                     || yytoken == Token_PUBLIC
                     || yytoken == Token_STATIC
                     || yytoken == Token_STRICTFP
                     || yytoken == Token_SYNCHRONIZED
                     || yytoken == Token_TRANSIENT
                     || yytoken == Token_VOLATILE
                     || yytoken == Token_AT)
              {
                optional_modifiers_ast *__node_51 = 0;
                if (!parse_optional_modifiers(&__node_51))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_optional_modifiers, "optional_modifiers");
                      }
                    return false;
                  }
                modifiers = __node_51;

                if (yytoken == Token_CLASS)
                  {
                    class_declaration_ast *__node_52 = 0;
                    if (!parse_class_declaration(&__node_52, modifiers))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_class_declaration, "class_declaration");
                          }
                        return false;
                      }
                    (*yynode)->class_declaration = __node_52;

                  }
                else if (yytoken == Token_ENUM)
                  {
                    enum_declaration_ast *__node_53 = 0;
                    if (!parse_enum_declaration(&__node_53, modifiers))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_enum_declaration, "enum_declaration");
                          }
                        return false;
                      }
                    (*yynode)->enum_declaration = __node_53;

                  }
                else if (yytoken == Token_INTERFACE)
                  {
                    interface_declaration_ast *__node_54 = 0;
                    if (!parse_interface_declaration(&__node_54, modifiers))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_interface_declaration, "interface_declaration");
                          }
                        return false;
                      }
                    (*yynode)->interface_declaration = __node_54;

                  }
                else if (yytoken == Token_AT)
                  {
                    annotation_type_declaration_ast *__node_55 = 0;
                    if (!parse_annotation_type_declaration(&__node_55, modifiers))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_annotation_type_declaration, "annotation_type_declaration");
                          }
                        return false;
                      }
                    (*yynode)->annotation_type_declaration = __node_55;

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
          }

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_break_statement(break_statement_ast **yynode)
  {
    *yynode = create<break_statement_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BREAK)
      {
        if (yytoken != Token_BREAK)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_BREAK, "break");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_IDENTIFIER)
          {
            identifier_ast *__node_56 = 0;
            if (!parse_identifier(&__node_56))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                  }
                return false;
              }
            (*yynode)->label = __node_56;

          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        if (yytoken != Token_SEMICOLON)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_SEMICOLON, ";");
              }
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

  bool parser::parse_builtin_type(builtin_type_ast **yynode)
  {
    *yynode = create<builtin_type_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOOLEAN
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_SHORT
        || yytoken == Token_VOID)
      {
        if (yytoken == Token_VOID)
          {
            if (yytoken != Token_VOID)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_VOID, "void");
                  }
                return false;
              }
            yylex();

            (*yynode)->type = builtin_type::type_void;
          }
        else if (yytoken == Token_BOOLEAN)
          {
            if (yytoken != Token_BOOLEAN)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_BOOLEAN, "boolean");
                  }
                return false;
              }
            yylex();

            (*yynode)->type = builtin_type::type_boolean;
          }
        else if (yytoken == Token_BYTE)
          {
            if (yytoken != Token_BYTE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_BYTE, "byte");
                  }
                return false;
              }
            yylex();

            (*yynode)->type = builtin_type::type_byte;
          }
        else if (yytoken == Token_CHAR)
          {
            if (yytoken != Token_CHAR)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_CHAR, "char");
                  }
                return false;
              }
            yylex();

            (*yynode)->type = builtin_type::type_char;
          }
        else if (yytoken == Token_SHORT)
          {
            if (yytoken != Token_SHORT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_SHORT, "short");
                  }
                return false;
              }
            yylex();

            (*yynode)->type = builtin_type::type_short;
          }
        else if (yytoken == Token_INT)
          {
            if (yytoken != Token_INT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_INT, "int");
                  }
                return false;
              }
            yylex();

            (*yynode)->type = builtin_type::type_int;
          }
        else if (yytoken == Token_FLOAT)
          {
            if (yytoken != Token_FLOAT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_FLOAT, "float");
                  }
                return false;
              }
            yylex();

            (*yynode)->type = builtin_type::type_float;
          }
        else if (yytoken == Token_LONG)
          {
            if (yytoken != Token_LONG)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LONG, "long");
                  }
                return false;
              }
            yylex();

            (*yynode)->type = builtin_type::type_long;
          }
        else if (yytoken == Token_DOUBLE)
          {
            if (yytoken != Token_DOUBLE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_DOUBLE, "double");
                  }
                return false;
              }
            yylex();

            (*yynode)->type = builtin_type::type_double;
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

  bool parser::parse_builtin_type_dot_class(builtin_type_dot_class_ast **yynode)
  {
    *yynode = create<builtin_type_dot_class_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOOLEAN
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_SHORT
        || yytoken == Token_VOID)
      {
        optional_array_builtin_type_ast *__node_57 = 0;
        if (!parse_optional_array_builtin_type(&__node_57))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_optional_array_builtin_type, "optional_array_builtin_type");
              }
            return false;
          }
        (*yynode)->builtin_type = __node_57;

        if (yytoken != Token_DOT)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_DOT, ".");
              }
            return false;
          }
        yylex();

        if (yytoken != Token_CLASS)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_CLASS, "class");
              }
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

  bool parser::parse_cast_expression(cast_expression_ast **yynode)
  {
    *yynode = create<cast_expression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_LPAREN)
      {
        if (yytoken != Token_LPAREN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_LPAREN, "(");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_BOOLEAN
            || yytoken == Token_BYTE
            || yytoken == Token_CHAR
            || yytoken == Token_DOUBLE
            || yytoken == Token_FLOAT
            || yytoken == Token_INT
            || yytoken == Token_LONG
            || yytoken == Token_SHORT
            || yytoken == Token_VOID)
          {
            optional_array_builtin_type_ast *__node_58 = 0;
            if (!parse_optional_array_builtin_type(&__node_58))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_optional_array_builtin_type, "optional_array_builtin_type");
                  }
                return false;
              }
            (*yynode)->builtin_type = __node_58;

            if (yytoken != Token_RPAREN)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_RPAREN, ")");
                  }
                return false;
              }
            yylex();

            unary_expression_ast *__node_59 = 0;
            if (!parse_unary_expression(&__node_59))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_unary_expression, "unary_expression");
                  }
                return false;
              }
            (*yynode)->builtin_casted_expression = __node_59;

          }
        else if (yytoken == Token_IDENTIFIER)
          {
            class_type_ast *__node_60 = 0;
            if (!parse_class_type(&__node_60))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_class_type, "class_type");
                  }
                return false;
              }
            (*yynode)->class_type = __node_60;

            if (yytoken != Token_RPAREN)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_RPAREN, ")");
                  }
                return false;
              }
            yylex();

            unary_expression_not_plusminus_ast *__node_61 = 0;
            if (!parse_unary_expression_not_plusminus(&__node_61))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_unary_expression_not_plusminus, "unary_expression_not_plusminus");
                  }
                return false;
              }
            (*yynode)->class_casted_expression = __node_61;

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

  bool parser::parse_catch_clause(catch_clause_ast **yynode)
  {
    *yynode = create<catch_clause_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_CATCH)
      {
        parameter_declaration_ast *exception_parameter = 0;

        if (yytoken != Token_CATCH)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_CATCH, "catch");
              }
            return false;
          }
        yylex();

        if (yytoken != Token_LPAREN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_LPAREN, "(");
              }
            return false;
          }
        yylex();

        parameter_declaration_ast *__node_62 = 0;
        if (!parse_parameter_declaration(&__node_62))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_parameter_declaration, "parameter_declaration");
              }
            return false;
          }
        exception_parameter = __node_62;

        if (yytoken != Token_RPAREN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_RPAREN, ")");
              }
            return false;
          }
        yylex();

        variable_declaration_split_data_ast *__node_63 = 0;
        if (!parse_variable_declaration_split_data(&__node_63, exception_parameter, 0))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_variable_declaration_split_data, "variable_declaration_split_data");
              }
            return false;
          }
        (*yynode)->exception_declaration = __node_63;

        block_ast *__node_64 = 0;
        if (!parse_block(&__node_64))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_block, "block");
              }
            return false;
          }
        (*yynode)->body = __node_64;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_class_access_data(class_access_data_ast **yynode)
  {
    *yynode = create<class_access_data_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (true /*epsilon*/ || yytoken == Token_INSTANCEOF
        || yytoken == Token_RPAREN
        || yytoken == Token_RBRACE
        || yytoken == Token_LBRACKET
        || yytoken == Token_RBRACKET
        || yytoken == Token_SEMICOLON
        || yytoken == Token_COMMA
        || yytoken == Token_DOT
        || yytoken == Token_ASSIGN
        || yytoken == Token_LESS_THAN
        || yytoken == Token_GREATER_THAN
        || yytoken == Token_QUESTION
        || yytoken == Token_COLON
        || yytoken == Token_EQUAL
        || yytoken == Token_LESS_EQUAL
        || yytoken == Token_GREATER_EQUAL
        || yytoken == Token_NOT_EQUAL
        || yytoken == Token_LOG_AND
        || yytoken == Token_LOG_OR
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_STAR
        || yytoken == Token_SLASH
        || yytoken == Token_BIT_AND
        || yytoken == Token_BIT_OR
        || yytoken == Token_BIT_XOR
        || yytoken == Token_REMAINDER
        || yytoken == Token_LSHIFT
        || yytoken == Token_SIGNED_RSHIFT
        || yytoken == Token_UNSIGNED_RSHIFT
        || yytoken == Token_PLUS_ASSIGN
        || yytoken == Token_MINUS_ASSIGN
        || yytoken == Token_STAR_ASSIGN
        || yytoken == Token_SLASH_ASSIGN
        || yytoken == Token_BIT_AND_ASSIGN
        || yytoken == Token_BIT_OR_ASSIGN
        || yytoken == Token_BIT_XOR_ASSIGN
        || yytoken == Token_REMAINDER_ASSIGN
        || yytoken == Token_LSHIFT_ASSIGN
        || yytoken == Token_SIGNED_RSHIFT_ASSIGN
        || yytoken == Token_UNSIGNED_RSHIFT_ASSIGN)
    {}
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_class_body(class_body_ast **yynode)
  {
    *yynode = create<class_body_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_LBRACE)
      {
        if (yytoken != Token_LBRACE)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_LBRACE, "{");
              }
            return false;
          }
        yylex();

        while (yytoken == Token_ABSTRACT
               || yytoken == Token_BOOLEAN
               || yytoken == Token_BYTE
               || yytoken == Token_CHAR
               || yytoken == Token_CLASS
               || yytoken == Token_DOUBLE
               || yytoken == Token_ENUM
               || yytoken == Token_FINAL
               || yytoken == Token_FLOAT
               || yytoken == Token_INT
               || yytoken == Token_INTERFACE
               || yytoken == Token_LONG
               || yytoken == Token_NATIVE
               || yytoken == Token_PRIVATE
               || yytoken == Token_PROTECTED
               || yytoken == Token_PUBLIC
               || yytoken == Token_SHORT
               || yytoken == Token_STATIC
               || yytoken == Token_STRICTFP
               || yytoken == Token_SYNCHRONIZED
               || yytoken == Token_TRANSIENT
               || yytoken == Token_VOID
               || yytoken == Token_VOLATILE
               || yytoken == Token_LBRACE
               || yytoken == Token_SEMICOLON
               || yytoken == Token_AT
               || yytoken == Token_LESS_THAN
               || yytoken == Token_IDENTIFIER)
          {
            std::size_t try_start_token_4 = token_stream->index() - 1;
            parser_state *try_start_state_4 = copy_current_state();
            {
              class_field_ast *__node_65 = 0;
              if (!parse_class_field(&__node_65))
                {
                  goto __catch_4;
                }
              (*yynode)->declaration_sequence = snoc((*yynode)->declaration_sequence, __node_65, memory_pool);

            }
            if (try_start_state_4)
              delete try_start_state_4;

            if (false) // the only way to enter here is using goto
              {
              __catch_4:
                if (try_start_state_4)
                  {
                    restore_state(try_start_state_4);
                    delete try_start_state_4;
                  }
                if (try_start_token_4 == token_stream->index() - 1)
                  yylex();

                while (yytoken != Token_EOF
                       && yytoken != Token_ABSTRACT
                       && yytoken != Token_BOOLEAN
                       && yytoken != Token_BYTE
                       && yytoken != Token_CHAR
                       && yytoken != Token_CLASS
                       && yytoken != Token_DOUBLE
                       && yytoken != Token_ENUM
                       && yytoken != Token_FINAL
                       && yytoken != Token_FLOAT
                       && yytoken != Token_INT
                       && yytoken != Token_INTERFACE
                       && yytoken != Token_LONG
                       && yytoken != Token_NATIVE
                       && yytoken != Token_PRIVATE
                       && yytoken != Token_PROTECTED
                       && yytoken != Token_PUBLIC
                       && yytoken != Token_SHORT
                       && yytoken != Token_STATIC
                       && yytoken != Token_STRICTFP
                       && yytoken != Token_SYNCHRONIZED
                       && yytoken != Token_TRANSIENT
                       && yytoken != Token_VOID
                       && yytoken != Token_VOLATILE
                       && yytoken != Token_LBRACE
                       && yytoken != Token_RBRACE
                       && yytoken != Token_SEMICOLON
                       && yytoken != Token_AT
                       && yytoken != Token_LESS_THAN
                       && yytoken != Token_IDENTIFIER)
                  {
                    yylex();
                  }
              }

          }
        if (yytoken != Token_RBRACE)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_RBRACE, "}");
              }
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

  bool parser::parse_class_declaration(class_declaration_ast **yynode, optional_modifiers_ast *modifiers)
  {
    *yynode = create<class_declaration_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    (*yynode)->modifiers = modifiers;
    if (modifiers && modifiers->start_token < (*yynode)->start_token)
      (*yynode)->start_token = modifiers->start_token;

    if (yytoken == Token_CLASS)
      {
        if (yytoken != Token_CLASS)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_CLASS, "class");
              }
            return false;
          }
        yylex();

        identifier_ast *__node_66 = 0;
        if (!parse_identifier(&__node_66))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_identifier, "identifier");
              }
            return false;
          }
        (*yynode)->class_name = __node_66;

        if ((yytoken == Token_LESS_THAN) && ( compatibility_mode() >= java15_compatibility ))
          {
            type_parameters_ast *__node_67 = 0;
            if (!parse_type_parameters(&__node_67))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_type_parameters, "type_parameters");
                  }
                return false;
              }
            (*yynode)->type_parameters = __node_67;

          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        if (yytoken == Token_EXTENDS)
          {
            class_extends_clause_ast *__node_68 = 0;
            if (!parse_class_extends_clause(&__node_68))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_class_extends_clause, "class_extends_clause");
                  }
                return false;
              }
            (*yynode)->extends = __node_68;

          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        if (yytoken == Token_IMPLEMENTS)
          {
            implements_clause_ast *__node_69 = 0;
            if (!parse_implements_clause(&__node_69))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_implements_clause, "implements_clause");
                  }
                return false;
              }
            (*yynode)->implements = __node_69;

          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        class_body_ast *__node_70 = 0;
        if (!parse_class_body(&__node_70))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_class_body, "class_body");
              }
            return false;
          }
        (*yynode)->body = __node_70;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_class_extends_clause(class_extends_clause_ast **yynode)
  {
    *yynode = create<class_extends_clause_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_EXTENDS)
      {
        if (yytoken != Token_EXTENDS)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_EXTENDS, "extends");
              }
            return false;
          }
        yylex();

        class_or_interface_type_name_ast *__node_71 = 0;
        if (!parse_class_or_interface_type_name(&__node_71))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_class_or_interface_type_name, "class_or_interface_type_name");
              }
            return false;
          }
        (*yynode)->type = __node_71;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_class_field(class_field_ast **yynode)
  {
    *yynode = create<class_field_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_ABSTRACT
        || yytoken == Token_BOOLEAN
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_CLASS
        || yytoken == Token_DOUBLE
        || yytoken == Token_ENUM
        || yytoken == Token_FINAL
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_INTERFACE
        || yytoken == Token_LONG
        || yytoken == Token_NATIVE
        || yytoken == Token_PRIVATE
        || yytoken == Token_PROTECTED
        || yytoken == Token_PUBLIC
        || yytoken == Token_SHORT
        || yytoken == Token_STATIC
        || yytoken == Token_STRICTFP
        || yytoken == Token_SYNCHRONIZED
        || yytoken == Token_TRANSIENT
        || yytoken == Token_VOID
        || yytoken == Token_VOLATILE
        || yytoken == Token_LBRACE
        || yytoken == Token_SEMICOLON
        || yytoken == Token_AT
        || yytoken == Token_LESS_THAN
        || yytoken == Token_IDENTIFIER)
      {
        optional_modifiers_ast *modifiers = 0;

        type_parameters_ast *type_parameters = 0;

        type_ast *type = 0;

        const list_node<variable_declarator_ast *> *variable_declarator_sequence = 0;

        if ((yytoken == Token_ABSTRACT
             || yytoken == Token_BOOLEAN
             || yytoken == Token_BYTE
             || yytoken == Token_CHAR
             || yytoken == Token_CLASS
             || yytoken == Token_DOUBLE
             || yytoken == Token_ENUM
             || yytoken == Token_FINAL
             || yytoken == Token_FLOAT
             || yytoken == Token_INT
             || yytoken == Token_INTERFACE
             || yytoken == Token_LONG
             || yytoken == Token_NATIVE
             || yytoken == Token_PRIVATE
             || yytoken == Token_PROTECTED
             || yytoken == Token_PUBLIC
             || yytoken == Token_SHORT
             || yytoken == Token_STATIC
             || yytoken == Token_STRICTFP
             || yytoken == Token_SYNCHRONIZED
             || yytoken == Token_TRANSIENT
             || yytoken == Token_VOID
             || yytoken == Token_VOLATILE
             || yytoken == Token_AT
             || yytoken == Token_LESS_THAN
             || yytoken == Token_IDENTIFIER) && ( !(yytoken == Token_STATIC && LA(2).kind == Token_LBRACE) ))
          {
            optional_modifiers_ast *__node_72 = 0;
            if (!parse_optional_modifiers(&__node_72))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_optional_modifiers, "optional_modifiers");
                  }
                return false;
              }
            modifiers = __node_72;

            if (yytoken == Token_CLASS)
              {
                class_declaration_ast *__node_73 = 0;
                if (!parse_class_declaration(&__node_73, modifiers))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_class_declaration, "class_declaration");
                      }
                    return false;
                  }
                (*yynode)->class_declaration = __node_73;

              }
            else if (yytoken == Token_ENUM)
              {
                enum_declaration_ast *__node_74 = 0;
                if (!parse_enum_declaration(&__node_74, modifiers))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_enum_declaration, "enum_declaration");
                      }
                    return false;
                  }
                (*yynode)->enum_declaration = __node_74;

              }
            else if (yytoken == Token_INTERFACE)
              {
                interface_declaration_ast *__node_75 = 0;
                if (!parse_interface_declaration(&__node_75, modifiers))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_interface_declaration, "interface_declaration");
                      }
                    return false;
                  }
                (*yynode)->interface_declaration = __node_75;

              }
            else if (yytoken == Token_AT)
              {
                annotation_type_declaration_ast *__node_76 = 0;
                if (!parse_annotation_type_declaration(&__node_76, modifiers))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_annotation_type_declaration, "annotation_type_declaration");
                      }
                    return false;
                  }
                (*yynode)->annotation_type_declaration = __node_76;

              }
            else if (yytoken == Token_BOOLEAN
                     || yytoken == Token_BYTE
                     || yytoken == Token_CHAR
                     || yytoken == Token_DOUBLE
                     || yytoken == Token_FLOAT
                     || yytoken == Token_INT
                     || yytoken == Token_LONG
                     || yytoken == Token_SHORT
                     || yytoken == Token_VOID
                     || yytoken == Token_LESS_THAN
                     || yytoken == Token_IDENTIFIER)
              {
                if ((yytoken == Token_LESS_THAN) && ( compatibility_mode() >= java15_compatibility ))
                  {
                    type_parameters_ast *__node_77 = 0;
                    if (!parse_type_parameters(&__node_77))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_type_parameters, "type_parameters");
                          }
                        return false;
                      }
                    type_parameters = __node_77;

                  }
                else if (true /*epsilon*/)
                {}
                else
                  {
                    return false;
                  }
                if ((yytoken == Token_IDENTIFIER) && ( LA(2).kind == Token_LPAREN ))
                  {
                    constructor_declaration_ast *__node_78 = 0;
                    if (!parse_constructor_declaration(&__node_78,
                                                       modifiers, type_parameters
                                                      ))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_constructor_declaration, "constructor_declaration");
                          }
                        return false;
                      }
                    (*yynode)->constructor_declaration = __node_78;

                  }
                else if (yytoken == Token_BOOLEAN
                         || yytoken == Token_BYTE
                         || yytoken == Token_CHAR
                         || yytoken == Token_DOUBLE
                         || yytoken == Token_FLOAT
                         || yytoken == Token_INT
                         || yytoken == Token_LONG
                         || yytoken == Token_SHORT
                         || yytoken == Token_VOID
                         || yytoken == Token_IDENTIFIER)
                  {
                    type_ast *__node_79 = 0;
                    if (!parse_type(&__node_79))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_type, "type");
                          }
                        return false;
                      }
                    type = __node_79;

                    if ((yytoken == Token_IDENTIFIER) && ( LA(2).kind == Token_LPAREN ))
                      {
                        method_declaration_ast *__node_80 = 0;
                        if (!parse_method_declaration(&__node_80,
                                                      modifiers, type_parameters, type
                                                     ))
                          {
                            if (!yy_block_errors)
                              {
                                yy_expected_symbol(ast_node::Kind_method_declaration, "method_declaration");
                              }
                            return false;
                          }
                        (*yynode)->method_declaration = __node_80;

                      }
                    else if ((yytoken == Token_IDENTIFIER) && ( type_parameters == 0 ))
                      {
                        variable_declarator_ast *__node_81 = 0;
                        if (!parse_variable_declarator(&__node_81))
                          {
                            if (!yy_block_errors)
                              {
                                yy_expected_symbol(ast_node::Kind_variable_declarator, "variable_declarator");
                              }
                            return false;
                          }
                        variable_declarator_sequence = snoc(variable_declarator_sequence, __node_81, memory_pool);

                        while (yytoken == Token_COMMA)
                          {
                            if (yytoken != Token_COMMA)
                              {
                                if (!yy_block_errors)
                                  {
                                    yy_expected_token(yytoken, Token_COMMA, ",");
                                  }
                                return false;
                              }
                            yylex();

                            variable_declarator_ast *__node_82 = 0;
                            if (!parse_variable_declarator(&__node_82))
                              {
                                if (!yy_block_errors)
                                  {
                                    yy_expected_symbol(ast_node::Kind_variable_declarator, "variable_declarator");
                                  }
                                return false;
                              }
                            variable_declarator_sequence = snoc(variable_declarator_sequence, __node_82, memory_pool);

                          }
                        if (yytoken != Token_SEMICOLON)
                          {
                            if (!yy_block_errors)
                              {
                                yy_expected_token(yytoken, Token_SEMICOLON, ";");
                              }
                            return false;
                          }
                        yylex();

                        variable_declaration_data_ast *__node_83 = 0;
                        if (!parse_variable_declaration_data(&__node_83,
                                                             modifiers, type, variable_declarator_sequence
                                                            ))
                          {
                            if (!yy_block_errors)
                              {
                                yy_expected_symbol(ast_node::Kind_variable_declaration_data, "variable_declaration_data");
                              }
                            return false;
                          }
                        (*yynode)->variable_declaration = __node_83;

                      }
                    else if (yytoken == Token_SEMICOLON)
                      {
                        report_problem( error,
                                        "Expected method declaration after type parameter list" );
                        if (yytoken != Token_SEMICOLON)
                          {
                            if (!yy_block_errors)
                              {
                                yy_expected_token(yytoken, Token_SEMICOLON, ";");
                              }
                            return false;
                          }
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
              }
            else
              {
                return false;
              }
          }
        else if (yytoken == Token_LBRACE)
          {
            block_ast *__node_84 = 0;
            if (!parse_block(&__node_84))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_block, "block");
                  }
                return false;
              }
            (*yynode)->instance_initializer_block = __node_84;

          }
        else if (yytoken == Token_STATIC)
          {
            if (yytoken != Token_STATIC)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_STATIC, "static");
                  }
                return false;
              }
            yylex();

            block_ast *__node_85 = 0;
            if (!parse_block(&__node_85))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_block, "block");
                  }
                return false;
              }
            (*yynode)->static_initializer_block = __node_85;

          }
        else if (yytoken == Token_SEMICOLON)
          {
            if (yytoken != Token_SEMICOLON)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_SEMICOLON, ";");
                  }
                return false;
              }
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

  bool parser::parse_class_or_interface_type_name(class_or_interface_type_name_ast **yynode)
  {
    *yynode = create<class_or_interface_type_name_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_IDENTIFIER)
      {
        class_or_interface_type_name_part_ast *__node_86 = 0;
        if (!parse_class_or_interface_type_name_part(&__node_86))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_class_or_interface_type_name_part, "class_or_interface_type_name_part");
              }
            return false;
          }
        (*yynode)->part_sequence = snoc((*yynode)->part_sequence, __node_86, memory_pool);

        while (yytoken == Token_DOT)
          {
            if (yytoken != Token_DOT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_DOT, ".");
                  }
                return false;
              }
            yylex();

            class_or_interface_type_name_part_ast *__node_87 = 0;
            if (!parse_class_or_interface_type_name_part(&__node_87))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_class_or_interface_type_name_part, "class_or_interface_type_name_part");
                  }
                return false;
              }
            (*yynode)->part_sequence = snoc((*yynode)->part_sequence, __node_87, memory_pool);

          }
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_class_or_interface_type_name_part(class_or_interface_type_name_part_ast **yynode)
  {
    *yynode = create<class_or_interface_type_name_part_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_IDENTIFIER)
      {
        identifier_ast *__node_88 = 0;
        if (!parse_identifier(&__node_88))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_identifier, "identifier");
              }
            return false;
          }
        (*yynode)->identifier = __node_88;

        if ((yytoken == Token_LESS_THAN) && ( compatibility_mode() >= java15_compatibility ))
          {
            type_arguments_ast *__node_89 = 0;
            if (!parse_type_arguments(&__node_89))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_type_arguments, "type_arguments");
                  }
                return false;
              }
            (*yynode)->type_arguments = __node_89;

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

  bool parser::parse_class_type(class_type_ast **yynode)
  {
    *yynode = create<class_type_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_IDENTIFIER)
      {
        class_or_interface_type_name_ast *__node_90 = 0;
        if (!parse_class_or_interface_type_name(&__node_90))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_class_or_interface_type_name, "class_or_interface_type_name");
              }
            return false;
          }
        (*yynode)->type = __node_90;

        optional_declarator_brackets_ast *__node_91 = 0;
        if (!parse_optional_declarator_brackets(&__node_91))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_optional_declarator_brackets, "optional_declarator_brackets");
              }
            return false;
          }
        (*yynode)->declarator_brackets = __node_91;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_compilation_unit(compilation_unit_ast **yynode)
  {
    *yynode = create<compilation_unit_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_ABSTRACT
        || yytoken == Token_CLASS
        || yytoken == Token_ENUM
        || yytoken == Token_FINAL
        || yytoken == Token_IMPORT
        || yytoken == Token_INTERFACE
        || yytoken == Token_NATIVE
        || yytoken == Token_PACKAGE
        || yytoken == Token_PRIVATE
        || yytoken == Token_PROTECTED
        || yytoken == Token_PUBLIC
        || yytoken == Token_STATIC
        || yytoken == Token_STRICTFP
        || yytoken == Token_SYNCHRONIZED
        || yytoken == Token_TRANSIENT
        || yytoken == Token_VOLATILE
        || yytoken == Token_SEMICOLON
        || yytoken == Token_AT || yytoken == Token_EOF)
      {
        _M_state.ltCounter = 0;
        bool block_errors_5 = block_errors(true);
        std::size_t try_start_token_5 = token_stream->index() - 1;
        parser_state *try_start_state_5 = copy_current_state();
        {
          package_declaration_ast *__node_92 = 0;
          if (!parse_package_declaration(&__node_92))
            {
              goto __catch_5;
            }
          (*yynode)->package_declaration = __node_92;

        }
        block_errors(block_errors_5);
        if (try_start_state_5)
          delete try_start_state_5;

        if (false) // the only way to enter here is using goto
          {
          __catch_5:
            if (try_start_state_5)
              {
                restore_state(try_start_state_5);
                delete try_start_state_5;
              }
            block_errors(block_errors_5);
            rewind(try_start_token_5);

          }

        while (yytoken == Token_IMPORT)
          {
            std::size_t try_start_token_6 = token_stream->index() - 1;
            parser_state *try_start_state_6 = copy_current_state();
            {
              import_declaration_ast *__node_93 = 0;
              if (!parse_import_declaration(&__node_93))
                {
                  goto __catch_6;
                }
              (*yynode)->import_declaration_sequence = snoc((*yynode)->import_declaration_sequence, __node_93, memory_pool);

            }
            if (try_start_state_6)
              delete try_start_state_6;

            if (false) // the only way to enter here is using goto
              {
              __catch_6:
                if (try_start_state_6)
                  {
                    restore_state(try_start_state_6);
                    delete try_start_state_6;
                  }
                if (try_start_token_6 == token_stream->index() - 1)
                  yylex();

                while (yytoken != Token_EOF
                       && yytoken != Token_ABSTRACT
                       && yytoken != Token_CLASS
                       && yytoken != Token_ENUM
                       && yytoken != Token_FINAL
                       && yytoken != Token_IMPORT
                       && yytoken != Token_INTERFACE
                       && yytoken != Token_NATIVE
                       && yytoken != Token_PRIVATE
                       && yytoken != Token_PROTECTED
                       && yytoken != Token_PUBLIC
                       && yytoken != Token_STATIC
                       && yytoken != Token_STRICTFP
                       && yytoken != Token_SYNCHRONIZED
                       && yytoken != Token_TRANSIENT
                       && yytoken != Token_VOLATILE
                       && yytoken != Token_SEMICOLON
                       && yytoken != Token_AT
                       && yytoken != Token_EOF)
                  {
                    yylex();
                  }
              }

          }
        while (yytoken == Token_ABSTRACT
               || yytoken == Token_CLASS
               || yytoken == Token_ENUM
               || yytoken == Token_FINAL
               || yytoken == Token_INTERFACE
               || yytoken == Token_NATIVE
               || yytoken == Token_PRIVATE
               || yytoken == Token_PROTECTED
               || yytoken == Token_PUBLIC
               || yytoken == Token_STATIC
               || yytoken == Token_STRICTFP
               || yytoken == Token_SYNCHRONIZED
               || yytoken == Token_TRANSIENT
               || yytoken == Token_VOLATILE
               || yytoken == Token_SEMICOLON
               || yytoken == Token_AT)
          {
            std::size_t try_start_token_7 = token_stream->index() - 1;
            parser_state *try_start_state_7 = copy_current_state();
            {
              type_declaration_ast *__node_94 = 0;
              if (!parse_type_declaration(&__node_94))
                {
                  goto __catch_7;
                }
              (*yynode)->type_declaration_sequence = snoc((*yynode)->type_declaration_sequence, __node_94, memory_pool);

            }
            if (try_start_state_7)
              delete try_start_state_7;

            if (false) // the only way to enter here is using goto
              {
              __catch_7:
                if (try_start_state_7)
                  {
                    restore_state(try_start_state_7);
                    delete try_start_state_7;
                  }
                if (try_start_token_7 == token_stream->index() - 1)
                  yylex();

                while (yytoken != Token_EOF
                       && yytoken != Token_ABSTRACT
                       && yytoken != Token_CLASS
                       && yytoken != Token_ENUM
                       && yytoken != Token_FINAL
                       && yytoken != Token_INTERFACE
                       && yytoken != Token_NATIVE
                       && yytoken != Token_PRIVATE
                       && yytoken != Token_PROTECTED
                       && yytoken != Token_PUBLIC
                       && yytoken != Token_STATIC
                       && yytoken != Token_STRICTFP
                       && yytoken != Token_SYNCHRONIZED
                       && yytoken != Token_TRANSIENT
                       && yytoken != Token_VOLATILE
                       && yytoken != Token_SEMICOLON
                       && yytoken != Token_AT
                       && yytoken != Token_EOF)
                  {
                    yylex();
                  }
              }

          }
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

  bool parser::parse_conditional_expression(conditional_expression_ast **yynode)
  {
    *yynode = create<conditional_expression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOOLEAN
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_SHORT
        || yytoken == Token_SUPER
        || yytoken == Token_THIS
        || yytoken == Token_VOID
        || yytoken == Token_LPAREN
        || yytoken == Token_LESS_THAN
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_FLOATING_POINT_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        logical_or_expression_ast *__node_95 = 0;
        if (!parse_logical_or_expression(&__node_95))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_logical_or_expression, "logical_or_expression");
              }
            return false;
          }
        (*yynode)->logical_or_expression = __node_95;

        if (yytoken == Token_QUESTION)
          {
            if (yytoken != Token_QUESTION)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_QUESTION, "?");
                  }
                return false;
              }
            yylex();

            expression_ast *__node_96 = 0;
            if (!parse_expression(&__node_96))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_expression, "expression");
                  }
                return false;
              }
            (*yynode)->if_expression = __node_96;

            if (yytoken != Token_COLON)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_COLON, ":");
                  }
                return false;
              }
            yylex();

            conditional_expression_ast *__node_97 = 0;
            if (!parse_conditional_expression(&__node_97))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_conditional_expression, "conditional_expression");
                  }
                return false;
              }
            (*yynode)->else_expression = __node_97;

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

  bool parser::parse_constructor_declaration(constructor_declaration_ast **yynode, optional_modifiers_ast *modifiers, type_parameters_ast *type_parameters)
  {
    *yynode = create<constructor_declaration_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    (*yynode)->modifiers = modifiers;
    if (modifiers && modifiers->start_token < (*yynode)->start_token)
      (*yynode)->start_token = modifiers->start_token;

    (*yynode)->type_parameters = type_parameters;
    if (type_parameters && type_parameters->start_token < (*yynode)->start_token)
      (*yynode)->start_token = type_parameters->start_token;

    if (yytoken == Token_IDENTIFIER)
      {
        identifier_ast *__node_98 = 0;
        if (!parse_identifier(&__node_98))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_identifier, "identifier");
              }
            return false;
          }
        (*yynode)->class_name = __node_98;

        if (yytoken != Token_LPAREN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_LPAREN, "(");
              }
            return false;
          }
        yylex();

        optional_parameter_declaration_list_ast *__node_99 = 0;
        if (!parse_optional_parameter_declaration_list(&__node_99))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_optional_parameter_declaration_list, "optional_parameter_declaration_list");
              }
            return false;
          }
        (*yynode)->parameters = __node_99;

        if (yytoken != Token_RPAREN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_RPAREN, ")");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_THROWS)
          {
            throws_clause_ast *__node_100 = 0;
            if (!parse_throws_clause(&__node_100))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_throws_clause, "throws_clause");
                  }
                return false;
              }
            (*yynode)->throws_clause = __node_100;

          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        block_ast *__node_101 = 0;
        if (!parse_block(&__node_101))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_block, "block");
              }
            return false;
          }
        (*yynode)->body = __node_101;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_continue_statement(continue_statement_ast **yynode)
  {
    *yynode = create<continue_statement_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_CONTINUE)
      {
        if (yytoken != Token_CONTINUE)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_CONTINUE, "continue");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_IDENTIFIER)
          {
            identifier_ast *__node_102 = 0;
            if (!parse_identifier(&__node_102))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                  }
                return false;
              }
            (*yynode)->label = __node_102;

          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        if (yytoken != Token_SEMICOLON)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_SEMICOLON, ";");
              }
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

  bool parser::parse_do_while_statement(do_while_statement_ast **yynode)
  {
    *yynode = create<do_while_statement_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_DO)
      {
        if (yytoken != Token_DO)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_DO, "do");
              }
            return false;
          }
        yylex();

        embedded_statement_ast *__node_103 = 0;
        if (!parse_embedded_statement(&__node_103))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_embedded_statement, "embedded_statement");
              }
            return false;
          }
        (*yynode)->body = __node_103;

        if (yytoken != Token_WHILE)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_WHILE, "while");
              }
            return false;
          }
        yylex();

        if (yytoken != Token_LPAREN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_LPAREN, "(");
              }
            return false;
          }
        yylex();

        expression_ast *__node_104 = 0;
        if (!parse_expression(&__node_104))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_expression, "expression");
              }
            return false;
          }
        (*yynode)->condition = __node_104;

        if (yytoken != Token_RPAREN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_RPAREN, ")");
              }
            return false;
          }
        yylex();

        if (yytoken != Token_SEMICOLON)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_SEMICOLON, ";");
              }
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

  bool parser::parse_embedded_statement(embedded_statement_ast **yynode)
  {
    *yynode = create<embedded_statement_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_ASSERT
        || yytoken == Token_BOOLEAN
        || yytoken == Token_BREAK
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_CONTINUE
        || yytoken == Token_DO
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_FOR
        || yytoken == Token_IF
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_RETURN
        || yytoken == Token_SHORT
        || yytoken == Token_SUPER
        || yytoken == Token_SWITCH
        || yytoken == Token_SYNCHRONIZED
        || yytoken == Token_THIS
        || yytoken == Token_THROW
        || yytoken == Token_TRY
        || yytoken == Token_VOID
        || yytoken == Token_WHILE
        || yytoken == Token_LPAREN
        || yytoken == Token_LBRACE
        || yytoken == Token_SEMICOLON
        || yytoken == Token_LESS_THAN
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_FLOATING_POINT_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        if (yytoken == Token_LBRACE)
          {
            block_ast *__node_105 = 0;
            if (!parse_block(&__node_105))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_block, "block");
                  }
                return false;
              }
            (*yynode)->block = __node_105;

          }
        else if (yytoken == Token_ASSERT)
          {
            assert_statement_ast *__node_106 = 0;
            if (!parse_assert_statement(&__node_106))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_assert_statement, "assert_statement");
                  }
                return false;
              }
            (*yynode)->assert_statement = __node_106;

          }
        else if (yytoken == Token_IF)
          {
            if_statement_ast *__node_107 = 0;
            if (!parse_if_statement(&__node_107))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_if_statement, "if_statement");
                  }
                return false;
              }
            (*yynode)->if_statement = __node_107;

          }
        else if (yytoken == Token_FOR)
          {
            for_statement_ast *__node_108 = 0;
            if (!parse_for_statement(&__node_108))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_for_statement, "for_statement");
                  }
                return false;
              }
            (*yynode)->for_statement = __node_108;

          }
        else if (yytoken == Token_WHILE)
          {
            while_statement_ast *__node_109 = 0;
            if (!parse_while_statement(&__node_109))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_while_statement, "while_statement");
                  }
                return false;
              }
            (*yynode)->while_statement = __node_109;

          }
        else if (yytoken == Token_DO)
          {
            do_while_statement_ast *__node_110 = 0;
            if (!parse_do_while_statement(&__node_110))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_do_while_statement, "do_while_statement");
                  }
                return false;
              }
            (*yynode)->do_while_statement = __node_110;

          }
        else if (yytoken == Token_TRY)
          {
            try_statement_ast *__node_111 = 0;
            if (!parse_try_statement(&__node_111))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_try_statement, "try_statement");
                  }
                return false;
              }
            (*yynode)->try_statement = __node_111;

          }
        else if (yytoken == Token_SWITCH)
          {
            switch_statement_ast *__node_112 = 0;
            if (!parse_switch_statement(&__node_112))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_switch_statement, "switch_statement");
                  }
                return false;
              }
            (*yynode)->switch_statement = __node_112;

          }
        else if (yytoken == Token_SYNCHRONIZED)
          {
            synchronized_statement_ast *__node_113 = 0;
            if (!parse_synchronized_statement(&__node_113))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_synchronized_statement, "synchronized_statement");
                  }
                return false;
              }
            (*yynode)->synchronized_statement = __node_113;

          }
        else if (yytoken == Token_RETURN)
          {
            return_statement_ast *__node_114 = 0;
            if (!parse_return_statement(&__node_114))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_return_statement, "return_statement");
                  }
                return false;
              }
            (*yynode)->return_statement = __node_114;

          }
        else if (yytoken == Token_THROW)
          {
            throw_statement_ast *__node_115 = 0;
            if (!parse_throw_statement(&__node_115))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_throw_statement, "throw_statement");
                  }
                return false;
              }
            (*yynode)->throw_statement = __node_115;

          }
        else if (yytoken == Token_BREAK)
          {
            break_statement_ast *__node_116 = 0;
            if (!parse_break_statement(&__node_116))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_break_statement, "break_statement");
                  }
                return false;
              }
            (*yynode)->break_statement = __node_116;

          }
        else if (yytoken == Token_CONTINUE)
          {
            continue_statement_ast *__node_117 = 0;
            if (!parse_continue_statement(&__node_117))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_continue_statement, "continue_statement");
                  }
                return false;
              }
            (*yynode)->continue_statement = __node_117;

          }
        else if (yytoken == Token_SEMICOLON)
          {
            if (yytoken != Token_SEMICOLON)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_SEMICOLON, ";");
                  }
                return false;
              }
            yylex();

          }
        else if ((yytoken == Token_IDENTIFIER) && ( LA(2).kind == Token_COLON ))
          {
            labeled_statement_ast *__node_118 = 0;
            if (!parse_labeled_statement(&__node_118))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_labeled_statement, "labeled_statement");
                  }
                return false;
              }
            (*yynode)->labeled_statement = __node_118;

          }
        else if (yytoken == Token_BOOLEAN
                 || yytoken == Token_BYTE
                 || yytoken == Token_CHAR
                 || yytoken == Token_DOUBLE
                 || yytoken == Token_FLOAT
                 || yytoken == Token_INT
                 || yytoken == Token_LONG
                 || yytoken == Token_NEW
                 || yytoken == Token_SHORT
                 || yytoken == Token_SUPER
                 || yytoken == Token_THIS
                 || yytoken == Token_VOID
                 || yytoken == Token_LPAREN
                 || yytoken == Token_LESS_THAN
                 || yytoken == Token_BANG
                 || yytoken == Token_TILDE
                 || yytoken == Token_INCREMENT
                 || yytoken == Token_DECREMENT
                 || yytoken == Token_PLUS
                 || yytoken == Token_MINUS
                 || yytoken == Token_TRUE
                 || yytoken == Token_FALSE
                 || yytoken == Token_NULL
                 || yytoken == Token_INTEGER_LITERAL
                 || yytoken == Token_FLOATING_POINT_LITERAL
                 || yytoken == Token_CHARACTER_LITERAL
                 || yytoken == Token_STRING_LITERAL
                 || yytoken == Token_IDENTIFIER)
          {
            statement_expression_ast *__node_119 = 0;
            if (!parse_statement_expression(&__node_119))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_statement_expression, "statement_expression");
                  }
                return false;
              }
            (*yynode)->expression_statement = __node_119;

            if (yytoken != Token_SEMICOLON)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_SEMICOLON, ";");
                  }
                return false;
              }
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

  bool parser::parse_enum_body(enum_body_ast **yynode)
  {
    *yynode = create<enum_body_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_LBRACE)
      {
        if (yytoken != Token_LBRACE)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_LBRACE, "{");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_AT
            || yytoken == Token_IDENTIFIER)
          {
            std::size_t try_start_token_8 = token_stream->index() - 1;
            parser_state *try_start_state_8 = copy_current_state();
            {
              enum_constant_ast *__node_120 = 0;
              if (!parse_enum_constant(&__node_120))
                {
                  goto __catch_8;
                }
              (*yynode)->enum_constant_sequence = snoc((*yynode)->enum_constant_sequence, __node_120, memory_pool);

            }
            if (try_start_state_8)
              delete try_start_state_8;

            if (false) // the only way to enter here is using goto
              {
              __catch_8:
                if (try_start_state_8)
                  {
                    restore_state(try_start_state_8);
                    delete try_start_state_8;
                  }
                if (try_start_token_8 == token_stream->index() - 1)
                  yylex();

                while (yytoken != Token_EOF
                       && yytoken != Token_RBRACE
                       && yytoken != Token_SEMICOLON
                       && yytoken != Token_COMMA)
                  {
                    yylex();
                  }
              }

            while (yytoken == Token_COMMA)
              {
                if ( LA(2).kind == Token_SEMICOLON
                     || LA(2).kind == Token_RBRACE )
                  {
                    break;
                  }
                if (yytoken != Token_COMMA)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_COMMA, ",");
                      }
                    return false;
                  }
                yylex();

                std::size_t try_start_token_9 = token_stream->index() - 1;
                parser_state *try_start_state_9 = copy_current_state();
                {
                  enum_constant_ast *__node_121 = 0;
                  if (!parse_enum_constant(&__node_121))
                    {
                      goto __catch_9;
                    }
                  (*yynode)->enum_constant_sequence = snoc((*yynode)->enum_constant_sequence, __node_121, memory_pool);

                }
                if (try_start_state_9)
                  delete try_start_state_9;

                if (false) // the only way to enter here is using goto
                  {
                  __catch_9:
                    if (try_start_state_9)
                      {
                        restore_state(try_start_state_9);
                        delete try_start_state_9;
                      }
                    if (try_start_token_9 == token_stream->index() - 1)
                      yylex();

                    while (yytoken != Token_EOF
                           && yytoken != Token_RBRACE
                           && yytoken != Token_SEMICOLON
                           && yytoken != Token_COMMA)
                      {
                        yylex();
                      }
                  }

              }
          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        if (yytoken == Token_COMMA)
          {
            if (yytoken != Token_COMMA)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_COMMA, ",");
                  }
                return false;
              }
            yylex();

          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        if (yytoken == Token_SEMICOLON)
          {
            if (yytoken != Token_SEMICOLON)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_SEMICOLON, ";");
                  }
                return false;
              }
            yylex();

            while (yytoken == Token_ABSTRACT
                   || yytoken == Token_BOOLEAN
                   || yytoken == Token_BYTE
                   || yytoken == Token_CHAR
                   || yytoken == Token_CLASS
                   || yytoken == Token_DOUBLE
                   || yytoken == Token_ENUM
                   || yytoken == Token_FINAL
                   || yytoken == Token_FLOAT
                   || yytoken == Token_INT
                   || yytoken == Token_INTERFACE
                   || yytoken == Token_LONG
                   || yytoken == Token_NATIVE
                   || yytoken == Token_PRIVATE
                   || yytoken == Token_PROTECTED
                   || yytoken == Token_PUBLIC
                   || yytoken == Token_SHORT
                   || yytoken == Token_STATIC
                   || yytoken == Token_STRICTFP
                   || yytoken == Token_SYNCHRONIZED
                   || yytoken == Token_TRANSIENT
                   || yytoken == Token_VOID
                   || yytoken == Token_VOLATILE
                   || yytoken == Token_LBRACE
                   || yytoken == Token_SEMICOLON
                   || yytoken == Token_AT
                   || yytoken == Token_LESS_THAN
                   || yytoken == Token_IDENTIFIER)
              {
                std::size_t try_start_token_10 = token_stream->index() - 1;
                parser_state *try_start_state_10 = copy_current_state();
                {
                  class_field_ast *__node_122 = 0;
                  if (!parse_class_field(&__node_122))
                    {
                      goto __catch_10;
                    }
                  (*yynode)->class_field_sequence = snoc((*yynode)->class_field_sequence, __node_122, memory_pool);

                }
                if (try_start_state_10)
                  delete try_start_state_10;

                if (false) // the only way to enter here is using goto
                  {
                  __catch_10:
                    if (try_start_state_10)
                      {
                        restore_state(try_start_state_10);
                        delete try_start_state_10;
                      }
                    if (try_start_token_10 == token_stream->index() - 1)
                      yylex();

                    while (yytoken != Token_EOF
                           && yytoken != Token_ABSTRACT
                           && yytoken != Token_BOOLEAN
                           && yytoken != Token_BYTE
                           && yytoken != Token_CHAR
                           && yytoken != Token_CLASS
                           && yytoken != Token_DOUBLE
                           && yytoken != Token_ENUM
                           && yytoken != Token_FINAL
                           && yytoken != Token_FLOAT
                           && yytoken != Token_INT
                           && yytoken != Token_INTERFACE
                           && yytoken != Token_LONG
                           && yytoken != Token_NATIVE
                           && yytoken != Token_PRIVATE
                           && yytoken != Token_PROTECTED
                           && yytoken != Token_PUBLIC
                           && yytoken != Token_SHORT
                           && yytoken != Token_STATIC
                           && yytoken != Token_STRICTFP
                           && yytoken != Token_SYNCHRONIZED
                           && yytoken != Token_TRANSIENT
                           && yytoken != Token_VOID
                           && yytoken != Token_VOLATILE
                           && yytoken != Token_LBRACE
                           && yytoken != Token_RBRACE
                           && yytoken != Token_SEMICOLON
                           && yytoken != Token_AT
                           && yytoken != Token_LESS_THAN
                           && yytoken != Token_IDENTIFIER)
                      {
                        yylex();
                      }
                  }

              }
          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        if (yytoken != Token_RBRACE)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_RBRACE, "}");
              }
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

  bool parser::parse_enum_constant(enum_constant_ast **yynode)
  {
    *yynode = create<enum_constant_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_AT
        || yytoken == Token_IDENTIFIER)
      {
        while (yytoken == Token_AT)
          {
            std::size_t try_start_token_11 = token_stream->index() - 1;
            parser_state *try_start_state_11 = copy_current_state();
            {
              annotation_ast *__node_123 = 0;
              if (!parse_annotation(&__node_123))
                {
                  goto __catch_11;
                }
              (*yynode)->annotation_sequence = snoc((*yynode)->annotation_sequence, __node_123, memory_pool);

            }
            if (try_start_state_11)
              delete try_start_state_11;

            if (false) // the only way to enter here is using goto
              {
              __catch_11:
                if (try_start_state_11)
                  {
                    restore_state(try_start_state_11);
                    delete try_start_state_11;
                  }
                if (try_start_token_11 == token_stream->index() - 1)
                  yylex();

                while (yytoken != Token_EOF
                       && yytoken != Token_AT
                       && yytoken != Token_IDENTIFIER)
                  {
                    yylex();
                  }
              }

          }
        identifier_ast *__node_124 = 0;
        if (!parse_identifier(&__node_124))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_identifier, "identifier");
              }
            return false;
          }
        (*yynode)->identifier = __node_124;

        if (yytoken == Token_LPAREN)
          {
            if (yytoken != Token_LPAREN)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LPAREN, "(");
                  }
                return false;
              }
            yylex();

            optional_argument_list_ast *__node_125 = 0;
            if (!parse_optional_argument_list(&__node_125))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_optional_argument_list, "optional_argument_list");
                  }
                return false;
              }
            (*yynode)->arguments = __node_125;

            if (yytoken != Token_RPAREN)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_RPAREN, ")");
                  }
                return false;
              }
            yylex();

          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        if (yytoken == Token_LBRACE)
          {
            enum_constant_body_ast *__node_126 = 0;
            if (!parse_enum_constant_body(&__node_126))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_enum_constant_body, "enum_constant_body");
                  }
                return false;
              }
            (*yynode)->body = __node_126;

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

  bool parser::parse_enum_constant_body(enum_constant_body_ast **yynode)
  {
    *yynode = create<enum_constant_body_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_LBRACE)
      {
        if (yytoken != Token_LBRACE)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_LBRACE, "{");
              }
            return false;
          }
        yylex();

        while (yytoken == Token_ABSTRACT
               || yytoken == Token_BOOLEAN
               || yytoken == Token_BYTE
               || yytoken == Token_CHAR
               || yytoken == Token_CLASS
               || yytoken == Token_DOUBLE
               || yytoken == Token_ENUM
               || yytoken == Token_FINAL
               || yytoken == Token_FLOAT
               || yytoken == Token_INT
               || yytoken == Token_INTERFACE
               || yytoken == Token_LONG
               || yytoken == Token_NATIVE
               || yytoken == Token_PRIVATE
               || yytoken == Token_PROTECTED
               || yytoken == Token_PUBLIC
               || yytoken == Token_SHORT
               || yytoken == Token_STATIC
               || yytoken == Token_STRICTFP
               || yytoken == Token_SYNCHRONIZED
               || yytoken == Token_TRANSIENT
               || yytoken == Token_VOID
               || yytoken == Token_VOLATILE
               || yytoken == Token_LBRACE
               || yytoken == Token_SEMICOLON
               || yytoken == Token_AT
               || yytoken == Token_LESS_THAN
               || yytoken == Token_IDENTIFIER)
          {
            std::size_t try_start_token_12 = token_stream->index() - 1;
            parser_state *try_start_state_12 = copy_current_state();
            {
              enum_constant_field_ast *__node_127 = 0;
              if (!parse_enum_constant_field(&__node_127))
                {
                  goto __catch_12;
                }
              (*yynode)->declaration_sequence = snoc((*yynode)->declaration_sequence, __node_127, memory_pool);

            }
            if (try_start_state_12)
              delete try_start_state_12;

            if (false) // the only way to enter here is using goto
              {
              __catch_12:
                if (try_start_state_12)
                  {
                    restore_state(try_start_state_12);
                    delete try_start_state_12;
                  }
                if (try_start_token_12 == token_stream->index() - 1)
                  yylex();

                while (yytoken != Token_EOF
                       && yytoken != Token_ABSTRACT
                       && yytoken != Token_BOOLEAN
                       && yytoken != Token_BYTE
                       && yytoken != Token_CHAR
                       && yytoken != Token_CLASS
                       && yytoken != Token_DOUBLE
                       && yytoken != Token_ENUM
                       && yytoken != Token_FINAL
                       && yytoken != Token_FLOAT
                       && yytoken != Token_INT
                       && yytoken != Token_INTERFACE
                       && yytoken != Token_LONG
                       && yytoken != Token_NATIVE
                       && yytoken != Token_PRIVATE
                       && yytoken != Token_PROTECTED
                       && yytoken != Token_PUBLIC
                       && yytoken != Token_SHORT
                       && yytoken != Token_STATIC
                       && yytoken != Token_STRICTFP
                       && yytoken != Token_SYNCHRONIZED
                       && yytoken != Token_TRANSIENT
                       && yytoken != Token_VOID
                       && yytoken != Token_VOLATILE
                       && yytoken != Token_LBRACE
                       && yytoken != Token_RBRACE
                       && yytoken != Token_SEMICOLON
                       && yytoken != Token_AT
                       && yytoken != Token_LESS_THAN
                       && yytoken != Token_IDENTIFIER)
                  {
                    yylex();
                  }
              }

          }
        if (yytoken != Token_RBRACE)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_RBRACE, "}");
              }
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

  bool parser::parse_enum_constant_field(enum_constant_field_ast **yynode)
  {
    *yynode = create<enum_constant_field_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_ABSTRACT
        || yytoken == Token_BOOLEAN
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_CLASS
        || yytoken == Token_DOUBLE
        || yytoken == Token_ENUM
        || yytoken == Token_FINAL
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_INTERFACE
        || yytoken == Token_LONG
        || yytoken == Token_NATIVE
        || yytoken == Token_PRIVATE
        || yytoken == Token_PROTECTED
        || yytoken == Token_PUBLIC
        || yytoken == Token_SHORT
        || yytoken == Token_STATIC
        || yytoken == Token_STRICTFP
        || yytoken == Token_SYNCHRONIZED
        || yytoken == Token_TRANSIENT
        || yytoken == Token_VOID
        || yytoken == Token_VOLATILE
        || yytoken == Token_LBRACE
        || yytoken == Token_SEMICOLON
        || yytoken == Token_AT
        || yytoken == Token_LESS_THAN
        || yytoken == Token_IDENTIFIER)
      {
        optional_modifiers_ast *modifiers = 0;

        type_parameters_ast *type_parameters = 0;

        type_ast *type = 0;

        const list_node<variable_declarator_ast *> *variable_declarator_sequence = 0;

        if (yytoken == Token_ABSTRACT
            || yytoken == Token_BOOLEAN
            || yytoken == Token_BYTE
            || yytoken == Token_CHAR
            || yytoken == Token_CLASS
            || yytoken == Token_DOUBLE
            || yytoken == Token_ENUM
            || yytoken == Token_FINAL
            || yytoken == Token_FLOAT
            || yytoken == Token_INT
            || yytoken == Token_INTERFACE
            || yytoken == Token_LONG
            || yytoken == Token_NATIVE
            || yytoken == Token_PRIVATE
            || yytoken == Token_PROTECTED
            || yytoken == Token_PUBLIC
            || yytoken == Token_SHORT
            || yytoken == Token_STATIC
            || yytoken == Token_STRICTFP
            || yytoken == Token_SYNCHRONIZED
            || yytoken == Token_TRANSIENT
            || yytoken == Token_VOID
            || yytoken == Token_VOLATILE
            || yytoken == Token_AT
            || yytoken == Token_LESS_THAN
            || yytoken == Token_IDENTIFIER)
          {
            optional_modifiers_ast *__node_128 = 0;
            if (!parse_optional_modifiers(&__node_128))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_optional_modifiers, "optional_modifiers");
                  }
                return false;
              }
            modifiers = __node_128;

            if (yytoken == Token_CLASS)
              {
                class_declaration_ast *__node_129 = 0;
                if (!parse_class_declaration(&__node_129, modifiers))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_class_declaration, "class_declaration");
                      }
                    return false;
                  }
                (*yynode)->class_declaration = __node_129;

              }
            else if (yytoken == Token_ENUM)
              {
                enum_declaration_ast *__node_130 = 0;
                if (!parse_enum_declaration(&__node_130, modifiers))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_enum_declaration, "enum_declaration");
                      }
                    return false;
                  }
                (*yynode)->enum_declaration = __node_130;

              }
            else if (yytoken == Token_INTERFACE)
              {
                interface_declaration_ast *__node_131 = 0;
                if (!parse_interface_declaration(&__node_131, modifiers))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_interface_declaration, "interface_declaration");
                      }
                    return false;
                  }
                (*yynode)->interface_declaration = __node_131;

              }
            else if (yytoken == Token_AT)
              {
                annotation_type_declaration_ast *__node_132 = 0;
                if (!parse_annotation_type_declaration(&__node_132, modifiers))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_annotation_type_declaration, "annotation_type_declaration");
                      }
                    return false;
                  }
                (*yynode)->annotation_type_declaration = __node_132;

              }
            else if (yytoken == Token_BOOLEAN
                     || yytoken == Token_BYTE
                     || yytoken == Token_CHAR
                     || yytoken == Token_DOUBLE
                     || yytoken == Token_FLOAT
                     || yytoken == Token_INT
                     || yytoken == Token_LONG
                     || yytoken == Token_SHORT
                     || yytoken == Token_VOID
                     || yytoken == Token_LESS_THAN
                     || yytoken == Token_IDENTIFIER)
              {
                if ((yytoken == Token_LESS_THAN) && ( compatibility_mode() >= java15_compatibility ))
                  {
                    type_parameters_ast *__node_133 = 0;
                    if (!parse_type_parameters(&__node_133))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_type_parameters, "type_parameters");
                          }
                        return false;
                      }
                    type_parameters = __node_133;

                  }
                else if (true /*epsilon*/)
                {}
                else
                  {
                    return false;
                  }
                type_ast *__node_134 = 0;
                if (!parse_type(&__node_134))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_type, "type");
                      }
                    return false;
                  }
                type = __node_134;

                if ((yytoken == Token_IDENTIFIER) && ( LA(2).kind == Token_LPAREN ))
                  {
                    method_declaration_ast *__node_135 = 0;
                    if (!parse_method_declaration(&__node_135,
                                                  modifiers, type_parameters, type
                                                 ))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_method_declaration, "method_declaration");
                          }
                        return false;
                      }
                    (*yynode)->method_declaration = __node_135;

                  }
                else if ((yytoken == Token_IDENTIFIER) && ( type_parameters == 0 ))
                  {
                    variable_declarator_ast *__node_136 = 0;
                    if (!parse_variable_declarator(&__node_136))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_variable_declarator, "variable_declarator");
                          }
                        return false;
                      }
                    variable_declarator_sequence = snoc(variable_declarator_sequence, __node_136, memory_pool);

                    while (yytoken == Token_COMMA)
                      {
                        if (yytoken != Token_COMMA)
                          {
                            if (!yy_block_errors)
                              {
                                yy_expected_token(yytoken, Token_COMMA, ",");
                              }
                            return false;
                          }
                        yylex();

                        variable_declarator_ast *__node_137 = 0;
                        if (!parse_variable_declarator(&__node_137))
                          {
                            if (!yy_block_errors)
                              {
                                yy_expected_symbol(ast_node::Kind_variable_declarator, "variable_declarator");
                              }
                            return false;
                          }
                        variable_declarator_sequence = snoc(variable_declarator_sequence, __node_137, memory_pool);

                      }
                    if (yytoken != Token_SEMICOLON)
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_token(yytoken, Token_SEMICOLON, ";");
                          }
                        return false;
                      }
                    yylex();

                    variable_declaration_data_ast *__node_138 = 0;
                    if (!parse_variable_declaration_data(&__node_138,
                                                         modifiers, type, variable_declarator_sequence
                                                        ))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_variable_declaration_data, "variable_declaration_data");
                          }
                        return false;
                      }
                    (*yynode)->variable_declaration = __node_138;

                  }
                else if (yytoken == Token_SEMICOLON)
                  {
                    report_problem( error,
                                    "Expected method declaration after type parameter list" );
                    if (yytoken != Token_SEMICOLON)
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_token(yytoken, Token_SEMICOLON, ";");
                          }
                        return false;
                      }
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
          }
        else if (yytoken == Token_LBRACE)
          {
            block_ast *__node_139 = 0;
            if (!parse_block(&__node_139))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_block, "block");
                  }
                return false;
              }
            (*yynode)->instance_initializer_block = __node_139;

          }
        else if (yytoken == Token_SEMICOLON)
          {
            if (yytoken != Token_SEMICOLON)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_SEMICOLON, ";");
                  }
                return false;
              }
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

  bool parser::parse_enum_declaration(enum_declaration_ast **yynode, optional_modifiers_ast *modifiers)
  {
    *yynode = create<enum_declaration_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    (*yynode)->modifiers = modifiers;
    if (modifiers && modifiers->start_token < (*yynode)->start_token)
      (*yynode)->start_token = modifiers->start_token;

    if (yytoken == Token_ENUM)
      {
        if (yytoken != Token_ENUM)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_ENUM, "enum");
              }
            return false;
          }
        yylex();

        identifier_ast *__node_140 = 0;
        if (!parse_identifier(&__node_140))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_identifier, "identifier");
              }
            return false;
          }
        (*yynode)->enum_name = __node_140;

        if (yytoken == Token_IMPLEMENTS)
          {
            implements_clause_ast *__node_141 = 0;
            if (!parse_implements_clause(&__node_141))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_implements_clause, "implements_clause");
                  }
                return false;
              }
            (*yynode)->implements = __node_141;

          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        enum_body_ast *__node_142 = 0;
        if (!parse_enum_body(&__node_142))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_enum_body, "enum_body");
              }
            return false;
          }
        (*yynode)->body = __node_142;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_equality_expression(equality_expression_ast **yynode)
  {
    *yynode = create<equality_expression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOOLEAN
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_SHORT
        || yytoken == Token_SUPER
        || yytoken == Token_THIS
        || yytoken == Token_VOID
        || yytoken == Token_LPAREN
        || yytoken == Token_LESS_THAN
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_FLOATING_POINT_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        relational_expression_ast *__node_143 = 0;
        if (!parse_relational_expression(&__node_143))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_relational_expression, "relational_expression");
              }
            return false;
          }
        (*yynode)->expression = __node_143;

        while (yytoken == Token_EQUAL
               || yytoken == Token_NOT_EQUAL)
          {
            equality_expression_rest_ast *__node_144 = 0;
            if (!parse_equality_expression_rest(&__node_144))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_equality_expression_rest, "equality_expression_rest");
                  }
                return false;
              }
            (*yynode)->additional_expression_sequence = snoc((*yynode)->additional_expression_sequence, __node_144, memory_pool);

          }
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_equality_expression_rest(equality_expression_rest_ast **yynode)
  {
    *yynode = create<equality_expression_rest_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_EQUAL
        || yytoken == Token_NOT_EQUAL)
      {
        if (yytoken == Token_EQUAL)
          {
            if (yytoken != Token_EQUAL)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_EQUAL, "==");
                  }
                return false;
              }
            yylex();

            (*yynode)->equality_operator = equality_expression_rest::op_equal;
          }
        else if (yytoken == Token_NOT_EQUAL)
          {
            if (yytoken != Token_NOT_EQUAL)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_NOT_EQUAL, "!=");
                  }
                return false;
              }
            yylex();

            (*yynode)->equality_operator = equality_expression_rest::op_not_equal;
          }
        else
          {
            return false;
          }
        relational_expression_ast *__node_145 = 0;
        if (!parse_relational_expression(&__node_145))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_relational_expression, "relational_expression");
              }
            return false;
          }
        (*yynode)->expression = __node_145;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_expression(expression_ast **yynode)
  {
    *yynode = create<expression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOOLEAN
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_SHORT
        || yytoken == Token_SUPER
        || yytoken == Token_THIS
        || yytoken == Token_VOID
        || yytoken == Token_LPAREN
        || yytoken == Token_LESS_THAN
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_FLOATING_POINT_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        conditional_expression_ast *__node_146 = 0;
        if (!parse_conditional_expression(&__node_146))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_conditional_expression, "conditional_expression");
              }
            return false;
          }
        (*yynode)->conditional_expression = __node_146;

        if (yytoken == Token_ASSIGN
            || yytoken == Token_PLUS_ASSIGN
            || yytoken == Token_MINUS_ASSIGN
            || yytoken == Token_STAR_ASSIGN
            || yytoken == Token_SLASH_ASSIGN
            || yytoken == Token_BIT_AND_ASSIGN
            || yytoken == Token_BIT_OR_ASSIGN
            || yytoken == Token_BIT_XOR_ASSIGN
            || yytoken == Token_REMAINDER_ASSIGN
            || yytoken == Token_LSHIFT_ASSIGN
            || yytoken == Token_SIGNED_RSHIFT_ASSIGN
            || yytoken == Token_UNSIGNED_RSHIFT_ASSIGN)
          {
            if (yytoken == Token_ASSIGN)
              {
                if (yytoken != Token_ASSIGN)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_ASSIGN, "=");
                      }
                    return false;
                  }
                yylex();

                (*yynode)->assignment_operator = expression::op_assign;
              }
            else if (yytoken == Token_PLUS_ASSIGN)
              {
                if (yytoken != Token_PLUS_ASSIGN)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_PLUS_ASSIGN, "+=");
                      }
                    return false;
                  }
                yylex();

                (*yynode)->assignment_operator = expression::op_plus_assign;
              }
            else if (yytoken == Token_MINUS_ASSIGN)
              {
                if (yytoken != Token_MINUS_ASSIGN)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_MINUS_ASSIGN, "-=");
                      }
                    return false;
                  }
                yylex();

                (*yynode)->assignment_operator = expression::op_minus_assign;
              }
            else if (yytoken == Token_STAR_ASSIGN)
              {
                if (yytoken != Token_STAR_ASSIGN)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_STAR_ASSIGN, "*=");
                      }
                    return false;
                  }
                yylex();

                (*yynode)->assignment_operator = expression::op_star_assign;
              }
            else if (yytoken == Token_SLASH_ASSIGN)
              {
                if (yytoken != Token_SLASH_ASSIGN)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_SLASH_ASSIGN, "/=");
                      }
                    return false;
                  }
                yylex();

                (*yynode)->assignment_operator = expression::op_slash_assign;
              }
            else if (yytoken == Token_BIT_AND_ASSIGN)
              {
                if (yytoken != Token_BIT_AND_ASSIGN)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_BIT_AND_ASSIGN, "&=");
                      }
                    return false;
                  }
                yylex();

                (*yynode)->assignment_operator = expression::op_bit_and_assign;
              }
            else if (yytoken == Token_BIT_OR_ASSIGN)
              {
                if (yytoken != Token_BIT_OR_ASSIGN)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_BIT_OR_ASSIGN, "|=");
                      }
                    return false;
                  }
                yylex();

                (*yynode)->assignment_operator = expression::op_bit_or_assign;
              }
            else if (yytoken == Token_BIT_XOR_ASSIGN)
              {
                if (yytoken != Token_BIT_XOR_ASSIGN)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_BIT_XOR_ASSIGN, "^=");
                      }
                    return false;
                  }
                yylex();

                (*yynode)->assignment_operator = expression::op_bit_xor_assign;
              }
            else if (yytoken == Token_REMAINDER_ASSIGN)
              {
                if (yytoken != Token_REMAINDER_ASSIGN)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_REMAINDER_ASSIGN, "%=");
                      }
                    return false;
                  }
                yylex();

                (*yynode)->assignment_operator = expression::op_remainder_assign;
              }
            else if (yytoken == Token_LSHIFT_ASSIGN)
              {
                if (yytoken != Token_LSHIFT_ASSIGN)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_LSHIFT_ASSIGN, "<<=");
                      }
                    return false;
                  }
                yylex();

                (*yynode)->assignment_operator = expression::op_lshift_assign;
              }
            else if (yytoken == Token_SIGNED_RSHIFT_ASSIGN)
              {
                if (yytoken != Token_SIGNED_RSHIFT_ASSIGN)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_SIGNED_RSHIFT_ASSIGN, ">>=");
                      }
                    return false;
                  }
                yylex();

                (*yynode)->assignment_operator = expression::op_signed_rshift_assign;
              }
            else if (yytoken == Token_UNSIGNED_RSHIFT_ASSIGN)
              {
                if (yytoken != Token_UNSIGNED_RSHIFT_ASSIGN)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_UNSIGNED_RSHIFT_ASSIGN, ">>>=");
                      }
                    return false;
                  }
                yylex();

                (*yynode)->assignment_operator = expression::op_unsigned_rshift_assign;
              }
            else
              {
                return false;
              }
            expression_ast *__node_147 = 0;
            if (!parse_expression(&__node_147))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_expression, "expression");
                  }
                return false;
              }
            (*yynode)->assignment_expression = __node_147;

          }
        else if (true /*epsilon*/)
          {
            (*yynode)->assignment_operator = expression::no_assignment;
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

  bool parser::parse_for_clause_traditional_rest(for_clause_traditional_rest_ast **yynode)
  {
    *yynode = create<for_clause_traditional_rest_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_SEMICOLON)
      {
        if (yytoken != Token_SEMICOLON)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_SEMICOLON, ";");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_BOOLEAN
            || yytoken == Token_BYTE
            || yytoken == Token_CHAR
            || yytoken == Token_DOUBLE
            || yytoken == Token_FLOAT
            || yytoken == Token_INT
            || yytoken == Token_LONG
            || yytoken == Token_NEW
            || yytoken == Token_SHORT
            || yytoken == Token_SUPER
            || yytoken == Token_THIS
            || yytoken == Token_VOID
            || yytoken == Token_LPAREN
            || yytoken == Token_LESS_THAN
            || yytoken == Token_BANG
            || yytoken == Token_TILDE
            || yytoken == Token_INCREMENT
            || yytoken == Token_DECREMENT
            || yytoken == Token_PLUS
            || yytoken == Token_MINUS
            || yytoken == Token_TRUE
            || yytoken == Token_FALSE
            || yytoken == Token_NULL
            || yytoken == Token_INTEGER_LITERAL
            || yytoken == Token_FLOATING_POINT_LITERAL
            || yytoken == Token_CHARACTER_LITERAL
            || yytoken == Token_STRING_LITERAL
            || yytoken == Token_IDENTIFIER)
          {
            expression_ast *__node_148 = 0;
            if (!parse_expression(&__node_148))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_expression, "expression");
                  }
                return false;
              }
            (*yynode)->for_condition = __node_148;

          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        if (yytoken != Token_SEMICOLON)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_SEMICOLON, ";");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_BOOLEAN
            || yytoken == Token_BYTE
            || yytoken == Token_CHAR
            || yytoken == Token_DOUBLE
            || yytoken == Token_FLOAT
            || yytoken == Token_INT
            || yytoken == Token_LONG
            || yytoken == Token_NEW
            || yytoken == Token_SHORT
            || yytoken == Token_SUPER
            || yytoken == Token_THIS
            || yytoken == Token_VOID
            || yytoken == Token_LPAREN
            || yytoken == Token_LESS_THAN
            || yytoken == Token_BANG
            || yytoken == Token_TILDE
            || yytoken == Token_INCREMENT
            || yytoken == Token_DECREMENT
            || yytoken == Token_PLUS
            || yytoken == Token_MINUS
            || yytoken == Token_TRUE
            || yytoken == Token_FALSE
            || yytoken == Token_NULL
            || yytoken == Token_INTEGER_LITERAL
            || yytoken == Token_FLOATING_POINT_LITERAL
            || yytoken == Token_CHARACTER_LITERAL
            || yytoken == Token_STRING_LITERAL
            || yytoken == Token_IDENTIFIER)
          {
            statement_expression_ast *__node_149 = 0;
            if (!parse_statement_expression(&__node_149))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_statement_expression, "statement_expression");
                  }
                return false;
              }
            (*yynode)->for_update_expression_sequence = snoc((*yynode)->for_update_expression_sequence, __node_149, memory_pool);

            while (yytoken == Token_COMMA)
              {
                if (yytoken != Token_COMMA)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_COMMA, ",");
                      }
                    return false;
                  }
                yylex();

                statement_expression_ast *__node_150 = 0;
                if (!parse_statement_expression(&__node_150))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_statement_expression, "statement_expression");
                      }
                    return false;
                  }
                (*yynode)->for_update_expression_sequence = snoc((*yynode)->for_update_expression_sequence, __node_150, memory_pool);

              }
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

  bool parser::parse_for_control(for_control_ast **yynode)
  {
    *yynode = create<for_control_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_ABSTRACT
        || yytoken == Token_BOOLEAN
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DOUBLE
        || yytoken == Token_FINAL
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NATIVE
        || yytoken == Token_NEW
        || yytoken == Token_PRIVATE
        || yytoken == Token_PROTECTED
        || yytoken == Token_PUBLIC
        || yytoken == Token_SHORT
        || yytoken == Token_STATIC
        || yytoken == Token_STRICTFP
        || yytoken == Token_SUPER
        || yytoken == Token_SYNCHRONIZED
        || yytoken == Token_THIS
        || yytoken == Token_TRANSIENT
        || yytoken == Token_VOID
        || yytoken == Token_VOLATILE
        || yytoken == Token_LPAREN
        || yytoken == Token_SEMICOLON
        || yytoken == Token_AT
        || yytoken == Token_LESS_THAN
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_FLOATING_POINT_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        parameter_declaration_ast *vardecl_start_or_foreach_parameter = 0;

        expression_ast *iterable_expression = 0;

        variable_declaration_rest_ast *variable_declaration_rest = 0;

        if (yytoken == Token_ABSTRACT
            || yytoken == Token_BOOLEAN
            || yytoken == Token_BYTE
            || yytoken == Token_CHAR
            || yytoken == Token_DOUBLE
            || yytoken == Token_FINAL
            || yytoken == Token_FLOAT
            || yytoken == Token_INT
            || yytoken == Token_LONG
            || yytoken == Token_NATIVE
            || yytoken == Token_NEW
            || yytoken == Token_PRIVATE
            || yytoken == Token_PROTECTED
            || yytoken == Token_PUBLIC
            || yytoken == Token_SHORT
            || yytoken == Token_STATIC
            || yytoken == Token_STRICTFP
            || yytoken == Token_SUPER
            || yytoken == Token_SYNCHRONIZED
            || yytoken == Token_THIS
            || yytoken == Token_TRANSIENT
            || yytoken == Token_VOID
            || yytoken == Token_VOLATILE
            || yytoken == Token_LPAREN
            || yytoken == Token_AT
            || yytoken == Token_LESS_THAN
            || yytoken == Token_BANG
            || yytoken == Token_TILDE
            || yytoken == Token_INCREMENT
            || yytoken == Token_DECREMENT
            || yytoken == Token_PLUS
            || yytoken == Token_MINUS
            || yytoken == Token_TRUE
            || yytoken == Token_FALSE
            || yytoken == Token_NULL
            || yytoken == Token_INTEGER_LITERAL
            || yytoken == Token_FLOATING_POINT_LITERAL
            || yytoken == Token_CHARACTER_LITERAL
            || yytoken == Token_STRING_LITERAL
            || yytoken == Token_IDENTIFIER)
          {
            bool block_errors_13 = block_errors(true);
            std::size_t try_start_token_13 = token_stream->index() - 1;
            parser_state *try_start_state_13 = copy_current_state();
            {
              parameter_declaration_ast *__node_151 = 0;
              if (!parse_parameter_declaration(&__node_151))
                {
                  goto __catch_13;
                }
              vardecl_start_or_foreach_parameter = __node_151;

              if ((yytoken == Token_COLON) && ( compatibility_mode() >= java15_compatibility ))
                {
                  if (yytoken != Token_COLON)
                    goto __catch_13;
                  yylex();

                  expression_ast *__node_152 = 0;
                  if (!parse_expression(&__node_152))
                    {
                      goto __catch_13;
                    }
                  iterable_expression = __node_152;

                  foreach_declaration_data_ast *__node_153 = 0;
                  if (!parse_foreach_declaration_data(&__node_153,
                                                      vardecl_start_or_foreach_parameter, iterable_expression
                                                     ))
                    {
                      goto __catch_13;
                    }
                  (*yynode)->foreach_declaration = __node_153;

                }
              else if (yytoken == Token_SEMICOLON
                       || yytoken == Token_COMMA
                       || yytoken == Token_ASSIGN)
                {
                  variable_declaration_rest_ast *__node_154 = 0;
                  if (!parse_variable_declaration_rest(&__node_154))
                    {
                      goto __catch_13;
                    }
                  variable_declaration_rest = __node_154;

                  variable_declaration_split_data_ast *__node_155 = 0;
                  if (!parse_variable_declaration_split_data(&__node_155,
                      vardecl_start_or_foreach_parameter, variable_declaration_rest
                                                            ))
                    {
                      goto __catch_13;
                    }
                  (*yynode)->variable_declaration = __node_155;

                  for_clause_traditional_rest_ast *__node_156 = 0;
                  if (!parse_for_clause_traditional_rest(&__node_156))
                    {
                      goto __catch_13;
                    }
                  (*yynode)->traditional_for_rest = __node_156;

                }
              else
                {
                  goto __catch_13;
                }
            }
            block_errors(block_errors_13);
            if (try_start_state_13)
              delete try_start_state_13;

            if (false) // the only way to enter here is using goto
              {
              __catch_13:
                if (try_start_state_13)
                  {
                    restore_state(try_start_state_13);
                    delete try_start_state_13;
                  }
                block_errors(block_errors_13);
                rewind(try_start_token_13);

                statement_expression_ast *__node_157 = 0;
                if (!parse_statement_expression(&__node_157))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_statement_expression, "statement_expression");
                      }
                    return false;
                  }
                (*yynode)->statement_expression_sequence = snoc((*yynode)->statement_expression_sequence, __node_157, memory_pool);

                while (yytoken == Token_COMMA)
                  {
                    if (yytoken != Token_COMMA)
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_token(yytoken, Token_COMMA, ",");
                          }
                        return false;
                      }
                    yylex();

                    statement_expression_ast *__node_158 = 0;
                    if (!parse_statement_expression(&__node_158))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_statement_expression, "statement_expression");
                          }
                        return false;
                      }
                    (*yynode)->statement_expression_sequence = snoc((*yynode)->statement_expression_sequence, __node_158, memory_pool);

                  }
                for_clause_traditional_rest_ast *__node_159 = 0;
                if (!parse_for_clause_traditional_rest(&__node_159))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_for_clause_traditional_rest, "for_clause_traditional_rest");
                      }
                    return false;
                  }
                (*yynode)->traditional_for_rest = __node_159;

              }

          }
        else if (yytoken == Token_SEMICOLON)
          {
            for_clause_traditional_rest_ast *__node_160 = 0;
            if (!parse_for_clause_traditional_rest(&__node_160))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_for_clause_traditional_rest, "for_clause_traditional_rest");
                  }
                return false;
              }
            (*yynode)->traditional_for_rest = __node_160;

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

  bool parser::parse_for_statement(for_statement_ast **yynode)
  {
    *yynode = create<for_statement_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_FOR)
      {
        if (yytoken != Token_FOR)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_FOR, "for");
              }
            return false;
          }
        yylex();

        if (yytoken != Token_LPAREN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_LPAREN, "(");
              }
            return false;
          }
        yylex();

        for_control_ast *__node_161 = 0;
        if (!parse_for_control(&__node_161))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_for_control, "for_control");
              }
            return false;
          }
        (*yynode)->for_control = __node_161;

        if (yytoken != Token_RPAREN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_RPAREN, ")");
              }
            return false;
          }
        yylex();

        embedded_statement_ast *__node_162 = 0;
        if (!parse_embedded_statement(&__node_162))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_embedded_statement, "embedded_statement");
              }
            return false;
          }
        (*yynode)->for_body = __node_162;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_foreach_declaration_data(foreach_declaration_data_ast **yynode, parameter_declaration_ast *foreach_parameter, expression_ast *iterable_expression)
  {
    *yynode = create<foreach_declaration_data_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    (*yynode)->foreach_parameter = foreach_parameter;
    if (foreach_parameter && foreach_parameter->start_token < (*yynode)->start_token)
      (*yynode)->start_token = foreach_parameter->start_token;

    (*yynode)->iterable_expression = iterable_expression;
    if (iterable_expression && iterable_expression->start_token < (*yynode)->start_token)
      (*yynode)->start_token = iterable_expression->start_token;

    if (true /*epsilon*/ || yytoken == Token_RPAREN)
    {}
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_identifier(identifier_ast **yynode)
  {
    *yynode = create<identifier_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_IDENTIFIER)
      {
        if (yytoken != Token_IDENTIFIER)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_IDENTIFIER, "identifier");
              }
            return false;
          }
        (*yynode)->ident = token_stream->index() - 1;
        yylex();

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_if_statement(if_statement_ast **yynode)
  {
    *yynode = create<if_statement_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_IF)
      {
        if (yytoken != Token_IF)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_IF, "if");
              }
            return false;
          }
        yylex();

        if (yytoken != Token_LPAREN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_LPAREN, "(");
              }
            return false;
          }
        yylex();

        expression_ast *__node_163 = 0;
        if (!parse_expression(&__node_163))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_expression, "expression");
              }
            return false;
          }
        (*yynode)->condition = __node_163;

        if (yytoken != Token_RPAREN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_RPAREN, ")");
              }
            return false;
          }
        yylex();

        embedded_statement_ast *__node_164 = 0;
        if (!parse_embedded_statement(&__node_164))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_embedded_statement, "embedded_statement");
              }
            return false;
          }
        (*yynode)->if_body = __node_164;

        if (yytoken == Token_ELSE)
          {
            if (yytoken != Token_ELSE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_ELSE, "else");
                  }
                return false;
              }
            yylex();

            embedded_statement_ast *__node_165 = 0;
            if (!parse_embedded_statement(&__node_165))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_embedded_statement, "embedded_statement");
                  }
                return false;
              }
            (*yynode)->else_body = __node_165;

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

  bool parser::parse_implements_clause(implements_clause_ast **yynode)
  {
    *yynode = create<implements_clause_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_IMPLEMENTS)
      {
        if (yytoken != Token_IMPLEMENTS)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_IMPLEMENTS, "implements");
              }
            return false;
          }
        yylex();

        class_or_interface_type_name_ast *__node_166 = 0;
        if (!parse_class_or_interface_type_name(&__node_166))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_class_or_interface_type_name, "class_or_interface_type_name");
              }
            return false;
          }
        (*yynode)->type_sequence = snoc((*yynode)->type_sequence, __node_166, memory_pool);

        while (yytoken == Token_COMMA)
          {
            if (yytoken != Token_COMMA)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_COMMA, ",");
                  }
                return false;
              }
            yylex();

            class_or_interface_type_name_ast *__node_167 = 0;
            if (!parse_class_or_interface_type_name(&__node_167))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_class_or_interface_type_name, "class_or_interface_type_name");
                  }
                return false;
              }
            (*yynode)->type_sequence = snoc((*yynode)->type_sequence, __node_167, memory_pool);

          }
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_import_declaration(import_declaration_ast **yynode)
  {
    *yynode = create<import_declaration_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_IMPORT)
      {
        if (yytoken != Token_IMPORT)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_IMPORT, "import");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_STATIC)
          {
            if (yytoken != Token_STATIC)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_STATIC, "static");
                  }
                return false;
              }
            yylex();

            (*yynode)->static_import = true;
          }
        else if (true /*epsilon*/)
          {
            (*yynode)->static_import = false;
          }
        else
          {
            return false;
          }
        qualified_identifier_with_optional_star_ast *__node_168 = 0;
        if (!parse_qualified_identifier_with_optional_star(&__node_168))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_qualified_identifier_with_optional_star, "qualified_identifier_with_optional_star");
              }
            return false;
          }
        (*yynode)->identifier_name = __node_168;

        if (yytoken != Token_SEMICOLON)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_SEMICOLON, ";");
              }
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

  bool parser::parse_interface_body(interface_body_ast **yynode)
  {
    *yynode = create<interface_body_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_LBRACE)
      {
        if (yytoken != Token_LBRACE)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_LBRACE, "{");
              }
            return false;
          }
        yylex();

        while (yytoken == Token_ABSTRACT
               || yytoken == Token_BOOLEAN
               || yytoken == Token_BYTE
               || yytoken == Token_CHAR
               || yytoken == Token_CLASS
               || yytoken == Token_DOUBLE
               || yytoken == Token_ENUM
               || yytoken == Token_FINAL
               || yytoken == Token_FLOAT
               || yytoken == Token_INT
               || yytoken == Token_INTERFACE
               || yytoken == Token_LONG
               || yytoken == Token_NATIVE
               || yytoken == Token_PRIVATE
               || yytoken == Token_PROTECTED
               || yytoken == Token_PUBLIC
               || yytoken == Token_SHORT
               || yytoken == Token_STATIC
               || yytoken == Token_STRICTFP
               || yytoken == Token_SYNCHRONIZED
               || yytoken == Token_TRANSIENT
               || yytoken == Token_VOID
               || yytoken == Token_VOLATILE
               || yytoken == Token_SEMICOLON
               || yytoken == Token_AT
               || yytoken == Token_LESS_THAN
               || yytoken == Token_IDENTIFIER)
          {
            std::size_t try_start_token_14 = token_stream->index() - 1;
            parser_state *try_start_state_14 = copy_current_state();
            {
              interface_field_ast *__node_169 = 0;
              if (!parse_interface_field(&__node_169))
                {
                  goto __catch_14;
                }
              (*yynode)->declaration_sequence = snoc((*yynode)->declaration_sequence, __node_169, memory_pool);

            }
            if (try_start_state_14)
              delete try_start_state_14;

            if (false) // the only way to enter here is using goto
              {
              __catch_14:
                if (try_start_state_14)
                  {
                    restore_state(try_start_state_14);
                    delete try_start_state_14;
                  }
                if (try_start_token_14 == token_stream->index() - 1)
                  yylex();

                while (yytoken != Token_EOF
                       && yytoken != Token_ABSTRACT
                       && yytoken != Token_BOOLEAN
                       && yytoken != Token_BYTE
                       && yytoken != Token_CHAR
                       && yytoken != Token_CLASS
                       && yytoken != Token_DOUBLE
                       && yytoken != Token_ENUM
                       && yytoken != Token_FINAL
                       && yytoken != Token_FLOAT
                       && yytoken != Token_INT
                       && yytoken != Token_INTERFACE
                       && yytoken != Token_LONG
                       && yytoken != Token_NATIVE
                       && yytoken != Token_PRIVATE
                       && yytoken != Token_PROTECTED
                       && yytoken != Token_PUBLIC
                       && yytoken != Token_SHORT
                       && yytoken != Token_STATIC
                       && yytoken != Token_STRICTFP
                       && yytoken != Token_SYNCHRONIZED
                       && yytoken != Token_TRANSIENT
                       && yytoken != Token_VOID
                       && yytoken != Token_VOLATILE
                       && yytoken != Token_RBRACE
                       && yytoken != Token_SEMICOLON
                       && yytoken != Token_AT
                       && yytoken != Token_LESS_THAN
                       && yytoken != Token_IDENTIFIER)
                  {
                    yylex();
                  }
              }

          }
        if (yytoken != Token_RBRACE)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_RBRACE, "}");
              }
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

  bool parser::parse_interface_declaration(interface_declaration_ast **yynode, optional_modifiers_ast *modifiers)
  {
    *yynode = create<interface_declaration_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    (*yynode)->modifiers = modifiers;
    if (modifiers && modifiers->start_token < (*yynode)->start_token)
      (*yynode)->start_token = modifiers->start_token;

    if (yytoken == Token_INTERFACE)
      {
        if (yytoken != Token_INTERFACE)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_INTERFACE, "interface");
              }
            return false;
          }
        yylex();

        identifier_ast *__node_170 = 0;
        if (!parse_identifier(&__node_170))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_identifier, "identifier");
              }
            return false;
          }
        (*yynode)->interface_name = __node_170;

        if ((yytoken == Token_LESS_THAN) && ( compatibility_mode() >= java15_compatibility ))
          {
            type_parameters_ast *__node_171 = 0;
            if (!parse_type_parameters(&__node_171))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_type_parameters, "type_parameters");
                  }
                return false;
              }
            (*yynode)->type_parameters = __node_171;

          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        if (yytoken == Token_EXTENDS)
          {
            interface_extends_clause_ast *__node_172 = 0;
            if (!parse_interface_extends_clause(&__node_172))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_interface_extends_clause, "interface_extends_clause");
                  }
                return false;
              }
            (*yynode)->extends = __node_172;

          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        interface_body_ast *__node_173 = 0;
        if (!parse_interface_body(&__node_173))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_interface_body, "interface_body");
              }
            return false;
          }
        (*yynode)->body = __node_173;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_interface_extends_clause(interface_extends_clause_ast **yynode)
  {
    *yynode = create<interface_extends_clause_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_EXTENDS)
      {
        if (yytoken != Token_EXTENDS)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_EXTENDS, "extends");
              }
            return false;
          }
        yylex();

        class_or_interface_type_name_ast *__node_174 = 0;
        if (!parse_class_or_interface_type_name(&__node_174))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_class_or_interface_type_name, "class_or_interface_type_name");
              }
            return false;
          }
        (*yynode)->type_sequence = snoc((*yynode)->type_sequence, __node_174, memory_pool);

        while (yytoken == Token_COMMA)
          {
            if (yytoken != Token_COMMA)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_COMMA, ",");
                  }
                return false;
              }
            yylex();

            class_or_interface_type_name_ast *__node_175 = 0;
            if (!parse_class_or_interface_type_name(&__node_175))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_class_or_interface_type_name, "class_or_interface_type_name");
                  }
                return false;
              }
            (*yynode)->type_sequence = snoc((*yynode)->type_sequence, __node_175, memory_pool);

          }
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_interface_field(interface_field_ast **yynode)
  {
    *yynode = create<interface_field_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_ABSTRACT
        || yytoken == Token_BOOLEAN
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_CLASS
        || yytoken == Token_DOUBLE
        || yytoken == Token_ENUM
        || yytoken == Token_FINAL
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_INTERFACE
        || yytoken == Token_LONG
        || yytoken == Token_NATIVE
        || yytoken == Token_PRIVATE
        || yytoken == Token_PROTECTED
        || yytoken == Token_PUBLIC
        || yytoken == Token_SHORT
        || yytoken == Token_STATIC
        || yytoken == Token_STRICTFP
        || yytoken == Token_SYNCHRONIZED
        || yytoken == Token_TRANSIENT
        || yytoken == Token_VOID
        || yytoken == Token_VOLATILE
        || yytoken == Token_SEMICOLON
        || yytoken == Token_AT
        || yytoken == Token_LESS_THAN
        || yytoken == Token_IDENTIFIER)
      {
        optional_modifiers_ast *modifiers = 0;

        type_parameters_ast *type_parameters = 0;

        type_ast *type = 0;

        const list_node<variable_declarator_ast *> *variable_declarator_sequence = 0;

        if (yytoken == Token_ABSTRACT
            || yytoken == Token_BOOLEAN
            || yytoken == Token_BYTE
            || yytoken == Token_CHAR
            || yytoken == Token_CLASS
            || yytoken == Token_DOUBLE
            || yytoken == Token_ENUM
            || yytoken == Token_FINAL
            || yytoken == Token_FLOAT
            || yytoken == Token_INT
            || yytoken == Token_INTERFACE
            || yytoken == Token_LONG
            || yytoken == Token_NATIVE
            || yytoken == Token_PRIVATE
            || yytoken == Token_PROTECTED
            || yytoken == Token_PUBLIC
            || yytoken == Token_SHORT
            || yytoken == Token_STATIC
            || yytoken == Token_STRICTFP
            || yytoken == Token_SYNCHRONIZED
            || yytoken == Token_TRANSIENT
            || yytoken == Token_VOID
            || yytoken == Token_VOLATILE
            || yytoken == Token_AT
            || yytoken == Token_LESS_THAN
            || yytoken == Token_IDENTIFIER)
          {
            optional_modifiers_ast *__node_176 = 0;
            if (!parse_optional_modifiers(&__node_176))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_optional_modifiers, "optional_modifiers");
                  }
                return false;
              }
            modifiers = __node_176;

            if (yytoken == Token_CLASS)
              {
                class_declaration_ast *__node_177 = 0;
                if (!parse_class_declaration(&__node_177, modifiers))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_class_declaration, "class_declaration");
                      }
                    return false;
                  }
                (*yynode)->class_declaration = __node_177;

              }
            else if (yytoken == Token_ENUM)
              {
                enum_declaration_ast *__node_178 = 0;
                if (!parse_enum_declaration(&__node_178, modifiers))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_enum_declaration, "enum_declaration");
                      }
                    return false;
                  }
                (*yynode)->enum_declaration = __node_178;

              }
            else if (yytoken == Token_INTERFACE)
              {
                interface_declaration_ast *__node_179 = 0;
                if (!parse_interface_declaration(&__node_179, modifiers))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_interface_declaration, "interface_declaration");
                      }
                    return false;
                  }
                (*yynode)->interface_declaration = __node_179;

              }
            else if (yytoken == Token_AT)
              {
                annotation_type_declaration_ast *__node_180 = 0;
                if (!parse_annotation_type_declaration(&__node_180, modifiers))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_annotation_type_declaration, "annotation_type_declaration");
                      }
                    return false;
                  }
                (*yynode)->annotation_type_declaration = __node_180;

              }
            else if (yytoken == Token_BOOLEAN
                     || yytoken == Token_BYTE
                     || yytoken == Token_CHAR
                     || yytoken == Token_DOUBLE
                     || yytoken == Token_FLOAT
                     || yytoken == Token_INT
                     || yytoken == Token_LONG
                     || yytoken == Token_SHORT
                     || yytoken == Token_VOID
                     || yytoken == Token_LESS_THAN
                     || yytoken == Token_IDENTIFIER)
              {
                bool has_type_parameters = false;
                if ((yytoken == Token_LESS_THAN) && ( compatibility_mode() >= java15_compatibility ))
                  {
                    type_parameters_ast *__node_181 = 0;
                    if (!parse_type_parameters(&__node_181))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_type_parameters, "type_parameters");
                          }
                        return false;
                      }
                    type_parameters = __node_181;

                    has_type_parameters = true;
                  }
                else if (true /*epsilon*/)
                {}
                else
                  {
                    return false;
                  }
                type_ast *__node_182 = 0;
                if (!parse_type(&__node_182))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_type, "type");
                      }
                    return false;
                  }
                type = __node_182;

                if ((yytoken == Token_IDENTIFIER) && ( LA(2).kind == Token_LPAREN ))
                  {
                    interface_method_declaration_ast *__node_183 = 0;
                    if (!parse_interface_method_declaration(&__node_183,
                                                            modifiers, type_parameters, type
                                                           ))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_interface_method_declaration, "interface_method_declaration");
                          }
                        return false;
                      }
                    (*yynode)->interface_method_declaration = __node_183;

                  }
                else if ((yytoken == Token_IDENTIFIER) && ( type_parameters == 0 ))
                  {
                    variable_declarator_ast *__node_184 = 0;
                    if (!parse_variable_declarator(&__node_184))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_variable_declarator, "variable_declarator");
                          }
                        return false;
                      }
                    variable_declarator_sequence = snoc(variable_declarator_sequence, __node_184, memory_pool);

                    while (yytoken == Token_COMMA)
                      {
                        if (yytoken != Token_COMMA)
                          {
                            if (!yy_block_errors)
                              {
                                yy_expected_token(yytoken, Token_COMMA, ",");
                              }
                            return false;
                          }
                        yylex();

                        variable_declarator_ast *__node_185 = 0;
                        if (!parse_variable_declarator(&__node_185))
                          {
                            if (!yy_block_errors)
                              {
                                yy_expected_symbol(ast_node::Kind_variable_declarator, "variable_declarator");
                              }
                            return false;
                          }
                        variable_declarator_sequence = snoc(variable_declarator_sequence, __node_185, memory_pool);

                      }
                    if (yytoken != Token_SEMICOLON)
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_token(yytoken, Token_SEMICOLON, ";");
                          }
                        return false;
                      }
                    yylex();

                    variable_declaration_data_ast *__node_186 = 0;
                    if (!parse_variable_declaration_data(&__node_186,
                                                         modifiers, type, variable_declarator_sequence
                                                        ))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_variable_declaration_data, "variable_declaration_data");
                          }
                        return false;
                      }
                    (*yynode)->variable_declaration = __node_186;

                  }
                else if (yytoken == Token_SEMICOLON)
                  {
                    report_problem( error,
                                    "Expected method declaration after type parameter list" );
                    if (yytoken != Token_SEMICOLON)
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_token(yytoken, Token_SEMICOLON, ";");
                          }
                        return false;
                      }
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
          }
        else if (yytoken == Token_SEMICOLON)
          {
            if (yytoken != Token_SEMICOLON)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_SEMICOLON, ";");
                  }
                return false;
              }
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

  bool parser::parse_interface_method_declaration(interface_method_declaration_ast **yynode, optional_modifiers_ast *modifiers, type_parameters_ast *type_parameters, type_ast *return_type)
  {
    *yynode = create<interface_method_declaration_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    (*yynode)->modifiers = modifiers;
    if (modifiers && modifiers->start_token < (*yynode)->start_token)
      (*yynode)->start_token = modifiers->start_token;

    (*yynode)->type_parameters = type_parameters;
    if (type_parameters && type_parameters->start_token < (*yynode)->start_token)
      (*yynode)->start_token = type_parameters->start_token;

    (*yynode)->return_type = return_type;
    if (return_type && return_type->start_token < (*yynode)->start_token)
      (*yynode)->start_token = return_type->start_token;

    if (yytoken == Token_IDENTIFIER)
      {
        identifier_ast *__node_187 = 0;
        if (!parse_identifier(&__node_187))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_identifier, "identifier");
              }
            return false;
          }
        (*yynode)->method_name = __node_187;

        if (yytoken != Token_LPAREN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_LPAREN, "(");
              }
            return false;
          }
        yylex();

        optional_parameter_declaration_list_ast *__node_188 = 0;
        if (!parse_optional_parameter_declaration_list(&__node_188))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_optional_parameter_declaration_list, "optional_parameter_declaration_list");
              }
            return false;
          }
        (*yynode)->parameters = __node_188;

        if (yytoken != Token_RPAREN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_RPAREN, ")");
              }
            return false;
          }
        yylex();

        optional_declarator_brackets_ast *__node_189 = 0;
        if (!parse_optional_declarator_brackets(&__node_189))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_optional_declarator_brackets, "optional_declarator_brackets");
              }
            return false;
          }
        (*yynode)->declarator_brackets = __node_189;

        if (yytoken == Token_THROWS)
          {
            throws_clause_ast *__node_190 = 0;
            if (!parse_throws_clause(&__node_190))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_throws_clause, "throws_clause");
                  }
                return false;
              }
            (*yynode)->throws_clause = __node_190;

          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        if (yytoken != Token_SEMICOLON)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_SEMICOLON, ";");
              }
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

  bool parser::parse_labeled_statement(labeled_statement_ast **yynode)
  {
    *yynode = create<labeled_statement_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_IDENTIFIER)
      {
        identifier_ast *__node_191 = 0;
        if (!parse_identifier(&__node_191))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_identifier, "identifier");
              }
            return false;
          }
        (*yynode)->label = __node_191;

        if (yytoken != Token_COLON)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_COLON, ":");
              }
            return false;
          }
        yylex();

        embedded_statement_ast *__node_192 = 0;
        if (!parse_embedded_statement(&__node_192))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_embedded_statement, "embedded_statement");
              }
            return false;
          }
        (*yynode)->statement = __node_192;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_literal(literal_ast **yynode)
  {
    *yynode = create<literal_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_FLOATING_POINT_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL)
      {
        if (yytoken == Token_TRUE)
          {
            if (yytoken != Token_TRUE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_TRUE, "true");
                  }
                return false;
              }
            yylex();

            (*yynode)->literal_type = literal::type_true;
          }
        else if (yytoken == Token_FALSE)
          {
            if (yytoken != Token_FALSE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_FALSE, "false");
                  }
                return false;
              }
            yylex();

            (*yynode)->literal_type = literal::type_false;
          }
        else if (yytoken == Token_NULL)
          {
            if (yytoken != Token_NULL)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_NULL, "null");
                  }
                return false;
              }
            yylex();

            (*yynode)->literal_type = literal::type_null;
          }
        else if (yytoken == Token_INTEGER_LITERAL)
          {
            if (yytoken != Token_INTEGER_LITERAL)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_INTEGER_LITERAL, "integer literal");
                  }
                return false;
              }
            (*yynode)->integer_literal = token_stream->index() - 1;
            yylex();

            (*yynode)->literal_type = literal::type_integer;
          }
        else if (yytoken == Token_FLOATING_POINT_LITERAL)
          {
            if (yytoken != Token_FLOATING_POINT_LITERAL)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_FLOATING_POINT_LITERAL, "floating point literal");
                  }
                return false;
              }
            (*yynode)->floating_point_literal = token_stream->index() - 1;
            yylex();

            (*yynode)->literal_type = literal::type_floating_point;
          }
        else if (yytoken == Token_CHARACTER_LITERAL)
          {
            if (yytoken != Token_CHARACTER_LITERAL)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_CHARACTER_LITERAL, "character literal");
                  }
                return false;
              }
            (*yynode)->character_literal = token_stream->index() - 1;
            yylex();

            (*yynode)->literal_type = literal::type_character;
          }
        else if (yytoken == Token_STRING_LITERAL)
          {
            if (yytoken != Token_STRING_LITERAL)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_STRING_LITERAL, "string literal");
                  }
                return false;
              }
            (*yynode)->string_literal = token_stream->index() - 1;
            yylex();

            (*yynode)->literal_type = literal::type_string;
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

  bool parser::parse_logical_and_expression(logical_and_expression_ast **yynode)
  {
    *yynode = create<logical_and_expression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOOLEAN
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_SHORT
        || yytoken == Token_SUPER
        || yytoken == Token_THIS
        || yytoken == Token_VOID
        || yytoken == Token_LPAREN
        || yytoken == Token_LESS_THAN
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_FLOATING_POINT_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        bit_or_expression_ast *__node_193 = 0;
        if (!parse_bit_or_expression(&__node_193))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_bit_or_expression, "bit_or_expression");
              }
            return false;
          }
        (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_193, memory_pool);

        while (yytoken == Token_LOG_AND)
          {
            if (yytoken != Token_LOG_AND)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LOG_AND, "&&");
                  }
                return false;
              }
            yylex();

            bit_or_expression_ast *__node_194 = 0;
            if (!parse_bit_or_expression(&__node_194))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_bit_or_expression, "bit_or_expression");
                  }
                return false;
              }
            (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_194, memory_pool);

          }
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_logical_or_expression(logical_or_expression_ast **yynode)
  {
    *yynode = create<logical_or_expression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOOLEAN
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_SHORT
        || yytoken == Token_SUPER
        || yytoken == Token_THIS
        || yytoken == Token_VOID
        || yytoken == Token_LPAREN
        || yytoken == Token_LESS_THAN
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_FLOATING_POINT_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        logical_and_expression_ast *__node_195 = 0;
        if (!parse_logical_and_expression(&__node_195))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_logical_and_expression, "logical_and_expression");
              }
            return false;
          }
        (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_195, memory_pool);

        while (yytoken == Token_LOG_OR)
          {
            if (yytoken != Token_LOG_OR)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LOG_OR, "||");
                  }
                return false;
              }
            yylex();

            logical_and_expression_ast *__node_196 = 0;
            if (!parse_logical_and_expression(&__node_196))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_logical_and_expression, "logical_and_expression");
                  }
                return false;
              }
            (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_196, memory_pool);

          }
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_mandatory_array_builtin_type(mandatory_array_builtin_type_ast **yynode)
  {
    *yynode = create<mandatory_array_builtin_type_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOOLEAN
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_SHORT
        || yytoken == Token_VOID)
      {
        builtin_type_ast *__node_197 = 0;
        if (!parse_builtin_type(&__node_197))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_builtin_type, "builtin_type");
              }
            return false;
          }
        (*yynode)->type = __node_197;

        mandatory_declarator_brackets_ast *__node_198 = 0;
        if (!parse_mandatory_declarator_brackets(&__node_198))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_mandatory_declarator_brackets, "mandatory_declarator_brackets");
              }
            return false;
          }
        (*yynode)->declarator_brackets = __node_198;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_mandatory_declarator_brackets(mandatory_declarator_brackets_ast **yynode)
  {
    *yynode = create<mandatory_declarator_brackets_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_LBRACKET)
      {
        do
          {
            if (yytoken != Token_LBRACKET)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LBRACKET, "[");
                  }
                return false;
              }
            yylex();

            if (yytoken != Token_RBRACKET)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_RBRACKET, "]");
                  }
                return false;
              }
            yylex();

            (*yynode)->bracket_count++;
          }
        while (yytoken == Token_LBRACKET);
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_method_call_data(method_call_data_ast **yynode, non_wildcard_type_arguments_ast *type_arguments, identifier_ast *method_name, optional_argument_list_ast *arguments)
  {
    *yynode = create<method_call_data_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    (*yynode)->type_arguments = type_arguments;
    if (type_arguments && type_arguments->start_token < (*yynode)->start_token)
      (*yynode)->start_token = type_arguments->start_token;

    (*yynode)->method_name = method_name;
    if (method_name && method_name->start_token < (*yynode)->start_token)
      (*yynode)->start_token = method_name->start_token;

    (*yynode)->arguments = arguments;
    if (arguments && arguments->start_token < (*yynode)->start_token)
      (*yynode)->start_token = arguments->start_token;

    if (true /*epsilon*/ || yytoken == Token_INSTANCEOF
        || yytoken == Token_RPAREN
        || yytoken == Token_RBRACE
        || yytoken == Token_LBRACKET
        || yytoken == Token_RBRACKET
        || yytoken == Token_SEMICOLON
        || yytoken == Token_COMMA
        || yytoken == Token_DOT
        || yytoken == Token_ASSIGN
        || yytoken == Token_LESS_THAN
        || yytoken == Token_GREATER_THAN
        || yytoken == Token_QUESTION
        || yytoken == Token_COLON
        || yytoken == Token_EQUAL
        || yytoken == Token_LESS_EQUAL
        || yytoken == Token_GREATER_EQUAL
        || yytoken == Token_NOT_EQUAL
        || yytoken == Token_LOG_AND
        || yytoken == Token_LOG_OR
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_STAR
        || yytoken == Token_SLASH
        || yytoken == Token_BIT_AND
        || yytoken == Token_BIT_OR
        || yytoken == Token_BIT_XOR
        || yytoken == Token_REMAINDER
        || yytoken == Token_LSHIFT
        || yytoken == Token_SIGNED_RSHIFT
        || yytoken == Token_UNSIGNED_RSHIFT
        || yytoken == Token_PLUS_ASSIGN
        || yytoken == Token_MINUS_ASSIGN
        || yytoken == Token_STAR_ASSIGN
        || yytoken == Token_SLASH_ASSIGN
        || yytoken == Token_BIT_AND_ASSIGN
        || yytoken == Token_BIT_OR_ASSIGN
        || yytoken == Token_BIT_XOR_ASSIGN
        || yytoken == Token_REMAINDER_ASSIGN
        || yytoken == Token_LSHIFT_ASSIGN
        || yytoken == Token_SIGNED_RSHIFT_ASSIGN
        || yytoken == Token_UNSIGNED_RSHIFT_ASSIGN)
    {}
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_method_declaration(method_declaration_ast **yynode, optional_modifiers_ast *modifiers, type_parameters_ast *type_parameters, type_ast *return_type)
  {
    *yynode = create<method_declaration_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    (*yynode)->modifiers = modifiers;
    if (modifiers && modifiers->start_token < (*yynode)->start_token)
      (*yynode)->start_token = modifiers->start_token;

    (*yynode)->type_parameters = type_parameters;
    if (type_parameters && type_parameters->start_token < (*yynode)->start_token)
      (*yynode)->start_token = type_parameters->start_token;

    (*yynode)->return_type = return_type;
    if (return_type && return_type->start_token < (*yynode)->start_token)
      (*yynode)->start_token = return_type->start_token;

    if (yytoken == Token_IDENTIFIER)
      {
        block_ast *body = 0;

        identifier_ast *__node_199 = 0;
        if (!parse_identifier(&__node_199))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_identifier, "identifier");
              }
            return false;
          }
        (*yynode)->method_name = __node_199;

        if (yytoken != Token_LPAREN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_LPAREN, "(");
              }
            return false;
          }
        yylex();

        optional_parameter_declaration_list_ast *__node_200 = 0;
        if (!parse_optional_parameter_declaration_list(&__node_200))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_optional_parameter_declaration_list, "optional_parameter_declaration_list");
              }
            return false;
          }
        (*yynode)->parameters = __node_200;

        if (yytoken != Token_RPAREN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_RPAREN, ")");
              }
            return false;
          }
        yylex();

        optional_declarator_brackets_ast *__node_201 = 0;
        if (!parse_optional_declarator_brackets(&__node_201))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_optional_declarator_brackets, "optional_declarator_brackets");
              }
            return false;
          }
        (*yynode)->declarator_brackets = __node_201;

        if (yytoken == Token_THROWS)
          {
            throws_clause_ast *__node_202 = 0;
            if (!parse_throws_clause(&__node_202))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_throws_clause, "throws_clause");
                  }
                return false;
              }
            (*yynode)->throws_clause = __node_202;

          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        if (yytoken == Token_LBRACE)
          {
            block_ast *__node_203 = 0;
            if (!parse_block(&__node_203))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_block, "block");
                  }
                return false;
              }
            body = __node_203;

          }
        else if (yytoken == Token_SEMICOLON)
          {
            if (yytoken != Token_SEMICOLON)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_SEMICOLON, ";");
                  }
                return false;
              }
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

  bool parser::parse_multiplicative_expression(multiplicative_expression_ast **yynode)
  {
    *yynode = create<multiplicative_expression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOOLEAN
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_SHORT
        || yytoken == Token_SUPER
        || yytoken == Token_THIS
        || yytoken == Token_VOID
        || yytoken == Token_LPAREN
        || yytoken == Token_LESS_THAN
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_FLOATING_POINT_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        unary_expression_ast *__node_204 = 0;
        if (!parse_unary_expression(&__node_204))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_unary_expression, "unary_expression");
              }
            return false;
          }
        (*yynode)->expression = __node_204;

        while (yytoken == Token_STAR
               || yytoken == Token_SLASH
               || yytoken == Token_REMAINDER)
          {
            multiplicative_expression_rest_ast *__node_205 = 0;
            if (!parse_multiplicative_expression_rest(&__node_205))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_multiplicative_expression_rest, "multiplicative_expression_rest");
                  }
                return false;
              }
            (*yynode)->additional_expression_sequence = snoc((*yynode)->additional_expression_sequence, __node_205, memory_pool);

          }
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_multiplicative_expression_rest(multiplicative_expression_rest_ast **yynode)
  {
    *yynode = create<multiplicative_expression_rest_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_STAR
        || yytoken == Token_SLASH
        || yytoken == Token_REMAINDER)
      {
        if (yytoken == Token_STAR)
          {
            if (yytoken != Token_STAR)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_STAR, "*");
                  }
                return false;
              }
            yylex();

            (*yynode)->multiplicative_operator = multiplicative_expression_rest::op_star;
          }
        else if (yytoken == Token_SLASH)
          {
            if (yytoken != Token_SLASH)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_SLASH, "/");
                  }
                return false;
              }
            yylex();

            (*yynode)->multiplicative_operator = multiplicative_expression_rest::op_slash;
          }
        else if (yytoken == Token_REMAINDER)
          {
            if (yytoken != Token_REMAINDER)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_REMAINDER, "%");
                  }
                return false;
              }
            yylex();

            (*yynode)->multiplicative_operator = multiplicative_expression_rest::op_remainder;
          }
        else
          {
            return false;
          }
        unary_expression_ast *__node_206 = 0;
        if (!parse_unary_expression(&__node_206))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_unary_expression, "unary_expression");
              }
            return false;
          }
        (*yynode)->expression = __node_206;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_new_expression(new_expression_ast **yynode)
  {
    *yynode = create<new_expression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_NEW)
      {
        if (yytoken != Token_NEW)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_NEW, "new");
              }
            return false;
          }
        yylex();

        if ((yytoken == Token_LESS_THAN) && ( compatibility_mode() >= java15_compatibility ))
          {
            non_wildcard_type_arguments_ast *__node_207 = 0;
            if (!parse_non_wildcard_type_arguments(&__node_207))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_non_wildcard_type_arguments, "non_wildcard_type_arguments");
                  }
                return false;
              }
            (*yynode)->type_arguments = __node_207;

          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        non_array_type_ast *__node_208 = 0;
        if (!parse_non_array_type(&__node_208))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_non_array_type, "non_array_type");
              }
            return false;
          }
        (*yynode)->type = __node_208;

        if (yytoken == Token_LPAREN)
          {
            if (yytoken != Token_LPAREN)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LPAREN, "(");
                  }
                return false;
              }
            yylex();

            optional_argument_list_ast *__node_209 = 0;
            if (!parse_optional_argument_list(&__node_209))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_optional_argument_list, "optional_argument_list");
                  }
                return false;
              }
            (*yynode)->class_constructor_arguments = __node_209;

            if (yytoken != Token_RPAREN)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_RPAREN, ")");
                  }
                return false;
              }
            yylex();

            if (yytoken == Token_LBRACE)
              {
                class_body_ast *__node_210 = 0;
                if (!parse_class_body(&__node_210))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_class_body, "class_body");
                      }
                    return false;
                  }
                (*yynode)->class_body = __node_210;

              }
            else if (true /*epsilon*/)
            {}
            else
              {
                return false;
              }
          }
        else if (yytoken == Token_LBRACKET)
          {
            array_creator_rest_ast *__node_211 = 0;
            if (!parse_array_creator_rest(&__node_211))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_array_creator_rest, "array_creator_rest");
                  }
                return false;
              }
            (*yynode)->array_creator_rest = __node_211;

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

  bool parser::parse_non_array_type(non_array_type_ast **yynode)
  {
    *yynode = create<non_array_type_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOOLEAN
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_SHORT
        || yytoken == Token_VOID
        || yytoken == Token_IDENTIFIER)
      {
        if (yytoken == Token_IDENTIFIER)
          {
            class_or_interface_type_name_ast *__node_212 = 0;
            if (!parse_class_or_interface_type_name(&__node_212))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_class_or_interface_type_name, "class_or_interface_type_name");
                  }
                return false;
              }
            (*yynode)->class_or_interface_type = __node_212;

          }
        else if (yytoken == Token_BOOLEAN
                 || yytoken == Token_BYTE
                 || yytoken == Token_CHAR
                 || yytoken == Token_DOUBLE
                 || yytoken == Token_FLOAT
                 || yytoken == Token_INT
                 || yytoken == Token_LONG
                 || yytoken == Token_SHORT
                 || yytoken == Token_VOID)
          {
            builtin_type_ast *__node_213 = 0;
            if (!parse_builtin_type(&__node_213))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_builtin_type, "builtin_type");
                  }
                return false;
              }
            (*yynode)->builtin_type = __node_213;

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

  bool parser::parse_non_wildcard_type_arguments(non_wildcard_type_arguments_ast **yynode)
  {
    *yynode = create<non_wildcard_type_arguments_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_LESS_THAN)
      {
        if (yytoken != Token_LESS_THAN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_LESS_THAN, "<");
              }
            return false;
          }
        yylex();

        int currentLtLevel = _M_state.ltCounter;
        _M_state.ltCounter++;
        type_argument_type_ast *__node_214 = 0;
        if (!parse_type_argument_type(&__node_214))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_type_argument_type, "type_argument_type");
              }
            return false;
          }
        (*yynode)->type_argument_type_sequence = snoc((*yynode)->type_argument_type_sequence, __node_214, memory_pool);

        while (yytoken == Token_COMMA)
          {
            if ( _M_state.ltCounter != currentLtLevel + 1 )
              {
                break;
              }
            if (yytoken != Token_COMMA)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_COMMA, ",");
                  }
                return false;
              }
            yylex();

            type_argument_type_ast *__node_215 = 0;
            if (!parse_type_argument_type(&__node_215))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_type_argument_type, "type_argument_type");
                  }
                return false;
              }
            (*yynode)->type_argument_type_sequence = snoc((*yynode)->type_argument_type_sequence, __node_215, memory_pool);

          }
        if (yytoken == Token_GREATER_THAN
            || yytoken == Token_SIGNED_RSHIFT
            || yytoken == Token_UNSIGNED_RSHIFT)
          {
            type_arguments_or_parameters_end_ast *__node_216 = 0;
            if (!parse_type_arguments_or_parameters_end(&__node_216))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_type_arguments_or_parameters_end, "type_arguments_or_parameters_end");
                  }
                return false;
              }
          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        if (currentLtLevel == 0 && _M_state.ltCounter != currentLtLevel )
          {
            if (!yy_block_errors)
              {
                report_problem(error, "The amount of closing ``>'' characters is incorrect");
              }
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

  bool parser::parse_optional_argument_list(optional_argument_list_ast **yynode)
  {
    *yynode = create<optional_argument_list_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOOLEAN
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_SHORT
        || yytoken == Token_SUPER
        || yytoken == Token_THIS
        || yytoken == Token_VOID
        || yytoken == Token_LPAREN
        || yytoken == Token_LESS_THAN
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_FLOATING_POINT_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER || yytoken == Token_RPAREN)
      {
        if (yytoken == Token_BOOLEAN
            || yytoken == Token_BYTE
            || yytoken == Token_CHAR
            || yytoken == Token_DOUBLE
            || yytoken == Token_FLOAT
            || yytoken == Token_INT
            || yytoken == Token_LONG
            || yytoken == Token_NEW
            || yytoken == Token_SHORT
            || yytoken == Token_SUPER
            || yytoken == Token_THIS
            || yytoken == Token_VOID
            || yytoken == Token_LPAREN
            || yytoken == Token_LESS_THAN
            || yytoken == Token_BANG
            || yytoken == Token_TILDE
            || yytoken == Token_INCREMENT
            || yytoken == Token_DECREMENT
            || yytoken == Token_PLUS
            || yytoken == Token_MINUS
            || yytoken == Token_TRUE
            || yytoken == Token_FALSE
            || yytoken == Token_NULL
            || yytoken == Token_INTEGER_LITERAL
            || yytoken == Token_FLOATING_POINT_LITERAL
            || yytoken == Token_CHARACTER_LITERAL
            || yytoken == Token_STRING_LITERAL
            || yytoken == Token_IDENTIFIER)
          {
            std::size_t try_start_token_15 = token_stream->index() - 1;
            parser_state *try_start_state_15 = copy_current_state();
            {
              expression_ast *__node_217 = 0;
              if (!parse_expression(&__node_217))
                {
                  goto __catch_15;
                }
              (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_217, memory_pool);

              while (yytoken == Token_COMMA)
                {
                  if (yytoken != Token_COMMA)
                    goto __catch_15;
                  yylex();

                  expression_ast *__node_218 = 0;
                  if (!parse_expression(&__node_218))
                    {
                      goto __catch_15;
                    }
                  (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_218, memory_pool);

                }
            }
            if (try_start_state_15)
              delete try_start_state_15;

            if (false) // the only way to enter here is using goto
              {
              __catch_15:
                if (try_start_state_15)
                  {
                    restore_state(try_start_state_15);
                    delete try_start_state_15;
                  }
                if (try_start_token_15 == token_stream->index() - 1)
                  yylex();

                while (yytoken != Token_EOF
                       && yytoken != Token_RPAREN)
                  {
                    yylex();
                  }
              }

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

  bool parser::parse_optional_array_builtin_type(optional_array_builtin_type_ast **yynode)
  {
    *yynode = create<optional_array_builtin_type_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOOLEAN
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_SHORT
        || yytoken == Token_VOID)
      {
        builtin_type_ast *__node_219 = 0;
        if (!parse_builtin_type(&__node_219))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_builtin_type, "builtin_type");
              }
            return false;
          }
        (*yynode)->type = __node_219;

        optional_declarator_brackets_ast *__node_220 = 0;
        if (!parse_optional_declarator_brackets(&__node_220))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_optional_declarator_brackets, "optional_declarator_brackets");
              }
            return false;
          }
        (*yynode)->declarator_brackets = __node_220;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_optional_declarator_brackets(optional_declarator_brackets_ast **yynode)
  {
    *yynode = create<optional_declarator_brackets_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_LBRACKET || yytoken == Token_BOOLEAN
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DOUBLE
        || yytoken == Token_EXTENDS
        || yytoken == Token_FLOAT
        || yytoken == Token_IMPLEMENTS
        || yytoken == Token_INSTANCEOF
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_SHORT
        || yytoken == Token_SUPER
        || yytoken == Token_THIS
        || yytoken == Token_THROWS
        || yytoken == Token_VOID
        || yytoken == Token_LPAREN
        || yytoken == Token_RPAREN
        || yytoken == Token_LBRACE
        || yytoken == Token_RBRACE
        || yytoken == Token_LBRACKET
        || yytoken == Token_RBRACKET
        || yytoken == Token_SEMICOLON
        || yytoken == Token_COMMA
        || yytoken == Token_DOT
        || yytoken == Token_ASSIGN
        || yytoken == Token_LESS_THAN
        || yytoken == Token_GREATER_THAN
        || yytoken == Token_QUESTION
        || yytoken == Token_COLON
        || yytoken == Token_EQUAL
        || yytoken == Token_LESS_EQUAL
        || yytoken == Token_GREATER_EQUAL
        || yytoken == Token_NOT_EQUAL
        || yytoken == Token_LOG_AND
        || yytoken == Token_LOG_OR
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_STAR
        || yytoken == Token_SLASH
        || yytoken == Token_BIT_AND
        || yytoken == Token_BIT_OR
        || yytoken == Token_BIT_XOR
        || yytoken == Token_REMAINDER
        || yytoken == Token_LSHIFT
        || yytoken == Token_SIGNED_RSHIFT
        || yytoken == Token_UNSIGNED_RSHIFT
        || yytoken == Token_PLUS_ASSIGN
        || yytoken == Token_MINUS_ASSIGN
        || yytoken == Token_STAR_ASSIGN
        || yytoken == Token_SLASH_ASSIGN
        || yytoken == Token_BIT_AND_ASSIGN
        || yytoken == Token_BIT_OR_ASSIGN
        || yytoken == Token_BIT_XOR_ASSIGN
        || yytoken == Token_REMAINDER_ASSIGN
        || yytoken == Token_LSHIFT_ASSIGN
        || yytoken == Token_SIGNED_RSHIFT_ASSIGN
        || yytoken == Token_UNSIGNED_RSHIFT_ASSIGN
        || yytoken == Token_ELLIPSIS
        || yytoken == Token_IDENTIFIER)
      {
        while (yytoken == Token_LBRACKET)
          {
            if (yytoken != Token_LBRACKET)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LBRACKET, "[");
                  }
                return false;
              }
            yylex();

            if (yytoken != Token_RBRACKET)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_RBRACKET, "]");
                  }
                return false;
              }
            yylex();

            (*yynode)->bracket_count++;
          }
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_optional_modifiers(optional_modifiers_ast **yynode)
  {
    *yynode = create<optional_modifiers_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_ABSTRACT
        || yytoken == Token_FINAL
        || yytoken == Token_NATIVE
        || yytoken == Token_PRIVATE
        || yytoken == Token_PROTECTED
        || yytoken == Token_PUBLIC
        || yytoken == Token_STATIC
        || yytoken == Token_STRICTFP
        || yytoken == Token_SYNCHRONIZED
        || yytoken == Token_TRANSIENT
        || yytoken == Token_VOLATILE
        || yytoken == Token_AT || yytoken == Token_BOOLEAN
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_CLASS
        || yytoken == Token_DOUBLE
        || yytoken == Token_ENUM
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_INTERFACE
        || yytoken == Token_LONG
        || yytoken == Token_SHORT
        || yytoken == Token_VOID
        || yytoken == Token_AT
        || yytoken == Token_LESS_THAN
        || yytoken == Token_IDENTIFIER)
      {
        while (yytoken == Token_ABSTRACT
               || yytoken == Token_FINAL
               || yytoken == Token_NATIVE
               || yytoken == Token_PRIVATE
               || yytoken == Token_PROTECTED
               || yytoken == Token_PUBLIC
               || yytoken == Token_STATIC
               || yytoken == Token_STRICTFP
               || yytoken == Token_SYNCHRONIZED
               || yytoken == Token_TRANSIENT
               || yytoken == Token_VOLATILE
               || yytoken == Token_AT)
          {
            if (yytoken == Token_PRIVATE)
              {
                if (yytoken != Token_PRIVATE)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_PRIVATE, "private");
                      }
                    return false;
                  }
                yylex();

                (*yynode)->modifiers |= modifiers::mod_private;
              }
            else if (yytoken == Token_PUBLIC)
              {
                if (yytoken != Token_PUBLIC)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_PUBLIC, "public");
                      }
                    return false;
                  }
                yylex();

                (*yynode)->modifiers |= modifiers::mod_public;
              }
            else if (yytoken == Token_PROTECTED)
              {
                if (yytoken != Token_PROTECTED)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_PROTECTED, "protected");
                      }
                    return false;
                  }
                yylex();

                (*yynode)->modifiers |= modifiers::mod_protected;
              }
            else if (yytoken == Token_STATIC)
              {
                if (yytoken != Token_STATIC)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_STATIC, "static");
                      }
                    return false;
                  }
                yylex();

                (*yynode)->modifiers |= modifiers::mod_static;
              }
            else if (yytoken == Token_TRANSIENT)
              {
                if (yytoken != Token_TRANSIENT)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_TRANSIENT, "transient");
                      }
                    return false;
                  }
                yylex();

                (*yynode)->modifiers |= modifiers::mod_transient;
              }
            else if (yytoken == Token_FINAL)
              {
                if (yytoken != Token_FINAL)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_FINAL, "final");
                      }
                    return false;
                  }
                yylex();

                (*yynode)->modifiers |= modifiers::mod_final;
              }
            else if (yytoken == Token_ABSTRACT)
              {
                if (yytoken != Token_ABSTRACT)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_ABSTRACT, "abstract");
                      }
                    return false;
                  }
                yylex();

                (*yynode)->modifiers |= modifiers::mod_abstract;
              }
            else if (yytoken == Token_NATIVE)
              {
                if (yytoken != Token_NATIVE)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_NATIVE, "native");
                      }
                    return false;
                  }
                yylex();

                (*yynode)->modifiers |= modifiers::mod_native;
              }
            else if (yytoken == Token_SYNCHRONIZED)
              {
                if (yytoken != Token_SYNCHRONIZED)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_SYNCHRONIZED, "synchronized");
                      }
                    return false;
                  }
                yylex();

                (*yynode)->modifiers |= modifiers::mod_synchronized;
              }
            else if (yytoken == Token_VOLATILE)
              {
                if (yytoken != Token_VOLATILE)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_VOLATILE, "volatile");
                      }
                    return false;
                  }
                yylex();

                (*yynode)->modifiers |= modifiers::mod_volatile;
              }
            else if (yytoken == Token_STRICTFP)
              {
                if (yytoken != Token_STRICTFP)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_STRICTFP, "strictfp");
                      }
                    return false;
                  }
                yylex();

                (*yynode)->modifiers |= modifiers::mod_strictfp;
              }
            else if (yytoken == Token_AT)
              {
                if (yytoken == Token_AT && LA(2).kind == Token_INTERFACE)
                  {
                    break;
                  }
                std::size_t try_start_token_16 = token_stream->index() - 1;
                parser_state *try_start_state_16 = copy_current_state();
                {
                  annotation_ast *__node_221 = 0;
                  if (!parse_annotation(&__node_221))
                    {
                      goto __catch_16;
                    }
                  (*yynode)->mod_annotation_sequence = snoc((*yynode)->mod_annotation_sequence, __node_221, memory_pool);

                }
                if (try_start_state_16)
                  delete try_start_state_16;

                if (false) // the only way to enter here is using goto
                  {
                  __catch_16:
                    if (try_start_state_16)
                      {
                        restore_state(try_start_state_16);
                        delete try_start_state_16;
                      }
                    if (try_start_token_16 == token_stream->index() - 1)
                      yylex();

                    while (yytoken != Token_EOF
                           && yytoken != Token_ABSTRACT
                           && yytoken != Token_BOOLEAN
                           && yytoken != Token_BYTE
                           && yytoken != Token_CHAR
                           && yytoken != Token_CLASS
                           && yytoken != Token_DOUBLE
                           && yytoken != Token_ENUM
                           && yytoken != Token_FINAL
                           && yytoken != Token_FLOAT
                           && yytoken != Token_INT
                           && yytoken != Token_INTERFACE
                           && yytoken != Token_LONG
                           && yytoken != Token_NATIVE
                           && yytoken != Token_PRIVATE
                           && yytoken != Token_PROTECTED
                           && yytoken != Token_PUBLIC
                           && yytoken != Token_SHORT
                           && yytoken != Token_STATIC
                           && yytoken != Token_STRICTFP
                           && yytoken != Token_SYNCHRONIZED
                           && yytoken != Token_TRANSIENT
                           && yytoken != Token_VOID
                           && yytoken != Token_VOLATILE
                           && yytoken != Token_AT
                           && yytoken != Token_LESS_THAN
                           && yytoken != Token_IDENTIFIER)
                      {
                        yylex();
                      }
                  }

              }
            else
              {
                return false;
              }
          }
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_optional_parameter_declaration_list(optional_parameter_declaration_list_ast **yynode)
  {
    *yynode = create<optional_parameter_declaration_list_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOOLEAN
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DOUBLE
        || yytoken == Token_FINAL
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_SHORT
        || yytoken == Token_VOID
        || yytoken == Token_AT
        || yytoken == Token_IDENTIFIER || yytoken == Token_RPAREN)
      {
        bool ellipsis_occurred = false;
        if (yytoken == Token_BOOLEAN
            || yytoken == Token_BYTE
            || yytoken == Token_CHAR
            || yytoken == Token_DOUBLE
            || yytoken == Token_FINAL
            || yytoken == Token_FLOAT
            || yytoken == Token_INT
            || yytoken == Token_LONG
            || yytoken == Token_SHORT
            || yytoken == Token_VOID
            || yytoken == Token_AT
            || yytoken == Token_IDENTIFIER)
          {
            parameter_declaration_ellipsis_ast *__node_222 = 0;
            if (!parse_parameter_declaration_ellipsis(&__node_222, &ellipsis_occurred))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_parameter_declaration_ellipsis, "parameter_declaration_ellipsis");
                  }
                return false;
              }
            (*yynode)->parameter_declaration_sequence = snoc((*yynode)->parameter_declaration_sequence, __node_222, memory_pool);

            while (yytoken == Token_COMMA)
              {
                if ( ellipsis_occurred == true )
                  {
                    break;
                  }
                if (yytoken != Token_COMMA)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_COMMA, ",");
                      }
                    return false;
                  }
                yylex();

                parameter_declaration_ellipsis_ast *__node_223 = 0;
                if (!parse_parameter_declaration_ellipsis(&__node_223, &ellipsis_occurred))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_parameter_declaration_ellipsis, "parameter_declaration_ellipsis");
                      }
                    return false;
                  }
                (*yynode)->parameter_declaration_sequence = snoc((*yynode)->parameter_declaration_sequence, __node_223, memory_pool);

              }
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

  bool parser::parse_optional_parameter_modifiers(optional_parameter_modifiers_ast **yynode)
  {
    *yynode = create<optional_parameter_modifiers_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_FINAL
        || yytoken == Token_AT || yytoken == Token_BOOLEAN
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_SHORT
        || yytoken == Token_VOID
        || yytoken == Token_IDENTIFIER)
      {
        (*yynode)->has_mod_final = false;
        while (yytoken == Token_FINAL
               || yytoken == Token_AT)
          {
            if (yytoken == Token_FINAL)
              {
                if (yytoken != Token_FINAL)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_FINAL, "final");
                      }
                    return false;
                  }
                yylex();

                (*yynode)->has_mod_final = true;
              }
            else if (yytoken == Token_AT)
              {
                std::size_t try_start_token_17 = token_stream->index() - 1;
                parser_state *try_start_state_17 = copy_current_state();
                {
                  annotation_ast *__node_224 = 0;
                  if (!parse_annotation(&__node_224))
                    {
                      goto __catch_17;
                    }
                  (*yynode)->mod_annotation_sequence = snoc((*yynode)->mod_annotation_sequence, __node_224, memory_pool);

                }
                if (try_start_state_17)
                  delete try_start_state_17;

                if (false) // the only way to enter here is using goto
                  {
                  __catch_17:
                    if (try_start_state_17)
                      {
                        restore_state(try_start_state_17);
                        delete try_start_state_17;
                      }
                    if (try_start_token_17 == token_stream->index() - 1)
                      yylex();

                    while (yytoken != Token_EOF
                           && yytoken != Token_BOOLEAN
                           && yytoken != Token_BYTE
                           && yytoken != Token_CHAR
                           && yytoken != Token_DOUBLE
                           && yytoken != Token_FINAL
                           && yytoken != Token_FLOAT
                           && yytoken != Token_INT
                           && yytoken != Token_LONG
                           && yytoken != Token_SHORT
                           && yytoken != Token_VOID
                           && yytoken != Token_AT
                           && yytoken != Token_IDENTIFIER)
                      {
                        yylex();
                      }
                  }

              }
            else
              {
                return false;
              }
          }
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_package_declaration(package_declaration_ast **yynode)
  {
    *yynode = create<package_declaration_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_PACKAGE
        || yytoken == Token_AT)
      {
        while (yytoken == Token_AT)
          {
            std::size_t try_start_token_18 = token_stream->index() - 1;
            parser_state *try_start_state_18 = copy_current_state();
            {
              annotation_ast *__node_225 = 0;
              if (!parse_annotation(&__node_225))
                {
                  goto __catch_18;
                }
              (*yynode)->annotation_sequence = snoc((*yynode)->annotation_sequence, __node_225, memory_pool);

            }
            if (try_start_state_18)
              delete try_start_state_18;

            if (false) // the only way to enter here is using goto
              {
              __catch_18:
                if (try_start_state_18)
                  {
                    restore_state(try_start_state_18);
                    delete try_start_state_18;
                  }
                if (try_start_token_18 == token_stream->index() - 1)
                  yylex();

                while (yytoken != Token_EOF
                       && yytoken != Token_PACKAGE
                       && yytoken != Token_AT)
                  {
                    yylex();
                  }
              }

          }
        if (yytoken != Token_PACKAGE)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_PACKAGE, "package");
              }
            return false;
          }
        yylex();

        qualified_identifier_ast *__node_226 = 0;
        if (!parse_qualified_identifier(&__node_226))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_qualified_identifier, "qualified_identifier");
              }
            return false;
          }
        (*yynode)->package_name = __node_226;

        if (yytoken != Token_SEMICOLON)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_SEMICOLON, ";");
              }
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

  bool parser::parse_parameter_declaration(parameter_declaration_ast **yynode)
  {
    *yynode = create<parameter_declaration_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_ABSTRACT
        || yytoken == Token_BOOLEAN
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DOUBLE
        || yytoken == Token_FINAL
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NATIVE
        || yytoken == Token_PRIVATE
        || yytoken == Token_PROTECTED
        || yytoken == Token_PUBLIC
        || yytoken == Token_SHORT
        || yytoken == Token_STATIC
        || yytoken == Token_STRICTFP
        || yytoken == Token_SYNCHRONIZED
        || yytoken == Token_TRANSIENT
        || yytoken == Token_VOID
        || yytoken == Token_VOLATILE
        || yytoken == Token_AT
        || yytoken == Token_IDENTIFIER)
      {
        optional_modifiers_ast *__node_227 = 0;
        if (!parse_optional_modifiers(&__node_227))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_optional_modifiers, "optional_modifiers");
              }
            return false;
          }
        (*yynode)->parameter_modifiers = __node_227;

        type_ast *__node_228 = 0;
        if (!parse_type(&__node_228))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_type, "type");
              }
            return false;
          }
        (*yynode)->type = __node_228;

        identifier_ast *__node_229 = 0;
        if (!parse_identifier(&__node_229))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_identifier, "identifier");
              }
            return false;
          }
        (*yynode)->variable_name = __node_229;

        optional_declarator_brackets_ast *__node_230 = 0;
        if (!parse_optional_declarator_brackets(&__node_230))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_optional_declarator_brackets, "optional_declarator_brackets");
              }
            return false;
          }
        (*yynode)->declarator_brackets = __node_230;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_parameter_declaration_ellipsis(parameter_declaration_ellipsis_ast **yynode, bool* ellipsis_occurred)
  {
    *yynode = create<parameter_declaration_ellipsis_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOOLEAN
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DOUBLE
        || yytoken == Token_FINAL
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_SHORT
        || yytoken == Token_VOID
        || yytoken == Token_AT
        || yytoken == Token_IDENTIFIER)
      {
        optional_parameter_modifiers_ast *__node_231 = 0;
        if (!parse_optional_parameter_modifiers(&__node_231))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_optional_parameter_modifiers, "optional_parameter_modifiers");
              }
            return false;
          }
        (*yynode)->parameter_modifiers = __node_231;

        type_ast *__node_232 = 0;
        if (!parse_type(&__node_232))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_type, "type");
              }
            return false;
          }
        (*yynode)->type = __node_232;

        if (yytoken == Token_ELLIPSIS)
          {
            if (yytoken != Token_ELLIPSIS)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_ELLIPSIS, "...");
                  }
                return false;
              }
            yylex();

            (*yynode)->has_ellipsis = true;
            *ellipsis_occurred = true;
          }
        else if (true /*epsilon*/)
          {
            (*yynode)->has_ellipsis = false;
          }
        else
          {
            return false;
          }
        identifier_ast *__node_233 = 0;
        if (!parse_identifier(&__node_233))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_identifier, "identifier");
              }
            return false;
          }
        (*yynode)->variable_name = __node_233;

        optional_declarator_brackets_ast *__node_234 = 0;
        if (!parse_optional_declarator_brackets(&__node_234))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_optional_declarator_brackets, "optional_declarator_brackets");
              }
            return false;
          }
        (*yynode)->declarator_brackets = __node_234;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_postfix_operator(postfix_operator_ast **yynode)
  {
    *yynode = create<postfix_operator_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT)
      {
        if (yytoken == Token_INCREMENT)
          {
            if (yytoken != Token_INCREMENT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_INCREMENT, "++");
                  }
                return false;
              }
            yylex();

            (*yynode)->postfix_operator = postfix_operator::op_increment;
          }
        else if (yytoken == Token_DECREMENT)
          {
            if (yytoken != Token_DECREMENT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_DECREMENT, "--");
                  }
                return false;
              }
            yylex();

            (*yynode)->postfix_operator = postfix_operator::op_decrement;
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

  bool parser::parse_primary_atom(primary_atom_ast **yynode)
  {
    *yynode = create<primary_atom_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOOLEAN
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_SHORT
        || yytoken == Token_SUPER
        || yytoken == Token_THIS
        || yytoken == Token_VOID
        || yytoken == Token_LPAREN
        || yytoken == Token_LESS_THAN
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_FLOATING_POINT_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        optional_argument_list_ast *arguments = 0;

        super_suffix_ast *super_suffix = 0;

        non_wildcard_type_arguments_ast *type_arguments = 0;

        identifier_ast *identifier = 0;

        if (yytoken == Token_TRUE
            || yytoken == Token_FALSE
            || yytoken == Token_NULL
            || yytoken == Token_INTEGER_LITERAL
            || yytoken == Token_FLOATING_POINT_LITERAL
            || yytoken == Token_CHARACTER_LITERAL
            || yytoken == Token_STRING_LITERAL)
          {
            literal_ast *__node_235 = 0;
            if (!parse_literal(&__node_235))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_literal, "literal");
                  }
                return false;
              }
            (*yynode)->literal = __node_235;

          }
        else if (yytoken == Token_NEW)
          {
            new_expression_ast *__node_236 = 0;
            if (!parse_new_expression(&__node_236))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_new_expression, "new_expression");
                  }
                return false;
              }
            (*yynode)->new_expression = __node_236;

          }
        else if (yytoken == Token_LPAREN)
          {
            if (yytoken != Token_LPAREN)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LPAREN, "(");
                  }
                return false;
              }
            yylex();

            expression_ast *__node_237 = 0;
            if (!parse_expression(&__node_237))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_expression, "expression");
                  }
                return false;
              }
            (*yynode)->parenthesis_expression = __node_237;

            if (yytoken != Token_RPAREN)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_RPAREN, ")");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_BOOLEAN
                 || yytoken == Token_BYTE
                 || yytoken == Token_CHAR
                 || yytoken == Token_DOUBLE
                 || yytoken == Token_FLOAT
                 || yytoken == Token_INT
                 || yytoken == Token_LONG
                 || yytoken == Token_SHORT
                 || yytoken == Token_VOID)
          {
            builtin_type_dot_class_ast *__node_238 = 0;
            if (!parse_builtin_type_dot_class(&__node_238))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_builtin_type_dot_class, "builtin_type_dot_class");
                  }
                return false;
              }
            (*yynode)->builtin_type_dot_class = __node_238;

          }
        else if (yytoken == Token_THIS)
          {
            if (yytoken != Token_THIS)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_THIS, "this");
                  }
                return false;
              }
            yylex();

            if (yytoken == Token_LPAREN)
              {
                if (yytoken != Token_LPAREN)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_LPAREN, "(");
                      }
                    return false;
                  }
                yylex();

                optional_argument_list_ast *__node_239 = 0;
                if (!parse_optional_argument_list(&__node_239))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_optional_argument_list, "optional_argument_list");
                      }
                    return false;
                  }
                arguments = __node_239;

                if (yytoken != Token_RPAREN)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_RPAREN, ")");
                      }
                    return false;
                  }
                yylex();

                this_call_data_ast *__node_240 = 0;
                if (!parse_this_call_data(&__node_240, 0 /* no type arguments */, arguments ))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_this_call_data, "this_call_data");
                      }
                    return false;
                  }
                (*yynode)->this_call = __node_240;

              }
            else if (true /*epsilon*/)
              {
                this_access_data_ast *__node_241 = 0;
                if (!parse_this_access_data(&__node_241))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_this_access_data, "this_access_data");
                      }
                    return false;
                  }
                (*yynode)->this_access = __node_241;

              }
            else
              {
                return false;
              }
          }
        else if (yytoken == Token_SUPER)
          {
            if (yytoken != Token_SUPER)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_SUPER, "super");
                  }
                return false;
              }
            yylex();

            super_suffix_ast *__node_242 = 0;
            if (!parse_super_suffix(&__node_242))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_super_suffix, "super_suffix");
                  }
                return false;
              }
            super_suffix = __node_242;

            super_access_data_ast *__node_243 = 0;
            if (!parse_super_access_data(&__node_243, 0 /* no type arguments */, super_suffix ))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_super_access_data, "super_access_data");
                  }
                return false;
              }
            (*yynode)->super_access = __node_243;

          }
        else if ((yytoken == Token_LESS_THAN) && ( compatibility_mode() >= java15_compatibility ))
          {
            non_wildcard_type_arguments_ast *__node_244 = 0;
            if (!parse_non_wildcard_type_arguments(&__node_244))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_non_wildcard_type_arguments, "non_wildcard_type_arguments");
                  }
                return false;
              }
            type_arguments = __node_244;

            if (yytoken == Token_SUPER)
              {
                if (yytoken != Token_SUPER)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_SUPER, "super");
                      }
                    return false;
                  }
                yylex();

                super_suffix_ast *__node_245 = 0;
                if (!parse_super_suffix(&__node_245))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_super_suffix, "super_suffix");
                      }
                    return false;
                  }
                super_suffix = __node_245;

                super_access_data_ast *__node_246 = 0;
                if (!parse_super_access_data(&__node_246, type_arguments, super_suffix ))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_super_access_data, "super_access_data");
                      }
                    return false;
                  }
                (*yynode)->super_access = __node_246;

              }
            else if (yytoken == Token_THIS)
              {
                if (yytoken != Token_THIS)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_THIS, "this");
                      }
                    return false;
                  }
                yylex();

                if (yytoken != Token_LPAREN)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_LPAREN, "(");
                      }
                    return false;
                  }
                yylex();

                optional_argument_list_ast *__node_247 = 0;
                if (!parse_optional_argument_list(&__node_247))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_optional_argument_list, "optional_argument_list");
                      }
                    return false;
                  }
                arguments = __node_247;

                if (yytoken != Token_RPAREN)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_RPAREN, ")");
                      }
                    return false;
                  }
                yylex();

                this_call_data_ast *__node_248 = 0;
                if (!parse_this_call_data(&__node_248, type_arguments, arguments ))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_this_call_data, "this_call_data");
                      }
                    return false;
                  }
                (*yynode)->this_call = __node_248;

              }
            else if (yytoken == Token_IDENTIFIER)
              {
                identifier_ast *__node_249 = 0;
                if (!parse_identifier(&__node_249))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                      }
                    return false;
                  }
                identifier = __node_249;

                if (yytoken != Token_LPAREN)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_LPAREN, "(");
                      }
                    return false;
                  }
                yylex();

                optional_argument_list_ast *__node_250 = 0;
                if (!parse_optional_argument_list(&__node_250))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_optional_argument_list, "optional_argument_list");
                      }
                    return false;
                  }
                arguments = __node_250;

                if (yytoken != Token_RPAREN)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_RPAREN, ")");
                      }
                    return false;
                  }
                yylex();

                method_call_data_ast *__node_251 = 0;
                if (!parse_method_call_data(&__node_251, type_arguments, identifier, arguments ))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_method_call_data, "method_call_data");
                      }
                    return false;
                  }
                (*yynode)->method_call = __node_251;

              }
            else
              {
                return false;
              }
          }
        else if (yytoken == Token_IDENTIFIER)
          {
            bool block_errors_19 = block_errors(true);
            std::size_t try_start_token_19 = token_stream->index() - 1;
            parser_state *try_start_state_19 = copy_current_state();
            {
              array_type_dot_class_ast *__node_252 = 0;
              if (!parse_array_type_dot_class(&__node_252))
                {
                  goto __catch_19;
                }
              (*yynode)->array_type_dot_class = __node_252;

            }
            block_errors(block_errors_19);
            if (try_start_state_19)
              delete try_start_state_19;

            if (false) // the only way to enter here is using goto
              {
              __catch_19:
                if (try_start_state_19)
                  {
                    restore_state(try_start_state_19);
                    delete try_start_state_19;
                  }
                block_errors(block_errors_19);
                rewind(try_start_token_19);

                identifier_ast *__node_253 = 0;
                if (!parse_identifier(&__node_253))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                      }
                    return false;
                  }
                identifier = __node_253;

                if (yytoken == Token_LPAREN)
                  {
                    if (yytoken != Token_LPAREN)
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_token(yytoken, Token_LPAREN, "(");
                          }
                        return false;
                      }
                    yylex();

                    optional_argument_list_ast *__node_254 = 0;
                    if (!parse_optional_argument_list(&__node_254))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_optional_argument_list, "optional_argument_list");
                          }
                        return false;
                      }
                    arguments = __node_254;

                    if (yytoken != Token_RPAREN)
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_token(yytoken, Token_RPAREN, ")");
                          }
                        return false;
                      }
                    yylex();

                    method_call_data_ast *__node_255 = 0;
                    if (!parse_method_call_data(&__node_255,
                                                0 /* no type arguments */, identifier, arguments
                                               ))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_method_call_data, "method_call_data");
                          }
                        return false;
                      }
                    (*yynode)->method_call = __node_255;

                  }
                else if (true /*epsilon*/)
                  {
                    simple_name_access_data_ast *__node_256 = 0;
                    if (!parse_simple_name_access_data(&__node_256, identifier ))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_simple_name_access_data, "simple_name_access_data");
                          }
                        return false;
                      }
                    (*yynode)->simple_name_access = __node_256;

                  }
                else
                  {
                    return false;
                  }
              }

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

  bool parser::parse_primary_expression(primary_expression_ast **yynode)
  {
    *yynode = create<primary_expression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOOLEAN
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_SHORT
        || yytoken == Token_SUPER
        || yytoken == Token_THIS
        || yytoken == Token_VOID
        || yytoken == Token_LPAREN
        || yytoken == Token_LESS_THAN
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_FLOATING_POINT_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        primary_atom_ast *__node_257 = 0;
        if (!parse_primary_atom(&__node_257))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_primary_atom, "primary_atom");
              }
            return false;
          }
        (*yynode)->primary_atom = __node_257;

        while (yytoken == Token_LBRACKET
               || yytoken == Token_DOT)
          {
            primary_selector_ast *__node_258 = 0;
            if (!parse_primary_selector(&__node_258))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_primary_selector, "primary_selector");
                  }
                return false;
              }
            (*yynode)->selector_sequence = snoc((*yynode)->selector_sequence, __node_258, memory_pool);

          }
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_primary_selector(primary_selector_ast **yynode)
  {
    *yynode = create<primary_selector_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_LBRACKET
        || yytoken == Token_DOT)
      {
        identifier_ast *identifier = 0;

        non_wildcard_type_arguments_ast *type_arguments = 0;

        super_suffix_ast *super_suffix = 0;

        optional_argument_list_ast *arguments = 0;

        if (yytoken == Token_DOT)
          {
            if (yytoken != Token_DOT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_DOT, ".");
                  }
                return false;
              }
            yylex();

            if (yytoken == Token_CLASS)
              {
                if (yytoken != Token_CLASS)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_CLASS, "class");
                      }
                    return false;
                  }
                yylex();

                class_access_data_ast *__node_259 = 0;
                if (!parse_class_access_data(&__node_259))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_class_access_data, "class_access_data");
                      }
                    return false;
                  }
                (*yynode)->class_access = __node_259;

              }
            else if (yytoken == Token_THIS)
              {
                if (yytoken != Token_THIS)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_THIS, "this");
                      }
                    return false;
                  }
                yylex();

                this_access_data_ast *__node_260 = 0;
                if (!parse_this_access_data(&__node_260))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_this_access_data, "this_access_data");
                      }
                    return false;
                  }
                (*yynode)->this_access = __node_260;

              }
            else if (yytoken == Token_NEW)
              {
                new_expression_ast *__node_261 = 0;
                if (!parse_new_expression(&__node_261))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_new_expression, "new_expression");
                      }
                    return false;
                  }
                (*yynode)->new_expression = __node_261;

              }
            else if ((yytoken == Token_IDENTIFIER) && ( LA(2).kind != Token_LPAREN ))
              {
                identifier_ast *__node_262 = 0;
                if (!parse_identifier(&__node_262))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                      }
                    return false;
                  }
                identifier = __node_262;

                simple_name_access_data_ast *__node_263 = 0;
                if (!parse_simple_name_access_data(&__node_263, identifier ))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_simple_name_access_data, "simple_name_access_data");
                      }
                    return false;
                  }
                (*yynode)->simple_name_access = __node_263;

              }
            else if (yytoken == Token_SUPER
                     || yytoken == Token_LESS_THAN
                     || yytoken == Token_IDENTIFIER)
              {
                if ((yytoken == Token_LESS_THAN) && ( compatibility_mode() >= java15_compatibility ))
                  {
                    non_wildcard_type_arguments_ast *__node_264 = 0;
                    if (!parse_non_wildcard_type_arguments(&__node_264))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_non_wildcard_type_arguments, "non_wildcard_type_arguments");
                          }
                        return false;
                      }
                    type_arguments = __node_264;

                  }
                else if (true /*epsilon*/)
                {}
                else
                  {
                    return false;
                  }
                if (yytoken == Token_SUPER)
                  {
                    if (yytoken != Token_SUPER)
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_token(yytoken, Token_SUPER, "super");
                          }
                        return false;
                      }
                    yylex();

                    super_suffix_ast *__node_265 = 0;
                    if (!parse_super_suffix(&__node_265))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_super_suffix, "super_suffix");
                          }
                        return false;
                      }
                    super_suffix = __node_265;

                    super_access_data_ast *__node_266 = 0;
                    if (!parse_super_access_data(&__node_266, type_arguments, super_suffix ))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_super_access_data, "super_access_data");
                          }
                        return false;
                      }
                    (*yynode)->super_access = __node_266;

                  }
                else if (yytoken == Token_IDENTIFIER)
                  {
                    identifier_ast *__node_267 = 0;
                    if (!parse_identifier(&__node_267))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                          }
                        return false;
                      }
                    identifier = __node_267;

                    if (yytoken != Token_LPAREN)
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_token(yytoken, Token_LPAREN, "(");
                          }
                        return false;
                      }
                    yylex();

                    optional_argument_list_ast *__node_268 = 0;
                    if (!parse_optional_argument_list(&__node_268))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_optional_argument_list, "optional_argument_list");
                          }
                        return false;
                      }
                    arguments = __node_268;

                    if (yytoken != Token_RPAREN)
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_token(yytoken, Token_RPAREN, ")");
                          }
                        return false;
                      }
                    yylex();

                    method_call_data_ast *__node_269 = 0;
                    if (!parse_method_call_data(&__node_269, type_arguments, identifier, arguments ))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_method_call_data, "method_call_data");
                          }
                        return false;
                      }
                    (*yynode)->method_call = __node_269;

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
          }
        else if (yytoken == Token_LBRACKET)
          {
            array_access_ast *__node_270 = 0;
            if (!parse_array_access(&__node_270))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_array_access, "array_access");
                  }
                return false;
              }
            (*yynode)->array_access = __node_270;

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

  bool parser::parse_qualified_identifier(qualified_identifier_ast **yynode)
  {
    *yynode = create<qualified_identifier_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_IDENTIFIER)
      {
        identifier_ast *__node_271 = 0;
        if (!parse_identifier(&__node_271))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_identifier, "identifier");
              }
            return false;
          }
        (*yynode)->name_sequence = snoc((*yynode)->name_sequence, __node_271, memory_pool);

        while (yytoken == Token_DOT)
          {
            if (yytoken != Token_DOT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_DOT, ".");
                  }
                return false;
              }
            yylex();

            identifier_ast *__node_272 = 0;
            if (!parse_identifier(&__node_272))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                  }
                return false;
              }
            (*yynode)->name_sequence = snoc((*yynode)->name_sequence, __node_272, memory_pool);

          }
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_qualified_identifier_with_optional_star(qualified_identifier_with_optional_star_ast **yynode)
  {
    *yynode = create<qualified_identifier_with_optional_star_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_IDENTIFIER)
      {
        identifier_ast *__node_273 = 0;
        if (!parse_identifier(&__node_273))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_identifier, "identifier");
              }
            return false;
          }
        (*yynode)->name_sequence = snoc((*yynode)->name_sequence, __node_273, memory_pool);

        (*yynode)->has_star = false;
        while (yytoken == Token_DOT)
          {
            if (yytoken != Token_DOT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_DOT, ".");
                  }
                return false;
              }
            yylex();

            if (yytoken == Token_IDENTIFIER)
              {
                identifier_ast *__node_274 = 0;
                if (!parse_identifier(&__node_274))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                      }
                    return false;
                  }
                (*yynode)->name_sequence = snoc((*yynode)->name_sequence, __node_274, memory_pool);

              }
            else if (yytoken == Token_STAR)
              {
                if (yytoken != Token_STAR)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_STAR, "*");
                      }
                    return false;
                  }
                yylex();

                (*yynode)->has_star = true;
                break;
              }
            else
              {
                return false;
              }
          }
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_relational_expression(relational_expression_ast **yynode)
  {
    *yynode = create<relational_expression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOOLEAN
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_SHORT
        || yytoken == Token_SUPER
        || yytoken == Token_THIS
        || yytoken == Token_VOID
        || yytoken == Token_LPAREN
        || yytoken == Token_LESS_THAN
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_FLOATING_POINT_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        shift_expression_ast *__node_275 = 0;
        if (!parse_shift_expression(&__node_275))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_shift_expression, "shift_expression");
              }
            return false;
          }
        (*yynode)->expression = __node_275;

        if (yytoken == Token_LESS_THAN
            || yytoken == Token_GREATER_THAN
            || yytoken == Token_LESS_EQUAL
            || yytoken == Token_GREATER_EQUAL)
          {
            do
              {
                relational_expression_rest_ast *__node_276 = 0;
                if (!parse_relational_expression_rest(&__node_276))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_relational_expression_rest, "relational_expression_rest");
                      }
                    return false;
                  }
                (*yynode)->additional_expression_sequence = snoc((*yynode)->additional_expression_sequence, __node_276, memory_pool);

              }
            while (yytoken == Token_LESS_THAN
                   || yytoken == Token_GREATER_THAN
                   || yytoken == Token_LESS_EQUAL
                   || yytoken == Token_GREATER_EQUAL);
          }
        else if (yytoken == Token_INSTANCEOF)
          {
            if (yytoken != Token_INSTANCEOF)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_INSTANCEOF, "instanceof");
                  }
                return false;
              }
            yylex();

            type_ast *__node_277 = 0;
            if (!parse_type(&__node_277))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_type, "type");
                  }
                return false;
              }
            (*yynode)->instanceof_type = __node_277;

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

  bool parser::parse_relational_expression_rest(relational_expression_rest_ast **yynode)
  {
    *yynode = create<relational_expression_rest_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_LESS_THAN
        || yytoken == Token_GREATER_THAN
        || yytoken == Token_LESS_EQUAL
        || yytoken == Token_GREATER_EQUAL)
      {
        if (yytoken == Token_LESS_THAN)
          {
            if (yytoken != Token_LESS_THAN)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LESS_THAN, "<");
                  }
                return false;
              }
            yylex();

            (*yynode)->relational_operator = relational_expression_rest::op_less_than;
          }
        else if (yytoken == Token_GREATER_THAN)
          {
            if (yytoken != Token_GREATER_THAN)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_GREATER_THAN, ">");
                  }
                return false;
              }
            yylex();

            (*yynode)->relational_operator = relational_expression_rest::op_greater_than;
          }
        else if (yytoken == Token_LESS_EQUAL)
          {
            if (yytoken != Token_LESS_EQUAL)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LESS_EQUAL, "<=");
                  }
                return false;
              }
            yylex();

            (*yynode)->relational_operator = relational_expression_rest::op_less_equal;
          }
        else if (yytoken == Token_GREATER_EQUAL)
          {
            if (yytoken != Token_GREATER_EQUAL)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_GREATER_EQUAL, ">=");
                  }
                return false;
              }
            yylex();

            (*yynode)->relational_operator = relational_expression_rest::op_greater_equal;
          }
        else
          {
            return false;
          }
        shift_expression_ast *__node_278 = 0;
        if (!parse_shift_expression(&__node_278))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_shift_expression, "shift_expression");
              }
            return false;
          }
        (*yynode)->expression = __node_278;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_return_statement(return_statement_ast **yynode)
                           {
                             *yynode = create<return_statement_ast>();

                             (*yynode)->start_token = token_stream->index() - 1;

                             if (yytoken == Token_RETURN)
                               {
                                 if (yytoken != Token_RETURN)
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_token(yytoken, Token_RETURN, "return");
                                       }
                                     return false;
                                   }
                                 yylex();

                                 if (yytoken == Token_BOOLEAN
                                     || yytoken == Token_BYTE
                                     || yytoken == Token_CHAR
                                     || yytoken == Token_DOUBLE
                                     || yytoken == Token_FLOAT
                                     || yytoken == Token_INT
                                     || yytoken == Token_LONG
                                     || yytoken == Token_NEW
                                     || yytoken == Token_SHORT
                                     || yytoken == Token_SUPER
                                     || yytoken == Token_THIS
                                     || yytoken == Token_VOID
                                     || yytoken == Token_LPAREN
                                     || yytoken == Token_LESS_THAN
                                     || yytoken == Token_BANG
                                     || yytoken == Token_TILDE
                                     || yytoken == Token_INCREMENT
                                     || yytoken == Token_DECREMENT
                                     || yytoken == Token_PLUS
                                     || yytoken == Token_MINUS
                                     || yytoken == Token_TRUE
                                     || yytoken == Token_FALSE
                                     || yytoken == Token_NULL
                                     || yytoken == Token_INTEGER_LITERAL
                                     || yytoken == Token_FLOATING_POINT_LITERAL
                                     || yytoken == Token_CHARACTER_LITERAL
                                     || yytoken == Token_STRING_LITERAL
                                     || yytoken == Token_IDENTIFIER)
                                   {
                                     expression_ast *__node_279 = 0;
                                     if (!parse_expression(&__node_279))
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_symbol(ast_node::Kind_expression, "expression");
                                           }
                                         return false;
                                       }
                                     (*yynode)->return_expression = __node_279;

                                   }
                                 else if (true /*epsilon*/)
                                 {}
                                 else
                                   {
                                     return false;
                                   }
                                 if (yytoken != Token_SEMICOLON)
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_token(yytoken, Token_SEMICOLON, ";");
                                       }
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

                           bool parser::parse_shift_expression(shift_expression_ast **yynode)
                           {
                             *yynode = create<shift_expression_ast>();

                             (*yynode)->start_token = token_stream->index() - 1;

                             if (yytoken == Token_BOOLEAN
                                 || yytoken == Token_BYTE
                                 || yytoken == Token_CHAR
                                 || yytoken == Token_DOUBLE
                                 || yytoken == Token_FLOAT
                                 || yytoken == Token_INT
                                 || yytoken == Token_LONG
                                 || yytoken == Token_NEW
                                 || yytoken == Token_SHORT
                                 || yytoken == Token_SUPER
                                 || yytoken == Token_THIS
                                 || yytoken == Token_VOID
                                 || yytoken == Token_LPAREN
                                 || yytoken == Token_LESS_THAN
                                 || yytoken == Token_BANG
                                 || yytoken == Token_TILDE
                                 || yytoken == Token_INCREMENT
                                 || yytoken == Token_DECREMENT
                                 || yytoken == Token_PLUS
                                 || yytoken == Token_MINUS
                                 || yytoken == Token_TRUE
                                 || yytoken == Token_FALSE
                                 || yytoken == Token_NULL
                                 || yytoken == Token_INTEGER_LITERAL
                                 || yytoken == Token_FLOATING_POINT_LITERAL
                                 || yytoken == Token_CHARACTER_LITERAL
                                 || yytoken == Token_STRING_LITERAL
                                 || yytoken == Token_IDENTIFIER)
                               {
                                 additive_expression_ast *__node_280 = 0;
                                 if (!parse_additive_expression(&__node_280))
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_symbol(ast_node::Kind_additive_expression, "additive_expression");
                                       }
                                     return false;
                                   }
                                 (*yynode)->expression = __node_280;

                                 while (yytoken == Token_LSHIFT
                                        || yytoken == Token_SIGNED_RSHIFT
                                        || yytoken == Token_UNSIGNED_RSHIFT)
                                   {
                                     shift_expression_rest_ast *__node_281 = 0;
                                     if (!parse_shift_expression_rest(&__node_281))
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_symbol(ast_node::Kind_shift_expression_rest, "shift_expression_rest");
                                           }
                                         return false;
                                       }
                                     (*yynode)->additional_expression_sequence = snoc((*yynode)->additional_expression_sequence, __node_281, memory_pool);

                                   }
                               }
                             else
                               {
                                 return false;
                               }

                             (*yynode)->end_token = token_stream->index() - 1;

                             return true;
                           }

                           bool parser::parse_shift_expression_rest(shift_expression_rest_ast **yynode)
                           {
                             *yynode = create<shift_expression_rest_ast>();

                             (*yynode)->start_token = token_stream->index() - 1;

                             if (yytoken == Token_LSHIFT
                                 || yytoken == Token_SIGNED_RSHIFT
                                 || yytoken == Token_UNSIGNED_RSHIFT)
                               {
                                 if (yytoken == Token_LSHIFT)
                                   {
                                     if (yytoken != Token_LSHIFT)
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_token(yytoken, Token_LSHIFT, "<<");
                                           }
                                         return false;
                                       }
                                     yylex();

                                     (*yynode)->shift_operator = shift_expression_rest::op_lshift;
                                   }
                                 else if (yytoken == Token_SIGNED_RSHIFT)
                                   {
                                     if (yytoken != Token_SIGNED_RSHIFT)
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_token(yytoken, Token_SIGNED_RSHIFT, ">>");
                                           }
                                         return false;
                                       }
                                     yylex();

                                     (*yynode)->shift_operator = shift_expression_rest::op_signed_rshift;
                                   }
                                 else if (yytoken == Token_UNSIGNED_RSHIFT)
                                   {
                                     if (yytoken != Token_UNSIGNED_RSHIFT)
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_token(yytoken, Token_UNSIGNED_RSHIFT, ">>>");
                                           }
                                         return false;
                                       }
                                     yylex();

                                     (*yynode)->shift_operator = shift_expression_rest::op_unsigned_rshift;
                                   }
                                 else
                                   {
                                     return false;
                                   }
                                 additive_expression_ast *__node_282 = 0;
                                 if (!parse_additive_expression(&__node_282))
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_symbol(ast_node::Kind_additive_expression, "additive_expression");
                                       }
                                     return false;
                                   }
                                 (*yynode)->expression = __node_282;

                               }
                             else
                               {
                                 return false;
                               }

                             (*yynode)->end_token = token_stream->index() - 1;

                             return true;
                           }

                           bool parser::parse_simple_name_access_data(simple_name_access_data_ast **yynode, identifier_ast *name)
                           {
                             *yynode = create<simple_name_access_data_ast>();

                             (*yynode)->start_token = token_stream->index() - 1;

                             (*yynode)->name = name;
                             if (name && name->start_token < (*yynode)->start_token)
                               (*yynode)->start_token = name->start_token;

                             if (true /*epsilon*/ || yytoken == Token_INSTANCEOF
                                 || yytoken == Token_RPAREN
                                 || yytoken == Token_RBRACE
                                 || yytoken == Token_LBRACKET
                                 || yytoken == Token_RBRACKET
                                 || yytoken == Token_SEMICOLON
                                 || yytoken == Token_COMMA
                                 || yytoken == Token_DOT
                                 || yytoken == Token_ASSIGN
                                 || yytoken == Token_LESS_THAN
                                 || yytoken == Token_GREATER_THAN
                                 || yytoken == Token_QUESTION
                                 || yytoken == Token_COLON
                                 || yytoken == Token_EQUAL
                                 || yytoken == Token_LESS_EQUAL
                                 || yytoken == Token_GREATER_EQUAL
                                 || yytoken == Token_NOT_EQUAL
                                 || yytoken == Token_LOG_AND
                                 || yytoken == Token_LOG_OR
                                 || yytoken == Token_INCREMENT
                                 || yytoken == Token_DECREMENT
                                 || yytoken == Token_PLUS
                                 || yytoken == Token_MINUS
                                 || yytoken == Token_STAR
                                 || yytoken == Token_SLASH
                                 || yytoken == Token_BIT_AND
                                 || yytoken == Token_BIT_OR
                                 || yytoken == Token_BIT_XOR
                                 || yytoken == Token_REMAINDER
                                 || yytoken == Token_LSHIFT
                                 || yytoken == Token_SIGNED_RSHIFT
                                 || yytoken == Token_UNSIGNED_RSHIFT
                                 || yytoken == Token_PLUS_ASSIGN
                                 || yytoken == Token_MINUS_ASSIGN
                                 || yytoken == Token_STAR_ASSIGN
                                 || yytoken == Token_SLASH_ASSIGN
                                 || yytoken == Token_BIT_AND_ASSIGN
                                 || yytoken == Token_BIT_OR_ASSIGN
                                 || yytoken == Token_BIT_XOR_ASSIGN
                                 || yytoken == Token_REMAINDER_ASSIGN
                                 || yytoken == Token_LSHIFT_ASSIGN
                                 || yytoken == Token_SIGNED_RSHIFT_ASSIGN
                                 || yytoken == Token_UNSIGNED_RSHIFT_ASSIGN)
                             {}
                             else
                               {
                                 return false;
                               }

                             (*yynode)->end_token = token_stream->index() - 1;

                             return true;
                           }

                           bool parser::parse_statement_expression(statement_expression_ast **yynode)
                           {
                             *yynode = create<statement_expression_ast>();

                             (*yynode)->start_token = token_stream->index() - 1;

                             if (yytoken == Token_BOOLEAN
                                 || yytoken == Token_BYTE
                                 || yytoken == Token_CHAR
                                 || yytoken == Token_DOUBLE
                                 || yytoken == Token_FLOAT
                                 || yytoken == Token_INT
                                 || yytoken == Token_LONG
                                 || yytoken == Token_NEW
                                 || yytoken == Token_SHORT
                                 || yytoken == Token_SUPER
                                 || yytoken == Token_THIS
                                 || yytoken == Token_VOID
                                 || yytoken == Token_LPAREN
                                 || yytoken == Token_LESS_THAN
                                 || yytoken == Token_BANG
                                 || yytoken == Token_TILDE
                                 || yytoken == Token_INCREMENT
                                 || yytoken == Token_DECREMENT
                                 || yytoken == Token_PLUS
                                 || yytoken == Token_MINUS
                                 || yytoken == Token_TRUE
                                 || yytoken == Token_FALSE
                                 || yytoken == Token_NULL
                                 || yytoken == Token_INTEGER_LITERAL
                                 || yytoken == Token_FLOATING_POINT_LITERAL
                                 || yytoken == Token_CHARACTER_LITERAL
                                 || yytoken == Token_STRING_LITERAL
                                 || yytoken == Token_IDENTIFIER)
                               {
                                 expression_ast *__node_283 = 0;
                                 if (!parse_expression(&__node_283))
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_symbol(ast_node::Kind_expression, "expression");
                                       }
                                     return false;
                                   }
                                 (*yynode)->expression = __node_283;

                               }
                             else
                               {
                                 return false;
                               }

                             (*yynode)->end_token = token_stream->index() - 1;

                             return true;
                           }

                           bool parser::parse_super_access_data(super_access_data_ast **yynode, non_wildcard_type_arguments_ast *type_arguments, super_suffix_ast *super_suffix)
                           {
                             *yynode = create<super_access_data_ast>();

                             (*yynode)->start_token = token_stream->index() - 1;

                             (*yynode)->type_arguments = type_arguments;
                             if (type_arguments && type_arguments->start_token < (*yynode)->start_token)
                               (*yynode)->start_token = type_arguments->start_token;

                             (*yynode)->super_suffix = super_suffix;
                             if (super_suffix && super_suffix->start_token < (*yynode)->start_token)
                               (*yynode)->start_token = super_suffix->start_token;

                             if (true /*epsilon*/ || yytoken == Token_INSTANCEOF
                                 || yytoken == Token_RPAREN
                                 || yytoken == Token_RBRACE
                                 || yytoken == Token_LBRACKET
                                 || yytoken == Token_RBRACKET
                                 || yytoken == Token_SEMICOLON
                                 || yytoken == Token_COMMA
                                 || yytoken == Token_DOT
                                 || yytoken == Token_ASSIGN
                                 || yytoken == Token_LESS_THAN
                                 || yytoken == Token_GREATER_THAN
                                 || yytoken == Token_QUESTION
                                 || yytoken == Token_COLON
                                 || yytoken == Token_EQUAL
                                 || yytoken == Token_LESS_EQUAL
                                 || yytoken == Token_GREATER_EQUAL
                                 || yytoken == Token_NOT_EQUAL
                                 || yytoken == Token_LOG_AND
                                 || yytoken == Token_LOG_OR
                                 || yytoken == Token_INCREMENT
                                 || yytoken == Token_DECREMENT
                                 || yytoken == Token_PLUS
                                 || yytoken == Token_MINUS
                                 || yytoken == Token_STAR
                                 || yytoken == Token_SLASH
                                 || yytoken == Token_BIT_AND
                                 || yytoken == Token_BIT_OR
                                 || yytoken == Token_BIT_XOR
                                 || yytoken == Token_REMAINDER
                                 || yytoken == Token_LSHIFT
                                 || yytoken == Token_SIGNED_RSHIFT
                                 || yytoken == Token_UNSIGNED_RSHIFT
                                 || yytoken == Token_PLUS_ASSIGN
                                 || yytoken == Token_MINUS_ASSIGN
                                 || yytoken == Token_STAR_ASSIGN
                                 || yytoken == Token_SLASH_ASSIGN
                                 || yytoken == Token_BIT_AND_ASSIGN
                                 || yytoken == Token_BIT_OR_ASSIGN
                                 || yytoken == Token_BIT_XOR_ASSIGN
                                 || yytoken == Token_REMAINDER_ASSIGN
                                 || yytoken == Token_LSHIFT_ASSIGN
                                 || yytoken == Token_SIGNED_RSHIFT_ASSIGN
                                 || yytoken == Token_UNSIGNED_RSHIFT_ASSIGN)
                             {}
                             else
                               {
                                 return false;
                               }

                             (*yynode)->end_token = token_stream->index() - 1;

                             return true;
                           }

                           bool parser::parse_super_suffix(super_suffix_ast **yynode)
                           {
                             *yynode = create<super_suffix_ast>();

                             (*yynode)->start_token = token_stream->index() - 1;

                             if (yytoken == Token_LPAREN
                                 || yytoken == Token_DOT)
                               {
                                 identifier_ast *identifier = 0;

                                 non_wildcard_type_arguments_ast *type_arguments = 0;

                                 optional_argument_list_ast *arguments = 0;

                                 if (yytoken == Token_LPAREN)
                                   {
                                     if (yytoken != Token_LPAREN)
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_token(yytoken, Token_LPAREN, "(");
                                           }
                                         return false;
                                       }
                                     yylex();

                                     optional_argument_list_ast *__node_284 = 0;
                                     if (!parse_optional_argument_list(&__node_284))
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_symbol(ast_node::Kind_optional_argument_list, "optional_argument_list");
                                           }
                                         return false;
                                       }
                                     (*yynode)->constructor_arguments = __node_284;

                                     if (yytoken != Token_RPAREN)
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_token(yytoken, Token_RPAREN, ")");
                                           }
                                         return false;
                                       }
                                     yylex();

                                   }
                                 else if (yytoken == Token_DOT)
                                   {
                                     if (yytoken != Token_DOT)
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_token(yytoken, Token_DOT, ".");
                                           }
                                         return false;
                                       }
                                     yylex();

                                     if ((yytoken == Token_IDENTIFIER) && ( LA(2).kind != Token_LPAREN ))
                                       {
                                         identifier_ast *__node_285 = 0;
                                         if (!parse_identifier(&__node_285))
                                           {
                                             if (!yy_block_errors)
                                               {
                                                 yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                                               }
                                             return false;
                                           }
                                         identifier = __node_285;

                                         simple_name_access_data_ast *__node_286 = 0;
                                         if (!parse_simple_name_access_data(&__node_286, identifier ))
                                           {
                                             if (!yy_block_errors)
                                               {
                                                 yy_expected_symbol(ast_node::Kind_simple_name_access_data, "simple_name_access_data");
                                               }
                                             return false;
                                           }
                                         (*yynode)->simple_name_access = __node_286;

                                       }
                                     else if (yytoken == Token_LESS_THAN
                                              || yytoken == Token_IDENTIFIER)
                                       {
                                         if ((yytoken == Token_LESS_THAN) && ( compatibility_mode() >= java15_compatibility ))
                                           {
                                             non_wildcard_type_arguments_ast *__node_287 = 0;
                                             if (!parse_non_wildcard_type_arguments(&__node_287))
                                               {
                                                 if (!yy_block_errors)
                                                   {
                                                     yy_expected_symbol(ast_node::Kind_non_wildcard_type_arguments, "non_wildcard_type_arguments");
                                                   }
                                                 return false;
                                               }
                                             type_arguments = __node_287;

                                           }
                                         else if (true /*epsilon*/)
                                         {}
                                         else
                                           {
                                             return false;
                                           }
                                         identifier_ast *__node_288 = 0;
                                         if (!parse_identifier(&__node_288))
                                           {
                                             if (!yy_block_errors)
                                               {
                                                 yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                                               }
                                             return false;
                                           }
                                         identifier = __node_288;

                                         if (yytoken != Token_LPAREN)
                                           {
                                             if (!yy_block_errors)
                                               {
                                                 yy_expected_token(yytoken, Token_LPAREN, "(");
                                               }
                                             return false;
                                           }
                                         yylex();

                                         optional_argument_list_ast *__node_289 = 0;
                                         if (!parse_optional_argument_list(&__node_289))
                                           {
                                             if (!yy_block_errors)
                                               {
                                                 yy_expected_symbol(ast_node::Kind_optional_argument_list, "optional_argument_list");
                                               }
                                             return false;
                                           }
                                         arguments = __node_289;

                                         if (yytoken != Token_RPAREN)
                                           {
                                             if (!yy_block_errors)
                                               {
                                                 yy_expected_token(yytoken, Token_RPAREN, ")");
                                               }
                                             return false;
                                           }
                                         yylex();

                                         method_call_data_ast *__node_290 = 0;
                                         if (!parse_method_call_data(&__node_290, type_arguments, identifier, arguments ))
                                           {
                                             if (!yy_block_errors)
                                               {
                                                 yy_expected_symbol(ast_node::Kind_method_call_data, "method_call_data");
                                               }
                                             return false;
                                           }
                                         (*yynode)->method_call = __node_290;

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
                               }
                             else
                               {
                                 return false;
                               }

                             (*yynode)->end_token = token_stream->index() - 1;

                             return true;
                           }

                           bool parser::parse_switch_label(switch_label_ast **yynode)
                           {
                             *yynode = create<switch_label_ast>();

                             (*yynode)->start_token = token_stream->index() - 1;

                             if (yytoken == Token_CASE
                                 || yytoken == Token_DEFAULT)
                               {
                                 if (yytoken == Token_CASE)
                                   {
                                     if (yytoken != Token_CASE)
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_token(yytoken, Token_CASE, "case");
                                           }
                                         return false;
                                       }
                                     yylex();

                                     expression_ast *__node_291 = 0;
                                     if (!parse_expression(&__node_291))
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_symbol(ast_node::Kind_expression, "expression");
                                           }
                                         return false;
                                       }
                                     (*yynode)->case_expression = __node_291;

                                     (*yynode)->branch_type = switch_label::case_branch;
                                   }
                                 else if (yytoken == Token_DEFAULT)
                                   {
                                     if (yytoken != Token_DEFAULT)
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_token(yytoken, Token_DEFAULT, "default");
                                           }
                                         return false;
                                       }
                                     yylex();

                                     (*yynode)->branch_type = switch_label::default_branch;
                                   }
                                 else
                                   {
                                     return false;
                                   }
                                 if (yytoken != Token_COLON)
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_token(yytoken, Token_COLON, ":");
                                       }
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

                           bool parser::parse_switch_section(switch_section_ast **yynode)
                           {
                             *yynode = create<switch_section_ast>();

                             (*yynode)->start_token = token_stream->index() - 1;

                             if (yytoken == Token_CASE
                                 || yytoken == Token_DEFAULT)
                               {
                                 do
                                   {
                                     switch_label_ast *__node_292 = 0;
                                     if (!parse_switch_label(&__node_292))
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_symbol(ast_node::Kind_switch_label, "switch_label");
                                           }
                                         return false;
                                       }
                                     (*yynode)->label_sequence = snoc((*yynode)->label_sequence, __node_292, memory_pool);

                                   }
                                 while (yytoken == Token_CASE
                                        || yytoken == Token_DEFAULT);
                                 while (yytoken == Token_ABSTRACT
                                        || yytoken == Token_ASSERT
                                        || yytoken == Token_BOOLEAN
                                        || yytoken == Token_BREAK
                                        || yytoken == Token_BYTE
                                        || yytoken == Token_CHAR
                                        || yytoken == Token_CLASS
                                        || yytoken == Token_CONTINUE
                                        || yytoken == Token_DO
                                        || yytoken == Token_DOUBLE
                                        || yytoken == Token_ENUM
                                        || yytoken == Token_FINAL
                                        || yytoken == Token_FLOAT
                                        || yytoken == Token_FOR
                                        || yytoken == Token_IF
                                        || yytoken == Token_INT
                                        || yytoken == Token_INTERFACE
                                        || yytoken == Token_LONG
                                        || yytoken == Token_NATIVE
                                        || yytoken == Token_NEW
                                        || yytoken == Token_PRIVATE
                                        || yytoken == Token_PROTECTED
                                        || yytoken == Token_PUBLIC
                                        || yytoken == Token_RETURN
                                        || yytoken == Token_SHORT
                                        || yytoken == Token_STATIC
                                        || yytoken == Token_STRICTFP
                                        || yytoken == Token_SUPER
                                        || yytoken == Token_SWITCH
                                        || yytoken == Token_SYNCHRONIZED
                                        || yytoken == Token_THIS
                                        || yytoken == Token_THROW
                                        || yytoken == Token_TRANSIENT
                                        || yytoken == Token_TRY
                                        || yytoken == Token_VOID
                                        || yytoken == Token_VOLATILE
                                        || yytoken == Token_WHILE
                                        || yytoken == Token_LPAREN
                                        || yytoken == Token_LBRACE
                                        || yytoken == Token_SEMICOLON
                                        || yytoken == Token_AT
                                        || yytoken == Token_LESS_THAN
                                        || yytoken == Token_BANG
                                        || yytoken == Token_TILDE
                                        || yytoken == Token_INCREMENT
                                        || yytoken == Token_DECREMENT
                                        || yytoken == Token_PLUS
                                        || yytoken == Token_MINUS
                                        || yytoken == Token_TRUE
                                        || yytoken == Token_FALSE
                                        || yytoken == Token_NULL
                                        || yytoken == Token_INTEGER_LITERAL
                                        || yytoken == Token_FLOATING_POINT_LITERAL
                                        || yytoken == Token_CHARACTER_LITERAL
                                        || yytoken == Token_STRING_LITERAL
                                        || yytoken == Token_IDENTIFIER)
                                   {
                                     std::size_t try_start_token_20 = token_stream->index() - 1;
                                     parser_state *try_start_state_20 = copy_current_state();
                                     {
                                       block_statement_ast *__node_293 = 0;
                                       if (!parse_block_statement(&__node_293))
                                         {
                                           goto __catch_20;
                                         }
                                       (*yynode)->statement_sequence = snoc((*yynode)->statement_sequence, __node_293, memory_pool);

                                     }
                                     if (try_start_state_20)
                                       delete try_start_state_20;

                                     if (false) // the only way to enter here is using goto
                                       {
                                       __catch_20:
                                         if (try_start_state_20)
                                           {
                                             restore_state(try_start_state_20);
                                             delete try_start_state_20;
                                           }
                                         if (try_start_token_20 == token_stream->index() - 1)
                                           yylex();

                                         while (yytoken != Token_EOF
                                                && yytoken != Token_ABSTRACT
                                                && yytoken != Token_ASSERT
                                                && yytoken != Token_BOOLEAN
                                                && yytoken != Token_BREAK
                                                && yytoken != Token_BYTE
                                                && yytoken != Token_CASE
                                                && yytoken != Token_CHAR
                                                && yytoken != Token_CLASS
                                                && yytoken != Token_CONTINUE
                                                && yytoken != Token_DEFAULT
                                                && yytoken != Token_DO
                                                && yytoken != Token_DOUBLE
                                                && yytoken != Token_ENUM
                                                && yytoken != Token_FINAL
                                                && yytoken != Token_FLOAT
                                                && yytoken != Token_FOR
                                                && yytoken != Token_IF
                                                && yytoken != Token_INT
                                                && yytoken != Token_INTERFACE
                                                && yytoken != Token_LONG
                                                && yytoken != Token_NATIVE
                                                && yytoken != Token_NEW
                                                && yytoken != Token_PRIVATE
                                                && yytoken != Token_PROTECTED
                                                && yytoken != Token_PUBLIC
                                                && yytoken != Token_RETURN
                                                && yytoken != Token_SHORT
                                                && yytoken != Token_STATIC
                                                && yytoken != Token_STRICTFP
                                                && yytoken != Token_SUPER
                                                && yytoken != Token_SWITCH
                                                && yytoken != Token_SYNCHRONIZED
                                                && yytoken != Token_THIS
                                                && yytoken != Token_THROW
                                                && yytoken != Token_TRANSIENT
                                                && yytoken != Token_TRY
                                                && yytoken != Token_VOID
                                                && yytoken != Token_VOLATILE
                                                && yytoken != Token_WHILE
                                                && yytoken != Token_LPAREN
                                                && yytoken != Token_LBRACE
                                                && yytoken != Token_RBRACE
                                                && yytoken != Token_SEMICOLON
                                                && yytoken != Token_AT
                                                && yytoken != Token_LESS_THAN
                                                && yytoken != Token_BANG
                                                && yytoken != Token_TILDE
                                                && yytoken != Token_INCREMENT
                                                && yytoken != Token_DECREMENT
                                                && yytoken != Token_PLUS
                                                && yytoken != Token_MINUS
                                                && yytoken != Token_TRUE
                                                && yytoken != Token_FALSE
                                                && yytoken != Token_NULL
                                                && yytoken != Token_INTEGER_LITERAL
                                                && yytoken != Token_FLOATING_POINT_LITERAL
                                                && yytoken != Token_CHARACTER_LITERAL
                                                && yytoken != Token_STRING_LITERAL
                                                && yytoken != Token_IDENTIFIER)
                                           {
                                             yylex();
                                           }
                                       }

                                   }
                               }
                             else
                               {
                                 return false;
                               }

                             (*yynode)->end_token = token_stream->index() - 1;

                             return true;
                           }

                           bool parser::parse_switch_statement(switch_statement_ast **yynode)
                           {
                             *yynode = create<switch_statement_ast>();

                             (*yynode)->start_token = token_stream->index() - 1;

                             if (yytoken == Token_SWITCH)
                               {
                                 if (yytoken != Token_SWITCH)
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_token(yytoken, Token_SWITCH, "switch");
                                       }
                                     return false;
                                   }
                                 yylex();

                                 if (yytoken != Token_LPAREN)
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_token(yytoken, Token_LPAREN, "(");
                                       }
                                     return false;
                                   }
                                 yylex();

                                 expression_ast *__node_294 = 0;
                                 if (!parse_expression(&__node_294))
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_symbol(ast_node::Kind_expression, "expression");
                                       }
                                     return false;
                                   }
                                 (*yynode)->switch_expression = __node_294;

                                 if (yytoken != Token_RPAREN)
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_token(yytoken, Token_RPAREN, ")");
                                       }
                                     return false;
                                   }
                                 yylex();

                                 if (yytoken != Token_LBRACE)
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_token(yytoken, Token_LBRACE, "{");
                                       }
                                     return false;
                                   }
                                 yylex();

                                 while (yytoken == Token_CASE
                                        || yytoken == Token_DEFAULT)
                                   {
                                     std::size_t try_start_token_21 = token_stream->index() - 1;
                                     parser_state *try_start_state_21 = copy_current_state();
                                     {
                                       switch_section_ast *__node_295 = 0;
                                       if (!parse_switch_section(&__node_295))
                                         {
                                           goto __catch_21;
                                         }
                                       (*yynode)->switch_section_sequence = snoc((*yynode)->switch_section_sequence, __node_295, memory_pool);

                                     }
                                     if (try_start_state_21)
                                       delete try_start_state_21;

                                     if (false) // the only way to enter here is using goto
                                       {
                                       __catch_21:
                                         if (try_start_state_21)
                                           {
                                             restore_state(try_start_state_21);
                                             delete try_start_state_21;
                                           }
                                         if (try_start_token_21 == token_stream->index() - 1)
                                           yylex();

                                         while (yytoken != Token_EOF
                                                && yytoken != Token_CASE
                                                && yytoken != Token_DEFAULT
                                                && yytoken != Token_RBRACE)
                                           {
                                             yylex();
                                           }
                                       }

                                   }
                                 if (yytoken != Token_RBRACE)
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_token(yytoken, Token_RBRACE, "}");
                                       }
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

                           bool parser::parse_synchronized_statement(synchronized_statement_ast **yynode)
                           {
                             *yynode = create<synchronized_statement_ast>();

                             (*yynode)->start_token = token_stream->index() - 1;

                             if (yytoken == Token_SYNCHRONIZED)
                               {
                                 if (yytoken != Token_SYNCHRONIZED)
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_token(yytoken, Token_SYNCHRONIZED, "synchronized");
                                       }
                                     return false;
                                   }
                                 yylex();

                                 if (yytoken != Token_LPAREN)
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_token(yytoken, Token_LPAREN, "(");
                                       }
                                     return false;
                                   }
                                 yylex();

                                 expression_ast *__node_296 = 0;
                                 if (!parse_expression(&__node_296))
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_symbol(ast_node::Kind_expression, "expression");
                                       }
                                     return false;
                                   }
                                 (*yynode)->locked_type = __node_296;

                                 if (yytoken != Token_RPAREN)
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_token(yytoken, Token_RPAREN, ")");
                                       }
                                     return false;
                                   }
                                 yylex();

                                 block_ast *__node_297 = 0;
                                 if (!parse_block(&__node_297))
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_symbol(ast_node::Kind_block, "block");
                                       }
                                     return false;
                                   }
                                 (*yynode)->synchronized_body = __node_297;

                               }
                             else
                               {
                                 return false;
                               }

                             (*yynode)->end_token = token_stream->index() - 1;

                             return true;
                           }

                           bool parser::parse_this_access_data(this_access_data_ast **yynode)
                           {
                             *yynode = create<this_access_data_ast>();

                             (*yynode)->start_token = token_stream->index() - 1;

                             if (true /*epsilon*/ || yytoken == Token_INSTANCEOF
                                 || yytoken == Token_RPAREN
                                 || yytoken == Token_RBRACE
                                 || yytoken == Token_LBRACKET
                                 || yytoken == Token_RBRACKET
                                 || yytoken == Token_SEMICOLON
                                 || yytoken == Token_COMMA
                                 || yytoken == Token_DOT
                                 || yytoken == Token_ASSIGN
                                 || yytoken == Token_LESS_THAN
                                 || yytoken == Token_GREATER_THAN
                                 || yytoken == Token_QUESTION
                                 || yytoken == Token_COLON
                                 || yytoken == Token_EQUAL
                                 || yytoken == Token_LESS_EQUAL
                                 || yytoken == Token_GREATER_EQUAL
                                 || yytoken == Token_NOT_EQUAL
                                 || yytoken == Token_LOG_AND
                                 || yytoken == Token_LOG_OR
                                 || yytoken == Token_INCREMENT
                                 || yytoken == Token_DECREMENT
                                 || yytoken == Token_PLUS
                                 || yytoken == Token_MINUS
                                 || yytoken == Token_STAR
                                 || yytoken == Token_SLASH
                                 || yytoken == Token_BIT_AND
                                 || yytoken == Token_BIT_OR
                                 || yytoken == Token_BIT_XOR
                                 || yytoken == Token_REMAINDER
                                 || yytoken == Token_LSHIFT
                                 || yytoken == Token_SIGNED_RSHIFT
                                 || yytoken == Token_UNSIGNED_RSHIFT
                                 || yytoken == Token_PLUS_ASSIGN
                                 || yytoken == Token_MINUS_ASSIGN
                                 || yytoken == Token_STAR_ASSIGN
                                 || yytoken == Token_SLASH_ASSIGN
                                 || yytoken == Token_BIT_AND_ASSIGN
                                 || yytoken == Token_BIT_OR_ASSIGN
                                 || yytoken == Token_BIT_XOR_ASSIGN
                                 || yytoken == Token_REMAINDER_ASSIGN
                                 || yytoken == Token_LSHIFT_ASSIGN
                                 || yytoken == Token_SIGNED_RSHIFT_ASSIGN
                                 || yytoken == Token_UNSIGNED_RSHIFT_ASSIGN)
                             {}
                             else
                               {
                                 return false;
                               }

                             (*yynode)->end_token = token_stream->index() - 1;

                             return true;
                           }

                           bool parser::parse_this_call_data(this_call_data_ast **yynode, non_wildcard_type_arguments_ast *type_arguments, optional_argument_list_ast *arguments)
                           {
                             *yynode = create<this_call_data_ast>();

                             (*yynode)->start_token = token_stream->index() - 1;

                             (*yynode)->type_arguments = type_arguments;
                             if (type_arguments && type_arguments->start_token < (*yynode)->start_token)
                               (*yynode)->start_token = type_arguments->start_token;

                             (*yynode)->arguments = arguments;
                             if (arguments && arguments->start_token < (*yynode)->start_token)
                               (*yynode)->start_token = arguments->start_token;

                             if (true /*epsilon*/ || yytoken == Token_INSTANCEOF
                                 || yytoken == Token_RPAREN
                                 || yytoken == Token_RBRACE
                                 || yytoken == Token_LBRACKET
                                 || yytoken == Token_RBRACKET
                                 || yytoken == Token_SEMICOLON
                                 || yytoken == Token_COMMA
                                 || yytoken == Token_DOT
                                 || yytoken == Token_ASSIGN
                                 || yytoken == Token_LESS_THAN
                                 || yytoken == Token_GREATER_THAN
                                 || yytoken == Token_QUESTION
                                 || yytoken == Token_COLON
                                 || yytoken == Token_EQUAL
                                 || yytoken == Token_LESS_EQUAL
                                 || yytoken == Token_GREATER_EQUAL
                                 || yytoken == Token_NOT_EQUAL
                                 || yytoken == Token_LOG_AND
                                 || yytoken == Token_LOG_OR
                                 || yytoken == Token_INCREMENT
                                 || yytoken == Token_DECREMENT
                                 || yytoken == Token_PLUS
                                 || yytoken == Token_MINUS
                                 || yytoken == Token_STAR
                                 || yytoken == Token_SLASH
                                 || yytoken == Token_BIT_AND
                                 || yytoken == Token_BIT_OR
                                 || yytoken == Token_BIT_XOR
                                 || yytoken == Token_REMAINDER
                                 || yytoken == Token_LSHIFT
                                 || yytoken == Token_SIGNED_RSHIFT
                                 || yytoken == Token_UNSIGNED_RSHIFT
                                 || yytoken == Token_PLUS_ASSIGN
                                 || yytoken == Token_MINUS_ASSIGN
                                 || yytoken == Token_STAR_ASSIGN
                                 || yytoken == Token_SLASH_ASSIGN
                                 || yytoken == Token_BIT_AND_ASSIGN
                                 || yytoken == Token_BIT_OR_ASSIGN
                                 || yytoken == Token_BIT_XOR_ASSIGN
                                 || yytoken == Token_REMAINDER_ASSIGN
                                 || yytoken == Token_LSHIFT_ASSIGN
                                 || yytoken == Token_SIGNED_RSHIFT_ASSIGN
                                 || yytoken == Token_UNSIGNED_RSHIFT_ASSIGN)
                             {}
                             else
                               {
                                 return false;
                               }

                             (*yynode)->end_token = token_stream->index() - 1;

                             return true;
                           }

                           bool parser::parse_throw_statement(throw_statement_ast **yynode)
                           {
                             *yynode = create<throw_statement_ast>();

                             (*yynode)->start_token = token_stream->index() - 1;

                             if (yytoken == Token_THROW)
                               {
                                 if (yytoken != Token_THROW)
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_token(yytoken, Token_THROW, "throw");
                                       }
                                     return false;
                                   }
                                 yylex();

                                 expression_ast *__node_298 = 0;
                                 if (!parse_expression(&__node_298))
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_symbol(ast_node::Kind_expression, "expression");
                                       }
                                     return false;
                                   }
                                 (*yynode)->exception = __node_298;

                                 if (yytoken != Token_SEMICOLON)
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_token(yytoken, Token_SEMICOLON, ";");
                                       }
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

                           bool parser::parse_throws_clause(throws_clause_ast **yynode)
                           {
                             *yynode = create<throws_clause_ast>();

                             (*yynode)->start_token = token_stream->index() - 1;

                             if (yytoken == Token_THROWS)
                               {
                                 if (yytoken != Token_THROWS)
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_token(yytoken, Token_THROWS, "throws");
                                       }
                                     return false;
                                   }
                                 yylex();

                                 qualified_identifier_ast *__node_299 = 0;
                                 if (!parse_qualified_identifier(&__node_299))
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_symbol(ast_node::Kind_qualified_identifier, "qualified_identifier");
                                       }
                                     return false;
                                   }
                                 (*yynode)->identifier_sequence = snoc((*yynode)->identifier_sequence, __node_299, memory_pool);

                                 while (yytoken == Token_COMMA)
                                   {
                                     if (yytoken != Token_COMMA)
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_token(yytoken, Token_COMMA, ",");
                                           }
                                         return false;
                                       }
                                     yylex();

                                     qualified_identifier_ast *__node_300 = 0;
                                     if (!parse_qualified_identifier(&__node_300))
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_symbol(ast_node::Kind_qualified_identifier, "qualified_identifier");
                                           }
                                         return false;
                                       }
                                     (*yynode)->identifier_sequence = snoc((*yynode)->identifier_sequence, __node_300, memory_pool);

                                   }
                               }
                             else
                               {
                                 return false;
                               }

                             (*yynode)->end_token = token_stream->index() - 1;

                             return true;
                           }

                           bool parser::parse_try_statement(try_statement_ast **yynode)
                           {
                             *yynode = create<try_statement_ast>();

                             (*yynode)->start_token = token_stream->index() - 1;

                             if (yytoken == Token_TRY)
                               {
                                 if (yytoken != Token_TRY)
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_token(yytoken, Token_TRY, "try");
                                       }
                                     return false;
                                   }
                                 yylex();

                                 block_ast *__node_301 = 0;
                                 if (!parse_block(&__node_301))
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_symbol(ast_node::Kind_block, "block");
                                       }
                                     return false;
                                   }
                                 (*yynode)->try_body = __node_301;

                                 if (yytoken == Token_CATCH)
                                   {
                                     do
                                       {
                                         catch_clause_ast *__node_302 = 0;
                                         if (!parse_catch_clause(&__node_302))
                                           {
                                             if (!yy_block_errors)
                                               {
                                                 yy_expected_symbol(ast_node::Kind_catch_clause, "catch_clause");
                                               }
                                             return false;
                                           }
                                         (*yynode)->catch_clause_sequence = snoc((*yynode)->catch_clause_sequence, __node_302, memory_pool);

                                       }
                                     while (yytoken == Token_CATCH);
                                     if (yytoken == Token_FINALLY)
                                       {
                                         if (yytoken != Token_FINALLY)
                                           {
                                             if (!yy_block_errors)
                                               {
                                                 yy_expected_token(yytoken, Token_FINALLY, "finally");
                                               }
                                             return false;
                                           }
                                         yylex();

                                         block_ast *__node_303 = 0;
                                         if (!parse_block(&__node_303))
                                           {
                                             if (!yy_block_errors)
                                               {
                                                 yy_expected_symbol(ast_node::Kind_block, "block");
                                               }
                                             return false;
                                           }
                                         (*yynode)->finally_body = __node_303;

                                       }
                                     else if (true /*epsilon*/)
                                     {}
                                     else
                                       {
                                         return false;
                                       }
                                   }
                                 else if (yytoken == Token_FINALLY)
                                   {
                                     if (yytoken != Token_FINALLY)
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_token(yytoken, Token_FINALLY, "finally");
                                           }
                                         return false;
                                       }
                                     yylex();

                                     block_ast *__node_304 = 0;
                                     if (!parse_block(&__node_304))
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_symbol(ast_node::Kind_block, "block");
                                           }
                                         return false;
                                       }
                                     (*yynode)->finally_body = __node_304;

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

                           bool parser::parse_type(type_ast **yynode)
                           {
                             *yynode = create<type_ast>();

                             (*yynode)->start_token = token_stream->index() - 1;

                             if (yytoken == Token_BOOLEAN
                                 || yytoken == Token_BYTE
                                 || yytoken == Token_CHAR
                                 || yytoken == Token_DOUBLE
                                 || yytoken == Token_FLOAT
                                 || yytoken == Token_INT
                                 || yytoken == Token_LONG
                                 || yytoken == Token_SHORT
                                 || yytoken == Token_VOID
                                 || yytoken == Token_IDENTIFIER)
                               {
                                 if (yytoken == Token_IDENTIFIER)
                                   {
                                     class_type_ast *__node_305 = 0;
                                     if (!parse_class_type(&__node_305))
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_symbol(ast_node::Kind_class_type, "class_type");
                                           }
                                         return false;
                                       }
                                     (*yynode)->class_type = __node_305;

                                   }
                                 else if (yytoken == Token_BOOLEAN
                                          || yytoken == Token_BYTE
                                          || yytoken == Token_CHAR
                                          || yytoken == Token_DOUBLE
                                          || yytoken == Token_FLOAT
                                          || yytoken == Token_INT
                                          || yytoken == Token_LONG
                                          || yytoken == Token_SHORT
                                          || yytoken == Token_VOID)
                                   {
                                     optional_array_builtin_type_ast *__node_306 = 0;
                                     if (!parse_optional_array_builtin_type(&__node_306))
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_symbol(ast_node::Kind_optional_array_builtin_type, "optional_array_builtin_type");
                                           }
                                         return false;
                                       }
                                     (*yynode)->builtin_type = __node_306;

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

                           bool parser::parse_type_argument(type_argument_ast **yynode)
                           {
                             *yynode = create<type_argument_ast>();

                             (*yynode)->start_token = token_stream->index() - 1;

                             if (yytoken == Token_BOOLEAN
                                 || yytoken == Token_BYTE
                                 || yytoken == Token_CHAR
                                 || yytoken == Token_DOUBLE
                                 || yytoken == Token_FLOAT
                                 || yytoken == Token_INT
                                 || yytoken == Token_LONG
                                 || yytoken == Token_SHORT
                                 || yytoken == Token_VOID
                                 || yytoken == Token_QUESTION
                                 || yytoken == Token_IDENTIFIER)
                               {
                                 if (yytoken == Token_BOOLEAN
                                     || yytoken == Token_BYTE
                                     || yytoken == Token_CHAR
                                     || yytoken == Token_DOUBLE
                                     || yytoken == Token_FLOAT
                                     || yytoken == Token_INT
                                     || yytoken == Token_LONG
                                     || yytoken == Token_SHORT
                                     || yytoken == Token_VOID
                                     || yytoken == Token_IDENTIFIER)
                                   {
                                     type_argument_type_ast *__node_307 = 0;
                                     if (!parse_type_argument_type(&__node_307))
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_symbol(ast_node::Kind_type_argument_type, "type_argument_type");
                                           }
                                         return false;
                                       }
                                     (*yynode)->type_argument_type = __node_307;

                                   }
                                 else if (yytoken == Token_QUESTION)
                                   {
                                     wildcard_type_ast *__node_308 = 0;
                                     if (!parse_wildcard_type(&__node_308))
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_symbol(ast_node::Kind_wildcard_type, "wildcard_type");
                                           }
                                         return false;
                                       }
                                     (*yynode)->wildcard_type = __node_308;

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

                           bool parser::parse_type_argument_type(type_argument_type_ast **yynode)
                           {
                             *yynode = create<type_argument_type_ast>();

                             (*yynode)->start_token = token_stream->index() - 1;

                             if (yytoken == Token_BOOLEAN
                                 || yytoken == Token_BYTE
                                 || yytoken == Token_CHAR
                                 || yytoken == Token_DOUBLE
                                 || yytoken == Token_FLOAT
                                 || yytoken == Token_INT
                                 || yytoken == Token_LONG
                                 || yytoken == Token_SHORT
                                 || yytoken == Token_VOID
                                 || yytoken == Token_IDENTIFIER)
                               {
                                 if (yytoken == Token_IDENTIFIER)
                                   {
                                     class_type_ast *__node_309 = 0;
                                     if (!parse_class_type(&__node_309))
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_symbol(ast_node::Kind_class_type, "class_type");
                                           }
                                         return false;
                                       }
                                     (*yynode)->class_type = __node_309;

                                   }
                                 else if (yytoken == Token_BOOLEAN
                                          || yytoken == Token_BYTE
                                          || yytoken == Token_CHAR
                                          || yytoken == Token_DOUBLE
                                          || yytoken == Token_FLOAT
                                          || yytoken == Token_INT
                                          || yytoken == Token_LONG
                                          || yytoken == Token_SHORT
                                          || yytoken == Token_VOID)
                                   {
                                     mandatory_array_builtin_type_ast *__node_310 = 0;
                                     if (!parse_mandatory_array_builtin_type(&__node_310))
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_symbol(ast_node::Kind_mandatory_array_builtin_type, "mandatory_array_builtin_type");
                                           }
                                         return false;
                                       }
                                     (*yynode)->mandatory_array_builtin_type = __node_310;

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

                           bool parser::parse_type_arguments(type_arguments_ast **yynode)
                           {
                             *yynode = create<type_arguments_ast>();

                             (*yynode)->start_token = token_stream->index() - 1;

                             if (yytoken == Token_LESS_THAN)
                               {
                                 if (yytoken != Token_LESS_THAN)
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_token(yytoken, Token_LESS_THAN, "<");
                                       }
                                     return false;
                                   }
                                 yylex();

                                 int currentLtLevel = _M_state.ltCounter;
                                 _M_state.ltCounter++;
                                 type_argument_ast *__node_311 = 0;
                                 if (!parse_type_argument(&__node_311))
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_symbol(ast_node::Kind_type_argument, "type_argument");
                                       }
                                     return false;
                                   }
                                 (*yynode)->type_argument_sequence = snoc((*yynode)->type_argument_sequence, __node_311, memory_pool);

                                 while (yytoken == Token_COMMA)
                                   {
                                     if ( _M_state.ltCounter != currentLtLevel + 1 )
                                       {
                                         break;
                                       }
                                     if (yytoken != Token_COMMA)
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_token(yytoken, Token_COMMA, ",");
                                           }
                                         return false;
                                       }
                                     yylex();

                                     type_argument_ast *__node_312 = 0;
                                     if (!parse_type_argument(&__node_312))
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_symbol(ast_node::Kind_type_argument, "type_argument");
                                           }
                                         return false;
                                       }
                                     (*yynode)->type_argument_sequence = snoc((*yynode)->type_argument_sequence, __node_312, memory_pool);

                                   }
                                 if (yytoken == Token_GREATER_THAN
                                     || yytoken == Token_SIGNED_RSHIFT
                                     || yytoken == Token_UNSIGNED_RSHIFT)
                                   {
                                     type_arguments_or_parameters_end_ast *__node_313 = 0;
                                     if (!parse_type_arguments_or_parameters_end(&__node_313))
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_symbol(ast_node::Kind_type_arguments_or_parameters_end, "type_arguments_or_parameters_end");
                                           }
                                         return false;
                                       }
                                   }
                                 else if (true /*epsilon*/)
                                 {}
                                 else
                                   {
                                     return false;
                                   }
                                 if (currentLtLevel == 0 && _M_state.ltCounter != currentLtLevel )
                                   {
                                     if (!yy_block_errors)
                                       {
                                         report_problem(error, "The amount of closing ``>'' characters is incorrect");
                                       }
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

                           bool parser::parse_type_arguments_or_parameters_end(type_arguments_or_parameters_end_ast **yynode)
                           {
                             *yynode = create<type_arguments_or_parameters_end_ast>();

                             (*yynode)->start_token = token_stream->index() - 1;

                             if (yytoken == Token_GREATER_THAN
                                 || yytoken == Token_SIGNED_RSHIFT
                                 || yytoken == Token_UNSIGNED_RSHIFT)
                               {
                                 if (yytoken == Token_GREATER_THAN)
                                   {
                                     if (yytoken != Token_GREATER_THAN)
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_token(yytoken, Token_GREATER_THAN, ">");
                                           }
                                         return false;
                                       }
                                     yylex();

                                     _M_state.ltCounter -= 1;
                                   }
                                 else if (yytoken == Token_SIGNED_RSHIFT)
                                   {
                                     if (yytoken != Token_SIGNED_RSHIFT)
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_token(yytoken, Token_SIGNED_RSHIFT, ">>");
                                           }
                                         return false;
                                       }
                                     yylex();

                                     _M_state.ltCounter -= 2;
                                   }
                                 else if (yytoken == Token_UNSIGNED_RSHIFT)
                                   {
                                     if (yytoken != Token_UNSIGNED_RSHIFT)
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_token(yytoken, Token_UNSIGNED_RSHIFT, ">>>");
                                           }
                                         return false;
                                       }
                                     yylex();

                                     _M_state.ltCounter -= 3;
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

                           bool parser::parse_type_declaration(type_declaration_ast **yynode)
                           {
                             *yynode = create<type_declaration_ast>();

                             (*yynode)->start_token = token_stream->index() - 1;

                             if (yytoken == Token_ABSTRACT
                                 || yytoken == Token_CLASS
                                 || yytoken == Token_ENUM
                                 || yytoken == Token_FINAL
                                 || yytoken == Token_INTERFACE
                                 || yytoken == Token_NATIVE
                                 || yytoken == Token_PRIVATE
                                 || yytoken == Token_PROTECTED
                                 || yytoken == Token_PUBLIC
                                 || yytoken == Token_STATIC
                                 || yytoken == Token_STRICTFP
                                 || yytoken == Token_SYNCHRONIZED
                                 || yytoken == Token_TRANSIENT
                                 || yytoken == Token_VOLATILE
                                 || yytoken == Token_SEMICOLON
                                 || yytoken == Token_AT)
                               {
                                 optional_modifiers_ast *modifiers = 0;

                                 if (yytoken == Token_ABSTRACT
                                     || yytoken == Token_CLASS
                                     || yytoken == Token_ENUM
                                     || yytoken == Token_FINAL
                                     || yytoken == Token_INTERFACE
                                     || yytoken == Token_NATIVE
                                     || yytoken == Token_PRIVATE
                                     || yytoken == Token_PROTECTED
                                     || yytoken == Token_PUBLIC
                                     || yytoken == Token_STATIC
                                     || yytoken == Token_STRICTFP
                                     || yytoken == Token_SYNCHRONIZED
                                     || yytoken == Token_TRANSIENT
                                     || yytoken == Token_VOLATILE
                                     || yytoken == Token_AT)
                                   {
                                     optional_modifiers_ast *__node_314 = 0;
                                     if (!parse_optional_modifiers(&__node_314))
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_symbol(ast_node::Kind_optional_modifiers, "optional_modifiers");
                                           }
                                         return false;
                                       }
                                     modifiers = __node_314;

                                     if (yytoken == Token_CLASS)
                                       {
                                         class_declaration_ast *__node_315 = 0;
                                         if (!parse_class_declaration(&__node_315, modifiers))
                                           {
                                             if (!yy_block_errors)
                                               {
                                                 yy_expected_symbol(ast_node::Kind_class_declaration, "class_declaration");
                                               }
                                             return false;
                                           }
                                         (*yynode)->class_declaration = __node_315;

                                       }
                                     else if (yytoken == Token_ENUM)
                                       {
                                         enum_declaration_ast *__node_316 = 0;
                                         if (!parse_enum_declaration(&__node_316, modifiers))
                                           {
                                             if (!yy_block_errors)
                                               {
                                                 yy_expected_symbol(ast_node::Kind_enum_declaration, "enum_declaration");
                                               }
                                             return false;
                                           }
                                         (*yynode)->enum_declaration = __node_316;

                                       }
                                     else if (yytoken == Token_INTERFACE)
                                       {
                                         interface_declaration_ast *__node_317 = 0;
                                         if (!parse_interface_declaration(&__node_317, modifiers))
                                           {
                                             if (!yy_block_errors)
                                               {
                                                 yy_expected_symbol(ast_node::Kind_interface_declaration, "interface_declaration");
                                               }
                                             return false;
                                           }
                                         (*yynode)->interface_declaration = __node_317;

                                       }
                                     else if (yytoken == Token_AT)
                                       {
                                         annotation_type_declaration_ast *__node_318 = 0;
                                         if (!parse_annotation_type_declaration(&__node_318, modifiers))
                                           {
                                             if (!yy_block_errors)
                                               {
                                                 yy_expected_symbol(ast_node::Kind_annotation_type_declaration, "annotation_type_declaration");
                                               }
                                             return false;
                                           }
                                         (*yynode)->annotation_type_declaration = __node_318;

                                       }
                                     else
                                       {
                                         return false;
                                       }
                                   }
                                 else if (yytoken == Token_SEMICOLON)
                                   {
                                     if (yytoken != Token_SEMICOLON)
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_token(yytoken, Token_SEMICOLON, ";");
                                           }
                                         return false;
                                       }
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

                           bool parser::parse_type_parameter(type_parameter_ast **yynode)
                           {
                             *yynode = create<type_parameter_ast>();

                             (*yynode)->start_token = token_stream->index() - 1;

                             if (yytoken == Token_IDENTIFIER)
                               {
                                 identifier_ast *__node_319 = 0;
                                 if (!parse_identifier(&__node_319))
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                                       }
                                     return false;
                                   }
                                 (*yynode)->identifier = __node_319;

                                 if (yytoken == Token_EXTENDS)
                                   {
                                     if (yytoken != Token_EXTENDS)
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_token(yytoken, Token_EXTENDS, "extends");
                                           }
                                         return false;
                                       }
                                     yylex();

                                     class_or_interface_type_name_ast *__node_320 = 0;
                                     if (!parse_class_or_interface_type_name(&__node_320))
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_symbol(ast_node::Kind_class_or_interface_type_name, "class_or_interface_type_name");
                                           }
                                         return false;
                                       }
                                     (*yynode)->extends_type_sequence = snoc((*yynode)->extends_type_sequence, __node_320, memory_pool);

                                     while (yytoken == Token_BIT_AND)
                                       {
                                         if (yytoken != Token_BIT_AND)
                                           {
                                             if (!yy_block_errors)
                                               {
                                                 yy_expected_token(yytoken, Token_BIT_AND, "&");
                                               }
                                             return false;
                                           }
                                         yylex();

                                         class_or_interface_type_name_ast *__node_321 = 0;
                                         if (!parse_class_or_interface_type_name(&__node_321))
                                           {
                                             if (!yy_block_errors)
                                               {
                                                 yy_expected_symbol(ast_node::Kind_class_or_interface_type_name, "class_or_interface_type_name");
                                               }
                                             return false;
                                           }
                                         (*yynode)->extends_type_sequence = snoc((*yynode)->extends_type_sequence, __node_321, memory_pool);

                                       }
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

                           bool parser::parse_type_parameters(type_parameters_ast **yynode)
                           {
                             *yynode = create<type_parameters_ast>();

                             (*yynode)->start_token = token_stream->index() - 1;

                             if (yytoken == Token_LESS_THAN)
                               {
                                 if (yytoken != Token_LESS_THAN)
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_token(yytoken, Token_LESS_THAN, "<");
                                       }
                                     return false;
                                   }
                                 yylex();

                                 int currentLtLevel = _M_state.ltCounter;
                                 _M_state.ltCounter++;
                                 type_parameter_ast *__node_322 = 0;
                                 if (!parse_type_parameter(&__node_322))
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_symbol(ast_node::Kind_type_parameter, "type_parameter");
                                       }
                                     return false;
                                   }
                                 (*yynode)->type_parameter_sequence = snoc((*yynode)->type_parameter_sequence, __node_322, memory_pool);

                                 while (yytoken == Token_COMMA)
                                   {
                                     if (yytoken != Token_COMMA)
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_token(yytoken, Token_COMMA, ",");
                                           }
                                         return false;
                                       }
                                     yylex();

                                     type_parameter_ast *__node_323 = 0;
                                     if (!parse_type_parameter(&__node_323))
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_symbol(ast_node::Kind_type_parameter, "type_parameter");
                                           }
                                         return false;
                                       }
                                     (*yynode)->type_parameter_sequence = snoc((*yynode)->type_parameter_sequence, __node_323, memory_pool);

                                   }
                                 if (yytoken == Token_GREATER_THAN
                                     || yytoken == Token_SIGNED_RSHIFT
                                     || yytoken == Token_UNSIGNED_RSHIFT)
                                   {
                                     type_arguments_or_parameters_end_ast *__node_324 = 0;
                                     if (!parse_type_arguments_or_parameters_end(&__node_324))
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_symbol(ast_node::Kind_type_arguments_or_parameters_end, "type_arguments_or_parameters_end");
                                           }
                                         return false;
                                       }
                                   }
                                 else if (true /*epsilon*/)
                                 {}
                                 else
                                   {
                                     return false;
                                   }
                                 if (currentLtLevel == 0 && _M_state.ltCounter != currentLtLevel )
                                   {
                                     if (!yy_block_errors)
                                       {
                                         report_problem(error, "The amount of closing ``>'' characters is incorrect");
                                       }
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

                           bool parser::parse_unary_expression(unary_expression_ast **yynode)
                           {
                             *yynode = create<unary_expression_ast>();

                             (*yynode)->start_token = token_stream->index() - 1;

                             if (yytoken == Token_BOOLEAN
                                 || yytoken == Token_BYTE
                                 || yytoken == Token_CHAR
                                 || yytoken == Token_DOUBLE
                                 || yytoken == Token_FLOAT
                                 || yytoken == Token_INT
                                 || yytoken == Token_LONG
                                 || yytoken == Token_NEW
                                 || yytoken == Token_SHORT
                                 || yytoken == Token_SUPER
                                 || yytoken == Token_THIS
                                 || yytoken == Token_VOID
                                 || yytoken == Token_LPAREN
                                 || yytoken == Token_LESS_THAN
                                 || yytoken == Token_BANG
                                 || yytoken == Token_TILDE
                                 || yytoken == Token_INCREMENT
                                 || yytoken == Token_DECREMENT
                                 || yytoken == Token_PLUS
                                 || yytoken == Token_MINUS
                                 || yytoken == Token_TRUE
                                 || yytoken == Token_FALSE
                                 || yytoken == Token_NULL
                                 || yytoken == Token_INTEGER_LITERAL
                                 || yytoken == Token_FLOATING_POINT_LITERAL
                                 || yytoken == Token_CHARACTER_LITERAL
                                 || yytoken == Token_STRING_LITERAL
                                 || yytoken == Token_IDENTIFIER)
                               {
                                 if (yytoken == Token_INCREMENT)
                                   {
                                     if (yytoken != Token_INCREMENT)
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_token(yytoken, Token_INCREMENT, "++");
                                           }
                                         return false;
                                       }
                                     yylex();

                                     unary_expression_ast *__node_325 = 0;
                                     if (!parse_unary_expression(&__node_325))
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_symbol(ast_node::Kind_unary_expression, "unary_expression");
                                           }
                                         return false;
                                       }
                                     (*yynode)->unary_expression = __node_325;

                                     (*yynode)->rule_type = unary_expression::type_incremented_expression;
                                   }
                                 else if (yytoken == Token_DECREMENT)
                                   {
                                     if (yytoken != Token_DECREMENT)
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_token(yytoken, Token_DECREMENT, "--");
                                           }
                                         return false;
                                       }
                                     yylex();

                                     unary_expression_ast *__node_326 = 0;
                                     if (!parse_unary_expression(&__node_326))
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_symbol(ast_node::Kind_unary_expression, "unary_expression");
                                           }
                                         return false;
                                       }
                                     (*yynode)->unary_expression = __node_326;

                                     (*yynode)->rule_type = unary_expression::type_decremented_expression;
                                   }
                                 else if (yytoken == Token_MINUS)
                                   {
                                     if (yytoken != Token_MINUS)
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_token(yytoken, Token_MINUS, "-");
                                           }
                                         return false;
                                       }
                                     yylex();

                                     unary_expression_ast *__node_327 = 0;
                                     if (!parse_unary_expression(&__node_327))
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_symbol(ast_node::Kind_unary_expression, "unary_expression");
                                           }
                                         return false;
                                       }
                                     (*yynode)->unary_expression = __node_327;

                                     (*yynode)->rule_type = unary_expression::type_unary_minus_expression;
                                   }
                                 else if (yytoken == Token_PLUS)
                                   {
                                     if (yytoken != Token_PLUS)
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_token(yytoken, Token_PLUS, "+");
                                           }
                                         return false;
                                       }
                                     yylex();

                                     unary_expression_ast *__node_328 = 0;
                                     if (!parse_unary_expression(&__node_328))
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_symbol(ast_node::Kind_unary_expression, "unary_expression");
                                           }
                                         return false;
                                       }
                                     (*yynode)->unary_expression = __node_328;

                                     (*yynode)->rule_type = unary_expression::type_unary_plus_expression;
                                   }
                                 else if (yytoken == Token_BOOLEAN
                                          || yytoken == Token_BYTE
                                          || yytoken == Token_CHAR
                                          || yytoken == Token_DOUBLE
                                          || yytoken == Token_FLOAT
                                          || yytoken == Token_INT
                                          || yytoken == Token_LONG
                                          || yytoken == Token_NEW
                                          || yytoken == Token_SHORT
                                          || yytoken == Token_SUPER
                                          || yytoken == Token_THIS
                                          || yytoken == Token_VOID
                                          || yytoken == Token_LPAREN
                                          || yytoken == Token_LESS_THAN
                                          || yytoken == Token_BANG
                                          || yytoken == Token_TILDE
                                          || yytoken == Token_TRUE
                                          || yytoken == Token_FALSE
                                          || yytoken == Token_NULL
                                          || yytoken == Token_INTEGER_LITERAL
                                          || yytoken == Token_FLOATING_POINT_LITERAL
                                          || yytoken == Token_CHARACTER_LITERAL
                                          || yytoken == Token_STRING_LITERAL
                                          || yytoken == Token_IDENTIFIER)
                                   {
                                     unary_expression_not_plusminus_ast *__node_329 = 0;
                                     if (!parse_unary_expression_not_plusminus(&__node_329))
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_symbol(ast_node::Kind_unary_expression_not_plusminus, "unary_expression_not_plusminus");
                                           }
                                         return false;
                                       }
                                     (*yynode)->unary_expression_not_plusminus = __node_329;

                                     (*yynode)->rule_type = unary_expression::type_unary_expression_not_plusminus;
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

                           bool parser::parse_unary_expression_not_plusminus(unary_expression_not_plusminus_ast **yynode)
                           {
                             *yynode = create<unary_expression_not_plusminus_ast>();

                             (*yynode)->start_token = token_stream->index() - 1;

                             if (yytoken == Token_BOOLEAN
                                 || yytoken == Token_BYTE
                                 || yytoken == Token_CHAR
                                 || yytoken == Token_DOUBLE
                                 || yytoken == Token_FLOAT
                                 || yytoken == Token_INT
                                 || yytoken == Token_LONG
                                 || yytoken == Token_NEW
                                 || yytoken == Token_SHORT
                                 || yytoken == Token_SUPER
                                 || yytoken == Token_THIS
                                 || yytoken == Token_VOID
                                 || yytoken == Token_LPAREN
                                 || yytoken == Token_LESS_THAN
                                 || yytoken == Token_BANG
                                 || yytoken == Token_TILDE
                                 || yytoken == Token_TRUE
                                 || yytoken == Token_FALSE
                                 || yytoken == Token_NULL
                                 || yytoken == Token_INTEGER_LITERAL
                                 || yytoken == Token_FLOATING_POINT_LITERAL
                                 || yytoken == Token_CHARACTER_LITERAL
                                 || yytoken == Token_STRING_LITERAL
                                 || yytoken == Token_IDENTIFIER)
                               {
                                 if (yytoken == Token_TILDE)
                                   {
                                     if (yytoken != Token_TILDE)
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_token(yytoken, Token_TILDE, "~");
                                           }
                                         return false;
                                       }
                                     yylex();

                                     unary_expression_ast *__node_330 = 0;
                                     if (!parse_unary_expression(&__node_330))
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_symbol(ast_node::Kind_unary_expression, "unary_expression");
                                           }
                                         return false;
                                       }
                                     (*yynode)->bitwise_not_expression = __node_330;

                                     (*yynode)->rule_type = unary_expression_not_plusminus::type_bitwise_not_expression;
                                   }
                                 else if (yytoken == Token_BANG)
                                   {
                                     if (yytoken != Token_BANG)
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_token(yytoken, Token_BANG, "!");
                                           }
                                         return false;
                                       }
                                     yylex();

                                     unary_expression_ast *__node_331 = 0;
                                     if (!parse_unary_expression(&__node_331))
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_symbol(ast_node::Kind_unary_expression, "unary_expression");
                                           }
                                         return false;
                                       }
                                     (*yynode)->logical_not_expression = __node_331;

                                     (*yynode)->rule_type = unary_expression_not_plusminus::type_logical_not_expression;
                                   }
                                 else if (yytoken == Token_BOOLEAN
                                          || yytoken == Token_BYTE
                                          || yytoken == Token_CHAR
                                          || yytoken == Token_DOUBLE
                                          || yytoken == Token_FLOAT
                                          || yytoken == Token_INT
                                          || yytoken == Token_LONG
                                          || yytoken == Token_NEW
                                          || yytoken == Token_SHORT
                                          || yytoken == Token_SUPER
                                          || yytoken == Token_THIS
                                          || yytoken == Token_VOID
                                          || yytoken == Token_LPAREN
                                          || yytoken == Token_LESS_THAN
                                          || yytoken == Token_TRUE
                                          || yytoken == Token_FALSE
                                          || yytoken == Token_NULL
                                          || yytoken == Token_INTEGER_LITERAL
                                          || yytoken == Token_FLOATING_POINT_LITERAL
                                          || yytoken == Token_CHARACTER_LITERAL
                                          || yytoken == Token_STRING_LITERAL
                                          || yytoken == Token_IDENTIFIER)
                                   {
                                     bool block_errors_22 = block_errors(true);
                                     std::size_t try_start_token_22 = token_stream->index() - 1;
                                     parser_state *try_start_state_22 = copy_current_state();
                                     {
                                       cast_expression_ast *__node_332 = 0;
                                       if (!parse_cast_expression(&__node_332))
                                         {
                                           goto __catch_22;
                                         }
                                       (*yynode)->cast_expression = __node_332;

                                       (*yynode)->rule_type = unary_expression_not_plusminus::type_cast_expression;
                                     }
                                     block_errors(block_errors_22);
                                     if (try_start_state_22)
                                       delete try_start_state_22;

                                     if (false) // the only way to enter here is using goto
                                       {
                                       __catch_22:
                                         if (try_start_state_22)
                                           {
                                             restore_state(try_start_state_22);
                                             delete try_start_state_22;
                                           }
                                         block_errors(block_errors_22);
                                         rewind(try_start_token_22);

                                         primary_expression_ast *__node_333 = 0;
                                         if (!parse_primary_expression(&__node_333))
                                           {
                                             if (!yy_block_errors)
                                               {
                                                 yy_expected_symbol(ast_node::Kind_primary_expression, "primary_expression");
                                               }
                                             return false;
                                           }
                                         (*yynode)->primary_expression = __node_333;

                                         while (yytoken == Token_INCREMENT
                                                || yytoken == Token_DECREMENT)
                                           {
                                             postfix_operator_ast *__node_334 = 0;
                                             if (!parse_postfix_operator(&__node_334))
                                               {
                                                 if (!yy_block_errors)
                                                   {
                                                     yy_expected_symbol(ast_node::Kind_postfix_operator, "postfix_operator");
                                                   }
                                                 return false;
                                               }
                                             (*yynode)->postfix_operator_sequence = snoc((*yynode)->postfix_operator_sequence, __node_334, memory_pool);

                                           }
                                         (*yynode)->rule_type = unary_expression_not_plusminus::type_primary_expression;
                                       }

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

                           bool parser::parse_variable_array_initializer(variable_array_initializer_ast **yynode)
                           {
                             *yynode = create<variable_array_initializer_ast>();

                             (*yynode)->start_token = token_stream->index() - 1;

                             if (yytoken == Token_LBRACE)
                               {
                                 if (yytoken != Token_LBRACE)
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_token(yytoken, Token_LBRACE, "{");
                                       }
                                     return false;
                                   }
                                 yylex();

                                 if (yytoken == Token_BOOLEAN
                                     || yytoken == Token_BYTE
                                     || yytoken == Token_CHAR
                                     || yytoken == Token_DOUBLE
                                     || yytoken == Token_FLOAT
                                     || yytoken == Token_INT
                                     || yytoken == Token_LONG
                                     || yytoken == Token_NEW
                                     || yytoken == Token_SHORT
                                     || yytoken == Token_SUPER
                                     || yytoken == Token_THIS
                                     || yytoken == Token_VOID
                                     || yytoken == Token_LPAREN
                                     || yytoken == Token_LBRACE
                                     || yytoken == Token_LESS_THAN
                                     || yytoken == Token_BANG
                                     || yytoken == Token_TILDE
                                     || yytoken == Token_INCREMENT
                                     || yytoken == Token_DECREMENT
                                     || yytoken == Token_PLUS
                                     || yytoken == Token_MINUS
                                     || yytoken == Token_TRUE
                                     || yytoken == Token_FALSE
                                     || yytoken == Token_NULL
                                     || yytoken == Token_INTEGER_LITERAL
                                     || yytoken == Token_FLOATING_POINT_LITERAL
                                     || yytoken == Token_CHARACTER_LITERAL
                                     || yytoken == Token_STRING_LITERAL
                                     || yytoken == Token_IDENTIFIER)
                                   {
                                     variable_initializer_ast *__node_335 = 0;
                                     if (!parse_variable_initializer(&__node_335))
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_symbol(ast_node::Kind_variable_initializer, "variable_initializer");
                                           }
                                         return false;
                                       }
                                     (*yynode)->variable_initializer_sequence = snoc((*yynode)->variable_initializer_sequence, __node_335, memory_pool);

                                     while (yytoken == Token_COMMA)
                                       {
                                         if (LA(2).kind == Token_RBRACE)
                                           {
                                             break;
                                           }
                                         if (yytoken != Token_COMMA)
                                           {
                                             if (!yy_block_errors)
                                               {
                                                 yy_expected_token(yytoken, Token_COMMA, ",");
                                               }
                                             return false;
                                           }
                                         yylex();

                                         variable_initializer_ast *__node_336 = 0;
                                         if (!parse_variable_initializer(&__node_336))
                                           {
                                             if (!yy_block_errors)
                                               {
                                                 yy_expected_symbol(ast_node::Kind_variable_initializer, "variable_initializer");
                                               }
                                             return false;
                                           }
                                         (*yynode)->variable_initializer_sequence = snoc((*yynode)->variable_initializer_sequence, __node_336, memory_pool);

                                       }
                                     if (yytoken == Token_COMMA)
                                       {
                                         if (yytoken != Token_COMMA)
                                           {
                                             if (!yy_block_errors)
                                               {
                                                 yy_expected_token(yytoken, Token_COMMA, ",");
                                               }
                                             return false;
                                           }
                                         yylex();

                                       }
                                     else if (true /*epsilon*/)
                                     {}
                                     else
                                       {
                                         return false;
                                       }
                                   }
                                 else if (true /*epsilon*/)
                                 {}
                                 else
                                   {
                                     return false;
                                   }
                                 if (yytoken != Token_RBRACE)
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_token(yytoken, Token_RBRACE, "}");
                                       }
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

                           bool parser::parse_variable_declaration(variable_declaration_ast **yynode)
                           {
                             *yynode = create<variable_declaration_ast>();

                             (*yynode)->start_token = token_stream->index() - 1;

                             if (yytoken == Token_ABSTRACT
                                 || yytoken == Token_BOOLEAN
                                 || yytoken == Token_BYTE
                                 || yytoken == Token_CHAR
                                 || yytoken == Token_DOUBLE
                                 || yytoken == Token_FINAL
                                 || yytoken == Token_FLOAT
                                 || yytoken == Token_INT
                                 || yytoken == Token_LONG
                                 || yytoken == Token_NATIVE
                                 || yytoken == Token_PRIVATE
                                 || yytoken == Token_PROTECTED
                                 || yytoken == Token_PUBLIC
                                 || yytoken == Token_SHORT
                                 || yytoken == Token_STATIC
                                 || yytoken == Token_STRICTFP
                                 || yytoken == Token_SYNCHRONIZED
                                 || yytoken == Token_TRANSIENT
                                 || yytoken == Token_VOID
                                 || yytoken == Token_VOLATILE
                                 || yytoken == Token_AT
                                 || yytoken == Token_IDENTIFIER)
                               {
                                 optional_modifiers_ast *modifiers = 0;

                                 type_ast *type = 0;

                                 const list_node<variable_declarator_ast *> *variable_declarator_sequence = 0;

                                 optional_modifiers_ast *__node_337 = 0;
                                 if (!parse_optional_modifiers(&__node_337))
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_symbol(ast_node::Kind_optional_modifiers, "optional_modifiers");
                                       }
                                     return false;
                                   }
                                 modifiers = __node_337;

                                 type_ast *__node_338 = 0;
                                 if (!parse_type(&__node_338))
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_symbol(ast_node::Kind_type, "type");
                                       }
                                     return false;
                                   }
                                 type = __node_338;

                                 variable_declarator_ast *__node_339 = 0;
                                 if (!parse_variable_declarator(&__node_339))
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_symbol(ast_node::Kind_variable_declarator, "variable_declarator");
                                       }
                                     return false;
                                   }
                                 variable_declarator_sequence = snoc(variable_declarator_sequence, __node_339, memory_pool);

                                 while (yytoken == Token_COMMA)
                                   {
                                     if (yytoken != Token_COMMA)
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_token(yytoken, Token_COMMA, ",");
                                           }
                                         return false;
                                       }
                                     yylex();

                                     variable_declarator_ast *__node_340 = 0;
                                     if (!parse_variable_declarator(&__node_340))
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_symbol(ast_node::Kind_variable_declarator, "variable_declarator");
                                           }
                                         return false;
                                       }
                                     variable_declarator_sequence = snoc(variable_declarator_sequence, __node_340, memory_pool);

                                   }
                                 variable_declaration_data_ast *__node_341 = 0;
                                 if (!parse_variable_declaration_data(&__node_341,
                                                                      modifiers, type, variable_declarator_sequence
                                                                     ))
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_symbol(ast_node::Kind_variable_declaration_data, "variable_declaration_data");
                                       }
                                     return false;
                                   }
                                 (*yynode)->data = __node_341;

                               }
                             else
                               {
                                 return false;
                               }

                             (*yynode)->end_token = token_stream->index() - 1;

                             return true;
                           }

                           bool parser::parse_variable_declaration_data(variable_declaration_data_ast **yynode, optional_modifiers_ast *modifiers, type_ast *type, const list_node<variable_declarator_ast *> *declarator_sequence)
                           {
                             *yynode = create<variable_declaration_data_ast>();

                             (*yynode)->start_token = token_stream->index() - 1;

                             (*yynode)->modifiers = modifiers;
                             if (modifiers && modifiers->start_token < (*yynode)->start_token)
                               (*yynode)->start_token = modifiers->start_token;

                             (*yynode)->type = type;
                             if (type && type->start_token < (*yynode)->start_token)
                               (*yynode)->start_token = type->start_token;

                             (*yynode)->declarator_sequence = declarator_sequence;
                             if (declarator_sequence && declarator_sequence->to_front()->element->start_token < (*yynode)->start_token)
                               (*yynode)->start_token = declarator_sequence->to_front()->element->start_token;

                             if (true /*epsilon*/ || yytoken == Token_ABSTRACT
                                 || yytoken == Token_BOOLEAN
                                 || yytoken == Token_BYTE
                                 || yytoken == Token_CHAR
                                 || yytoken == Token_CLASS
                                 || yytoken == Token_DOUBLE
                                 || yytoken == Token_ENUM
                                 || yytoken == Token_FINAL
                                 || yytoken == Token_FLOAT
                                 || yytoken == Token_INT
                                 || yytoken == Token_INTERFACE
                                 || yytoken == Token_LONG
                                 || yytoken == Token_NATIVE
                                 || yytoken == Token_PRIVATE
                                 || yytoken == Token_PROTECTED
                                 || yytoken == Token_PUBLIC
                                 || yytoken == Token_SHORT
                                 || yytoken == Token_STATIC
                                 || yytoken == Token_STRICTFP
                                 || yytoken == Token_SYNCHRONIZED
                                 || yytoken == Token_TRANSIENT
                                 || yytoken == Token_VOID
                                 || yytoken == Token_VOLATILE
                                 || yytoken == Token_LBRACE
                                 || yytoken == Token_RBRACE
                                 || yytoken == Token_SEMICOLON
                                 || yytoken == Token_AT
                                 || yytoken == Token_LESS_THAN
                                 || yytoken == Token_IDENTIFIER)
                             {}
                             else
                               {
                                 return false;
                               }

                             (*yynode)->end_token = token_stream->index() - 1;

                             return true;
                           }

                           bool parser::parse_variable_declaration_rest(variable_declaration_rest_ast **yynode)
                           {
                             *yynode = create<variable_declaration_rest_ast>();

                             (*yynode)->start_token = token_stream->index() - 1;

                             if (yytoken == Token_COMMA
                                 || yytoken == Token_ASSIGN || yytoken == Token_SEMICOLON)
                               {
                                 if (yytoken == Token_ASSIGN)
                                   {
                                     if (yytoken != Token_ASSIGN)
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_token(yytoken, Token_ASSIGN, "=");
                                           }
                                         return false;
                                       }
                                     yylex();

                                     variable_initializer_ast *__node_342 = 0;
                                     if (!parse_variable_initializer(&__node_342))
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_symbol(ast_node::Kind_variable_initializer, "variable_initializer");
                                           }
                                         return false;
                                       }
                                     (*yynode)->first_initializer = __node_342;

                                   }
                                 else if (true /*epsilon*/)
                                 {}
                                 else
                                   {
                                     return false;
                                   }
                                 while (yytoken == Token_COMMA)
                                   {
                                     if (yytoken != Token_COMMA)
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_token(yytoken, Token_COMMA, ",");
                                           }
                                         return false;
                                       }
                                     yylex();

                                     variable_declarator_ast *__node_343 = 0;
                                     if (!parse_variable_declarator(&__node_343))
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_symbol(ast_node::Kind_variable_declarator, "variable_declarator");
                                           }
                                         return false;
                                       }
                                     (*yynode)->variable_declarator_sequence = snoc((*yynode)->variable_declarator_sequence, __node_343, memory_pool);

                                   }
                               }
                             else
                               {
                                 return false;
                               }

                             (*yynode)->end_token = token_stream->index() - 1;

                             return true;
                           }

                           bool parser::parse_variable_declaration_split_data(variable_declaration_split_data_ast **yynode, parameter_declaration_ast *parameter_declaration, variable_declaration_rest_ast *rest)
                           {
                             *yynode = create<variable_declaration_split_data_ast>();

                             (*yynode)->start_token = token_stream->index() - 1;

                             const list_node<variable_declarator_ast *> *declarator_sequence = 0;

                             if (true /*epsilon*/ || yytoken == Token_LBRACE
                                 || yytoken == Token_SEMICOLON)
                               {

                                 variable_declarator_ast* declarator = create<variable_declarator_ast>();
                                 declarator->variable_name = parameter_declaration->variable_name;
                                 declarator->declarator_brackets = parameter_declaration->declarator_brackets;

                                 if (rest)
                                   declarator->initializer = rest->first_initializer;
                                 else
                                   declarator->initializer = 0;

                                 declarator_sequence = snoc(declarator_sequence, declarator, memory_pool);

                                 if (rest && rest->variable_declarator_sequence)
                                   {
                                     const list_node<variable_declarator_ast*> *__it
                                     = rest->variable_declarator_sequence->to_front(), *__end = __it;

                                     do
                                       {
                                         declarator_sequence = snoc(declarator_sequence, __it->element, memory_pool);
                                         __it = __it->next;
                                       }
                                     while (__it != __end);
                                   }
                                 variable_declaration_data_ast *__node_344 = 0;
                                 if (!parse_variable_declaration_data(&__node_344,
                                                                      parameter_declaration->parameter_modifiers, parameter_declaration->type,
                                                                      declarator_sequence
                                                                     ))
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_symbol(ast_node::Kind_variable_declaration_data, "variable_declaration_data");
                                       }
                                     return false;
                                   }
                                 if (__node_344->start_token < (*yynode)->start_token)
                                   (*yynode)->start_token = __node_344->start_token;
                                 (*yynode)->data = __node_344;

                               }
                             else
                               {
                                 return false;
                               }

                             (*yynode)->end_token = token_stream->index() - 1;

                             return true;
                           }

                           bool parser::parse_variable_declaration_statement(variable_declaration_statement_ast **yynode)
                           {
                             *yynode = create<variable_declaration_statement_ast>();

                             (*yynode)->start_token = token_stream->index() - 1;

                             if (yytoken == Token_ABSTRACT
                                 || yytoken == Token_BOOLEAN
                                 || yytoken == Token_BYTE
                                 || yytoken == Token_CHAR
                                 || yytoken == Token_DOUBLE
                                 || yytoken == Token_FINAL
                                 || yytoken == Token_FLOAT
                                 || yytoken == Token_INT
                                 || yytoken == Token_LONG
                                 || yytoken == Token_NATIVE
                                 || yytoken == Token_PRIVATE
                                 || yytoken == Token_PROTECTED
                                 || yytoken == Token_PUBLIC
                                 || yytoken == Token_SHORT
                                 || yytoken == Token_STATIC
                                 || yytoken == Token_STRICTFP
                                 || yytoken == Token_SYNCHRONIZED
                                 || yytoken == Token_TRANSIENT
                                 || yytoken == Token_VOID
                                 || yytoken == Token_VOLATILE
                                 || yytoken == Token_AT
                                 || yytoken == Token_IDENTIFIER)
                               {
                                 variable_declaration_ast *__node_345 = 0;
                                 if (!parse_variable_declaration(&__node_345))
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_symbol(ast_node::Kind_variable_declaration, "variable_declaration");
                                       }
                                     return false;
                                   }
                                 (*yynode)->variable_declaration = __node_345;

                                 if (yytoken != Token_SEMICOLON)
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_token(yytoken, Token_SEMICOLON, ";");
                                       }
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

                           bool parser::parse_variable_declarator(variable_declarator_ast **yynode)
                           {
                             *yynode = create<variable_declarator_ast>();

                             (*yynode)->start_token = token_stream->index() - 1;

                             if (yytoken == Token_IDENTIFIER)
                               {
                                 identifier_ast *__node_346 = 0;
                                 if (!parse_identifier(&__node_346))
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                                       }
                                     return false;
                                   }
                                 (*yynode)->variable_name = __node_346;

                                 optional_declarator_brackets_ast *__node_347 = 0;
                                 if (!parse_optional_declarator_brackets(&__node_347))
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_symbol(ast_node::Kind_optional_declarator_brackets, "optional_declarator_brackets");
                                       }
                                     return false;
                                   }
                                 (*yynode)->declarator_brackets = __node_347;

                                 if (yytoken == Token_ASSIGN)
                                   {
                                     if (yytoken != Token_ASSIGN)
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_token(yytoken, Token_ASSIGN, "=");
                                           }
                                         return false;
                                       }
                                     yylex();

                                     variable_initializer_ast *__node_348 = 0;
                                     if (!parse_variable_initializer(&__node_348))
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_symbol(ast_node::Kind_variable_initializer, "variable_initializer");
                                           }
                                         return false;
                                       }
                                     (*yynode)->initializer = __node_348;

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

                           bool parser::parse_variable_initializer(variable_initializer_ast **yynode)
                           {
                             *yynode = create<variable_initializer_ast>();

                             (*yynode)->start_token = token_stream->index() - 1;

                             if (yytoken == Token_BOOLEAN
                                 || yytoken == Token_BYTE
                                 || yytoken == Token_CHAR
                                 || yytoken == Token_DOUBLE
                                 || yytoken == Token_FLOAT
                                 || yytoken == Token_INT
                                 || yytoken == Token_LONG
                                 || yytoken == Token_NEW
                                 || yytoken == Token_SHORT
                                 || yytoken == Token_SUPER
                                 || yytoken == Token_THIS
                                 || yytoken == Token_VOID
                                 || yytoken == Token_LPAREN
                                 || yytoken == Token_LBRACE
                                 || yytoken == Token_LESS_THAN
                                 || yytoken == Token_BANG
                                 || yytoken == Token_TILDE
                                 || yytoken == Token_INCREMENT
                                 || yytoken == Token_DECREMENT
                                 || yytoken == Token_PLUS
                                 || yytoken == Token_MINUS
                                 || yytoken == Token_TRUE
                                 || yytoken == Token_FALSE
                                 || yytoken == Token_NULL
                                 || yytoken == Token_INTEGER_LITERAL
                                 || yytoken == Token_FLOATING_POINT_LITERAL
                                 || yytoken == Token_CHARACTER_LITERAL
                                 || yytoken == Token_STRING_LITERAL
                                 || yytoken == Token_IDENTIFIER)
                               {
                                 if (yytoken == Token_BOOLEAN
                                     || yytoken == Token_BYTE
                                     || yytoken == Token_CHAR
                                     || yytoken == Token_DOUBLE
                                     || yytoken == Token_FLOAT
                                     || yytoken == Token_INT
                                     || yytoken == Token_LONG
                                     || yytoken == Token_NEW
                                     || yytoken == Token_SHORT
                                     || yytoken == Token_SUPER
                                     || yytoken == Token_THIS
                                     || yytoken == Token_VOID
                                     || yytoken == Token_LPAREN
                                     || yytoken == Token_LESS_THAN
                                     || yytoken == Token_BANG
                                     || yytoken == Token_TILDE
                                     || yytoken == Token_INCREMENT
                                     || yytoken == Token_DECREMENT
                                     || yytoken == Token_PLUS
                                     || yytoken == Token_MINUS
                                     || yytoken == Token_TRUE
                                     || yytoken == Token_FALSE
                                     || yytoken == Token_NULL
                                     || yytoken == Token_INTEGER_LITERAL
                                     || yytoken == Token_FLOATING_POINT_LITERAL
                                     || yytoken == Token_CHARACTER_LITERAL
                                     || yytoken == Token_STRING_LITERAL
                                     || yytoken == Token_IDENTIFIER)
                                   {
                                     expression_ast *__node_349 = 0;
                                     if (!parse_expression(&__node_349))
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_symbol(ast_node::Kind_expression, "expression");
                                           }
                                         return false;
                                       }
                                     (*yynode)->expression = __node_349;

                                   }
                                 else if (yytoken == Token_LBRACE)
                                   {
                                     variable_array_initializer_ast *__node_350 = 0;
                                     if (!parse_variable_array_initializer(&__node_350))
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_symbol(ast_node::Kind_variable_array_initializer, "variable_array_initializer");
                                           }
                                         return false;
                                       }
                                     (*yynode)->array_initializer = __node_350;

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

                           bool parser::parse_while_statement(while_statement_ast **yynode)
                           {
                             *yynode = create<while_statement_ast>();

                             (*yynode)->start_token = token_stream->index() - 1;

                             if (yytoken == Token_WHILE)
                               {
                                 if (yytoken != Token_WHILE)
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_token(yytoken, Token_WHILE, "while");
                                       }
                                     return false;
                                   }
                                 yylex();

                                 if (yytoken != Token_LPAREN)
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_token(yytoken, Token_LPAREN, "(");
                                       }
                                     return false;
                                   }
                                 yylex();

                                 expression_ast *__node_351 = 0;
                                 if (!parse_expression(&__node_351))
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_symbol(ast_node::Kind_expression, "expression");
                                       }
                                     return false;
                                   }
                                 (*yynode)->condition = __node_351;

                                 if (yytoken != Token_RPAREN)
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_token(yytoken, Token_RPAREN, ")");
                                       }
                                     return false;
                                   }
                                 yylex();

                                 embedded_statement_ast *__node_352 = 0;
                                 if (!parse_embedded_statement(&__node_352))
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_symbol(ast_node::Kind_embedded_statement, "embedded_statement");
                                       }
                                     return false;
                                   }
                                 (*yynode)->body = __node_352;

                               }
                             else
                               {
                                 return false;
                               }

                             (*yynode)->end_token = token_stream->index() - 1;

                             return true;
                           }

                           bool parser::parse_wildcard_type(wildcard_type_ast **yynode)
                           {
                             *yynode = create<wildcard_type_ast>();

                             (*yynode)->start_token = token_stream->index() - 1;

                             if (yytoken == Token_QUESTION)
                               {
                                 if (yytoken != Token_QUESTION)
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_token(yytoken, Token_QUESTION, "?");
                                       }
                                     return false;
                                   }
                                 yylex();

                                 if (yytoken == Token_EXTENDS
                                     || yytoken == Token_SUPER)
                                   {
                                     wildcard_type_bounds_ast *__node_353 = 0;
                                     if (!parse_wildcard_type_bounds(&__node_353))
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_symbol(ast_node::Kind_wildcard_type_bounds, "wildcard_type_bounds");
                                           }
                                         return false;
                                       }
                                     (*yynode)->bounds = __node_353;

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

                           bool parser::parse_wildcard_type_bounds(wildcard_type_bounds_ast **yynode)
                           {
                             *yynode = create<wildcard_type_bounds_ast>();

                             (*yynode)->start_token = token_stream->index() - 1;

                             if (yytoken == Token_EXTENDS
                                 || yytoken == Token_SUPER)
                               {
                                 if (yytoken == Token_EXTENDS)
                                   {
                                     if (yytoken != Token_EXTENDS)
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_token(yytoken, Token_EXTENDS, "extends");
                                           }
                                         return false;
                                       }
                                     yylex();

                                     (*yynode)->extends_or_super = wildcard_type_bounds::extends;
                                   }
                                 else if (yytoken == Token_SUPER)
                                   {
                                     if (yytoken != Token_SUPER)
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_token(yytoken, Token_SUPER, "super");
                                           }
                                         return false;
                                       }
                                     yylex();

                                     (*yynode)->extends_or_super = wildcard_type_bounds::super;
                                   }
                                 else
                                   {
                                     return false;
                                   }
                                 class_type_ast *__node_354 = 0;
                                 if (!parse_class_type(&__node_354))
                                   {
                                     if (!yy_block_errors)
                                       {
                                         yy_expected_symbol(ast_node::Kind_class_type, "class_type");
                                       }
                                     return false;
                                   }
                                 (*yynode)->type = __node_354;

                               }
                             else
                               {
                                 return false;
                               }

                             (*yynode)->end_token = token_stream->index() - 1;

                             return true;
                           }

                           visitor::parser_fun_t visitor::_S_parser_table[] = {
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_additive_expression),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_additive_expression_rest),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_annotation),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_annotation_arguments),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_annotation_element_array_initializer),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_annotation_element_array_value),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_annotation_element_value),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_annotation_element_value_pair),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_annotation_method_declaration),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_annotation_type_body),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_annotation_type_declaration),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_annotation_type_field),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_array_access),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_array_creator_rest),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_array_type_dot_class),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_assert_statement),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_bit_and_expression),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_bit_or_expression),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_bit_xor_expression),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_block),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_block_statement),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_break_statement),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_builtin_type),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_builtin_type_dot_class),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_cast_expression),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_catch_clause),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_class_access_data),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_class_body),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_class_declaration),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_class_extends_clause),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_class_field),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_class_or_interface_type_name),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_class_or_interface_type_name_part),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_class_type),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_compilation_unit),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_conditional_expression),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_constructor_declaration),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_continue_statement),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_do_while_statement),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_embedded_statement),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_enum_body),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_enum_constant),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_enum_constant_body),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_enum_constant_field),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_enum_declaration),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_equality_expression),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_equality_expression_rest),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_expression),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_for_clause_traditional_rest),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_for_control),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_for_statement),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_foreach_declaration_data),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_identifier),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_if_statement),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_implements_clause),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_import_declaration),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_interface_body),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_interface_declaration),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_interface_extends_clause),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_interface_field),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_interface_method_declaration),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_labeled_statement),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_literal),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_logical_and_expression),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_logical_or_expression),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_mandatory_array_builtin_type),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_mandatory_declarator_brackets),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_method_call_data),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_method_declaration),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_multiplicative_expression),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_multiplicative_expression_rest),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_new_expression),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_non_array_type),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_non_wildcard_type_arguments),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_optional_argument_list),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_optional_array_builtin_type),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_optional_declarator_brackets),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_optional_modifiers),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_optional_parameter_declaration_list),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_optional_parameter_modifiers),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_package_declaration),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_parameter_declaration),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_parameter_declaration_ellipsis),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_postfix_operator),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_primary_atom),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_primary_expression),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_primary_selector),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_qualified_identifier),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_qualified_identifier_with_optional_star),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_relational_expression),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_relational_expression_rest),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_return_statement),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_shift_expression),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_shift_expression_rest),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_simple_name_access_data),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_statement_expression),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_super_access_data),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_super_suffix),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_switch_label),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_switch_section),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_switch_statement),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_synchronized_statement),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_this_access_data),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_this_call_data),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_throw_statement),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_throws_clause),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_try_statement),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_type),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_type_argument),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_type_argument_type),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_type_arguments),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_type_arguments_or_parameters_end),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_type_declaration),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_type_parameter),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_type_parameters),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_unary_expression),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_unary_expression_not_plusminus),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_variable_array_initializer),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_variable_declaration),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_variable_declaration_data),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_variable_declaration_rest),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_variable_declaration_split_data),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_variable_declaration_statement),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_variable_declarator),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_variable_initializer),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_while_statement),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_wildcard_type),
                                 reinterpret_cast<parser_fun_t>(&visitor::visit_wildcard_type_bounds)
                               }; // _S_parser_table[]

} // end of namespace java


