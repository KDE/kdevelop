// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#include "ruby_parser.h"



#include "ruby_lexer.h"

namespace ruby
  {

  void parser::tokenize( char *contents )
  {
    m_lexer = new Lexer( this, contents );

    int kind = parser::Token_EOF;
    do
      {
        kind = m_lexer->yylex();
        std::cerr << m_lexer->YYText() << " of kind " << kind << std::endl; //" "; // debug output

        if ( !kind ) // when the lexer returns 0, the end of file is reached
          kind = parser::Token_EOF;

        parser::token_type &t = this->token_stream->next();
        t.kind = kind;
        t.begin = m_lexer->tokenBegin();
        t.end = m_lexer->tokenEnd();
        t.text = contents;
      }
    while (kind != parser::Token_EOF);

    this->yylex(); // produce the look ahead token
    delete m_lexer;
    m_lexer = 0;
  }

  parser::parser_state *parser::copy_current_state()
  {
    parser_state *state = new parser_state();
    state->ltCounter = _M_state.ltCounter;
    return state;
  }

  void parser::restore_state( parser::parser_state *state )
  {
    _M_state.ltCounter = state->ltCounter;
  }

} // end of namespace ruby


namespace ruby
  {

  bool parser::parse_additiveExpression(additiveExpression_ast **yynode)
  {
    *yynode = create<additiveExpression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BEGIN
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_LPAREN
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_UNARY_MINUS
        || yytoken == Token_UNARY_PLUS
        || yytoken == Token_NOT
        || yytoken == Token_BNOT
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE)
      {
        multiplicativeExpression_ast *__node_0 = 0;
        if (!parse_multiplicativeExpression(&__node_0))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_multiplicativeExpression, "multiplicativeExpression");
              }
            return false;
          }
        while (yytoken == Token_PLUS
               || yytoken == Token_MINUS)
          {
            if (yytoken == Token_PLUS)
              {
                operatorPlus_ast *__node_1 = 0;
                if (!parse_operatorPlus(&__node_1))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorPlus, "operatorPlus");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_MINUS)
              {
                operatorMinus_ast *__node_2 = 0;
                if (!parse_operatorMinus(&__node_2))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorMinus, "operatorMinus");
                      }
                    return false;
                  }
              }
            else
              {
                return false;
              }
            multiplicativeExpression_ast *__node_3 = 0;
            if (!parse_multiplicativeExpression(&__node_3))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_multiplicativeExpression, "multiplicativeExpression");
                  }
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

  bool parser::parse_aliasParameter(aliasParameter_ast **yynode)
  {
    *yynode = create<aliasParameter_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_END
        || yytoken == Token_ELSE
        || yytoken == Token_ELSIF
        || yytoken == Token_RESCUE
        || yytoken == Token_ENSURE
        || yytoken == Token_WHEN
        || yytoken == Token_UNDEF
        || yytoken == Token_ALIAS
        || yytoken == Token_BEGIN
        || yytoken == Token_DO
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_THEN
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_OR
        || yytoken == Token_AND
        || yytoken == Token_IN
        || yytoken == Token_END_UPCASE
        || yytoken == Token_BEGIN_UPCASE
        || yytoken == Token_SINGLE_QUOTE
        || yytoken == Token_LBRACK
        || yytoken == Token_LBRACK_ARRAY_ACCESS
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_BOR
        || yytoken == Token_COMPARE
        || yytoken == Token_EQUAL
        || yytoken == Token_CASE_EQUAL
        || yytoken == Token_MATCH
        || yytoken == Token_LESS_THAN
        || yytoken == Token_GREATER_THAN
        || yytoken == Token_LESS_OR_EQUAL
        || yytoken == Token_GREATER_OR_EQUAL
        || yytoken == Token_BXOR
        || yytoken == Token_BAND
        || yytoken == Token_LEFT_SHIFT
        || yytoken == Token_RIGHT_SHIFT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_STAR
        || yytoken == Token_DIV
        || yytoken == Token_MOD
        || yytoken == Token_NOT
        || yytoken == Token_BNOT
        || yytoken == Token_POWER
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE)
      {
        if (yytoken == Token_END
            || yytoken == Token_ELSE
            || yytoken == Token_ELSIF
            || yytoken == Token_RESCUE
            || yytoken == Token_ENSURE
            || yytoken == Token_WHEN
            || yytoken == Token_UNDEF
            || yytoken == Token_ALIAS
            || yytoken == Token_BEGIN
            || yytoken == Token_DO
            || yytoken == Token_RETURN
            || yytoken == Token_BREAK
            || yytoken == Token_NEXT
            || yytoken == Token_SUPER
            || yytoken == Token_RETRY
            || yytoken == Token_YIELD
            || yytoken == Token_DEFINED
            || yytoken == Token_REDO
            || yytoken == Token_IF
            || yytoken == Token_CASE
            || yytoken == Token_UNTIL
            || yytoken == Token_WHILE
            || yytoken == Token_FOR
            || yytoken == Token_MODULE
            || yytoken == Token_DEF
            || yytoken == Token_THEN
            || yytoken == Token_CLASS
            || yytoken == Token_UNLESS
            || yytoken == Token_OR
            || yytoken == Token_AND
            || yytoken == Token_IN
            || yytoken == Token_END_UPCASE
            || yytoken == Token_BEGIN_UPCASE
            || yytoken == Token_SINGLE_QUOTE
            || yytoken == Token_LBRACK
            || yytoken == Token_LBRACK_ARRAY_ACCESS
            || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
            || yytoken == Token_BOR
            || yytoken == Token_COMPARE
            || yytoken == Token_EQUAL
            || yytoken == Token_CASE_EQUAL
            || yytoken == Token_MATCH
            || yytoken == Token_LESS_THAN
            || yytoken == Token_GREATER_THAN
            || yytoken == Token_LESS_OR_EQUAL
            || yytoken == Token_GREATER_OR_EQUAL
            || yytoken == Token_BXOR
            || yytoken == Token_BAND
            || yytoken == Token_LEFT_SHIFT
            || yytoken == Token_RIGHT_SHIFT
            || yytoken == Token_PLUS
            || yytoken == Token_MINUS
            || yytoken == Token_STAR
            || yytoken == Token_DIV
            || yytoken == Token_MOD
            || yytoken == Token_NOT
            || yytoken == Token_BNOT
            || yytoken == Token_POWER
            || yytoken == Token_IDENTIFIER
            || yytoken == Token_CONSTANT
            || yytoken == Token_FUNCTION)
          {
            undefParameter_ast *__node_4 = 0;
            if (!parse_undefParameter(&__node_4))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_undefParameter, "undefParameter");
                  }
                return false;
              }
          }
        else if (yytoken == Token_GLOBAL_VARIABLE)
          {
            if (yytoken != Token_GLOBAL_VARIABLE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_GLOBAL_VARIABLE, "global variable");
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

  bool parser::parse_andExpression(andExpression_ast **yynode)
  {
    *yynode = create<andExpression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BEGIN
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_LPAREN
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_UNARY_MINUS
        || yytoken == Token_UNARY_PLUS
        || yytoken == Token_NOT
        || yytoken == Token_BNOT
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE)
      {
        shiftExpression_ast *__node_5 = 0;
        if (!parse_shiftExpression(&__node_5))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_shiftExpression, "shiftExpression");
              }
            return false;
          }
        while (yytoken == Token_BAND)
          {
            operatorBAnd_ast *__node_6 = 0;
            if (!parse_operatorBAnd(&__node_6))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_operatorBAnd, "operatorBAnd");
                  }
                return false;
              }
            shiftExpression_ast *__node_7 = 0;
            if (!parse_shiftExpression(&__node_7))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_shiftExpression, "shiftExpression");
                  }
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

  bool parser::parse_andorExpression(andorExpression_ast **yynode)
  {
    *yynode = create<andorExpression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BEGIN
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_KEYWORD_NOT
        || yytoken == Token_LPAREN
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_UNARY_MINUS
        || yytoken == Token_UNARY_PLUS
        || yytoken == Token_NOT
        || yytoken == Token_BNOT
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE)
      {
        notExpression_ast *__node_8 = 0;
        if (!parse_notExpression(&__node_8))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_notExpression, "notExpression");
              }
            return false;
          }
        while (yytoken == Token_OR
               || yytoken == Token_AND)
          {
            if (yytoken == Token_AND)
              {
                keywordAnd_ast *__node_9 = 0;
                if (!parse_keywordAnd(&__node_9))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_keywordAnd, "keywordAnd");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_OR)
              {
                keywordOr_ast *__node_10 = 0;
                if (!parse_keywordOr(&__node_10))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_keywordOr, "keywordOr");
                      }
                    return false;
                  }
              }
            else
              {
                return false;
              }
            notExpression_ast *__node_11 = 0;
            if (!parse_notExpression(&__node_11))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_notExpression, "notExpression");
                  }
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

  bool parser::parse_arrayAccess(arrayAccess_ast **yynode)
  {
    *yynode = create<arrayAccess_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_LBRACK_ARRAY_ACCESS)
      {
        if (yytoken != Token_LBRACK_ARRAY_ACCESS)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_LBRACK_ARRAY_ACCESS, "[");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_BEGIN
            || yytoken == Token_RETURN
            || yytoken == Token_BREAK
            || yytoken == Token_NEXT
            || yytoken == Token_NIL
            || yytoken == Token_TRUE
            || yytoken == Token_FALSE
            || yytoken == Token_FILE
            || yytoken == Token_LINE
            || yytoken == Token_SELF
            || yytoken == Token_SUPER
            || yytoken == Token_RETRY
            || yytoken == Token_YIELD
            || yytoken == Token_DEFINED
            || yytoken == Token_REDO
            || yytoken == Token_IF
            || yytoken == Token_CASE
            || yytoken == Token_UNTIL
            || yytoken == Token_WHILE
            || yytoken == Token_FOR
            || yytoken == Token_MODULE
            || yytoken == Token_DEF
            || yytoken == Token_CLASS
            || yytoken == Token_UNLESS
            || yytoken == Token_KEYWORD_NOT
            || yytoken == Token_LPAREN
            || yytoken == Token_LEADING_TWO_COLON
            || yytoken == Token_LBRACK
            || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
            || yytoken == Token_LCURLY_HASH
            || yytoken == Token_REST_ARG_PREFIX
            || yytoken == Token_UNARY_MINUS
            || yytoken == Token_UNARY_PLUS
            || yytoken == Token_NOT
            || yytoken == Token_BNOT
            || yytoken == Token_IDENTIFIER
            || yytoken == Token_CONSTANT
            || yytoken == Token_FUNCTION
            || yytoken == Token_GLOBAL_VARIABLE
            || yytoken == Token_INSTANCE_VARIABLE
            || yytoken == Token_CLASS_VARIABLE
            || yytoken == Token_DOUBLE_QUOTED_STRING
            || yytoken == Token_SINGLE_QUOTED_STRING
            || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_REGEX
            || yytoken == Token_COMMAND_OUTPUT
            || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_HERE_DOC_BEGIN
            || yytoken == Token_W_ARRAY
            || yytoken == Token_INTEGER
            || yytoken == Token_HEX
            || yytoken == Token_OCTAL
            || yytoken == Token_BINARY
            || yytoken == Token_FLOAT
            || yytoken == Token_ASCII_VALUE)
          {
            arrayReferenceArgument_ast *__node_12 = 0;
            if (!parse_arrayReferenceArgument(&__node_12))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_arrayReferenceArgument, "arrayReferenceArgument");
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
        if (yytoken != Token_RBRACK)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_RBRACK, "]");
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

  bool parser::parse_arrayExpression(arrayExpression_ast **yynode)
  {
    *yynode = create<arrayExpression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_LBRACK)
      {
        if (yytoken != Token_LBRACK)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_LBRACK, "[");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_BEGIN
            || yytoken == Token_RETURN
            || yytoken == Token_BREAK
            || yytoken == Token_NEXT
            || yytoken == Token_NIL
            || yytoken == Token_TRUE
            || yytoken == Token_FALSE
            || yytoken == Token_FILE
            || yytoken == Token_LINE
            || yytoken == Token_SELF
            || yytoken == Token_SUPER
            || yytoken == Token_RETRY
            || yytoken == Token_YIELD
            || yytoken == Token_DEFINED
            || yytoken == Token_REDO
            || yytoken == Token_IF
            || yytoken == Token_CASE
            || yytoken == Token_UNTIL
            || yytoken == Token_WHILE
            || yytoken == Token_FOR
            || yytoken == Token_MODULE
            || yytoken == Token_DEF
            || yytoken == Token_CLASS
            || yytoken == Token_UNLESS
            || yytoken == Token_KEYWORD_NOT
            || yytoken == Token_LPAREN
            || yytoken == Token_LEADING_TWO_COLON
            || yytoken == Token_LBRACK
            || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
            || yytoken == Token_LCURLY_HASH
            || yytoken == Token_REST_ARG_PREFIX
            || yytoken == Token_UNARY_MINUS
            || yytoken == Token_UNARY_PLUS
            || yytoken == Token_NOT
            || yytoken == Token_BNOT
            || yytoken == Token_IDENTIFIER
            || yytoken == Token_CONSTANT
            || yytoken == Token_FUNCTION
            || yytoken == Token_GLOBAL_VARIABLE
            || yytoken == Token_INSTANCE_VARIABLE
            || yytoken == Token_CLASS_VARIABLE
            || yytoken == Token_DOUBLE_QUOTED_STRING
            || yytoken == Token_SINGLE_QUOTED_STRING
            || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_REGEX
            || yytoken == Token_COMMAND_OUTPUT
            || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_HERE_DOC_BEGIN
            || yytoken == Token_W_ARRAY
            || yytoken == Token_INTEGER
            || yytoken == Token_HEX
            || yytoken == Token_OCTAL
            || yytoken == Token_BINARY
            || yytoken == Token_FLOAT
            || yytoken == Token_ASCII_VALUE)
          {
            arrayReferenceArgument_ast *__node_13 = 0;
            if (!parse_arrayReferenceArgument(&__node_13))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_arrayReferenceArgument, "arrayReferenceArgument");
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
        if (yytoken != Token_RBRACK)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_RBRACK, "]");
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

  bool parser::parse_arrayReferenceArgument(arrayReferenceArgument_ast **yynode)
  {
    *yynode = create<arrayReferenceArgument_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BEGIN
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_KEYWORD_NOT
        || yytoken == Token_LPAREN
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_REST_ARG_PREFIX
        || yytoken == Token_UNARY_MINUS
        || yytoken == Token_UNARY_PLUS
        || yytoken == Token_NOT
        || yytoken == Token_BNOT
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE)
      {
        if (yytoken == Token_BEGIN
            || yytoken == Token_RETURN
            || yytoken == Token_BREAK
            || yytoken == Token_NEXT
            || yytoken == Token_NIL
            || yytoken == Token_TRUE
            || yytoken == Token_FALSE
            || yytoken == Token_FILE
            || yytoken == Token_LINE
            || yytoken == Token_SELF
            || yytoken == Token_SUPER
            || yytoken == Token_RETRY
            || yytoken == Token_YIELD
            || yytoken == Token_DEFINED
            || yytoken == Token_REDO
            || yytoken == Token_IF
            || yytoken == Token_CASE
            || yytoken == Token_UNTIL
            || yytoken == Token_WHILE
            || yytoken == Token_FOR
            || yytoken == Token_MODULE
            || yytoken == Token_DEF
            || yytoken == Token_CLASS
            || yytoken == Token_UNLESS
            || yytoken == Token_KEYWORD_NOT
            || yytoken == Token_LPAREN
            || yytoken == Token_LEADING_TWO_COLON
            || yytoken == Token_LBRACK
            || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
            || yytoken == Token_LCURLY_HASH
            || yytoken == Token_UNARY_MINUS
            || yytoken == Token_UNARY_PLUS
            || yytoken == Token_NOT
            || yytoken == Token_BNOT
            || yytoken == Token_IDENTIFIER
            || yytoken == Token_CONSTANT
            || yytoken == Token_FUNCTION
            || yytoken == Token_GLOBAL_VARIABLE
            || yytoken == Token_INSTANCE_VARIABLE
            || yytoken == Token_CLASS_VARIABLE
            || yytoken == Token_DOUBLE_QUOTED_STRING
            || yytoken == Token_SINGLE_QUOTED_STRING
            || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_REGEX
            || yytoken == Token_COMMAND_OUTPUT
            || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_HERE_DOC_BEGIN
            || yytoken == Token_W_ARRAY
            || yytoken == Token_INTEGER
            || yytoken == Token_HEX
            || yytoken == Token_OCTAL
            || yytoken == Token_BINARY
            || yytoken == Token_FLOAT
            || yytoken == Token_ASCII_VALUE)
          {
            keyValuePair_ast *__node_14 = 0;
            if (!parse_keyValuePair(&__node_14))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_keyValuePair, "keyValuePair");
                  }
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

                if ((yytoken == Token_ASSIGN) || (yytoken == Token_RBRACK))
                  break;
                if (yytoken == Token_REST_ARG_PREFIX)
                  {
                    if (yytoken != Token_REST_ARG_PREFIX)
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_token(yytoken, Token_REST_ARG_PREFIX, "*");
                          }
                        return false;
                      }
                    yylex();

                    seen_star = true;
                  }
                else if (true /*epsilon*/)
                  {
                    seen_star = false;
                  }
                else
                  {
                    return false;
                  }
                keyValuePair_ast *__node_15 = 0;
                if (!parse_keyValuePair(&__node_15))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_keyValuePair, "keyValuePair");
                      }
                    return false;
                  }
                if (seen_star)
                  break;
              }
          }
        else if (yytoken == Token_REST_ARG_PREFIX)
          {
            if (yytoken != Token_REST_ARG_PREFIX)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_REST_ARG_PREFIX, "*");
                  }
                return false;
              }
            yylex();

            expression_ast *__node_16 = 0;
            if (!parse_expression(&__node_16))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_expression, "expression");
                  }
                return false;
              }
            if (yytoken == Token_LINE_BREAK)
              {
                if (yytoken != Token_LINE_BREAK)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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

  bool parser::parse_assignmentExpression(assignmentExpression_ast **yynode)
  {
    *yynode = create<assignmentExpression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BEGIN
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_LPAREN
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_UNARY_MINUS
        || yytoken == Token_UNARY_PLUS
        || yytoken == Token_NOT
        || yytoken == Token_BNOT
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE)
      {
        rangeExpression_ast *__node_17 = 0;
        if (!parse_rangeExpression(&__node_17))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_rangeExpression, "rangeExpression");
              }
            return false;
          }
        while (yytoken == Token_ASSIGN
               || yytoken == Token_ASSIGN_WITH_NO_LEADING_SPACE
               || yytoken == Token_PLUS_ASSIGN
               || yytoken == Token_MINUS_ASSIGN
               || yytoken == Token_STAR_ASSIGN
               || yytoken == Token_DIV_ASSIGN
               || yytoken == Token_MOD_ASSIGN
               || yytoken == Token_POWER_ASSIGN
               || yytoken == Token_BAND_ASSIGN
               || yytoken == Token_BXOR_ASSIGN
               || yytoken == Token_BOR_ASSIGN
               || yytoken == Token_LEFT_SHIFT_ASSIGN
               || yytoken == Token_RIGHT_SHIFT_ASSIGN
               || yytoken == Token_LOGICAL_AND_ASSIGN
               || yytoken == Token_LOGICAL_OR_ASSIGN)
          {
            if (yytoken == Token_ASSIGN
                || yytoken == Token_ASSIGN_WITH_NO_LEADING_SPACE)
              {
                operatorAssign_ast *__node_18 = 0;
                if (!parse_operatorAssign(&__node_18))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorAssign, "operatorAssign");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_PLUS_ASSIGN)
              {
                operatorPlusAssign_ast *__node_19 = 0;
                if (!parse_operatorPlusAssign(&__node_19))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorPlusAssign, "operatorPlusAssign");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_MINUS_ASSIGN)
              {
                operatorMinusAssign_ast *__node_20 = 0;
                if (!parse_operatorMinusAssign(&__node_20))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorMinusAssign, "operatorMinusAssign");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_STAR_ASSIGN)
              {
                operatorStarAssign_ast *__node_21 = 0;
                if (!parse_operatorStarAssign(&__node_21))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorStarAssign, "operatorStarAssign");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_DIV_ASSIGN)
              {
                operatorDivAssign_ast *__node_22 = 0;
                if (!parse_operatorDivAssign(&__node_22))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorDivAssign, "operatorDivAssign");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_MOD_ASSIGN)
              {
                operatorModAssign_ast *__node_23 = 0;
                if (!parse_operatorModAssign(&__node_23))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorModAssign, "operatorModAssign");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_POWER_ASSIGN)
              {
                operatorPowerAssign_ast *__node_24 = 0;
                if (!parse_operatorPowerAssign(&__node_24))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorPowerAssign, "operatorPowerAssign");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_BAND_ASSIGN)
              {
                operatorBAndAssign_ast *__node_25 = 0;
                if (!parse_operatorBAndAssign(&__node_25))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorBAndAssign, "operatorBAndAssign");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_BXOR_ASSIGN)
              {
                operatorBXorAssign_ast *__node_26 = 0;
                if (!parse_operatorBXorAssign(&__node_26))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorBXorAssign, "operatorBXorAssign");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_BOR_ASSIGN)
              {
                operatorBorAssign_ast *__node_27 = 0;
                if (!parse_operatorBorAssign(&__node_27))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorBorAssign, "operatorBorAssign");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_LEFT_SHIFT_ASSIGN)
              {
                operatorLeftShiftAssign_ast *__node_28 = 0;
                if (!parse_operatorLeftShiftAssign(&__node_28))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorLeftShiftAssign, "operatorLeftShiftAssign");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_RIGHT_SHIFT_ASSIGN)
              {
                operatorRightShiftAssign_ast *__node_29 = 0;
                if (!parse_operatorRightShiftAssign(&__node_29))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorRightShiftAssign, "operatorRightShiftAssign");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_LOGICAL_AND_ASSIGN)
              {
                operatorLogicalAndAssign_ast *__node_30 = 0;
                if (!parse_operatorLogicalAndAssign(&__node_30))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorLogicalAndAssign, "operatorLogicalAndAssign");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_LOGICAL_OR_ASSIGN)
              {
                operatorLogicalOrAssign_ast *__node_31 = 0;
                if (!parse_operatorLogicalOrAssign(&__node_31))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorLogicalOrAssign, "operatorLogicalOrAssign");
                      }
                    return false;
                  }
              }
            else
              {
                return false;
              }
            if (yytoken == Token_REST_ARG_PREFIX)
              {
                if (yytoken != Token_REST_ARG_PREFIX)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_REST_ARG_PREFIX, "*");
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
            rangeExpression_ast *__node_32 = 0;
            if (!parse_rangeExpression(&__node_32))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_rangeExpression, "rangeExpression");
                  }
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

  bool parser::parse_blockContent(blockContent_ast **yynode)
  {
    *yynode = create<blockContent_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_UNDEF
        || yytoken == Token_ALIAS
        || yytoken == Token_BEGIN
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_KEYWORD_NOT
        || yytoken == Token_END_UPCASE
        || yytoken == Token_BEGIN_UPCASE
        || yytoken == Token_LPAREN
        || yytoken == Token_SEMI
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_REST_ARG_PREFIX
        || yytoken == Token_BOR
        || yytoken == Token_LOGICAL_OR
        || yytoken == Token_UNARY_MINUS
        || yytoken == Token_UNARY_PLUS
        || yytoken == Token_NOT
        || yytoken == Token_BNOT
        || yytoken == Token_LINE_BREAK
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE || yytoken == Token_END
        || yytoken == Token_RCURLY)
      {
        if (yytoken == Token_BOR)
          {
            if (yytoken != Token_BOR)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_BOR, "|");
                  }
                return false;
              }
            yylex();

            if (yytoken == Token_BEGIN
                || yytoken == Token_RETURN
                || yytoken == Token_BREAK
                || yytoken == Token_NEXT
                || yytoken == Token_NIL
                || yytoken == Token_TRUE
                || yytoken == Token_FALSE
                || yytoken == Token_FILE
                || yytoken == Token_LINE
                || yytoken == Token_SELF
                || yytoken == Token_SUPER
                || yytoken == Token_RETRY
                || yytoken == Token_YIELD
                || yytoken == Token_DEFINED
                || yytoken == Token_REDO
                || yytoken == Token_IF
                || yytoken == Token_CASE
                || yytoken == Token_UNTIL
                || yytoken == Token_WHILE
                || yytoken == Token_FOR
                || yytoken == Token_MODULE
                || yytoken == Token_DEF
                || yytoken == Token_CLASS
                || yytoken == Token_UNLESS
                || yytoken == Token_LPAREN
                || yytoken == Token_LEADING_TWO_COLON
                || yytoken == Token_LBRACK
                || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
                || yytoken == Token_LCURLY_HASH
                || yytoken == Token_REST_ARG_PREFIX
                || yytoken == Token_IDENTIFIER
                || yytoken == Token_CONSTANT
                || yytoken == Token_FUNCTION
                || yytoken == Token_GLOBAL_VARIABLE
                || yytoken == Token_INSTANCE_VARIABLE
                || yytoken == Token_CLASS_VARIABLE
                || yytoken == Token_DOUBLE_QUOTED_STRING
                || yytoken == Token_SINGLE_QUOTED_STRING
                || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_REGEX
                || yytoken == Token_COMMAND_OUTPUT
                || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_HERE_DOC_BEGIN
                || yytoken == Token_W_ARRAY
                || yytoken == Token_INTEGER
                || yytoken == Token_HEX
                || yytoken == Token_OCTAL
                || yytoken == Token_BINARY
                || yytoken == Token_FLOAT
                || yytoken == Token_ASCII_VALUE)
              {
                block_vars_ast *__node_33 = 0;
                if (!parse_block_vars(&__node_33))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_block_vars, "block_vars");
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
            if (yytoken != Token_BOR)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_BOR, "|");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_LOGICAL_OR)
          {
            if (yytoken != Token_LOGICAL_OR)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LOGICAL_OR, "||");
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
        if (yytoken == Token_UNDEF
            || yytoken == Token_ALIAS
            || yytoken == Token_BEGIN
            || yytoken == Token_RETURN
            || yytoken == Token_BREAK
            || yytoken == Token_NEXT
            || yytoken == Token_NIL
            || yytoken == Token_TRUE
            || yytoken == Token_FALSE
            || yytoken == Token_FILE
            || yytoken == Token_LINE
            || yytoken == Token_SELF
            || yytoken == Token_SUPER
            || yytoken == Token_RETRY
            || yytoken == Token_YIELD
            || yytoken == Token_DEFINED
            || yytoken == Token_REDO
            || yytoken == Token_IF
            || yytoken == Token_CASE
            || yytoken == Token_UNTIL
            || yytoken == Token_WHILE
            || yytoken == Token_FOR
            || yytoken == Token_MODULE
            || yytoken == Token_DEF
            || yytoken == Token_CLASS
            || yytoken == Token_UNLESS
            || yytoken == Token_KEYWORD_NOT
            || yytoken == Token_END_UPCASE
            || yytoken == Token_BEGIN_UPCASE
            || yytoken == Token_LPAREN
            || yytoken == Token_SEMI
            || yytoken == Token_LEADING_TWO_COLON
            || yytoken == Token_LBRACK
            || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
            || yytoken == Token_LCURLY_HASH
            || yytoken == Token_REST_ARG_PREFIX
            || yytoken == Token_UNARY_MINUS
            || yytoken == Token_UNARY_PLUS
            || yytoken == Token_NOT
            || yytoken == Token_BNOT
            || yytoken == Token_LINE_BREAK
            || yytoken == Token_IDENTIFIER
            || yytoken == Token_CONSTANT
            || yytoken == Token_FUNCTION
            || yytoken == Token_GLOBAL_VARIABLE
            || yytoken == Token_INSTANCE_VARIABLE
            || yytoken == Token_CLASS_VARIABLE
            || yytoken == Token_DOUBLE_QUOTED_STRING
            || yytoken == Token_SINGLE_QUOTED_STRING
            || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_REGEX
            || yytoken == Token_COMMAND_OUTPUT
            || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_HERE_DOC_BEGIN
            || yytoken == Token_W_ARRAY
            || yytoken == Token_INTEGER
            || yytoken == Token_HEX
            || yytoken == Token_OCTAL
            || yytoken == Token_BINARY
            || yytoken == Token_FLOAT
            || yytoken == Token_ASCII_VALUE)
          {
            compoundStatement_ast *__node_34 = 0;
            if (!parse_compoundStatement(&__node_34))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_compoundStatement, "compoundStatement");
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
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_blockMethodDefinitionArgument(blockMethodDefinitionArgument_ast **yynode)
  {
    *yynode = create<blockMethodDefinitionArgument_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if ((yytoken == Token_BAND) && ( expect_array_or_block_arguments ))
      {
        if (yytoken != Token_BAND)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_BAND, "&");
              }
            return false;
          }
        yylex();

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
            yylex();

          }
        else if (yytoken == Token_FUNCTION)
          {
            if (yytoken != Token_FUNCTION)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_FUNCTION, "function");
                  }
                return false;
              }
            yylex();

          }
        else
          {
            return false;
          }
        expect_array_or_block_arguments = false;
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_blockMethodInvocationArgument(blockMethodInvocationArgument_ast **yynode)
  {
    *yynode = create<blockMethodInvocationArgument_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BLOCK_ARG_PREFIX)
      {
        if (yytoken != Token_BLOCK_ARG_PREFIX)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_BLOCK_ARG_PREFIX, "block argument prefix");
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
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_block_var(block_var_ast **yynode)
  {
    *yynode = create<block_var_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BEGIN
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_LPAREN
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_REST_ARG_PREFIX
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE)
      {
        bool block_errors_1 = block_errors(true);
        std::size_t try_start_token_1 = token_stream->index() - 1;
        parser_state *try_start_state_1 = copy_current_state();
        {
          if (yytoken != Token_LPAREN)
            goto __catch_1;
          yylex();

          elementReference_ast *__node_36 = 0;
          if (!parse_elementReference(&__node_36))
            {
              goto __catch_1;
            }
          while (yytoken == Token_COMMA)
            {
              if (yytoken != Token_COMMA)
                goto __catch_1;
              yylex();

              if (yytoken == Token_RPAREN)
                break;
              if (yytoken == Token_REST_ARG_PREFIX)
                {
                  if (yytoken != Token_REST_ARG_PREFIX)
                    goto __catch_1;
                  yylex();

                  seen_star = true;
                }
              else if (true /*epsilon*/)
                {
                  seen_star = false;
                }
              else
                {
                  goto __catch_1;
                }
              elementReference_ast *__node_37 = 0;
              if (!parse_elementReference(&__node_37))
                {
                  goto __catch_1;
                }
              if (seen_star)
                break;
            }
          if (yytoken != Token_RPAREN)
            goto __catch_1;
          yylex();

        }
        block_errors(block_errors_1);
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
            block_errors(block_errors_1);
            rewind(try_start_token_1);

            if (yytoken == Token_REST_ARG_PREFIX)
              {
                if (yytoken != Token_REST_ARG_PREFIX)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_REST_ARG_PREFIX, "*");
                      }
                    return false;
                  }
                yylex();

                if (yytoken == Token_BEGIN
                    || yytoken == Token_RETURN
                    || yytoken == Token_BREAK
                    || yytoken == Token_NEXT
                    || yytoken == Token_NIL
                    || yytoken == Token_TRUE
                    || yytoken == Token_FALSE
                    || yytoken == Token_FILE
                    || yytoken == Token_LINE
                    || yytoken == Token_SELF
                    || yytoken == Token_SUPER
                    || yytoken == Token_RETRY
                    || yytoken == Token_YIELD
                    || yytoken == Token_DEFINED
                    || yytoken == Token_REDO
                    || yytoken == Token_IF
                    || yytoken == Token_CASE
                    || yytoken == Token_UNTIL
                    || yytoken == Token_WHILE
                    || yytoken == Token_FOR
                    || yytoken == Token_MODULE
                    || yytoken == Token_DEF
                    || yytoken == Token_CLASS
                    || yytoken == Token_UNLESS
                    || yytoken == Token_LPAREN
                    || yytoken == Token_LEADING_TWO_COLON
                    || yytoken == Token_LBRACK
                    || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
                    || yytoken == Token_LCURLY_HASH
                    || yytoken == Token_IDENTIFIER
                    || yytoken == Token_CONSTANT
                    || yytoken == Token_FUNCTION
                    || yytoken == Token_GLOBAL_VARIABLE
                    || yytoken == Token_INSTANCE_VARIABLE
                    || yytoken == Token_CLASS_VARIABLE
                    || yytoken == Token_DOUBLE_QUOTED_STRING
                    || yytoken == Token_SINGLE_QUOTED_STRING
                    || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
                    || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
                    || yytoken == Token_REGEX
                    || yytoken == Token_COMMAND_OUTPUT
                    || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
                    || yytoken == Token_HERE_DOC_BEGIN
                    || yytoken == Token_W_ARRAY
                    || yytoken == Token_INTEGER
                    || yytoken == Token_HEX
                    || yytoken == Token_OCTAL
                    || yytoken == Token_BINARY
                    || yytoken == Token_FLOAT
                    || yytoken == Token_ASCII_VALUE)
                  {
                    elementReference_ast *__node_38 = 0;
                    if (!parse_elementReference(&__node_38))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_elementReference, "elementReference");
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
              }
            else if (yytoken == Token_BEGIN
                     || yytoken == Token_RETURN
                     || yytoken == Token_BREAK
                     || yytoken == Token_NEXT
                     || yytoken == Token_NIL
                     || yytoken == Token_TRUE
                     || yytoken == Token_FALSE
                     || yytoken == Token_FILE
                     || yytoken == Token_LINE
                     || yytoken == Token_SELF
                     || yytoken == Token_SUPER
                     || yytoken == Token_RETRY
                     || yytoken == Token_YIELD
                     || yytoken == Token_DEFINED
                     || yytoken == Token_REDO
                     || yytoken == Token_IF
                     || yytoken == Token_CASE
                     || yytoken == Token_UNTIL
                     || yytoken == Token_WHILE
                     || yytoken == Token_FOR
                     || yytoken == Token_MODULE
                     || yytoken == Token_DEF
                     || yytoken == Token_CLASS
                     || yytoken == Token_UNLESS
                     || yytoken == Token_LPAREN
                     || yytoken == Token_LEADING_TWO_COLON
                     || yytoken == Token_LBRACK
                     || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
                     || yytoken == Token_LCURLY_HASH
                     || yytoken == Token_IDENTIFIER
                     || yytoken == Token_CONSTANT
                     || yytoken == Token_FUNCTION
                     || yytoken == Token_GLOBAL_VARIABLE
                     || yytoken == Token_INSTANCE_VARIABLE
                     || yytoken == Token_CLASS_VARIABLE
                     || yytoken == Token_DOUBLE_QUOTED_STRING
                     || yytoken == Token_SINGLE_QUOTED_STRING
                     || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
                     || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
                     || yytoken == Token_REGEX
                     || yytoken == Token_COMMAND_OUTPUT
                     || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
                     || yytoken == Token_HERE_DOC_BEGIN
                     || yytoken == Token_W_ARRAY
                     || yytoken == Token_INTEGER
                     || yytoken == Token_HEX
                     || yytoken == Token_OCTAL
                     || yytoken == Token_BINARY
                     || yytoken == Token_FLOAT
                     || yytoken == Token_ASCII_VALUE)
              {
                elementReference_ast *__node_39 = 0;
                if (!parse_elementReference(&__node_39))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_elementReference, "elementReference");
                      }
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

  bool parser::parse_block_vars(block_vars_ast **yynode)
  {
    *yynode = create<block_vars_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BEGIN
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_LPAREN
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_REST_ARG_PREFIX
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE)
      {
        block_var_ast *__node_40 = 0;
        if (!parse_block_var(&__node_40))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_block_var, "block_var");
              }
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

            if ((yytoken == Token_BOR) || (yytoken == Token_IN))
              break;
            block_var_ast *__node_41 = 0;
            if (!parse_block_var(&__node_41))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_block_var, "block_var");
                  }
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

  bool parser::parse_bodyStatement(bodyStatement_ast **yynode)
  {
    *yynode = create<bodyStatement_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_ELSE
        || yytoken == Token_RESCUE
        || yytoken == Token_ENSURE
        || yytoken == Token_UNDEF
        || yytoken == Token_ALIAS
        || yytoken == Token_BEGIN
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_KEYWORD_NOT
        || yytoken == Token_END_UPCASE
        || yytoken == Token_BEGIN_UPCASE
        || yytoken == Token_LPAREN
        || yytoken == Token_SEMI
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_REST_ARG_PREFIX
        || yytoken == Token_UNARY_MINUS
        || yytoken == Token_UNARY_PLUS
        || yytoken == Token_NOT
        || yytoken == Token_BNOT
        || yytoken == Token_LINE_BREAK
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE || yytoken == Token_END)
      {
        if (yytoken == Token_UNDEF
            || yytoken == Token_ALIAS
            || yytoken == Token_BEGIN
            || yytoken == Token_RETURN
            || yytoken == Token_BREAK
            || yytoken == Token_NEXT
            || yytoken == Token_NIL
            || yytoken == Token_TRUE
            || yytoken == Token_FALSE
            || yytoken == Token_FILE
            || yytoken == Token_LINE
            || yytoken == Token_SELF
            || yytoken == Token_SUPER
            || yytoken == Token_RETRY
            || yytoken == Token_YIELD
            || yytoken == Token_DEFINED
            || yytoken == Token_REDO
            || yytoken == Token_IF
            || yytoken == Token_CASE
            || yytoken == Token_UNTIL
            || yytoken == Token_WHILE
            || yytoken == Token_FOR
            || yytoken == Token_MODULE
            || yytoken == Token_DEF
            || yytoken == Token_CLASS
            || yytoken == Token_UNLESS
            || yytoken == Token_KEYWORD_NOT
            || yytoken == Token_END_UPCASE
            || yytoken == Token_BEGIN_UPCASE
            || yytoken == Token_LPAREN
            || yytoken == Token_SEMI
            || yytoken == Token_LEADING_TWO_COLON
            || yytoken == Token_LBRACK
            || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
            || yytoken == Token_LCURLY_HASH
            || yytoken == Token_REST_ARG_PREFIX
            || yytoken == Token_UNARY_MINUS
            || yytoken == Token_UNARY_PLUS
            || yytoken == Token_NOT
            || yytoken == Token_BNOT
            || yytoken == Token_LINE_BREAK
            || yytoken == Token_IDENTIFIER
            || yytoken == Token_CONSTANT
            || yytoken == Token_FUNCTION
            || yytoken == Token_GLOBAL_VARIABLE
            || yytoken == Token_INSTANCE_VARIABLE
            || yytoken == Token_CLASS_VARIABLE
            || yytoken == Token_DOUBLE_QUOTED_STRING
            || yytoken == Token_SINGLE_QUOTED_STRING
            || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_REGEX
            || yytoken == Token_COMMAND_OUTPUT
            || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_HERE_DOC_BEGIN
            || yytoken == Token_W_ARRAY
            || yytoken == Token_INTEGER
            || yytoken == Token_HEX
            || yytoken == Token_OCTAL
            || yytoken == Token_BINARY
            || yytoken == Token_FLOAT
            || yytoken == Token_ASCII_VALUE)
          {
            compoundStatement_ast *__node_42 = 0;
            if (!parse_compoundStatement(&__node_42))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_compoundStatement, "compoundStatement");
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
        while (yytoken == Token_RESCUE)
          {
            if (yytoken != Token_RESCUE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_RESCUE, "rescue");
                  }
                return false;
              }
            yylex();

            exceptionList_ast *__node_43 = 0;
            if (!parse_exceptionList(&__node_43))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_exceptionList, "exceptionList");
                  }
                return false;
              }
            thenOrTerminalOrColon_ast *__node_44 = 0;
            if (!parse_thenOrTerminalOrColon(&__node_44))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_thenOrTerminalOrColon, "thenOrTerminalOrColon");
                  }
                return false;
              }
            if (yytoken == Token_UNDEF
                || yytoken == Token_ALIAS
                || yytoken == Token_BEGIN
                || yytoken == Token_RETURN
                || yytoken == Token_BREAK
                || yytoken == Token_NEXT
                || yytoken == Token_NIL
                || yytoken == Token_TRUE
                || yytoken == Token_FALSE
                || yytoken == Token_FILE
                || yytoken == Token_LINE
                || yytoken == Token_SELF
                || yytoken == Token_SUPER
                || yytoken == Token_RETRY
                || yytoken == Token_YIELD
                || yytoken == Token_DEFINED
                || yytoken == Token_REDO
                || yytoken == Token_IF
                || yytoken == Token_CASE
                || yytoken == Token_UNTIL
                || yytoken == Token_WHILE
                || yytoken == Token_FOR
                || yytoken == Token_MODULE
                || yytoken == Token_DEF
                || yytoken == Token_CLASS
                || yytoken == Token_UNLESS
                || yytoken == Token_KEYWORD_NOT
                || yytoken == Token_END_UPCASE
                || yytoken == Token_BEGIN_UPCASE
                || yytoken == Token_LPAREN
                || yytoken == Token_SEMI
                || yytoken == Token_LEADING_TWO_COLON
                || yytoken == Token_LBRACK
                || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
                || yytoken == Token_LCURLY_HASH
                || yytoken == Token_REST_ARG_PREFIX
                || yytoken == Token_UNARY_MINUS
                || yytoken == Token_UNARY_PLUS
                || yytoken == Token_NOT
                || yytoken == Token_BNOT
                || yytoken == Token_LINE_BREAK
                || yytoken == Token_IDENTIFIER
                || yytoken == Token_CONSTANT
                || yytoken == Token_FUNCTION
                || yytoken == Token_GLOBAL_VARIABLE
                || yytoken == Token_INSTANCE_VARIABLE
                || yytoken == Token_CLASS_VARIABLE
                || yytoken == Token_DOUBLE_QUOTED_STRING
                || yytoken == Token_SINGLE_QUOTED_STRING
                || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_REGEX
                || yytoken == Token_COMMAND_OUTPUT
                || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_HERE_DOC_BEGIN
                || yytoken == Token_W_ARRAY
                || yytoken == Token_INTEGER
                || yytoken == Token_HEX
                || yytoken == Token_OCTAL
                || yytoken == Token_BINARY
                || yytoken == Token_FLOAT
                || yytoken == Token_ASCII_VALUE)
              {
                compoundStatement_ast *__node_45 = 0;
                if (!parse_compoundStatement(&__node_45))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_compoundStatement, "compoundStatement");
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
          }
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

            if (yytoken == Token_UNDEF
                || yytoken == Token_ALIAS
                || yytoken == Token_BEGIN
                || yytoken == Token_RETURN
                || yytoken == Token_BREAK
                || yytoken == Token_NEXT
                || yytoken == Token_NIL
                || yytoken == Token_TRUE
                || yytoken == Token_FALSE
                || yytoken == Token_FILE
                || yytoken == Token_LINE
                || yytoken == Token_SELF
                || yytoken == Token_SUPER
                || yytoken == Token_RETRY
                || yytoken == Token_YIELD
                || yytoken == Token_DEFINED
                || yytoken == Token_REDO
                || yytoken == Token_IF
                || yytoken == Token_CASE
                || yytoken == Token_UNTIL
                || yytoken == Token_WHILE
                || yytoken == Token_FOR
                || yytoken == Token_MODULE
                || yytoken == Token_DEF
                || yytoken == Token_CLASS
                || yytoken == Token_UNLESS
                || yytoken == Token_KEYWORD_NOT
                || yytoken == Token_END_UPCASE
                || yytoken == Token_BEGIN_UPCASE
                || yytoken == Token_LPAREN
                || yytoken == Token_SEMI
                || yytoken == Token_LEADING_TWO_COLON
                || yytoken == Token_LBRACK
                || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
                || yytoken == Token_LCURLY_HASH
                || yytoken == Token_REST_ARG_PREFIX
                || yytoken == Token_UNARY_MINUS
                || yytoken == Token_UNARY_PLUS
                || yytoken == Token_NOT
                || yytoken == Token_BNOT
                || yytoken == Token_LINE_BREAK
                || yytoken == Token_IDENTIFIER
                || yytoken == Token_CONSTANT
                || yytoken == Token_FUNCTION
                || yytoken == Token_GLOBAL_VARIABLE
                || yytoken == Token_INSTANCE_VARIABLE
                || yytoken == Token_CLASS_VARIABLE
                || yytoken == Token_DOUBLE_QUOTED_STRING
                || yytoken == Token_SINGLE_QUOTED_STRING
                || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_REGEX
                || yytoken == Token_COMMAND_OUTPUT
                || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_HERE_DOC_BEGIN
                || yytoken == Token_W_ARRAY
                || yytoken == Token_INTEGER
                || yytoken == Token_HEX
                || yytoken == Token_OCTAL
                || yytoken == Token_BINARY
                || yytoken == Token_FLOAT
                || yytoken == Token_ASCII_VALUE)
              {
                compoundStatement_ast *__node_46 = 0;
                if (!parse_compoundStatement(&__node_46))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_compoundStatement, "compoundStatement");
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
          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        if (yytoken == Token_ENSURE)
          {
            if (yytoken != Token_ENSURE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_ENSURE, "ensure");
                  }
                return false;
              }
            yylex();

            if (yytoken == Token_UNDEF
                || yytoken == Token_ALIAS
                || yytoken == Token_BEGIN
                || yytoken == Token_RETURN
                || yytoken == Token_BREAK
                || yytoken == Token_NEXT
                || yytoken == Token_NIL
                || yytoken == Token_TRUE
                || yytoken == Token_FALSE
                || yytoken == Token_FILE
                || yytoken == Token_LINE
                || yytoken == Token_SELF
                || yytoken == Token_SUPER
                || yytoken == Token_RETRY
                || yytoken == Token_YIELD
                || yytoken == Token_DEFINED
                || yytoken == Token_REDO
                || yytoken == Token_IF
                || yytoken == Token_CASE
                || yytoken == Token_UNTIL
                || yytoken == Token_WHILE
                || yytoken == Token_FOR
                || yytoken == Token_MODULE
                || yytoken == Token_DEF
                || yytoken == Token_CLASS
                || yytoken == Token_UNLESS
                || yytoken == Token_KEYWORD_NOT
                || yytoken == Token_END_UPCASE
                || yytoken == Token_BEGIN_UPCASE
                || yytoken == Token_LPAREN
                || yytoken == Token_SEMI
                || yytoken == Token_LEADING_TWO_COLON
                || yytoken == Token_LBRACK
                || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
                || yytoken == Token_LCURLY_HASH
                || yytoken == Token_REST_ARG_PREFIX
                || yytoken == Token_UNARY_MINUS
                || yytoken == Token_UNARY_PLUS
                || yytoken == Token_NOT
                || yytoken == Token_BNOT
                || yytoken == Token_LINE_BREAK
                || yytoken == Token_IDENTIFIER
                || yytoken == Token_CONSTANT
                || yytoken == Token_FUNCTION
                || yytoken == Token_GLOBAL_VARIABLE
                || yytoken == Token_INSTANCE_VARIABLE
                || yytoken == Token_CLASS_VARIABLE
                || yytoken == Token_DOUBLE_QUOTED_STRING
                || yytoken == Token_SINGLE_QUOTED_STRING
                || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_REGEX
                || yytoken == Token_COMMAND_OUTPUT
                || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_HERE_DOC_BEGIN
                || yytoken == Token_W_ARRAY
                || yytoken == Token_INTEGER
                || yytoken == Token_HEX
                || yytoken == Token_OCTAL
                || yytoken == Token_BINARY
                || yytoken == Token_FLOAT
                || yytoken == Token_ASCII_VALUE)
              {
                compoundStatement_ast *__node_47 = 0;
                if (!parse_compoundStatement(&__node_47))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_compoundStatement, "compoundStatement");
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

  bool parser::parse_caseExpression(caseExpression_ast **yynode)
  {
    *yynode = create<caseExpression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

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

        if (yytoken == Token_UNDEF
            || yytoken == Token_ALIAS
            || yytoken == Token_BEGIN
            || yytoken == Token_RETURN
            || yytoken == Token_BREAK
            || yytoken == Token_NEXT
            || yytoken == Token_NIL
            || yytoken == Token_TRUE
            || yytoken == Token_FALSE
            || yytoken == Token_FILE
            || yytoken == Token_LINE
            || yytoken == Token_SELF
            || yytoken == Token_SUPER
            || yytoken == Token_RETRY
            || yytoken == Token_YIELD
            || yytoken == Token_DEFINED
            || yytoken == Token_REDO
            || yytoken == Token_IF
            || yytoken == Token_CASE
            || yytoken == Token_UNTIL
            || yytoken == Token_WHILE
            || yytoken == Token_FOR
            || yytoken == Token_MODULE
            || yytoken == Token_DEF
            || yytoken == Token_CLASS
            || yytoken == Token_UNLESS
            || yytoken == Token_KEYWORD_NOT
            || yytoken == Token_END_UPCASE
            || yytoken == Token_BEGIN_UPCASE
            || yytoken == Token_LPAREN
            || yytoken == Token_SEMI
            || yytoken == Token_LEADING_TWO_COLON
            || yytoken == Token_LBRACK
            || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
            || yytoken == Token_LCURLY_HASH
            || yytoken == Token_REST_ARG_PREFIX
            || yytoken == Token_UNARY_MINUS
            || yytoken == Token_UNARY_PLUS
            || yytoken == Token_NOT
            || yytoken == Token_BNOT
            || yytoken == Token_LINE_BREAK
            || yytoken == Token_IDENTIFIER
            || yytoken == Token_CONSTANT
            || yytoken == Token_FUNCTION
            || yytoken == Token_GLOBAL_VARIABLE
            || yytoken == Token_INSTANCE_VARIABLE
            || yytoken == Token_CLASS_VARIABLE
            || yytoken == Token_DOUBLE_QUOTED_STRING
            || yytoken == Token_SINGLE_QUOTED_STRING
            || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_REGEX
            || yytoken == Token_COMMAND_OUTPUT
            || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_HERE_DOC_BEGIN
            || yytoken == Token_W_ARRAY
            || yytoken == Token_INTEGER
            || yytoken == Token_HEX
            || yytoken == Token_OCTAL
            || yytoken == Token_BINARY
            || yytoken == Token_FLOAT
            || yytoken == Token_ASCII_VALUE)
          {
            compoundStatement_ast *__node_48 = 0;
            if (!parse_compoundStatement(&__node_48))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_compoundStatement, "compoundStatement");
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
        do
          {
            keywordWhen_ast *__node_49 = 0;
            if (!parse_keywordWhen(&__node_49))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_keywordWhen, "keywordWhen");
                  }
                return false;
              }
            mrhs_ast *__node_50 = 0;
            if (!parse_mrhs(&__node_50))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_mrhs, "mrhs");
                  }
                return false;
              }
            thenOrTerminalOrColon_ast *__node_51 = 0;
            if (!parse_thenOrTerminalOrColon(&__node_51))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_thenOrTerminalOrColon, "thenOrTerminalOrColon");
                  }
                return false;
              }
            if (yytoken == Token_UNDEF
                || yytoken == Token_ALIAS
                || yytoken == Token_BEGIN
                || yytoken == Token_RETURN
                || yytoken == Token_BREAK
                || yytoken == Token_NEXT
                || yytoken == Token_NIL
                || yytoken == Token_TRUE
                || yytoken == Token_FALSE
                || yytoken == Token_FILE
                || yytoken == Token_LINE
                || yytoken == Token_SELF
                || yytoken == Token_SUPER
                || yytoken == Token_RETRY
                || yytoken == Token_YIELD
                || yytoken == Token_DEFINED
                || yytoken == Token_REDO
                || yytoken == Token_IF
                || yytoken == Token_CASE
                || yytoken == Token_UNTIL
                || yytoken == Token_WHILE
                || yytoken == Token_FOR
                || yytoken == Token_MODULE
                || yytoken == Token_DEF
                || yytoken == Token_CLASS
                || yytoken == Token_UNLESS
                || yytoken == Token_KEYWORD_NOT
                || yytoken == Token_END_UPCASE
                || yytoken == Token_BEGIN_UPCASE
                || yytoken == Token_LPAREN
                || yytoken == Token_SEMI
                || yytoken == Token_LEADING_TWO_COLON
                || yytoken == Token_LBRACK
                || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
                || yytoken == Token_LCURLY_HASH
                || yytoken == Token_REST_ARG_PREFIX
                || yytoken == Token_UNARY_MINUS
                || yytoken == Token_UNARY_PLUS
                || yytoken == Token_NOT
                || yytoken == Token_BNOT
                || yytoken == Token_LINE_BREAK
                || yytoken == Token_IDENTIFIER
                || yytoken == Token_CONSTANT
                || yytoken == Token_FUNCTION
                || yytoken == Token_GLOBAL_VARIABLE
                || yytoken == Token_INSTANCE_VARIABLE
                || yytoken == Token_CLASS_VARIABLE
                || yytoken == Token_DOUBLE_QUOTED_STRING
                || yytoken == Token_SINGLE_QUOTED_STRING
                || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_REGEX
                || yytoken == Token_COMMAND_OUTPUT
                || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_HERE_DOC_BEGIN
                || yytoken == Token_W_ARRAY
                || yytoken == Token_INTEGER
                || yytoken == Token_HEX
                || yytoken == Token_OCTAL
                || yytoken == Token_BINARY
                || yytoken == Token_FLOAT
                || yytoken == Token_ASCII_VALUE)
              {
                compoundStatement_ast *__node_52 = 0;
                if (!parse_compoundStatement(&__node_52))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_compoundStatement, "compoundStatement");
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
          }
        while (yytoken == Token_WHEN);
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

            if (yytoken == Token_UNDEF
                || yytoken == Token_ALIAS
                || yytoken == Token_BEGIN
                || yytoken == Token_RETURN
                || yytoken == Token_BREAK
                || yytoken == Token_NEXT
                || yytoken == Token_NIL
                || yytoken == Token_TRUE
                || yytoken == Token_FALSE
                || yytoken == Token_FILE
                || yytoken == Token_LINE
                || yytoken == Token_SELF
                || yytoken == Token_SUPER
                || yytoken == Token_RETRY
                || yytoken == Token_YIELD
                || yytoken == Token_DEFINED
                || yytoken == Token_REDO
                || yytoken == Token_IF
                || yytoken == Token_CASE
                || yytoken == Token_UNTIL
                || yytoken == Token_WHILE
                || yytoken == Token_FOR
                || yytoken == Token_MODULE
                || yytoken == Token_DEF
                || yytoken == Token_CLASS
                || yytoken == Token_UNLESS
                || yytoken == Token_KEYWORD_NOT
                || yytoken == Token_END_UPCASE
                || yytoken == Token_BEGIN_UPCASE
                || yytoken == Token_LPAREN
                || yytoken == Token_SEMI
                || yytoken == Token_LEADING_TWO_COLON
                || yytoken == Token_LBRACK
                || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
                || yytoken == Token_LCURLY_HASH
                || yytoken == Token_REST_ARG_PREFIX
                || yytoken == Token_UNARY_MINUS
                || yytoken == Token_UNARY_PLUS
                || yytoken == Token_NOT
                || yytoken == Token_BNOT
                || yytoken == Token_LINE_BREAK
                || yytoken == Token_IDENTIFIER
                || yytoken == Token_CONSTANT
                || yytoken == Token_FUNCTION
                || yytoken == Token_GLOBAL_VARIABLE
                || yytoken == Token_INSTANCE_VARIABLE
                || yytoken == Token_CLASS_VARIABLE
                || yytoken == Token_DOUBLE_QUOTED_STRING
                || yytoken == Token_SINGLE_QUOTED_STRING
                || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_REGEX
                || yytoken == Token_COMMAND_OUTPUT
                || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_HERE_DOC_BEGIN
                || yytoken == Token_W_ARRAY
                || yytoken == Token_INTEGER
                || yytoken == Token_HEX
                || yytoken == Token_OCTAL
                || yytoken == Token_BINARY
                || yytoken == Token_FLOAT
                || yytoken == Token_ASCII_VALUE)
              {
                compoundStatement_ast *__node_53 = 0;
                if (!parse_compoundStatement(&__node_53))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_compoundStatement, "compoundStatement");
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
          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        if (yytoken != Token_END)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_END, "end");
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

  bool parser::parse_classDefinition(classDefinition_ast **yynode)
  {
    *yynode = create<classDefinition_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_CLASS)
      {
        keywordClass_ast *__node_54 = 0;
        if (!parse_keywordClass(&__node_54))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_keywordClass, "keywordClass");
              }
            return false;
          }
        if (yytoken == Token_LEADING_TWO_COLON
            || yytoken == Token_CONSTANT
            || yytoken == Token_FUNCTION)
          {
            className_ast *__node_55 = 0;
            if (!parse_className(&__node_55))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_className, "className");
                  }
                return false;
              }
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

                expression_ast *__node_56 = 0;
                if (!parse_expression(&__node_56))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_expression, "expression");
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
          }
        else if (yytoken == Token_LEFT_SHIFT)
          {
            if (yytoken != Token_LEFT_SHIFT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LEFT_SHIFT, "<<");
                  }
                return false;
              }
            yylex();

            expression_ast *__node_57 = 0;
            if (!parse_expression(&__node_57))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_expression, "expression");
                  }
                return false;
              }
          }
        else
          {
            return false;
          }
        terminal_ast *__node_58 = 0;
        if (!parse_terminal(&__node_58))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_terminal, "terminal");
              }
            return false;
          }
        bodyStatement_ast *__node_59 = 0;
        if (!parse_bodyStatement(&__node_59))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_bodyStatement, "bodyStatement");
              }
            return false;
          }
        if (yytoken != Token_END)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_END, "end");
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

  bool parser::parse_className(className_ast **yynode)
  {
    *yynode = create<className_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION)
      {
        if (yytoken == Token_CONSTANT)
          {
            if (yytoken != Token_CONSTANT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_CONSTANT, "constant");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_FUNCTION)
          {
            if (yytoken != Token_FUNCTION)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_FUNCTION, "function");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_LEADING_TWO_COLON)
          {
            if (yytoken != Token_LEADING_TWO_COLON)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LEADING_TWO_COLON, "::");
                  }
                return false;
              }
            yylex();

            if (yytoken != Token_FUNCTION)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_FUNCTION, "function");
                  }
                return false;
              }
            yylex();

          }
        else
          {
            return false;
          }
        while (yytoken == Token_TWO_COLON)
          {
            if (yytoken != Token_TWO_COLON)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_TWO_COLON, "::");
                  }
                return false;
              }
            yylex();

            if (yytoken != Token_FUNCTION)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_FUNCTION, "function");
                  }
                return false;
              }
            yylex();

          }
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_codeBlock(codeBlock_ast **yynode)
  {
    *yynode = create<codeBlock_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_DO
        || yytoken == Token_LCURLY_BLOCK)
      {
        if (yytoken == Token_DO)
          {
            keywordDo_ast *__node_60 = 0;
            if (!parse_keywordDo(&__node_60))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_keywordDo, "keywordDo");
                  }
                return false;
              }
            blockContent_ast *__node_61 = 0;
            if (!parse_blockContent(&__node_61))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_blockContent, "blockContent");
                  }
                return false;
              }
            if (yytoken != Token_END)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_END, "end");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_LCURLY_BLOCK)
          {
            if (yytoken != Token_LCURLY_BLOCK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LCURLY_BLOCK, "{");
                  }
                return false;
              }
            yylex();

            blockContent_ast *__node_62 = 0;
            if (!parse_blockContent(&__node_62))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_blockContent, "blockContent");
                  }
                return false;
              }
            if (yytoken != Token_RCURLY)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_RCURLY, "}");
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

  bool parser::parse_colonAccess(colonAccess_ast **yynode)
  {
    *yynode = create<colonAccess_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BEGIN
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_LPAREN
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE)
      {
        methodCall_ast *__node_63 = 0;
        if (!parse_methodCall(&__node_63))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_methodCall, "methodCall");
              }
            return false;
          }
        while (yytoken == Token_TWO_COLON)
          {
            if (yytoken != Token_TWO_COLON)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_TWO_COLON, "::");
                  }
                return false;
              }
            yylex();

            methodCall_ast *__node_64 = 0;
            if (!parse_methodCall(&__node_64))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_methodCall, "methodCall");
                  }
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

  bool parser::parse_command(command_ast **yynode)
  {
    *yynode = create<command_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BEGIN
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_LPAREN
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE)
      {
        if (yytoken == Token_BEGIN
            || yytoken == Token_NIL
            || yytoken == Token_TRUE
            || yytoken == Token_FALSE
            || yytoken == Token_FILE
            || yytoken == Token_LINE
            || yytoken == Token_SELF
            || yytoken == Token_SUPER
            || yytoken == Token_RETRY
            || yytoken == Token_YIELD
            || yytoken == Token_DEFINED
            || yytoken == Token_REDO
            || yytoken == Token_IF
            || yytoken == Token_CASE
            || yytoken == Token_UNTIL
            || yytoken == Token_WHILE
            || yytoken == Token_FOR
            || yytoken == Token_MODULE
            || yytoken == Token_DEF
            || yytoken == Token_CLASS
            || yytoken == Token_UNLESS
            || yytoken == Token_LPAREN
            || yytoken == Token_LEADING_TWO_COLON
            || yytoken == Token_LBRACK
            || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
            || yytoken == Token_LCURLY_HASH
            || yytoken == Token_IDENTIFIER
            || yytoken == Token_CONSTANT
            || yytoken == Token_FUNCTION
            || yytoken == Token_GLOBAL_VARIABLE
            || yytoken == Token_INSTANCE_VARIABLE
            || yytoken == Token_CLASS_VARIABLE
            || yytoken == Token_DOUBLE_QUOTED_STRING
            || yytoken == Token_SINGLE_QUOTED_STRING
            || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_REGEX
            || yytoken == Token_COMMAND_OUTPUT
            || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_HERE_DOC_BEGIN
            || yytoken == Token_W_ARRAY
            || yytoken == Token_INTEGER
            || yytoken == Token_HEX
            || yytoken == Token_OCTAL
            || yytoken == Token_BINARY
            || yytoken == Token_FLOAT
            || yytoken == Token_ASCII_VALUE)
          {
            colonAccess_ast *__node_65 = 0;
            if (!parse_colonAccess(&__node_65))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_colonAccess, "colonAccess");
                  }
                return false;
              }
            if (yytoken == Token_LPAREN)
              {
                methodInvocationArgumentWithParen_ast *__node_66 = 0;
                if (!parse_methodInvocationArgumentWithParen(&__node_66))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_methodInvocationArgumentWithParen, "methodInvocationArgumentWithParen");
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
            if (yytoken == Token_DO
                || yytoken == Token_LCURLY_BLOCK)
              {
                codeBlock_ast *__node_67 = 0;
                if (!parse_codeBlock(&__node_67))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_codeBlock, "codeBlock");
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
          }
        else if (yytoken == Token_RETURN
                 || yytoken == Token_BREAK
                 || yytoken == Token_NEXT)
          {
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

              }
            else if (yytoken == Token_BREAK)
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

              }
            else if (yytoken == Token_NEXT)
              {
                if (yytoken != Token_NEXT)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_NEXT, "next");
                      }
                    return false;
                  }
                yylex();

              }
            else
              {
                return false;
              }
            if (yytoken == Token_LPAREN)
              {
                methodInvocationArgumentWithParen_ast *__node_68 = 0;
                if (!parse_methodInvocationArgumentWithParen(&__node_68))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_methodInvocationArgumentWithParen, "methodInvocationArgumentWithParen");
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

  bool parser::parse_commandOutput(commandOutput_ast **yynode)
  {
    *yynode = create<commandOutput_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION)
      {
        if (yytoken == Token_COMMAND_OUTPUT)
          {
            if (yytoken != Token_COMMAND_OUTPUT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_COMMAND_OUTPUT, "command output");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION)
          {
            if (yytoken != Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION, "command output before expression substitution");
                  }
                return false;
              }
            yylex();

            expressionSubstitution_ast *__node_69 = 0;
            if (!parse_expressionSubstitution(&__node_69))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_expressionSubstitution, "expressionSubstitution");
                  }
                return false;
              }
            while (yytoken == Token_STRING_BETWEEN_EXPRESSION_SUBSTITUTION)
              {
                if (yytoken != Token_STRING_BETWEEN_EXPRESSION_SUBSTITUTION)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_STRING_BETWEEN_EXPRESSION_SUBSTITUTION, "string between expression substitution");
                      }
                    return false;
                  }
                yylex();

                expressionSubstitution_ast *__node_70 = 0;
                if (!parse_expressionSubstitution(&__node_70))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_expressionSubstitution, "expressionSubstitution");
                      }
                    return false;
                  }
              }
            if (yytoken != Token_STRING_AFTER_EXPRESSION_SUBSTITUTION)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_STRING_AFTER_EXPRESSION_SUBSTITUTION, "string after expression substitution");
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

  bool parser::parse_compoundStatement(compoundStatement_ast **yynode)
  {
    *yynode = create<compoundStatement_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_UNDEF
        || yytoken == Token_ALIAS
        || yytoken == Token_BEGIN
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_KEYWORD_NOT
        || yytoken == Token_END_UPCASE
        || yytoken == Token_BEGIN_UPCASE
        || yytoken == Token_LPAREN
        || yytoken == Token_SEMI
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_REST_ARG_PREFIX
        || yytoken == Token_UNARY_MINUS
        || yytoken == Token_UNARY_PLUS
        || yytoken == Token_NOT
        || yytoken == Token_BNOT
        || yytoken == Token_LINE_BREAK
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE)
      {
        if (yytoken == Token_SEMI
            || yytoken == Token_LINE_BREAK)
          {
            terminal_ast *__node_71 = 0;
            if (!parse_terminal(&__node_71))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_terminal, "terminal");
                  }
                return false;
              }
            if (yytoken == Token_UNDEF
                || yytoken == Token_ALIAS
                || yytoken == Token_BEGIN
                || yytoken == Token_RETURN
                || yytoken == Token_BREAK
                || yytoken == Token_NEXT
                || yytoken == Token_NIL
                || yytoken == Token_TRUE
                || yytoken == Token_FALSE
                || yytoken == Token_FILE
                || yytoken == Token_LINE
                || yytoken == Token_SELF
                || yytoken == Token_SUPER
                || yytoken == Token_RETRY
                || yytoken == Token_YIELD
                || yytoken == Token_DEFINED
                || yytoken == Token_REDO
                || yytoken == Token_IF
                || yytoken == Token_CASE
                || yytoken == Token_UNTIL
                || yytoken == Token_WHILE
                || yytoken == Token_FOR
                || yytoken == Token_MODULE
                || yytoken == Token_DEF
                || yytoken == Token_CLASS
                || yytoken == Token_UNLESS
                || yytoken == Token_KEYWORD_NOT
                || yytoken == Token_END_UPCASE
                || yytoken == Token_BEGIN_UPCASE
                || yytoken == Token_LPAREN
                || yytoken == Token_LEADING_TWO_COLON
                || yytoken == Token_LBRACK
                || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
                || yytoken == Token_LCURLY_HASH
                || yytoken == Token_REST_ARG_PREFIX
                || yytoken == Token_UNARY_MINUS
                || yytoken == Token_UNARY_PLUS
                || yytoken == Token_NOT
                || yytoken == Token_BNOT
                || yytoken == Token_IDENTIFIER
                || yytoken == Token_CONSTANT
                || yytoken == Token_FUNCTION
                || yytoken == Token_GLOBAL_VARIABLE
                || yytoken == Token_INSTANCE_VARIABLE
                || yytoken == Token_CLASS_VARIABLE
                || yytoken == Token_DOUBLE_QUOTED_STRING
                || yytoken == Token_SINGLE_QUOTED_STRING
                || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_REGEX
                || yytoken == Token_COMMAND_OUTPUT
                || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_HERE_DOC_BEGIN
                || yytoken == Token_W_ARRAY
                || yytoken == Token_INTEGER
                || yytoken == Token_HEX
                || yytoken == Token_OCTAL
                || yytoken == Token_BINARY
                || yytoken == Token_FLOAT
                || yytoken == Token_ASCII_VALUE)
              {
                statements_ast *__node_72 = 0;
                if (!parse_statements(&__node_72))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_statements, "statements");
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
          }
        else if (yytoken == Token_UNDEF
                 || yytoken == Token_ALIAS
                 || yytoken == Token_BEGIN
                 || yytoken == Token_RETURN
                 || yytoken == Token_BREAK
                 || yytoken == Token_NEXT
                 || yytoken == Token_NIL
                 || yytoken == Token_TRUE
                 || yytoken == Token_FALSE
                 || yytoken == Token_FILE
                 || yytoken == Token_LINE
                 || yytoken == Token_SELF
                 || yytoken == Token_SUPER
                 || yytoken == Token_RETRY
                 || yytoken == Token_YIELD
                 || yytoken == Token_DEFINED
                 || yytoken == Token_REDO
                 || yytoken == Token_IF
                 || yytoken == Token_CASE
                 || yytoken == Token_UNTIL
                 || yytoken == Token_WHILE
                 || yytoken == Token_FOR
                 || yytoken == Token_MODULE
                 || yytoken == Token_DEF
                 || yytoken == Token_CLASS
                 || yytoken == Token_UNLESS
                 || yytoken == Token_KEYWORD_NOT
                 || yytoken == Token_END_UPCASE
                 || yytoken == Token_BEGIN_UPCASE
                 || yytoken == Token_LPAREN
                 || yytoken == Token_LEADING_TWO_COLON
                 || yytoken == Token_LBRACK
                 || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
                 || yytoken == Token_LCURLY_HASH
                 || yytoken == Token_REST_ARG_PREFIX
                 || yytoken == Token_UNARY_MINUS
                 || yytoken == Token_UNARY_PLUS
                 || yytoken == Token_NOT
                 || yytoken == Token_BNOT
                 || yytoken == Token_IDENTIFIER
                 || yytoken == Token_CONSTANT
                 || yytoken == Token_FUNCTION
                 || yytoken == Token_GLOBAL_VARIABLE
                 || yytoken == Token_INSTANCE_VARIABLE
                 || yytoken == Token_CLASS_VARIABLE
                 || yytoken == Token_DOUBLE_QUOTED_STRING
                 || yytoken == Token_SINGLE_QUOTED_STRING
                 || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
                 || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
                 || yytoken == Token_REGEX
                 || yytoken == Token_COMMAND_OUTPUT
                 || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
                 || yytoken == Token_HERE_DOC_BEGIN
                 || yytoken == Token_W_ARRAY
                 || yytoken == Token_INTEGER
                 || yytoken == Token_HEX
                 || yytoken == Token_OCTAL
                 || yytoken == Token_BINARY
                 || yytoken == Token_FLOAT
                 || yytoken == Token_ASCII_VALUE)
          {
            statements_ast *__node_73 = 0;
            if (!parse_statements(&__node_73))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_statements, "statements");
                  }
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

  bool parser::parse_doOrTerminalOrColon(doOrTerminalOrColon_ast **yynode)
  {
    *yynode = create<doOrTerminalOrColon_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_SEMI
        || yytoken == Token_COLON
        || yytoken == Token_LINE_BREAK
        || yytoken == Token_DO_IN_CONDITION)
      {
        if (yytoken == Token_DO_IN_CONDITION)
          {
            if (yytoken != Token_DO_IN_CONDITION)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_DO_IN_CONDITION, "do in condition");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_SEMI
                 || yytoken == Token_LINE_BREAK)
          {
            terminal_ast *__node_74 = 0;
            if (!parse_terminal(&__node_74))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_terminal, "terminal");
                  }
                return false;
              }
          }
        else if (yytoken == Token_COLON)
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

  bool parser::parse_dotAccess(dotAccess_ast **yynode)
  {
    *yynode = create<dotAccess_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BEGIN
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_LPAREN
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE)
      {
        elementReference_ast *__node_75 = 0;
        if (!parse_elementReference(&__node_75))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_elementReference, "elementReference");
              }
            return false;
          }
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

            elementReference_ast *__node_76 = 0;
            if (!parse_elementReference(&__node_76))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_elementReference, "elementReference");
                  }
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

  bool parser::parse_elementReference(elementReference_ast **yynode)
  {
    *yynode = create<elementReference_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BEGIN
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_LPAREN
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE)
      {
        command_ast *__node_77 = 0;
        if (!parse_command(&__node_77))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_command, "command");
              }
            return false;
          }
        while (yytoken == Token_LBRACK_ARRAY_ACCESS)
          {
            arrayAccess_ast *__node_78 = 0;
            if (!parse_arrayAccess(&__node_78))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_arrayAccess, "arrayAccess");
                  }
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

  bool parser::parse_equalityExpression(equalityExpression_ast **yynode)
  {
    *yynode = create<equalityExpression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BEGIN
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_LPAREN
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_UNARY_MINUS
        || yytoken == Token_UNARY_PLUS
        || yytoken == Token_NOT
        || yytoken == Token_BNOT
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE)
      {
        relationalExpression_ast *__node_79 = 0;
        if (!parse_relationalExpression(&__node_79))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_relationalExpression, "relationalExpression");
              }
            return false;
          }
        while (yytoken == Token_COMPARE
               || yytoken == Token_EQUAL
               || yytoken == Token_CASE_EQUAL
               || yytoken == Token_MATCH
               || yytoken == Token_NOT_EQUAL
               || yytoken == Token_NOT_MATCH)
          {
            if (yytoken == Token_COMPARE)
              {
                operatorCompare_ast *__node_80 = 0;
                if (!parse_operatorCompare(&__node_80))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorCompare, "operatorCompare");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_EQUAL)
              {
                operatorEqual_ast *__node_81 = 0;
                if (!parse_operatorEqual(&__node_81))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorEqual, "operatorEqual");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_CASE_EQUAL)
              {
                operatorCaseEqual_ast *__node_82 = 0;
                if (!parse_operatorCaseEqual(&__node_82))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorCaseEqual, "operatorCaseEqual");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_NOT_EQUAL)
              {
                operatorNotEqual_ast *__node_83 = 0;
                if (!parse_operatorNotEqual(&__node_83))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorNotEqual, "operatorNotEqual");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_MATCH)
              {
                operatorMatch_ast *__node_84 = 0;
                if (!parse_operatorMatch(&__node_84))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorMatch, "operatorMatch");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_NOT_MATCH)
              {
                operatorNotMatch_ast *__node_85 = 0;
                if (!parse_operatorNotMatch(&__node_85))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorNotMatch, "operatorNotMatch");
                      }
                    return false;
                  }
              }
            else
              {
                return false;
              }
            relationalExpression_ast *__node_86 = 0;
            if (!parse_relationalExpression(&__node_86))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_relationalExpression, "relationalExpression");
                  }
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

  bool parser::parse_exceptionHandlingExpression(exceptionHandlingExpression_ast **yynode)
  {
    *yynode = create<exceptionHandlingExpression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BEGIN)
      {
        if (yytoken != Token_BEGIN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_BEGIN, "begin");
              }
            return false;
          }
        yylex();

        bodyStatement_ast *__node_87 = 0;
        if (!parse_bodyStatement(&__node_87))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_bodyStatement, "bodyStatement");
              }
            return false;
          }
        if (yytoken != Token_END)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_END, "end");
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

  bool parser::parse_exceptionList(exceptionList_ast **yynode)
  {
    *yynode = create<exceptionList_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_ASSOC
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_INSTANCE_VARIABLE || yytoken == Token_THEN
        || yytoken == Token_SEMI
        || yytoken == Token_COLON
        || yytoken == Token_LINE_BREAK)
      {
        if (yytoken == Token_LEADING_TWO_COLON
            || yytoken == Token_CONSTANT
            || yytoken == Token_FUNCTION
            || yytoken == Token_INSTANCE_VARIABLE)
          {
            if (yytoken == Token_LEADING_TWO_COLON
                || yytoken == Token_CONSTANT
                || yytoken == Token_FUNCTION)
              {
                className_ast *__node_88 = 0;
                if (!parse_className(&__node_88))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_className, "className");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_INSTANCE_VARIABLE)
              {
                if (yytoken != Token_INSTANCE_VARIABLE)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_INSTANCE_VARIABLE, "instance variable");
                      }
                    return false;
                  }
                yylex();

              }
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

                if (yytoken == Token_LEADING_TWO_COLON
                    || yytoken == Token_CONSTANT
                    || yytoken == Token_FUNCTION)
                  {
                    className_ast *__node_89 = 0;
                    if (!parse_className(&__node_89))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_className, "className");
                          }
                        return false;
                      }
                  }
                else if (yytoken == Token_INSTANCE_VARIABLE)
                  {
                    if (yytoken != Token_INSTANCE_VARIABLE)
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_token(yytoken, Token_INSTANCE_VARIABLE, "instance variable");
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
          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        if (yytoken == Token_ASSOC)
          {
            if (yytoken != Token_ASSOC)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_ASSOC, "=>");
                  }
                return false;
              }
            yylex();

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
                yylex();

              }
            else if (yytoken == Token_FUNCTION)
              {
                if (yytoken != Token_FUNCTION)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_FUNCTION, "function");
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

  bool parser::parse_expression(expression_ast **yynode)
  {
    *yynode = create<expression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BEGIN
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_KEYWORD_NOT
        || yytoken == Token_LPAREN
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_UNARY_MINUS
        || yytoken == Token_UNARY_PLUS
        || yytoken == Token_NOT
        || yytoken == Token_BNOT
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE)
      {
        andorExpression_ast *__node_90 = 0;
        if (!parse_andorExpression(&__node_90))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_andorExpression, "andorExpression");
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

  bool parser::parse_expressionSubstitution(expressionSubstitution_ast **yynode)
  {
    *yynode = create<expressionSubstitution_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_LCURLY_BLOCK
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE)
      {
        if (yytoken == Token_LCURLY_BLOCK)
          {
            if (yytoken != Token_LCURLY_BLOCK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LCURLY_BLOCK, "{");
                  }
                return false;
              }
            yylex();

            compoundStatement_ast *__node_91 = 0;
            if (!parse_compoundStatement(&__node_91))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_compoundStatement, "compoundStatement");
                  }
                return false;
              }
            if (yytoken != Token_RCURLY)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_RCURLY, "}");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_GLOBAL_VARIABLE)
          {
            if (yytoken != Token_GLOBAL_VARIABLE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_GLOBAL_VARIABLE, "global variable");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_INSTANCE_VARIABLE)
          {
            if (yytoken != Token_INSTANCE_VARIABLE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_INSTANCE_VARIABLE, "instance variable");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_CLASS_VARIABLE)
          {
            if (yytoken != Token_CLASS_VARIABLE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_CLASS_VARIABLE, "class variable");
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

  bool parser::parse_forExpression(forExpression_ast **yynode)
  {
    *yynode = create<forExpression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_FOR)
      {
        keywordFor_ast *__node_92 = 0;
        if (!parse_keywordFor(&__node_92))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_keywordFor, "keywordFor");
              }
            return false;
          }
        block_vars_ast *__node_93 = 0;
        if (!parse_block_vars(&__node_93))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_block_vars, "block_vars");
              }
            return false;
          }
        keywordIn_ast *__node_94 = 0;
        if (!parse_keywordIn(&__node_94))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_keywordIn, "keywordIn");
              }
            return false;
          }
        expression_ast *__node_95 = 0;
        if (!parse_expression(&__node_95))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_expression, "expression");
              }
            return false;
          }
        doOrTerminalOrColon_ast *__node_96 = 0;
        if (!parse_doOrTerminalOrColon(&__node_96))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_doOrTerminalOrColon, "doOrTerminalOrColon");
              }
            return false;
          }
        if (yytoken == Token_UNDEF
            || yytoken == Token_ALIAS
            || yytoken == Token_BEGIN
            || yytoken == Token_RETURN
            || yytoken == Token_BREAK
            || yytoken == Token_NEXT
            || yytoken == Token_NIL
            || yytoken == Token_TRUE
            || yytoken == Token_FALSE
            || yytoken == Token_FILE
            || yytoken == Token_LINE
            || yytoken == Token_SELF
            || yytoken == Token_SUPER
            || yytoken == Token_RETRY
            || yytoken == Token_YIELD
            || yytoken == Token_DEFINED
            || yytoken == Token_REDO
            || yytoken == Token_IF
            || yytoken == Token_CASE
            || yytoken == Token_UNTIL
            || yytoken == Token_WHILE
            || yytoken == Token_FOR
            || yytoken == Token_MODULE
            || yytoken == Token_DEF
            || yytoken == Token_CLASS
            || yytoken == Token_UNLESS
            || yytoken == Token_KEYWORD_NOT
            || yytoken == Token_END_UPCASE
            || yytoken == Token_BEGIN_UPCASE
            || yytoken == Token_LPAREN
            || yytoken == Token_SEMI
            || yytoken == Token_LEADING_TWO_COLON
            || yytoken == Token_LBRACK
            || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
            || yytoken == Token_LCURLY_HASH
            || yytoken == Token_REST_ARG_PREFIX
            || yytoken == Token_UNARY_MINUS
            || yytoken == Token_UNARY_PLUS
            || yytoken == Token_NOT
            || yytoken == Token_BNOT
            || yytoken == Token_LINE_BREAK
            || yytoken == Token_IDENTIFIER
            || yytoken == Token_CONSTANT
            || yytoken == Token_FUNCTION
            || yytoken == Token_GLOBAL_VARIABLE
            || yytoken == Token_INSTANCE_VARIABLE
            || yytoken == Token_CLASS_VARIABLE
            || yytoken == Token_DOUBLE_QUOTED_STRING
            || yytoken == Token_SINGLE_QUOTED_STRING
            || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_REGEX
            || yytoken == Token_COMMAND_OUTPUT
            || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_HERE_DOC_BEGIN
            || yytoken == Token_W_ARRAY
            || yytoken == Token_INTEGER
            || yytoken == Token_HEX
            || yytoken == Token_OCTAL
            || yytoken == Token_BINARY
            || yytoken == Token_FLOAT
            || yytoken == Token_ASCII_VALUE)
          {
            compoundStatement_ast *__node_97 = 0;
            if (!parse_compoundStatement(&__node_97))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_compoundStatement, "compoundStatement");
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
        if (yytoken != Token_END)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_END, "end");
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

  bool parser::parse_hashExpression(hashExpression_ast **yynode)
  {
    *yynode = create<hashExpression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_LCURLY_HASH)
      {
        if (yytoken != Token_LCURLY_HASH)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_LCURLY_HASH, "{");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_BEGIN
            || yytoken == Token_RETURN
            || yytoken == Token_BREAK
            || yytoken == Token_NEXT
            || yytoken == Token_NIL
            || yytoken == Token_TRUE
            || yytoken == Token_FALSE
            || yytoken == Token_FILE
            || yytoken == Token_LINE
            || yytoken == Token_SELF
            || yytoken == Token_SUPER
            || yytoken == Token_RETRY
            || yytoken == Token_YIELD
            || yytoken == Token_DEFINED
            || yytoken == Token_REDO
            || yytoken == Token_IF
            || yytoken == Token_CASE
            || yytoken == Token_UNTIL
            || yytoken == Token_WHILE
            || yytoken == Token_FOR
            || yytoken == Token_MODULE
            || yytoken == Token_DEF
            || yytoken == Token_CLASS
            || yytoken == Token_UNLESS
            || yytoken == Token_KEYWORD_NOT
            || yytoken == Token_LPAREN
            || yytoken == Token_LEADING_TWO_COLON
            || yytoken == Token_LBRACK
            || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
            || yytoken == Token_LCURLY_HASH
            || yytoken == Token_UNARY_MINUS
            || yytoken == Token_UNARY_PLUS
            || yytoken == Token_NOT
            || yytoken == Token_BNOT
            || yytoken == Token_IDENTIFIER
            || yytoken == Token_CONSTANT
            || yytoken == Token_FUNCTION
            || yytoken == Token_GLOBAL_VARIABLE
            || yytoken == Token_INSTANCE_VARIABLE
            || yytoken == Token_CLASS_VARIABLE
            || yytoken == Token_DOUBLE_QUOTED_STRING
            || yytoken == Token_SINGLE_QUOTED_STRING
            || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_REGEX
            || yytoken == Token_COMMAND_OUTPUT
            || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_HERE_DOC_BEGIN
            || yytoken == Token_W_ARRAY
            || yytoken == Token_INTEGER
            || yytoken == Token_HEX
            || yytoken == Token_OCTAL
            || yytoken == Token_BINARY
            || yytoken == Token_FLOAT
            || yytoken == Token_ASCII_VALUE)
          {
            keyValuePair_ast *__node_98 = 0;
            if (!parse_keyValuePair(&__node_98))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_keyValuePair, "keyValuePair");
                  }
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

                keyValuePair_ast *__node_99 = 0;
                if (!parse_keyValuePair(&__node_99))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_keyValuePair, "keyValuePair");
                      }
                    return false;
                  }
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
        if (yytoken != Token_RCURLY)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_RCURLY, "}");
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

  bool parser::parse_ifExpression(ifExpression_ast **yynode)
  {
    *yynode = create<ifExpression_ast>();

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

        expression_ast *__node_100 = 0;
        if (!parse_expression(&__node_100))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_expression, "expression");
              }
            return false;
          }
        thenOrTerminalOrColon_ast *__node_101 = 0;
        if (!parse_thenOrTerminalOrColon(&__node_101))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_thenOrTerminalOrColon, "thenOrTerminalOrColon");
              }
            return false;
          }
        if (yytoken == Token_UNDEF
            || yytoken == Token_ALIAS
            || yytoken == Token_BEGIN
            || yytoken == Token_RETURN
            || yytoken == Token_BREAK
            || yytoken == Token_NEXT
            || yytoken == Token_NIL
            || yytoken == Token_TRUE
            || yytoken == Token_FALSE
            || yytoken == Token_FILE
            || yytoken == Token_LINE
            || yytoken == Token_SELF
            || yytoken == Token_SUPER
            || yytoken == Token_RETRY
            || yytoken == Token_YIELD
            || yytoken == Token_DEFINED
            || yytoken == Token_REDO
            || yytoken == Token_IF
            || yytoken == Token_CASE
            || yytoken == Token_UNTIL
            || yytoken == Token_WHILE
            || yytoken == Token_FOR
            || yytoken == Token_MODULE
            || yytoken == Token_DEF
            || yytoken == Token_CLASS
            || yytoken == Token_UNLESS
            || yytoken == Token_KEYWORD_NOT
            || yytoken == Token_END_UPCASE
            || yytoken == Token_BEGIN_UPCASE
            || yytoken == Token_LPAREN
            || yytoken == Token_SEMI
            || yytoken == Token_LEADING_TWO_COLON
            || yytoken == Token_LBRACK
            || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
            || yytoken == Token_LCURLY_HASH
            || yytoken == Token_REST_ARG_PREFIX
            || yytoken == Token_UNARY_MINUS
            || yytoken == Token_UNARY_PLUS
            || yytoken == Token_NOT
            || yytoken == Token_BNOT
            || yytoken == Token_LINE_BREAK
            || yytoken == Token_IDENTIFIER
            || yytoken == Token_CONSTANT
            || yytoken == Token_FUNCTION
            || yytoken == Token_GLOBAL_VARIABLE
            || yytoken == Token_INSTANCE_VARIABLE
            || yytoken == Token_CLASS_VARIABLE
            || yytoken == Token_DOUBLE_QUOTED_STRING
            || yytoken == Token_SINGLE_QUOTED_STRING
            || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_REGEX
            || yytoken == Token_COMMAND_OUTPUT
            || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_HERE_DOC_BEGIN
            || yytoken == Token_W_ARRAY
            || yytoken == Token_INTEGER
            || yytoken == Token_HEX
            || yytoken == Token_OCTAL
            || yytoken == Token_BINARY
            || yytoken == Token_FLOAT
            || yytoken == Token_ASCII_VALUE)
          {
            compoundStatement_ast *__node_102 = 0;
            if (!parse_compoundStatement(&__node_102))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_compoundStatement, "compoundStatement");
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
        while (yytoken == Token_ELSIF)
          {
            if (yytoken != Token_ELSIF)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_ELSIF, "elsif");
                  }
                return false;
              }
            yylex();

            if (yytoken == Token_BEGIN
                || yytoken == Token_RETURN
                || yytoken == Token_BREAK
                || yytoken == Token_NEXT
                || yytoken == Token_NIL
                || yytoken == Token_TRUE
                || yytoken == Token_FALSE
                || yytoken == Token_FILE
                || yytoken == Token_LINE
                || yytoken == Token_SELF
                || yytoken == Token_SUPER
                || yytoken == Token_RETRY
                || yytoken == Token_YIELD
                || yytoken == Token_DEFINED
                || yytoken == Token_REDO
                || yytoken == Token_IF
                || yytoken == Token_CASE
                || yytoken == Token_UNTIL
                || yytoken == Token_WHILE
                || yytoken == Token_FOR
                || yytoken == Token_MODULE
                || yytoken == Token_DEF
                || yytoken == Token_CLASS
                || yytoken == Token_UNLESS
                || yytoken == Token_KEYWORD_NOT
                || yytoken == Token_LPAREN
                || yytoken == Token_LEADING_TWO_COLON
                || yytoken == Token_LBRACK
                || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
                || yytoken == Token_LCURLY_HASH
                || yytoken == Token_UNARY_MINUS
                || yytoken == Token_UNARY_PLUS
                || yytoken == Token_NOT
                || yytoken == Token_BNOT
                || yytoken == Token_IDENTIFIER
                || yytoken == Token_CONSTANT
                || yytoken == Token_FUNCTION
                || yytoken == Token_GLOBAL_VARIABLE
                || yytoken == Token_INSTANCE_VARIABLE
                || yytoken == Token_CLASS_VARIABLE
                || yytoken == Token_DOUBLE_QUOTED_STRING
                || yytoken == Token_SINGLE_QUOTED_STRING
                || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_REGEX
                || yytoken == Token_COMMAND_OUTPUT
                || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_HERE_DOC_BEGIN
                || yytoken == Token_W_ARRAY
                || yytoken == Token_INTEGER
                || yytoken == Token_HEX
                || yytoken == Token_OCTAL
                || yytoken == Token_BINARY
                || yytoken == Token_FLOAT
                || yytoken == Token_ASCII_VALUE)
              {
                expression_ast *__node_103 = 0;
                if (!parse_expression(&__node_103))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_expression, "expression");
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
            thenOrTerminalOrColon_ast *__node_104 = 0;
            if (!parse_thenOrTerminalOrColon(&__node_104))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_thenOrTerminalOrColon, "thenOrTerminalOrColon");
                  }
                return false;
              }
            if (yytoken == Token_UNDEF
                || yytoken == Token_ALIAS
                || yytoken == Token_BEGIN
                || yytoken == Token_RETURN
                || yytoken == Token_BREAK
                || yytoken == Token_NEXT
                || yytoken == Token_NIL
                || yytoken == Token_TRUE
                || yytoken == Token_FALSE
                || yytoken == Token_FILE
                || yytoken == Token_LINE
                || yytoken == Token_SELF
                || yytoken == Token_SUPER
                || yytoken == Token_RETRY
                || yytoken == Token_YIELD
                || yytoken == Token_DEFINED
                || yytoken == Token_REDO
                || yytoken == Token_IF
                || yytoken == Token_CASE
                || yytoken == Token_UNTIL
                || yytoken == Token_WHILE
                || yytoken == Token_FOR
                || yytoken == Token_MODULE
                || yytoken == Token_DEF
                || yytoken == Token_CLASS
                || yytoken == Token_UNLESS
                || yytoken == Token_KEYWORD_NOT
                || yytoken == Token_END_UPCASE
                || yytoken == Token_BEGIN_UPCASE
                || yytoken == Token_LPAREN
                || yytoken == Token_SEMI
                || yytoken == Token_LEADING_TWO_COLON
                || yytoken == Token_LBRACK
                || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
                || yytoken == Token_LCURLY_HASH
                || yytoken == Token_REST_ARG_PREFIX
                || yytoken == Token_UNARY_MINUS
                || yytoken == Token_UNARY_PLUS
                || yytoken == Token_NOT
                || yytoken == Token_BNOT
                || yytoken == Token_LINE_BREAK
                || yytoken == Token_IDENTIFIER
                || yytoken == Token_CONSTANT
                || yytoken == Token_FUNCTION
                || yytoken == Token_GLOBAL_VARIABLE
                || yytoken == Token_INSTANCE_VARIABLE
                || yytoken == Token_CLASS_VARIABLE
                || yytoken == Token_DOUBLE_QUOTED_STRING
                || yytoken == Token_SINGLE_QUOTED_STRING
                || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_REGEX
                || yytoken == Token_COMMAND_OUTPUT
                || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_HERE_DOC_BEGIN
                || yytoken == Token_W_ARRAY
                || yytoken == Token_INTEGER
                || yytoken == Token_HEX
                || yytoken == Token_OCTAL
                || yytoken == Token_BINARY
                || yytoken == Token_FLOAT
                || yytoken == Token_ASCII_VALUE)
              {
                compoundStatement_ast *__node_105 = 0;
                if (!parse_compoundStatement(&__node_105))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_compoundStatement, "compoundStatement");
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
          }
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

            if (yytoken == Token_UNDEF
                || yytoken == Token_ALIAS
                || yytoken == Token_BEGIN
                || yytoken == Token_RETURN
                || yytoken == Token_BREAK
                || yytoken == Token_NEXT
                || yytoken == Token_NIL
                || yytoken == Token_TRUE
                || yytoken == Token_FALSE
                || yytoken == Token_FILE
                || yytoken == Token_LINE
                || yytoken == Token_SELF
                || yytoken == Token_SUPER
                || yytoken == Token_RETRY
                || yytoken == Token_YIELD
                || yytoken == Token_DEFINED
                || yytoken == Token_REDO
                || yytoken == Token_IF
                || yytoken == Token_CASE
                || yytoken == Token_UNTIL
                || yytoken == Token_WHILE
                || yytoken == Token_FOR
                || yytoken == Token_MODULE
                || yytoken == Token_DEF
                || yytoken == Token_CLASS
                || yytoken == Token_UNLESS
                || yytoken == Token_KEYWORD_NOT
                || yytoken == Token_END_UPCASE
                || yytoken == Token_BEGIN_UPCASE
                || yytoken == Token_LPAREN
                || yytoken == Token_SEMI
                || yytoken == Token_LEADING_TWO_COLON
                || yytoken == Token_LBRACK
                || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
                || yytoken == Token_LCURLY_HASH
                || yytoken == Token_REST_ARG_PREFIX
                || yytoken == Token_UNARY_MINUS
                || yytoken == Token_UNARY_PLUS
                || yytoken == Token_NOT
                || yytoken == Token_BNOT
                || yytoken == Token_LINE_BREAK
                || yytoken == Token_IDENTIFIER
                || yytoken == Token_CONSTANT
                || yytoken == Token_FUNCTION
                || yytoken == Token_GLOBAL_VARIABLE
                || yytoken == Token_INSTANCE_VARIABLE
                || yytoken == Token_CLASS_VARIABLE
                || yytoken == Token_DOUBLE_QUOTED_STRING
                || yytoken == Token_SINGLE_QUOTED_STRING
                || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_REGEX
                || yytoken == Token_COMMAND_OUTPUT
                || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_HERE_DOC_BEGIN
                || yytoken == Token_W_ARRAY
                || yytoken == Token_INTEGER
                || yytoken == Token_HEX
                || yytoken == Token_OCTAL
                || yytoken == Token_BINARY
                || yytoken == Token_FLOAT
                || yytoken == Token_ASCII_VALUE)
              {
                compoundStatement_ast *__node_106 = 0;
                if (!parse_compoundStatement(&__node_106))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_compoundStatement, "compoundStatement");
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
          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        if (yytoken != Token_END)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_END, "end");
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

  bool parser::parse_keyValuePair(keyValuePair_ast **yynode)
  {
    *yynode = create<keyValuePair_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BEGIN
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_KEYWORD_NOT
        || yytoken == Token_LPAREN
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_UNARY_MINUS
        || yytoken == Token_UNARY_PLUS
        || yytoken == Token_NOT
        || yytoken == Token_BNOT
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE)
      {
        expression_ast *__node_107 = 0;
        if (!parse_expression(&__node_107))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_expression, "expression");
              }
            return false;
          }
        if (yytoken == Token_ASSOC)
          {
            if (yytoken != Token_ASSOC)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_ASSOC, "=>");
                  }
                return false;
              }
            yylex();

            expression_ast *__node_108 = 0;
            if (!parse_expression(&__node_108))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_expression, "expression");
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
        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_keyword(keyword_ast **yynode)
  {
    *yynode = create<keyword_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_END
        || yytoken == Token_ELSE
        || yytoken == Token_ELSIF
        || yytoken == Token_RESCUE
        || yytoken == Token_ENSURE
        || yytoken == Token_WHEN
        || yytoken == Token_UNDEF
        || yytoken == Token_ALIAS
        || yytoken == Token_BEGIN
        || yytoken == Token_DO
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_THEN
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_OR
        || yytoken == Token_AND
        || yytoken == Token_IN
        || yytoken == Token_END_UPCASE
        || yytoken == Token_BEGIN_UPCASE
        || yytoken == Token_NOT)
      {
        if (yytoken == Token_END
            || yytoken == Token_ELSE
            || yytoken == Token_ELSIF
            || yytoken == Token_RESCUE
            || yytoken == Token_ENSURE
            || yytoken == Token_WHEN
            || yytoken == Token_UNDEF
            || yytoken == Token_ALIAS
            || yytoken == Token_BEGIN
            || yytoken == Token_DO
            || yytoken == Token_RETURN
            || yytoken == Token_BREAK
            || yytoken == Token_NEXT
            || yytoken == Token_SUPER
            || yytoken == Token_RETRY
            || yytoken == Token_YIELD
            || yytoken == Token_DEFINED
            || yytoken == Token_REDO
            || yytoken == Token_IF
            || yytoken == Token_CASE
            || yytoken == Token_UNTIL
            || yytoken == Token_WHILE
            || yytoken == Token_FOR
            || yytoken == Token_MODULE
            || yytoken == Token_DEF
            || yytoken == Token_THEN
            || yytoken == Token_CLASS
            || yytoken == Token_UNLESS
            || yytoken == Token_OR
            || yytoken == Token_AND
            || yytoken == Token_IN
            || yytoken == Token_END_UPCASE
            || yytoken == Token_BEGIN_UPCASE
            || yytoken == Token_NOT)
          {
            keywordAsMethodName_ast *__node_109 = 0;
            if (!parse_keywordAsMethodName(&__node_109))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_keywordAsMethodName, "keywordAsMethodName");
                  }
                return false;
              }
          }
        else if (yytoken == Token_NIL)
          {
            if (yytoken != Token_NIL)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_NIL, "nil");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_SELF)
          {
            if (yytoken != Token_SELF)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_SELF, "self");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_TRUE)
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

          }
        else if (yytoken == Token_FILE)
          {
            if (yytoken != Token_FILE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_FILE, "__FILE__");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_LINE)
          {
            if (yytoken != Token_LINE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE, "__LINE__");
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

  bool parser::parse_keywordAlias(keywordAlias_ast **yynode)
  {
    *yynode = create<keywordAlias_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_ALIAS)
      {
        if (yytoken != Token_ALIAS)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_ALIAS, "alias");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_keywordAnd(keywordAnd_ast **yynode)
  {
    *yynode = create<keywordAnd_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_AND)
      {
        if (yytoken != Token_AND)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_AND, "and");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_keywordAsMethodName(keywordAsMethodName_ast **yynode)
  {
    *yynode = create<keywordAsMethodName_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_END
        || yytoken == Token_ELSE
        || yytoken == Token_ELSIF
        || yytoken == Token_RESCUE
        || yytoken == Token_ENSURE
        || yytoken == Token_WHEN
        || yytoken == Token_UNDEF
        || yytoken == Token_ALIAS
        || yytoken == Token_BEGIN
        || yytoken == Token_DO
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_THEN
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_OR
        || yytoken == Token_AND
        || yytoken == Token_IN
        || yytoken == Token_END_UPCASE
        || yytoken == Token_BEGIN_UPCASE
        || yytoken == Token_NOT)
      {
        if (yytoken == Token_AND)
          {
            if (yytoken != Token_AND)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_AND, "and");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_DEF)
          {
            if (yytoken != Token_DEF)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_DEF, "def");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_END)
          {
            if (yytoken != Token_END)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_END, "end");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_IN)
          {
            if (yytoken != Token_IN)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_IN, "in");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_OR)
          {
            if (yytoken != Token_OR)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_OR, "or");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_UNLESS)
          {
            if (yytoken != Token_UNLESS)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_UNLESS, "unless");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_BEGIN)
          {
            if (yytoken != Token_BEGIN)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_BEGIN, "begin");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_DEFINED)
          {
            if (yytoken != Token_DEFINED)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_DEFINED, "defined?");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_ENSURE)
          {
            if (yytoken != Token_ENSURE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_ENSURE, "ensure");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_MODULE)
          {
            if (yytoken != Token_MODULE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_MODULE, "module");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_REDO)
          {
            if (yytoken != Token_REDO)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_REDO, "redo");
                  }
                return false;
              }
            yylex();

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

          }
        else if (yytoken == Token_UNTIL)
          {
            if (yytoken != Token_UNTIL)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_UNTIL, "until");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_BEGIN_UPCASE)
          {
            if (yytoken != Token_BEGIN_UPCASE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_BEGIN_UPCASE, "BEGIN");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_BREAK)
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

          }
        else if (yytoken == Token_DO)
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

          }
        else if (yytoken == Token_NEXT)
          {
            if (yytoken != Token_NEXT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_NEXT, "next");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_RESCUE)
          {
            if (yytoken != Token_RESCUE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_RESCUE, "rescue");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_THEN)
          {
            if (yytoken != Token_THEN)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_THEN, "then");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_WHEN)
          {
            if (yytoken != Token_WHEN)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_WHEN, "when");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_END_UPCASE)
          {
            if (yytoken != Token_END_UPCASE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_END_UPCASE, "END");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_CASE)
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

          }
        else if (yytoken == Token_ELSE)
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

          }
        else if (yytoken == Token_FOR)
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

          }
        else if (yytoken == Token_RETRY)
          {
            if (yytoken != Token_RETRY)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_RETRY, "retry");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_WHILE)
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

          }
        else if (yytoken == Token_ALIAS)
          {
            if (yytoken != Token_ALIAS)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_ALIAS, "alias");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_CLASS)
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

          }
        else if (yytoken == Token_ELSIF)
          {
            if (yytoken != Token_ELSIF)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_ELSIF, "elsif");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_IF)
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

          }
        else if (yytoken == Token_NOT)
          {
            if (yytoken != Token_NOT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_NOT, "!");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_RETURN)
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

          }
        else if (yytoken == Token_UNDEF)
          {
            if (yytoken != Token_UNDEF)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_UNDEF, "undef");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_YIELD)
          {
            if (yytoken != Token_YIELD)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_YIELD, "yield");
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

  bool parser::parse_keywordBeginUpcase(keywordBeginUpcase_ast **yynode)
  {
    *yynode = create<keywordBeginUpcase_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BEGIN_UPCASE)
      {
        if (yytoken != Token_BEGIN_UPCASE)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_BEGIN_UPCASE, "BEGIN");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_keywordBreak(keywordBreak_ast **yynode)
  {
    *yynode = create<keywordBreak_ast>();

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

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_keywordClass(keywordClass_ast **yynode)
  {
    *yynode = create<keywordClass_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

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

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_keywordDef(keywordDef_ast **yynode)
  {
    *yynode = create<keywordDef_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_DEF)
      {
        if (yytoken != Token_DEF)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_DEF, "def");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_keywordDefined(keywordDefined_ast **yynode)
  {
    *yynode = create<keywordDefined_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_DEFINED)
      {
        if (yytoken != Token_DEFINED)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_DEFINED, "defined?");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_keywordDo(keywordDo_ast **yynode)
  {
    *yynode = create<keywordDo_ast>();

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

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_keywordEndUpcase(keywordEndUpcase_ast **yynode)
  {
    *yynode = create<keywordEndUpcase_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_END_UPCASE)
      {
        if (yytoken != Token_END_UPCASE)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_END_UPCASE, "END");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_keywordFor(keywordFor_ast **yynode)
  {
    *yynode = create<keywordFor_ast>();

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

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_keywordIn(keywordIn_ast **yynode)
  {
    *yynode = create<keywordIn_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_IN)
      {
        if (yytoken != Token_IN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_IN, "in");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_keywordModule(keywordModule_ast **yynode)
  {
    *yynode = create<keywordModule_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_MODULE)
      {
        if (yytoken != Token_MODULE)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_MODULE, "module");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_keywordNot(keywordNot_ast **yynode)
  {
    *yynode = create<keywordNot_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_KEYWORD_NOT)
      {
        if (yytoken != Token_KEYWORD_NOT)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_KEYWORD_NOT, "not");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_keywordOr(keywordOr_ast **yynode)
  {
    *yynode = create<keywordOr_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_OR)
      {
        if (yytoken != Token_OR)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_OR, "or");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_keywordUndef(keywordUndef_ast **yynode)
  {
    *yynode = create<keywordUndef_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_UNDEF)
      {
        if (yytoken != Token_UNDEF)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_UNDEF, "undef");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_keywordUntil(keywordUntil_ast **yynode)
  {
    *yynode = create<keywordUntil_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_UNTIL)
      {
        if (yytoken != Token_UNTIL)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_UNTIL, "until");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_keywordWhen(keywordWhen_ast **yynode)
  {
    *yynode = create<keywordWhen_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_WHEN)
      {
        if (yytoken != Token_WHEN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_WHEN, "when");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_keywordWhile(keywordWhile_ast **yynode)
  {
    *yynode = create<keywordWhile_ast>();

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

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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

    if (yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY)
      {
        if (yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_REGEX)
          {
            regex_ast *__node_110 = 0;
            if (!parse_regex(&__node_110))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_regex, "regex");
                  }
                return false;
              }
          }
        else if (yytoken == Token_DOUBLE_QUOTED_STRING
                 || yytoken == Token_SINGLE_QUOTED_STRING
                 || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION)
          {
            do
              {
                string_ast *__node_111 = 0;
                if (!parse_string(&__node_111))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_string, "string");
                      }
                    return false;
                  }
              }
            while (yytoken == Token_DOUBLE_QUOTED_STRING
                   || yytoken == Token_SINGLE_QUOTED_STRING
                   || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION);
          }
        else if (yytoken == Token_HERE_DOC_BEGIN)
          {
            if (yytoken != Token_HERE_DOC_BEGIN)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_HERE_DOC_BEGIN, "here document beginning");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_COMMAND_OUTPUT
                 || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION)
          {
            commandOutput_ast *__node_112 = 0;
            if (!parse_commandOutput(&__node_112))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_commandOutput, "commandOutput");
                  }
                return false;
              }
          }
        else if (yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE)
          {
            symbol_ast *__node_113 = 0;
            if (!parse_symbol(&__node_113))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_symbol, "symbol");
                  }
                return false;
              }
          }
        else if (yytoken == Token_W_ARRAY)
          {
            if (yytoken != Token_W_ARRAY)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_W_ARRAY, "%w{} array");
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

  bool parser::parse_logicalAndExpression(logicalAndExpression_ast **yynode)
  {
    *yynode = create<logicalAndExpression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BEGIN
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_LPAREN
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_UNARY_MINUS
        || yytoken == Token_UNARY_PLUS
        || yytoken == Token_NOT
        || yytoken == Token_BNOT
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE)
      {
        equalityExpression_ast *__node_114 = 0;
        if (!parse_equalityExpression(&__node_114))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_equalityExpression, "equalityExpression");
              }
            return false;
          }
        while (yytoken == Token_LOGICAL_AND)
          {
            operatorLogicalAnd_ast *__node_115 = 0;
            if (!parse_operatorLogicalAnd(&__node_115))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_operatorLogicalAnd, "operatorLogicalAnd");
                  }
                return false;
              }
            equalityExpression_ast *__node_116 = 0;
            if (!parse_equalityExpression(&__node_116))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_equalityExpression, "equalityExpression");
                  }
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

  bool parser::parse_logicalOrExpression(logicalOrExpression_ast **yynode)
  {
    *yynode = create<logicalOrExpression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BEGIN
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_LPAREN
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_UNARY_MINUS
        || yytoken == Token_UNARY_PLUS
        || yytoken == Token_NOT
        || yytoken == Token_BNOT
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE)
      {
        logicalAndExpression_ast *__node_117 = 0;
        if (!parse_logicalAndExpression(&__node_117))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_logicalAndExpression, "logicalAndExpression");
              }
            return false;
          }
        while (yytoken == Token_LOGICAL_OR)
          {
            operatorLogicalOr_ast *__node_118 = 0;
            if (!parse_operatorLogicalOr(&__node_118))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_operatorLogicalOr, "operatorLogicalOr");
                  }
                return false;
              }
            logicalAndExpression_ast *__node_119 = 0;
            if (!parse_logicalAndExpression(&__node_119))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_logicalAndExpression, "logicalAndExpression");
                  }
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

  bool parser::parse_methodCall(methodCall_ast **yynode)
  {
    *yynode = create<methodCall_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BEGIN
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_LPAREN
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE)
      {
        primaryExpression_ast *__node_120 = 0;
        if (!parse_primaryExpression(&__node_120))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_primaryExpression, "primaryExpression");
              }
            return false;
          }
        if (yytoken == Token_LPAREN)
          {
            methodInvocationArgumentWithParen_ast *__node_121 = 0;
            if (!parse_methodInvocationArgumentWithParen(&__node_121))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_methodInvocationArgumentWithParen, "methodInvocationArgumentWithParen");
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
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_methodDefinition(methodDefinition_ast **yynode)
  {
    *yynode = create<methodDefinition_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_DEF)
      {
        keywordDef_ast *__node_122 = 0;
        if (!parse_keywordDef(&__node_122))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_keywordDef, "keywordDef");
              }
            return false;
          }
        methodName_ast *__node_123 = 0;
        if (!parse_methodName(&__node_123))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_methodName, "methodName");
              }
            return false;
          }
        methodDefinitionArgument_ast *__node_124 = 0;
        if (!parse_methodDefinitionArgument(&__node_124))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_methodDefinitionArgument, "methodDefinitionArgument");
              }
            return false;
          }
        bodyStatement_ast *__node_125 = 0;
        if (!parse_bodyStatement(&__node_125))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_bodyStatement, "bodyStatement");
              }
            return false;
          }
        if (yytoken != Token_END)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_END, "end");
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

  bool parser::parse_methodDefinitionArgument(methodDefinitionArgument_ast **yynode)
  {
    *yynode = create<methodDefinitionArgument_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_LPAREN
        || yytoken == Token_SEMI
        || yytoken == Token_BAND
        || yytoken == Token_STAR
        || yytoken == Token_LINE_BREAK
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_FUNCTION)
      {
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

            if (yytoken == Token_BAND
                || yytoken == Token_STAR
                || yytoken == Token_IDENTIFIER
                || yytoken == Token_FUNCTION)
              {
                methodDefinitionArgumentWithoutParen_ast *__node_126 = 0;
                if (!parse_methodDefinitionArgumentWithoutParen(&__node_126))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_methodDefinitionArgumentWithoutParen, "methodDefinitionArgumentWithoutParen");
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
            if (yytoken != Token_RPAREN)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_RPAREN, ")");
                  }
                return false;
              }
            yylex();

            if (yytoken == Token_SEMI
                || yytoken == Token_LINE_BREAK)
              {
                terminal_ast *__node_127 = 0;
                if (!parse_terminal(&__node_127))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_terminal, "terminal");
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
          }
        else if (yytoken == Token_SEMI
                 || yytoken == Token_BAND
                 || yytoken == Token_STAR
                 || yytoken == Token_LINE_BREAK
                 || yytoken == Token_IDENTIFIER
                 || yytoken == Token_FUNCTION)
          {
            if (yytoken == Token_BAND
                || yytoken == Token_STAR
                || yytoken == Token_IDENTIFIER
                || yytoken == Token_FUNCTION)
              {
                methodDefinitionArgumentWithoutParen_ast *__node_128 = 0;
                if (!parse_methodDefinitionArgumentWithoutParen(&__node_128))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_methodDefinitionArgumentWithoutParen, "methodDefinitionArgumentWithoutParen");
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
            terminal_ast *__node_129 = 0;
            if (!parse_terminal(&__node_129))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_terminal, "terminal");
                  }
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

  bool parser::parse_methodDefinitionArgumentWithoutParen(methodDefinitionArgumentWithoutParen_ast **yynode)
  {
    *yynode = create<methodDefinitionArgumentWithoutParen_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BAND
        || yytoken == Token_STAR
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_FUNCTION)
      {
        if (yytoken == Token_IDENTIFIER
            || yytoken == Token_FUNCTION)
          {
            normalMethodDefinitionArgument_ast *__node_130 = 0;
            if (!parse_normalMethodDefinitionArgument(&__node_130))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_normalMethodDefinitionArgument, "normalMethodDefinitionArgument");
                  }
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

                if (Token_STAR == yytoken || Token_BAND == yytoken)
                  {
                    expect_array_or_block_arguments = true;
                    break;
                  }
                normalMethodDefinitionArgument_ast *__node_131 = 0;
                if (!parse_normalMethodDefinitionArgument(&__node_131))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_normalMethodDefinitionArgument, "normalMethodDefinitionArgument");
                      }
                    return false;
                  }
              }
            if ((yytoken == Token_STAR) && ( expect_array_or_block_arguments ))
              {
                restMethodDefinitionArgument_ast *__node_132 = 0;
                if (!parse_restMethodDefinitionArgument(&__node_132))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_restMethodDefinitionArgument, "restMethodDefinitionArgument");
                      }
                    return false;
                  }
              }
            else if ((yytoken == Token_BAND) && ( expect_array_or_block_arguments ))
              {
                blockMethodDefinitionArgument_ast *__node_133 = 0;
                if (!parse_blockMethodDefinitionArgument(&__node_133))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_blockMethodDefinitionArgument, "blockMethodDefinitionArgument");
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
          }
        else if ((yytoken == Token_STAR) && ( (expect_array_or_block_arguments = true) ))
          {
            restMethodDefinitionArgument_ast *__node_134 = 0;
            if (!parse_restMethodDefinitionArgument(&__node_134))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_restMethodDefinitionArgument, "restMethodDefinitionArgument");
                  }
                return false;
              }
          }
        else if ((yytoken == Token_BAND) && ( (expect_array_or_block_arguments = true) ))
          {
            blockMethodDefinitionArgument_ast *__node_135 = 0;
            if (!parse_blockMethodDefinitionArgument(&__node_135))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_blockMethodDefinitionArgument, "blockMethodDefinitionArgument");
                  }
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

  bool parser::parse_methodInvocationArgumentWithParen(methodInvocationArgumentWithParen_ast **yynode)
  {
    *yynode = create<methodInvocationArgumentWithParen_ast>();

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

        if (yytoken == Token_BEGIN
            || yytoken == Token_RETURN
            || yytoken == Token_BREAK
            || yytoken == Token_NEXT
            || yytoken == Token_NIL
            || yytoken == Token_TRUE
            || yytoken == Token_FALSE
            || yytoken == Token_FILE
            || yytoken == Token_LINE
            || yytoken == Token_SELF
            || yytoken == Token_SUPER
            || yytoken == Token_RETRY
            || yytoken == Token_YIELD
            || yytoken == Token_DEFINED
            || yytoken == Token_REDO
            || yytoken == Token_IF
            || yytoken == Token_CASE
            || yytoken == Token_UNTIL
            || yytoken == Token_WHILE
            || yytoken == Token_FOR
            || yytoken == Token_MODULE
            || yytoken == Token_DEF
            || yytoken == Token_CLASS
            || yytoken == Token_UNLESS
            || yytoken == Token_KEYWORD_NOT
            || yytoken == Token_LPAREN
            || yytoken == Token_LEADING_TWO_COLON
            || yytoken == Token_LBRACK
            || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
            || yytoken == Token_LCURLY_HASH
            || yytoken == Token_REST_ARG_PREFIX
            || yytoken == Token_UNARY_MINUS
            || yytoken == Token_UNARY_PLUS
            || yytoken == Token_NOT
            || yytoken == Token_BNOT
            || yytoken == Token_IDENTIFIER
            || yytoken == Token_CONSTANT
            || yytoken == Token_FUNCTION
            || yytoken == Token_GLOBAL_VARIABLE
            || yytoken == Token_INSTANCE_VARIABLE
            || yytoken == Token_CLASS_VARIABLE
            || yytoken == Token_DOUBLE_QUOTED_STRING
            || yytoken == Token_SINGLE_QUOTED_STRING
            || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_REGEX
            || yytoken == Token_COMMAND_OUTPUT
            || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_HERE_DOC_BEGIN
            || yytoken == Token_W_ARRAY
            || yytoken == Token_INTEGER
            || yytoken == Token_HEX
            || yytoken == Token_OCTAL
            || yytoken == Token_BINARY
            || yytoken == Token_FLOAT
            || yytoken == Token_ASCII_VALUE
            || yytoken == Token_BLOCK_ARG_PREFIX)
          {
            methodInvocationArgumentWithoutParen_ast *__node_136 = 0;
            if (!parse_methodInvocationArgumentWithoutParen(&__node_136))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_methodInvocationArgumentWithoutParen, "methodInvocationArgumentWithoutParen");
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
        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_methodInvocationArgumentWithoutParen(methodInvocationArgumentWithoutParen_ast **yynode)
  {
    *yynode = create<methodInvocationArgumentWithoutParen_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BEGIN
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_KEYWORD_NOT
        || yytoken == Token_LPAREN
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_REST_ARG_PREFIX
        || yytoken == Token_UNARY_MINUS
        || yytoken == Token_UNARY_PLUS
        || yytoken == Token_NOT
        || yytoken == Token_BNOT
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE
        || yytoken == Token_BLOCK_ARG_PREFIX)
      {
        if (yytoken == Token_BEGIN
            || yytoken == Token_RETURN
            || yytoken == Token_BREAK
            || yytoken == Token_NEXT
            || yytoken == Token_NIL
            || yytoken == Token_TRUE
            || yytoken == Token_FALSE
            || yytoken == Token_FILE
            || yytoken == Token_LINE
            || yytoken == Token_SELF
            || yytoken == Token_SUPER
            || yytoken == Token_RETRY
            || yytoken == Token_YIELD
            || yytoken == Token_DEFINED
            || yytoken == Token_REDO
            || yytoken == Token_IF
            || yytoken == Token_CASE
            || yytoken == Token_UNTIL
            || yytoken == Token_WHILE
            || yytoken == Token_FOR
            || yytoken == Token_MODULE
            || yytoken == Token_DEF
            || yytoken == Token_CLASS
            || yytoken == Token_UNLESS
            || yytoken == Token_KEYWORD_NOT
            || yytoken == Token_LPAREN
            || yytoken == Token_LEADING_TWO_COLON
            || yytoken == Token_LBRACK
            || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
            || yytoken == Token_LCURLY_HASH
            || yytoken == Token_UNARY_MINUS
            || yytoken == Token_UNARY_PLUS
            || yytoken == Token_NOT
            || yytoken == Token_BNOT
            || yytoken == Token_IDENTIFIER
            || yytoken == Token_CONSTANT
            || yytoken == Token_FUNCTION
            || yytoken == Token_GLOBAL_VARIABLE
            || yytoken == Token_INSTANCE_VARIABLE
            || yytoken == Token_CLASS_VARIABLE
            || yytoken == Token_DOUBLE_QUOTED_STRING
            || yytoken == Token_SINGLE_QUOTED_STRING
            || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_REGEX
            || yytoken == Token_COMMAND_OUTPUT
            || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_HERE_DOC_BEGIN
            || yytoken == Token_W_ARRAY
            || yytoken == Token_INTEGER
            || yytoken == Token_HEX
            || yytoken == Token_OCTAL
            || yytoken == Token_BINARY
            || yytoken == Token_FLOAT
            || yytoken == Token_ASCII_VALUE)
          {
            normalMethodInvocationArgument_ast *__node_137 = 0;
            if (!parse_normalMethodInvocationArgument(&__node_137))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_normalMethodInvocationArgument, "normalMethodInvocationArgument");
                  }
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

                if ((yytoken == Token_REST_ARG_PREFIX) || (yytoken == Token_BLOCK_ARG_PREFIX))
                  break;
                normalMethodInvocationArgument_ast *__node_138 = 0;
                if (!parse_normalMethodInvocationArgument(&__node_138))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_normalMethodInvocationArgument, "normalMethodInvocationArgument");
                      }
                    return false;
                  }
              }
            if (yytoken == Token_REST_ARG_PREFIX)
              {
                restMethodInvocationArgument_ast *__node_139 = 0;
                if (!parse_restMethodInvocationArgument(&__node_139))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_restMethodInvocationArgument, "restMethodInvocationArgument");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_BLOCK_ARG_PREFIX)
              {
                blockMethodInvocationArgument_ast *__node_140 = 0;
                if (!parse_blockMethodInvocationArgument(&__node_140))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_blockMethodInvocationArgument, "blockMethodInvocationArgument");
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
          }
        else if (yytoken == Token_REST_ARG_PREFIX)
          {
            restMethodInvocationArgument_ast *__node_141 = 0;
            if (!parse_restMethodInvocationArgument(&__node_141))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_restMethodInvocationArgument, "restMethodInvocationArgument");
                  }
                return false;
              }
          }
        else if (yytoken == Token_BLOCK_ARG_PREFIX)
          {
            blockMethodInvocationArgument_ast *__node_142 = 0;
            if (!parse_blockMethodInvocationArgument(&__node_142))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_blockMethodInvocationArgument, "blockMethodInvocationArgument");
                  }
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

  bool parser::parse_methodName(methodName_ast **yynode)
  {
    *yynode = create<methodName_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_END
        || yytoken == Token_ELSE
        || yytoken == Token_ELSIF
        || yytoken == Token_RESCUE
        || yytoken == Token_ENSURE
        || yytoken == Token_WHEN
        || yytoken == Token_UNDEF
        || yytoken == Token_ALIAS
        || yytoken == Token_BEGIN
        || yytoken == Token_DO
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_THEN
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_OR
        || yytoken == Token_AND
        || yytoken == Token_IN
        || yytoken == Token_END_UPCASE
        || yytoken == Token_BEGIN_UPCASE
        || yytoken == Token_LPAREN
        || yytoken == Token_SINGLE_QUOTE
        || yytoken == Token_LBRACK
        || yytoken == Token_LBRACK_ARRAY_ACCESS
        || yytoken == Token_BOR
        || yytoken == Token_COMPARE
        || yytoken == Token_EQUAL
        || yytoken == Token_CASE_EQUAL
        || yytoken == Token_MATCH
        || yytoken == Token_LESS_THAN
        || yytoken == Token_GREATER_THAN
        || yytoken == Token_LESS_OR_EQUAL
        || yytoken == Token_GREATER_OR_EQUAL
        || yytoken == Token_BXOR
        || yytoken == Token_BAND
        || yytoken == Token_LEFT_SHIFT
        || yytoken == Token_RIGHT_SHIFT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_STAR
        || yytoken == Token_DIV
        || yytoken == Token_MOD
        || yytoken == Token_NOT
        || yytoken == Token_BNOT
        || yytoken == Token_POWER
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_UNARY_PLUS_MINUS_METHOD_NAME)
      {
        if (yytoken == Token_SINGLE_QUOTE
            || yytoken == Token_LBRACK
            || yytoken == Token_LBRACK_ARRAY_ACCESS
            || yytoken == Token_BOR
            || yytoken == Token_COMPARE
            || yytoken == Token_EQUAL
            || yytoken == Token_CASE_EQUAL
            || yytoken == Token_MATCH
            || yytoken == Token_LESS_THAN
            || yytoken == Token_GREATER_THAN
            || yytoken == Token_LESS_OR_EQUAL
            || yytoken == Token_GREATER_OR_EQUAL
            || yytoken == Token_BXOR
            || yytoken == Token_BAND
            || yytoken == Token_LEFT_SHIFT
            || yytoken == Token_RIGHT_SHIFT
            || yytoken == Token_PLUS
            || yytoken == Token_MINUS
            || yytoken == Token_STAR
            || yytoken == Token_DIV
            || yytoken == Token_MOD
            || yytoken == Token_BNOT
            || yytoken == Token_POWER)
          {
            operatorAsMethodname_ast *__node_143 = 0;
            if (!parse_operatorAsMethodname(&__node_143))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_operatorAsMethodname, "operatorAsMethodname");
                  }
                return false;
              }
          }
        else if (yytoken == Token_END
                 || yytoken == Token_ELSE
                 || yytoken == Token_ELSIF
                 || yytoken == Token_RESCUE
                 || yytoken == Token_ENSURE
                 || yytoken == Token_WHEN
                 || yytoken == Token_UNDEF
                 || yytoken == Token_ALIAS
                 || yytoken == Token_BEGIN
                 || yytoken == Token_DO
                 || yytoken == Token_RETURN
                 || yytoken == Token_BREAK
                 || yytoken == Token_NEXT
                 || yytoken == Token_SUPER
                 || yytoken == Token_RETRY
                 || yytoken == Token_YIELD
                 || yytoken == Token_DEFINED
                 || yytoken == Token_REDO
                 || yytoken == Token_IF
                 || yytoken == Token_CASE
                 || yytoken == Token_UNTIL
                 || yytoken == Token_WHILE
                 || yytoken == Token_FOR
                 || yytoken == Token_MODULE
                 || yytoken == Token_DEF
                 || yytoken == Token_THEN
                 || yytoken == Token_CLASS
                 || yytoken == Token_UNLESS
                 || yytoken == Token_OR
                 || yytoken == Token_AND
                 || yytoken == Token_IN
                 || yytoken == Token_END_UPCASE
                 || yytoken == Token_BEGIN_UPCASE
                 || yytoken == Token_NOT)
          {
            keywordAsMethodName_ast *__node_144 = 0;
            if (!parse_keywordAsMethodName(&__node_144))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_keywordAsMethodName, "keywordAsMethodName");
                  }
                return false;
              }
          }
        else if (yytoken == Token_IDENTIFIER
                 || yytoken == Token_CONSTANT
                 || yytoken == Token_FUNCTION)
          {
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
                yylex();

              }
            else if (yytoken == Token_FUNCTION)
              {
                if (yytoken != Token_FUNCTION)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_FUNCTION, "function");
                      }
                    return false;
                  }
                yylex();

              }
            else if (yytoken == Token_CONSTANT)
              {
                if (yytoken != Token_CONSTANT)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_CONSTANT, "constant");
                      }
                    return false;
                  }
                yylex();

              }
            else
              {
                return false;
              }
            if (yytoken == Token_TWO_COLON
                || yytoken == Token_DOT)
              {
                methodNameSupplement_ast *__node_145 = 0;
                if (!parse_methodNameSupplement(&__node_145))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_methodNameSupplement, "methodNameSupplement");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_ASSIGN_WITH_NO_LEADING_SPACE)
              {
                if (yytoken != Token_ASSIGN_WITH_NO_LEADING_SPACE)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_ASSIGN_WITH_NO_LEADING_SPACE, "=");
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

            expression_ast *__node_146 = 0;
            if (!parse_expression(&__node_146))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_expression, "expression");
                  }
                return false;
              }
            if (yytoken == Token_LINE_BREAK)
              {
                if (yytoken != Token_LINE_BREAK)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
            if (yytoken != Token_RPAREN)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_RPAREN, ")");
                  }
                return false;
              }
            yylex();

            methodNameSupplement_ast *__node_147 = 0;
            if (!parse_methodNameSupplement(&__node_147))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_methodNameSupplement, "methodNameSupplement");
                  }
                return false;
              }
          }
        else if (yytoken == Token_GLOBAL_VARIABLE
                 || yytoken == Token_INSTANCE_VARIABLE
                 || yytoken == Token_CLASS_VARIABLE)
          {
            if (yytoken == Token_INSTANCE_VARIABLE)
              {
                if (yytoken != Token_INSTANCE_VARIABLE)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_INSTANCE_VARIABLE, "instance variable");
                      }
                    return false;
                  }
                yylex();

              }
            else if (yytoken == Token_CLASS_VARIABLE)
              {
                if (yytoken != Token_CLASS_VARIABLE)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_CLASS_VARIABLE, "class variable");
                      }
                    return false;
                  }
                yylex();

              }
            else if (yytoken == Token_GLOBAL_VARIABLE)
              {
                if (yytoken != Token_GLOBAL_VARIABLE)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_GLOBAL_VARIABLE, "global variable");
                      }
                    return false;
                  }
                yylex();

              }
            else
              {
                return false;
              }
            methodNameSupplement_ast *__node_148 = 0;
            if (!parse_methodNameSupplement(&__node_148))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_methodNameSupplement, "methodNameSupplement");
                  }
                return false;
              }
          }
        else if (yytoken == Token_NIL
                 || yytoken == Token_TRUE
                 || yytoken == Token_FALSE
                 || yytoken == Token_FILE
                 || yytoken == Token_LINE
                 || yytoken == Token_SELF)
          {
            if (yytoken == Token_NIL)
              {
                if (yytoken != Token_NIL)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_NIL, "nil");
                      }
                    return false;
                  }
                yylex();

              }
            else if (yytoken == Token_SELF)
              {
                if (yytoken != Token_SELF)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_SELF, "self");
                      }
                    return false;
                  }
                yylex();

              }
            else if (yytoken == Token_TRUE)
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

              }
            else if (yytoken == Token_FILE)
              {
                if (yytoken != Token_FILE)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_FILE, "__FILE__");
                      }
                    return false;
                  }
                yylex();

              }
            else if (yytoken == Token_LINE)
              {
                if (yytoken != Token_LINE)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_LINE, "__LINE__");
                      }
                    return false;
                  }
                yylex();

              }
            else
              {
                return false;
              }
            methodNameSupplement_ast *__node_149 = 0;
            if (!parse_methodNameSupplement(&__node_149))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_methodNameSupplement, "methodNameSupplement");
                  }
                return false;
              }
          }
        else if (yytoken == Token_UNARY_PLUS_MINUS_METHOD_NAME)
          {
            if (yytoken != Token_UNARY_PLUS_MINUS_METHOD_NAME)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_UNARY_PLUS_MINUS_METHOD_NAME, "+/- method");
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

  bool parser::parse_methodNameSupplement(methodNameSupplement_ast **yynode)
  {
    *yynode = create<methodNameSupplement_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_TWO_COLON
        || yytoken == Token_DOT)
      {
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

          }
        else if (yytoken == Token_TWO_COLON)
          {
            if (yytoken != Token_TWO_COLON)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_TWO_COLON, "::");
                  }
                return false;
              }
            yylex();

          }
        else
          {
            return false;
          }
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
            yylex();

            if (yytoken == Token_ASSIGN_WITH_NO_LEADING_SPACE)
              {
                if (yytoken != Token_ASSIGN_WITH_NO_LEADING_SPACE)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_ASSIGN_WITH_NO_LEADING_SPACE, "=");
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
        else if (yytoken == Token_FUNCTION)
          {
            if (yytoken != Token_FUNCTION)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_FUNCTION, "function");
                  }
                return false;
              }
            yylex();

            if (yytoken == Token_ASSIGN_WITH_NO_LEADING_SPACE)
              {
                if (yytoken != Token_ASSIGN_WITH_NO_LEADING_SPACE)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_ASSIGN_WITH_NO_LEADING_SPACE, "=");
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
        else if (yytoken == Token_CONSTANT)
          {
            if (yytoken != Token_CONSTANT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_CONSTANT, "constant");
                  }
                return false;
              }
            yylex();

            if (yytoken == Token_ASSIGN_WITH_NO_LEADING_SPACE)
              {
                if (yytoken != Token_ASSIGN_WITH_NO_LEADING_SPACE)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_ASSIGN_WITH_NO_LEADING_SPACE, "=");
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
        else if (yytoken == Token_SINGLE_QUOTE
                 || yytoken == Token_LBRACK
                 || yytoken == Token_LBRACK_ARRAY_ACCESS
                 || yytoken == Token_BOR
                 || yytoken == Token_COMPARE
                 || yytoken == Token_EQUAL
                 || yytoken == Token_CASE_EQUAL
                 || yytoken == Token_MATCH
                 || yytoken == Token_LESS_THAN
                 || yytoken == Token_GREATER_THAN
                 || yytoken == Token_LESS_OR_EQUAL
                 || yytoken == Token_GREATER_OR_EQUAL
                 || yytoken == Token_BXOR
                 || yytoken == Token_BAND
                 || yytoken == Token_LEFT_SHIFT
                 || yytoken == Token_RIGHT_SHIFT
                 || yytoken == Token_PLUS
                 || yytoken == Token_MINUS
                 || yytoken == Token_STAR
                 || yytoken == Token_DIV
                 || yytoken == Token_MOD
                 || yytoken == Token_BNOT
                 || yytoken == Token_POWER)
          {
            operatorAsMethodname_ast *__node_150 = 0;
            if (!parse_operatorAsMethodname(&__node_150))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_operatorAsMethodname, "operatorAsMethodname");
                  }
                return false;
              }
          }
        else if (yytoken == Token_END
                 || yytoken == Token_ELSE
                 || yytoken == Token_ELSIF
                 || yytoken == Token_RESCUE
                 || yytoken == Token_ENSURE
                 || yytoken == Token_WHEN
                 || yytoken == Token_UNDEF
                 || yytoken == Token_ALIAS
                 || yytoken == Token_BEGIN
                 || yytoken == Token_DO
                 || yytoken == Token_RETURN
                 || yytoken == Token_BREAK
                 || yytoken == Token_NEXT
                 || yytoken == Token_NIL
                 || yytoken == Token_TRUE
                 || yytoken == Token_FALSE
                 || yytoken == Token_FILE
                 || yytoken == Token_LINE
                 || yytoken == Token_SELF
                 || yytoken == Token_SUPER
                 || yytoken == Token_RETRY
                 || yytoken == Token_YIELD
                 || yytoken == Token_DEFINED
                 || yytoken == Token_REDO
                 || yytoken == Token_IF
                 || yytoken == Token_CASE
                 || yytoken == Token_UNTIL
                 || yytoken == Token_WHILE
                 || yytoken == Token_FOR
                 || yytoken == Token_MODULE
                 || yytoken == Token_DEF
                 || yytoken == Token_THEN
                 || yytoken == Token_CLASS
                 || yytoken == Token_UNLESS
                 || yytoken == Token_OR
                 || yytoken == Token_AND
                 || yytoken == Token_IN
                 || yytoken == Token_END_UPCASE
                 || yytoken == Token_BEGIN_UPCASE
                 || yytoken == Token_NOT)
          {
            keyword_ast *__node_151 = 0;
            if (!parse_keyword(&__node_151))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_keyword, "keyword");
                  }
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

  bool parser::parse_mlhs_item(mlhs_item_ast **yynode)
  {
    *yynode = create<mlhs_item_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BEGIN
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_LPAREN
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE)
      {
        bool block_errors_2 = block_errors(true);
        std::size_t try_start_token_2 = token_stream->index() - 1;
        parser_state *try_start_state_2 = copy_current_state();
        {
          if (yytoken != Token_LPAREN)
            goto __catch_2;
          yylex();

          dotAccess_ast *__node_152 = 0;
          if (!parse_dotAccess(&__node_152))
            {
              goto __catch_2;
            }
          do
            {
              if (yytoken != Token_COMMA)
                goto __catch_2;
              yylex();

              dotAccess_ast *__node_153 = 0;
              if (!parse_dotAccess(&__node_153))
                {
                  goto __catch_2;
                }
            }
          while (yytoken == Token_COMMA);
          if (yytoken != Token_RPAREN)
            goto __catch_2;
          yylex();

        }
        block_errors(block_errors_2);
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
            block_errors(block_errors_2);
            rewind(try_start_token_2);

            dotAccess_ast *__node_154 = 0;
            if (!parse_dotAccess(&__node_154))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_dotAccess, "dotAccess");
                  }
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

  bool parser::parse_moduleDefinition(moduleDefinition_ast **yynode)
  {
    *yynode = create<moduleDefinition_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_MODULE)
      {
        keywordModule_ast *__node_155 = 0;
        if (!parse_keywordModule(&__node_155))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_keywordModule, "keywordModule");
              }
            return false;
          }
        moduleName_ast *__node_156 = 0;
        if (!parse_moduleName(&__node_156))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_moduleName, "moduleName");
              }
            return false;
          }
        terminal_ast *__node_157 = 0;
        if (!parse_terminal(&__node_157))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_terminal, "terminal");
              }
            return false;
          }
        bodyStatement_ast *__node_158 = 0;
        if (!parse_bodyStatement(&__node_158))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_bodyStatement, "bodyStatement");
              }
            return false;
          }
        if (yytoken != Token_END)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_END, "end");
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

  bool parser::parse_moduleName(moduleName_ast **yynode)
  {
    *yynode = create<moduleName_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_CONSTANT)
      {
        if (yytoken != Token_CONSTANT)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_CONSTANT, "constant");
              }
            return false;
          }
        yylex();

        while (yytoken == Token_TWO_COLON)
          {
            if (yytoken != Token_TWO_COLON)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_TWO_COLON, "::");
                  }
                return false;
              }
            yylex();

            if (yytoken != Token_FUNCTION)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_FUNCTION, "function");
                  }
                return false;
              }
            yylex();

          }
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_mrhs(mrhs_ast **yynode)
  {
    *yynode = create<mrhs_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BEGIN
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_KEYWORD_NOT
        || yytoken == Token_LPAREN
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_REST_ARG_PREFIX
        || yytoken == Token_UNARY_MINUS
        || yytoken == Token_UNARY_PLUS
        || yytoken == Token_NOT
        || yytoken == Token_BNOT
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE)
      {
        if (yytoken == Token_BEGIN
            || yytoken == Token_RETURN
            || yytoken == Token_BREAK
            || yytoken == Token_NEXT
            || yytoken == Token_NIL
            || yytoken == Token_TRUE
            || yytoken == Token_FALSE
            || yytoken == Token_FILE
            || yytoken == Token_LINE
            || yytoken == Token_SELF
            || yytoken == Token_SUPER
            || yytoken == Token_RETRY
            || yytoken == Token_YIELD
            || yytoken == Token_DEFINED
            || yytoken == Token_REDO
            || yytoken == Token_IF
            || yytoken == Token_CASE
            || yytoken == Token_UNTIL
            || yytoken == Token_WHILE
            || yytoken == Token_FOR
            || yytoken == Token_MODULE
            || yytoken == Token_DEF
            || yytoken == Token_CLASS
            || yytoken == Token_UNLESS
            || yytoken == Token_KEYWORD_NOT
            || yytoken == Token_LPAREN
            || yytoken == Token_LEADING_TWO_COLON
            || yytoken == Token_LBRACK
            || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
            || yytoken == Token_LCURLY_HASH
            || yytoken == Token_UNARY_MINUS
            || yytoken == Token_UNARY_PLUS
            || yytoken == Token_NOT
            || yytoken == Token_BNOT
            || yytoken == Token_IDENTIFIER
            || yytoken == Token_CONSTANT
            || yytoken == Token_FUNCTION
            || yytoken == Token_GLOBAL_VARIABLE
            || yytoken == Token_INSTANCE_VARIABLE
            || yytoken == Token_CLASS_VARIABLE
            || yytoken == Token_DOUBLE_QUOTED_STRING
            || yytoken == Token_SINGLE_QUOTED_STRING
            || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_REGEX
            || yytoken == Token_COMMAND_OUTPUT
            || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_HERE_DOC_BEGIN
            || yytoken == Token_W_ARRAY
            || yytoken == Token_INTEGER
            || yytoken == Token_HEX
            || yytoken == Token_OCTAL
            || yytoken == Token_BINARY
            || yytoken == Token_FLOAT
            || yytoken == Token_ASCII_VALUE)
          {
            expression_ast *__node_159 = 0;
            if (!parse_expression(&__node_159))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_expression, "expression");
                  }
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

                if ((yytoken == Token_ASSIGN) || (yytoken == Token_RBRACK))
                  break;
                if (yytoken == Token_REST_ARG_PREFIX)
                  {
                    if (yytoken != Token_REST_ARG_PREFIX)
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_token(yytoken, Token_REST_ARG_PREFIX, "*");
                          }
                        return false;
                      }
                    yylex();

                    seen_star = true;
                  }
                else if (true /*epsilon*/)
                  {
                    seen_star = false;
                  }
                else
                  {
                    return false;
                  }
                expression_ast *__node_160 = 0;
                if (!parse_expression(&__node_160))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_expression, "expression");
                      }
                    return false;
                  }
                if (seen_star)
                  break;
              }
          }
        else if (yytoken == Token_REST_ARG_PREFIX)
          {
            if (yytoken != Token_REST_ARG_PREFIX)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_REST_ARG_PREFIX, "*");
                  }
                return false;
              }
            yylex();

            expression_ast *__node_161 = 0;
            if (!parse_expression(&__node_161))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_expression, "expression");
                  }
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

  bool parser::parse_multiplicativeExpression(multiplicativeExpression_ast **yynode)
  {
    *yynode = create<multiplicativeExpression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BEGIN
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_LPAREN
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_UNARY_MINUS
        || yytoken == Token_UNARY_PLUS
        || yytoken == Token_NOT
        || yytoken == Token_BNOT
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE)
      {
        powerExpression_ast *__node_162 = 0;
        if (!parse_powerExpression(&__node_162))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_powerExpression, "powerExpression");
              }
            return false;
          }
        while (yytoken == Token_STAR
               || yytoken == Token_DIV
               || yytoken == Token_MOD)
          {
            if (yytoken == Token_STAR)
              {
                operatorStar_ast *__node_163 = 0;
                if (!parse_operatorStar(&__node_163))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorStar, "operatorStar");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_DIV)
              {
                operatorDiv_ast *__node_164 = 0;
                if (!parse_operatorDiv(&__node_164))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorDiv, "operatorDiv");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_MOD)
              {
                operatorMod_ast *__node_165 = 0;
                if (!parse_operatorMod(&__node_165))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorMod, "operatorMod");
                      }
                    return false;
                  }
              }
            else
              {
                return false;
              }
            powerExpression_ast *__node_166 = 0;
            if (!parse_powerExpression(&__node_166))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_powerExpression, "powerExpression");
                  }
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

  bool parser::parse_normalMethodDefinitionArgument(normalMethodDefinitionArgument_ast **yynode)
  {
    *yynode = create<normalMethodDefinitionArgument_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_IDENTIFIER
        || yytoken == Token_FUNCTION)
      {
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
            yylex();

          }
        else if (yytoken == Token_FUNCTION)
          {
            if (yytoken != Token_FUNCTION)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_FUNCTION, "function");
                  }
                return false;
              }
            yylex();

          }
        else
          {
            return false;
          }
        if (yytoken == Token_ASSIGN
            || yytoken == Token_ASSIGN_WITH_NO_LEADING_SPACE)
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

              }
            else if (yytoken == Token_ASSIGN_WITH_NO_LEADING_SPACE)
              {
                if (yytoken != Token_ASSIGN_WITH_NO_LEADING_SPACE)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_ASSIGN_WITH_NO_LEADING_SPACE, "=");
                      }
                    return false;
                  }
                yylex();

              }
            else
              {
                return false;
              }
            expression_ast *__node_167 = 0;
            if (!parse_expression(&__node_167))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_expression, "expression");
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
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_normalMethodInvocationArgument(normalMethodInvocationArgument_ast **yynode)
  {
    *yynode = create<normalMethodInvocationArgument_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BEGIN
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_KEYWORD_NOT
        || yytoken == Token_LPAREN
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_UNARY_MINUS
        || yytoken == Token_UNARY_PLUS
        || yytoken == Token_NOT
        || yytoken == Token_BNOT
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE)
      {
        expression_ast *__node_168 = 0;
        if (!parse_expression(&__node_168))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_expression, "expression");
              }
            return false;
          }
        if (yytoken == Token_ASSOC)
          {
            if (yytoken != Token_ASSOC)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_ASSOC, "=>");
                  }
                return false;
              }
            yylex();

            expression_ast *__node_169 = 0;
            if (!parse_expression(&__node_169))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_expression, "expression");
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
        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_notExpression(notExpression_ast **yynode)
  {
    *yynode = create<notExpression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BEGIN
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_KEYWORD_NOT
        || yytoken == Token_LPAREN
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_UNARY_MINUS
        || yytoken == Token_UNARY_PLUS
        || yytoken == Token_NOT
        || yytoken == Token_BNOT
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE)
      {
        if (yytoken == Token_KEYWORD_NOT)
          {
            keywordNot_ast *__node_170 = 0;
            if (!parse_keywordNot(&__node_170))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_keywordNot, "keywordNot");
                  }
                return false;
              }
            notExpression_ast *__node_171 = 0;
            if (!parse_notExpression(&__node_171))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_notExpression, "notExpression");
                  }
                return false;
              }
          }
        else if (yytoken == Token_BEGIN
                 || yytoken == Token_RETURN
                 || yytoken == Token_BREAK
                 || yytoken == Token_NEXT
                 || yytoken == Token_NIL
                 || yytoken == Token_TRUE
                 || yytoken == Token_FALSE
                 || yytoken == Token_FILE
                 || yytoken == Token_LINE
                 || yytoken == Token_SELF
                 || yytoken == Token_SUPER
                 || yytoken == Token_RETRY
                 || yytoken == Token_YIELD
                 || yytoken == Token_DEFINED
                 || yytoken == Token_REDO
                 || yytoken == Token_IF
                 || yytoken == Token_CASE
                 || yytoken == Token_UNTIL
                 || yytoken == Token_WHILE
                 || yytoken == Token_FOR
                 || yytoken == Token_MODULE
                 || yytoken == Token_DEF
                 || yytoken == Token_CLASS
                 || yytoken == Token_UNLESS
                 || yytoken == Token_LPAREN
                 || yytoken == Token_LEADING_TWO_COLON
                 || yytoken == Token_LBRACK
                 || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
                 || yytoken == Token_LCURLY_HASH
                 || yytoken == Token_UNARY_MINUS
                 || yytoken == Token_UNARY_PLUS
                 || yytoken == Token_NOT
                 || yytoken == Token_BNOT
                 || yytoken == Token_IDENTIFIER
                 || yytoken == Token_CONSTANT
                 || yytoken == Token_FUNCTION
                 || yytoken == Token_GLOBAL_VARIABLE
                 || yytoken == Token_INSTANCE_VARIABLE
                 || yytoken == Token_CLASS_VARIABLE
                 || yytoken == Token_DOUBLE_QUOTED_STRING
                 || yytoken == Token_SINGLE_QUOTED_STRING
                 || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
                 || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
                 || yytoken == Token_REGEX
                 || yytoken == Token_COMMAND_OUTPUT
                 || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
                 || yytoken == Token_HERE_DOC_BEGIN
                 || yytoken == Token_W_ARRAY
                 || yytoken == Token_INTEGER
                 || yytoken == Token_HEX
                 || yytoken == Token_OCTAL
                 || yytoken == Token_BINARY
                 || yytoken == Token_FLOAT
                 || yytoken == Token_ASCII_VALUE)
          {
            ternaryIfThenElseExpression_ast *__node_172 = 0;
            if (!parse_ternaryIfThenElseExpression(&__node_172))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_ternaryIfThenElseExpression, "ternaryIfThenElseExpression");
                  }
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

  bool parser::parse_numeric(numeric_ast **yynode)
  {
    *yynode = create<numeric_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE)
      {
        if (yytoken == Token_INTEGER)
          {
            if (yytoken != Token_INTEGER)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_INTEGER, "integer number");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_HEX)
          {
            if (yytoken != Token_HEX)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_HEX, "hexadecimal number");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_BINARY)
          {
            if (yytoken != Token_BINARY)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_BINARY, "binary number");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_OCTAL)
          {
            if (yytoken != Token_OCTAL)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_OCTAL, "octal number");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_FLOAT)
          {
            if (yytoken != Token_FLOAT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_FLOAT, "float number");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_ASCII_VALUE)
          {
            if (yytoken != Token_ASCII_VALUE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_ASCII_VALUE, "ascii value");
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

  bool parser::parse_operatorAsMethodname(operatorAsMethodname_ast **yynode)
  {
    *yynode = create<operatorAsMethodname_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_SINGLE_QUOTE
        || yytoken == Token_LBRACK
        || yytoken == Token_LBRACK_ARRAY_ACCESS
        || yytoken == Token_BOR
        || yytoken == Token_COMPARE
        || yytoken == Token_EQUAL
        || yytoken == Token_CASE_EQUAL
        || yytoken == Token_MATCH
        || yytoken == Token_LESS_THAN
        || yytoken == Token_GREATER_THAN
        || yytoken == Token_LESS_OR_EQUAL
        || yytoken == Token_GREATER_OR_EQUAL
        || yytoken == Token_BXOR
        || yytoken == Token_BAND
        || yytoken == Token_LEFT_SHIFT
        || yytoken == Token_RIGHT_SHIFT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_STAR
        || yytoken == Token_DIV
        || yytoken == Token_MOD
        || yytoken == Token_BNOT
        || yytoken == Token_POWER)
      {
        if (yytoken == Token_LEFT_SHIFT)
          {
            if (yytoken != Token_LEFT_SHIFT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LEFT_SHIFT, "<<");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_RIGHT_SHIFT)
          {
            if (yytoken != Token_RIGHT_SHIFT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_RIGHT_SHIFT, ">>");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_EQUAL)
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

          }
        else if (yytoken == Token_CASE_EQUAL)
          {
            if (yytoken != Token_CASE_EQUAL)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_CASE_EQUAL, "===");
                  }
                return false;
              }
            yylex();

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

          }
        else if (yytoken == Token_GREATER_OR_EQUAL)
          {
            if (yytoken != Token_GREATER_OR_EQUAL)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_GREATER_OR_EQUAL, ">=");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_LESS_THAN)
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

          }
        else if (yytoken == Token_LESS_OR_EQUAL)
          {
            if (yytoken != Token_LESS_OR_EQUAL)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LESS_OR_EQUAL, "<=");
                  }
                return false;
              }
            yylex();

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

          }
        else if (yytoken == Token_DIV)
          {
            if (yytoken != Token_DIV)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_DIV, "/");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_MOD)
          {
            if (yytoken != Token_MOD)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_MOD, "%");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_POWER)
          {
            if (yytoken != Token_POWER)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_POWER, "**");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_BAND)
          {
            if (yytoken != Token_BAND)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_BAND, "&");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_BOR)
          {
            if (yytoken != Token_BOR)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_BOR, "|");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_BXOR)
          {
            if (yytoken != Token_BXOR)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_BXOR, "^");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_LBRACK
                 || yytoken == Token_LBRACK_ARRAY_ACCESS)
          {
            if (yytoken == Token_LBRACK)
              {
                if (yytoken != Token_LBRACK)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_LBRACK, "[");
                      }
                    return false;
                  }
                yylex();

              }
            else if (yytoken == Token_LBRACK_ARRAY_ACCESS)
              {
                if (yytoken != Token_LBRACK_ARRAY_ACCESS)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_LBRACK_ARRAY_ACCESS, "[");
                      }
                    return false;
                  }
                yylex();

              }
            else
              {
                return false;
              }
            if (yytoken != Token_RBRACK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_RBRACK, "]");
                  }
                return false;
              }
            yylex();

            if (yytoken == Token_ASSIGN_WITH_NO_LEADING_SPACE)
              {
                if (yytoken != Token_ASSIGN_WITH_NO_LEADING_SPACE)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_ASSIGN_WITH_NO_LEADING_SPACE, "=");
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
        else if (yytoken == Token_MATCH)
          {
            if (yytoken != Token_MATCH)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_MATCH, "=~");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_COMPARE)
          {
            if (yytoken != Token_COMPARE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_COMPARE, "<=>");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_BNOT)
          {
            if (yytoken != Token_BNOT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_BNOT, "~");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_SINGLE_QUOTE)
          {
            if (yytoken != Token_SINGLE_QUOTE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_SINGLE_QUOTE, "'");
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

  bool parser::parse_operatorAssign(operatorAssign_ast **yynode)
  {
    *yynode = create<operatorAssign_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_ASSIGN
        || yytoken == Token_ASSIGN_WITH_NO_LEADING_SPACE)
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

          }
        else if (yytoken == Token_ASSIGN_WITH_NO_LEADING_SPACE)
          {
            if (yytoken != Token_ASSIGN_WITH_NO_LEADING_SPACE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_ASSIGN_WITH_NO_LEADING_SPACE, "=");
                  }
                return false;
              }
            yylex();

          }
        else
          {
            return false;
          }
        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorBAnd(operatorBAnd_ast **yynode)
  {
    *yynode = create<operatorBAnd_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BAND)
      {
        if (yytoken != Token_BAND)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_BAND, "&");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorBAndAssign(operatorBAndAssign_ast **yynode)
  {
    *yynode = create<operatorBAndAssign_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BAND_ASSIGN)
      {
        if (yytoken != Token_BAND_ASSIGN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_BAND_ASSIGN, "&=");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorBNot(operatorBNot_ast **yynode)
  {
    *yynode = create<operatorBNot_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BNOT)
      {
        if (yytoken != Token_BNOT)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_BNOT, "~");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorBOr(operatorBOr_ast **yynode)
  {
    *yynode = create<operatorBOr_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOR)
      {
        if (yytoken != Token_BOR)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_BOR, "|");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorBXor(operatorBXor_ast **yynode)
  {
    *yynode = create<operatorBXor_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BXOR)
      {
        if (yytoken != Token_BXOR)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_BXOR, "^");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorBXorAssign(operatorBXorAssign_ast **yynode)
  {
    *yynode = create<operatorBXorAssign_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BXOR_ASSIGN)
      {
        if (yytoken != Token_BXOR_ASSIGN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_BXOR_ASSIGN, "^=");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorBorAssign(operatorBorAssign_ast **yynode)
  {
    *yynode = create<operatorBorAssign_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOR_ASSIGN)
      {
        if (yytoken != Token_BOR_ASSIGN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_BOR_ASSIGN, "|=");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorCaseEqual(operatorCaseEqual_ast **yynode)
  {
    *yynode = create<operatorCaseEqual_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_CASE_EQUAL)
      {
        if (yytoken != Token_CASE_EQUAL)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_CASE_EQUAL, "===");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorColon(operatorColon_ast **yynode)
  {
    *yynode = create<operatorColon_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_COLON
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE)
      {
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

          }
        else if (yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE)
          {
            if (yytoken != Token_COLON_WITH_NO_FOLLOWING_SPACE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_COLON_WITH_NO_FOLLOWING_SPACE, ":");
                  }
                return false;
              }
            yylex();

          }
        else
          {
            return false;
          }
        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorCompare(operatorCompare_ast **yynode)
  {
    *yynode = create<operatorCompare_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_COMPARE)
      {
        if (yytoken != Token_COMPARE)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_COMPARE, "<=>");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorDiv(operatorDiv_ast **yynode)
  {
    *yynode = create<operatorDiv_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_DIV)
      {
        if (yytoken != Token_DIV)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_DIV, "/");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorDivAssign(operatorDivAssign_ast **yynode)
  {
    *yynode = create<operatorDivAssign_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_DIV_ASSIGN)
      {
        if (yytoken != Token_DIV_ASSIGN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_DIV_ASSIGN, "/=");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorEqual(operatorEqual_ast **yynode)
  {
    *yynode = create<operatorEqual_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

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

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorExclusiveRange(operatorExclusiveRange_ast **yynode)
  {
    *yynode = create<operatorExclusiveRange_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_EXCLUSIVE_RANGE)
      {
        if (yytoken != Token_EXCLUSIVE_RANGE)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_EXCLUSIVE_RANGE, "...");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorGreaterOrEqual(operatorGreaterOrEqual_ast **yynode)
  {
    *yynode = create<operatorGreaterOrEqual_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_GREATER_OR_EQUAL)
      {
        if (yytoken != Token_GREATER_OR_EQUAL)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_GREATER_OR_EQUAL, ">=");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorGreaterThan(operatorGreaterThan_ast **yynode)
  {
    *yynode = create<operatorGreaterThan_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

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

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorInclusiveRange(operatorInclusiveRange_ast **yynode)
  {
    *yynode = create<operatorInclusiveRange_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_INCLUSIVE_RANGE)
      {
        if (yytoken != Token_INCLUSIVE_RANGE)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_INCLUSIVE_RANGE, "..");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorLeftShift(operatorLeftShift_ast **yynode)
  {
    *yynode = create<operatorLeftShift_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_LEFT_SHIFT)
      {
        if (yytoken != Token_LEFT_SHIFT)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_LEFT_SHIFT, "<<");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorLeftShiftAssign(operatorLeftShiftAssign_ast **yynode)
  {
    *yynode = create<operatorLeftShiftAssign_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_LEFT_SHIFT_ASSIGN)
      {
        if (yytoken != Token_LEFT_SHIFT_ASSIGN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_LEFT_SHIFT_ASSIGN, "<<=");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorLessOrEqual(operatorLessOrEqual_ast **yynode)
  {
    *yynode = create<operatorLessOrEqual_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_LESS_OR_EQUAL)
      {
        if (yytoken != Token_LESS_OR_EQUAL)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_LESS_OR_EQUAL, "<=");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorLessThan(operatorLessThan_ast **yynode)
  {
    *yynode = create<operatorLessThan_ast>();

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

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorLogicalAnd(operatorLogicalAnd_ast **yynode)
  {
    *yynode = create<operatorLogicalAnd_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_LOGICAL_AND)
      {
        if (yytoken != Token_LOGICAL_AND)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_LOGICAL_AND, "&");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorLogicalAndAssign(operatorLogicalAndAssign_ast **yynode)
  {
    *yynode = create<operatorLogicalAndAssign_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_LOGICAL_AND_ASSIGN)
      {
        if (yytoken != Token_LOGICAL_AND_ASSIGN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_LOGICAL_AND_ASSIGN, "&&=");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorLogicalOr(operatorLogicalOr_ast **yynode)
  {
    *yynode = create<operatorLogicalOr_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_LOGICAL_OR)
      {
        if (yytoken != Token_LOGICAL_OR)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_LOGICAL_OR, "||");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorLogicalOrAssign(operatorLogicalOrAssign_ast **yynode)
  {
    *yynode = create<operatorLogicalOrAssign_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_LOGICAL_OR_ASSIGN)
      {
        if (yytoken != Token_LOGICAL_OR_ASSIGN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_LOGICAL_OR_ASSIGN, "||=");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorMatch(operatorMatch_ast **yynode)
  {
    *yynode = create<operatorMatch_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_MATCH)
      {
        if (yytoken != Token_MATCH)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_MATCH, "=~");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorMinus(operatorMinus_ast **yynode)
  {
    *yynode = create<operatorMinus_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_MINUS)
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

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorMinusAssign(operatorMinusAssign_ast **yynode)
  {
    *yynode = create<operatorMinusAssign_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_MINUS_ASSIGN)
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

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorMod(operatorMod_ast **yynode)
  {
    *yynode = create<operatorMod_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_MOD)
      {
        if (yytoken != Token_MOD)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_MOD, "%");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorModAssign(operatorModAssign_ast **yynode)
  {
    *yynode = create<operatorModAssign_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_MOD_ASSIGN)
      {
        if (yytoken != Token_MOD_ASSIGN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_MOD_ASSIGN, "%=");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorNot(operatorNot_ast **yynode)
  {
    *yynode = create<operatorNot_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_NOT)
      {
        if (yytoken != Token_NOT)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_NOT, "!");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorNotEqual(operatorNotEqual_ast **yynode)
  {
    *yynode = create<operatorNotEqual_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_NOT_EQUAL)
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

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorNotMatch(operatorNotMatch_ast **yynode)
  {
    *yynode = create<operatorNotMatch_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_NOT_MATCH)
      {
        if (yytoken != Token_NOT_MATCH)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_NOT_MATCH, "!~");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorPlus(operatorPlus_ast **yynode)
  {
    *yynode = create<operatorPlus_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

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

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorPlusAssign(operatorPlusAssign_ast **yynode)
  {
    *yynode = create<operatorPlusAssign_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_PLUS_ASSIGN)
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

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorPower(operatorPower_ast **yynode)
  {
    *yynode = create<operatorPower_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_POWER)
      {
        if (yytoken != Token_POWER)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_POWER, "**");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorPowerAssign(operatorPowerAssign_ast **yynode)
  {
    *yynode = create<operatorPowerAssign_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_POWER_ASSIGN)
      {
        if (yytoken != Token_POWER_ASSIGN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_POWER_ASSIGN, "**=");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorQuestion(operatorQuestion_ast **yynode)
  {
    *yynode = create<operatorQuestion_ast>();

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

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorRightShift(operatorRightShift_ast **yynode)
  {
    *yynode = create<operatorRightShift_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_RIGHT_SHIFT)
      {
        if (yytoken != Token_RIGHT_SHIFT)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_RIGHT_SHIFT, ">>");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorRightShiftAssign(operatorRightShiftAssign_ast **yynode)
  {
    *yynode = create<operatorRightShiftAssign_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_RIGHT_SHIFT_ASSIGN)
      {
        if (yytoken != Token_RIGHT_SHIFT_ASSIGN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_RIGHT_SHIFT_ASSIGN, ">>=");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorStar(operatorStar_ast **yynode)
  {
    *yynode = create<operatorStar_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

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

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorStarAssign(operatorStarAssign_ast **yynode)
  {
    *yynode = create<operatorStarAssign_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_STAR_ASSIGN)
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

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorUnaryMinus(operatorUnaryMinus_ast **yynode)
  {
    *yynode = create<operatorUnaryMinus_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_UNARY_MINUS)
      {
        if (yytoken != Token_UNARY_MINUS)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_UNARY_MINUS, "-");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_operatorUnaryPlus(operatorUnaryPlus_ast **yynode)
  {
    *yynode = create<operatorUnaryPlus_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_UNARY_PLUS)
      {
        if (yytoken != Token_UNARY_PLUS)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_UNARY_PLUS, "+");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_orExpression(orExpression_ast **yynode)
  {
    *yynode = create<orExpression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BEGIN
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_LPAREN
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_UNARY_MINUS
        || yytoken == Token_UNARY_PLUS
        || yytoken == Token_NOT
        || yytoken == Token_BNOT
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE)
      {
        andExpression_ast *__node_173 = 0;
        if (!parse_andExpression(&__node_173))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_andExpression, "andExpression");
              }
            return false;
          }
        while (yytoken == Token_BOR
               || yytoken == Token_BXOR)
          {
            if (yytoken == Token_BXOR)
              {
                operatorBXor_ast *__node_174 = 0;
                if (!parse_operatorBXor(&__node_174))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorBXor, "operatorBXor");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_BOR)
              {
                operatorBOr_ast *__node_175 = 0;
                if (!parse_operatorBOr(&__node_175))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorBOr, "operatorBOr");
                      }
                    return false;
                  }
              }
            else
              {
                return false;
              }
            andExpression_ast *__node_176 = 0;
            if (!parse_andExpression(&__node_176))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_andExpression, "andExpression");
                  }
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

  bool parser::parse_parallelAssignmentLeftOver(parallelAssignmentLeftOver_ast **yynode)
  {
    *yynode = create<parallelAssignmentLeftOver_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_COMMA)
      {
        do
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

            if ((yytoken == Token_ASSIGN) || (yytoken == Token_ASSIGN_WITH_NO_LEADING_SPACE))
              break;
            if (yytoken == Token_REST_ARG_PREFIX)
              {
                if (yytoken != Token_REST_ARG_PREFIX)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_REST_ARG_PREFIX, "*");
                      }
                    return false;
                  }
                yylex();

                seen_star = true;
              }
            else if (true /*epsilon*/)
              {
                seen_star = false;
              }
            else
              {
                return false;
              }
            if (yytoken == Token_BEGIN
                || yytoken == Token_RETURN
                || yytoken == Token_BREAK
                || yytoken == Token_NEXT
                || yytoken == Token_NIL
                || yytoken == Token_TRUE
                || yytoken == Token_FALSE
                || yytoken == Token_FILE
                || yytoken == Token_LINE
                || yytoken == Token_SELF
                || yytoken == Token_SUPER
                || yytoken == Token_RETRY
                || yytoken == Token_YIELD
                || yytoken == Token_DEFINED
                || yytoken == Token_REDO
                || yytoken == Token_IF
                || yytoken == Token_CASE
                || yytoken == Token_UNTIL
                || yytoken == Token_WHILE
                || yytoken == Token_FOR
                || yytoken == Token_MODULE
                || yytoken == Token_DEF
                || yytoken == Token_CLASS
                || yytoken == Token_UNLESS
                || yytoken == Token_LPAREN
                || yytoken == Token_LEADING_TWO_COLON
                || yytoken == Token_LBRACK
                || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
                || yytoken == Token_LCURLY_HASH
                || yytoken == Token_IDENTIFIER
                || yytoken == Token_CONSTANT
                || yytoken == Token_FUNCTION
                || yytoken == Token_GLOBAL_VARIABLE
                || yytoken == Token_INSTANCE_VARIABLE
                || yytoken == Token_CLASS_VARIABLE
                || yytoken == Token_DOUBLE_QUOTED_STRING
                || yytoken == Token_SINGLE_QUOTED_STRING
                || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_REGEX
                || yytoken == Token_COMMAND_OUTPUT
                || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_HERE_DOC_BEGIN
                || yytoken == Token_W_ARRAY
                || yytoken == Token_INTEGER
                || yytoken == Token_HEX
                || yytoken == Token_OCTAL
                || yytoken == Token_BINARY
                || yytoken == Token_FLOAT
                || yytoken == Token_ASCII_VALUE)
              {
                mlhs_item_ast *__node_177 = 0;
                if (!parse_mlhs_item(&__node_177))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_mlhs_item, "mlhs_item");
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
            if (seen_star)
              break;
          }
        while (yytoken == Token_COMMA);
        if (yytoken == Token_ASSIGN
            || yytoken == Token_ASSIGN_WITH_NO_LEADING_SPACE)
          {
            operatorAssign_ast *__node_178 = 0;
            if (!parse_operatorAssign(&__node_178))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_operatorAssign, "operatorAssign");
                  }
                return false;
              }
            mrhs_ast *__node_179 = 0;
            if (!parse_mrhs(&__node_179))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_mrhs, "mrhs");
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
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_powerExpression(powerExpression_ast **yynode)
  {
    *yynode = create<powerExpression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BEGIN
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_LPAREN
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_UNARY_MINUS
        || yytoken == Token_UNARY_PLUS
        || yytoken == Token_NOT
        || yytoken == Token_BNOT
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE)
      {
        unaryExpression_ast *__node_180 = 0;
        if (!parse_unaryExpression(&__node_180))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_unaryExpression, "unaryExpression");
              }
            return false;
          }
        while (yytoken == Token_POWER)
          {
            operatorPower_ast *__node_181 = 0;
            if (!parse_operatorPower(&__node_181))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_operatorPower, "operatorPower");
                  }
                return false;
              }
            unaryExpression_ast *__node_182 = 0;
            if (!parse_unaryExpression(&__node_182))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_unaryExpression, "unaryExpression");
                  }
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

  bool parser::parse_predefinedValue(predefinedValue_ast **yynode)
  {
    *yynode = create<predefinedValue_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE)
      {
        if (yytoken == Token_NIL)
          {
            if (yytoken != Token_NIL)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_NIL, "nil");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_TRUE)
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

          }
        else if (yytoken == Token_FILE)
          {
            if (yytoken != Token_FILE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_FILE, "__FILE__");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_LINE)
          {
            if (yytoken != Token_LINE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE, "__LINE__");
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

  bool parser::parse_primaryExpression(primaryExpression_ast **yynode)
  {
    *yynode = create<primaryExpression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BEGIN
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_LPAREN
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE)
      {
        if (yytoken == Token_LEADING_TWO_COLON)
          {
            if (yytoken != Token_LEADING_TWO_COLON)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LEADING_TWO_COLON, "::");
                  }
                return false;
              }
            yylex();

            if (yytoken != Token_FUNCTION)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_FUNCTION, "function");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_NIL
                 || yytoken == Token_TRUE
                 || yytoken == Token_FALSE
                 || yytoken == Token_FILE
                 || yytoken == Token_LINE
                 || yytoken == Token_SELF
                 || yytoken == Token_SUPER
                 || yytoken == Token_IDENTIFIER
                 || yytoken == Token_CONSTANT
                 || yytoken == Token_FUNCTION
                 || yytoken == Token_GLOBAL_VARIABLE
                 || yytoken == Token_INSTANCE_VARIABLE
                 || yytoken == Token_CLASS_VARIABLE)
          {
            variable_ast *__node_183 = 0;
            if (!parse_variable(&__node_183))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_variable, "variable");
                  }
                return false;
              }
          }
        else if (yytoken == Token_INTEGER
                 || yytoken == Token_HEX
                 || yytoken == Token_OCTAL
                 || yytoken == Token_BINARY
                 || yytoken == Token_FLOAT
                 || yytoken == Token_ASCII_VALUE)
          {
            numeric_ast *__node_184 = 0;
            if (!parse_numeric(&__node_184))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_numeric, "numeric");
                  }
                return false;
              }
          }
        else if (yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
                 || yytoken == Token_DOUBLE_QUOTED_STRING
                 || yytoken == Token_SINGLE_QUOTED_STRING
                 || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
                 || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
                 || yytoken == Token_REGEX
                 || yytoken == Token_COMMAND_OUTPUT
                 || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
                 || yytoken == Token_HERE_DOC_BEGIN
                 || yytoken == Token_W_ARRAY)
          {
            literal_ast *__node_185 = 0;
            if (!parse_literal(&__node_185))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_literal, "literal");
                  }
                return false;
              }
          }
        else if (yytoken == Token_LBRACK)
          {
            arrayExpression_ast *__node_186 = 0;
            if (!parse_arrayExpression(&__node_186))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_arrayExpression, "arrayExpression");
                  }
                return false;
              }
          }
        else if (yytoken == Token_LCURLY_HASH)
          {
            hashExpression_ast *__node_187 = 0;
            if (!parse_hashExpression(&__node_187))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_hashExpression, "hashExpression");
                  }
                return false;
              }
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

            compoundStatement_ast *__node_188 = 0;
            if (!parse_compoundStatement(&__node_188))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_compoundStatement, "compoundStatement");
                  }
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

          }
        else if (yytoken == Token_IF)
          {
            ifExpression_ast *__node_189 = 0;
            if (!parse_ifExpression(&__node_189))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_ifExpression, "ifExpression");
                  }
                return false;
              }
          }
        else if (yytoken == Token_UNLESS)
          {
            unlessExpression_ast *__node_190 = 0;
            if (!parse_unlessExpression(&__node_190))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_unlessExpression, "unlessExpression");
                  }
                return false;
              }
          }
        else if (yytoken == Token_WHILE)
          {
            whileExpression_ast *__node_191 = 0;
            if (!parse_whileExpression(&__node_191))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_whileExpression, "whileExpression");
                  }
                return false;
              }
          }
        else if (yytoken == Token_UNTIL)
          {
            untilExpression_ast *__node_192 = 0;
            if (!parse_untilExpression(&__node_192))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_untilExpression, "untilExpression");
                  }
                return false;
              }
          }
        else if (yytoken == Token_CASE)
          {
            caseExpression_ast *__node_193 = 0;
            if (!parse_caseExpression(&__node_193))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_caseExpression, "caseExpression");
                  }
                return false;
              }
          }
        else if (yytoken == Token_FOR)
          {
            forExpression_ast *__node_194 = 0;
            if (!parse_forExpression(&__node_194))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_forExpression, "forExpression");
                  }
                return false;
              }
          }
        else if (yytoken == Token_BEGIN)
          {
            exceptionHandlingExpression_ast *__node_195 = 0;
            if (!parse_exceptionHandlingExpression(&__node_195))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_exceptionHandlingExpression, "exceptionHandlingExpression");
                  }
                return false;
              }
          }
        else if (yytoken == Token_MODULE)
          {
            moduleDefinition_ast *__node_196 = 0;
            if (!parse_moduleDefinition(&__node_196))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_moduleDefinition, "moduleDefinition");
                  }
                return false;
              }
          }
        else if (yytoken == Token_CLASS)
          {
            classDefinition_ast *__node_197 = 0;
            if (!parse_classDefinition(&__node_197))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_classDefinition, "classDefinition");
                  }
                return false;
              }
          }
        else if (yytoken == Token_DEF)
          {
            methodDefinition_ast *__node_198 = 0;
            if (!parse_methodDefinition(&__node_198))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_methodDefinition, "methodDefinition");
                  }
                return false;
              }
          }
        else if (yytoken == Token_RETRY)
          {
            if (yytoken != Token_RETRY)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_RETRY, "retry");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_YIELD)
          {
            if (yytoken != Token_YIELD)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_YIELD, "yield");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_DEFINED)
          {
            keywordDefined_ast *__node_199 = 0;
            if (!parse_keywordDefined(&__node_199))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_keywordDefined, "keywordDefined");
                  }
                return false;
              }
          }
        else if (yytoken == Token_REDO)
          {
            if (yytoken != Token_REDO)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_REDO, "redo");
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

  bool parser::parse_program(program_ast **yynode)
  {
    *yynode = create<program_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_UNDEF
        || yytoken == Token_ALIAS
        || yytoken == Token_BEGIN
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_KEYWORD_NOT
        || yytoken == Token_END_UPCASE
        || yytoken == Token_BEGIN_UPCASE
        || yytoken == Token_LPAREN
        || yytoken == Token_SEMI
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_REST_ARG_PREFIX
        || yytoken == Token_UNARY_MINUS
        || yytoken == Token_UNARY_PLUS
        || yytoken == Token_NOT
        || yytoken == Token_BNOT
        || yytoken == Token_LINE_BREAK
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE || yytoken == Token_EOF)
      {
        if (yytoken == Token_UNDEF
            || yytoken == Token_ALIAS
            || yytoken == Token_BEGIN
            || yytoken == Token_RETURN
            || yytoken == Token_BREAK
            || yytoken == Token_NEXT
            || yytoken == Token_NIL
            || yytoken == Token_TRUE
            || yytoken == Token_FALSE
            || yytoken == Token_FILE
            || yytoken == Token_LINE
            || yytoken == Token_SELF
            || yytoken == Token_SUPER
            || yytoken == Token_RETRY
            || yytoken == Token_YIELD
            || yytoken == Token_DEFINED
            || yytoken == Token_REDO
            || yytoken == Token_IF
            || yytoken == Token_CASE
            || yytoken == Token_UNTIL
            || yytoken == Token_WHILE
            || yytoken == Token_FOR
            || yytoken == Token_MODULE
            || yytoken == Token_DEF
            || yytoken == Token_CLASS
            || yytoken == Token_UNLESS
            || yytoken == Token_KEYWORD_NOT
            || yytoken == Token_END_UPCASE
            || yytoken == Token_BEGIN_UPCASE
            || yytoken == Token_LPAREN
            || yytoken == Token_SEMI
            || yytoken == Token_LEADING_TWO_COLON
            || yytoken == Token_LBRACK
            || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
            || yytoken == Token_LCURLY_HASH
            || yytoken == Token_REST_ARG_PREFIX
            || yytoken == Token_UNARY_MINUS
            || yytoken == Token_UNARY_PLUS
            || yytoken == Token_NOT
            || yytoken == Token_BNOT
            || yytoken == Token_LINE_BREAK
            || yytoken == Token_IDENTIFIER
            || yytoken == Token_CONSTANT
            || yytoken == Token_FUNCTION
            || yytoken == Token_GLOBAL_VARIABLE
            || yytoken == Token_INSTANCE_VARIABLE
            || yytoken == Token_CLASS_VARIABLE
            || yytoken == Token_DOUBLE_QUOTED_STRING
            || yytoken == Token_SINGLE_QUOTED_STRING
            || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_REGEX
            || yytoken == Token_COMMAND_OUTPUT
            || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_HERE_DOC_BEGIN
            || yytoken == Token_W_ARRAY
            || yytoken == Token_INTEGER
            || yytoken == Token_HEX
            || yytoken == Token_OCTAL
            || yytoken == Token_BINARY
            || yytoken == Token_FLOAT
            || yytoken == Token_ASCII_VALUE)
          {
            compoundStatement_ast *__node_200 = 0;
            if (!parse_compoundStatement(&__node_200))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_compoundStatement, "compoundStatement");
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

  bool parser::parse_rangeExpression(rangeExpression_ast **yynode)
  {
    *yynode = create<rangeExpression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BEGIN
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_LPAREN
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_UNARY_MINUS
        || yytoken == Token_UNARY_PLUS
        || yytoken == Token_NOT
        || yytoken == Token_BNOT
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE)
      {
        logicalOrExpression_ast *__node_201 = 0;
        if (!parse_logicalOrExpression(&__node_201))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_logicalOrExpression, "logicalOrExpression");
              }
            return false;
          }
        while (yytoken == Token_INCLUSIVE_RANGE
               || yytoken == Token_EXCLUSIVE_RANGE)
          {
            if (yytoken == Token_INCLUSIVE_RANGE)
              {
                operatorInclusiveRange_ast *__node_202 = 0;
                if (!parse_operatorInclusiveRange(&__node_202))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorInclusiveRange, "operatorInclusiveRange");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_EXCLUSIVE_RANGE)
              {
                operatorExclusiveRange_ast *__node_203 = 0;
                if (!parse_operatorExclusiveRange(&__node_203))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorExclusiveRange, "operatorExclusiveRange");
                      }
                    return false;
                  }
              }
            else
              {
                return false;
              }
            logicalOrExpression_ast *__node_204 = 0;
            if (!parse_logicalOrExpression(&__node_204))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_logicalOrExpression, "logicalOrExpression");
                  }
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

  bool parser::parse_regex(regex_ast **yynode)
  {
    *yynode = create<regex_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX)
      {
        if (yytoken == Token_REGEX)
          {
            if (yytoken != Token_REGEX)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_REGEX, "regular expression");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION)
          {
            if (yytoken != Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION, "regular expression before expression substitution");
                  }
                return false;
              }
            yylex();

            expressionSubstitution_ast *__node_205 = 0;
            if (!parse_expressionSubstitution(&__node_205))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_expressionSubstitution, "expressionSubstitution");
                  }
                return false;
              }
            while (yytoken == Token_STRING_BETWEEN_EXPRESSION_SUBSTITUTION)
              {
                if (yytoken != Token_STRING_BETWEEN_EXPRESSION_SUBSTITUTION)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_STRING_BETWEEN_EXPRESSION_SUBSTITUTION, "string between expression substitution");
                      }
                    return false;
                  }
                yylex();

                expressionSubstitution_ast *__node_206 = 0;
                if (!parse_expressionSubstitution(&__node_206))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_expressionSubstitution, "expressionSubstitution");
                      }
                    return false;
                  }
              }
            if (yytoken != Token_STRING_AFTER_EXPRESSION_SUBSTITUTION)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_STRING_AFTER_EXPRESSION_SUBSTITUTION, "string after expression substitution");
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

  bool parser::parse_relationalExpression(relationalExpression_ast **yynode)
  {
    *yynode = create<relationalExpression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BEGIN
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_LPAREN
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_UNARY_MINUS
        || yytoken == Token_UNARY_PLUS
        || yytoken == Token_NOT
        || yytoken == Token_BNOT
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE)
      {
        orExpression_ast *__node_207 = 0;
        if (!parse_orExpression(&__node_207))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_orExpression, "orExpression");
              }
            return false;
          }
        while (yytoken == Token_LESS_THAN
               || yytoken == Token_GREATER_THAN
               || yytoken == Token_LESS_OR_EQUAL
               || yytoken == Token_GREATER_OR_EQUAL)
          {
            if (yytoken == Token_LESS_THAN)
              {
                operatorLessThan_ast *__node_208 = 0;
                if (!parse_operatorLessThan(&__node_208))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorLessThan, "operatorLessThan");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_GREATER_THAN)
              {
                operatorGreaterThan_ast *__node_209 = 0;
                if (!parse_operatorGreaterThan(&__node_209))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorGreaterThan, "operatorGreaterThan");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_LESS_OR_EQUAL)
              {
                operatorLessOrEqual_ast *__node_210 = 0;
                if (!parse_operatorLessOrEqual(&__node_210))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorLessOrEqual, "operatorLessOrEqual");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_GREATER_OR_EQUAL)
              {
                operatorGreaterOrEqual_ast *__node_211 = 0;
                if (!parse_operatorGreaterOrEqual(&__node_211))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorGreaterOrEqual, "operatorGreaterOrEqual");
                      }
                    return false;
                  }
              }
            else
              {
                return false;
              }
            orExpression_ast *__node_212 = 0;
            if (!parse_orExpression(&__node_212))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_orExpression, "orExpression");
                  }
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

  bool parser::parse_restMethodDefinitionArgument(restMethodDefinitionArgument_ast **yynode)
  {
    *yynode = create<restMethodDefinitionArgument_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if ((yytoken == Token_STAR) && ( expect_array_or_block_arguments ))
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

        if (yytoken == Token_IDENTIFIER
            || yytoken == Token_FUNCTION)
          {
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
                yylex();

              }
            else if (yytoken == Token_FUNCTION)
              {
                if (yytoken != Token_FUNCTION)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_FUNCTION, "function");
                      }
                    return false;
                  }
                yylex();

              }
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

                blockMethodDefinitionArgument_ast *__node_213 = 0;
                if (!parse_blockMethodDefinitionArgument(&__node_213))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_blockMethodDefinitionArgument, "blockMethodDefinitionArgument");
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
          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        expect_array_or_block_arguments = false;
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_restMethodInvocationArgument(restMethodInvocationArgument_ast **yynode)
  {
    *yynode = create<restMethodInvocationArgument_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_REST_ARG_PREFIX)
      {
        if (yytoken != Token_REST_ARG_PREFIX)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_REST_ARG_PREFIX, "*");
              }
            return false;
          }
        yylex();

        expression_ast *__node_214 = 0;
        if (!parse_expression(&__node_214))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_expression, "expression");
              }
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

            blockMethodInvocationArgument_ast *__node_215 = 0;
            if (!parse_blockMethodInvocationArgument(&__node_215))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_blockMethodInvocationArgument, "blockMethodInvocationArgument");
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
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_shiftExpression(shiftExpression_ast **yynode)
  {
    *yynode = create<shiftExpression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BEGIN
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_LPAREN
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_UNARY_MINUS
        || yytoken == Token_UNARY_PLUS
        || yytoken == Token_NOT
        || yytoken == Token_BNOT
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE)
      {
        additiveExpression_ast *__node_216 = 0;
        if (!parse_additiveExpression(&__node_216))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_additiveExpression, "additiveExpression");
              }
            return false;
          }
        while (yytoken == Token_LEFT_SHIFT
               || yytoken == Token_RIGHT_SHIFT)
          {
            if (yytoken == Token_LEFT_SHIFT)
              {
                operatorLeftShift_ast *__node_217 = 0;
                if (!parse_operatorLeftShift(&__node_217))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorLeftShift, "operatorLeftShift");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_RIGHT_SHIFT)
              {
                operatorRightShift_ast *__node_218 = 0;
                if (!parse_operatorRightShift(&__node_218))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorRightShift, "operatorRightShift");
                      }
                    return false;
                  }
              }
            else
              {
                return false;
              }
            additiveExpression_ast *__node_219 = 0;
            if (!parse_additiveExpression(&__node_219))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_additiveExpression, "additiveExpression");
                  }
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

  bool parser::parse_statement(statement_ast **yynode)
  {
    *yynode = create<statement_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_UNDEF
        || yytoken == Token_ALIAS
        || yytoken == Token_BEGIN
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_KEYWORD_NOT
        || yytoken == Token_END_UPCASE
        || yytoken == Token_BEGIN_UPCASE
        || yytoken == Token_LPAREN
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_REST_ARG_PREFIX
        || yytoken == Token_UNARY_MINUS
        || yytoken == Token_UNARY_PLUS
        || yytoken == Token_NOT
        || yytoken == Token_BNOT
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE)
      {
        statementWithoutModifier_ast *__node_220 = 0;
        if (!parse_statementWithoutModifier(&__node_220))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_statementWithoutModifier, "statementWithoutModifier");
              }
            return false;
          }
        while (yytoken == Token_IF_MODIFIER
               || yytoken == Token_WHILE_MODIFIER
               || yytoken == Token_UNLESS_MODIFIER
               || yytoken == Token_UNTIL_MODIFIER
               || yytoken == Token_RESCUE_MODIFIER)
          {
            if (yytoken == Token_IF_MODIFIER)
              {
                if (yytoken != Token_IF_MODIFIER)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_IF_MODIFIER, "if");
                      }
                    return false;
                  }
                yylex();

                expression_ast *__node_221 = 0;
                if (!parse_expression(&__node_221))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_expression, "expression");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_UNLESS_MODIFIER)
              {
                if (yytoken != Token_UNLESS_MODIFIER)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_UNLESS_MODIFIER, "unless");
                      }
                    return false;
                  }
                yylex();

                expression_ast *__node_222 = 0;
                if (!parse_expression(&__node_222))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_expression, "expression");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_WHILE_MODIFIER)
              {
                if (yytoken != Token_WHILE_MODIFIER)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_WHILE_MODIFIER, "while");
                      }
                    return false;
                  }
                yylex();

                expression_ast *__node_223 = 0;
                if (!parse_expression(&__node_223))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_expression, "expression");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_UNTIL_MODIFIER)
              {
                if (yytoken != Token_UNTIL_MODIFIER)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_UNTIL_MODIFIER, "until");
                      }
                    return false;
                  }
                yylex();

                expression_ast *__node_224 = 0;
                if (!parse_expression(&__node_224))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_expression, "expression");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_RESCUE_MODIFIER)
              {
                if (yytoken != Token_RESCUE_MODIFIER)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_RESCUE_MODIFIER, "rescue");
                      }
                    return false;
                  }
                yylex();

                expression_ast *__node_225 = 0;
                if (!parse_expression(&__node_225))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_expression, "expression");
                      }
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

  bool parser::parse_statementWithoutModifier(statementWithoutModifier_ast **yynode)
  {
    *yynode = create<statementWithoutModifier_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_UNDEF
        || yytoken == Token_ALIAS
        || yytoken == Token_BEGIN
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_KEYWORD_NOT
        || yytoken == Token_END_UPCASE
        || yytoken == Token_BEGIN_UPCASE
        || yytoken == Token_LPAREN
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_REST_ARG_PREFIX
        || yytoken == Token_UNARY_MINUS
        || yytoken == Token_UNARY_PLUS
        || yytoken == Token_NOT
        || yytoken == Token_BNOT
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE)
      {
        if (yytoken == Token_ALIAS)
          {
            keywordAlias_ast *__node_226 = 0;
            if (!parse_keywordAlias(&__node_226))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_keywordAlias, "keywordAlias");
                  }
                return false;
              }
            aliasParameter_ast *__node_227 = 0;
            if (!parse_aliasParameter(&__node_227))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_aliasParameter, "aliasParameter");
                  }
                return false;
              }
            if (yytoken == Token_LINE_BREAK)
              {
                if (yytoken != Token_LINE_BREAK)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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
            aliasParameter_ast *__node_228 = 0;
            if (!parse_aliasParameter(&__node_228))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_aliasParameter, "aliasParameter");
                  }
                return false;
              }
          }
        else if (yytoken == Token_UNDEF)
          {
            keywordUndef_ast *__node_229 = 0;
            if (!parse_keywordUndef(&__node_229))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_keywordUndef, "keywordUndef");
                  }
                return false;
              }
            undefParameter_ast *__node_230 = 0;
            if (!parse_undefParameter(&__node_230))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_undefParameter, "undefParameter");
                  }
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

                undefParameter_ast *__node_231 = 0;
                if (!parse_undefParameter(&__node_231))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_undefParameter, "undefParameter");
                      }
                    return false;
                  }
              }
          }
        else if (yytoken == Token_BEGIN_UPCASE)
          {
            keywordBeginUpcase_ast *__node_232 = 0;
            if (!parse_keywordBeginUpcase(&__node_232))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_keywordBeginUpcase, "keywordBeginUpcase");
                  }
                return false;
              }
            if (yytoken != Token_LCURLY_BLOCK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LCURLY_BLOCK, "{");
                  }
                return false;
              }
            yylex();

            if (yytoken == Token_UNDEF
                || yytoken == Token_ALIAS
                || yytoken == Token_BEGIN
                || yytoken == Token_RETURN
                || yytoken == Token_BREAK
                || yytoken == Token_NEXT
                || yytoken == Token_NIL
                || yytoken == Token_TRUE
                || yytoken == Token_FALSE
                || yytoken == Token_FILE
                || yytoken == Token_LINE
                || yytoken == Token_SELF
                || yytoken == Token_SUPER
                || yytoken == Token_RETRY
                || yytoken == Token_YIELD
                || yytoken == Token_DEFINED
                || yytoken == Token_REDO
                || yytoken == Token_IF
                || yytoken == Token_CASE
                || yytoken == Token_UNTIL
                || yytoken == Token_WHILE
                || yytoken == Token_FOR
                || yytoken == Token_MODULE
                || yytoken == Token_DEF
                || yytoken == Token_CLASS
                || yytoken == Token_UNLESS
                || yytoken == Token_KEYWORD_NOT
                || yytoken == Token_END_UPCASE
                || yytoken == Token_BEGIN_UPCASE
                || yytoken == Token_LPAREN
                || yytoken == Token_SEMI
                || yytoken == Token_LEADING_TWO_COLON
                || yytoken == Token_LBRACK
                || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
                || yytoken == Token_LCURLY_HASH
                || yytoken == Token_REST_ARG_PREFIX
                || yytoken == Token_UNARY_MINUS
                || yytoken == Token_UNARY_PLUS
                || yytoken == Token_NOT
                || yytoken == Token_BNOT
                || yytoken == Token_LINE_BREAK
                || yytoken == Token_IDENTIFIER
                || yytoken == Token_CONSTANT
                || yytoken == Token_FUNCTION
                || yytoken == Token_GLOBAL_VARIABLE
                || yytoken == Token_INSTANCE_VARIABLE
                || yytoken == Token_CLASS_VARIABLE
                || yytoken == Token_DOUBLE_QUOTED_STRING
                || yytoken == Token_SINGLE_QUOTED_STRING
                || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_REGEX
                || yytoken == Token_COMMAND_OUTPUT
                || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_HERE_DOC_BEGIN
                || yytoken == Token_W_ARRAY
                || yytoken == Token_INTEGER
                || yytoken == Token_HEX
                || yytoken == Token_OCTAL
                || yytoken == Token_BINARY
                || yytoken == Token_FLOAT
                || yytoken == Token_ASCII_VALUE)
              {
                compoundStatement_ast *__node_233 = 0;
                if (!parse_compoundStatement(&__node_233))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_compoundStatement, "compoundStatement");
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
            if (yytoken != Token_RCURLY)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_RCURLY, "}");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_END_UPCASE)
          {
            keywordEndUpcase_ast *__node_234 = 0;
            if (!parse_keywordEndUpcase(&__node_234))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_keywordEndUpcase, "keywordEndUpcase");
                  }
                return false;
              }
            if (yytoken != Token_LCURLY_BLOCK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LCURLY_BLOCK, "{");
                  }
                return false;
              }
            yylex();

            if (yytoken == Token_UNDEF
                || yytoken == Token_ALIAS
                || yytoken == Token_BEGIN
                || yytoken == Token_RETURN
                || yytoken == Token_BREAK
                || yytoken == Token_NEXT
                || yytoken == Token_NIL
                || yytoken == Token_TRUE
                || yytoken == Token_FALSE
                || yytoken == Token_FILE
                || yytoken == Token_LINE
                || yytoken == Token_SELF
                || yytoken == Token_SUPER
                || yytoken == Token_RETRY
                || yytoken == Token_YIELD
                || yytoken == Token_DEFINED
                || yytoken == Token_REDO
                || yytoken == Token_IF
                || yytoken == Token_CASE
                || yytoken == Token_UNTIL
                || yytoken == Token_WHILE
                || yytoken == Token_FOR
                || yytoken == Token_MODULE
                || yytoken == Token_DEF
                || yytoken == Token_CLASS
                || yytoken == Token_UNLESS
                || yytoken == Token_KEYWORD_NOT
                || yytoken == Token_END_UPCASE
                || yytoken == Token_BEGIN_UPCASE
                || yytoken == Token_LPAREN
                || yytoken == Token_SEMI
                || yytoken == Token_LEADING_TWO_COLON
                || yytoken == Token_LBRACK
                || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
                || yytoken == Token_LCURLY_HASH
                || yytoken == Token_REST_ARG_PREFIX
                || yytoken == Token_UNARY_MINUS
                || yytoken == Token_UNARY_PLUS
                || yytoken == Token_NOT
                || yytoken == Token_BNOT
                || yytoken == Token_LINE_BREAK
                || yytoken == Token_IDENTIFIER
                || yytoken == Token_CONSTANT
                || yytoken == Token_FUNCTION
                || yytoken == Token_GLOBAL_VARIABLE
                || yytoken == Token_INSTANCE_VARIABLE
                || yytoken == Token_CLASS_VARIABLE
                || yytoken == Token_DOUBLE_QUOTED_STRING
                || yytoken == Token_SINGLE_QUOTED_STRING
                || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_REGEX
                || yytoken == Token_COMMAND_OUTPUT
                || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_HERE_DOC_BEGIN
                || yytoken == Token_W_ARRAY
                || yytoken == Token_INTEGER
                || yytoken == Token_HEX
                || yytoken == Token_OCTAL
                || yytoken == Token_BINARY
                || yytoken == Token_FLOAT
                || yytoken == Token_ASCII_VALUE)
              {
                compoundStatement_ast *__node_235 = 0;
                if (!parse_compoundStatement(&__node_235))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_compoundStatement, "compoundStatement");
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
            if (yytoken != Token_RCURLY)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_RCURLY, "}");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_BEGIN
                 || yytoken == Token_RETURN
                 || yytoken == Token_BREAK
                 || yytoken == Token_NEXT
                 || yytoken == Token_NIL
                 || yytoken == Token_TRUE
                 || yytoken == Token_FALSE
                 || yytoken == Token_FILE
                 || yytoken == Token_LINE
                 || yytoken == Token_SELF
                 || yytoken == Token_SUPER
                 || yytoken == Token_RETRY
                 || yytoken == Token_YIELD
                 || yytoken == Token_DEFINED
                 || yytoken == Token_REDO
                 || yytoken == Token_IF
                 || yytoken == Token_CASE
                 || yytoken == Token_UNTIL
                 || yytoken == Token_WHILE
                 || yytoken == Token_FOR
                 || yytoken == Token_MODULE
                 || yytoken == Token_DEF
                 || yytoken == Token_CLASS
                 || yytoken == Token_UNLESS
                 || yytoken == Token_KEYWORD_NOT
                 || yytoken == Token_LPAREN
                 || yytoken == Token_LEADING_TWO_COLON
                 || yytoken == Token_LBRACK
                 || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
                 || yytoken == Token_LCURLY_HASH
                 || yytoken == Token_UNARY_MINUS
                 || yytoken == Token_UNARY_PLUS
                 || yytoken == Token_NOT
                 || yytoken == Token_BNOT
                 || yytoken == Token_IDENTIFIER
                 || yytoken == Token_CONSTANT
                 || yytoken == Token_FUNCTION
                 || yytoken == Token_GLOBAL_VARIABLE
                 || yytoken == Token_INSTANCE_VARIABLE
                 || yytoken == Token_CLASS_VARIABLE
                 || yytoken == Token_DOUBLE_QUOTED_STRING
                 || yytoken == Token_SINGLE_QUOTED_STRING
                 || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
                 || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
                 || yytoken == Token_REGEX
                 || yytoken == Token_COMMAND_OUTPUT
                 || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
                 || yytoken == Token_HERE_DOC_BEGIN
                 || yytoken == Token_W_ARRAY
                 || yytoken == Token_INTEGER
                 || yytoken == Token_HEX
                 || yytoken == Token_OCTAL
                 || yytoken == Token_BINARY
                 || yytoken == Token_FLOAT
                 || yytoken == Token_ASCII_VALUE)
          {
            expression_ast *__node_236 = 0;
            if (!parse_expression(&__node_236))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_expression, "expression");
                  }
                return false;
              }
            if (yytoken == Token_COMMA)
              {
                parallelAssignmentLeftOver_ast *__node_237 = 0;
                if (!parse_parallelAssignmentLeftOver(&__node_237))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_parallelAssignmentLeftOver, "parallelAssignmentLeftOver");
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
          }
        else if (yytoken == Token_REST_ARG_PREFIX)
          {
            if (yytoken != Token_REST_ARG_PREFIX)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_REST_ARG_PREFIX, "*");
                  }
                return false;
              }
            yylex();

            mlhs_item_ast *__node_238 = 0;
            if (!parse_mlhs_item(&__node_238))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_mlhs_item, "mlhs_item");
                  }
                return false;
              }
            operatorAssign_ast *__node_239 = 0;
            if (!parse_operatorAssign(&__node_239))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_operatorAssign, "operatorAssign");
                  }
                return false;
              }
            mrhs_ast *__node_240 = 0;
            if (!parse_mrhs(&__node_240))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_mrhs, "mrhs");
                  }
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

  bool parser::parse_statements(statements_ast **yynode)
  {
    *yynode = create<statements_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_UNDEF
        || yytoken == Token_ALIAS
        || yytoken == Token_BEGIN
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_KEYWORD_NOT
        || yytoken == Token_END_UPCASE
        || yytoken == Token_BEGIN_UPCASE
        || yytoken == Token_LPAREN
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_REST_ARG_PREFIX
        || yytoken == Token_UNARY_MINUS
        || yytoken == Token_UNARY_PLUS
        || yytoken == Token_NOT
        || yytoken == Token_BNOT
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE)
      {
        statement_ast *__node_241 = 0;
        if (!parse_statement(&__node_241))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_statement, "statement");
              }
            return false;
          }
        while (yytoken == Token_SEMI
               || yytoken == Token_LINE_BREAK)
          {
            terminal_ast *__node_242 = 0;
            if (!parse_terminal(&__node_242))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_terminal, "terminal");
                  }
                return false;
              }
            if (yytoken == Token_EOF)
              break;
            if ((yytoken == Token_LCURLY) || (yytoken == Token_END) //block end
                || (yytoken == Token_RPAREN) || (yytoken == Token_ELSE)
                || (yytoken == Token_ELSIF) || (yytoken == Token_RESCUE)
                || (yytoken == Token_ENSURE) || (yytoken == Token_WHEN) )
              break;
            statement_ast *__node_243 = 0;
            if (!parse_statement(&__node_243))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_statement, "statement");
                  }
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

  bool parser::parse_string(string_ast **yynode)
  {
    *yynode = create<string_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION)
      {
        if (yytoken == Token_DOUBLE_QUOTED_STRING)
          {
            if (yytoken != Token_DOUBLE_QUOTED_STRING)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_DOUBLE_QUOTED_STRING, "double quoted string");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_SINGLE_QUOTED_STRING)
          {
            if (yytoken != Token_SINGLE_QUOTED_STRING)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_SINGLE_QUOTED_STRING, "single quoted string");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION)
          {
            if (yytoken != Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION, "string before expression substitution");
                  }
                return false;
              }
            yylex();

            expressionSubstitution_ast *__node_244 = 0;
            if (!parse_expressionSubstitution(&__node_244))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_expressionSubstitution, "expressionSubstitution");
                  }
                return false;
              }
            while (yytoken == Token_STRING_BETWEEN_EXPRESSION_SUBSTITUTION)
              {
                if (yytoken != Token_STRING_BETWEEN_EXPRESSION_SUBSTITUTION)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_STRING_BETWEEN_EXPRESSION_SUBSTITUTION, "string between expression substitution");
                      }
                    return false;
                  }
                yylex();

                expressionSubstitution_ast *__node_245 = 0;
                if (!parse_expressionSubstitution(&__node_245))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_expressionSubstitution, "expressionSubstitution");
                      }
                    return false;
                  }
              }
            if (yytoken != Token_STRING_AFTER_EXPRESSION_SUBSTITUTION)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_STRING_AFTER_EXPRESSION_SUBSTITUTION, "string after expression substitution");
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

  bool parser::parse_symbol(symbol_ast **yynode)
  {
    *yynode = create<symbol_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE)
      {
        if (yytoken != Token_COLON_WITH_NO_FOLLOWING_SPACE)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_COLON_WITH_NO_FOLLOWING_SPACE, ":");
              }
            return false;
          }
        yylex();

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
            yylex();

            if (yytoken == Token_ASSIGN_WITH_NO_LEADING_SPACE)
              {
                if (yytoken != Token_ASSIGN_WITH_NO_LEADING_SPACE)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_ASSIGN_WITH_NO_LEADING_SPACE, "=");
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
        else if (yytoken == Token_FUNCTION)
          {
            if (yytoken != Token_FUNCTION)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_FUNCTION, "function");
                  }
                return false;
              }
            yylex();

            if (yytoken == Token_ASSIGN_WITH_NO_LEADING_SPACE)
              {
                if (yytoken != Token_ASSIGN_WITH_NO_LEADING_SPACE)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_ASSIGN_WITH_NO_LEADING_SPACE, "=");
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
        else if (yytoken == Token_CONSTANT)
          {
            if (yytoken != Token_CONSTANT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_CONSTANT, "constant");
                  }
                return false;
              }
            yylex();

            if (yytoken == Token_ASSIGN_WITH_NO_LEADING_SPACE)
              {
                if (yytoken != Token_ASSIGN_WITH_NO_LEADING_SPACE)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_ASSIGN_WITH_NO_LEADING_SPACE, "=");
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
        else if (yytoken == Token_GLOBAL_VARIABLE)
          {
            if (yytoken != Token_GLOBAL_VARIABLE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_GLOBAL_VARIABLE, "global variable");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_INSTANCE_VARIABLE)
          {
            if (yytoken != Token_INSTANCE_VARIABLE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_INSTANCE_VARIABLE, "instance variable");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_CLASS_VARIABLE)
          {
            if (yytoken != Token_CLASS_VARIABLE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_CLASS_VARIABLE, "class variable");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_UNARY_PLUS_MINUS_METHOD_NAME)
          {
            if (yytoken != Token_UNARY_PLUS_MINUS_METHOD_NAME)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_UNARY_PLUS_MINUS_METHOD_NAME, "+/- method");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_SINGLE_QUOTE
                 || yytoken == Token_LBRACK
                 || yytoken == Token_LBRACK_ARRAY_ACCESS
                 || yytoken == Token_BOR
                 || yytoken == Token_COMPARE
                 || yytoken == Token_EQUAL
                 || yytoken == Token_CASE_EQUAL
                 || yytoken == Token_MATCH
                 || yytoken == Token_LESS_THAN
                 || yytoken == Token_GREATER_THAN
                 || yytoken == Token_LESS_OR_EQUAL
                 || yytoken == Token_GREATER_OR_EQUAL
                 || yytoken == Token_BXOR
                 || yytoken == Token_BAND
                 || yytoken == Token_LEFT_SHIFT
                 || yytoken == Token_RIGHT_SHIFT
                 || yytoken == Token_PLUS
                 || yytoken == Token_MINUS
                 || yytoken == Token_STAR
                 || yytoken == Token_DIV
                 || yytoken == Token_MOD
                 || yytoken == Token_BNOT
                 || yytoken == Token_POWER)
          {
            operatorAsMethodname_ast *__node_246 = 0;
            if (!parse_operatorAsMethodname(&__node_246))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_operatorAsMethodname, "operatorAsMethodname");
                  }
                return false;
              }
          }
        else if (yytoken == Token_END
                 || yytoken == Token_ELSE
                 || yytoken == Token_ELSIF
                 || yytoken == Token_RESCUE
                 || yytoken == Token_ENSURE
                 || yytoken == Token_WHEN
                 || yytoken == Token_UNDEF
                 || yytoken == Token_ALIAS
                 || yytoken == Token_BEGIN
                 || yytoken == Token_DO
                 || yytoken == Token_RETURN
                 || yytoken == Token_BREAK
                 || yytoken == Token_NEXT
                 || yytoken == Token_NIL
                 || yytoken == Token_TRUE
                 || yytoken == Token_FALSE
                 || yytoken == Token_FILE
                 || yytoken == Token_LINE
                 || yytoken == Token_SELF
                 || yytoken == Token_SUPER
                 || yytoken == Token_RETRY
                 || yytoken == Token_YIELD
                 || yytoken == Token_DEFINED
                 || yytoken == Token_REDO
                 || yytoken == Token_IF
                 || yytoken == Token_CASE
                 || yytoken == Token_UNTIL
                 || yytoken == Token_WHILE
                 || yytoken == Token_FOR
                 || yytoken == Token_MODULE
                 || yytoken == Token_DEF
                 || yytoken == Token_THEN
                 || yytoken == Token_CLASS
                 || yytoken == Token_UNLESS
                 || yytoken == Token_OR
                 || yytoken == Token_AND
                 || yytoken == Token_IN
                 || yytoken == Token_END_UPCASE
                 || yytoken == Token_BEGIN_UPCASE
                 || yytoken == Token_NOT)
          {
            keyword_ast *__node_247 = 0;
            if (!parse_keyword(&__node_247))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_keyword, "keyword");
                  }
                return false;
              }
          }
        else if (yytoken == Token_DOUBLE_QUOTED_STRING
                 || yytoken == Token_SINGLE_QUOTED_STRING
                 || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION)
          {
            string_ast *__node_248 = 0;
            if (!parse_string(&__node_248))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_string, "string");
                  }
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

  bool parser::parse_terminal(terminal_ast **yynode)
  {
    *yynode = create<terminal_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_SEMI
        || yytoken == Token_LINE_BREAK)
      {
        if (yytoken == Token_SEMI)
          {
            if (yytoken != Token_SEMI)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_SEMI, ";");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_LINE_BREAK)
          {
            if (yytoken != Token_LINE_BREAK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LINE_BREAK, "line break");
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

  bool parser::parse_ternaryIfThenElseExpression(ternaryIfThenElseExpression_ast **yynode)
  {
    *yynode = create<ternaryIfThenElseExpression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BEGIN
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_LPAREN
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_UNARY_MINUS
        || yytoken == Token_UNARY_PLUS
        || yytoken == Token_NOT
        || yytoken == Token_BNOT
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE)
      {
        assignmentExpression_ast *__node_249 = 0;
        if (!parse_assignmentExpression(&__node_249))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_assignmentExpression, "assignmentExpression");
              }
            return false;
          }
        if (yytoken == Token_QUESTION)
          {
            operatorQuestion_ast *__node_250 = 0;
            if (!parse_operatorQuestion(&__node_250))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_operatorQuestion, "operatorQuestion");
                  }
                return false;
              }
            ternaryIfThenElseExpression_ast *__node_251 = 0;
            if (!parse_ternaryIfThenElseExpression(&__node_251))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_ternaryIfThenElseExpression, "ternaryIfThenElseExpression");
                  }
                return false;
              }
            operatorColon_ast *__node_252 = 0;
            if (!parse_operatorColon(&__node_252))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_operatorColon, "operatorColon");
                  }
                return false;
              }
            ternaryIfThenElseExpression_ast *__node_253 = 0;
            if (!parse_ternaryIfThenElseExpression(&__node_253))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_ternaryIfThenElseExpression, "ternaryIfThenElseExpression");
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
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_thenOrTerminalOrColon(thenOrTerminalOrColon_ast **yynode)
  {
    *yynode = create<thenOrTerminalOrColon_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_THEN
        || yytoken == Token_SEMI
        || yytoken == Token_COLON
        || yytoken == Token_LINE_BREAK)
      {
        if (yytoken == Token_THEN)
          {
            if (yytoken != Token_THEN)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_THEN, "then");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_SEMI
                 || yytoken == Token_LINE_BREAK)
          {
            terminal_ast *__node_254 = 0;
            if (!parse_terminal(&__node_254))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_terminal, "terminal");
                  }
                return false;
              }
            if (yytoken == Token_THEN)
              {
                if (yytoken != Token_THEN)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_THEN, "then");
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
        else if (yytoken == Token_COLON)
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

  bool parser::parse_unaryExpression(unaryExpression_ast **yynode)
  {
    *yynode = create<unaryExpression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BEGIN
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_LPAREN
        || yytoken == Token_LEADING_TWO_COLON
        || yytoken == Token_LBRACK
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_LCURLY_HASH
        || yytoken == Token_UNARY_MINUS
        || yytoken == Token_UNARY_PLUS
        || yytoken == Token_NOT
        || yytoken == Token_BNOT
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE
        || yytoken == Token_DOUBLE_QUOTED_STRING
        || yytoken == Token_SINGLE_QUOTED_STRING
        || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_REGEX
        || yytoken == Token_COMMAND_OUTPUT
        || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
        || yytoken == Token_HERE_DOC_BEGIN
        || yytoken == Token_W_ARRAY
        || yytoken == Token_INTEGER
        || yytoken == Token_HEX
        || yytoken == Token_OCTAL
        || yytoken == Token_BINARY
        || yytoken == Token_FLOAT
        || yytoken == Token_ASCII_VALUE)
      {
        while (yytoken == Token_UNARY_MINUS
               || yytoken == Token_UNARY_PLUS
               || yytoken == Token_NOT
               || yytoken == Token_BNOT)
          {
            if (yytoken == Token_UNARY_PLUS)
              {
                operatorUnaryPlus_ast *__node_255 = 0;
                if (!parse_operatorUnaryPlus(&__node_255))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorUnaryPlus, "operatorUnaryPlus");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_UNARY_MINUS)
              {
                operatorUnaryMinus_ast *__node_256 = 0;
                if (!parse_operatorUnaryMinus(&__node_256))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorUnaryMinus, "operatorUnaryMinus");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_BNOT)
              {
                operatorBNot_ast *__node_257 = 0;
                if (!parse_operatorBNot(&__node_257))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorBNot, "operatorBNot");
                      }
                    return false;
                  }
              }
            else if (yytoken == Token_NOT)
              {
                operatorNot_ast *__node_258 = 0;
                if (!parse_operatorNot(&__node_258))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_operatorNot, "operatorNot");
                      }
                    return false;
                  }
              }
            else
              {
                return false;
              }
          }
        dotAccess_ast *__node_259 = 0;
        if (!parse_dotAccess(&__node_259))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_dotAccess, "dotAccess");
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

  bool parser::parse_undefParameter(undefParameter_ast **yynode)
  {
    *yynode = create<undefParameter_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_END
        || yytoken == Token_ELSE
        || yytoken == Token_ELSIF
        || yytoken == Token_RESCUE
        || yytoken == Token_ENSURE
        || yytoken == Token_WHEN
        || yytoken == Token_UNDEF
        || yytoken == Token_ALIAS
        || yytoken == Token_BEGIN
        || yytoken == Token_DO
        || yytoken == Token_RETURN
        || yytoken == Token_BREAK
        || yytoken == Token_NEXT
        || yytoken == Token_SUPER
        || yytoken == Token_RETRY
        || yytoken == Token_YIELD
        || yytoken == Token_DEFINED
        || yytoken == Token_REDO
        || yytoken == Token_IF
        || yytoken == Token_CASE
        || yytoken == Token_UNTIL
        || yytoken == Token_WHILE
        || yytoken == Token_FOR
        || yytoken == Token_MODULE
        || yytoken == Token_DEF
        || yytoken == Token_THEN
        || yytoken == Token_CLASS
        || yytoken == Token_UNLESS
        || yytoken == Token_OR
        || yytoken == Token_AND
        || yytoken == Token_IN
        || yytoken == Token_END_UPCASE
        || yytoken == Token_BEGIN_UPCASE
        || yytoken == Token_SINGLE_QUOTE
        || yytoken == Token_LBRACK
        || yytoken == Token_LBRACK_ARRAY_ACCESS
        || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
        || yytoken == Token_BOR
        || yytoken == Token_COMPARE
        || yytoken == Token_EQUAL
        || yytoken == Token_CASE_EQUAL
        || yytoken == Token_MATCH
        || yytoken == Token_LESS_THAN
        || yytoken == Token_GREATER_THAN
        || yytoken == Token_LESS_OR_EQUAL
        || yytoken == Token_GREATER_OR_EQUAL
        || yytoken == Token_BXOR
        || yytoken == Token_BAND
        || yytoken == Token_LEFT_SHIFT
        || yytoken == Token_RIGHT_SHIFT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_STAR
        || yytoken == Token_DIV
        || yytoken == Token_MOD
        || yytoken == Token_NOT
        || yytoken == Token_BNOT
        || yytoken == Token_POWER
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION)
      {
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
            yylex();

            if (yytoken == Token_ASSIGN_WITH_NO_LEADING_SPACE)
              {
                if (yytoken != Token_ASSIGN_WITH_NO_LEADING_SPACE)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_ASSIGN_WITH_NO_LEADING_SPACE, "=");
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
        else if (yytoken == Token_CONSTANT)
          {
            if (yytoken != Token_CONSTANT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_CONSTANT, "constant");
                  }
                return false;
              }
            yylex();

            if (yytoken == Token_ASSIGN_WITH_NO_LEADING_SPACE)
              {
                if (yytoken != Token_ASSIGN_WITH_NO_LEADING_SPACE)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_ASSIGN_WITH_NO_LEADING_SPACE, "=");
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
        else if (yytoken == Token_FUNCTION)
          {
            if (yytoken != Token_FUNCTION)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_FUNCTION, "function");
                  }
                return false;
              }
            yylex();

            if (yytoken == Token_ASSIGN_WITH_NO_LEADING_SPACE)
              {
                if (yytoken != Token_ASSIGN_WITH_NO_LEADING_SPACE)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_ASSIGN_WITH_NO_LEADING_SPACE, "=");
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
        else if (yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE)
          {
            symbol_ast *__node_260 = 0;
            if (!parse_symbol(&__node_260))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_symbol, "symbol");
                  }
                return false;
              }
          }
        else if (yytoken == Token_END
                 || yytoken == Token_ELSE
                 || yytoken == Token_ELSIF
                 || yytoken == Token_RESCUE
                 || yytoken == Token_ENSURE
                 || yytoken == Token_WHEN
                 || yytoken == Token_UNDEF
                 || yytoken == Token_ALIAS
                 || yytoken == Token_BEGIN
                 || yytoken == Token_DO
                 || yytoken == Token_RETURN
                 || yytoken == Token_BREAK
                 || yytoken == Token_NEXT
                 || yytoken == Token_SUPER
                 || yytoken == Token_RETRY
                 || yytoken == Token_YIELD
                 || yytoken == Token_DEFINED
                 || yytoken == Token_REDO
                 || yytoken == Token_IF
                 || yytoken == Token_CASE
                 || yytoken == Token_UNTIL
                 || yytoken == Token_WHILE
                 || yytoken == Token_FOR
                 || yytoken == Token_MODULE
                 || yytoken == Token_DEF
                 || yytoken == Token_THEN
                 || yytoken == Token_CLASS
                 || yytoken == Token_UNLESS
                 || yytoken == Token_OR
                 || yytoken == Token_AND
                 || yytoken == Token_IN
                 || yytoken == Token_END_UPCASE
                 || yytoken == Token_BEGIN_UPCASE
                 || yytoken == Token_NOT)
          {
            keywordAsMethodName_ast *__node_261 = 0;
            if (!parse_keywordAsMethodName(&__node_261))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_keywordAsMethodName, "keywordAsMethodName");
                  }
                return false;
              }
          }
        else if (yytoken == Token_SINGLE_QUOTE
                 || yytoken == Token_LBRACK
                 || yytoken == Token_LBRACK_ARRAY_ACCESS
                 || yytoken == Token_BOR
                 || yytoken == Token_COMPARE
                 || yytoken == Token_EQUAL
                 || yytoken == Token_CASE_EQUAL
                 || yytoken == Token_MATCH
                 || yytoken == Token_LESS_THAN
                 || yytoken == Token_GREATER_THAN
                 || yytoken == Token_LESS_OR_EQUAL
                 || yytoken == Token_GREATER_OR_EQUAL
                 || yytoken == Token_BXOR
                 || yytoken == Token_BAND
                 || yytoken == Token_LEFT_SHIFT
                 || yytoken == Token_RIGHT_SHIFT
                 || yytoken == Token_PLUS
                 || yytoken == Token_MINUS
                 || yytoken == Token_STAR
                 || yytoken == Token_DIV
                 || yytoken == Token_MOD
                 || yytoken == Token_BNOT
                 || yytoken == Token_POWER)
          {
            operatorAsMethodname_ast *__node_262 = 0;
            if (!parse_operatorAsMethodname(&__node_262))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_operatorAsMethodname, "operatorAsMethodname");
                  }
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

  bool parser::parse_unlessExpression(unlessExpression_ast **yynode)
  {
    *yynode = create<unlessExpression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_UNLESS)
      {
        if (yytoken != Token_UNLESS)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_UNLESS, "unless");
              }
            return false;
          }
        yylex();

        expression_ast *__node_263 = 0;
        if (!parse_expression(&__node_263))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_expression, "expression");
              }
            return false;
          }
        thenOrTerminalOrColon_ast *__node_264 = 0;
        if (!parse_thenOrTerminalOrColon(&__node_264))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_thenOrTerminalOrColon, "thenOrTerminalOrColon");
              }
            return false;
          }
        if (yytoken == Token_UNDEF
            || yytoken == Token_ALIAS
            || yytoken == Token_BEGIN
            || yytoken == Token_RETURN
            || yytoken == Token_BREAK
            || yytoken == Token_NEXT
            || yytoken == Token_NIL
            || yytoken == Token_TRUE
            || yytoken == Token_FALSE
            || yytoken == Token_FILE
            || yytoken == Token_LINE
            || yytoken == Token_SELF
            || yytoken == Token_SUPER
            || yytoken == Token_RETRY
            || yytoken == Token_YIELD
            || yytoken == Token_DEFINED
            || yytoken == Token_REDO
            || yytoken == Token_IF
            || yytoken == Token_CASE
            || yytoken == Token_UNTIL
            || yytoken == Token_WHILE
            || yytoken == Token_FOR
            || yytoken == Token_MODULE
            || yytoken == Token_DEF
            || yytoken == Token_CLASS
            || yytoken == Token_UNLESS
            || yytoken == Token_KEYWORD_NOT
            || yytoken == Token_END_UPCASE
            || yytoken == Token_BEGIN_UPCASE
            || yytoken == Token_LPAREN
            || yytoken == Token_SEMI
            || yytoken == Token_LEADING_TWO_COLON
            || yytoken == Token_LBRACK
            || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
            || yytoken == Token_LCURLY_HASH
            || yytoken == Token_REST_ARG_PREFIX
            || yytoken == Token_UNARY_MINUS
            || yytoken == Token_UNARY_PLUS
            || yytoken == Token_NOT
            || yytoken == Token_BNOT
            || yytoken == Token_LINE_BREAK
            || yytoken == Token_IDENTIFIER
            || yytoken == Token_CONSTANT
            || yytoken == Token_FUNCTION
            || yytoken == Token_GLOBAL_VARIABLE
            || yytoken == Token_INSTANCE_VARIABLE
            || yytoken == Token_CLASS_VARIABLE
            || yytoken == Token_DOUBLE_QUOTED_STRING
            || yytoken == Token_SINGLE_QUOTED_STRING
            || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_REGEX
            || yytoken == Token_COMMAND_OUTPUT
            || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_HERE_DOC_BEGIN
            || yytoken == Token_W_ARRAY
            || yytoken == Token_INTEGER
            || yytoken == Token_HEX
            || yytoken == Token_OCTAL
            || yytoken == Token_BINARY
            || yytoken == Token_FLOAT
            || yytoken == Token_ASCII_VALUE)
          {
            compoundStatement_ast *__node_265 = 0;
            if (!parse_compoundStatement(&__node_265))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_compoundStatement, "compoundStatement");
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

            if (yytoken == Token_UNDEF
                || yytoken == Token_ALIAS
                || yytoken == Token_BEGIN
                || yytoken == Token_RETURN
                || yytoken == Token_BREAK
                || yytoken == Token_NEXT
                || yytoken == Token_NIL
                || yytoken == Token_TRUE
                || yytoken == Token_FALSE
                || yytoken == Token_FILE
                || yytoken == Token_LINE
                || yytoken == Token_SELF
                || yytoken == Token_SUPER
                || yytoken == Token_RETRY
                || yytoken == Token_YIELD
                || yytoken == Token_DEFINED
                || yytoken == Token_REDO
                || yytoken == Token_IF
                || yytoken == Token_CASE
                || yytoken == Token_UNTIL
                || yytoken == Token_WHILE
                || yytoken == Token_FOR
                || yytoken == Token_MODULE
                || yytoken == Token_DEF
                || yytoken == Token_CLASS
                || yytoken == Token_UNLESS
                || yytoken == Token_KEYWORD_NOT
                || yytoken == Token_END_UPCASE
                || yytoken == Token_BEGIN_UPCASE
                || yytoken == Token_LPAREN
                || yytoken == Token_SEMI
                || yytoken == Token_LEADING_TWO_COLON
                || yytoken == Token_LBRACK
                || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
                || yytoken == Token_LCURLY_HASH
                || yytoken == Token_REST_ARG_PREFIX
                || yytoken == Token_UNARY_MINUS
                || yytoken == Token_UNARY_PLUS
                || yytoken == Token_NOT
                || yytoken == Token_BNOT
                || yytoken == Token_LINE_BREAK
                || yytoken == Token_IDENTIFIER
                || yytoken == Token_CONSTANT
                || yytoken == Token_FUNCTION
                || yytoken == Token_GLOBAL_VARIABLE
                || yytoken == Token_INSTANCE_VARIABLE
                || yytoken == Token_CLASS_VARIABLE
                || yytoken == Token_DOUBLE_QUOTED_STRING
                || yytoken == Token_SINGLE_QUOTED_STRING
                || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_REGEX
                || yytoken == Token_COMMAND_OUTPUT
                || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
                || yytoken == Token_HERE_DOC_BEGIN
                || yytoken == Token_W_ARRAY
                || yytoken == Token_INTEGER
                || yytoken == Token_HEX
                || yytoken == Token_OCTAL
                || yytoken == Token_BINARY
                || yytoken == Token_FLOAT
                || yytoken == Token_ASCII_VALUE)
              {
                compoundStatement_ast *__node_266 = 0;
                if (!parse_compoundStatement(&__node_266))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_compoundStatement, "compoundStatement");
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
          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        if (yytoken != Token_END)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_END, "end");
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

  bool parser::parse_untilExpression(untilExpression_ast **yynode)
  {
    *yynode = create<untilExpression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_UNTIL)
      {
        keywordUntil_ast *__node_267 = 0;
        if (!parse_keywordUntil(&__node_267))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_keywordUntil, "keywordUntil");
              }
            return false;
          }
        expression_ast *__node_268 = 0;
        if (!parse_expression(&__node_268))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_expression, "expression");
              }
            return false;
          }
        doOrTerminalOrColon_ast *__node_269 = 0;
        if (!parse_doOrTerminalOrColon(&__node_269))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_doOrTerminalOrColon, "doOrTerminalOrColon");
              }
            return false;
          }
        if (yytoken == Token_UNDEF
            || yytoken == Token_ALIAS
            || yytoken == Token_BEGIN
            || yytoken == Token_RETURN
            || yytoken == Token_BREAK
            || yytoken == Token_NEXT
            || yytoken == Token_NIL
            || yytoken == Token_TRUE
            || yytoken == Token_FALSE
            || yytoken == Token_FILE
            || yytoken == Token_LINE
            || yytoken == Token_SELF
            || yytoken == Token_SUPER
            || yytoken == Token_RETRY
            || yytoken == Token_YIELD
            || yytoken == Token_DEFINED
            || yytoken == Token_REDO
            || yytoken == Token_IF
            || yytoken == Token_CASE
            || yytoken == Token_UNTIL
            || yytoken == Token_WHILE
            || yytoken == Token_FOR
            || yytoken == Token_MODULE
            || yytoken == Token_DEF
            || yytoken == Token_CLASS
            || yytoken == Token_UNLESS
            || yytoken == Token_KEYWORD_NOT
            || yytoken == Token_END_UPCASE
            || yytoken == Token_BEGIN_UPCASE
            || yytoken == Token_LPAREN
            || yytoken == Token_SEMI
            || yytoken == Token_LEADING_TWO_COLON
            || yytoken == Token_LBRACK
            || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
            || yytoken == Token_LCURLY_HASH
            || yytoken == Token_REST_ARG_PREFIX
            || yytoken == Token_UNARY_MINUS
            || yytoken == Token_UNARY_PLUS
            || yytoken == Token_NOT
            || yytoken == Token_BNOT
            || yytoken == Token_LINE_BREAK
            || yytoken == Token_IDENTIFIER
            || yytoken == Token_CONSTANT
            || yytoken == Token_FUNCTION
            || yytoken == Token_GLOBAL_VARIABLE
            || yytoken == Token_INSTANCE_VARIABLE
            || yytoken == Token_CLASS_VARIABLE
            || yytoken == Token_DOUBLE_QUOTED_STRING
            || yytoken == Token_SINGLE_QUOTED_STRING
            || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_REGEX
            || yytoken == Token_COMMAND_OUTPUT
            || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_HERE_DOC_BEGIN
            || yytoken == Token_W_ARRAY
            || yytoken == Token_INTEGER
            || yytoken == Token_HEX
            || yytoken == Token_OCTAL
            || yytoken == Token_BINARY
            || yytoken == Token_FLOAT
            || yytoken == Token_ASCII_VALUE)
          {
            compoundStatement_ast *__node_270 = 0;
            if (!parse_compoundStatement(&__node_270))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_compoundStatement, "compoundStatement");
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
        if (yytoken != Token_END)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_END, "end");
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

  bool parser::parse_variable(variable_ast **yynode)
  {
    *yynode = create<variable_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_NIL
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_FILE
        || yytoken == Token_LINE
        || yytoken == Token_SELF
        || yytoken == Token_SUPER
        || yytoken == Token_IDENTIFIER
        || yytoken == Token_CONSTANT
        || yytoken == Token_FUNCTION
        || yytoken == Token_GLOBAL_VARIABLE
        || yytoken == Token_INSTANCE_VARIABLE
        || yytoken == Token_CLASS_VARIABLE)
      {
        if (yytoken == Token_FUNCTION)
          {
            if (yytoken != Token_FUNCTION)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_FUNCTION, "function");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_SELF)
          {
            if (yytoken != Token_SELF)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_SELF, "self");
                  }
                return false;
              }
            yylex();

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

          }
        else if (yytoken == Token_IDENTIFIER)
          {
            if (yytoken != Token_IDENTIFIER)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_IDENTIFIER, "identifier");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_CONSTANT)
          {
            if (yytoken != Token_CONSTANT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_CONSTANT, "constant");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_INSTANCE_VARIABLE)
          {
            if (yytoken != Token_INSTANCE_VARIABLE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_INSTANCE_VARIABLE, "instance variable");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_GLOBAL_VARIABLE)
          {
            if (yytoken != Token_GLOBAL_VARIABLE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_GLOBAL_VARIABLE, "global variable");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_CLASS_VARIABLE)
          {
            if (yytoken != Token_CLASS_VARIABLE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_CLASS_VARIABLE, "class variable");
                  }
                return false;
              }
            yylex();

          }
        else if (yytoken == Token_NIL
                 || yytoken == Token_TRUE
                 || yytoken == Token_FALSE
                 || yytoken == Token_FILE
                 || yytoken == Token_LINE)
          {
            predefinedValue_ast *__node_271 = 0;
            if (!parse_predefinedValue(&__node_271))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_predefinedValue, "predefinedValue");
                  }
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

  bool parser::parse_whileExpression(whileExpression_ast **yynode)
  {
    *yynode = create<whileExpression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_WHILE)
      {
        keywordWhile_ast *__node_272 = 0;
        if (!parse_keywordWhile(&__node_272))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_keywordWhile, "keywordWhile");
              }
            return false;
          }
        expression_ast *__node_273 = 0;
        if (!parse_expression(&__node_273))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_expression, "expression");
              }
            return false;
          }
        doOrTerminalOrColon_ast *__node_274 = 0;
        if (!parse_doOrTerminalOrColon(&__node_274))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_doOrTerminalOrColon, "doOrTerminalOrColon");
              }
            return false;
          }
        if (yytoken == Token_UNDEF
            || yytoken == Token_ALIAS
            || yytoken == Token_BEGIN
            || yytoken == Token_RETURN
            || yytoken == Token_BREAK
            || yytoken == Token_NEXT
            || yytoken == Token_NIL
            || yytoken == Token_TRUE
            || yytoken == Token_FALSE
            || yytoken == Token_FILE
            || yytoken == Token_LINE
            || yytoken == Token_SELF
            || yytoken == Token_SUPER
            || yytoken == Token_RETRY
            || yytoken == Token_YIELD
            || yytoken == Token_DEFINED
            || yytoken == Token_REDO
            || yytoken == Token_IF
            || yytoken == Token_CASE
            || yytoken == Token_UNTIL
            || yytoken == Token_WHILE
            || yytoken == Token_FOR
            || yytoken == Token_MODULE
            || yytoken == Token_DEF
            || yytoken == Token_CLASS
            || yytoken == Token_UNLESS
            || yytoken == Token_KEYWORD_NOT
            || yytoken == Token_END_UPCASE
            || yytoken == Token_BEGIN_UPCASE
            || yytoken == Token_LPAREN
            || yytoken == Token_SEMI
            || yytoken == Token_LEADING_TWO_COLON
            || yytoken == Token_LBRACK
            || yytoken == Token_COLON_WITH_NO_FOLLOWING_SPACE
            || yytoken == Token_LCURLY_HASH
            || yytoken == Token_REST_ARG_PREFIX
            || yytoken == Token_UNARY_MINUS
            || yytoken == Token_UNARY_PLUS
            || yytoken == Token_NOT
            || yytoken == Token_BNOT
            || yytoken == Token_LINE_BREAK
            || yytoken == Token_IDENTIFIER
            || yytoken == Token_CONSTANT
            || yytoken == Token_FUNCTION
            || yytoken == Token_GLOBAL_VARIABLE
            || yytoken == Token_INSTANCE_VARIABLE
            || yytoken == Token_CLASS_VARIABLE
            || yytoken == Token_DOUBLE_QUOTED_STRING
            || yytoken == Token_SINGLE_QUOTED_STRING
            || yytoken == Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_REGEX
            || yytoken == Token_COMMAND_OUTPUT
            || yytoken == Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION
            || yytoken == Token_HERE_DOC_BEGIN
            || yytoken == Token_W_ARRAY
            || yytoken == Token_INTEGER
            || yytoken == Token_HEX
            || yytoken == Token_OCTAL
            || yytoken == Token_BINARY
            || yytoken == Token_FLOAT
            || yytoken == Token_ASCII_VALUE)
          {
            compoundStatement_ast *__node_275 = 0;
            if (!parse_compoundStatement(&__node_275))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_compoundStatement, "compoundStatement");
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
        if (yytoken != Token_END)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_END, "end");
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


} // end of namespace ruby


