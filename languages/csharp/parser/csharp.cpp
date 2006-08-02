// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#include "csharp.h"


namespace csharp
  {

  parser::csharp_compatibility_mode parser::compatibility_mode()
  {
    return _M_compatibility_mode;
  }
  void parser::set_compatibility_mode( parser::csharp_compatibility_mode mode )
  {
    _M_compatibility_mode = mode;
  }

  void parser::pp_define_symbol( std::string symbol_name )
  {
    _M_pp_defined_symbols.insert(symbol_name);
  }

  void parser::pp_undefine_symbol( std::string symbol_name )
  {
    _M_pp_defined_symbols.erase(symbol_name);
  }

  bool parser::pp_is_symbol_defined( std::string symbol_name )
  {
    return (_M_pp_defined_symbols.find(symbol_name) != _M_pp_defined_symbols.end());
  }


  // Rather hackish solution for recognizing expressions like
  // "a is sometype ? if_exp : else_exp", see conditional_expression.
  // Needs three methods to fix parsing for about 0.2% of all C# source files.

  bool parser::is_nullable_type(type_ast *type)
  {
    if (!type)
      return false;
    else if (!type->unmanaged_type)
      return false;
    else if (!type->unmanaged_type->regular_type || type->unmanaged_type->unmanaged_type_suffix_sequence)
      return false;
    else if (!type->unmanaged_type->regular_type->optionally_nullable_type)
      return false;
    else if (type->unmanaged_type->regular_type->optionally_nullable_type->nullable == false)
      return false;
    else // if (optionally_nullable_type->nullable == true)
      return true;
  }

  // This method is only to be called after is_nullable_type(type) returns true,
  // and therefore expects all the appropriate members not to be 0.
  void parser::unset_nullable_type(type_ast *type)
  {
    type->unmanaged_type->regular_type->optionally_nullable_type->nullable = false;
  }

  // This method expects null_coalescing_expression to be fully parsed and valid.
  // (Otherwise, this method is not called at all.
  type_ast *parser::last_relational_expression_rest_type(
    null_coalescing_expression_ast *null_coalescing_expression)
  {
    relational_expression_ast *relexp =
      null_coalescing_expression
      ->expression_sequence->to_back()->element // gets a logical_or_expression
      ->expression_sequence->to_back()->element // gets a logical_and_expression
      ->expression_sequence->to_back()->element // gets a bit_or_expression
      ->expression_sequence->to_back()->element // gets a bit_xor_expression
      ->expression_sequence->to_back()->element // gets a bit_and_expression
      ->expression_sequence->to_back()->element // gets an equality_expression
      ->expression                              // gets a relational_expression
      ;

    if (relexp->additional_expression_sequence != 0)
      return relexp->additional_expression_sequence->to_back()->element->type;
    else
      return 0;
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

} // end of namespace csharp


namespace csharp
  {

  bool parser::parse_accessor_declarations(accessor_declarations_ast **yynode)
  {
    *yynode = create<accessor_declarations_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_INTERNAL
        || yytoken == Token_PRIVATE
        || yytoken == Token_PROTECTED
        || yytoken == Token_GET
        || yytoken == Token_SET
        || yytoken == Token_LBRACKET)
      {
        optional_attribute_sections_ast *__node_0 = 0;
        if (!parse_optional_attribute_sections(&__node_0))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_optional_attribute_sections, "optional_attribute_sections");
              }
            return false;
          }
        (*yynode)->accessor1_attributes = __node_0;

        if (yytoken == Token_INTERNAL
            || yytoken == Token_PRIVATE
            || yytoken == Token_PROTECTED)
          {
            accessor_modifier_ast *__node_1 = 0;
            if (!parse_accessor_modifier(&__node_1))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_accessor_modifier, "accessor_modifier");
                  }
                return false;
              }
            (*yynode)->accessor1_modifier = __node_1;

          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        if (yytoken == Token_GET)
          {
            if (yytoken != Token_GET)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_GET, "get");
                  }
                return false;
              }
            yylex();

            if (yytoken == Token_LBRACE)
              {
                block_ast *__node_2 = 0;
                if (!parse_block(&__node_2))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_block, "block");
                      }
                    return false;
                  }
                (*yynode)->accessor1_body = __node_2;

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
            (*yynode)->accessor1_type = accessor_declarations::type_get;
            if (yytoken == Token_INTERNAL
                || yytoken == Token_PRIVATE
                || yytoken == Token_PROTECTED
                || yytoken == Token_SET
                || yytoken == Token_LBRACKET)
              {
                optional_attribute_sections_ast *__node_3 = 0;
                if (!parse_optional_attribute_sections(&__node_3))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_optional_attribute_sections, "optional_attribute_sections");
                      }
                    return false;
                  }
                (*yynode)->accessor2_attributes = __node_3;

                if (yytoken == Token_INTERNAL
                    || yytoken == Token_PRIVATE
                    || yytoken == Token_PROTECTED)
                  {
                    accessor_modifier_ast *__node_4 = 0;
                    if (!parse_accessor_modifier(&__node_4))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_accessor_modifier, "accessor_modifier");
                          }
                        return false;
                      }
                    (*yynode)->accessor2_modifier = __node_4;

                  }
                else if (true /*epsilon*/)
                {}
                else
                  {
                    return false;
                  }
                if (yytoken != Token_SET)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_SET, "set");
                      }
                    return false;
                  }
                yylex();

                if (yytoken == Token_LBRACE)
                  {
                    block_ast *__node_5 = 0;
                    if (!parse_block(&__node_5))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_block, "block");
                          }
                        return false;
                      }
                    (*yynode)->accessor2_body = __node_5;

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
                (*yynode)->accessor2_type = accessor_declarations::type_set;
              }
            else if (true /*epsilon*/)
              {
                (*yynode)->accessor2_type = accessor_declarations::type_none;
              }
            else
              {
                return false;
              }
          }
        else if (yytoken == Token_SET)
          {
            if (yytoken != Token_SET)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_SET, "set");
                  }
                return false;
              }
            yylex();

            if (yytoken == Token_LBRACE)
              {
                block_ast *__node_6 = 0;
                if (!parse_block(&__node_6))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_block, "block");
                      }
                    return false;
                  }
                (*yynode)->accessor1_body = __node_6;

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
            (*yynode)->accessor1_type = accessor_declarations::type_set;
            if (yytoken == Token_INTERNAL
                || yytoken == Token_PRIVATE
                || yytoken == Token_PROTECTED
                || yytoken == Token_GET
                || yytoken == Token_LBRACKET)
              {
                optional_attribute_sections_ast *__node_7 = 0;
                if (!parse_optional_attribute_sections(&__node_7))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_optional_attribute_sections, "optional_attribute_sections");
                      }
                    return false;
                  }
                (*yynode)->accessor2_attributes = __node_7;

                if (yytoken == Token_INTERNAL
                    || yytoken == Token_PRIVATE
                    || yytoken == Token_PROTECTED)
                  {
                    accessor_modifier_ast *__node_8 = 0;
                    if (!parse_accessor_modifier(&__node_8))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_accessor_modifier, "accessor_modifier");
                          }
                        return false;
                      }
                    (*yynode)->accessor2_modifier = __node_8;

                  }
                else if (true /*epsilon*/)
                {}
                else
                  {
                    return false;
                  }
                if (yytoken != Token_GET)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_GET, "get");
                      }
                    return false;
                  }
                yylex();

                if (yytoken == Token_LBRACE)
                  {
                    block_ast *__node_9 = 0;
                    if (!parse_block(&__node_9))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_block, "block");
                          }
                        return false;
                      }
                    (*yynode)->accessor2_body = __node_9;

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
                (*yynode)->accessor2_type = accessor_declarations::type_get;
              }
            else if (true /*epsilon*/)
              {
                (*yynode)->accessor2_type = accessor_declarations::type_none;
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

  bool parser::parse_accessor_modifier(accessor_modifier_ast **yynode)
  {
    *yynode = create<accessor_modifier_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_INTERNAL
        || yytoken == Token_PRIVATE
        || yytoken == Token_PROTECTED)
      {
        if (yytoken == Token_PROTECTED)
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
            if (yytoken == Token_INTERNAL)
              {
                if (yytoken != Token_INTERNAL)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_INTERNAL, "internal");
                      }
                    return false;
                  }
                yylex();

                (*yynode)->modifiers |= modifiers::mod_internal;
              }
            else if (true /*epsilon*/)
            {}
            else
              {
                return false;
              }
          }
        else if (yytoken == Token_INTERNAL)
          {
            if (yytoken != Token_INTERNAL)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_INTERNAL, "internal");
                  }
                return false;
              }
            yylex();

            (*yynode)->modifiers |= modifiers::mod_internal;
            if (yytoken == Token_PROTECTED)
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
            else if (true /*epsilon*/)
            {}
            else
              {
                return false;
              }
          }
        else if (yytoken == Token_PRIVATE)
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

  bool parser::parse_additive_expression(additive_expression_ast **yynode)
  {
    *yynode = create<additive_expression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BASE
        || yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_CHECKED
        || yytoken == Token_DECIMAL
        || yytoken == Token_DEFAULT
        || yytoken == Token_DELEGATE
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_OBJECT
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_SIZEOF
        || yytoken == Token_STRING
        || yytoken == Token_THIS
        || yytoken == Token_TYPEOF
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_UNCHECKED
        || yytoken == Token_USHORT
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_LPAREN
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_STAR
        || yytoken == Token_BIT_AND
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_REAL_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        multiplicative_expression_ast *__node_10 = 0;
        if (!parse_multiplicative_expression(&__node_10))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_multiplicative_expression, "multiplicative_expression");
              }
            return false;
          }
        (*yynode)->expression = __node_10;

        while (yytoken == Token_PLUS
               || yytoken == Token_MINUS)
          {
            additive_expression_rest_ast *__node_11 = 0;
            if (!parse_additive_expression_rest(&__node_11))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_additive_expression_rest, "additive_expression_rest");
                  }
                return false;
              }
            (*yynode)->additional_expression_sequence = snoc((*yynode)->additional_expression_sequence, __node_11, memory_pool);

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
        multiplicative_expression_ast *__node_12 = 0;
        if (!parse_multiplicative_expression(&__node_12))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_multiplicative_expression, "multiplicative_expression");
              }
            return false;
          }
        (*yynode)->expression = __node_12;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_anonymous_method_expression(anonymous_method_expression_ast **yynode)
  {
    *yynode = create<anonymous_method_expression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_DELEGATE)
      {
        if (yytoken != Token_DELEGATE)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_DELEGATE, "delegate");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_LPAREN)
          {
            anonymous_method_signature_ast *__node_13 = 0;
            if (!parse_anonymous_method_signature(&__node_13))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_anonymous_method_signature, "anonymous_method_signature");
                  }
                return false;
              }
            (*yynode)->anonymous_method_signature = __node_13;

          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        block_ast *__node_14 = 0;
        if (!parse_block(&__node_14))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_block, "block");
              }
            return false;
          }
        (*yynode)->body = __node_14;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_anonymous_method_parameter(anonymous_method_parameter_ast **yynode)
  {
    *yynode = create<anonymous_method_parameter_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DECIMAL
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_OBJECT
        || yytoken == Token_OUT
        || yytoken == Token_REF
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_STRING
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_USHORT
        || yytoken == Token_VOID
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_IDENTIFIER)
      {
        if (yytoken == Token_OUT
            || yytoken == Token_REF)
          {
            parameter_modifier_ast *__node_15 = 0;
            if (!parse_parameter_modifier(&__node_15))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_parameter_modifier, "parameter_modifier");
                  }
                return false;
              }
            (*yynode)->modifier = __node_15;

          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        type_ast *__node_16 = 0;
        if (!parse_type(&__node_16))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_type, "type");
              }
            return false;
          }
        (*yynode)->type = __node_16;

        identifier_ast *__node_17 = 0;
        if (!parse_identifier(&__node_17))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_identifier, "identifier");
              }
            return false;
          }
        (*yynode)->variable_name = __node_17;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_anonymous_method_signature(anonymous_method_signature_ast **yynode)
  {
    *yynode = create<anonymous_method_signature_ast>();

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

        if (yytoken == Token_BOOL
            || yytoken == Token_BYTE
            || yytoken == Token_CHAR
            || yytoken == Token_DECIMAL
            || yytoken == Token_DOUBLE
            || yytoken == Token_FLOAT
            || yytoken == Token_INT
            || yytoken == Token_LONG
            || yytoken == Token_OBJECT
            || yytoken == Token_OUT
            || yytoken == Token_REF
            || yytoken == Token_SBYTE
            || yytoken == Token_SHORT
            || yytoken == Token_STRING
            || yytoken == Token_UINT
            || yytoken == Token_ULONG
            || yytoken == Token_USHORT
            || yytoken == Token_VOID
            || yytoken == Token_ADD
            || yytoken == Token_ALIAS
            || yytoken == Token_GET
            || yytoken == Token_GLOBAL
            || yytoken == Token_PARTIAL
            || yytoken == Token_REMOVE
            || yytoken == Token_SET
            || yytoken == Token_VALUE
            || yytoken == Token_WHERE
            || yytoken == Token_YIELD
            || yytoken == Token_ASSEMBLY
            || yytoken == Token_IDENTIFIER)
          {
            anonymous_method_parameter_ast *__node_18 = 0;
            if (!parse_anonymous_method_parameter(&__node_18))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_anonymous_method_parameter, "anonymous_method_parameter");
                  }
                return false;
              }
            (*yynode)->anonymous_method_parameter_sequence = snoc((*yynode)->anonymous_method_parameter_sequence, __node_18, memory_pool);

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

                anonymous_method_parameter_ast *__node_19 = 0;
                if (!parse_anonymous_method_parameter(&__node_19))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_anonymous_method_parameter, "anonymous_method_parameter");
                      }
                    return false;
                  }
                (*yynode)->anonymous_method_parameter_sequence = snoc((*yynode)->anonymous_method_parameter_sequence, __node_19, memory_pool);

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

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_argument(argument_ast **yynode)
  {
    *yynode = create<argument_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BASE
        || yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_CHECKED
        || yytoken == Token_DECIMAL
        || yytoken == Token_DEFAULT
        || yytoken == Token_DELEGATE
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_OBJECT
        || yytoken == Token_OUT
        || yytoken == Token_REF
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_SIZEOF
        || yytoken == Token_STRING
        || yytoken == Token_THIS
        || yytoken == Token_TYPEOF
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_UNCHECKED
        || yytoken == Token_USHORT
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_LPAREN
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_STAR
        || yytoken == Token_BIT_AND
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_REAL_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        if (yytoken == Token_BASE
            || yytoken == Token_BOOL
            || yytoken == Token_BYTE
            || yytoken == Token_CHAR
            || yytoken == Token_CHECKED
            || yytoken == Token_DECIMAL
            || yytoken == Token_DEFAULT
            || yytoken == Token_DELEGATE
            || yytoken == Token_DOUBLE
            || yytoken == Token_FLOAT
            || yytoken == Token_INT
            || yytoken == Token_LONG
            || yytoken == Token_NEW
            || yytoken == Token_OBJECT
            || yytoken == Token_SBYTE
            || yytoken == Token_SHORT
            || yytoken == Token_SIZEOF
            || yytoken == Token_STRING
            || yytoken == Token_THIS
            || yytoken == Token_TYPEOF
            || yytoken == Token_UINT
            || yytoken == Token_ULONG
            || yytoken == Token_UNCHECKED
            || yytoken == Token_USHORT
            || yytoken == Token_ADD
            || yytoken == Token_ALIAS
            || yytoken == Token_GET
            || yytoken == Token_GLOBAL
            || yytoken == Token_PARTIAL
            || yytoken == Token_REMOVE
            || yytoken == Token_SET
            || yytoken == Token_VALUE
            || yytoken == Token_WHERE
            || yytoken == Token_YIELD
            || yytoken == Token_ASSEMBLY
            || yytoken == Token_LPAREN
            || yytoken == Token_BANG
            || yytoken == Token_TILDE
            || yytoken == Token_INCREMENT
            || yytoken == Token_DECREMENT
            || yytoken == Token_PLUS
            || yytoken == Token_MINUS
            || yytoken == Token_STAR
            || yytoken == Token_BIT_AND
            || yytoken == Token_TRUE
            || yytoken == Token_FALSE
            || yytoken == Token_NULL
            || yytoken == Token_INTEGER_LITERAL
            || yytoken == Token_REAL_LITERAL
            || yytoken == Token_CHARACTER_LITERAL
            || yytoken == Token_STRING_LITERAL
            || yytoken == Token_IDENTIFIER)
          {
            expression_ast *__node_20 = 0;
            if (!parse_expression(&__node_20))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_expression, "expression");
                  }
                return false;
              }
            (*yynode)->expression = __node_20;

            (*yynode)->argument_type = argument::type_value_parameter;
          }
        else if (yytoken == Token_REF)
          {
            if (yytoken != Token_REF)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_REF, "ref");
                  }
                return false;
              }
            yylex();

            expression_ast *__node_21 = 0;
            if (!parse_expression(&__node_21))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_expression, "expression");
                  }
                return false;
              }
            (*yynode)->expression = __node_21;

            (*yynode)->argument_type = argument::type_reference_parameter;
          }
        else if (yytoken == Token_OUT)
          {
            if (yytoken != Token_OUT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_OUT, "out");
                  }
                return false;
              }
            yylex();

            expression_ast *__node_22 = 0;
            if (!parse_expression(&__node_22))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_expression, "expression");
                  }
                return false;
              }
            (*yynode)->expression = __node_22;

            (*yynode)->argument_type = argument::type_output_parameter;
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

  bool parser::parse_array_creation_expression_rest(array_creation_expression_rest_ast **yynode, type_ast *type)
  {
    *yynode = create<array_creation_expression_rest_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    (*yynode)->type = type;
    if (type && type->start_token < (*yynode)->start_token)
      (*yynode)->start_token = type->start_token;

    if (yytoken == Token_LBRACE
        || yytoken == Token_LBRACKET)
      {
        if (yytoken == Token_LBRACE)
          {
            array_initializer_ast *__node_23 = 0;
            if (!parse_array_initializer(&__node_23))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_array_initializer, "array_initializer");
                  }
                return false;
              }
            (*yynode)->array_initializer = __node_23;

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

            expression_ast *__node_24 = 0;
            if (!parse_expression(&__node_24))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_expression, "expression");
                  }
                return false;
              }
            (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_24, memory_pool);

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

                expression_ast *__node_25 = 0;
                if (!parse_expression(&__node_25))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_expression, "expression");
                      }
                    return false;
                  }
                (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_25, memory_pool);

              }
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
                if (LA(2).kind != Token_COMMA && LA(2).kind != Token_RBRACKET)
                  {
                    break;
                  }
                rank_specifier_ast *__node_26 = 0;
                if (!parse_rank_specifier(&__node_26))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_rank_specifier, "rank_specifier");
                      }
                    return false;
                  }
                (*yynode)->rank_specifier_sequence = snoc((*yynode)->rank_specifier_sequence, __node_26, memory_pool);

              }
            if (yytoken == Token_LBRACE)
              {
                array_initializer_ast *__node_27 = 0;
                if (!parse_array_initializer(&__node_27))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_array_initializer, "array_initializer");
                      }
                    return false;
                  }
                (*yynode)->array_initializer = __node_27;

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

  bool parser::parse_array_initializer(array_initializer_ast **yynode)
  {
    *yynode = create<array_initializer_ast>();

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

        std::size_t try_start_token_1 = token_stream->index() - 1;
        parser_state *try_start_state_1 = copy_current_state();
        {
          if (yytoken == Token_BASE
              || yytoken == Token_BOOL
              || yytoken == Token_BYTE
              || yytoken == Token_CHAR
              || yytoken == Token_CHECKED
              || yytoken == Token_DECIMAL
              || yytoken == Token_DEFAULT
              || yytoken == Token_DELEGATE
              || yytoken == Token_DOUBLE
              || yytoken == Token_FLOAT
              || yytoken == Token_INT
              || yytoken == Token_LONG
              || yytoken == Token_NEW
              || yytoken == Token_OBJECT
              || yytoken == Token_SBYTE
              || yytoken == Token_SHORT
              || yytoken == Token_SIZEOF
              || yytoken == Token_STACKALLOC
              || yytoken == Token_STRING
              || yytoken == Token_THIS
              || yytoken == Token_TYPEOF
              || yytoken == Token_UINT
              || yytoken == Token_ULONG
              || yytoken == Token_UNCHECKED
              || yytoken == Token_USHORT
              || yytoken == Token_ADD
              || yytoken == Token_ALIAS
              || yytoken == Token_GET
              || yytoken == Token_GLOBAL
              || yytoken == Token_PARTIAL
              || yytoken == Token_REMOVE
              || yytoken == Token_SET
              || yytoken == Token_VALUE
              || yytoken == Token_WHERE
              || yytoken == Token_YIELD
              || yytoken == Token_ASSEMBLY
              || yytoken == Token_LPAREN
              || yytoken == Token_LBRACE
              || yytoken == Token_BANG
              || yytoken == Token_TILDE
              || yytoken == Token_INCREMENT
              || yytoken == Token_DECREMENT
              || yytoken == Token_PLUS
              || yytoken == Token_MINUS
              || yytoken == Token_STAR
              || yytoken == Token_BIT_AND
              || yytoken == Token_TRUE
              || yytoken == Token_FALSE
              || yytoken == Token_NULL
              || yytoken == Token_INTEGER_LITERAL
              || yytoken == Token_REAL_LITERAL
              || yytoken == Token_CHARACTER_LITERAL
              || yytoken == Token_STRING_LITERAL
              || yytoken == Token_IDENTIFIER)
            {
              variable_initializer_ast *__node_28 = 0;
              if (!parse_variable_initializer(&__node_28))
                {
                  goto __catch_1;
                }
              (*yynode)->variable_initializer_sequence = snoc((*yynode)->variable_initializer_sequence, __node_28, memory_pool);

              while (yytoken == Token_COMMA)
                {
                  if (LA(2).kind == Token_RBRACE)
                    {
                      break;
                    }
                  if (yytoken != Token_COMMA)
                    goto __catch_1;
                  yylex();

                  variable_initializer_ast *__node_29 = 0;
                  if (!parse_variable_initializer(&__node_29))
                    {
                      goto __catch_1;
                    }
                  (*yynode)->variable_initializer_sequence = snoc((*yynode)->variable_initializer_sequence, __node_29, memory_pool);

                }
              if (yytoken == Token_COMMA)
                {
                  if (yytoken != Token_COMMA)
                    goto __catch_1;
                  yylex();

                }
              else if (true /*epsilon*/)
              {}
              else
                {
                  goto __catch_1;
                }
            }
          else if (true /*epsilon*/)
          {}
          else
            {
              goto __catch_1;
            }
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
                   && yytoken != Token_RBRACE)
              {
                yylex();
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

  bool parser::parse_array_type(array_type_ast **yynode)
  {
    *yynode = create<array_type_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DECIMAL
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_OBJECT
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_STRING
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_USHORT
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_IDENTIFIER)
      {
        non_array_type_ast *__node_30 = 0;
        if (!parse_non_array_type(&__node_30))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_non_array_type, "non_array_type");
              }
            return false;
          }
        (*yynode)->non_array_type = __node_30;

        do
          {
            rank_specifier_ast *__node_31 = 0;
            if (!parse_rank_specifier(&__node_31))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_rank_specifier, "rank_specifier");
                  }
                return false;
              }
            (*yynode)->rank_specifier_sequence = snoc((*yynode)->rank_specifier_sequence, __node_31, memory_pool);

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

  bool parser::parse_attribute(attribute_ast **yynode)
  {
    *yynode = create<attribute_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_IDENTIFIER)
      {
        type_name_ast *__node_32 = 0;
        if (!parse_type_name(&__node_32))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_type_name, "type_name");
              }
            return false;
          }
        (*yynode)->name = __node_32;

        if (yytoken == Token_LPAREN)
          {
            attribute_arguments_ast *__node_33 = 0;
            if (!parse_attribute_arguments(&__node_33))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_attribute_arguments, "attribute_arguments");
                  }
                return false;
              }
            (*yynode)->arguments = __node_33;

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

  bool parser::parse_attribute_arguments(attribute_arguments_ast **yynode)
  {
    *yynode = create<attribute_arguments_ast>();

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

        if (yytoken == Token_RPAREN)
          {
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
        else if ((yytoken == Token_ADD
                  || yytoken == Token_ALIAS
                  || yytoken == Token_GET
                  || yytoken == Token_GLOBAL
                  || yytoken == Token_PARTIAL
                  || yytoken == Token_REMOVE
                  || yytoken == Token_SET
                  || yytoken == Token_VALUE
                  || yytoken == Token_WHERE
                  || yytoken == Token_YIELD
                  || yytoken == Token_ASSEMBLY
                  || yytoken == Token_IDENTIFIER) && ( LA(2).kind == Token_ASSIGN ))
          {
            named_argument_ast *__node_34 = 0;
            if (!parse_named_argument(&__node_34))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_named_argument, "named_argument");
                  }
                return false;
              }
            (*yynode)->named_argument_sequence = snoc((*yynode)->named_argument_sequence, __node_34, memory_pool);

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

                named_argument_ast *__node_35 = 0;
                if (!parse_named_argument(&__node_35))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_named_argument, "named_argument");
                      }
                    return false;
                  }
                (*yynode)->named_argument_sequence = snoc((*yynode)->named_argument_sequence, __node_35, memory_pool);

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
        else if (yytoken == Token_BASE
                 || yytoken == Token_BOOL
                 || yytoken == Token_BYTE
                 || yytoken == Token_CHAR
                 || yytoken == Token_CHECKED
                 || yytoken == Token_DECIMAL
                 || yytoken == Token_DEFAULT
                 || yytoken == Token_DELEGATE
                 || yytoken == Token_DOUBLE
                 || yytoken == Token_FLOAT
                 || yytoken == Token_INT
                 || yytoken == Token_LONG
                 || yytoken == Token_NEW
                 || yytoken == Token_OBJECT
                 || yytoken == Token_SBYTE
                 || yytoken == Token_SHORT
                 || yytoken == Token_SIZEOF
                 || yytoken == Token_STRING
                 || yytoken == Token_THIS
                 || yytoken == Token_TYPEOF
                 || yytoken == Token_UINT
                 || yytoken == Token_ULONG
                 || yytoken == Token_UNCHECKED
                 || yytoken == Token_USHORT
                 || yytoken == Token_ADD
                 || yytoken == Token_ALIAS
                 || yytoken == Token_GET
                 || yytoken == Token_GLOBAL
                 || yytoken == Token_PARTIAL
                 || yytoken == Token_REMOVE
                 || yytoken == Token_SET
                 || yytoken == Token_VALUE
                 || yytoken == Token_WHERE
                 || yytoken == Token_YIELD
                 || yytoken == Token_ASSEMBLY
                 || yytoken == Token_LPAREN
                 || yytoken == Token_BANG
                 || yytoken == Token_TILDE
                 || yytoken == Token_INCREMENT
                 || yytoken == Token_DECREMENT
                 || yytoken == Token_PLUS
                 || yytoken == Token_MINUS
                 || yytoken == Token_STAR
                 || yytoken == Token_BIT_AND
                 || yytoken == Token_TRUE
                 || yytoken == Token_FALSE
                 || yytoken == Token_NULL
                 || yytoken == Token_INTEGER_LITERAL
                 || yytoken == Token_REAL_LITERAL
                 || yytoken == Token_CHARACTER_LITERAL
                 || yytoken == Token_STRING_LITERAL
                 || yytoken == Token_IDENTIFIER)
          {
            positional_argument_ast *__node_36 = 0;
            if (!parse_positional_argument(&__node_36))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_positional_argument, "positional_argument");
                  }
                return false;
              }
            (*yynode)->positional_argument_sequence = snoc((*yynode)->positional_argument_sequence, __node_36, memory_pool);

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

                if ((yytoken == Token_ADD
                     || yytoken == Token_ALIAS
                     || yytoken == Token_GET
                     || yytoken == Token_GLOBAL
                     || yytoken == Token_PARTIAL
                     || yytoken == Token_REMOVE
                     || yytoken == Token_SET
                     || yytoken == Token_VALUE
                     || yytoken == Token_WHERE
                     || yytoken == Token_YIELD
                     || yytoken == Token_ASSEMBLY
                     || yytoken == Token_IDENTIFIER) && ( LA(2).kind == Token_ASSIGN ))
                  {
                    named_argument_ast *__node_37 = 0;
                    if (!parse_named_argument(&__node_37))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_named_argument, "named_argument");
                          }
                        return false;
                      }
                    (*yynode)->named_argument_sequence = snoc((*yynode)->named_argument_sequence, __node_37, memory_pool);

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

                        named_argument_ast *__node_38 = 0;
                        if (!parse_named_argument(&__node_38))
                          {
                            if (!yy_block_errors)
                              {
                                yy_expected_symbol(ast_node::Kind_named_argument, "named_argument");
                              }
                            return false;
                          }
                        (*yynode)->named_argument_sequence = snoc((*yynode)->named_argument_sequence, __node_38, memory_pool);

                      }
                    break;
                  }
                else if (yytoken == Token_BASE
                         || yytoken == Token_BOOL
                         || yytoken == Token_BYTE
                         || yytoken == Token_CHAR
                         || yytoken == Token_CHECKED
                         || yytoken == Token_DECIMAL
                         || yytoken == Token_DEFAULT
                         || yytoken == Token_DELEGATE
                         || yytoken == Token_DOUBLE
                         || yytoken == Token_FLOAT
                         || yytoken == Token_INT
                         || yytoken == Token_LONG
                         || yytoken == Token_NEW
                         || yytoken == Token_OBJECT
                         || yytoken == Token_SBYTE
                         || yytoken == Token_SHORT
                         || yytoken == Token_SIZEOF
                         || yytoken == Token_STRING
                         || yytoken == Token_THIS
                         || yytoken == Token_TYPEOF
                         || yytoken == Token_UINT
                         || yytoken == Token_ULONG
                         || yytoken == Token_UNCHECKED
                         || yytoken == Token_USHORT
                         || yytoken == Token_ADD
                         || yytoken == Token_ALIAS
                         || yytoken == Token_GET
                         || yytoken == Token_GLOBAL
                         || yytoken == Token_PARTIAL
                         || yytoken == Token_REMOVE
                         || yytoken == Token_SET
                         || yytoken == Token_VALUE
                         || yytoken == Token_WHERE
                         || yytoken == Token_YIELD
                         || yytoken == Token_ASSEMBLY
                         || yytoken == Token_LPAREN
                         || yytoken == Token_BANG
                         || yytoken == Token_TILDE
                         || yytoken == Token_INCREMENT
                         || yytoken == Token_DECREMENT
                         || yytoken == Token_PLUS
                         || yytoken == Token_MINUS
                         || yytoken == Token_STAR
                         || yytoken == Token_BIT_AND
                         || yytoken == Token_TRUE
                         || yytoken == Token_FALSE
                         || yytoken == Token_NULL
                         || yytoken == Token_INTEGER_LITERAL
                         || yytoken == Token_REAL_LITERAL
                         || yytoken == Token_CHARACTER_LITERAL
                         || yytoken == Token_STRING_LITERAL
                         || yytoken == Token_IDENTIFIER)
                  {
                    positional_argument_ast *__node_39 = 0;
                    if (!parse_positional_argument(&__node_39))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_positional_argument, "positional_argument");
                          }
                        return false;
                      }
                    (*yynode)->positional_argument_sequence = snoc((*yynode)->positional_argument_sequence, __node_39, memory_pool);

                  }
                else
                  {
                    return false;
                  }
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
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_attribute_section(attribute_section_ast **yynode)
  {
    *yynode = create<attribute_section_ast>();

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

        if ((yytoken == Token_ABSTRACT
             || yytoken == Token_AS
             || yytoken == Token_BASE
             || yytoken == Token_BOOL
             || yytoken == Token_BREAK
             || yytoken == Token_BYTE
             || yytoken == Token_CASE
             || yytoken == Token_CATCH
             || yytoken == Token_CHAR
             || yytoken == Token_CHECKED
             || yytoken == Token_CLASS
             || yytoken == Token_CONST
             || yytoken == Token_CONTINUE
             || yytoken == Token_DECIMAL
             || yytoken == Token_DEFAULT
             || yytoken == Token_DELEGATE
             || yytoken == Token_DO
             || yytoken == Token_DOUBLE
             || yytoken == Token_ELSE
             || yytoken == Token_ENUM
             || yytoken == Token_EVENT
             || yytoken == Token_EXPLICIT
             || yytoken == Token_EXTERN
             || yytoken == Token_FINALLY
             || yytoken == Token_FIXED
             || yytoken == Token_FLOAT
             || yytoken == Token_FOREACH
             || yytoken == Token_FOR
             || yytoken == Token_GOTO
             || yytoken == Token_IF
             || yytoken == Token_IMPLICIT
             || yytoken == Token_IN
             || yytoken == Token_INT
             || yytoken == Token_INTERFACE
             || yytoken == Token_INTERNAL
             || yytoken == Token_IS
             || yytoken == Token_LOCK
             || yytoken == Token_LONG
             || yytoken == Token_NAMESPACE
             || yytoken == Token_NEW
             || yytoken == Token_OBJECT
             || yytoken == Token_OPERATOR
             || yytoken == Token_OUT
             || yytoken == Token_OVERRIDE
             || yytoken == Token_PARAMS
             || yytoken == Token_PRIVATE
             || yytoken == Token_PROTECTED
             || yytoken == Token_PUBLIC
             || yytoken == Token_READONLY
             || yytoken == Token_REF
             || yytoken == Token_RETURN
             || yytoken == Token_SBYTE
             || yytoken == Token_SEALED
             || yytoken == Token_SHORT
             || yytoken == Token_SIZEOF
             || yytoken == Token_STACKALLOC
             || yytoken == Token_STATIC
             || yytoken == Token_STRING
             || yytoken == Token_STRUCT
             || yytoken == Token_SWITCH
             || yytoken == Token_THIS
             || yytoken == Token_THROW
             || yytoken == Token_TRY
             || yytoken == Token_TYPEOF
             || yytoken == Token_UINT
             || yytoken == Token_ULONG
             || yytoken == Token_UNCHECKED
             || yytoken == Token_UNSAFE
             || yytoken == Token_USHORT
             || yytoken == Token_USING
             || yytoken == Token_VIRTUAL
             || yytoken == Token_VOID
             || yytoken == Token_VOLATILE
             || yytoken == Token_WHILE
             || yytoken == Token_ADD
             || yytoken == Token_ALIAS
             || yytoken == Token_GET
             || yytoken == Token_GLOBAL
             || yytoken == Token_PARTIAL
             || yytoken == Token_REMOVE
             || yytoken == Token_SET
             || yytoken == Token_VALUE
             || yytoken == Token_WHERE
             || yytoken == Token_YIELD
             || yytoken == Token_ASSEMBLY
             || yytoken == Token_IDENTIFIER) && ( LA(2).kind == Token_COLON ))
          {
            attribute_target_ast *__node_40 = 0;
            if (!parse_attribute_target(&__node_40))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_attribute_target, "attribute_target");
                  }
                return false;
              }
            (*yynode)->target = __node_40;

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
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        attribute_ast *__node_41 = 0;
        if (!parse_attribute(&__node_41))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_attribute, "attribute");
              }
            return false;
          }
        (*yynode)->attribute_sequence = snoc((*yynode)->attribute_sequence, __node_41, memory_pool);

        while (yytoken == Token_COMMA)
          {
            if (LA(2).kind == Token_RBRACKET)
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

            attribute_ast *__node_42 = 0;
            if (!parse_attribute(&__node_42))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_attribute, "attribute");
                  }
                return false;
              }
            (*yynode)->attribute_sequence = snoc((*yynode)->attribute_sequence, __node_42, memory_pool);

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

  bool parser::parse_attribute_target(attribute_target_ast **yynode)
  {
    *yynode = create<attribute_target_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_ABSTRACT
        || yytoken == Token_AS
        || yytoken == Token_BASE
        || yytoken == Token_BOOL
        || yytoken == Token_BREAK
        || yytoken == Token_BYTE
        || yytoken == Token_CASE
        || yytoken == Token_CATCH
        || yytoken == Token_CHAR
        || yytoken == Token_CHECKED
        || yytoken == Token_CLASS
        || yytoken == Token_CONST
        || yytoken == Token_CONTINUE
        || yytoken == Token_DECIMAL
        || yytoken == Token_DEFAULT
        || yytoken == Token_DELEGATE
        || yytoken == Token_DO
        || yytoken == Token_DOUBLE
        || yytoken == Token_ELSE
        || yytoken == Token_ENUM
        || yytoken == Token_EVENT
        || yytoken == Token_EXPLICIT
        || yytoken == Token_EXTERN
        || yytoken == Token_FINALLY
        || yytoken == Token_FIXED
        || yytoken == Token_FLOAT
        || yytoken == Token_FOREACH
        || yytoken == Token_FOR
        || yytoken == Token_GOTO
        || yytoken == Token_IF
        || yytoken == Token_IMPLICIT
        || yytoken == Token_IN
        || yytoken == Token_INT
        || yytoken == Token_INTERFACE
        || yytoken == Token_INTERNAL
        || yytoken == Token_IS
        || yytoken == Token_LOCK
        || yytoken == Token_LONG
        || yytoken == Token_NAMESPACE
        || yytoken == Token_NEW
        || yytoken == Token_OBJECT
        || yytoken == Token_OPERATOR
        || yytoken == Token_OUT
        || yytoken == Token_OVERRIDE
        || yytoken == Token_PARAMS
        || yytoken == Token_PRIVATE
        || yytoken == Token_PROTECTED
        || yytoken == Token_PUBLIC
        || yytoken == Token_READONLY
        || yytoken == Token_REF
        || yytoken == Token_RETURN
        || yytoken == Token_SBYTE
        || yytoken == Token_SEALED
        || yytoken == Token_SHORT
        || yytoken == Token_SIZEOF
        || yytoken == Token_STACKALLOC
        || yytoken == Token_STATIC
        || yytoken == Token_STRING
        || yytoken == Token_STRUCT
        || yytoken == Token_SWITCH
        || yytoken == Token_THIS
        || yytoken == Token_THROW
        || yytoken == Token_TRY
        || yytoken == Token_TYPEOF
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_UNCHECKED
        || yytoken == Token_UNSAFE
        || yytoken == Token_USHORT
        || yytoken == Token_USING
        || yytoken == Token_VIRTUAL
        || yytoken == Token_VOID
        || yytoken == Token_VOLATILE
        || yytoken == Token_WHILE
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_IDENTIFIER)
      {
        if (yytoken == Token_ADD
            || yytoken == Token_ALIAS
            || yytoken == Token_GET
            || yytoken == Token_GLOBAL
            || yytoken == Token_PARTIAL
            || yytoken == Token_REMOVE
            || yytoken == Token_SET
            || yytoken == Token_VALUE
            || yytoken == Token_WHERE
            || yytoken == Token_YIELD
            || yytoken == Token_ASSEMBLY
            || yytoken == Token_IDENTIFIER)
          {
            identifier_ast *__node_43 = 0;
            if (!parse_identifier(&__node_43))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                  }
                return false;
              }
            (*yynode)->identifier = __node_43;

          }
        else if (yytoken == Token_ABSTRACT
                 || yytoken == Token_AS
                 || yytoken == Token_BASE
                 || yytoken == Token_BOOL
                 || yytoken == Token_BREAK
                 || yytoken == Token_BYTE
                 || yytoken == Token_CASE
                 || yytoken == Token_CATCH
                 || yytoken == Token_CHAR
                 || yytoken == Token_CHECKED
                 || yytoken == Token_CLASS
                 || yytoken == Token_CONST
                 || yytoken == Token_CONTINUE
                 || yytoken == Token_DECIMAL
                 || yytoken == Token_DEFAULT
                 || yytoken == Token_DELEGATE
                 || yytoken == Token_DO
                 || yytoken == Token_DOUBLE
                 || yytoken == Token_ELSE
                 || yytoken == Token_ENUM
                 || yytoken == Token_EVENT
                 || yytoken == Token_EXPLICIT
                 || yytoken == Token_EXTERN
                 || yytoken == Token_FINALLY
                 || yytoken == Token_FIXED
                 || yytoken == Token_FLOAT
                 || yytoken == Token_FOREACH
                 || yytoken == Token_FOR
                 || yytoken == Token_GOTO
                 || yytoken == Token_IF
                 || yytoken == Token_IMPLICIT
                 || yytoken == Token_IN
                 || yytoken == Token_INT
                 || yytoken == Token_INTERFACE
                 || yytoken == Token_INTERNAL
                 || yytoken == Token_IS
                 || yytoken == Token_LOCK
                 || yytoken == Token_LONG
                 || yytoken == Token_NAMESPACE
                 || yytoken == Token_NEW
                 || yytoken == Token_OBJECT
                 || yytoken == Token_OPERATOR
                 || yytoken == Token_OUT
                 || yytoken == Token_OVERRIDE
                 || yytoken == Token_PARAMS
                 || yytoken == Token_PRIVATE
                 || yytoken == Token_PROTECTED
                 || yytoken == Token_PUBLIC
                 || yytoken == Token_READONLY
                 || yytoken == Token_REF
                 || yytoken == Token_RETURN
                 || yytoken == Token_SBYTE
                 || yytoken == Token_SEALED
                 || yytoken == Token_SHORT
                 || yytoken == Token_SIZEOF
                 || yytoken == Token_STACKALLOC
                 || yytoken == Token_STATIC
                 || yytoken == Token_STRING
                 || yytoken == Token_STRUCT
                 || yytoken == Token_SWITCH
                 || yytoken == Token_THIS
                 || yytoken == Token_THROW
                 || yytoken == Token_TRY
                 || yytoken == Token_TYPEOF
                 || yytoken == Token_UINT
                 || yytoken == Token_ULONG
                 || yytoken == Token_UNCHECKED
                 || yytoken == Token_UNSAFE
                 || yytoken == Token_USHORT
                 || yytoken == Token_USING
                 || yytoken == Token_VIRTUAL
                 || yytoken == Token_VOID
                 || yytoken == Token_VOLATILE
                 || yytoken == Token_WHILE)
          {
            keyword_ast *__node_44 = 0;
            if (!parse_keyword(&__node_44))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_keyword, "keyword");
                  }
                return false;
              }
            (*yynode)->keyword = __node_44;

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

  bool parser::parse_base_access(base_access_ast **yynode)
  {
    *yynode = create<base_access_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BASE)
      {
        if (yytoken != Token_BASE)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_BASE, "base");
              }
            return false;
          }
        yylex();

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

            identifier_ast *__node_45 = 0;
            if (!parse_identifier(&__node_45))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                  }
                return false;
              }
            (*yynode)->identifier = __node_45;

            bool block_errors_2 = block_errors(true);
            std::size_t try_start_token_2 = token_stream->index() - 1;
            parser_state *try_start_state_2 = copy_current_state();
            {
              if ((yytoken == Token_LESS_THAN) && ( compatibility_mode() >= csharp20_compatibility ))
                {
                  type_arguments_ast *__node_46 = 0;
                  if (!parse_type_arguments(&__node_46))
                    {
                      goto __catch_2;
                    }
                  (*yynode)->type_arguments = __node_46;

                }
              else if (true /*epsilon*/)
              {}
              else
                {
                  goto __catch_2;
                }
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

              }

            (*yynode)->access_type = base_access::type_base_member_access;
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

            expression_ast *__node_47 = 0;
            if (!parse_expression(&__node_47))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_expression, "expression");
                  }
                return false;
              }
            (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_47, memory_pool);

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

                expression_ast *__node_48 = 0;
                if (!parse_expression(&__node_48))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_expression, "expression");
                      }
                    return false;
                  }
                (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_48, memory_pool);

              }
            if (yytoken != Token_RBRACKET)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_RBRACKET, "]");
                  }
                return false;
              }
            yylex();

            (*yynode)->access_type = base_access::type_base_indexer_access;
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

  bool parser::parse_bit_and_expression(bit_and_expression_ast **yynode)
  {
    *yynode = create<bit_and_expression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BASE
        || yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_CHECKED
        || yytoken == Token_DECIMAL
        || yytoken == Token_DEFAULT
        || yytoken == Token_DELEGATE
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_OBJECT
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_SIZEOF
        || yytoken == Token_STRING
        || yytoken == Token_THIS
        || yytoken == Token_TYPEOF
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_UNCHECKED
        || yytoken == Token_USHORT
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_LPAREN
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_STAR
        || yytoken == Token_BIT_AND
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_REAL_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        equality_expression_ast *__node_49 = 0;
        if (!parse_equality_expression(&__node_49))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_equality_expression, "equality_expression");
              }
            return false;
          }
        (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_49, memory_pool);

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

            equality_expression_ast *__node_50 = 0;
            if (!parse_equality_expression(&__node_50))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_equality_expression, "equality_expression");
                  }
                return false;
              }
            (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_50, memory_pool);

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

    if (yytoken == Token_BASE
        || yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_CHECKED
        || yytoken == Token_DECIMAL
        || yytoken == Token_DEFAULT
        || yytoken == Token_DELEGATE
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_OBJECT
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_SIZEOF
        || yytoken == Token_STRING
        || yytoken == Token_THIS
        || yytoken == Token_TYPEOF
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_UNCHECKED
        || yytoken == Token_USHORT
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_LPAREN
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_STAR
        || yytoken == Token_BIT_AND
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_REAL_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        bit_xor_expression_ast *__node_51 = 0;
        if (!parse_bit_xor_expression(&__node_51))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_bit_xor_expression, "bit_xor_expression");
              }
            return false;
          }
        (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_51, memory_pool);

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

            bit_xor_expression_ast *__node_52 = 0;
            if (!parse_bit_xor_expression(&__node_52))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_bit_xor_expression, "bit_xor_expression");
                  }
                return false;
              }
            (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_52, memory_pool);

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

    if (yytoken == Token_BASE
        || yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_CHECKED
        || yytoken == Token_DECIMAL
        || yytoken == Token_DEFAULT
        || yytoken == Token_DELEGATE
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_OBJECT
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_SIZEOF
        || yytoken == Token_STRING
        || yytoken == Token_THIS
        || yytoken == Token_TYPEOF
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_UNCHECKED
        || yytoken == Token_USHORT
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_LPAREN
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_STAR
        || yytoken == Token_BIT_AND
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_REAL_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        bit_and_expression_ast *__node_53 = 0;
        if (!parse_bit_and_expression(&__node_53))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_bit_and_expression, "bit_and_expression");
              }
            return false;
          }
        (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_53, memory_pool);

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

            bit_and_expression_ast *__node_54 = 0;
            if (!parse_bit_and_expression(&__node_54))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_bit_and_expression, "bit_and_expression");
                  }
                return false;
              }
            (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_54, memory_pool);

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

        while (yytoken == Token_BASE
               || yytoken == Token_BOOL
               || yytoken == Token_BREAK
               || yytoken == Token_BYTE
               || yytoken == Token_CHAR
               || yytoken == Token_CHECKED
               || yytoken == Token_CONST
               || yytoken == Token_CONTINUE
               || yytoken == Token_DECIMAL
               || yytoken == Token_DEFAULT
               || yytoken == Token_DELEGATE
               || yytoken == Token_DO
               || yytoken == Token_DOUBLE
               || yytoken == Token_FIXED
               || yytoken == Token_FLOAT
               || yytoken == Token_FOREACH
               || yytoken == Token_FOR
               || yytoken == Token_GOTO
               || yytoken == Token_IF
               || yytoken == Token_INT
               || yytoken == Token_LOCK
               || yytoken == Token_LONG
               || yytoken == Token_NEW
               || yytoken == Token_OBJECT
               || yytoken == Token_RETURN
               || yytoken == Token_SBYTE
               || yytoken == Token_SHORT
               || yytoken == Token_SIZEOF
               || yytoken == Token_STRING
               || yytoken == Token_SWITCH
               || yytoken == Token_THIS
               || yytoken == Token_THROW
               || yytoken == Token_TRY
               || yytoken == Token_TYPEOF
               || yytoken == Token_UINT
               || yytoken == Token_ULONG
               || yytoken == Token_UNCHECKED
               || yytoken == Token_UNSAFE
               || yytoken == Token_USHORT
               || yytoken == Token_USING
               || yytoken == Token_VOID
               || yytoken == Token_WHILE
               || yytoken == Token_ADD
               || yytoken == Token_ALIAS
               || yytoken == Token_GET
               || yytoken == Token_GLOBAL
               || yytoken == Token_PARTIAL
               || yytoken == Token_REMOVE
               || yytoken == Token_SET
               || yytoken == Token_VALUE
               || yytoken == Token_WHERE
               || yytoken == Token_YIELD
               || yytoken == Token_ASSEMBLY
               || yytoken == Token_LPAREN
               || yytoken == Token_LBRACE
               || yytoken == Token_SEMICOLON
               || yytoken == Token_BANG
               || yytoken == Token_TILDE
               || yytoken == Token_INCREMENT
               || yytoken == Token_DECREMENT
               || yytoken == Token_PLUS
               || yytoken == Token_MINUS
               || yytoken == Token_STAR
               || yytoken == Token_BIT_AND
               || yytoken == Token_TRUE
               || yytoken == Token_FALSE
               || yytoken == Token_NULL
               || yytoken == Token_INTEGER_LITERAL
               || yytoken == Token_REAL_LITERAL
               || yytoken == Token_CHARACTER_LITERAL
               || yytoken == Token_STRING_LITERAL
               || yytoken == Token_IDENTIFIER)
          {
            std::size_t try_start_token_3 = token_stream->index() - 1;
            parser_state *try_start_state_3 = copy_current_state();
            {
              block_statement_ast *__node_55 = 0;
              if (!parse_block_statement(&__node_55))
                {
                  goto __catch_3;
                }
              (*yynode)->statement_sequence = snoc((*yynode)->statement_sequence, __node_55, memory_pool);

            }
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
                if (try_start_token_3 == token_stream->index() - 1)
                  yylex();

                while (yytoken != Token_EOF
                       && yytoken != Token_BASE
                       && yytoken != Token_BOOL
                       && yytoken != Token_BREAK
                       && yytoken != Token_BYTE
                       && yytoken != Token_CHAR
                       && yytoken != Token_CHECKED
                       && yytoken != Token_CONST
                       && yytoken != Token_CONTINUE
                       && yytoken != Token_DECIMAL
                       && yytoken != Token_DEFAULT
                       && yytoken != Token_DELEGATE
                       && yytoken != Token_DO
                       && yytoken != Token_DOUBLE
                       && yytoken != Token_FIXED
                       && yytoken != Token_FLOAT
                       && yytoken != Token_FOREACH
                       && yytoken != Token_FOR
                       && yytoken != Token_GOTO
                       && yytoken != Token_IF
                       && yytoken != Token_INT
                       && yytoken != Token_LOCK
                       && yytoken != Token_LONG
                       && yytoken != Token_NEW
                       && yytoken != Token_OBJECT
                       && yytoken != Token_RETURN
                       && yytoken != Token_SBYTE
                       && yytoken != Token_SHORT
                       && yytoken != Token_SIZEOF
                       && yytoken != Token_STRING
                       && yytoken != Token_SWITCH
                       && yytoken != Token_THIS
                       && yytoken != Token_THROW
                       && yytoken != Token_TRY
                       && yytoken != Token_TYPEOF
                       && yytoken != Token_UINT
                       && yytoken != Token_ULONG
                       && yytoken != Token_UNCHECKED
                       && yytoken != Token_UNSAFE
                       && yytoken != Token_USHORT
                       && yytoken != Token_USING
                       && yytoken != Token_VOID
                       && yytoken != Token_WHILE
                       && yytoken != Token_ADD
                       && yytoken != Token_ALIAS
                       && yytoken != Token_GET
                       && yytoken != Token_GLOBAL
                       && yytoken != Token_PARTIAL
                       && yytoken != Token_REMOVE
                       && yytoken != Token_SET
                       && yytoken != Token_VALUE
                       && yytoken != Token_WHERE
                       && yytoken != Token_YIELD
                       && yytoken != Token_ASSEMBLY
                       && yytoken != Token_LPAREN
                       && yytoken != Token_LBRACE
                       && yytoken != Token_RBRACE
                       && yytoken != Token_SEMICOLON
                       && yytoken != Token_BANG
                       && yytoken != Token_TILDE
                       && yytoken != Token_INCREMENT
                       && yytoken != Token_DECREMENT
                       && yytoken != Token_PLUS
                       && yytoken != Token_MINUS
                       && yytoken != Token_STAR
                       && yytoken != Token_BIT_AND
                       && yytoken != Token_TRUE
                       && yytoken != Token_FALSE
                       && yytoken != Token_NULL
                       && yytoken != Token_INTEGER_LITERAL
                       && yytoken != Token_REAL_LITERAL
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

    if (yytoken == Token_BASE
        || yytoken == Token_BOOL
        || yytoken == Token_BREAK
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_CHECKED
        || yytoken == Token_CONST
        || yytoken == Token_CONTINUE
        || yytoken == Token_DECIMAL
        || yytoken == Token_DEFAULT
        || yytoken == Token_DELEGATE
        || yytoken == Token_DO
        || yytoken == Token_DOUBLE
        || yytoken == Token_FIXED
        || yytoken == Token_FLOAT
        || yytoken == Token_FOREACH
        || yytoken == Token_FOR
        || yytoken == Token_GOTO
        || yytoken == Token_IF
        || yytoken == Token_INT
        || yytoken == Token_LOCK
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_OBJECT
        || yytoken == Token_RETURN
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_SIZEOF
        || yytoken == Token_STRING
        || yytoken == Token_SWITCH
        || yytoken == Token_THIS
        || yytoken == Token_THROW
        || yytoken == Token_TRY
        || yytoken == Token_TYPEOF
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_UNCHECKED
        || yytoken == Token_UNSAFE
        || yytoken == Token_USHORT
        || yytoken == Token_USING
        || yytoken == Token_VOID
        || yytoken == Token_WHILE
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_LPAREN
        || yytoken == Token_LBRACE
        || yytoken == Token_SEMICOLON
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_STAR
        || yytoken == Token_BIT_AND
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_REAL_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        if ((yytoken == Token_ADD
             || yytoken == Token_ALIAS
             || yytoken == Token_GET
             || yytoken == Token_GLOBAL
             || yytoken == Token_PARTIAL
             || yytoken == Token_REMOVE
             || yytoken == Token_SET
             || yytoken == Token_VALUE
             || yytoken == Token_WHERE
             || yytoken == Token_YIELD
             || yytoken == Token_ASSEMBLY
             || yytoken == Token_IDENTIFIER) && ( LA(2).kind == Token_COLON ))
          {
            labeled_statement_ast *__node_56 = 0;
            if (!parse_labeled_statement(&__node_56))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_labeled_statement, "labeled_statement");
                  }
                return false;
              }
            (*yynode)->labeled_statement = __node_56;

          }
        else if (yytoken == Token_CONST)
          {
            local_constant_declaration_ast *__node_57 = 0;
            if (!parse_local_constant_declaration(&__node_57))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_local_constant_declaration, "local_constant_declaration");
                  }
                return false;
              }
            (*yynode)->local_constant_declaration_statement = __node_57;

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
        else if (yytoken == Token_BASE
                 || yytoken == Token_BOOL
                 || yytoken == Token_BREAK
                 || yytoken == Token_BYTE
                 || yytoken == Token_CHAR
                 || yytoken == Token_CHECKED
                 || yytoken == Token_CONTINUE
                 || yytoken == Token_DECIMAL
                 || yytoken == Token_DEFAULT
                 || yytoken == Token_DELEGATE
                 || yytoken == Token_DO
                 || yytoken == Token_DOUBLE
                 || yytoken == Token_FIXED
                 || yytoken == Token_FLOAT
                 || yytoken == Token_FOREACH
                 || yytoken == Token_FOR
                 || yytoken == Token_GOTO
                 || yytoken == Token_IF
                 || yytoken == Token_INT
                 || yytoken == Token_LOCK
                 || yytoken == Token_LONG
                 || yytoken == Token_NEW
                 || yytoken == Token_OBJECT
                 || yytoken == Token_RETURN
                 || yytoken == Token_SBYTE
                 || yytoken == Token_SHORT
                 || yytoken == Token_SIZEOF
                 || yytoken == Token_STRING
                 || yytoken == Token_SWITCH
                 || yytoken == Token_THIS
                 || yytoken == Token_THROW
                 || yytoken == Token_TRY
                 || yytoken == Token_TYPEOF
                 || yytoken == Token_UINT
                 || yytoken == Token_ULONG
                 || yytoken == Token_UNCHECKED
                 || yytoken == Token_UNSAFE
                 || yytoken == Token_USHORT
                 || yytoken == Token_USING
                 || yytoken == Token_VOID
                 || yytoken == Token_WHILE
                 || yytoken == Token_ADD
                 || yytoken == Token_ALIAS
                 || yytoken == Token_GET
                 || yytoken == Token_GLOBAL
                 || yytoken == Token_PARTIAL
                 || yytoken == Token_REMOVE
                 || yytoken == Token_SET
                 || yytoken == Token_VALUE
                 || yytoken == Token_WHERE
                 || yytoken == Token_YIELD
                 || yytoken == Token_ASSEMBLY
                 || yytoken == Token_LPAREN
                 || yytoken == Token_LBRACE
                 || yytoken == Token_SEMICOLON
                 || yytoken == Token_BANG
                 || yytoken == Token_TILDE
                 || yytoken == Token_INCREMENT
                 || yytoken == Token_DECREMENT
                 || yytoken == Token_PLUS
                 || yytoken == Token_MINUS
                 || yytoken == Token_STAR
                 || yytoken == Token_BIT_AND
                 || yytoken == Token_TRUE
                 || yytoken == Token_FALSE
                 || yytoken == Token_NULL
                 || yytoken == Token_INTEGER_LITERAL
                 || yytoken == Token_REAL_LITERAL
                 || yytoken == Token_CHARACTER_LITERAL
                 || yytoken == Token_STRING_LITERAL
                 || yytoken == Token_IDENTIFIER)
          {
            bool block_errors_4 = block_errors(true);
            std::size_t try_start_token_4 = token_stream->index() - 1;
            parser_state *try_start_state_4 = copy_current_state();
            {
              local_variable_declaration_statement_ast *__node_58 = 0;
              if (!parse_local_variable_declaration_statement(&__node_58))
                {
                  goto __catch_4;
                }
              (*yynode)->local_variable_declaration_statement = __node_58;

            }
            block_errors(block_errors_4);
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
                block_errors(block_errors_4);
                rewind(try_start_token_4);

                embedded_statement_ast *__node_59 = 0;
                if (!parse_embedded_statement(&__node_59))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_embedded_statement, "embedded_statement");
                      }
                    return false;
                  }
                (*yynode)->statement = __node_59;

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

  bool parser::parse_boolean_expression(boolean_expression_ast **yynode)
  {
    *yynode = create<boolean_expression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BASE
        || yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_CHECKED
        || yytoken == Token_DECIMAL
        || yytoken == Token_DEFAULT
        || yytoken == Token_DELEGATE
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_OBJECT
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_SIZEOF
        || yytoken == Token_STRING
        || yytoken == Token_THIS
        || yytoken == Token_TYPEOF
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_UNCHECKED
        || yytoken == Token_USHORT
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_LPAREN
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_STAR
        || yytoken == Token_BIT_AND
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_REAL_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        expression_ast *__node_60 = 0;
        if (!parse_expression(&__node_60))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_expression, "expression");
              }
            return false;
          }
        (*yynode)->expression = __node_60;

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

  bool parser::parse_builtin_class_type(builtin_class_type_ast **yynode)
  {
    *yynode = create<builtin_class_type_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_OBJECT
        || yytoken == Token_STRING)
      {
        if (yytoken == Token_OBJECT)
          {
            if (yytoken != Token_OBJECT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_OBJECT, "object");
                  }
                return false;
              }
            yylex();

            (*yynode)->type = builtin_class_type::type_object;
          }
        else if (yytoken == Token_STRING)
          {
            if (yytoken != Token_STRING)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_STRING, "string");
                  }
                return false;
              }
            yylex();

            (*yynode)->type = builtin_class_type::type_string;
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

        type_ast *__node_61 = 0;
        if (!parse_type(&__node_61))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_type, "type");
              }
            return false;
          }
        (*yynode)->type = __node_61;

        if (yytoken != Token_RPAREN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_RPAREN, ")");
              }
            return false;
          }
        yylex();

        unary_expression_ast *__node_62 = 0;
        if (!parse_unary_expression(&__node_62))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_unary_expression, "unary_expression");
              }
            return false;
          }
        (*yynode)->casted_expression = __node_62;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_catch_clauses(catch_clauses_ast **yynode)
  {
    *yynode = create<catch_clauses_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_CATCH)
      {
        if ((yytoken == Token_CATCH) && ( LA(2).kind != Token_LPAREN ))
          {
            general_catch_clause_ast *__node_63 = 0;
            if (!parse_general_catch_clause(&__node_63))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_general_catch_clause, "general_catch_clause");
                  }
                return false;
              }
            (*yynode)->general_catch_clause = __node_63;

          }
        else if (yytoken == Token_CATCH)
          {
            do
              {
                if (LA(2).kind != Token_LPAREN)
                  {
                    break;
                  }
                specific_catch_clause_ast *__node_64 = 0;
                if (!parse_specific_catch_clause(&__node_64))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_specific_catch_clause, "specific_catch_clause");
                      }
                    return false;
                  }
                (*yynode)->specific_catch_clause_sequence = snoc((*yynode)->specific_catch_clause_sequence, __node_64, memory_pool);

              }
            while (yytoken == Token_CATCH);
            if (yytoken == Token_CATCH)
              {
                general_catch_clause_ast *__node_65 = 0;
                if (!parse_general_catch_clause(&__node_65))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_general_catch_clause, "general_catch_clause");
                      }
                    return false;
                  }
                (*yynode)->general_catch_clause = __node_65;

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

  bool parser::parse_checked_statement(checked_statement_ast **yynode)
  {
    *yynode = create<checked_statement_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_CHECKED)
      {
        if (yytoken != Token_CHECKED)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_CHECKED, "checked");
              }
            return false;
          }
        yylex();

        block_ast *__node_66 = 0;
        if (!parse_block(&__node_66))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_block, "block");
              }
            return false;
          }
        (*yynode)->body = __node_66;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_class_base(class_base_ast **yynode)
  {
    *yynode = create<class_base_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

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

        if (yytoken == Token_OBJECT
            || yytoken == Token_STRING)
          {
            builtin_class_type_ast *__node_67 = 0;
            if (!parse_builtin_class_type(&__node_67))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_builtin_class_type, "builtin_class_type");
                  }
                return false;
              }
            (*yynode)->builtin_class_type = __node_67;

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

                type_name_ast *__node_68 = 0;
                if (!parse_type_name(&__node_68))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_type_name, "type_name");
                      }
                    return false;
                  }
                (*yynode)->interface_type_sequence = snoc((*yynode)->interface_type_sequence, __node_68, memory_pool);

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

                    type_name_ast *__node_69 = 0;
                    if (!parse_type_name(&__node_69))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_type_name, "type_name");
                          }
                        return false;
                      }
                    (*yynode)->interface_type_sequence = snoc((*yynode)->interface_type_sequence, __node_69, memory_pool);

                  }
              }
            else if (true /*epsilon*/)
            {}
            else
              {
                return false;
              }
          }
        else if (yytoken == Token_ADD
                 || yytoken == Token_ALIAS
                 || yytoken == Token_GET
                 || yytoken == Token_GLOBAL
                 || yytoken == Token_PARTIAL
                 || yytoken == Token_REMOVE
                 || yytoken == Token_SET
                 || yytoken == Token_VALUE
                 || yytoken == Token_WHERE
                 || yytoken == Token_YIELD
                 || yytoken == Token_ASSEMBLY
                 || yytoken == Token_IDENTIFIER)
          {
            type_name_ast *__node_70 = 0;
            if (!parse_type_name(&__node_70))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_type_name, "type_name");
                  }
                return false;
              }
            (*yynode)->base_type_sequence = snoc((*yynode)->base_type_sequence, __node_70, memory_pool);

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

                type_name_ast *__node_71 = 0;
                if (!parse_type_name(&__node_71))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_type_name, "type_name");
                      }
                    return false;
                  }
                (*yynode)->base_type_sequence = snoc((*yynode)->base_type_sequence, __node_71, memory_pool);

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
               || yytoken == Token_BOOL
               || yytoken == Token_BYTE
               || yytoken == Token_CHAR
               || yytoken == Token_CLASS
               || yytoken == Token_CONST
               || yytoken == Token_DECIMAL
               || yytoken == Token_DELEGATE
               || yytoken == Token_DOUBLE
               || yytoken == Token_ENUM
               || yytoken == Token_EVENT
               || yytoken == Token_EXPLICIT
               || yytoken == Token_EXTERN
               || yytoken == Token_FIXED
               || yytoken == Token_FLOAT
               || yytoken == Token_IMPLICIT
               || yytoken == Token_INT
               || yytoken == Token_INTERFACE
               || yytoken == Token_INTERNAL
               || yytoken == Token_LONG
               || yytoken == Token_NEW
               || yytoken == Token_OBJECT
               || yytoken == Token_OVERRIDE
               || yytoken == Token_PRIVATE
               || yytoken == Token_PROTECTED
               || yytoken == Token_PUBLIC
               || yytoken == Token_READONLY
               || yytoken == Token_SBYTE
               || yytoken == Token_SEALED
               || yytoken == Token_SHORT
               || yytoken == Token_STATIC
               || yytoken == Token_STRING
               || yytoken == Token_STRUCT
               || yytoken == Token_UINT
               || yytoken == Token_ULONG
               || yytoken == Token_UNSAFE
               || yytoken == Token_USHORT
               || yytoken == Token_VIRTUAL
               || yytoken == Token_VOID
               || yytoken == Token_VOLATILE
               || yytoken == Token_ADD
               || yytoken == Token_ALIAS
               || yytoken == Token_GET
               || yytoken == Token_GLOBAL
               || yytoken == Token_PARTIAL
               || yytoken == Token_REMOVE
               || yytoken == Token_SET
               || yytoken == Token_VALUE
               || yytoken == Token_WHERE
               || yytoken == Token_YIELD
               || yytoken == Token_ASSEMBLY
               || yytoken == Token_LBRACKET
               || yytoken == Token_TILDE
               || yytoken == Token_IDENTIFIER)
          {
            std::size_t try_start_token_5 = token_stream->index() - 1;
            parser_state *try_start_state_5 = copy_current_state();
            {
              class_member_declaration_ast *__node_72 = 0;
              if (!parse_class_member_declaration(&__node_72))
                {
                  goto __catch_5;
                }
              (*yynode)->member_declaration_sequence = snoc((*yynode)->member_declaration_sequence, __node_72, memory_pool);

            }
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
                if (try_start_token_5 == token_stream->index() - 1)
                  yylex();

                while (yytoken != Token_EOF
                       && yytoken != Token_ABSTRACT
                       && yytoken != Token_BOOL
                       && yytoken != Token_BYTE
                       && yytoken != Token_CHAR
                       && yytoken != Token_CLASS
                       && yytoken != Token_CONST
                       && yytoken != Token_DECIMAL
                       && yytoken != Token_DELEGATE
                       && yytoken != Token_DOUBLE
                       && yytoken != Token_ENUM
                       && yytoken != Token_EVENT
                       && yytoken != Token_EXPLICIT
                       && yytoken != Token_EXTERN
                       && yytoken != Token_FIXED
                       && yytoken != Token_FLOAT
                       && yytoken != Token_IMPLICIT
                       && yytoken != Token_INT
                       && yytoken != Token_INTERFACE
                       && yytoken != Token_INTERNAL
                       && yytoken != Token_LONG
                       && yytoken != Token_NEW
                       && yytoken != Token_OBJECT
                       && yytoken != Token_OVERRIDE
                       && yytoken != Token_PRIVATE
                       && yytoken != Token_PROTECTED
                       && yytoken != Token_PUBLIC
                       && yytoken != Token_READONLY
                       && yytoken != Token_SBYTE
                       && yytoken != Token_SEALED
                       && yytoken != Token_SHORT
                       && yytoken != Token_STATIC
                       && yytoken != Token_STRING
                       && yytoken != Token_STRUCT
                       && yytoken != Token_UINT
                       && yytoken != Token_ULONG
                       && yytoken != Token_UNSAFE
                       && yytoken != Token_USHORT
                       && yytoken != Token_VIRTUAL
                       && yytoken != Token_VOID
                       && yytoken != Token_VOLATILE
                       && yytoken != Token_ADD
                       && yytoken != Token_ALIAS
                       && yytoken != Token_GET
                       && yytoken != Token_GLOBAL
                       && yytoken != Token_PARTIAL
                       && yytoken != Token_REMOVE
                       && yytoken != Token_SET
                       && yytoken != Token_VALUE
                       && yytoken != Token_WHERE
                       && yytoken != Token_YIELD
                       && yytoken != Token_ASSEMBLY
                       && yytoken != Token_RBRACE
                       && yytoken != Token_LBRACKET
                       && yytoken != Token_TILDE
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

  bool parser::parse_class_declaration(class_declaration_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers, bool partial)
  {
    *yynode = create<class_declaration_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    (*yynode)->attributes = attributes;
    if (attributes && attributes->start_token < (*yynode)->start_token)
      (*yynode)->start_token = attributes->start_token;

    (*yynode)->modifiers = modifiers;
    if (modifiers && modifiers->start_token < (*yynode)->start_token)
      (*yynode)->start_token = modifiers->start_token;

    (*yynode)->partial = partial;
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

        identifier_ast *__node_73 = 0;
        if (!parse_identifier(&__node_73))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_identifier, "identifier");
              }
            return false;
          }
        (*yynode)->class_name = __node_73;

        if ((yytoken == Token_LESS_THAN) && ( compatibility_mode() >= csharp20_compatibility ))
          {
            std::size_t try_start_token_6 = token_stream->index() - 1;
            parser_state *try_start_state_6 = copy_current_state();
            {
              type_parameters_ast *__node_74 = 0;
              if (!parse_type_parameters(&__node_74))
                {
                  goto __catch_6;
                }
              (*yynode)->type_parameters = __node_74;

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
                       && yytoken != Token_WHERE
                       && yytoken != Token_LBRACE
                       && yytoken != Token_COLON)
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
        if (yytoken == Token_COLON)
          {
            class_base_ast *__node_75 = 0;
            if (!parse_class_base(&__node_75))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_class_base, "class_base");
                  }
                return false;
              }
            (*yynode)->class_base = __node_75;

          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        if ((yytoken == Token_WHERE) && ( compatibility_mode() >= csharp20_compatibility ))
          {
            do
              {
                std::size_t try_start_token_7 = token_stream->index() - 1;
                parser_state *try_start_state_7 = copy_current_state();
                {
                  type_parameter_constraints_clause_ast *__node_76 = 0;
                  if (!parse_type_parameter_constraints_clause(&__node_76))
                    {
                      goto __catch_7;
                    }
                  (*yynode)->type_parameter_constraints_sequence = snoc((*yynode)->type_parameter_constraints_sequence, __node_76, memory_pool);

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
                           && yytoken != Token_WHERE
                           && yytoken != Token_LBRACE)
                      {
                        yylex();
                      }
                  }

              }
            while (yytoken == Token_WHERE);
          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        class_body_ast *__node_77 = 0;
        if (!parse_class_body(&__node_77))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_class_body, "class_body");
              }
            return false;
          }
        (*yynode)->body = __node_77;

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

  bool parser::parse_class_member_declaration(class_member_declaration_ast **yynode)
  {
    *yynode = create<class_member_declaration_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_ABSTRACT
        || yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_CLASS
        || yytoken == Token_CONST
        || yytoken == Token_DECIMAL
        || yytoken == Token_DELEGATE
        || yytoken == Token_DOUBLE
        || yytoken == Token_ENUM
        || yytoken == Token_EVENT
        || yytoken == Token_EXPLICIT
        || yytoken == Token_EXTERN
        || yytoken == Token_FIXED
        || yytoken == Token_FLOAT
        || yytoken == Token_IMPLICIT
        || yytoken == Token_INT
        || yytoken == Token_INTERFACE
        || yytoken == Token_INTERNAL
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_OBJECT
        || yytoken == Token_OVERRIDE
        || yytoken == Token_PRIVATE
        || yytoken == Token_PROTECTED
        || yytoken == Token_PUBLIC
        || yytoken == Token_READONLY
        || yytoken == Token_SBYTE
        || yytoken == Token_SEALED
        || yytoken == Token_SHORT
        || yytoken == Token_STATIC
        || yytoken == Token_STRING
        || yytoken == Token_STRUCT
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_UNSAFE
        || yytoken == Token_USHORT
        || yytoken == Token_VIRTUAL
        || yytoken == Token_VOID
        || yytoken == Token_VOLATILE
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_LBRACKET
        || yytoken == Token_TILDE
        || yytoken == Token_IDENTIFIER)
      {
        optional_attribute_sections_ast *attributes = 0;

        optional_modifiers_ast *modifiers = 0;

        optional_attribute_sections_ast *__node_78 = 0;
        if (!parse_optional_attribute_sections(&__node_78))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_optional_attribute_sections, "optional_attribute_sections");
              }
            return false;
          }
        attributes = __node_78;

        optional_modifiers_ast *__node_79 = 0;
        if (!parse_optional_modifiers(&__node_79))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_optional_modifiers, "optional_modifiers");
              }
            return false;
          }
        modifiers = __node_79;

        if (yytoken == Token_TILDE)
          {
            finalizer_declaration_ast *__node_80 = 0;
            if (!parse_finalizer_declaration(&__node_80, attributes, modifiers ))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_finalizer_declaration, "finalizer_declaration");
                  }
                return false;
              }
            (*yynode)->finalizer_declaration = __node_80;

          }
        else if (yytoken == Token_BOOL
                 || yytoken == Token_BYTE
                 || yytoken == Token_CHAR
                 || yytoken == Token_CLASS
                 || yytoken == Token_CONST
                 || yytoken == Token_DECIMAL
                 || yytoken == Token_DELEGATE
                 || yytoken == Token_DOUBLE
                 || yytoken == Token_ENUM
                 || yytoken == Token_EVENT
                 || yytoken == Token_EXPLICIT
                 || yytoken == Token_FLOAT
                 || yytoken == Token_IMPLICIT
                 || yytoken == Token_INT
                 || yytoken == Token_INTERFACE
                 || yytoken == Token_LONG
                 || yytoken == Token_OBJECT
                 || yytoken == Token_SBYTE
                 || yytoken == Token_SHORT
                 || yytoken == Token_STRING
                 || yytoken == Token_STRUCT
                 || yytoken == Token_UINT
                 || yytoken == Token_ULONG
                 || yytoken == Token_USHORT
                 || yytoken == Token_VOID
                 || yytoken == Token_ADD
                 || yytoken == Token_ALIAS
                 || yytoken == Token_GET
                 || yytoken == Token_GLOBAL
                 || yytoken == Token_PARTIAL
                 || yytoken == Token_REMOVE
                 || yytoken == Token_SET
                 || yytoken == Token_VALUE
                 || yytoken == Token_WHERE
                 || yytoken == Token_YIELD
                 || yytoken == Token_ASSEMBLY
                 || yytoken == Token_IDENTIFIER)
          {
            class_or_struct_member_declaration_ast *__node_81 = 0;
            if (!parse_class_or_struct_member_declaration(&__node_81, attributes, modifiers ))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_class_or_struct_member_declaration, "class_or_struct_member_declaration");
                  }
                return false;
              }
            (*yynode)->other_declaration = __node_81;

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

  bool parser::parse_class_or_struct_member_declaration(class_or_struct_member_declaration_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers)
  {
    *yynode = create<class_or_struct_member_declaration_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_CLASS
        || yytoken == Token_CONST
        || yytoken == Token_DECIMAL
        || yytoken == Token_DELEGATE
        || yytoken == Token_DOUBLE
        || yytoken == Token_ENUM
        || yytoken == Token_EVENT
        || yytoken == Token_EXPLICIT
        || yytoken == Token_FLOAT
        || yytoken == Token_IMPLICIT
        || yytoken == Token_INT
        || yytoken == Token_INTERFACE
        || yytoken == Token_LONG
        || yytoken == Token_OBJECT
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_STRING
        || yytoken == Token_STRUCT
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_USHORT
        || yytoken == Token_VOID
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_IDENTIFIER)
      {
        return_type_ast *member_type = 0;

        const list_node<variable_declarator_ast *> *variable_declarator_sequence = 0;

        type_name_safe_ast *member_name_or_interface_type = 0;

        if (yytoken == Token_CONST)
          {
            constant_declaration_ast *__node_82 = 0;
            if (!parse_constant_declaration(&__node_82, attributes, modifiers ))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_constant_declaration, "constant_declaration");
                  }
                return false;
              }
            if (__node_82->start_token < (*yynode)->start_token)
              (*yynode)->start_token = __node_82->start_token;
            (*yynode)->constant_declaration = __node_82;

          }
        else if (yytoken == Token_EVENT)
          {
            event_declaration_ast *__node_83 = 0;
            if (!parse_event_declaration(&__node_83, attributes, modifiers ))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_event_declaration, "event_declaration");
                  }
                return false;
              }
            if (__node_83->start_token < (*yynode)->start_token)
              (*yynode)->start_token = __node_83->start_token;
            (*yynode)->event_declaration = __node_83;

          }
        else if (yytoken == Token_EXPLICIT
                 || yytoken == Token_IMPLICIT)
          {
            conversion_operator_declaration_ast *__node_84 = 0;
            if (!parse_conversion_operator_declaration(&__node_84, attributes, modifiers ))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_conversion_operator_declaration, "conversion_operator_declaration");
                  }
                return false;
              }
            if (__node_84->start_token < (*yynode)->start_token)
              (*yynode)->start_token = __node_84->start_token;
            (*yynode)->conversion_operator_declaration = __node_84;

          }
        else if ((yytoken == Token_ADD
                  || yytoken == Token_ALIAS
                  || yytoken == Token_GET
                  || yytoken == Token_GLOBAL
                  || yytoken == Token_PARTIAL
                  || yytoken == Token_REMOVE
                  || yytoken == Token_SET
                  || yytoken == Token_VALUE
                  || yytoken == Token_WHERE
                  || yytoken == Token_YIELD
                  || yytoken == Token_ASSEMBLY
                  || yytoken == Token_IDENTIFIER) && ( LA(2).kind == Token_LPAREN ))
          {
            constructor_declaration_ast *__node_85 = 0;
            if (!parse_constructor_declaration(&__node_85, attributes, modifiers ))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_constructor_declaration, "constructor_declaration");
                  }
                return false;
              }
            if (__node_85->start_token < (*yynode)->start_token)
              (*yynode)->start_token = __node_85->start_token;
            (*yynode)->constructor_declaration = __node_85;

          }
        else if ((yytoken == Token_CLASS
                  || yytoken == Token_DELEGATE
                  || yytoken == Token_ENUM
                  || yytoken == Token_INTERFACE
                  || yytoken == Token_STRUCT
                  || yytoken == Token_PARTIAL) && ( (yytoken != Token_PARTIAL) || (LA(2).kind == Token_CLASS
                                                    || LA(2).kind == Token_INTERFACE || LA(2).kind == Token_ENUM
                                                    || LA(2).kind == Token_STRUCT || LA(2).kind == Token_DELEGATE) ))
          {
            type_declaration_rest_ast *__node_86 = 0;
            if (!parse_type_declaration_rest(&__node_86, attributes, modifiers ))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_type_declaration_rest, "type_declaration_rest");
                  }
                return false;
              }
            if (__node_86->start_token < (*yynode)->start_token)
              (*yynode)->start_token = __node_86->start_token;
            (*yynode)->type_declaration_rest = __node_86;

          }
        else if (yytoken == Token_BOOL
                 || yytoken == Token_BYTE
                 || yytoken == Token_CHAR
                 || yytoken == Token_DECIMAL
                 || yytoken == Token_DOUBLE
                 || yytoken == Token_FLOAT
                 || yytoken == Token_INT
                 || yytoken == Token_LONG
                 || yytoken == Token_OBJECT
                 || yytoken == Token_SBYTE
                 || yytoken == Token_SHORT
                 || yytoken == Token_STRING
                 || yytoken == Token_UINT
                 || yytoken == Token_ULONG
                 || yytoken == Token_USHORT
                 || yytoken == Token_VOID
                 || yytoken == Token_ADD
                 || yytoken == Token_ALIAS
                 || yytoken == Token_GET
                 || yytoken == Token_GLOBAL
                 || yytoken == Token_PARTIAL
                 || yytoken == Token_REMOVE
                 || yytoken == Token_SET
                 || yytoken == Token_VALUE
                 || yytoken == Token_WHERE
                 || yytoken == Token_YIELD
                 || yytoken == Token_ASSEMBLY
                 || yytoken == Token_IDENTIFIER)
          {
            return_type_ast *__node_87 = 0;
            if (!parse_return_type(&__node_87))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_return_type, "return_type");
                  }
                return false;
              }
            member_type = __node_87;

            if ((yytoken == Token_OPERATOR) && ( member_type->type == return_type::type_regular ))
              {
                unary_or_binary_operator_declaration_ast *__node_88 = 0;
                if (!parse_unary_or_binary_operator_declaration(&__node_88,
                    attributes, modifiers, member_type->regular_type
                                                               ))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_unary_or_binary_operator_declaration, "unary_or_binary_operator_declaration");
                      }
                    return false;
                  }
                if (__node_88->start_token < (*yynode)->start_token)
                  (*yynode)->start_token = __node_88->start_token;
                (*yynode)->unary_or_binary_operator_declaration = __node_88;

              }
            else if ((yytoken == Token_THIS) && ( member_type->type == return_type::type_regular ))
              {
                indexer_declaration_ast *__node_89 = 0;
                if (!parse_indexer_declaration(&__node_89,
                                               attributes, modifiers, member_type->regular_type, 0 /* no interface type */
                                              ))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_indexer_declaration, "indexer_declaration");
                      }
                    return false;
                  }
                if (__node_89->start_token < (*yynode)->start_token)
                  (*yynode)->start_token = __node_89->start_token;
                (*yynode)->indexer_declaration = __node_89;

              }
            else if ((yytoken == Token_ADD
                      || yytoken == Token_ALIAS
                      || yytoken == Token_GET
                      || yytoken == Token_GLOBAL
                      || yytoken == Token_PARTIAL
                      || yytoken == Token_REMOVE
                      || yytoken == Token_SET
                      || yytoken == Token_VALUE
                      || yytoken == Token_WHERE
                      || yytoken == Token_YIELD
                      || yytoken == Token_ASSEMBLY
                      || yytoken == Token_IDENTIFIER) && ( ( LA(2).kind == Token_SEMICOLON || LA(2).kind == Token_ASSIGN
                                                             || LA(2).kind == Token_COMMA || LA(2).kind == Token_LBRACKET
                                                           ) && (member_type->type == return_type::type_regular) ))
              {
                variable_declarator_ast *__node_90 = 0;
                if (!parse_variable_declarator(&__node_90,
                                               ((modifiers->modifiers & modifiers::mod_fixed) != 0) /* is a fixed size buffer? */
                                              ))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_variable_declarator, "variable_declarator");
                      }
                    return false;
                  }
                variable_declarator_sequence = snoc(variable_declarator_sequence, __node_90, memory_pool);

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

                    variable_declarator_ast *__node_91 = 0;
                    if (!parse_variable_declarator(&__node_91,
                                                   ((modifiers->modifiers & modifiers::mod_fixed) != 0) /* is a fixed size buffer? */
                                                  ))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_variable_declarator, "variable_declarator");
                          }
                        return false;
                      }
                    variable_declarator_sequence = snoc(variable_declarator_sequence, __node_91, memory_pool);

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

                variable_declaration_data_ast *__node_92 = 0;
                if (!parse_variable_declaration_data(&__node_92,
                                                     attributes, modifiers,
                                                     member_type->regular_type, variable_declarator_sequence
                                                    ))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_variable_declaration_data, "variable_declaration_data");
                      }
                    return false;
                  }
                if (__node_92->start_token < (*yynode)->start_token)
                  (*yynode)->start_token = __node_92->start_token;
                (*yynode)->field_declaration = __node_92;

              }
            else if (yytoken == Token_ADD
                     || yytoken == Token_ALIAS
                     || yytoken == Token_GET
                     || yytoken == Token_GLOBAL
                     || yytoken == Token_PARTIAL
                     || yytoken == Token_REMOVE
                     || yytoken == Token_SET
                     || yytoken == Token_VALUE
                     || yytoken == Token_WHERE
                     || yytoken == Token_YIELD
                     || yytoken == Token_ASSEMBLY
                     || yytoken == Token_IDENTIFIER)
              {
                type_name_safe_ast *__node_93 = 0;
                if (!parse_type_name_safe(&__node_93))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_type_name_safe, "type_name_safe");
                      }
                    return false;
                  }
                member_name_or_interface_type = __node_93;

                if ((yytoken == Token_DOT) && ( member_type->type == return_type::type_regular ))
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

                    indexer_declaration_ast *__node_94 = 0;
                    if (!parse_indexer_declaration(&__node_94,
                                                   attributes, modifiers,
                                                   member_type->regular_type, member_name_or_interface_type
                                                  ))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_indexer_declaration, "indexer_declaration");
                          }
                        return false;
                      }
                    if (__node_94->start_token < (*yynode)->start_token)
                      (*yynode)->start_token = __node_94->start_token;
                    (*yynode)->indexer_declaration = __node_94;

                  }
                else if ((yytoken == Token_LBRACE) && ( member_type->type == return_type::type_regular ))
                  {
                    property_declaration_ast *__node_95 = 0;
                    if (!parse_property_declaration(&__node_95,
                                                    attributes, modifiers, member_type->regular_type, member_name_or_interface_type
                                                   ))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_property_declaration, "property_declaration");
                          }
                        return false;
                      }
                    if (__node_95->start_token < (*yynode)->start_token)
                      (*yynode)->start_token = __node_95->start_token;
                    (*yynode)->property_declaration = __node_95;

                  }
                else if (yytoken == Token_LPAREN
                         || yytoken == Token_LESS_THAN)
                  {
                    method_declaration_ast *__node_96 = 0;
                    if (!parse_method_declaration(&__node_96,
                                                  attributes, modifiers, member_type, member_name_or_interface_type
                                                 ))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_method_declaration, "method_declaration");
                          }
                        return false;
                      }
                    if (__node_96->start_token < (*yynode)->start_token)
                      (*yynode)->start_token = __node_96->start_token;
                    (*yynode)->method_declaration = __node_96;

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

    if (yytoken == Token_OBJECT
        || yytoken == Token_STRING
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_IDENTIFIER)
      {
        if (yytoken == Token_ADD
            || yytoken == Token_ALIAS
            || yytoken == Token_GET
            || yytoken == Token_GLOBAL
            || yytoken == Token_PARTIAL
            || yytoken == Token_REMOVE
            || yytoken == Token_SET
            || yytoken == Token_VALUE
            || yytoken == Token_WHERE
            || yytoken == Token_YIELD
            || yytoken == Token_ASSEMBLY
            || yytoken == Token_IDENTIFIER)
          {
            type_name_ast *__node_97 = 0;
            if (!parse_type_name(&__node_97))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_type_name, "type_name");
                  }
                return false;
              }
            (*yynode)->type_name = __node_97;

          }
        else if (yytoken == Token_OBJECT
                 || yytoken == Token_STRING)
          {
            builtin_class_type_ast *__node_98 = 0;
            if (!parse_builtin_class_type(&__node_98))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_builtin_class_type, "builtin_class_type");
                  }
                return false;
              }
            (*yynode)->builtin_class_type = __node_98;

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

  bool parser::parse_compilation_unit(compilation_unit_ast **yynode)
  {
    *yynode = create<compilation_unit_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_ABSTRACT
        || yytoken == Token_CLASS
        || yytoken == Token_DELEGATE
        || yytoken == Token_ENUM
        || yytoken == Token_EXTERN
        || yytoken == Token_FIXED
        || yytoken == Token_INTERFACE
        || yytoken == Token_INTERNAL
        || yytoken == Token_NAMESPACE
        || yytoken == Token_NEW
        || yytoken == Token_OVERRIDE
        || yytoken == Token_PRIVATE
        || yytoken == Token_PROTECTED
        || yytoken == Token_PUBLIC
        || yytoken == Token_READONLY
        || yytoken == Token_SEALED
        || yytoken == Token_STATIC
        || yytoken == Token_STRUCT
        || yytoken == Token_UNSAFE
        || yytoken == Token_USING
        || yytoken == Token_VIRTUAL
        || yytoken == Token_VOLATILE
        || yytoken == Token_PARTIAL
        || yytoken == Token_LBRACKET || yytoken == Token_EOF)
      {
        _M_state.ltCounter = 0;
        if ((yytoken == Token_EXTERN) && ( compatibility_mode() >= csharp20_compatibility ))
          {
            do
              {
                std::size_t try_start_token_8 = token_stream->index() - 1;
                parser_state *try_start_state_8 = copy_current_state();
                {
                  extern_alias_directive_ast *__node_99 = 0;
                  if (!parse_extern_alias_directive(&__node_99))
                    {
                      goto __catch_8;
                    }
                  (*yynode)->extern_alias_sequence = snoc((*yynode)->extern_alias_sequence, __node_99, memory_pool);

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
                           && yytoken != Token_ABSTRACT
                           && yytoken != Token_CLASS
                           && yytoken != Token_DELEGATE
                           && yytoken != Token_ENUM
                           && yytoken != Token_EXTERN
                           && yytoken != Token_FIXED
                           && yytoken != Token_INTERFACE
                           && yytoken != Token_INTERNAL
                           && yytoken != Token_NAMESPACE
                           && yytoken != Token_NEW
                           && yytoken != Token_OVERRIDE
                           && yytoken != Token_PRIVATE
                           && yytoken != Token_PROTECTED
                           && yytoken != Token_PUBLIC
                           && yytoken != Token_READONLY
                           && yytoken != Token_SEALED
                           && yytoken != Token_STATIC
                           && yytoken != Token_STRUCT
                           && yytoken != Token_UNSAFE
                           && yytoken != Token_USING
                           && yytoken != Token_VIRTUAL
                           && yytoken != Token_VOLATILE
                           && yytoken != Token_PARTIAL
                           && yytoken != Token_LBRACKET
                           && yytoken != Token_EOF)
                      {
                        yylex();
                      }
                  }

              }
            while (yytoken == Token_EXTERN);
          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        while (yytoken == Token_USING)
          {
            std::size_t try_start_token_9 = token_stream->index() - 1;
            parser_state *try_start_state_9 = copy_current_state();
            {
              using_directive_ast *__node_100 = 0;
              if (!parse_using_directive(&__node_100))
                {
                  goto __catch_9;
                }
              (*yynode)->using_sequence = snoc((*yynode)->using_sequence, __node_100, memory_pool);

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
                       && yytoken != Token_ABSTRACT
                       && yytoken != Token_CLASS
                       && yytoken != Token_DELEGATE
                       && yytoken != Token_ENUM
                       && yytoken != Token_EXTERN
                       && yytoken != Token_FIXED
                       && yytoken != Token_INTERFACE
                       && yytoken != Token_INTERNAL
                       && yytoken != Token_NAMESPACE
                       && yytoken != Token_NEW
                       && yytoken != Token_OVERRIDE
                       && yytoken != Token_PRIVATE
                       && yytoken != Token_PROTECTED
                       && yytoken != Token_PUBLIC
                       && yytoken != Token_READONLY
                       && yytoken != Token_SEALED
                       && yytoken != Token_STATIC
                       && yytoken != Token_STRUCT
                       && yytoken != Token_UNSAFE
                       && yytoken != Token_USING
                       && yytoken != Token_VIRTUAL
                       && yytoken != Token_VOLATILE
                       && yytoken != Token_PARTIAL
                       && yytoken != Token_LBRACKET
                       && yytoken != Token_EOF)
                  {
                    yylex();
                  }
              }

          }
        while (yytoken == Token_LBRACKET)
          {
            std::size_t try_start_token_10 = token_stream->index() - 1;
            parser_state *try_start_state_10 = copy_current_state();
            {
              if (LA(2).kind != Token_ASSEMBLY)
                break;
              global_attribute_section_ast *__node_101 = 0;
              if (!parse_global_attribute_section(&__node_101))
                {
                  goto __catch_10;
                }
              (*yynode)->global_attribute_sequence = snoc((*yynode)->global_attribute_sequence, __node_101, memory_pool);

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
                       && yytoken != Token_CLASS
                       && yytoken != Token_DELEGATE
                       && yytoken != Token_ENUM
                       && yytoken != Token_EXTERN
                       && yytoken != Token_FIXED
                       && yytoken != Token_INTERFACE
                       && yytoken != Token_INTERNAL
                       && yytoken != Token_NAMESPACE
                       && yytoken != Token_NEW
                       && yytoken != Token_OVERRIDE
                       && yytoken != Token_PRIVATE
                       && yytoken != Token_PROTECTED
                       && yytoken != Token_PUBLIC
                       && yytoken != Token_READONLY
                       && yytoken != Token_SEALED
                       && yytoken != Token_STATIC
                       && yytoken != Token_STRUCT
                       && yytoken != Token_UNSAFE
                       && yytoken != Token_VIRTUAL
                       && yytoken != Token_VOLATILE
                       && yytoken != Token_PARTIAL
                       && yytoken != Token_LBRACKET
                       && yytoken != Token_EOF)
                  {
                    yylex();
                  }
              }

          }
        while (yytoken == Token_ABSTRACT
               || yytoken == Token_CLASS
               || yytoken == Token_DELEGATE
               || yytoken == Token_ENUM
               || yytoken == Token_EXTERN
               || yytoken == Token_FIXED
               || yytoken == Token_INTERFACE
               || yytoken == Token_INTERNAL
               || yytoken == Token_NAMESPACE
               || yytoken == Token_NEW
               || yytoken == Token_OVERRIDE
               || yytoken == Token_PRIVATE
               || yytoken == Token_PROTECTED
               || yytoken == Token_PUBLIC
               || yytoken == Token_READONLY
               || yytoken == Token_SEALED
               || yytoken == Token_STATIC
               || yytoken == Token_STRUCT
               || yytoken == Token_UNSAFE
               || yytoken == Token_VIRTUAL
               || yytoken == Token_VOLATILE
               || yytoken == Token_PARTIAL
               || yytoken == Token_LBRACKET)
          {
            std::size_t try_start_token_11 = token_stream->index() - 1;
            parser_state *try_start_state_11 = copy_current_state();
            {
              namespace_member_declaration_ast *__node_102 = 0;
              if (!parse_namespace_member_declaration(&__node_102))
                {
                  goto __catch_11;
                }
              (*yynode)->namespace_sequence = snoc((*yynode)->namespace_sequence, __node_102, memory_pool);

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
                       && yytoken != Token_ABSTRACT
                       && yytoken != Token_CLASS
                       && yytoken != Token_DELEGATE
                       && yytoken != Token_ENUM
                       && yytoken != Token_EXTERN
                       && yytoken != Token_FIXED
                       && yytoken != Token_INTERFACE
                       && yytoken != Token_INTERNAL
                       && yytoken != Token_NAMESPACE
                       && yytoken != Token_NEW
                       && yytoken != Token_OVERRIDE
                       && yytoken != Token_PRIVATE
                       && yytoken != Token_PROTECTED
                       && yytoken != Token_PUBLIC
                       && yytoken != Token_READONLY
                       && yytoken != Token_SEALED
                       && yytoken != Token_STATIC
                       && yytoken != Token_STRUCT
                       && yytoken != Token_UNSAFE
                       && yytoken != Token_VIRTUAL
                       && yytoken != Token_VOLATILE
                       && yytoken != Token_PARTIAL
                       && yytoken != Token_LBRACKET
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

    if (yytoken == Token_BASE
        || yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_CHECKED
        || yytoken == Token_DECIMAL
        || yytoken == Token_DEFAULT
        || yytoken == Token_DELEGATE
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_OBJECT
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_SIZEOF
        || yytoken == Token_STRING
        || yytoken == Token_THIS
        || yytoken == Token_TYPEOF
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_UNCHECKED
        || yytoken == Token_USHORT
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_LPAREN
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_STAR
        || yytoken == Token_BIT_AND
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_REAL_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        null_coalescing_expression_ast *__node_103 = 0;
        if (!parse_null_coalescing_expression(&__node_103))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_null_coalescing_expression, "null_coalescing_expression");
              }
            return false;
          }
        (*yynode)->null_coalescing_expression = __node_103;

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

            expression_ast *__node_104 = 0;
            if (!parse_expression(&__node_104))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_expression, "expression");
                  }
                return false;
              }
            (*yynode)->if_expression = __node_104;

            if (yytoken != Token_COLON)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_COLON, ":");
                  }
                return false;
              }
            yylex();

            expression_ast *__node_105 = 0;
            if (!parse_expression(&__node_105))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_expression, "expression");
                  }
                return false;
              }
            (*yynode)->else_expression = __node_105;

          }
        else if ((yytoken == Token_BASE
                  || yytoken == Token_BOOL
                  || yytoken == Token_BYTE
                  || yytoken == Token_CHAR
                  || yytoken == Token_CHECKED
                  || yytoken == Token_DECIMAL
                  || yytoken == Token_DEFAULT
                  || yytoken == Token_DELEGATE
                  || yytoken == Token_DOUBLE
                  || yytoken == Token_FLOAT
                  || yytoken == Token_INT
                  || yytoken == Token_LONG
                  || yytoken == Token_NEW
                  || yytoken == Token_OBJECT
                  || yytoken == Token_SBYTE
                  || yytoken == Token_SHORT
                  || yytoken == Token_SIZEOF
                  || yytoken == Token_STRING
                  || yytoken == Token_THIS
                  || yytoken == Token_TYPEOF
                  || yytoken == Token_UINT
                  || yytoken == Token_ULONG
                  || yytoken == Token_UNCHECKED
                  || yytoken == Token_USHORT
                  || yytoken == Token_ADD
                  || yytoken == Token_ALIAS
                  || yytoken == Token_GET
                  || yytoken == Token_GLOBAL
                  || yytoken == Token_PARTIAL
                  || yytoken == Token_REMOVE
                  || yytoken == Token_SET
                  || yytoken == Token_VALUE
                  || yytoken == Token_WHERE
                  || yytoken == Token_YIELD
                  || yytoken == Token_ASSEMBLY
                  || yytoken == Token_LPAREN
                  || yytoken == Token_BANG
                  || yytoken == Token_TILDE
                  || yytoken == Token_INCREMENT
                  || yytoken == Token_DECREMENT
                  || yytoken == Token_PLUS
                  || yytoken == Token_MINUS
                  || yytoken == Token_STAR
                  || yytoken == Token_BIT_AND
                  || yytoken == Token_TRUE
                  || yytoken == Token_FALSE
                  || yytoken == Token_NULL
                  || yytoken == Token_INTEGER_LITERAL
                  || yytoken == Token_REAL_LITERAL
                  || yytoken == Token_CHARACTER_LITERAL
                  || yytoken == Token_STRING_LITERAL
                  || yytoken == Token_IDENTIFIER) && ( (compatibility_mode() >= csharp20_compatibility)
                                                       && is_nullable_type(last_relational_expression_rest_type((*yynode)->null_coalescing_expression)) ))
          {
            unset_nullable_type(last_relational_expression_rest_type((*yynode)->null_coalescing_expression));
            expression_ast *__node_106 = 0;
            if (!parse_expression(&__node_106))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_expression, "expression");
                  }
                return false;
              }
            (*yynode)->if_expression = __node_106;

            if (yytoken != Token_COLON)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_COLON, ":");
                  }
                return false;
              }
            yylex();

            expression_ast *__node_107 = 0;
            if (!parse_expression(&__node_107))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_expression, "expression");
                  }
                return false;
              }
            (*yynode)->else_expression = __node_107;

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

  bool parser::parse_constant_declaration(constant_declaration_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers)
  {
    *yynode = create<constant_declaration_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_CONST)
      {
        type_ast *type = 0;

        const list_node<constant_declarator_ast *> *declarator_sequence = 0;

        if (yytoken != Token_CONST)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_CONST, "const");
              }
            return false;
          }
        yylex();

        type_ast *__node_108 = 0;
        if (!parse_type(&__node_108))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_type, "type");
              }
            return false;
          }
        type = __node_108;

        constant_declarator_ast *__node_109 = 0;
        if (!parse_constant_declarator(&__node_109))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_constant_declarator, "constant_declarator");
              }
            return false;
          }
        declarator_sequence = snoc(declarator_sequence, __node_109, memory_pool);

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

            constant_declarator_ast *__node_110 = 0;
            if (!parse_constant_declarator(&__node_110))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_constant_declarator, "constant_declarator");
                  }
                return false;
              }
            declarator_sequence = snoc(declarator_sequence, __node_110, memory_pool);

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

        constant_declaration_data_ast *__node_111 = 0;
        if (!parse_constant_declaration_data(&__node_111,
                                             attributes, modifiers, type, declarator_sequence
                                            ))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_constant_declaration_data, "constant_declaration_data");
              }
            return false;
          }
        if (__node_111->start_token < (*yynode)->start_token)
          (*yynode)->start_token = __node_111->start_token;
        (*yynode)->data = __node_111;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_constant_declaration_data(constant_declaration_data_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers, type_ast *type, const list_node<constant_declarator_ast *> *constant_declarator_sequence)
  {
    *yynode = create<constant_declaration_data_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    (*yynode)->attributes = attributes;
    if (attributes && attributes->start_token < (*yynode)->start_token)
      (*yynode)->start_token = attributes->start_token;

    (*yynode)->modifiers = modifiers;
    if (modifiers && modifiers->start_token < (*yynode)->start_token)
      (*yynode)->start_token = modifiers->start_token;

    (*yynode)->type = type;
    if (type && type->start_token < (*yynode)->start_token)
      (*yynode)->start_token = type->start_token;

    (*yynode)->constant_declarator_sequence = constant_declarator_sequence;
    if (constant_declarator_sequence && constant_declarator_sequence->to_front()->element->start_token < (*yynode)->start_token)
      (*yynode)->start_token = constant_declarator_sequence->to_front()->element->start_token;

    if (true /*epsilon*/ || yytoken == Token_ABSTRACT
        || yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_CLASS
        || yytoken == Token_CONST
        || yytoken == Token_DECIMAL
        || yytoken == Token_DELEGATE
        || yytoken == Token_DOUBLE
        || yytoken == Token_ENUM
        || yytoken == Token_EVENT
        || yytoken == Token_EXPLICIT
        || yytoken == Token_EXTERN
        || yytoken == Token_FIXED
        || yytoken == Token_FLOAT
        || yytoken == Token_IMPLICIT
        || yytoken == Token_INT
        || yytoken == Token_INTERFACE
        || yytoken == Token_INTERNAL
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_OBJECT
        || yytoken == Token_OVERRIDE
        || yytoken == Token_PRIVATE
        || yytoken == Token_PROTECTED
        || yytoken == Token_PUBLIC
        || yytoken == Token_READONLY
        || yytoken == Token_SBYTE
        || yytoken == Token_SEALED
        || yytoken == Token_SHORT
        || yytoken == Token_STATIC
        || yytoken == Token_STRING
        || yytoken == Token_STRUCT
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_UNSAFE
        || yytoken == Token_USHORT
        || yytoken == Token_VIRTUAL
        || yytoken == Token_VOID
        || yytoken == Token_VOLATILE
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_RBRACE
        || yytoken == Token_LBRACKET
        || yytoken == Token_SEMICOLON
        || yytoken == Token_TILDE
        || yytoken == Token_IDENTIFIER)
    {}
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_constant_declarator(constant_declarator_ast **yynode)
  {
    *yynode = create<constant_declarator_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_IDENTIFIER)
      {
        identifier_ast *__node_112 = 0;
        if (!parse_identifier(&__node_112))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_identifier, "identifier");
              }
            return false;
          }
        (*yynode)->constant_name = __node_112;

        if (yytoken != Token_ASSIGN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_ASSIGN, "=");
              }
            return false;
          }
        yylex();

        constant_expression_ast *__node_113 = 0;
        if (!parse_constant_expression(&__node_113))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_constant_expression, "constant_expression");
              }
            return false;
          }
        (*yynode)->expression = __node_113;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_constant_expression(constant_expression_ast **yynode)
  {
    *yynode = create<constant_expression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BASE
        || yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_CHECKED
        || yytoken == Token_DECIMAL
        || yytoken == Token_DEFAULT
        || yytoken == Token_DELEGATE
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_OBJECT
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_SIZEOF
        || yytoken == Token_STRING
        || yytoken == Token_THIS
        || yytoken == Token_TYPEOF
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_UNCHECKED
        || yytoken == Token_USHORT
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_LPAREN
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_STAR
        || yytoken == Token_BIT_AND
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_REAL_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        expression_ast *__node_114 = 0;
        if (!parse_expression(&__node_114))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_expression, "expression");
              }
            return false;
          }
        (*yynode)->expression = __node_114;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_constructor_constraint(constructor_constraint_ast **yynode)
  {
    *yynode = create<constructor_constraint_ast>();

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

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_constructor_declaration(constructor_declaration_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers)
  {
    *yynode = create<constructor_declaration_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    (*yynode)->attributes = attributes;
    if (attributes && attributes->start_token < (*yynode)->start_token)
      (*yynode)->start_token = attributes->start_token;

    (*yynode)->modifiers = modifiers;
    if (modifiers && modifiers->start_token < (*yynode)->start_token)
      (*yynode)->start_token = modifiers->start_token;

    if (yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_IDENTIFIER)
      {
        identifier_ast *__node_115 = 0;
        if (!parse_identifier(&__node_115))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_identifier, "identifier");
              }
            return false;
          }
        (*yynode)->class_name = __node_115;

        if (yytoken != Token_LPAREN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_LPAREN, "(");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_BOOL
            || yytoken == Token_BYTE
            || yytoken == Token_CHAR
            || yytoken == Token_DECIMAL
            || yytoken == Token_DOUBLE
            || yytoken == Token_FLOAT
            || yytoken == Token_INT
            || yytoken == Token_LONG
            || yytoken == Token_OBJECT
            || yytoken == Token_OUT
            || yytoken == Token_PARAMS
            || yytoken == Token_REF
            || yytoken == Token_SBYTE
            || yytoken == Token_SHORT
            || yytoken == Token_STRING
            || yytoken == Token_UINT
            || yytoken == Token_ULONG
            || yytoken == Token_USHORT
            || yytoken == Token_VOID
            || yytoken == Token_ADD
            || yytoken == Token_ALIAS
            || yytoken == Token_GET
            || yytoken == Token_GLOBAL
            || yytoken == Token_PARTIAL
            || yytoken == Token_REMOVE
            || yytoken == Token_SET
            || yytoken == Token_VALUE
            || yytoken == Token_WHERE
            || yytoken == Token_YIELD
            || yytoken == Token_ASSEMBLY
            || yytoken == Token_LBRACKET
            || yytoken == Token_IDENTIFIER)
          {
            formal_parameter_list_ast *__node_116 = 0;
            if (!parse_formal_parameter_list(&__node_116))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_formal_parameter_list, "formal_parameter_list");
                  }
                return false;
              }
            (*yynode)->formal_parameters = __node_116;

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

        if (yytoken == Token_COLON)
          {
            constructor_initializer_ast *__node_117 = 0;
            if (!parse_constructor_initializer(&__node_117))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_constructor_initializer, "constructor_initializer");
                  }
                return false;
              }
            (*yynode)->constructor_initializer = __node_117;

          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        if (yytoken == Token_LBRACE)
          {
            block_ast *__node_118 = 0;
            if (!parse_block(&__node_118))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_block, "block");
                  }
                return false;
              }
            (*yynode)->body = __node_118;

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

  bool parser::parse_constructor_initializer(constructor_initializer_ast **yynode)
  {
    *yynode = create<constructor_initializer_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

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

        if (yytoken == Token_BASE)
          {
            if (yytoken != Token_BASE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_BASE, "base");
                  }
                return false;
              }
            yylex();

            (*yynode)->initializer_type = constructor_initializer::type_base;
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

            (*yynode)->initializer_type = constructor_initializer::type_this;
          }
        else
          {
            return false;
          }
        if (yytoken != Token_LPAREN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_LPAREN, "(");
              }
            return false;
          }
        yylex();

        optional_argument_list_ast *__node_119 = 0;
        if (!parse_optional_argument_list(&__node_119))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_optional_argument_list, "optional_argument_list");
              }
            return false;
          }
        (*yynode)->arguments = __node_119;

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

  bool parser::parse_conversion_operator_declaration(conversion_operator_declaration_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers)
  {
    *yynode = create<conversion_operator_declaration_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    (*yynode)->attributes = attributes;
    if (attributes && attributes->start_token < (*yynode)->start_token)
      (*yynode)->start_token = attributes->start_token;

    (*yynode)->modifiers = modifiers;
    if (modifiers && modifiers->start_token < (*yynode)->start_token)
      (*yynode)->start_token = modifiers->start_token;

    if (yytoken == Token_EXPLICIT
        || yytoken == Token_IMPLICIT)
      {
        if (yytoken == Token_IMPLICIT)
          {
            if (yytoken != Token_IMPLICIT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_IMPLICIT, "implicit");
                  }
                return false;
              }
            yylex();

            (*yynode)->conversion_type = conversion_operator_declaration::conversion_implicit;
          }
        else if (yytoken == Token_EXPLICIT)
          {
            if (yytoken != Token_EXPLICIT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_EXPLICIT, "explicit");
                  }
                return false;
              }
            yylex();

            (*yynode)->conversion_type = conversion_operator_declaration::conversion_explicit;
          }
        else
          {
            return false;
          }
        if (yytoken != Token_OPERATOR)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_OPERATOR, "operator");
              }
            return false;
          }
        yylex();

        type_ast *__node_120 = 0;
        if (!parse_type(&__node_120))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_type, "type");
              }
            return false;
          }
        (*yynode)->target_type = __node_120;

        if (yytoken != Token_LPAREN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_LPAREN, "(");
              }
            return false;
          }
        yylex();

        type_ast *__node_121 = 0;
        if (!parse_type(&__node_121))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_type, "type");
              }
            return false;
          }
        (*yynode)->source_type = __node_121;

        identifier_ast *__node_122 = 0;
        if (!parse_identifier(&__node_122))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_identifier, "identifier");
              }
            return false;
          }
        (*yynode)->source_name = __node_122;

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
            block_ast *__node_123 = 0;
            if (!parse_block(&__node_123))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_block, "block");
                  }
                return false;
              }
            (*yynode)->body = __node_123;

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

  bool parser::parse_delegate_declaration(delegate_declaration_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers)
  {
    *yynode = create<delegate_declaration_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    (*yynode)->attributes = attributes;
    if (attributes && attributes->start_token < (*yynode)->start_token)
      (*yynode)->start_token = attributes->start_token;

    (*yynode)->modifiers = modifiers;
    if (modifiers && modifiers->start_token < (*yynode)->start_token)
      (*yynode)->start_token = modifiers->start_token;

    if (yytoken == Token_DELEGATE)
      {
        if (yytoken != Token_DELEGATE)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_DELEGATE, "delegate");
              }
            return false;
          }
        yylex();

        return_type_ast *__node_124 = 0;
        if (!parse_return_type(&__node_124))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_return_type, "return_type");
              }
            return false;
          }
        (*yynode)->return_type = __node_124;

        identifier_ast *__node_125 = 0;
        if (!parse_identifier(&__node_125))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_identifier, "identifier");
              }
            return false;
          }
        (*yynode)->delegate_name = __node_125;

        if ((yytoken == Token_LESS_THAN) && ( compatibility_mode() >= csharp20_compatibility ))
          {
            std::size_t try_start_token_12 = token_stream->index() - 1;
            parser_state *try_start_state_12 = copy_current_state();
            {
              type_parameters_ast *__node_126 = 0;
              if (!parse_type_parameters(&__node_126))
                {
                  goto __catch_12;
                }
              (*yynode)->type_parameters = __node_126;

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
                       && yytoken != Token_LPAREN)
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
        if (yytoken != Token_LPAREN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_LPAREN, "(");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_BOOL
            || yytoken == Token_BYTE
            || yytoken == Token_CHAR
            || yytoken == Token_DECIMAL
            || yytoken == Token_DOUBLE
            || yytoken == Token_FLOAT
            || yytoken == Token_INT
            || yytoken == Token_LONG
            || yytoken == Token_OBJECT
            || yytoken == Token_OUT
            || yytoken == Token_PARAMS
            || yytoken == Token_REF
            || yytoken == Token_SBYTE
            || yytoken == Token_SHORT
            || yytoken == Token_STRING
            || yytoken == Token_UINT
            || yytoken == Token_ULONG
            || yytoken == Token_USHORT
            || yytoken == Token_VOID
            || yytoken == Token_ADD
            || yytoken == Token_ALIAS
            || yytoken == Token_GET
            || yytoken == Token_GLOBAL
            || yytoken == Token_PARTIAL
            || yytoken == Token_REMOVE
            || yytoken == Token_SET
            || yytoken == Token_VALUE
            || yytoken == Token_WHERE
            || yytoken == Token_YIELD
            || yytoken == Token_ASSEMBLY
            || yytoken == Token_LBRACKET
            || yytoken == Token_IDENTIFIER)
          {
            formal_parameter_list_ast *__node_127 = 0;
            if (!parse_formal_parameter_list(&__node_127))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_formal_parameter_list, "formal_parameter_list");
                  }
                return false;
              }
            (*yynode)->formal_parameters = __node_127;

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

        if ((yytoken == Token_WHERE) && ( compatibility_mode() >= csharp20_compatibility ))
          {
            do
              {
                std::size_t try_start_token_13 = token_stream->index() - 1;
                parser_state *try_start_state_13 = copy_current_state();
                {
                  type_parameter_constraints_clause_ast *__node_128 = 0;
                  if (!parse_type_parameter_constraints_clause(&__node_128))
                    {
                      goto __catch_13;
                    }
                  (*yynode)->type_parameter_constraints_sequence = snoc((*yynode)->type_parameter_constraints_sequence, __node_128, memory_pool);

                }
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
                    if (try_start_token_13 == token_stream->index() - 1)
                      yylex();

                    while (yytoken != Token_EOF
                           && yytoken != Token_WHERE
                           && yytoken != Token_SEMICOLON)
                      {
                        yylex();
                      }
                  }

              }
            while (yytoken == Token_WHERE);
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

        embedded_statement_ast *__node_129 = 0;
        if (!parse_embedded_statement(&__node_129))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_embedded_statement, "embedded_statement");
              }
            return false;
          }
        (*yynode)->body = __node_129;

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

        boolean_expression_ast *__node_130 = 0;
        if (!parse_boolean_expression(&__node_130))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_boolean_expression, "boolean_expression");
              }
            return false;
          }
        (*yynode)->condition = __node_130;

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

    if (yytoken == Token_BASE
        || yytoken == Token_BOOL
        || yytoken == Token_BREAK
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_CHECKED
        || yytoken == Token_CONTINUE
        || yytoken == Token_DECIMAL
        || yytoken == Token_DEFAULT
        || yytoken == Token_DELEGATE
        || yytoken == Token_DO
        || yytoken == Token_DOUBLE
        || yytoken == Token_FIXED
        || yytoken == Token_FLOAT
        || yytoken == Token_FOREACH
        || yytoken == Token_FOR
        || yytoken == Token_GOTO
        || yytoken == Token_IF
        || yytoken == Token_INT
        || yytoken == Token_LOCK
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_OBJECT
        || yytoken == Token_RETURN
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_SIZEOF
        || yytoken == Token_STRING
        || yytoken == Token_SWITCH
        || yytoken == Token_THIS
        || yytoken == Token_THROW
        || yytoken == Token_TRY
        || yytoken == Token_TYPEOF
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_UNCHECKED
        || yytoken == Token_UNSAFE
        || yytoken == Token_USHORT
        || yytoken == Token_USING
        || yytoken == Token_WHILE
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_LPAREN
        || yytoken == Token_LBRACE
        || yytoken == Token_SEMICOLON
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_STAR
        || yytoken == Token_BIT_AND
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_REAL_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        if (yytoken == Token_LBRACE)
          {
            block_ast *__node_131 = 0;
            if (!parse_block(&__node_131))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_block, "block");
                  }
                return false;
              }
            (*yynode)->block = __node_131;

          }
        else if (yytoken == Token_IF)
          {
            if_statement_ast *__node_132 = 0;
            if (!parse_if_statement(&__node_132))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_if_statement, "if_statement");
                  }
                return false;
              }
            (*yynode)->if_statement = __node_132;

          }
        else if (yytoken == Token_SWITCH)
          {
            switch_statement_ast *__node_133 = 0;
            if (!parse_switch_statement(&__node_133))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_switch_statement, "switch_statement");
                  }
                return false;
              }
            (*yynode)->switch_statement = __node_133;

          }
        else if (yytoken == Token_WHILE)
          {
            while_statement_ast *__node_134 = 0;
            if (!parse_while_statement(&__node_134))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_while_statement, "while_statement");
                  }
                return false;
              }
            (*yynode)->while_statement = __node_134;

          }
        else if (yytoken == Token_DO)
          {
            do_while_statement_ast *__node_135 = 0;
            if (!parse_do_while_statement(&__node_135))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_do_while_statement, "do_while_statement");
                  }
                return false;
              }
            (*yynode)->do_while_statement = __node_135;

          }
        else if (yytoken == Token_FOR)
          {
            for_statement_ast *__node_136 = 0;
            if (!parse_for_statement(&__node_136))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_for_statement, "for_statement");
                  }
                return false;
              }
            (*yynode)->for_statement = __node_136;

          }
        else if (yytoken == Token_FOREACH)
          {
            foreach_statement_ast *__node_137 = 0;
            if (!parse_foreach_statement(&__node_137))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_foreach_statement, "foreach_statement");
                  }
                return false;
              }
            (*yynode)->foreach_statement = __node_137;

          }
        else if (yytoken == Token_BREAK)
          {
            break_statement_ast *__node_138 = 0;
            if (!parse_break_statement(&__node_138))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_break_statement, "break_statement");
                  }
                return false;
              }
            (*yynode)->break_statement = __node_138;

          }
        else if (yytoken == Token_CONTINUE)
          {
            continue_statement_ast *__node_139 = 0;
            if (!parse_continue_statement(&__node_139))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_continue_statement, "continue_statement");
                  }
                return false;
              }
            (*yynode)->continue_statement = __node_139;

          }
        else if (yytoken == Token_GOTO)
          {
            goto_statement_ast *__node_140 = 0;
            if (!parse_goto_statement(&__node_140))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_goto_statement, "goto_statement");
                  }
                return false;
              }
            (*yynode)->goto_statement = __node_140;

          }
        else if (yytoken == Token_RETURN)
          {
            return_statement_ast *__node_141 = 0;
            if (!parse_return_statement(&__node_141))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_return_statement, "return_statement");
                  }
                return false;
              }
            (*yynode)->return_statement = __node_141;

          }
        else if (yytoken == Token_THROW)
          {
            throw_statement_ast *__node_142 = 0;
            if (!parse_throw_statement(&__node_142))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_throw_statement, "throw_statement");
                  }
                return false;
              }
            (*yynode)->throw_statement = __node_142;

          }
        else if (yytoken == Token_TRY)
          {
            try_statement_ast *__node_143 = 0;
            if (!parse_try_statement(&__node_143))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_try_statement, "try_statement");
                  }
                return false;
              }
            (*yynode)->try_statement = __node_143;

          }
        else if (yytoken == Token_LOCK)
          {
            lock_statement_ast *__node_144 = 0;
            if (!parse_lock_statement(&__node_144))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_lock_statement, "lock_statement");
                  }
                return false;
              }
            (*yynode)->lock_statement = __node_144;

          }
        else if (yytoken == Token_USING)
          {
            using_statement_ast *__node_145 = 0;
            if (!parse_using_statement(&__node_145))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_using_statement, "using_statement");
                  }
                return false;
              }
            (*yynode)->using_statement = __node_145;

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
        else if ((yytoken == Token_CHECKED) && ( LA(2).kind == Token_LBRACE ))
          {
            checked_statement_ast *__node_146 = 0;
            if (!parse_checked_statement(&__node_146))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_checked_statement, "checked_statement");
                  }
                return false;
              }
            (*yynode)->checked_statement = __node_146;

          }
        else if ((yytoken == Token_UNCHECKED) && ( LA(2).kind == Token_LBRACE ))
          {
            unchecked_statement_ast *__node_147 = 0;
            if (!parse_unchecked_statement(&__node_147))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_unchecked_statement, "unchecked_statement");
                  }
                return false;
              }
            (*yynode)->unchecked_statement = __node_147;

          }
        else if ((yytoken == Token_YIELD) && ( LA(2).kind == Token_RETURN || LA(2).kind == Token_BREAK ))
          {
            yield_statement_ast *__node_148 = 0;
            if (!parse_yield_statement(&__node_148))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_yield_statement, "yield_statement");
                  }
                return false;
              }
            (*yynode)->yield_statement = __node_148;

          }
        else if (yytoken == Token_BASE
                 || yytoken == Token_BOOL
                 || yytoken == Token_BYTE
                 || yytoken == Token_CHAR
                 || yytoken == Token_CHECKED
                 || yytoken == Token_DECIMAL
                 || yytoken == Token_DEFAULT
                 || yytoken == Token_DELEGATE
                 || yytoken == Token_DOUBLE
                 || yytoken == Token_FLOAT
                 || yytoken == Token_INT
                 || yytoken == Token_LONG
                 || yytoken == Token_NEW
                 || yytoken == Token_OBJECT
                 || yytoken == Token_SBYTE
                 || yytoken == Token_SHORT
                 || yytoken == Token_SIZEOF
                 || yytoken == Token_STRING
                 || yytoken == Token_THIS
                 || yytoken == Token_TYPEOF
                 || yytoken == Token_UINT
                 || yytoken == Token_ULONG
                 || yytoken == Token_UNCHECKED
                 || yytoken == Token_USHORT
                 || yytoken == Token_ADD
                 || yytoken == Token_ALIAS
                 || yytoken == Token_GET
                 || yytoken == Token_GLOBAL
                 || yytoken == Token_PARTIAL
                 || yytoken == Token_REMOVE
                 || yytoken == Token_SET
                 || yytoken == Token_VALUE
                 || yytoken == Token_WHERE
                 || yytoken == Token_YIELD
                 || yytoken == Token_ASSEMBLY
                 || yytoken == Token_LPAREN
                 || yytoken == Token_BANG
                 || yytoken == Token_TILDE
                 || yytoken == Token_INCREMENT
                 || yytoken == Token_DECREMENT
                 || yytoken == Token_PLUS
                 || yytoken == Token_MINUS
                 || yytoken == Token_STAR
                 || yytoken == Token_BIT_AND
                 || yytoken == Token_TRUE
                 || yytoken == Token_FALSE
                 || yytoken == Token_NULL
                 || yytoken == Token_INTEGER_LITERAL
                 || yytoken == Token_REAL_LITERAL
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
            (*yynode)->expression_statement = __node_149;

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
        else if (yytoken == Token_UNSAFE)
          {
            unsafe_statement_ast *__node_150 = 0;
            if (!parse_unsafe_statement(&__node_150))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_unsafe_statement, "unsafe_statement");
                  }
                return false;
              }
            (*yynode)->unsafe_statement = __node_150;

          }
        else if (yytoken == Token_FIXED)
          {
            fixed_statement_ast *__node_151 = 0;
            if (!parse_fixed_statement(&__node_151))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_fixed_statement, "fixed_statement");
                  }
                return false;
              }
            (*yynode)->fixed_statement = __node_151;

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

  bool parser::parse_enum_base(enum_base_ast **yynode)
  {
    *yynode = create<enum_base_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

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

        integral_type_ast *__node_152 = 0;
        if (!parse_integral_type(&__node_152))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_integral_type, "integral_type");
              }
            return false;
          }
        (*yynode)->integral_type = __node_152;

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

        std::size_t try_start_token_14 = token_stream->index() - 1;
        parser_state *try_start_state_14 = copy_current_state();
        {
          if (yytoken == Token_ADD
              || yytoken == Token_ALIAS
              || yytoken == Token_GET
              || yytoken == Token_GLOBAL
              || yytoken == Token_PARTIAL
              || yytoken == Token_REMOVE
              || yytoken == Token_SET
              || yytoken == Token_VALUE
              || yytoken == Token_WHERE
              || yytoken == Token_YIELD
              || yytoken == Token_ASSEMBLY
              || yytoken == Token_LBRACKET
              || yytoken == Token_IDENTIFIER)
            {
              enum_member_declaration_ast *__node_153 = 0;
              if (!parse_enum_member_declaration(&__node_153))
                {
                  goto __catch_14;
                }
              (*yynode)->member_declaration_sequence = snoc((*yynode)->member_declaration_sequence, __node_153, memory_pool);

              while (yytoken == Token_COMMA)
                {
                  if (LA(2).kind == Token_RBRACE)
                    break;
                  if (yytoken != Token_COMMA)
                    goto __catch_14;
                  yylex();

                  enum_member_declaration_ast *__node_154 = 0;
                  if (!parse_enum_member_declaration(&__node_154))
                    {
                      goto __catch_14;
                    }
                  (*yynode)->member_declaration_sequence = snoc((*yynode)->member_declaration_sequence, __node_154, memory_pool);

                }
              if (yytoken == Token_COMMA)
                {
                  if (yytoken != Token_COMMA)
                    goto __catch_14;
                  yylex();

                }
              else if (true /*epsilon*/)
              {}
              else
                {
                  goto __catch_14;
                }
            }
          else if (true /*epsilon*/)
          {}
          else
            {
              goto __catch_14;
            }
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
                   && yytoken != Token_RBRACE)
              {
                yylex();
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

  bool parser::parse_enum_declaration(enum_declaration_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers)
  {
    *yynode = create<enum_declaration_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    (*yynode)->attributes = attributes;
    if (attributes && attributes->start_token < (*yynode)->start_token)
      (*yynode)->start_token = attributes->start_token;

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

        identifier_ast *__node_155 = 0;
        if (!parse_identifier(&__node_155))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_identifier, "identifier");
              }
            return false;
          }
        (*yynode)->enum_name = __node_155;

        if (yytoken == Token_COLON)
          {
            enum_base_ast *__node_156 = 0;
            if (!parse_enum_base(&__node_156))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_enum_base, "enum_base");
                  }
                return false;
              }
            (*yynode)->enum_base = __node_156;

          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        enum_body_ast *__node_157 = 0;
        if (!parse_enum_body(&__node_157))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_enum_body, "enum_body");
              }
            return false;
          }
        (*yynode)->body = __node_157;

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

  bool parser::parse_enum_member_declaration(enum_member_declaration_ast **yynode)
  {
    *yynode = create<enum_member_declaration_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_LBRACKET
        || yytoken == Token_IDENTIFIER)
      {
        optional_attribute_sections_ast *__node_158 = 0;
        if (!parse_optional_attribute_sections(&__node_158))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_optional_attribute_sections, "optional_attribute_sections");
              }
            return false;
          }
        (*yynode)->attributes = __node_158;

        identifier_ast *__node_159 = 0;
        if (!parse_identifier(&__node_159))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_identifier, "identifier");
              }
            return false;
          }
        (*yynode)->member_name = __node_159;

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

            constant_expression_ast *__node_160 = 0;
            if (!parse_constant_expression(&__node_160))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_constant_expression, "constant_expression");
                  }
                return false;
              }
            (*yynode)->constant_expression = __node_160;

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

  bool parser::parse_equality_expression(equality_expression_ast **yynode)
  {
    *yynode = create<equality_expression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BASE
        || yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_CHECKED
        || yytoken == Token_DECIMAL
        || yytoken == Token_DEFAULT
        || yytoken == Token_DELEGATE
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_OBJECT
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_SIZEOF
        || yytoken == Token_STRING
        || yytoken == Token_THIS
        || yytoken == Token_TYPEOF
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_UNCHECKED
        || yytoken == Token_USHORT
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_LPAREN
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_STAR
        || yytoken == Token_BIT_AND
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_REAL_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        relational_expression_ast *__node_161 = 0;
        if (!parse_relational_expression(&__node_161))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_relational_expression, "relational_expression");
              }
            return false;
          }
        (*yynode)->expression = __node_161;

        while (yytoken == Token_EQUAL
               || yytoken == Token_NOT_EQUAL)
          {
            equality_expression_rest_ast *__node_162 = 0;
            if (!parse_equality_expression_rest(&__node_162))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_equality_expression_rest, "equality_expression_rest");
                  }
                return false;
              }
            (*yynode)->additional_expression_sequence = snoc((*yynode)->additional_expression_sequence, __node_162, memory_pool);

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
        relational_expression_ast *__node_163 = 0;
        if (!parse_relational_expression(&__node_163))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_relational_expression, "relational_expression");
              }
            return false;
          }
        (*yynode)->expression = __node_163;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_event_accessor_declarations(event_accessor_declarations_ast **yynode)
  {
    *yynode = create<event_accessor_declarations_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_ADD
        || yytoken == Token_REMOVE
        || yytoken == Token_LBRACKET)
      {
        optional_attribute_sections_ast *__node_164 = 0;
        if (!parse_optional_attribute_sections(&__node_164))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_optional_attribute_sections, "optional_attribute_sections");
              }
            return false;
          }
        (*yynode)->accessor1_attributes = __node_164;

        if (yytoken == Token_ADD)
          {
            if (yytoken != Token_ADD)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_ADD, "add");
                  }
                return false;
              }
            yylex();

            block_ast *__node_165 = 0;
            if (!parse_block(&__node_165))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_block, "block");
                  }
                return false;
              }
            (*yynode)->accessor1_body = __node_165;

            optional_attribute_sections_ast *__node_166 = 0;
            if (!parse_optional_attribute_sections(&__node_166))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_optional_attribute_sections, "optional_attribute_sections");
                  }
                return false;
              }
            (*yynode)->accessor2_attributes = __node_166;

            if (yytoken != Token_REMOVE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_REMOVE, "remove");
                  }
                return false;
              }
            yylex();

            block_ast *__node_167 = 0;
            if (!parse_block(&__node_167))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_block, "block");
                  }
                return false;
              }
            (*yynode)->accessor2_body = __node_167;

            (*yynode)->order = event_accessor_declarations::order_add_remove;
          }
        else if (yytoken == Token_REMOVE)
          {
            if (yytoken != Token_REMOVE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_REMOVE, "remove");
                  }
                return false;
              }
            yylex();

            block_ast *__node_168 = 0;
            if (!parse_block(&__node_168))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_block, "block");
                  }
                return false;
              }
            (*yynode)->accessor1_body = __node_168;

            optional_attribute_sections_ast *__node_169 = 0;
            if (!parse_optional_attribute_sections(&__node_169))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_optional_attribute_sections, "optional_attribute_sections");
                  }
                return false;
              }
            (*yynode)->accessor2_attributes = __node_169;

            if (yytoken != Token_ADD)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_ADD, "add");
                  }
                return false;
              }
            yylex();

            block_ast *__node_170 = 0;
            if (!parse_block(&__node_170))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_block, "block");
                  }
                return false;
              }
            (*yynode)->accessor2_body = __node_170;

            (*yynode)->order = event_accessor_declarations::order_remove_add;
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

  bool parser::parse_event_declaration(event_declaration_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers)
  {
    *yynode = create<event_declaration_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    (*yynode)->attributes = attributes;
    if (attributes && attributes->start_token < (*yynode)->start_token)
      (*yynode)->start_token = attributes->start_token;

    (*yynode)->modifiers = modifiers;
    if (modifiers && modifiers->start_token < (*yynode)->start_token)
      (*yynode)->start_token = modifiers->start_token;

    if (yytoken == Token_EVENT)
      {
        if (yytoken != Token_EVENT)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_EVENT, "event");
              }
            return false;
          }
        yylex();

        type_ast *__node_171 = 0;
        if (!parse_type(&__node_171))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_type, "type");
              }
            return false;
          }
        (*yynode)->type = __node_171;

        if ((yytoken == Token_ADD
             || yytoken == Token_ALIAS
             || yytoken == Token_GET
             || yytoken == Token_GLOBAL
             || yytoken == Token_PARTIAL
             || yytoken == Token_REMOVE
             || yytoken == Token_SET
             || yytoken == Token_VALUE
             || yytoken == Token_WHERE
             || yytoken == Token_YIELD
             || yytoken == Token_ASSEMBLY
             || yytoken == Token_IDENTIFIER) && ( (LA(2).kind == Token_COMMA) || (LA(2).kind == Token_ASSIGN)
                                                  || (LA(2).kind == Token_SEMICOLON)
                                                ))
          {
            variable_declarator_ast *__node_172 = 0;
            if (!parse_variable_declarator(&__node_172, false))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_variable_declarator, "variable_declarator");
                  }
                return false;
              }
            (*yynode)->variable_declarator_sequence = snoc((*yynode)->variable_declarator_sequence, __node_172, memory_pool);

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

                variable_declarator_ast *__node_173 = 0;
                if (!parse_variable_declarator(&__node_173, false))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_variable_declarator, "variable_declarator");
                      }
                    return false;
                  }
                (*yynode)->variable_declarator_sequence = snoc((*yynode)->variable_declarator_sequence, __node_173, memory_pool);

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
        else if (yytoken == Token_ADD
                 || yytoken == Token_ALIAS
                 || yytoken == Token_GET
                 || yytoken == Token_GLOBAL
                 || yytoken == Token_PARTIAL
                 || yytoken == Token_REMOVE
                 || yytoken == Token_SET
                 || yytoken == Token_VALUE
                 || yytoken == Token_WHERE
                 || yytoken == Token_YIELD
                 || yytoken == Token_ASSEMBLY
                 || yytoken == Token_IDENTIFIER)
          {
            type_name_ast *__node_174 = 0;
            if (!parse_type_name(&__node_174))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_type_name, "type_name");
                  }
                return false;
              }
            (*yynode)->event_name = __node_174;

            if (yytoken != Token_LBRACE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LBRACE, "{");
                  }
                return false;
              }
            yylex();

            event_accessor_declarations_ast *__node_175 = 0;
            if (!parse_event_accessor_declarations(&__node_175))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_event_accessor_declarations, "event_accessor_declarations");
                  }
                return false;
              }
            (*yynode)->event_accessor_declarations = __node_175;

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

    if (yytoken == Token_BASE
        || yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_CHECKED
        || yytoken == Token_DECIMAL
        || yytoken == Token_DEFAULT
        || yytoken == Token_DELEGATE
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_OBJECT
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_SIZEOF
        || yytoken == Token_STRING
        || yytoken == Token_THIS
        || yytoken == Token_TYPEOF
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_UNCHECKED
        || yytoken == Token_USHORT
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_LPAREN
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_STAR
        || yytoken == Token_BIT_AND
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_REAL_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        conditional_expression_ast *__node_176 = 0;
        if (!parse_conditional_expression(&__node_176))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_conditional_expression, "conditional_expression");
              }
            return false;
          }
        (*yynode)->conditional_expression = __node_176;

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
            || yytoken == Token_RSHIFT_ASSIGN)
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
            else if (yytoken == Token_RSHIFT_ASSIGN)
              {
                if (yytoken != Token_RSHIFT_ASSIGN)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_RSHIFT_ASSIGN, ">>=");
                      }
                    return false;
                  }
                yylex();

                (*yynode)->assignment_operator = expression::op_rshift_assign;
              }
            else
              {
                return false;
              }
            expression_ast *__node_177 = 0;
            if (!parse_expression(&__node_177))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_expression, "expression");
                  }
                return false;
              }
            (*yynode)->assignment_expression = __node_177;

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

  bool parser::parse_extern_alias_directive(extern_alias_directive_ast **yynode)
  {
    *yynode = create<extern_alias_directive_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_EXTERN)
      {
        if (yytoken != Token_EXTERN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_EXTERN, "extern");
              }
            return false;
          }
        yylex();

        if (yytoken != Token_ALIAS)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_ALIAS, "alias");
              }
            return false;
          }
        yylex();

        identifier_ast *__node_178 = 0;
        if (!parse_identifier(&__node_178))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_identifier, "identifier");
              }
            return false;
          }
        (*yynode)->identifier = __node_178;

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

  bool parser::parse_finalizer_declaration(finalizer_declaration_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers)
  {
    *yynode = create<finalizer_declaration_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    (*yynode)->attributes = attributes;
    if (attributes && attributes->start_token < (*yynode)->start_token)
      (*yynode)->start_token = attributes->start_token;

    (*yynode)->modifiers = modifiers;
    if (modifiers && modifiers->start_token < (*yynode)->start_token)
      (*yynode)->start_token = modifiers->start_token;

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

        identifier_ast *__node_179 = 0;
        if (!parse_identifier(&__node_179))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_identifier, "identifier");
              }
            return false;
          }
        (*yynode)->class_name = __node_179;

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

        if (yytoken == Token_LBRACE)
          {
            block_ast *__node_180 = 0;
            if (!parse_block(&__node_180))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_block, "block");
                  }
                return false;
              }
            (*yynode)->finalizer_body = __node_180;

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

  bool parser::parse_fixed_pointer_declarator(fixed_pointer_declarator_ast **yynode)
  {
    *yynode = create<fixed_pointer_declarator_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_IDENTIFIER)
      {
        identifier_ast *__node_181 = 0;
        if (!parse_identifier(&__node_181))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_identifier, "identifier");
              }
            return false;
          }
        (*yynode)->pointer_name = __node_181;

        if (yytoken != Token_ASSIGN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_ASSIGN, "=");
              }
            return false;
          }
        yylex();

        expression_ast *__node_182 = 0;
        if (!parse_expression(&__node_182))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_expression, "expression");
              }
            return false;
          }
        (*yynode)->initializer = __node_182;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_fixed_statement(fixed_statement_ast **yynode)
  {
    *yynode = create<fixed_statement_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_FIXED)
      {
        if (yytoken != Token_FIXED)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_FIXED, "fixed");
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

        pointer_type_ast *__node_183 = 0;
        if (!parse_pointer_type(&__node_183))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_pointer_type, "pointer_type");
              }
            return false;
          }
        (*yynode)->pointer_type = __node_183;

        fixed_pointer_declarator_ast *__node_184 = 0;
        if (!parse_fixed_pointer_declarator(&__node_184))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_fixed_pointer_declarator, "fixed_pointer_declarator");
              }
            return false;
          }
        (*yynode)->fixed_pointer_declarator = __node_184;

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

            fixed_pointer_declarator_ast *__node_185 = 0;
            if (!parse_fixed_pointer_declarator(&__node_185))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_fixed_pointer_declarator, "fixed_pointer_declarator");
                  }
                return false;
              }
            (*yynode)->fixed_pointer_declarator = __node_185;

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

        embedded_statement_ast *__node_186 = 0;
        if (!parse_embedded_statement(&__node_186))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_embedded_statement, "embedded_statement");
              }
            return false;
          }
        (*yynode)->body = __node_186;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_floating_point_type(floating_point_type_ast **yynode)
  {
    *yynode = create<floating_point_type_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT)
      {
        if (yytoken == Token_FLOAT)
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

            (*yynode)->type = floating_point_type::type_float;
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

            (*yynode)->type = floating_point_type::type_double;
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

  bool parser::parse_for_control(for_control_ast **yynode)
  {
    *yynode = create<for_control_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BASE
        || yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_CHECKED
        || yytoken == Token_DECIMAL
        || yytoken == Token_DEFAULT
        || yytoken == Token_DELEGATE
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_OBJECT
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_SIZEOF
        || yytoken == Token_STRING
        || yytoken == Token_THIS
        || yytoken == Token_TYPEOF
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_UNCHECKED
        || yytoken == Token_USHORT
        || yytoken == Token_VOID
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_LPAREN
        || yytoken == Token_SEMICOLON
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_STAR
        || yytoken == Token_BIT_AND
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_REAL_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        if (yytoken == Token_BASE
            || yytoken == Token_BOOL
            || yytoken == Token_BYTE
            || yytoken == Token_CHAR
            || yytoken == Token_CHECKED
            || yytoken == Token_DECIMAL
            || yytoken == Token_DEFAULT
            || yytoken == Token_DELEGATE
            || yytoken == Token_DOUBLE
            || yytoken == Token_FLOAT
            || yytoken == Token_INT
            || yytoken == Token_LONG
            || yytoken == Token_NEW
            || yytoken == Token_OBJECT
            || yytoken == Token_SBYTE
            || yytoken == Token_SHORT
            || yytoken == Token_SIZEOF
            || yytoken == Token_STRING
            || yytoken == Token_THIS
            || yytoken == Token_TYPEOF
            || yytoken == Token_UINT
            || yytoken == Token_ULONG
            || yytoken == Token_UNCHECKED
            || yytoken == Token_USHORT
            || yytoken == Token_VOID
            || yytoken == Token_ADD
            || yytoken == Token_ALIAS
            || yytoken == Token_GET
            || yytoken == Token_GLOBAL
            || yytoken == Token_PARTIAL
            || yytoken == Token_REMOVE
            || yytoken == Token_SET
            || yytoken == Token_VALUE
            || yytoken == Token_WHERE
            || yytoken == Token_YIELD
            || yytoken == Token_ASSEMBLY
            || yytoken == Token_LPAREN
            || yytoken == Token_BANG
            || yytoken == Token_TILDE
            || yytoken == Token_INCREMENT
            || yytoken == Token_DECREMENT
            || yytoken == Token_PLUS
            || yytoken == Token_MINUS
            || yytoken == Token_STAR
            || yytoken == Token_BIT_AND
            || yytoken == Token_TRUE
            || yytoken == Token_FALSE
            || yytoken == Token_NULL
            || yytoken == Token_INTEGER_LITERAL
            || yytoken == Token_REAL_LITERAL
            || yytoken == Token_CHARACTER_LITERAL
            || yytoken == Token_STRING_LITERAL
            || yytoken == Token_IDENTIFIER)
          {
            bool block_errors_15 = block_errors(true);
            std::size_t try_start_token_15 = token_stream->index() - 1;
            parser_state *try_start_state_15 = copy_current_state();
            {
              local_variable_declaration_ast *__node_187 = 0;
              if (!parse_local_variable_declaration(&__node_187))
                {
                  goto __catch_15;
                }
              (*yynode)->local_variable_declaration = __node_187;

            }
            block_errors(block_errors_15);
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
                block_errors(block_errors_15);
                rewind(try_start_token_15);

                statement_expression_ast *__node_188 = 0;
                if (!parse_statement_expression(&__node_188))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_statement_expression, "statement_expression");
                      }
                    return false;
                  }
                (*yynode)->statement_expression_sequence = snoc((*yynode)->statement_expression_sequence, __node_188, memory_pool);

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

                    statement_expression_ast *__node_189 = 0;
                    if (!parse_statement_expression(&__node_189))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_statement_expression, "statement_expression");
                          }
                        return false;
                      }
                    (*yynode)->statement_expression_sequence = snoc((*yynode)->statement_expression_sequence, __node_189, memory_pool);

                  }
              }

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

        if (yytoken == Token_BASE
            || yytoken == Token_BOOL
            || yytoken == Token_BYTE
            || yytoken == Token_CHAR
            || yytoken == Token_CHECKED
            || yytoken == Token_DECIMAL
            || yytoken == Token_DEFAULT
            || yytoken == Token_DELEGATE
            || yytoken == Token_DOUBLE
            || yytoken == Token_FLOAT
            || yytoken == Token_INT
            || yytoken == Token_LONG
            || yytoken == Token_NEW
            || yytoken == Token_OBJECT
            || yytoken == Token_SBYTE
            || yytoken == Token_SHORT
            || yytoken == Token_SIZEOF
            || yytoken == Token_STRING
            || yytoken == Token_THIS
            || yytoken == Token_TYPEOF
            || yytoken == Token_UINT
            || yytoken == Token_ULONG
            || yytoken == Token_UNCHECKED
            || yytoken == Token_USHORT
            || yytoken == Token_ADD
            || yytoken == Token_ALIAS
            || yytoken == Token_GET
            || yytoken == Token_GLOBAL
            || yytoken == Token_PARTIAL
            || yytoken == Token_REMOVE
            || yytoken == Token_SET
            || yytoken == Token_VALUE
            || yytoken == Token_WHERE
            || yytoken == Token_YIELD
            || yytoken == Token_ASSEMBLY
            || yytoken == Token_LPAREN
            || yytoken == Token_BANG
            || yytoken == Token_TILDE
            || yytoken == Token_INCREMENT
            || yytoken == Token_DECREMENT
            || yytoken == Token_PLUS
            || yytoken == Token_MINUS
            || yytoken == Token_STAR
            || yytoken == Token_BIT_AND
            || yytoken == Token_TRUE
            || yytoken == Token_FALSE
            || yytoken == Token_NULL
            || yytoken == Token_INTEGER_LITERAL
            || yytoken == Token_REAL_LITERAL
            || yytoken == Token_CHARACTER_LITERAL
            || yytoken == Token_STRING_LITERAL
            || yytoken == Token_IDENTIFIER)
          {
            boolean_expression_ast *__node_190 = 0;
            if (!parse_boolean_expression(&__node_190))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_boolean_expression, "boolean_expression");
                  }
                return false;
              }
            (*yynode)->for_condition = __node_190;

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

        if (yytoken == Token_BASE
            || yytoken == Token_BOOL
            || yytoken == Token_BYTE
            || yytoken == Token_CHAR
            || yytoken == Token_CHECKED
            || yytoken == Token_DECIMAL
            || yytoken == Token_DEFAULT
            || yytoken == Token_DELEGATE
            || yytoken == Token_DOUBLE
            || yytoken == Token_FLOAT
            || yytoken == Token_INT
            || yytoken == Token_LONG
            || yytoken == Token_NEW
            || yytoken == Token_OBJECT
            || yytoken == Token_SBYTE
            || yytoken == Token_SHORT
            || yytoken == Token_SIZEOF
            || yytoken == Token_STRING
            || yytoken == Token_THIS
            || yytoken == Token_TYPEOF
            || yytoken == Token_UINT
            || yytoken == Token_ULONG
            || yytoken == Token_UNCHECKED
            || yytoken == Token_USHORT
            || yytoken == Token_ADD
            || yytoken == Token_ALIAS
            || yytoken == Token_GET
            || yytoken == Token_GLOBAL
            || yytoken == Token_PARTIAL
            || yytoken == Token_REMOVE
            || yytoken == Token_SET
            || yytoken == Token_VALUE
            || yytoken == Token_WHERE
            || yytoken == Token_YIELD
            || yytoken == Token_ASSEMBLY
            || yytoken == Token_LPAREN
            || yytoken == Token_BANG
            || yytoken == Token_TILDE
            || yytoken == Token_INCREMENT
            || yytoken == Token_DECREMENT
            || yytoken == Token_PLUS
            || yytoken == Token_MINUS
            || yytoken == Token_STAR
            || yytoken == Token_BIT_AND
            || yytoken == Token_TRUE
            || yytoken == Token_FALSE
            || yytoken == Token_NULL
            || yytoken == Token_INTEGER_LITERAL
            || yytoken == Token_REAL_LITERAL
            || yytoken == Token_CHARACTER_LITERAL
            || yytoken == Token_STRING_LITERAL
            || yytoken == Token_IDENTIFIER)
          {
            statement_expression_ast *__node_191 = 0;
            if (!parse_statement_expression(&__node_191))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_statement_expression, "statement_expression");
                  }
                return false;
              }
            (*yynode)->for_iterator_sequence = snoc((*yynode)->for_iterator_sequence, __node_191, memory_pool);

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

                statement_expression_ast *__node_192 = 0;
                if (!parse_statement_expression(&__node_192))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_statement_expression, "statement_expression");
                      }
                    return false;
                  }
                (*yynode)->for_iterator_sequence = snoc((*yynode)->for_iterator_sequence, __node_192, memory_pool);

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

        for_control_ast *__node_193 = 0;
        if (!parse_for_control(&__node_193))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_for_control, "for_control");
              }
            return false;
          }
        (*yynode)->for_control = __node_193;

        if (yytoken != Token_RPAREN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_RPAREN, ")");
              }
            return false;
          }
        yylex();

        embedded_statement_ast *__node_194 = 0;
        if (!parse_embedded_statement(&__node_194))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_embedded_statement, "embedded_statement");
              }
            return false;
          }
        (*yynode)->for_body = __node_194;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_foreach_statement(foreach_statement_ast **yynode)
  {
    *yynode = create<foreach_statement_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_FOREACH)
      {
        if (yytoken != Token_FOREACH)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_FOREACH, "foreach");
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

        type_ast *__node_195 = 0;
        if (!parse_type(&__node_195))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_type, "type");
              }
            return false;
          }
        (*yynode)->variable_type = __node_195;

        identifier_ast *__node_196 = 0;
        if (!parse_identifier(&__node_196))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_identifier, "identifier");
              }
            return false;
          }
        (*yynode)->variable_name = __node_196;

        if (yytoken != Token_IN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_IN, "in");
              }
            return false;
          }
        yylex();

        expression_ast *__node_197 = 0;
        if (!parse_expression(&__node_197))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_expression, "expression");
              }
            return false;
          }
        (*yynode)->collection = __node_197;

        if (yytoken != Token_RPAREN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_RPAREN, ")");
              }
            return false;
          }
        yylex();

        embedded_statement_ast *__node_198 = 0;
        if (!parse_embedded_statement(&__node_198))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_embedded_statement, "embedded_statement");
              }
            return false;
          }
        (*yynode)->body = __node_198;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_formal_parameter(formal_parameter_ast **yynode, bool* parameter_array_occurred)
  {
    *yynode = create<formal_parameter_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DECIMAL
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_OBJECT
        || yytoken == Token_OUT
        || yytoken == Token_PARAMS
        || yytoken == Token_REF
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_STRING
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_USHORT
        || yytoken == Token_VOID
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_LBRACKET
        || yytoken == Token_IDENTIFIER)
      {
        optional_attribute_sections_ast *__node_199 = 0;
        if (!parse_optional_attribute_sections(&__node_199))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_optional_attribute_sections, "optional_attribute_sections");
              }
            return false;
          }
        (*yynode)->attributes = __node_199;

        if (yytoken == Token_PARAMS)
          {
            parameter_array_ast *__node_200 = 0;
            if (!parse_parameter_array(&__node_200))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_parameter_array, "parameter_array");
                  }
                return false;
              }
            (*yynode)->parameter_array = __node_200;

            *parameter_array_occurred = true;
          }
        else if (yytoken == Token_BOOL
                 || yytoken == Token_BYTE
                 || yytoken == Token_CHAR
                 || yytoken == Token_DECIMAL
                 || yytoken == Token_DOUBLE
                 || yytoken == Token_FLOAT
                 || yytoken == Token_INT
                 || yytoken == Token_LONG
                 || yytoken == Token_OBJECT
                 || yytoken == Token_OUT
                 || yytoken == Token_REF
                 || yytoken == Token_SBYTE
                 || yytoken == Token_SHORT
                 || yytoken == Token_STRING
                 || yytoken == Token_UINT
                 || yytoken == Token_ULONG
                 || yytoken == Token_USHORT
                 || yytoken == Token_VOID
                 || yytoken == Token_ADD
                 || yytoken == Token_ALIAS
                 || yytoken == Token_GET
                 || yytoken == Token_GLOBAL
                 || yytoken == Token_PARTIAL
                 || yytoken == Token_REMOVE
                 || yytoken == Token_SET
                 || yytoken == Token_VALUE
                 || yytoken == Token_WHERE
                 || yytoken == Token_YIELD
                 || yytoken == Token_ASSEMBLY
                 || yytoken == Token_IDENTIFIER)
          {
            if (yytoken == Token_OUT
                || yytoken == Token_REF)
              {
                parameter_modifier_ast *__node_201 = 0;
                if (!parse_parameter_modifier(&__node_201))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_parameter_modifier, "parameter_modifier");
                      }
                    return false;
                  }
                (*yynode)->modifier = __node_201;

              }
            else if (true /*epsilon*/)
            {}
            else
              {
                return false;
              }
            type_ast *__node_202 = 0;
            if (!parse_type(&__node_202))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_type, "type");
                  }
                return false;
              }
            (*yynode)->type = __node_202;

            identifier_ast *__node_203 = 0;
            if (!parse_identifier(&__node_203))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                  }
                return false;
              }
            (*yynode)->variable_name = __node_203;

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

  bool parser::parse_formal_parameter_list(formal_parameter_list_ast **yynode)
  {
    *yynode = create<formal_parameter_list_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DECIMAL
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_OBJECT
        || yytoken == Token_OUT
        || yytoken == Token_PARAMS
        || yytoken == Token_REF
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_STRING
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_USHORT
        || yytoken == Token_VOID
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_LBRACKET
        || yytoken == Token_IDENTIFIER || yytoken == Token_RPAREN
        || yytoken == Token_RBRACKET)
      {
        bool parameter_array_occurred = false;
        if (yytoken == Token_BOOL
            || yytoken == Token_BYTE
            || yytoken == Token_CHAR
            || yytoken == Token_DECIMAL
            || yytoken == Token_DOUBLE
            || yytoken == Token_FLOAT
            || yytoken == Token_INT
            || yytoken == Token_LONG
            || yytoken == Token_OBJECT
            || yytoken == Token_OUT
            || yytoken == Token_PARAMS
            || yytoken == Token_REF
            || yytoken == Token_SBYTE
            || yytoken == Token_SHORT
            || yytoken == Token_STRING
            || yytoken == Token_UINT
            || yytoken == Token_ULONG
            || yytoken == Token_USHORT
            || yytoken == Token_VOID
            || yytoken == Token_ADD
            || yytoken == Token_ALIAS
            || yytoken == Token_GET
            || yytoken == Token_GLOBAL
            || yytoken == Token_PARTIAL
            || yytoken == Token_REMOVE
            || yytoken == Token_SET
            || yytoken == Token_VALUE
            || yytoken == Token_WHERE
            || yytoken == Token_YIELD
            || yytoken == Token_ASSEMBLY
            || yytoken == Token_LBRACKET
            || yytoken == Token_IDENTIFIER)
          {
            std::size_t try_start_token_16 = token_stream->index() - 1;
            parser_state *try_start_state_16 = copy_current_state();
            {
              formal_parameter_ast *__node_204 = 0;
              if (!parse_formal_parameter(&__node_204, &parameter_array_occurred))
                {
                  goto __catch_16;
                }
              (*yynode)->formal_parameter_sequence = snoc((*yynode)->formal_parameter_sequence, __node_204, memory_pool);

              while (yytoken == Token_COMMA)
                {
                  if ( parameter_array_occurred == true )
                    {
                      break;
                    }
                  if (yytoken != Token_COMMA)
                    goto __catch_16;
                  yylex();

                  formal_parameter_ast *__node_205 = 0;
                  if (!parse_formal_parameter(&__node_205, &parameter_array_occurred))
                    {
                      goto __catch_16;
                    }
                  (*yynode)->formal_parameter_sequence = snoc((*yynode)->formal_parameter_sequence, __node_205, memory_pool);

                }
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
                       && yytoken != Token_RPAREN
                       && yytoken != Token_RBRACKET)
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

  bool parser::parse_general_catch_clause(general_catch_clause_ast **yynode)
  {
    *yynode = create<general_catch_clause_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_CATCH)
      {
        if (yytoken != Token_CATCH)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_CATCH, "catch");
              }
            return false;
          }
        yylex();

        block_ast *__node_206 = 0;
        if (!parse_block(&__node_206))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_block, "block");
              }
            return false;
          }
        (*yynode)->body = __node_206;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_generic_dimension_specifier(generic_dimension_specifier_ast **yynode)
  {
    *yynode = create<generic_dimension_specifier_ast>();

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

        (*yynode)->comma_count = 0;
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

            (*yynode)->comma_count++;
          }
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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_global_attribute_section(global_attribute_section_ast **yynode)
  {
    *yynode = create<global_attribute_section_ast>();

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

        if (yytoken != Token_ASSEMBLY)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_ASSEMBLY, "assembly");
              }
            return false;
          }
        yylex();

        if (yytoken != Token_COLON)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_COLON, ":");
              }
            return false;
          }
        yylex();

        attribute_ast *__node_207 = 0;
        if (!parse_attribute(&__node_207))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_attribute, "attribute");
              }
            return false;
          }
        (*yynode)->attribute_sequence = snoc((*yynode)->attribute_sequence, __node_207, memory_pool);

        while (yytoken == Token_COMMA)
          {
            if (LA(2).kind == Token_RBRACKET)
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

            attribute_ast *__node_208 = 0;
            if (!parse_attribute(&__node_208))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_attribute, "attribute");
                  }
                return false;
              }
            (*yynode)->attribute_sequence = snoc((*yynode)->attribute_sequence, __node_208, memory_pool);

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

  bool parser::parse_goto_statement(goto_statement_ast **yynode)
  {
    *yynode = create<goto_statement_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_GOTO)
      {
        if (yytoken != Token_GOTO)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_GOTO, "goto");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_ADD
            || yytoken == Token_ALIAS
            || yytoken == Token_GET
            || yytoken == Token_GLOBAL
            || yytoken == Token_PARTIAL
            || yytoken == Token_REMOVE
            || yytoken == Token_SET
            || yytoken == Token_VALUE
            || yytoken == Token_WHERE
            || yytoken == Token_YIELD
            || yytoken == Token_ASSEMBLY
            || yytoken == Token_IDENTIFIER)
          {
            identifier_ast *__node_209 = 0;
            if (!parse_identifier(&__node_209))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                  }
                return false;
              }
            (*yynode)->label = __node_209;

            (*yynode)->goto_type = goto_statement::type_labeled_statement;
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

            constant_expression_ast *__node_210 = 0;
            if (!parse_constant_expression(&__node_210))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_constant_expression, "constant_expression");
                  }
                return false;
              }
            (*yynode)->constant_expression = __node_210;

            (*yynode)->goto_type = goto_statement::type_switch_case;
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

            (*yynode)->goto_type = goto_statement::type_switch_default;
          }
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

  bool parser::parse_identifier(identifier_ast **yynode)
  {
    *yynode = create<identifier_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_IDENTIFIER)
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
            (*yynode)->ident = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_ADD)
          {
            if (yytoken != Token_ADD)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_ADD, "add");
                  }
                return false;
              }
            (*yynode)->ident = token_stream->index() - 1;
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
            (*yynode)->ident = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_GET)
          {
            if (yytoken != Token_GET)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_GET, "get");
                  }
                return false;
              }
            (*yynode)->ident = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_GLOBAL)
          {
            if (yytoken != Token_GLOBAL)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_GLOBAL, "global");
                  }
                return false;
              }
            (*yynode)->ident = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_PARTIAL)
          {
            if (yytoken != Token_PARTIAL)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_PARTIAL, "partial");
                  }
                return false;
              }
            (*yynode)->ident = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_REMOVE)
          {
            if (yytoken != Token_REMOVE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_REMOVE, "remove");
                  }
                return false;
              }
            (*yynode)->ident = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_SET)
          {
            if (yytoken != Token_SET)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_SET, "set");
                  }
                return false;
              }
            (*yynode)->ident = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_VALUE)
          {
            if (yytoken != Token_VALUE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_VALUE, "value");
                  }
                return false;
              }
            (*yynode)->ident = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_WHERE)
          {
            if (yytoken != Token_WHERE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_WHERE, "where");
                  }
                return false;
              }
            (*yynode)->ident = token_stream->index() - 1;
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
            (*yynode)->ident = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_ASSEMBLY)
          {
            if (yytoken != Token_ASSEMBLY)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_ASSEMBLY, "assembly");
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

        boolean_expression_ast *__node_211 = 0;
        if (!parse_boolean_expression(&__node_211))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_boolean_expression, "boolean_expression");
              }
            return false;
          }
        (*yynode)->condition = __node_211;

        if (yytoken != Token_RPAREN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_RPAREN, ")");
              }
            return false;
          }
        yylex();

        embedded_statement_ast *__node_212 = 0;
        if (!parse_embedded_statement(&__node_212))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_embedded_statement, "embedded_statement");
              }
            return false;
          }
        (*yynode)->if_body = __node_212;

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

            embedded_statement_ast *__node_213 = 0;
            if (!parse_embedded_statement(&__node_213))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_embedded_statement, "embedded_statement");
                  }
                return false;
              }
            (*yynode)->else_body = __node_213;

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

  bool parser::parse_indexer_declaration(indexer_declaration_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers, type_ast *type, type_name_safe_ast *interface_type)
  {
    *yynode = create<indexer_declaration_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    (*yynode)->attributes = attributes;
    if (attributes && attributes->start_token < (*yynode)->start_token)
      (*yynode)->start_token = attributes->start_token;

    (*yynode)->modifiers = modifiers;
    if (modifiers && modifiers->start_token < (*yynode)->start_token)
      (*yynode)->start_token = modifiers->start_token;

    (*yynode)->type = type;
    if (type && type->start_token < (*yynode)->start_token)
      (*yynode)->start_token = type->start_token;

    (*yynode)->interface_type = interface_type;
    if (interface_type && interface_type->start_token < (*yynode)->start_token)
      (*yynode)->start_token = interface_type->start_token;

    if (yytoken == Token_THIS)
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

        if (yytoken != Token_LBRACKET)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_LBRACKET, "[");
              }
            return false;
          }
        yylex();

        formal_parameter_list_ast *__node_214 = 0;
        if (!parse_formal_parameter_list(&__node_214))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_formal_parameter_list, "formal_parameter_list");
              }
            return false;
          }
        (*yynode)->formal_parameters = __node_214;

        if (yytoken != Token_RBRACKET)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_RBRACKET, "]");
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

        accessor_declarations_ast *__node_215 = 0;
        if (!parse_accessor_declarations(&__node_215))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_accessor_declarations, "accessor_declarations");
              }
            return false;
          }
        (*yynode)->accessor_declarations = __node_215;

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

  bool parser::parse_integral_type(integral_type_ast **yynode)
  {
    *yynode = create<integral_type_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_USHORT)
      {
        if (yytoken == Token_SBYTE)
          {
            if (yytoken != Token_SBYTE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_SBYTE, "sbyte");
                  }
                return false;
              }
            yylex();

            (*yynode)->type = integral_type::type_sbyte;
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

            (*yynode)->type = integral_type::type_byte;
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

            (*yynode)->type = integral_type::type_short;
          }
        else if (yytoken == Token_USHORT)
          {
            if (yytoken != Token_USHORT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_USHORT, "ushort");
                  }
                return false;
              }
            yylex();

            (*yynode)->type = integral_type::type_ushort;
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

            (*yynode)->type = integral_type::type_int;
          }
        else if (yytoken == Token_UINT)
          {
            if (yytoken != Token_UINT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_UINT, "uint");
                  }
                return false;
              }
            yylex();

            (*yynode)->type = integral_type::type_uint;
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

            (*yynode)->type = integral_type::type_long;
          }
        else if (yytoken == Token_ULONG)
          {
            if (yytoken != Token_ULONG)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_ULONG, "ulong");
                  }
                return false;
              }
            yylex();

            (*yynode)->type = integral_type::type_ulong;
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

            (*yynode)->type = integral_type::type_char;
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

  bool parser::parse_interface_accessors(interface_accessors_ast **yynode)
  {
    *yynode = create<interface_accessors_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_GET
        || yytoken == Token_SET
        || yytoken == Token_LBRACKET)
      {
        optional_attribute_sections_ast *__node_216 = 0;
        if (!parse_optional_attribute_sections(&__node_216))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_optional_attribute_sections, "optional_attribute_sections");
              }
            return false;
          }
        (*yynode)->accessor1_attributes = __node_216;

        if (yytoken == Token_GET)
          {
            if (yytoken != Token_GET)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_GET, "get");
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

            (*yynode)->accessor1_type = accessor_declarations::type_get;
            if (yytoken == Token_SET
                || yytoken == Token_LBRACKET)
              {
                optional_attribute_sections_ast *__node_217 = 0;
                if (!parse_optional_attribute_sections(&__node_217))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_optional_attribute_sections, "optional_attribute_sections");
                      }
                    return false;
                  }
                (*yynode)->accessor2_attributes = __node_217;

                if (yytoken != Token_SET)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_SET, "set");
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

                (*yynode)->accessor2_type = accessor_declarations::type_set;
              }
            else if (true /*epsilon*/)
              {
                (*yynode)->accessor2_type = accessor_declarations::type_none;
              }
            else
              {
                return false;
              }
          }
        else if (yytoken == Token_SET)
          {
            if (yytoken != Token_SET)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_SET, "set");
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

            (*yynode)->accessor1_type = accessor_declarations::type_set;
            if (yytoken == Token_GET
                || yytoken == Token_LBRACKET)
              {
                optional_attribute_sections_ast *__node_218 = 0;
                if (!parse_optional_attribute_sections(&__node_218))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_optional_attribute_sections, "optional_attribute_sections");
                      }
                    return false;
                  }
                (*yynode)->accessor2_attributes = __node_218;

                if (yytoken != Token_GET)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_GET, "get");
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

                (*yynode)->accessor2_type = accessor_declarations::type_get;
              }
            else if (true /*epsilon*/)
              {
                (*yynode)->accessor2_type = accessor_declarations::type_none;
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

  bool parser::parse_interface_base(interface_base_ast **yynode)
  {
    *yynode = create<interface_base_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

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

        type_name_ast *__node_219 = 0;
        if (!parse_type_name(&__node_219))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_type_name, "type_name");
              }
            return false;
          }
        (*yynode)->interface_type_sequence = snoc((*yynode)->interface_type_sequence, __node_219, memory_pool);

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

            type_name_ast *__node_220 = 0;
            if (!parse_type_name(&__node_220))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_type_name, "type_name");
                  }
                return false;
              }
            (*yynode)->interface_type_sequence = snoc((*yynode)->interface_type_sequence, __node_220, memory_pool);

          }
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

        while (yytoken == Token_BOOL
               || yytoken == Token_BYTE
               || yytoken == Token_CHAR
               || yytoken == Token_DECIMAL
               || yytoken == Token_DOUBLE
               || yytoken == Token_EVENT
               || yytoken == Token_FLOAT
               || yytoken == Token_INT
               || yytoken == Token_LONG
               || yytoken == Token_NEW
               || yytoken == Token_OBJECT
               || yytoken == Token_SBYTE
               || yytoken == Token_SHORT
               || yytoken == Token_STRING
               || yytoken == Token_UINT
               || yytoken == Token_ULONG
               || yytoken == Token_USHORT
               || yytoken == Token_VOID
               || yytoken == Token_ADD
               || yytoken == Token_ALIAS
               || yytoken == Token_GET
               || yytoken == Token_GLOBAL
               || yytoken == Token_PARTIAL
               || yytoken == Token_REMOVE
               || yytoken == Token_SET
               || yytoken == Token_VALUE
               || yytoken == Token_WHERE
               || yytoken == Token_YIELD
               || yytoken == Token_ASSEMBLY
               || yytoken == Token_LBRACKET
               || yytoken == Token_IDENTIFIER)
          {
            std::size_t try_start_token_17 = token_stream->index() - 1;
            parser_state *try_start_state_17 = copy_current_state();
            {
              interface_member_declaration_ast *__node_221 = 0;
              if (!parse_interface_member_declaration(&__node_221))
                {
                  goto __catch_17;
                }
              (*yynode)->member_declaration_sequence = snoc((*yynode)->member_declaration_sequence, __node_221, memory_pool);

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
                       && yytoken != Token_BOOL
                       && yytoken != Token_BYTE
                       && yytoken != Token_CHAR
                       && yytoken != Token_DECIMAL
                       && yytoken != Token_DOUBLE
                       && yytoken != Token_EVENT
                       && yytoken != Token_FLOAT
                       && yytoken != Token_INT
                       && yytoken != Token_LONG
                       && yytoken != Token_NEW
                       && yytoken != Token_OBJECT
                       && yytoken != Token_SBYTE
                       && yytoken != Token_SHORT
                       && yytoken != Token_STRING
                       && yytoken != Token_UINT
                       && yytoken != Token_ULONG
                       && yytoken != Token_USHORT
                       && yytoken != Token_VOID
                       && yytoken != Token_ADD
                       && yytoken != Token_ALIAS
                       && yytoken != Token_GET
                       && yytoken != Token_GLOBAL
                       && yytoken != Token_PARTIAL
                       && yytoken != Token_REMOVE
                       && yytoken != Token_SET
                       && yytoken != Token_VALUE
                       && yytoken != Token_WHERE
                       && yytoken != Token_YIELD
                       && yytoken != Token_ASSEMBLY
                       && yytoken != Token_RBRACE
                       && yytoken != Token_LBRACKET
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

  bool parser::parse_interface_declaration(interface_declaration_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers, bool partial)
  {
    *yynode = create<interface_declaration_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    (*yynode)->attributes = attributes;
    if (attributes && attributes->start_token < (*yynode)->start_token)
      (*yynode)->start_token = attributes->start_token;

    (*yynode)->modifiers = modifiers;
    if (modifiers && modifiers->start_token < (*yynode)->start_token)
      (*yynode)->start_token = modifiers->start_token;

    (*yynode)->partial = partial;
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

        identifier_ast *__node_222 = 0;
        if (!parse_identifier(&__node_222))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_identifier, "identifier");
              }
            return false;
          }
        (*yynode)->interface_name = __node_222;

        if ((yytoken == Token_LESS_THAN) && ( compatibility_mode() >= csharp20_compatibility ))
          {
            std::size_t try_start_token_18 = token_stream->index() - 1;
            parser_state *try_start_state_18 = copy_current_state();
            {
              type_parameters_ast *__node_223 = 0;
              if (!parse_type_parameters(&__node_223))
                {
                  goto __catch_18;
                }
              (*yynode)->type_parameters = __node_223;

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
                       && yytoken != Token_WHERE
                       && yytoken != Token_LBRACE
                       && yytoken != Token_COLON)
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
        if (yytoken == Token_COLON)
          {
            interface_base_ast *__node_224 = 0;
            if (!parse_interface_base(&__node_224))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_interface_base, "interface_base");
                  }
                return false;
              }
            (*yynode)->interface_base = __node_224;

          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        if ((yytoken == Token_WHERE) && ( compatibility_mode() >= csharp20_compatibility ))
          {
            do
              {
                std::size_t try_start_token_19 = token_stream->index() - 1;
                parser_state *try_start_state_19 = copy_current_state();
                {
                  type_parameter_constraints_clause_ast *__node_225 = 0;
                  if (!parse_type_parameter_constraints_clause(&__node_225))
                    {
                      goto __catch_19;
                    }
                  (*yynode)->type_parameter_constraints_sequence = snoc((*yynode)->type_parameter_constraints_sequence, __node_225, memory_pool);

                }
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
                    if (try_start_token_19 == token_stream->index() - 1)
                      yylex();

                    while (yytoken != Token_EOF
                           && yytoken != Token_WHERE
                           && yytoken != Token_LBRACE)
                      {
                        yylex();
                      }
                  }

              }
            while (yytoken == Token_WHERE);
          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        interface_body_ast *__node_226 = 0;
        if (!parse_interface_body(&__node_226))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_interface_body, "interface_body");
              }
            return false;
          }
        (*yynode)->body = __node_226;

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

  bool parser::parse_interface_event_declaration(interface_event_declaration_ast **yynode, optional_attribute_sections_ast *attributes, bool decl_new)
  {
    *yynode = create<interface_event_declaration_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    (*yynode)->attributes = attributes;
    if (attributes && attributes->start_token < (*yynode)->start_token)
      (*yynode)->start_token = attributes->start_token;

    (*yynode)->decl_new = decl_new;
    if (yytoken == Token_EVENT)
      {
        if (yytoken != Token_EVENT)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_EVENT, "event");
              }
            return false;
          }
        yylex();

        type_ast *__node_227 = 0;
        if (!parse_type(&__node_227))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_type, "type");
              }
            return false;
          }
        (*yynode)->event_type = __node_227;

        identifier_ast *__node_228 = 0;
        if (!parse_identifier(&__node_228))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_identifier, "identifier");
              }
            return false;
          }
        (*yynode)->event_name = __node_228;

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

  bool parser::parse_interface_indexer_declaration(interface_indexer_declaration_ast **yynode, optional_attribute_sections_ast *attributes, bool decl_new, type_ast *type)
  {
    *yynode = create<interface_indexer_declaration_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    (*yynode)->attributes = attributes;
    if (attributes && attributes->start_token < (*yynode)->start_token)
      (*yynode)->start_token = attributes->start_token;

    (*yynode)->decl_new = decl_new;
    (*yynode)->type = type;
    if (type && type->start_token < (*yynode)->start_token)
      (*yynode)->start_token = type->start_token;

    if (yytoken == Token_THIS)
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

        if (yytoken != Token_LBRACKET)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_LBRACKET, "[");
              }
            return false;
          }
        yylex();

        formal_parameter_list_ast *__node_229 = 0;
        if (!parse_formal_parameter_list(&__node_229))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_formal_parameter_list, "formal_parameter_list");
              }
            return false;
          }
        (*yynode)->formal_parameters = __node_229;

        if (yytoken != Token_RBRACKET)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_RBRACKET, "]");
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

        interface_accessors_ast *__node_230 = 0;
        if (!parse_interface_accessors(&__node_230))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_interface_accessors, "interface_accessors");
              }
            return false;
          }
        (*yynode)->interface_accessors = __node_230;

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

  bool parser::parse_interface_member_declaration(interface_member_declaration_ast **yynode)
  {
    *yynode = create<interface_member_declaration_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    bool decl_new;

    if (yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DECIMAL
        || yytoken == Token_DOUBLE
        || yytoken == Token_EVENT
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_OBJECT
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_STRING
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_USHORT
        || yytoken == Token_VOID
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_LBRACKET
        || yytoken == Token_IDENTIFIER)
      {
        optional_attribute_sections_ast *attributes = 0;

        return_type_ast *member_type = 0;

        identifier_ast *member_name = 0;

        optional_attribute_sections_ast *__node_231 = 0;
        if (!parse_optional_attribute_sections(&__node_231))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_optional_attribute_sections, "optional_attribute_sections");
              }
            return false;
          }
        attributes = __node_231;

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

            decl_new = true;
          }
        else if (true /*epsilon*/)
          {
            decl_new = false;
          }
        else
          {
            return false;
          }
        if (yytoken == Token_EVENT)
          {
            interface_event_declaration_ast *__node_232 = 0;
            if (!parse_interface_event_declaration(&__node_232,
                                                   attributes, decl_new
                                                  ))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_interface_event_declaration, "interface_event_declaration");
                  }
                return false;
              }
            (*yynode)->event_declaration = __node_232;

          }
        else if (yytoken == Token_BOOL
                 || yytoken == Token_BYTE
                 || yytoken == Token_CHAR
                 || yytoken == Token_DECIMAL
                 || yytoken == Token_DOUBLE
                 || yytoken == Token_FLOAT
                 || yytoken == Token_INT
                 || yytoken == Token_LONG
                 || yytoken == Token_OBJECT
                 || yytoken == Token_SBYTE
                 || yytoken == Token_SHORT
                 || yytoken == Token_STRING
                 || yytoken == Token_UINT
                 || yytoken == Token_ULONG
                 || yytoken == Token_USHORT
                 || yytoken == Token_VOID
                 || yytoken == Token_ADD
                 || yytoken == Token_ALIAS
                 || yytoken == Token_GET
                 || yytoken == Token_GLOBAL
                 || yytoken == Token_PARTIAL
                 || yytoken == Token_REMOVE
                 || yytoken == Token_SET
                 || yytoken == Token_VALUE
                 || yytoken == Token_WHERE
                 || yytoken == Token_YIELD
                 || yytoken == Token_ASSEMBLY
                 || yytoken == Token_IDENTIFIER)
          {
            return_type_ast *__node_233 = 0;
            if (!parse_return_type(&__node_233))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_return_type, "return_type");
                  }
                return false;
              }
            member_type = __node_233;

            if ((yytoken == Token_THIS) && ( member_type->type == return_type::type_regular ))
              {
                interface_indexer_declaration_ast *__node_234 = 0;
                if (!parse_interface_indexer_declaration(&__node_234,
                    attributes, decl_new, member_type->regular_type
                                                        ))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_interface_indexer_declaration, "interface_indexer_declaration");
                      }
                    return false;
                  }
                (*yynode)->indexer_declaration = __node_234;

              }
            else if (yytoken == Token_ADD
                     || yytoken == Token_ALIAS
                     || yytoken == Token_GET
                     || yytoken == Token_GLOBAL
                     || yytoken == Token_PARTIAL
                     || yytoken == Token_REMOVE
                     || yytoken == Token_SET
                     || yytoken == Token_VALUE
                     || yytoken == Token_WHERE
                     || yytoken == Token_YIELD
                     || yytoken == Token_ASSEMBLY
                     || yytoken == Token_IDENTIFIER)
              {
                identifier_ast *__node_235 = 0;
                if (!parse_identifier(&__node_235))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                      }
                    return false;
                  }
                member_name = __node_235;

                if ((yytoken == Token_LBRACE) && ( member_type->type == return_type::type_regular ))
                  {
                    interface_property_declaration_ast *__node_236 = 0;
                    if (!parse_interface_property_declaration(&__node_236,
                        attributes, decl_new,
                        member_type->regular_type, member_name
                                                             ))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_interface_property_declaration, "interface_property_declaration");
                          }
                        return false;
                      }
                    (*yynode)->interface_property_declaration = __node_236;

                  }
                else if (yytoken == Token_LPAREN
                         || yytoken == Token_LESS_THAN)
                  {
                    interface_method_declaration_ast *__node_237 = 0;
                    if (!parse_interface_method_declaration(&__node_237,
                                                            attributes, decl_new, member_type, member_name
                                                           ))
                      {
                        if (!yy_block_errors)
                          {
                            yy_expected_symbol(ast_node::Kind_interface_method_declaration, "interface_method_declaration");
                          }
                        return false;
                      }
                    (*yynode)->interface_method_declaration = __node_237;

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
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_interface_method_declaration(interface_method_declaration_ast **yynode, optional_attribute_sections_ast *attributes, bool decl_new, return_type_ast *return_type, identifier_ast *method_name)
  {
    *yynode = create<interface_method_declaration_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    (*yynode)->attributes = attributes;
    if (attributes && attributes->start_token < (*yynode)->start_token)
      (*yynode)->start_token = attributes->start_token;

    (*yynode)->decl_new = decl_new;
    (*yynode)->return_type = return_type;
    if (return_type && return_type->start_token < (*yynode)->start_token)
      (*yynode)->start_token = return_type->start_token;

    (*yynode)->method_name = method_name;
    if (method_name && method_name->start_token < (*yynode)->start_token)
      (*yynode)->start_token = method_name->start_token;

    if (yytoken == Token_LPAREN
        || yytoken == Token_LESS_THAN)
      {
        if ((yytoken == Token_LESS_THAN) && ( compatibility_mode() >= csharp20_compatibility ))
          {
            std::size_t try_start_token_20 = token_stream->index() - 1;
            parser_state *try_start_state_20 = copy_current_state();
            {
              type_parameters_ast *__node_238 = 0;
              if (!parse_type_parameters(&__node_238))
                {
                  goto __catch_20;
                }
              (*yynode)->type_parameters = __node_238;

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
                       && yytoken != Token_LPAREN)
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
        if (yytoken != Token_LPAREN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_LPAREN, "(");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_BOOL
            || yytoken == Token_BYTE
            || yytoken == Token_CHAR
            || yytoken == Token_DECIMAL
            || yytoken == Token_DOUBLE
            || yytoken == Token_FLOAT
            || yytoken == Token_INT
            || yytoken == Token_LONG
            || yytoken == Token_OBJECT
            || yytoken == Token_OUT
            || yytoken == Token_PARAMS
            || yytoken == Token_REF
            || yytoken == Token_SBYTE
            || yytoken == Token_SHORT
            || yytoken == Token_STRING
            || yytoken == Token_UINT
            || yytoken == Token_ULONG
            || yytoken == Token_USHORT
            || yytoken == Token_VOID
            || yytoken == Token_ADD
            || yytoken == Token_ALIAS
            || yytoken == Token_GET
            || yytoken == Token_GLOBAL
            || yytoken == Token_PARTIAL
            || yytoken == Token_REMOVE
            || yytoken == Token_SET
            || yytoken == Token_VALUE
            || yytoken == Token_WHERE
            || yytoken == Token_YIELD
            || yytoken == Token_ASSEMBLY
            || yytoken == Token_LBRACKET
            || yytoken == Token_IDENTIFIER)
          {
            formal_parameter_list_ast *__node_239 = 0;
            if (!parse_formal_parameter_list(&__node_239))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_formal_parameter_list, "formal_parameter_list");
                  }
                return false;
              }
            (*yynode)->formal_parameters = __node_239;

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

        if ((yytoken == Token_WHERE) && ( compatibility_mode() >= csharp20_compatibility ))
          {
            do
              {
                std::size_t try_start_token_21 = token_stream->index() - 1;
                parser_state *try_start_state_21 = copy_current_state();
                {
                  type_parameter_constraints_clause_ast *__node_240 = 0;
                  if (!parse_type_parameter_constraints_clause(&__node_240))
                    {
                      goto __catch_21;
                    }
                  (*yynode)->type_parameter_constraints_sequence = snoc((*yynode)->type_parameter_constraints_sequence, __node_240, memory_pool);

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
                           && yytoken != Token_WHERE
                           && yytoken != Token_SEMICOLON)
                      {
                        yylex();
                      }
                  }

              }
            while (yytoken == Token_WHERE);
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

  bool parser::parse_interface_property_declaration(interface_property_declaration_ast **yynode, optional_attribute_sections_ast *attributes, bool decl_new, type_ast *type, identifier_ast *property_name)
  {
    *yynode = create<interface_property_declaration_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    (*yynode)->attributes = attributes;
    if (attributes && attributes->start_token < (*yynode)->start_token)
      (*yynode)->start_token = attributes->start_token;

    (*yynode)->decl_new = decl_new;
    (*yynode)->type = type;
    if (type && type->start_token < (*yynode)->start_token)
      (*yynode)->start_token = type->start_token;

    (*yynode)->property_name = property_name;
    if (property_name && property_name->start_token < (*yynode)->start_token)
      (*yynode)->start_token = property_name->start_token;

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

        interface_accessors_ast *__node_241 = 0;
        if (!parse_interface_accessors(&__node_241))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_interface_accessors, "interface_accessors");
              }
            return false;
          }
        (*yynode)->interface_accessors = __node_241;

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

  bool parser::parse_keyword(keyword_ast **yynode)
  {
    *yynode = create<keyword_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_ABSTRACT
        || yytoken == Token_AS
        || yytoken == Token_BASE
        || yytoken == Token_BOOL
        || yytoken == Token_BREAK
        || yytoken == Token_BYTE
        || yytoken == Token_CASE
        || yytoken == Token_CATCH
        || yytoken == Token_CHAR
        || yytoken == Token_CHECKED
        || yytoken == Token_CLASS
        || yytoken == Token_CONST
        || yytoken == Token_CONTINUE
        || yytoken == Token_DECIMAL
        || yytoken == Token_DEFAULT
        || yytoken == Token_DELEGATE
        || yytoken == Token_DO
        || yytoken == Token_DOUBLE
        || yytoken == Token_ELSE
        || yytoken == Token_ENUM
        || yytoken == Token_EVENT
        || yytoken == Token_EXPLICIT
        || yytoken == Token_EXTERN
        || yytoken == Token_FINALLY
        || yytoken == Token_FIXED
        || yytoken == Token_FLOAT
        || yytoken == Token_FOREACH
        || yytoken == Token_FOR
        || yytoken == Token_GOTO
        || yytoken == Token_IF
        || yytoken == Token_IMPLICIT
        || yytoken == Token_IN
        || yytoken == Token_INT
        || yytoken == Token_INTERFACE
        || yytoken == Token_INTERNAL
        || yytoken == Token_IS
        || yytoken == Token_LOCK
        || yytoken == Token_LONG
        || yytoken == Token_NAMESPACE
        || yytoken == Token_NEW
        || yytoken == Token_OBJECT
        || yytoken == Token_OPERATOR
        || yytoken == Token_OUT
        || yytoken == Token_OVERRIDE
        || yytoken == Token_PARAMS
        || yytoken == Token_PRIVATE
        || yytoken == Token_PROTECTED
        || yytoken == Token_PUBLIC
        || yytoken == Token_READONLY
        || yytoken == Token_REF
        || yytoken == Token_RETURN
        || yytoken == Token_SBYTE
        || yytoken == Token_SEALED
        || yytoken == Token_SHORT
        || yytoken == Token_SIZEOF
        || yytoken == Token_STACKALLOC
        || yytoken == Token_STATIC
        || yytoken == Token_STRING
        || yytoken == Token_STRUCT
        || yytoken == Token_SWITCH
        || yytoken == Token_THIS
        || yytoken == Token_THROW
        || yytoken == Token_TRY
        || yytoken == Token_TYPEOF
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_UNCHECKED
        || yytoken == Token_UNSAFE
        || yytoken == Token_USHORT
        || yytoken == Token_USING
        || yytoken == Token_VIRTUAL
        || yytoken == Token_VOID
        || yytoken == Token_VOLATILE
        || yytoken == Token_WHILE)
      {
        if (yytoken == Token_ABSTRACT)
          {
            if (yytoken != Token_ABSTRACT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_ABSTRACT, "abstract");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_AS)
          {
            if (yytoken != Token_AS)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_AS, "as");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_BASE)
          {
            if (yytoken != Token_BASE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_BASE, "base");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_BOOL)
          {
            if (yytoken != Token_BOOL)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_BOOL, "bool");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
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
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

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
            (*yynode)->keyword = token_stream->index() - 1;
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
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_CATCH)
          {
            if (yytoken != Token_CATCH)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_CATCH, "catch");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

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
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_CHECKED)
          {
            if (yytoken != Token_CHECKED)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_CHECKED, "checked");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
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
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_CONST)
          {
            if (yytoken != Token_CONST)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_CONST, "const");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_CONTINUE)
          {
            if (yytoken != Token_CONTINUE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_CONTINUE, "continue");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_DECIMAL)
          {
            if (yytoken != Token_DECIMAL)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_DECIMAL, "decimal");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

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
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_DELEGATE)
          {
            if (yytoken != Token_DELEGATE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_DELEGATE, "delegate");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
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
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

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
            (*yynode)->keyword = token_stream->index() - 1;
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
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_ENUM)
          {
            if (yytoken != Token_ENUM)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_ENUM, "enum");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_EVENT)
          {
            if (yytoken != Token_EVENT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_EVENT, "event");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_EXPLICIT)
          {
            if (yytoken != Token_EXPLICIT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_EXPLICIT, "explicit");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_EXTERN)
          {
            if (yytoken != Token_EXTERN)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_EXTERN, "extern");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

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
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_FIXED)
          {
            if (yytoken != Token_FIXED)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_FIXED, "fixed");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

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
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_FOREACH)
          {
            if (yytoken != Token_FOREACH)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_FOREACH, "foreach");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
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
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_GOTO)
          {
            if (yytoken != Token_GOTO)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_GOTO, "goto");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
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
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_IMPLICIT)
          {
            if (yytoken != Token_IMPLICIT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_IMPLICIT, "implicit");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
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
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

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
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_INTERFACE)
          {
            if (yytoken != Token_INTERFACE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_INTERFACE, "interface");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_INTERNAL)
          {
            if (yytoken != Token_INTERNAL)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_INTERNAL, "internal");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_IS)
          {
            if (yytoken != Token_IS)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_IS, "is");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_LOCK)
          {
            if (yytoken != Token_LOCK)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LOCK, "lock");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

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
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_NAMESPACE)
          {
            if (yytoken != Token_NAMESPACE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_NAMESPACE, "namespace");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_NEW)
          {
            if (yytoken != Token_NEW)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_NEW, "new");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_OBJECT)
          {
            if (yytoken != Token_OBJECT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_OBJECT, "object");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_OPERATOR)
          {
            if (yytoken != Token_OPERATOR)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_OPERATOR, "operator");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_OUT)
          {
            if (yytoken != Token_OUT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_OUT, "out");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_OVERRIDE)
          {
            if (yytoken != Token_OVERRIDE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_OVERRIDE, "override");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_PARAMS)
          {
            if (yytoken != Token_PARAMS)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_PARAMS, "params");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_PRIVATE)
          {
            if (yytoken != Token_PRIVATE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_PRIVATE, "private");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

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
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

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
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_READONLY)
          {
            if (yytoken != Token_READONLY)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_READONLY, "readonly");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_REF)
          {
            if (yytoken != Token_REF)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_REF, "ref");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
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
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_SBYTE)
          {
            if (yytoken != Token_SBYTE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_SBYTE, "sbyte");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_SEALED)
          {
            if (yytoken != Token_SEALED)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_SEALED, "sealed");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

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
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_SIZEOF)
          {
            if (yytoken != Token_SIZEOF)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_SIZEOF, "sizeof");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_STACKALLOC)
          {
            if (yytoken != Token_STACKALLOC)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_STACKALLOC, "stackalloc");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

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
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_STRING)
          {
            if (yytoken != Token_STRING)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_STRING, "string");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_STRUCT)
          {
            if (yytoken != Token_STRUCT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_STRUCT, "struct");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_SWITCH)
          {
            if (yytoken != Token_SWITCH)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_SWITCH, "switch");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

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
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_THROW)
          {
            if (yytoken != Token_THROW)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_THROW, "throw");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_TRY)
          {
            if (yytoken != Token_TRY)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_TRY, "try");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_TYPEOF)
          {
            if (yytoken != Token_TYPEOF)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_TYPEOF, "typeof");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_UINT)
          {
            if (yytoken != Token_UINT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_UINT, "uint");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_ULONG)
          {
            if (yytoken != Token_ULONG)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_ULONG, "ulong");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_UNCHECKED)
          {
            if (yytoken != Token_UNCHECKED)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_UNCHECKED, "unchecked");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_UNSAFE)
          {
            if (yytoken != Token_UNSAFE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_UNSAFE, "unsafe");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_USHORT)
          {
            if (yytoken != Token_USHORT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_USHORT, "ushort");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_USING)
          {
            if (yytoken != Token_USING)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_USING, "using");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_VIRTUAL)
          {
            if (yytoken != Token_VIRTUAL)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_VIRTUAL, "virtual");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

          }
        else if (yytoken == Token_VOID)
          {
            if (yytoken != Token_VOID)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_VOID, "void");
                  }
                return false;
              }
            (*yynode)->keyword = token_stream->index() - 1;
            yylex();

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
            (*yynode)->keyword = token_stream->index() - 1;
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
            (*yynode)->keyword = token_stream->index() - 1;
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

  bool parser::parse_labeled_statement(labeled_statement_ast **yynode)
  {
    *yynode = create<labeled_statement_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_IDENTIFIER)
      {
        identifier_ast *__node_242 = 0;
        if (!parse_identifier(&__node_242))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_identifier, "identifier");
              }
            return false;
          }
        (*yynode)->label = __node_242;

        if (yytoken != Token_COLON)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_COLON, ":");
              }
            return false;
          }
        yylex();

        block_statement_ast *__node_243 = 0;
        if (!parse_block_statement(&__node_243))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_block_statement, "block_statement");
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

  bool parser::parse_literal(literal_ast **yynode)
  {
    *yynode = create<literal_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_REAL_LITERAL
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
        else if (yytoken == Token_REAL_LITERAL)
          {
            if (yytoken != Token_REAL_LITERAL)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_REAL_LITERAL, "real literal");
                  }
                return false;
              }
            (*yynode)->floating_point_literal = token_stream->index() - 1;
            yylex();

            (*yynode)->literal_type = literal::type_real;
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

  bool parser::parse_local_constant_declaration(local_constant_declaration_ast **yynode)
  {
    *yynode = create<local_constant_declaration_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_CONST)
      {
        type_ast *type = 0;

        const list_node<constant_declarator_ast *> *declarator_sequence = 0;

        if (yytoken != Token_CONST)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_CONST, "const");
              }
            return false;
          }
        yylex();

        type_ast *__node_244 = 0;
        if (!parse_type(&__node_244))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_type, "type");
              }
            return false;
          }
        type = __node_244;

        constant_declarator_ast *__node_245 = 0;
        if (!parse_constant_declarator(&__node_245))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_constant_declarator, "constant_declarator");
              }
            return false;
          }
        declarator_sequence = snoc(declarator_sequence, __node_245, memory_pool);

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

            constant_declarator_ast *__node_246 = 0;
            if (!parse_constant_declarator(&__node_246))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_constant_declarator, "constant_declarator");
                  }
                return false;
              }
            declarator_sequence = snoc(declarator_sequence, __node_246, memory_pool);

          }
        constant_declaration_data_ast *__node_247 = 0;
        if (!parse_constant_declaration_data(&__node_247,
                                             0 /* no attributes */, 0 /* no modifiers */, type, declarator_sequence
                                            ))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_constant_declaration_data, "constant_declaration_data");
              }
            return false;
          }
        (*yynode)->data = __node_247;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_local_variable_declaration(local_variable_declaration_ast **yynode)
  {
    *yynode = create<local_variable_declaration_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DECIMAL
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_OBJECT
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_STRING
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_USHORT
        || yytoken == Token_VOID
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_IDENTIFIER)
      {
        type_ast *type = 0;

        const list_node<variable_declarator_ast *> *declarator_sequence = 0;

        type_ast *__node_248 = 0;
        if (!parse_type(&__node_248))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_type, "type");
              }
            return false;
          }
        type = __node_248;

        variable_declarator_ast *__node_249 = 0;
        if (!parse_variable_declarator(&__node_249, false))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_variable_declarator, "variable_declarator");
              }
            return false;
          }
        declarator_sequence = snoc(declarator_sequence, __node_249, memory_pool);

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

            variable_declarator_ast *__node_250 = 0;
            if (!parse_variable_declarator(&__node_250, false))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_variable_declarator, "variable_declarator");
                  }
                return false;
              }
            declarator_sequence = snoc(declarator_sequence, __node_250, memory_pool);

          }
        variable_declaration_data_ast *__node_251 = 0;
        if (!parse_variable_declaration_data(&__node_251,
                                             0 /* no attributes */, 0 /* no modifiers */, type, declarator_sequence
                                            ))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_variable_declaration_data, "variable_declaration_data");
              }
            return false;
          }
        (*yynode)->data = __node_251;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_local_variable_declaration_statement(local_variable_declaration_statement_ast **yynode)
  {
    *yynode = create<local_variable_declaration_statement_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DECIMAL
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_OBJECT
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_STRING
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_USHORT
        || yytoken == Token_VOID
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_IDENTIFIER)
      {
        local_variable_declaration_ast *__node_252 = 0;
        if (!parse_local_variable_declaration(&__node_252))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_local_variable_declaration, "local_variable_declaration");
              }
            return false;
          }
        (*yynode)->declaration = __node_252;

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

  bool parser::parse_lock_statement(lock_statement_ast **yynode)
  {
    *yynode = create<lock_statement_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_LOCK)
      {
        if (yytoken != Token_LOCK)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_LOCK, "lock");
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

        expression_ast *__node_253 = 0;
        if (!parse_expression(&__node_253))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_expression, "expression");
              }
            return false;
          }
        (*yynode)->lock_expression = __node_253;

        if (yytoken != Token_RPAREN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_RPAREN, ")");
              }
            return false;
          }
        yylex();

        embedded_statement_ast *__node_254 = 0;
        if (!parse_embedded_statement(&__node_254))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_embedded_statement, "embedded_statement");
              }
            return false;
          }
        (*yynode)->body = __node_254;

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

    if (yytoken == Token_BASE
        || yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_CHECKED
        || yytoken == Token_DECIMAL
        || yytoken == Token_DEFAULT
        || yytoken == Token_DELEGATE
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_OBJECT
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_SIZEOF
        || yytoken == Token_STRING
        || yytoken == Token_THIS
        || yytoken == Token_TYPEOF
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_UNCHECKED
        || yytoken == Token_USHORT
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_LPAREN
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_STAR
        || yytoken == Token_BIT_AND
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_REAL_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        bit_or_expression_ast *__node_255 = 0;
        if (!parse_bit_or_expression(&__node_255))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_bit_or_expression, "bit_or_expression");
              }
            return false;
          }
        (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_255, memory_pool);

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

            bit_or_expression_ast *__node_256 = 0;
            if (!parse_bit_or_expression(&__node_256))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_bit_or_expression, "bit_or_expression");
                  }
                return false;
              }
            (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_256, memory_pool);

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

    if (yytoken == Token_BASE
        || yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_CHECKED
        || yytoken == Token_DECIMAL
        || yytoken == Token_DEFAULT
        || yytoken == Token_DELEGATE
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_OBJECT
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_SIZEOF
        || yytoken == Token_STRING
        || yytoken == Token_THIS
        || yytoken == Token_TYPEOF
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_UNCHECKED
        || yytoken == Token_USHORT
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_LPAREN
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_STAR
        || yytoken == Token_BIT_AND
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_REAL_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        logical_and_expression_ast *__node_257 = 0;
        if (!parse_logical_and_expression(&__node_257))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_logical_and_expression, "logical_and_expression");
              }
            return false;
          }
        (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_257, memory_pool);

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

            logical_and_expression_ast *__node_258 = 0;
            if (!parse_logical_and_expression(&__node_258))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_logical_and_expression, "logical_and_expression");
                  }
                return false;
              }
            (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_258, memory_pool);

          }
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_managed_type(managed_type_ast **yynode)
  {
    *yynode = create<managed_type_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DECIMAL
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_OBJECT
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_STRING
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_USHORT
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_IDENTIFIER)
      {
        non_array_type_ast *__node_259 = 0;
        if (!parse_non_array_type(&__node_259))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_non_array_type, "non_array_type");
              }
            return false;
          }
        (*yynode)->non_array_type = __node_259;

        while (yytoken == Token_LBRACKET)
          {
            if (LA(2).kind != Token_COMMA && LA(2).kind != Token_RBRACKET)
              {
                break;
              }
            rank_specifier_ast *__node_260 = 0;
            if (!parse_rank_specifier(&__node_260))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_rank_specifier, "rank_specifier");
                  }
                return false;
              }
            (*yynode)->rank_specifier_sequence = snoc((*yynode)->rank_specifier_sequence, __node_260, memory_pool);

          }
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_method_declaration(method_declaration_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers, return_type_ast *return_type, type_name_safe_ast *method_name)
  {
    *yynode = create<method_declaration_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    (*yynode)->attributes = attributes;
    if (attributes && attributes->start_token < (*yynode)->start_token)
      (*yynode)->start_token = attributes->start_token;

    (*yynode)->modifiers = modifiers;
    if (modifiers && modifiers->start_token < (*yynode)->start_token)
      (*yynode)->start_token = modifiers->start_token;

    (*yynode)->return_type = return_type;
    if (return_type && return_type->start_token < (*yynode)->start_token)
      (*yynode)->start_token = return_type->start_token;

    (*yynode)->method_name = method_name;
    if (method_name && method_name->start_token < (*yynode)->start_token)
      (*yynode)->start_token = method_name->start_token;

    if (yytoken == Token_LPAREN
        || yytoken == Token_LESS_THAN)
      {
        if ((yytoken == Token_LESS_THAN) && ( compatibility_mode() >= csharp20_compatibility ))
          {
            std::size_t try_start_token_22 = token_stream->index() - 1;
            parser_state *try_start_state_22 = copy_current_state();
            {
              type_parameters_ast *__node_261 = 0;
              if (!parse_type_parameters(&__node_261))
                {
                  goto __catch_22;
                }
              (*yynode)->type_parameters = __node_261;

            }
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
                if (try_start_token_22 == token_stream->index() - 1)
                  yylex();

                while (yytoken != Token_EOF
                       && yytoken != Token_LPAREN)
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
        if (yytoken != Token_LPAREN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_LPAREN, "(");
              }
            return false;
          }
        yylex();

        if (yytoken == Token_BOOL
            || yytoken == Token_BYTE
            || yytoken == Token_CHAR
            || yytoken == Token_DECIMAL
            || yytoken == Token_DOUBLE
            || yytoken == Token_FLOAT
            || yytoken == Token_INT
            || yytoken == Token_LONG
            || yytoken == Token_OBJECT
            || yytoken == Token_OUT
            || yytoken == Token_PARAMS
            || yytoken == Token_REF
            || yytoken == Token_SBYTE
            || yytoken == Token_SHORT
            || yytoken == Token_STRING
            || yytoken == Token_UINT
            || yytoken == Token_ULONG
            || yytoken == Token_USHORT
            || yytoken == Token_VOID
            || yytoken == Token_ADD
            || yytoken == Token_ALIAS
            || yytoken == Token_GET
            || yytoken == Token_GLOBAL
            || yytoken == Token_PARTIAL
            || yytoken == Token_REMOVE
            || yytoken == Token_SET
            || yytoken == Token_VALUE
            || yytoken == Token_WHERE
            || yytoken == Token_YIELD
            || yytoken == Token_ASSEMBLY
            || yytoken == Token_LBRACKET
            || yytoken == Token_IDENTIFIER)
          {
            formal_parameter_list_ast *__node_262 = 0;
            if (!parse_formal_parameter_list(&__node_262))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_formal_parameter_list, "formal_parameter_list");
                  }
                return false;
              }
            (*yynode)->formal_parameters = __node_262;

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

        if ((yytoken == Token_WHERE) && ( compatibility_mode() >= csharp20_compatibility ))
          {
            do
              {
                std::size_t try_start_token_23 = token_stream->index() - 1;
                parser_state *try_start_state_23 = copy_current_state();
                {
                  type_parameter_constraints_clause_ast *__node_263 = 0;
                  if (!parse_type_parameter_constraints_clause(&__node_263))
                    {
                      goto __catch_23;
                    }
                  (*yynode)->type_parameter_constraints_sequence = snoc((*yynode)->type_parameter_constraints_sequence, __node_263, memory_pool);

                }
                if (try_start_state_23)
                  delete try_start_state_23;

                if (false) // the only way to enter here is using goto
                  {
                  __catch_23:
                    if (try_start_state_23)
                      {
                        restore_state(try_start_state_23);
                        delete try_start_state_23;
                      }
                    if (try_start_token_23 == token_stream->index() - 1)
                      yylex();

                    while (yytoken != Token_EOF
                           && yytoken != Token_WHERE
                           && yytoken != Token_LBRACE
                           && yytoken != Token_SEMICOLON)
                      {
                        yylex();
                      }
                  }

              }
            while (yytoken == Token_WHERE);
          }
        else if (true /*epsilon*/)
        {}
        else
          {
            return false;
          }
        if (yytoken == Token_LBRACE)
          {
            block_ast *__node_264 = 0;
            if (!parse_block(&__node_264))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_block, "block");
                  }
                return false;
              }
            (*yynode)->method_body = __node_264;

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

    if (yytoken == Token_BASE
        || yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_CHECKED
        || yytoken == Token_DECIMAL
        || yytoken == Token_DEFAULT
        || yytoken == Token_DELEGATE
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_OBJECT
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_SIZEOF
        || yytoken == Token_STRING
        || yytoken == Token_THIS
        || yytoken == Token_TYPEOF
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_UNCHECKED
        || yytoken == Token_USHORT
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_LPAREN
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_STAR
        || yytoken == Token_BIT_AND
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_REAL_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        unary_expression_ast *__node_265 = 0;
        if (!parse_unary_expression(&__node_265))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_unary_expression, "unary_expression");
              }
            return false;
          }
        (*yynode)->expression = __node_265;

        while (yytoken == Token_STAR
               || yytoken == Token_SLASH
               || yytoken == Token_REMAINDER)
          {
            multiplicative_expression_rest_ast *__node_266 = 0;
            if (!parse_multiplicative_expression_rest(&__node_266))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_multiplicative_expression_rest, "multiplicative_expression_rest");
                  }
                return false;
              }
            (*yynode)->additional_expression_sequence = snoc((*yynode)->additional_expression_sequence, __node_266, memory_pool);

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
        unary_expression_ast *__node_267 = 0;
        if (!parse_unary_expression(&__node_267))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_unary_expression, "unary_expression");
              }
            return false;
          }
        (*yynode)->expression = __node_267;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_named_argument(named_argument_ast **yynode)
  {
    *yynode = create<named_argument_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_IDENTIFIER)
      {
        identifier_ast *__node_268 = 0;
        if (!parse_identifier(&__node_268))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_identifier, "identifier");
              }
            return false;
          }
        (*yynode)->argument_name = __node_268;

        if (yytoken != Token_ASSIGN)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_ASSIGN, "=");
              }
            return false;
          }
        yylex();

        expression_ast *__node_269 = 0;
        if (!parse_expression(&__node_269))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_expression, "expression");
              }
            return false;
          }
        (*yynode)->attribute_argument_expression = __node_269;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_namespace_body(namespace_body_ast **yynode)
  {
    *yynode = create<namespace_body_ast>();

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

        while (yytoken == Token_EXTERN)
          {
            std::size_t try_start_token_24 = token_stream->index() - 1;
            parser_state *try_start_state_24 = copy_current_state();
            {
              extern_alias_directive_ast *__node_270 = 0;
              if (!parse_extern_alias_directive(&__node_270))
                {
                  goto __catch_24;
                }
              (*yynode)->extern_alias_sequence = snoc((*yynode)->extern_alias_sequence, __node_270, memory_pool);

            }
            if (try_start_state_24)
              delete try_start_state_24;

            if (false) // the only way to enter here is using goto
              {
              __catch_24:
                if (try_start_state_24)
                  {
                    restore_state(try_start_state_24);
                    delete try_start_state_24;
                  }
                if (try_start_token_24 == token_stream->index() - 1)
                  yylex();

                while (yytoken != Token_EOF
                       && yytoken != Token_ABSTRACT
                       && yytoken != Token_CLASS
                       && yytoken != Token_DELEGATE
                       && yytoken != Token_ENUM
                       && yytoken != Token_EXTERN
                       && yytoken != Token_FIXED
                       && yytoken != Token_INTERFACE
                       && yytoken != Token_INTERNAL
                       && yytoken != Token_NAMESPACE
                       && yytoken != Token_NEW
                       && yytoken != Token_OVERRIDE
                       && yytoken != Token_PRIVATE
                       && yytoken != Token_PROTECTED
                       && yytoken != Token_PUBLIC
                       && yytoken != Token_READONLY
                       && yytoken != Token_SEALED
                       && yytoken != Token_STATIC
                       && yytoken != Token_STRUCT
                       && yytoken != Token_UNSAFE
                       && yytoken != Token_USING
                       && yytoken != Token_VIRTUAL
                       && yytoken != Token_VOLATILE
                       && yytoken != Token_PARTIAL
                       && yytoken != Token_RBRACE
                       && yytoken != Token_LBRACKET)
                  {
                    yylex();
                  }
              }

          }
        while (yytoken == Token_USING)
          {
            std::size_t try_start_token_25 = token_stream->index() - 1;
            parser_state *try_start_state_25 = copy_current_state();
            {
              using_directive_ast *__node_271 = 0;
              if (!parse_using_directive(&__node_271))
                {
                  goto __catch_25;
                }
              (*yynode)->using_sequence = snoc((*yynode)->using_sequence, __node_271, memory_pool);

            }
            if (try_start_state_25)
              delete try_start_state_25;

            if (false) // the only way to enter here is using goto
              {
              __catch_25:
                if (try_start_state_25)
                  {
                    restore_state(try_start_state_25);
                    delete try_start_state_25;
                  }
                if (try_start_token_25 == token_stream->index() - 1)
                  yylex();

                while (yytoken != Token_EOF
                       && yytoken != Token_ABSTRACT
                       && yytoken != Token_CLASS
                       && yytoken != Token_DELEGATE
                       && yytoken != Token_ENUM
                       && yytoken != Token_EXTERN
                       && yytoken != Token_FIXED
                       && yytoken != Token_INTERFACE
                       && yytoken != Token_INTERNAL
                       && yytoken != Token_NAMESPACE
                       && yytoken != Token_NEW
                       && yytoken != Token_OVERRIDE
                       && yytoken != Token_PRIVATE
                       && yytoken != Token_PROTECTED
                       && yytoken != Token_PUBLIC
                       && yytoken != Token_READONLY
                       && yytoken != Token_SEALED
                       && yytoken != Token_STATIC
                       && yytoken != Token_STRUCT
                       && yytoken != Token_UNSAFE
                       && yytoken != Token_USING
                       && yytoken != Token_VIRTUAL
                       && yytoken != Token_VOLATILE
                       && yytoken != Token_PARTIAL
                       && yytoken != Token_RBRACE
                       && yytoken != Token_LBRACKET)
                  {
                    yylex();
                  }
              }

          }
        while (yytoken == Token_ABSTRACT
               || yytoken == Token_CLASS
               || yytoken == Token_DELEGATE
               || yytoken == Token_ENUM
               || yytoken == Token_EXTERN
               || yytoken == Token_FIXED
               || yytoken == Token_INTERFACE
               || yytoken == Token_INTERNAL
               || yytoken == Token_NAMESPACE
               || yytoken == Token_NEW
               || yytoken == Token_OVERRIDE
               || yytoken == Token_PRIVATE
               || yytoken == Token_PROTECTED
               || yytoken == Token_PUBLIC
               || yytoken == Token_READONLY
               || yytoken == Token_SEALED
               || yytoken == Token_STATIC
               || yytoken == Token_STRUCT
               || yytoken == Token_UNSAFE
               || yytoken == Token_VIRTUAL
               || yytoken == Token_VOLATILE
               || yytoken == Token_PARTIAL
               || yytoken == Token_LBRACKET)
          {
            std::size_t try_start_token_26 = token_stream->index() - 1;
            parser_state *try_start_state_26 = copy_current_state();
            {
              namespace_member_declaration_ast *__node_272 = 0;
              if (!parse_namespace_member_declaration(&__node_272))
                {
                  goto __catch_26;
                }
              (*yynode)->namespace_sequence = snoc((*yynode)->namespace_sequence, __node_272, memory_pool);

            }
            if (try_start_state_26)
              delete try_start_state_26;

            if (false) // the only way to enter here is using goto
              {
              __catch_26:
                if (try_start_state_26)
                  {
                    restore_state(try_start_state_26);
                    delete try_start_state_26;
                  }
                if (try_start_token_26 == token_stream->index() - 1)
                  yylex();

                while (yytoken != Token_EOF
                       && yytoken != Token_ABSTRACT
                       && yytoken != Token_CLASS
                       && yytoken != Token_DELEGATE
                       && yytoken != Token_ENUM
                       && yytoken != Token_EXTERN
                       && yytoken != Token_FIXED
                       && yytoken != Token_INTERFACE
                       && yytoken != Token_INTERNAL
                       && yytoken != Token_NAMESPACE
                       && yytoken != Token_NEW
                       && yytoken != Token_OVERRIDE
                       && yytoken != Token_PRIVATE
                       && yytoken != Token_PROTECTED
                       && yytoken != Token_PUBLIC
                       && yytoken != Token_READONLY
                       && yytoken != Token_SEALED
                       && yytoken != Token_STATIC
                       && yytoken != Token_STRUCT
                       && yytoken != Token_UNSAFE
                       && yytoken != Token_VIRTUAL
                       && yytoken != Token_VOLATILE
                       && yytoken != Token_PARTIAL
                       && yytoken != Token_RBRACE
                       && yytoken != Token_LBRACKET)
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

  bool parser::parse_namespace_declaration(namespace_declaration_ast **yynode)
  {
    *yynode = create<namespace_declaration_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_NAMESPACE)
      {
        if (yytoken != Token_NAMESPACE)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_NAMESPACE, "namespace");
              }
            return false;
          }
        yylex();

        qualified_identifier_ast *__node_273 = 0;
        if (!parse_qualified_identifier(&__node_273))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_qualified_identifier, "qualified_identifier");
              }
            return false;
          }
        (*yynode)->name = __node_273;

        namespace_body_ast *__node_274 = 0;
        if (!parse_namespace_body(&__node_274))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_namespace_body, "namespace_body");
              }
            return false;
          }
        (*yynode)->body = __node_274;

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

  bool parser::parse_namespace_member_declaration(namespace_member_declaration_ast **yynode)
  {
    *yynode = create<namespace_member_declaration_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_ABSTRACT
        || yytoken == Token_CLASS
        || yytoken == Token_DELEGATE
        || yytoken == Token_ENUM
        || yytoken == Token_EXTERN
        || yytoken == Token_FIXED
        || yytoken == Token_INTERFACE
        || yytoken == Token_INTERNAL
        || yytoken == Token_NAMESPACE
        || yytoken == Token_NEW
        || yytoken == Token_OVERRIDE
        || yytoken == Token_PRIVATE
        || yytoken == Token_PROTECTED
        || yytoken == Token_PUBLIC
        || yytoken == Token_READONLY
        || yytoken == Token_SEALED
        || yytoken == Token_STATIC
        || yytoken == Token_STRUCT
        || yytoken == Token_UNSAFE
        || yytoken == Token_VIRTUAL
        || yytoken == Token_VOLATILE
        || yytoken == Token_PARTIAL
        || yytoken == Token_LBRACKET)
      {
        if (yytoken == Token_NAMESPACE)
          {
            namespace_declaration_ast *__node_275 = 0;
            if (!parse_namespace_declaration(&__node_275))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_namespace_declaration, "namespace_declaration");
                  }
                return false;
              }
            (*yynode)->namespace_declaration = __node_275;

          }
        else if (yytoken == Token_ABSTRACT
                 || yytoken == Token_CLASS
                 || yytoken == Token_DELEGATE
                 || yytoken == Token_ENUM
                 || yytoken == Token_EXTERN
                 || yytoken == Token_FIXED
                 || yytoken == Token_INTERFACE
                 || yytoken == Token_INTERNAL
                 || yytoken == Token_NEW
                 || yytoken == Token_OVERRIDE
                 || yytoken == Token_PRIVATE
                 || yytoken == Token_PROTECTED
                 || yytoken == Token_PUBLIC
                 || yytoken == Token_READONLY
                 || yytoken == Token_SEALED
                 || yytoken == Token_STATIC
                 || yytoken == Token_STRUCT
                 || yytoken == Token_UNSAFE
                 || yytoken == Token_VIRTUAL
                 || yytoken == Token_VOLATILE
                 || yytoken == Token_PARTIAL
                 || yytoken == Token_LBRACKET)
          {
            type_declaration_ast *__node_276 = 0;
            if (!parse_type_declaration(&__node_276))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_type_declaration, "type_declaration");
                  }
                return false;
              }
            (*yynode)->type_declaration = __node_276;

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

  bool parser::parse_namespace_name(namespace_name_ast **yynode)
  {
    *yynode = create<namespace_name_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_IDENTIFIER)
      {
        namespace_or_type_name_ast *__node_277 = 0;
        if (!parse_namespace_or_type_name(&__node_277))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_namespace_or_type_name, "namespace_or_type_name");
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

  bool parser::parse_namespace_or_type_name(namespace_or_type_name_ast **yynode)
  {
    *yynode = create<namespace_or_type_name_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_IDENTIFIER)
      {
        if ((yytoken == Token_ADD
             || yytoken == Token_ALIAS
             || yytoken == Token_GET
             || yytoken == Token_GLOBAL
             || yytoken == Token_PARTIAL
             || yytoken == Token_REMOVE
             || yytoken == Token_SET
             || yytoken == Token_VALUE
             || yytoken == Token_WHERE
             || yytoken == Token_YIELD
             || yytoken == Token_ASSEMBLY
             || yytoken == Token_IDENTIFIER) && ( LA(2).kind == Token_SCOPE ))
          {
            identifier_ast *__node_278 = 0;
            if (!parse_identifier(&__node_278))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                  }
                return false;
              }
            (*yynode)->qualified_alias_label = __node_278;

            if (yytoken != Token_SCOPE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_SCOPE, "::");
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
        namespace_or_type_name_part_ast *__node_279 = 0;
        if (!parse_namespace_or_type_name_part(&__node_279))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_namespace_or_type_name_part, "namespace_or_type_name_part");
              }
            return false;
          }
        (*yynode)->name_part_sequence = snoc((*yynode)->name_part_sequence, __node_279, memory_pool);

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

            namespace_or_type_name_part_ast *__node_280 = 0;
            if (!parse_namespace_or_type_name_part(&__node_280))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_namespace_or_type_name_part, "namespace_or_type_name_part");
                  }
                return false;
              }
            (*yynode)->name_part_sequence = snoc((*yynode)->name_part_sequence, __node_280, memory_pool);

          }
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_namespace_or_type_name_part(namespace_or_type_name_part_ast **yynode)
  {
    *yynode = create<namespace_or_type_name_part_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_IDENTIFIER)
      {
        identifier_ast *__node_281 = 0;
        if (!parse_identifier(&__node_281))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_identifier, "identifier");
              }
            return false;
          }
        (*yynode)->identifier = __node_281;

        bool block_errors_27 = block_errors(true);
        std::size_t try_start_token_27 = token_stream->index() - 1;
        parser_state *try_start_state_27 = copy_current_state();
        {
          if ((yytoken == Token_LESS_THAN) && ( compatibility_mode() >= csharp20_compatibility ))
            {
              type_arguments_ast *__node_282 = 0;
              if (!parse_type_arguments(&__node_282))
                {
                  goto __catch_27;
                }
              (*yynode)->type_arguments = __node_282;

            }
          else if (true /*epsilon*/)
          {}
          else
            {
              goto __catch_27;
            }
        }
        block_errors(block_errors_27);
        if (try_start_state_27)
          delete try_start_state_27;

        if (false) // the only way to enter here is using goto
          {
          __catch_27:
            if (try_start_state_27)
              {
                restore_state(try_start_state_27);
                delete try_start_state_27;
              }
            block_errors(block_errors_27);
            rewind(try_start_token_27);

          }

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_namespace_or_type_name_safe(namespace_or_type_name_safe_ast **yynode)
  {
    *yynode = create<namespace_or_type_name_safe_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_IDENTIFIER)
      {
        if ((yytoken == Token_ADD
             || yytoken == Token_ALIAS
             || yytoken == Token_GET
             || yytoken == Token_GLOBAL
             || yytoken == Token_PARTIAL
             || yytoken == Token_REMOVE
             || yytoken == Token_SET
             || yytoken == Token_VALUE
             || yytoken == Token_WHERE
             || yytoken == Token_YIELD
             || yytoken == Token_ASSEMBLY
             || yytoken == Token_IDENTIFIER) && ( LA(2).kind == Token_SCOPE ))
          {
            identifier_ast *__node_283 = 0;
            if (!parse_identifier(&__node_283))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                  }
                return false;
              }
            (*yynode)->qualified_alias_label = __node_283;

            if (yytoken != Token_SCOPE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_SCOPE, "::");
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
        namespace_or_type_name_part_ast *__node_284 = 0;
        if (!parse_namespace_or_type_name_part(&__node_284))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_namespace_or_type_name_part, "namespace_or_type_name_part");
              }
            return false;
          }
        (*yynode)->name_part_sequence = snoc((*yynode)->name_part_sequence, __node_284, memory_pool);

        while (yytoken == Token_DOT)
          {
            if (LA(2).kind != Token_IDENTIFIER)
              break;
            if (yytoken != Token_DOT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_DOT, ".");
                  }
                return false;
              }
            yylex();

            namespace_or_type_name_part_ast *__node_285 = 0;
            if (!parse_namespace_or_type_name_part(&__node_285))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_namespace_or_type_name_part, "namespace_or_type_name_part");
                  }
                return false;
              }
            (*yynode)->name_part_sequence = snoc((*yynode)->name_part_sequence, __node_285, memory_pool);

          }
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
        type_ast *type = 0;

        if (yytoken != Token_NEW)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_NEW, "new");
              }
            return false;
          }
        yylex();

        type_ast *__node_286 = 0;
        if (!parse_type(&__node_286))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_type, "type");
              }
            return false;
          }
        type = __node_286;

        if (yytoken == Token_LBRACE
            || yytoken == Token_LBRACKET)
          {
            array_creation_expression_rest_ast *__node_287 = 0;
            if (!parse_array_creation_expression_rest(&__node_287, type))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_array_creation_expression_rest, "array_creation_expression_rest");
                  }
                return false;
              }
            (*yynode)->array_creation_expression = __node_287;

          }
        else if (yytoken == Token_LPAREN)
          {
            object_or_delegate_creation_expression_rest_ast *__node_288 = 0;
            if (!parse_object_or_delegate_creation_expression_rest(&__node_288, type))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_object_or_delegate_creation_expression_rest, "object_or_delegate_creation_expression_rest");
                  }
                return false;
              }
            (*yynode)->object_or_delegate_creation_expression = __node_288;

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

    if (yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DECIMAL
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_OBJECT
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_STRING
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_USHORT
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_IDENTIFIER)
      {
        if (yytoken == Token_OBJECT
            || yytoken == Token_STRING)
          {
            builtin_class_type_ast *__node_289 = 0;
            if (!parse_builtin_class_type(&__node_289))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_builtin_class_type, "builtin_class_type");
                  }
                return false;
              }
            (*yynode)->builtin_class_type = __node_289;

          }
        else if (yytoken == Token_BOOL
                 || yytoken == Token_BYTE
                 || yytoken == Token_CHAR
                 || yytoken == Token_DECIMAL
                 || yytoken == Token_DOUBLE
                 || yytoken == Token_FLOAT
                 || yytoken == Token_INT
                 || yytoken == Token_LONG
                 || yytoken == Token_SBYTE
                 || yytoken == Token_SHORT
                 || yytoken == Token_UINT
                 || yytoken == Token_ULONG
                 || yytoken == Token_USHORT
                 || yytoken == Token_ADD
                 || yytoken == Token_ALIAS
                 || yytoken == Token_GET
                 || yytoken == Token_GLOBAL
                 || yytoken == Token_PARTIAL
                 || yytoken == Token_REMOVE
                 || yytoken == Token_SET
                 || yytoken == Token_VALUE
                 || yytoken == Token_WHERE
                 || yytoken == Token_YIELD
                 || yytoken == Token_ASSEMBLY
                 || yytoken == Token_IDENTIFIER)
          {
            optionally_nullable_type_ast *__node_290 = 0;
            if (!parse_optionally_nullable_type(&__node_290))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_optionally_nullable_type, "optionally_nullable_type");
                  }
                return false;
              }
            (*yynode)->optionally_nullable_type = __node_290;

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

  bool parser::parse_non_nullable_type(non_nullable_type_ast **yynode)
  {
    *yynode = create<non_nullable_type_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DECIMAL
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_USHORT
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_IDENTIFIER)
      {
        if (yytoken == Token_ADD
            || yytoken == Token_ALIAS
            || yytoken == Token_GET
            || yytoken == Token_GLOBAL
            || yytoken == Token_PARTIAL
            || yytoken == Token_REMOVE
            || yytoken == Token_SET
            || yytoken == Token_VALUE
            || yytoken == Token_WHERE
            || yytoken == Token_YIELD
            || yytoken == Token_ASSEMBLY
            || yytoken == Token_IDENTIFIER)
          {
            type_name_ast *__node_291 = 0;
            if (!parse_type_name(&__node_291))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_type_name, "type_name");
                  }
                return false;
              }
            (*yynode)->type_name = __node_291;

          }
        else if (yytoken == Token_BOOL
                 || yytoken == Token_BYTE
                 || yytoken == Token_CHAR
                 || yytoken == Token_DECIMAL
                 || yytoken == Token_DOUBLE
                 || yytoken == Token_FLOAT
                 || yytoken == Token_INT
                 || yytoken == Token_LONG
                 || yytoken == Token_SBYTE
                 || yytoken == Token_SHORT
                 || yytoken == Token_UINT
                 || yytoken == Token_ULONG
                 || yytoken == Token_USHORT)
          {
            simple_type_ast *__node_292 = 0;
            if (!parse_simple_type(&__node_292))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_simple_type, "simple_type");
                  }
                return false;
              }
            (*yynode)->simple_type = __node_292;

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

  bool parser::parse_null_coalescing_expression(null_coalescing_expression_ast **yynode)
  {
    *yynode = create<null_coalescing_expression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BASE
        || yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_CHECKED
        || yytoken == Token_DECIMAL
        || yytoken == Token_DEFAULT
        || yytoken == Token_DELEGATE
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_OBJECT
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_SIZEOF
        || yytoken == Token_STRING
        || yytoken == Token_THIS
        || yytoken == Token_TYPEOF
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_UNCHECKED
        || yytoken == Token_USHORT
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_LPAREN
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_STAR
        || yytoken == Token_BIT_AND
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_REAL_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        logical_or_expression_ast *__node_293 = 0;
        if (!parse_logical_or_expression(&__node_293))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_logical_or_expression, "logical_or_expression");
              }
            return false;
          }
        (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_293, memory_pool);

        while (yytoken == Token_QUESTIONQUESTION)
          {
            if (yytoken != Token_QUESTIONQUESTION)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_QUESTIONQUESTION, "??");
                  }
                return false;
              }
            yylex();

            logical_or_expression_ast *__node_294 = 0;
            if (!parse_logical_or_expression(&__node_294))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_logical_or_expression, "logical_or_expression");
                  }
                return false;
              }
            (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_294, memory_pool);

          }
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_numeric_type(numeric_type_ast **yynode)
  {
    *yynode = create<numeric_type_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DECIMAL
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_USHORT)
      {
        if (yytoken == Token_BYTE
            || yytoken == Token_CHAR
            || yytoken == Token_INT
            || yytoken == Token_LONG
            || yytoken == Token_SBYTE
            || yytoken == Token_SHORT
            || yytoken == Token_UINT
            || yytoken == Token_ULONG
            || yytoken == Token_USHORT)
          {
            integral_type_ast *__node_295 = 0;
            if (!parse_integral_type(&__node_295))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_integral_type, "integral_type");
                  }
                return false;
              }
            (*yynode)->int_type = __node_295;

            (*yynode)->type = numeric_type::type_integral;
          }
        else if (yytoken == Token_DOUBLE
                 || yytoken == Token_FLOAT)
          {
            floating_point_type_ast *__node_296 = 0;
            if (!parse_floating_point_type(&__node_296))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_floating_point_type, "floating_point_type");
                  }
                return false;
              }
            (*yynode)->float_type = __node_296;

            (*yynode)->type = numeric_type::type_floating_point;
          }
        else if (yytoken == Token_DECIMAL)
          {
            if (yytoken != Token_DECIMAL)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_DECIMAL, "decimal");
                  }
                return false;
              }
            yylex();

            (*yynode)->type = numeric_type::type_decimal;
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

  bool parser::parse_object_or_delegate_creation_expression_rest(object_or_delegate_creation_expression_rest_ast **yynode, type_ast *type)
  {
    *yynode = create<object_or_delegate_creation_expression_rest_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    (*yynode)->type = type;
    if (type && type->start_token < (*yynode)->start_token)
      (*yynode)->start_token = type->start_token;

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

        optional_argument_list_ast *__node_297 = 0;
        if (!parse_optional_argument_list(&__node_297))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_optional_argument_list, "optional_argument_list");
              }
            return false;
          }
        (*yynode)->argument_list_or_expression = __node_297;

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

  bool parser::parse_optional_argument_list(optional_argument_list_ast **yynode)
  {
    *yynode = create<optional_argument_list_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BASE
        || yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_CHECKED
        || yytoken == Token_DECIMAL
        || yytoken == Token_DEFAULT
        || yytoken == Token_DELEGATE
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_OBJECT
        || yytoken == Token_OUT
        || yytoken == Token_REF
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_SIZEOF
        || yytoken == Token_STRING
        || yytoken == Token_THIS
        || yytoken == Token_TYPEOF
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_UNCHECKED
        || yytoken == Token_USHORT
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_LPAREN
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_STAR
        || yytoken == Token_BIT_AND
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_REAL_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER || yytoken == Token_RPAREN)
      {
        if (yytoken == Token_BASE
            || yytoken == Token_BOOL
            || yytoken == Token_BYTE
            || yytoken == Token_CHAR
            || yytoken == Token_CHECKED
            || yytoken == Token_DECIMAL
            || yytoken == Token_DEFAULT
            || yytoken == Token_DELEGATE
            || yytoken == Token_DOUBLE
            || yytoken == Token_FLOAT
            || yytoken == Token_INT
            || yytoken == Token_LONG
            || yytoken == Token_NEW
            || yytoken == Token_OBJECT
            || yytoken == Token_OUT
            || yytoken == Token_REF
            || yytoken == Token_SBYTE
            || yytoken == Token_SHORT
            || yytoken == Token_SIZEOF
            || yytoken == Token_STRING
            || yytoken == Token_THIS
            || yytoken == Token_TYPEOF
            || yytoken == Token_UINT
            || yytoken == Token_ULONG
            || yytoken == Token_UNCHECKED
            || yytoken == Token_USHORT
            || yytoken == Token_ADD
            || yytoken == Token_ALIAS
            || yytoken == Token_GET
            || yytoken == Token_GLOBAL
            || yytoken == Token_PARTIAL
            || yytoken == Token_REMOVE
            || yytoken == Token_SET
            || yytoken == Token_VALUE
            || yytoken == Token_WHERE
            || yytoken == Token_YIELD
            || yytoken == Token_ASSEMBLY
            || yytoken == Token_LPAREN
            || yytoken == Token_BANG
            || yytoken == Token_TILDE
            || yytoken == Token_INCREMENT
            || yytoken == Token_DECREMENT
            || yytoken == Token_PLUS
            || yytoken == Token_MINUS
            || yytoken == Token_STAR
            || yytoken == Token_BIT_AND
            || yytoken == Token_TRUE
            || yytoken == Token_FALSE
            || yytoken == Token_NULL
            || yytoken == Token_INTEGER_LITERAL
            || yytoken == Token_REAL_LITERAL
            || yytoken == Token_CHARACTER_LITERAL
            || yytoken == Token_STRING_LITERAL
            || yytoken == Token_IDENTIFIER)
          {
            std::size_t try_start_token_28 = token_stream->index() - 1;
            parser_state *try_start_state_28 = copy_current_state();
            {
              argument_ast *__node_298 = 0;
              if (!parse_argument(&__node_298))
                {
                  goto __catch_28;
                }
              (*yynode)->argument_sequence = snoc((*yynode)->argument_sequence, __node_298, memory_pool);

              while (yytoken == Token_COMMA)
                {
                  if (yytoken != Token_COMMA)
                    goto __catch_28;
                  yylex();

                  argument_ast *__node_299 = 0;
                  if (!parse_argument(&__node_299))
                    {
                      goto __catch_28;
                    }
                  (*yynode)->argument_sequence = snoc((*yynode)->argument_sequence, __node_299, memory_pool);

                }
            }
            if (try_start_state_28)
              delete try_start_state_28;

            if (false) // the only way to enter here is using goto
              {
              __catch_28:
                if (try_start_state_28)
                  {
                    restore_state(try_start_state_28);
                    delete try_start_state_28;
                  }
                if (try_start_token_28 == token_stream->index() - 1)
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

  bool parser::parse_optional_attribute_sections(optional_attribute_sections_ast **yynode)
  {
    *yynode = create<optional_attribute_sections_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_LBRACKET || yytoken == Token_ABSTRACT
        || yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_CLASS
        || yytoken == Token_CONST
        || yytoken == Token_DECIMAL
        || yytoken == Token_DELEGATE
        || yytoken == Token_DOUBLE
        || yytoken == Token_ENUM
        || yytoken == Token_EVENT
        || yytoken == Token_EXPLICIT
        || yytoken == Token_EXTERN
        || yytoken == Token_FIXED
        || yytoken == Token_FLOAT
        || yytoken == Token_IMPLICIT
        || yytoken == Token_INT
        || yytoken == Token_INTERFACE
        || yytoken == Token_INTERNAL
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_OBJECT
        || yytoken == Token_OUT
        || yytoken == Token_OVERRIDE
        || yytoken == Token_PARAMS
        || yytoken == Token_PRIVATE
        || yytoken == Token_PROTECTED
        || yytoken == Token_PUBLIC
        || yytoken == Token_READONLY
        || yytoken == Token_REF
        || yytoken == Token_SBYTE
        || yytoken == Token_SEALED
        || yytoken == Token_SHORT
        || yytoken == Token_STATIC
        || yytoken == Token_STRING
        || yytoken == Token_STRUCT
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_UNSAFE
        || yytoken == Token_USHORT
        || yytoken == Token_VIRTUAL
        || yytoken == Token_VOID
        || yytoken == Token_VOLATILE
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_TILDE
        || yytoken == Token_IDENTIFIER)
      {
        while (yytoken == Token_LBRACKET)
          {
            std::size_t try_start_token_29 = token_stream->index() - 1;
            parser_state *try_start_state_29 = copy_current_state();
            {
              attribute_section_ast *__node_300 = 0;
              if (!parse_attribute_section(&__node_300))
                {
                  goto __catch_29;
                }
              (*yynode)->attribute_sequence = snoc((*yynode)->attribute_sequence, __node_300, memory_pool);

            }
            if (try_start_state_29)
              delete try_start_state_29;

            if (false) // the only way to enter here is using goto
              {
              __catch_29:
                if (try_start_state_29)
                  {
                    restore_state(try_start_state_29);
                    delete try_start_state_29;
                  }
                if (try_start_token_29 == token_stream->index() - 1)
                  yylex();

                while (yytoken != Token_EOF
                       && yytoken != Token_ABSTRACT
                       && yytoken != Token_BOOL
                       && yytoken != Token_BYTE
                       && yytoken != Token_CHAR
                       && yytoken != Token_CLASS
                       && yytoken != Token_CONST
                       && yytoken != Token_DECIMAL
                       && yytoken != Token_DELEGATE
                       && yytoken != Token_DOUBLE
                       && yytoken != Token_ENUM
                       && yytoken != Token_EVENT
                       && yytoken != Token_EXPLICIT
                       && yytoken != Token_EXTERN
                       && yytoken != Token_FIXED
                       && yytoken != Token_FLOAT
                       && yytoken != Token_IMPLICIT
                       && yytoken != Token_INT
                       && yytoken != Token_INTERFACE
                       && yytoken != Token_INTERNAL
                       && yytoken != Token_LONG
                       && yytoken != Token_NEW
                       && yytoken != Token_OBJECT
                       && yytoken != Token_OUT
                       && yytoken != Token_OVERRIDE
                       && yytoken != Token_PARAMS
                       && yytoken != Token_PRIVATE
                       && yytoken != Token_PROTECTED
                       && yytoken != Token_PUBLIC
                       && yytoken != Token_READONLY
                       && yytoken != Token_REF
                       && yytoken != Token_SBYTE
                       && yytoken != Token_SEALED
                       && yytoken != Token_SHORT
                       && yytoken != Token_STATIC
                       && yytoken != Token_STRING
                       && yytoken != Token_STRUCT
                       && yytoken != Token_UINT
                       && yytoken != Token_ULONG
                       && yytoken != Token_UNSAFE
                       && yytoken != Token_USHORT
                       && yytoken != Token_VIRTUAL
                       && yytoken != Token_VOID
                       && yytoken != Token_VOLATILE
                       && yytoken != Token_ADD
                       && yytoken != Token_ALIAS
                       && yytoken != Token_GET
                       && yytoken != Token_GLOBAL
                       && yytoken != Token_PARTIAL
                       && yytoken != Token_REMOVE
                       && yytoken != Token_SET
                       && yytoken != Token_VALUE
                       && yytoken != Token_WHERE
                       && yytoken != Token_YIELD
                       && yytoken != Token_ASSEMBLY
                       && yytoken != Token_LBRACKET
                       && yytoken != Token_TILDE
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

  bool parser::parse_optional_modifiers(optional_modifiers_ast **yynode)
  {
    *yynode = create<optional_modifiers_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_ABSTRACT
        || yytoken == Token_EXTERN
        || yytoken == Token_FIXED
        || yytoken == Token_INTERNAL
        || yytoken == Token_NEW
        || yytoken == Token_OVERRIDE
        || yytoken == Token_PRIVATE
        || yytoken == Token_PROTECTED
        || yytoken == Token_PUBLIC
        || yytoken == Token_READONLY
        || yytoken == Token_SEALED
        || yytoken == Token_STATIC
        || yytoken == Token_UNSAFE
        || yytoken == Token_VIRTUAL
        || yytoken == Token_VOLATILE || yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_CLASS
        || yytoken == Token_CONST
        || yytoken == Token_DECIMAL
        || yytoken == Token_DELEGATE
        || yytoken == Token_DOUBLE
        || yytoken == Token_ENUM
        || yytoken == Token_EVENT
        || yytoken == Token_EXPLICIT
        || yytoken == Token_FLOAT
        || yytoken == Token_IMPLICIT
        || yytoken == Token_INT
        || yytoken == Token_INTERFACE
        || yytoken == Token_LONG
        || yytoken == Token_OBJECT
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_STRING
        || yytoken == Token_STRUCT
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_USHORT
        || yytoken == Token_VOID
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_TILDE
        || yytoken == Token_IDENTIFIER)
      {
        while (yytoken == Token_ABSTRACT
               || yytoken == Token_EXTERN
               || yytoken == Token_FIXED
               || yytoken == Token_INTERNAL
               || yytoken == Token_NEW
               || yytoken == Token_OVERRIDE
               || yytoken == Token_PRIVATE
               || yytoken == Token_PROTECTED
               || yytoken == Token_PUBLIC
               || yytoken == Token_READONLY
               || yytoken == Token_SEALED
               || yytoken == Token_STATIC
               || yytoken == Token_UNSAFE
               || yytoken == Token_VIRTUAL
               || yytoken == Token_VOLATILE)
          {
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

                (*yynode)->modifiers |= modifiers::mod_new;
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
            else if (yytoken == Token_INTERNAL)
              {
                if (yytoken != Token_INTERNAL)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_INTERNAL, "internal");
                      }
                    return false;
                  }
                yylex();

                (*yynode)->modifiers |= modifiers::mod_internal;
              }
            else if (yytoken == Token_PRIVATE)
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
            else if (yytoken == Token_SEALED)
              {
                if (yytoken != Token_SEALED)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_SEALED, "sealed");
                      }
                    return false;
                  }
                yylex();

                (*yynode)->modifiers |= modifiers::mod_sealed;
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
            else if (yytoken == Token_READONLY)
              {
                if (yytoken != Token_READONLY)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_READONLY, "readonly");
                      }
                    return false;
                  }
                yylex();

                (*yynode)->modifiers |= modifiers::mod_readonly;
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
            else if (yytoken == Token_VIRTUAL)
              {
                if (yytoken != Token_VIRTUAL)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_VIRTUAL, "virtual");
                      }
                    return false;
                  }
                yylex();

                (*yynode)->modifiers |= modifiers::mod_virtual;
              }
            else if (yytoken == Token_OVERRIDE)
              {
                if (yytoken != Token_OVERRIDE)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_OVERRIDE, "override");
                      }
                    return false;
                  }
                yylex();

                (*yynode)->modifiers |= modifiers::mod_override;
              }
            else if (yytoken == Token_EXTERN)
              {
                if (yytoken != Token_EXTERN)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_EXTERN, "extern");
                      }
                    return false;
                  }
                yylex();

                (*yynode)->modifiers |= modifiers::mod_extern;
              }
            else if (yytoken == Token_UNSAFE)
              {
                if (yytoken != Token_UNSAFE)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_UNSAFE, "unsafe");
                      }
                    return false;
                  }
                yylex();

                (*yynode)->modifiers |= modifiers::mod_unsafe;
              }
            else if ((yytoken == Token_FIXED) && ( compatibility_mode() >= csharp20_compatibility ))
              {
                if (yytoken != Token_FIXED)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_FIXED, "fixed");
                      }
                    return false;
                  }
                yylex();

                (*yynode)->modifiers |= modifiers::mod_fixed;
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

  bool parser::parse_optionally_nullable_type(optionally_nullable_type_ast **yynode)
  {
    *yynode = create<optionally_nullable_type_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DECIMAL
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_USHORT
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_IDENTIFIER)
      {
        non_nullable_type_ast *__node_301 = 0;
        if (!parse_non_nullable_type(&__node_301))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_non_nullable_type, "non_nullable_type");
              }
            return false;
          }
        (*yynode)->non_nullable_type = __node_301;

        if ((yytoken == Token_QUESTION) && ( compatibility_mode() >= csharp20_compatibility ))
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

            (*yynode)->nullable = true;
          }
        else if (true /*epsilon*/)
          {
            (*yynode)->nullable = false;
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

  bool parser::parse_overloadable_binary_only_operator(overloadable_binary_only_operator_ast **yynode, overloadable_operator::overloadable_operator_enum* op)
  {
    *yynode = create<overloadable_binary_only_operator_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_EQUAL
        || yytoken == Token_LESS_THAN
        || yytoken == Token_LESS_EQUAL
        || yytoken == Token_GREATER_THAN
        || yytoken == Token_GREATER_EQUAL
        || yytoken == Token_NOT_EQUAL
        || yytoken == Token_STAR
        || yytoken == Token_SLASH
        || yytoken == Token_BIT_AND
        || yytoken == Token_BIT_OR
        || yytoken == Token_BIT_XOR
        || yytoken == Token_REMAINDER
        || yytoken == Token_LSHIFT
        || yytoken == Token_RSHIFT)
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
            (*yynode)->op = token_stream->index() - 1;
            yylex();

            *op = overloadable_operator::op_star;
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
            (*yynode)->op = token_stream->index() - 1;
            yylex();

            *op = overloadable_operator::op_slash;
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
            (*yynode)->op = token_stream->index() - 1;
            yylex();

            *op = overloadable_operator::op_remainder;
          }
        else if (yytoken == Token_BIT_AND)
          {
            if (yytoken != Token_BIT_AND)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_BIT_AND, "&");
                  }
                return false;
              }
            (*yynode)->op = token_stream->index() - 1;
            yylex();

            *op = overloadable_operator::op_bit_and;
          }
        else if (yytoken == Token_BIT_OR)
          {
            if (yytoken != Token_BIT_OR)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_BIT_OR, "|");
                  }
                return false;
              }
            (*yynode)->op = token_stream->index() - 1;
            yylex();

            *op = overloadable_operator::op_bit_or;
          }
        else if (yytoken == Token_BIT_XOR)
          {
            if (yytoken != Token_BIT_XOR)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_BIT_XOR, "^");
                  }
                return false;
              }
            (*yynode)->op = token_stream->index() - 1;
            yylex();

            *op = overloadable_operator::op_bit_xor;
          }
        else if (yytoken == Token_LSHIFT)
          {
            if (yytoken != Token_LSHIFT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LSHIFT, "<<");
                  }
                return false;
              }
            (*yynode)->op = token_stream->index() - 1;
            yylex();

            *op = overloadable_operator::op_lshift;
          }
        else if (yytoken == Token_RSHIFT)
          {
            if (yytoken != Token_RSHIFT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_RSHIFT, ">>");
                  }
                return false;
              }
            (*yynode)->op = token_stream->index() - 1;
            yylex();

            *op = overloadable_operator::op_rshift;
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
            (*yynode)->op = token_stream->index() - 1;
            yylex();

            *op = overloadable_operator::op_equal;
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
            (*yynode)->op = token_stream->index() - 1;
            yylex();

            *op = overloadable_operator::op_not_equal;
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
            (*yynode)->op = token_stream->index() - 1;
            yylex();

            *op = overloadable_operator::op_greater_than;
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
            (*yynode)->op = token_stream->index() - 1;
            yylex();

            *op = overloadable_operator::op_less_than;
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
            (*yynode)->op = token_stream->index() - 1;
            yylex();

            *op = overloadable_operator::op_greater_equal;
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
            (*yynode)->op = token_stream->index() - 1;
            yylex();

            *op = overloadable_operator::op_less_equal;
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

  bool parser::parse_overloadable_unary_only_operator(overloadable_unary_only_operator_ast **yynode, overloadable_operator::overloadable_operator_enum* op)
  {
    *yynode = create<overloadable_unary_only_operator_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE)
      {
        if (yytoken == Token_BANG)
          {
            if (yytoken != Token_BANG)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_BANG, "!");
                  }
                return false;
              }
            (*yynode)->op = token_stream->index() - 1;
            yylex();

            *op = overloadable_operator::op_bang;
          }
        else if (yytoken == Token_TILDE)
          {
            if (yytoken != Token_TILDE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_TILDE, "~");
                  }
                return false;
              }
            (*yynode)->op = token_stream->index() - 1;
            yylex();

            *op = overloadable_operator::op_tilde;
          }
        else if (yytoken == Token_INCREMENT)
          {
            if (yytoken != Token_INCREMENT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_INCREMENT, "++");
                  }
                return false;
              }
            (*yynode)->op = token_stream->index() - 1;
            yylex();

            *op = overloadable_operator::op_increment;
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
            (*yynode)->op = token_stream->index() - 1;
            yylex();

            *op = overloadable_operator::op_decrement;
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
            (*yynode)->op = token_stream->index() - 1;
            yylex();

            *op = overloadable_operator::op_true;
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
            (*yynode)->op = token_stream->index() - 1;
            yylex();

            *op = overloadable_operator::op_false;
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

  bool parser::parse_overloadable_unary_or_binary_operator(overloadable_unary_or_binary_operator_ast **yynode, overloadable_operator::overloadable_operator_enum* op)
  {
    *yynode = create<overloadable_unary_or_binary_operator_ast>();

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
            (*yynode)->op = token_stream->index() - 1;
            yylex();

            *op = overloadable_operator::op_plus;
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
            (*yynode)->op = token_stream->index() - 1;
            yylex();

            *op = overloadable_operator::op_minus;
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

  bool parser::parse_parameter_array(parameter_array_ast **yynode)
  {
    *yynode = create<parameter_array_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_PARAMS)
      {
        if (yytoken != Token_PARAMS)
          {
            if (!yy_block_errors)
              {
                yy_expected_token(yytoken, Token_PARAMS, "params");
              }
            return false;
          }
        yylex();

        array_type_ast *__node_302 = 0;
        if (!parse_array_type(&__node_302))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_array_type, "array_type");
              }
            return false;
          }
        (*yynode)->type = __node_302;

        identifier_ast *__node_303 = 0;
        if (!parse_identifier(&__node_303))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_identifier, "identifier");
              }
            return false;
          }
        (*yynode)->variable_name = __node_303;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_parameter_modifier(parameter_modifier_ast **yynode)
  {
    *yynode = create<parameter_modifier_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_OUT
        || yytoken == Token_REF)
      {
        if (yytoken == Token_REF)
          {
            if (yytoken != Token_REF)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_REF, "ref");
                  }
                return false;
              }
            yylex();

            (*yynode)->modifier = parameter_modifier::mod_ref;
          }
        else if (yytoken == Token_OUT)
          {
            if (yytoken != Token_OUT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_OUT, "out");
                  }
                return false;
              }
            yylex();

            (*yynode)->modifier = parameter_modifier::mod_out;
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

  bool parser::parse_pointer_type(pointer_type_ast **yynode)
  {
    *yynode = create<pointer_type_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DECIMAL
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_OBJECT
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_STRING
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_USHORT
        || yytoken == Token_VOID
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_IDENTIFIER)
      {
        if (yytoken == Token_BOOL
            || yytoken == Token_BYTE
            || yytoken == Token_CHAR
            || yytoken == Token_DECIMAL
            || yytoken == Token_DOUBLE
            || yytoken == Token_FLOAT
            || yytoken == Token_INT
            || yytoken == Token_LONG
            || yytoken == Token_OBJECT
            || yytoken == Token_SBYTE
            || yytoken == Token_SHORT
            || yytoken == Token_STRING
            || yytoken == Token_UINT
            || yytoken == Token_ULONG
            || yytoken == Token_USHORT
            || yytoken == Token_ADD
            || yytoken == Token_ALIAS
            || yytoken == Token_GET
            || yytoken == Token_GLOBAL
            || yytoken == Token_PARTIAL
            || yytoken == Token_REMOVE
            || yytoken == Token_SET
            || yytoken == Token_VALUE
            || yytoken == Token_WHERE
            || yytoken == Token_YIELD
            || yytoken == Token_ASSEMBLY
            || yytoken == Token_IDENTIFIER)
          {
            non_array_type_ast *__node_304 = 0;
            if (!parse_non_array_type(&__node_304))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_non_array_type, "non_array_type");
                  }
                return false;
              }
            (*yynode)->regular_type = __node_304;

            unmanaged_type_suffix_ast *__node_305 = 0;
            if (!parse_unmanaged_type_suffix(&__node_305))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_unmanaged_type_suffix, "unmanaged_type_suffix");
                  }
                return false;
              }
            (*yynode)->unmanaged_type_suffix_sequence = snoc((*yynode)->unmanaged_type_suffix_sequence, __node_305, memory_pool);

            (*yynode)->type = pointer_type::type_regular;
          }
        else if (yytoken == Token_VOID)
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

            if (yytoken != Token_STAR)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_STAR, "*");
                  }
                return false;
              }
            yylex();

            (*yynode)->type = pointer_type::type_void_star;
          }
        else
          {
            return false;
          }
        while (yytoken == Token_LBRACKET
               || yytoken == Token_STAR)
          {
            if (yytoken == Token_LBRACKET &&
                LA(2).kind != Token_COMMA && LA(2).kind != Token_RBRACKET)
              {
                break;
              }
            unmanaged_type_suffix_ast *__node_306 = 0;
            if (!parse_unmanaged_type_suffix(&__node_306))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_unmanaged_type_suffix, "unmanaged_type_suffix");
                  }
                return false;
              }
            (*yynode)->unmanaged_type_suffix_sequence = snoc((*yynode)->unmanaged_type_suffix_sequence, __node_306, memory_pool);

          }
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_positional_argument(positional_argument_ast **yynode)
  {
    *yynode = create<positional_argument_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BASE
        || yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_CHECKED
        || yytoken == Token_DECIMAL
        || yytoken == Token_DEFAULT
        || yytoken == Token_DELEGATE
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_OBJECT
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_SIZEOF
        || yytoken == Token_STRING
        || yytoken == Token_THIS
        || yytoken == Token_TYPEOF
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_UNCHECKED
        || yytoken == Token_USHORT
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_LPAREN
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_STAR
        || yytoken == Token_BIT_AND
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_REAL_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        expression_ast *__node_307 = 0;
        if (!parse_expression(&__node_307))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_expression, "expression");
              }
            return false;
          }
        (*yynode)->attribute_argument_expression = __node_307;

      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_predefined_type(predefined_type_ast **yynode)
  {
    *yynode = create<predefined_type_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_DECIMAL
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_OBJECT
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_STRING
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_USHORT)
      {
        if (yytoken == Token_BOOL)
          {
            if (yytoken != Token_BOOL)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_BOOL, "bool");
                  }
                return false;
              }
            yylex();

            (*yynode)->type = predefined_type::type_bool;
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

            (*yynode)->type = predefined_type::type_byte;
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

            (*yynode)->type = predefined_type::type_char;
          }
        else if (yytoken == Token_DECIMAL)
          {
            if (yytoken != Token_DECIMAL)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_DECIMAL, "decimal");
                  }
                return false;
              }
            yylex();

            (*yynode)->type = predefined_type::type_decimal;
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

            (*yynode)->type = predefined_type::type_double;
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

            (*yynode)->type = predefined_type::type_float;
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

            (*yynode)->type = predefined_type::type_int;
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

            (*yynode)->type = predefined_type::type_long;
          }
        else if (yytoken == Token_OBJECT)
          {
            if (yytoken != Token_OBJECT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_OBJECT, "object");
                  }
                return false;
              }
            yylex();

            (*yynode)->type = predefined_type::type_object;
          }
        else if (yytoken == Token_SBYTE)
          {
            if (yytoken != Token_SBYTE)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_SBYTE, "sbyte");
                  }
                return false;
              }
            yylex();

            (*yynode)->type = predefined_type::type_sbyte;
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

            (*yynode)->type = predefined_type::type_short;
          }
        else if (yytoken == Token_STRING)
          {
            if (yytoken != Token_STRING)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_STRING, "string");
                  }
                return false;
              }
            yylex();

            (*yynode)->type = predefined_type::type_string;
          }
        else if (yytoken == Token_UINT)
          {
            if (yytoken != Token_UINT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_UINT, "uint");
                  }
                return false;
              }
            yylex();

            (*yynode)->type = predefined_type::type_uint;
          }
        else if (yytoken == Token_ULONG)
          {
            if (yytoken != Token_ULONG)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_ULONG, "ulong");
                  }
                return false;
              }
            yylex();

            (*yynode)->type = predefined_type::type_ulong;
          }
        else if (yytoken == Token_USHORT)
          {
            if (yytoken != Token_USHORT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_USHORT, "ushort");
                  }
                return false;
              }
            yylex();

            (*yynode)->type = predefined_type::type_ushort;
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

    if (yytoken == Token_BASE
        || yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_CHECKED
        || yytoken == Token_DECIMAL
        || yytoken == Token_DEFAULT
        || yytoken == Token_DELEGATE
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_OBJECT
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_SIZEOF
        || yytoken == Token_STRING
        || yytoken == Token_THIS
        || yytoken == Token_TYPEOF
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_UNCHECKED
        || yytoken == Token_USHORT
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_LPAREN
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_REAL_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        if (yytoken == Token_TRUE
            || yytoken == Token_FALSE
            || yytoken == Token_NULL
            || yytoken == Token_INTEGER_LITERAL
            || yytoken == Token_REAL_LITERAL
            || yytoken == Token_CHARACTER_LITERAL
            || yytoken == Token_STRING_LITERAL)
          {
            literal_ast *__node_308 = 0;
            if (!parse_literal(&__node_308))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_literal, "literal");
                  }
                return false;
              }
            (*yynode)->literal = __node_308;

            (*yynode)->rule_type = primary_atom::type_literal;
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

            expression_ast *__node_309 = 0;
            if (!parse_expression(&__node_309))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_expression, "expression");
                  }
                return false;
              }
            (*yynode)->expression = __node_309;

            if (yytoken != Token_RPAREN)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_RPAREN, ")");
                  }
                return false;
              }
            yylex();

            (*yynode)->rule_type = primary_atom::type_parenthesized_expression;
          }
        else if (yytoken == Token_BOOL
                 || yytoken == Token_BYTE
                 || yytoken == Token_CHAR
                 || yytoken == Token_DECIMAL
                 || yytoken == Token_DOUBLE
                 || yytoken == Token_FLOAT
                 || yytoken == Token_INT
                 || yytoken == Token_LONG
                 || yytoken == Token_OBJECT
                 || yytoken == Token_SBYTE
                 || yytoken == Token_SHORT
                 || yytoken == Token_STRING
                 || yytoken == Token_UINT
                 || yytoken == Token_ULONG
                 || yytoken == Token_USHORT
                 || yytoken == Token_ADD
                 || yytoken == Token_ALIAS
                 || yytoken == Token_GET
                 || yytoken == Token_GLOBAL
                 || yytoken == Token_PARTIAL
                 || yytoken == Token_REMOVE
                 || yytoken == Token_SET
                 || yytoken == Token_VALUE
                 || yytoken == Token_WHERE
                 || yytoken == Token_YIELD
                 || yytoken == Token_ASSEMBLY
                 || yytoken == Token_IDENTIFIER)
          {
            simple_name_or_member_access_ast *__node_310 = 0;
            if (!parse_simple_name_or_member_access(&__node_310))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_simple_name_or_member_access, "simple_name_or_member_access");
                  }
                return false;
              }
            (*yynode)->simple_name_or_member_access = __node_310;

            (*yynode)->rule_type = primary_atom::type_member_access;
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

            (*yynode)->rule_type = primary_atom::type_this_access;
          }
        else if (yytoken == Token_BASE)
          {
            base_access_ast *__node_311 = 0;
            if (!parse_base_access(&__node_311))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_base_access, "base_access");
                  }
                return false;
              }
            (*yynode)->base_access = __node_311;

            (*yynode)->rule_type = primary_atom::type_base_access;
          }
        else if (yytoken == Token_NEW)
          {
            new_expression_ast *__node_312 = 0;
            if (!parse_new_expression(&__node_312))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_new_expression, "new_expression");
                  }
                return false;
              }
            (*yynode)->new_expression = __node_312;

            (*yynode)->rule_type = primary_atom::type_new_expression;
          }
        else if (yytoken == Token_TYPEOF)
          {
            typeof_expression_ast *__node_313 = 0;
            if (!parse_typeof_expression(&__node_313))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_typeof_expression, "typeof_expression");
                  }
                return false;
              }
            (*yynode)->typeof_expression = __node_313;

            (*yynode)->rule_type = primary_atom::type_typeof_expression;
          }
        else if (yytoken == Token_CHECKED)
          {
            if (yytoken != Token_CHECKED)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_CHECKED, "checked");
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

            expression_ast *__node_314 = 0;
            if (!parse_expression(&__node_314))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_expression, "expression");
                  }
                return false;
              }
            (*yynode)->expression = __node_314;

            if (yytoken != Token_RPAREN)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_RPAREN, ")");
                  }
                return false;
              }
            yylex();

            (*yynode)->rule_type = primary_atom::type_checked_expression;
          }
        else if (yytoken == Token_UNCHECKED)
          {
            if (yytoken != Token_UNCHECKED)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_UNCHECKED, "unchecked");
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

            expression_ast *__node_315 = 0;
            if (!parse_expression(&__node_315))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_expression, "expression");
                  }
                return false;
              }
            (*yynode)->expression = __node_315;

            if (yytoken != Token_RPAREN)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_RPAREN, ")");
                  }
                return false;
              }
            yylex();

            (*yynode)->rule_type = primary_atom::type_unchecked_expression;
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

            if (yytoken != Token_LPAREN)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_LPAREN, "(");
                  }
                return false;
              }
            yylex();

            type_ast *__node_316 = 0;
            if (!parse_type(&__node_316))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_type, "type");
                  }
                return false;
              }
            (*yynode)->type = __node_316;

            if (yytoken != Token_RPAREN)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_RPAREN, ")");
                  }
                return false;
              }
            yylex();

            (*yynode)->rule_type = primary_atom::type_default_value_expression;
          }
        else if ((yytoken == Token_DELEGATE) && ( compatibility_mode() >= csharp20_compatibility ))
          {
            anonymous_method_expression_ast *__node_317 = 0;
            if (!parse_anonymous_method_expression(&__node_317))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_anonymous_method_expression, "anonymous_method_expression");
                  }
                return false;
              }
            (*yynode)->anonymous_method_expression = __node_317;

            (*yynode)->rule_type = primary_atom::type_anonymous_method_expression;
          }
        else if (yytoken == Token_SIZEOF)
          {
            if (yytoken != Token_SIZEOF)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_SIZEOF, "sizeof");
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

            unmanaged_type_ast *__node_318 = 0;
            if (!parse_unmanaged_type(&__node_318))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_unmanaged_type, "unmanaged_type");
                  }
                return false;
              }
            (*yynode)->unmanaged_type = __node_318;

            if (yytoken != Token_RPAREN)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_RPAREN, ")");
                  }
                return false;
              }
            yylex();

            (*yynode)->rule_type = primary_atom::type_sizeof_expression;
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

    if (yytoken == Token_BASE
        || yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_CHECKED
        || yytoken == Token_DECIMAL
        || yytoken == Token_DEFAULT
        || yytoken == Token_DELEGATE
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_OBJECT
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_SIZEOF
        || yytoken == Token_STRING
        || yytoken == Token_THIS
        || yytoken == Token_TYPEOF
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_UNCHECKED
        || yytoken == Token_USHORT
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_LPAREN
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_REAL_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        primary_atom_ast *__node_319 = 0;
        if (!parse_primary_atom(&__node_319))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_primary_atom, "primary_atom");
              }
            return false;
          }
        (*yynode)->primary_atom = __node_319;

        while (yytoken == Token_LPAREN
               || yytoken == Token_LBRACKET
               || yytoken == Token_DOT
               || yytoken == Token_ARROW_RIGHT
               || yytoken == Token_INCREMENT
               || yytoken == Token_DECREMENT)
          {
            primary_suffix_ast *__node_320 = 0;
            if (!parse_primary_suffix(&__node_320))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_primary_suffix, "primary_suffix");
                  }
                return false;
              }
            (*yynode)->primary_suffix_sequence = snoc((*yynode)->primary_suffix_sequence, __node_320, memory_pool);

          }
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_primary_or_secondary_constraint(primary_or_secondary_constraint_ast **yynode)
  {
    *yynode = create<primary_or_secondary_constraint_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_CLASS
        || yytoken == Token_OBJECT
        || yytoken == Token_STRING
        || yytoken == Token_STRUCT
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_IDENTIFIER)
      {
        if (yytoken == Token_OBJECT
            || yytoken == Token_STRING
            || yytoken == Token_ADD
            || yytoken == Token_ALIAS
            || yytoken == Token_GET
            || yytoken == Token_GLOBAL
            || yytoken == Token_PARTIAL
            || yytoken == Token_REMOVE
            || yytoken == Token_SET
            || yytoken == Token_VALUE
            || yytoken == Token_WHERE
            || yytoken == Token_YIELD
            || yytoken == Token_ASSEMBLY
            || yytoken == Token_IDENTIFIER)
          {
            class_type_ast *__node_321 = 0;
            if (!parse_class_type(&__node_321))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_class_type, "class_type");
                  }
                return false;
              }
            (*yynode)->class_type_or_secondary_constraint = __node_321;

            (*yynode)->constraint_type = primary_or_secondary_constraint::type_type;
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

            (*yynode)->constraint_type = primary_or_secondary_constraint::type_class;
          }
        else if (yytoken == Token_STRUCT)
          {
            if (yytoken != Token_STRUCT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_STRUCT, "struct");
                  }
                return false;
              }
            yylex();

            (*yynode)->constraint_type = primary_or_secondary_constraint::type_struct;
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

  bool parser::parse_primary_suffix(primary_suffix_ast **yynode)
  {
    *yynode = create<primary_suffix_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_LPAREN
        || yytoken == Token_LBRACKET
        || yytoken == Token_DOT
        || yytoken == Token_ARROW_RIGHT
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT)
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

            identifier_ast *__node_322 = 0;
            if (!parse_identifier(&__node_322))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                  }
                return false;
              }
            (*yynode)->member_name = __node_322;

            bool block_errors_30 = block_errors(true);
            std::size_t try_start_token_30 = token_stream->index() - 1;
            parser_state *try_start_state_30 = copy_current_state();
            {
              if ((yytoken == Token_LESS_THAN) && ( compatibility_mode() >= csharp20_compatibility ))
                {
                  type_arguments_ast *__node_323 = 0;
                  if (!parse_type_arguments(&__node_323))
                    {
                      goto __catch_30;
                    }
                  (*yynode)->type_arguments = __node_323;

                }
              else if (true /*epsilon*/)
              {}
              else
                {
                  goto __catch_30;
                }
            }
            block_errors(block_errors_30);
            if (try_start_state_30)
              delete try_start_state_30;

            if (false) // the only way to enter here is using goto
              {
              __catch_30:
                if (try_start_state_30)
                  {
                    restore_state(try_start_state_30);
                    delete try_start_state_30;
                  }
                block_errors(block_errors_30);
                rewind(try_start_token_30);

              }

            (*yynode)->suffix_type = primary_suffix::type_member_access;
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

            optional_argument_list_ast *__node_324 = 0;
            if (!parse_optional_argument_list(&__node_324))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_optional_argument_list, "optional_argument_list");
                  }
                return false;
              }
            (*yynode)->arguments = __node_324;

            if (yytoken != Token_RPAREN)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_RPAREN, ")");
                  }
                return false;
              }
            yylex();

            (*yynode)->suffix_type = primary_suffix::type_invocation;
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

            expression_ast *__node_325 = 0;
            if (!parse_expression(&__node_325))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_expression, "expression");
                  }
                return false;
              }
            (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_325, memory_pool);

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

                expression_ast *__node_326 = 0;
                if (!parse_expression(&__node_326))
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_symbol(ast_node::Kind_expression, "expression");
                      }
                    return false;
                  }
                (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_326, memory_pool);

              }
            if (yytoken != Token_RBRACKET)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_RBRACKET, "]");
                  }
                return false;
              }
            yylex();

            (*yynode)->suffix_type = primary_suffix::type_element_access;
          }
        else if (yytoken == Token_INCREMENT)
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

            (*yynode)->suffix_type = primary_suffix::type_increment;
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

            (*yynode)->suffix_type = primary_suffix::type_decrement;
          }
        else if (yytoken == Token_ARROW_RIGHT)
          {
            if (yytoken != Token_ARROW_RIGHT)
              {
                if (!yy_block_errors)
                  {
                    yy_expected_token(yytoken, Token_ARROW_RIGHT, "->");
                  }
                return false;
              }
            yylex();

            identifier_ast *__node_327 = 0;
            if (!parse_identifier(&__node_327))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                  }
                return false;
              }
            (*yynode)->member_name = __node_327;

            bool block_errors_31 = block_errors(true);
            std::size_t try_start_token_31 = token_stream->index() - 1;
            parser_state *try_start_state_31 = copy_current_state();
            {
              if ((yytoken == Token_LESS_THAN) && ( compatibility_mode() >= csharp20_compatibility ))
                {
                  type_arguments_ast *__node_328 = 0;
                  if (!parse_type_arguments(&__node_328))
                    {
                      goto __catch_31;
                    }
                  (*yynode)->type_arguments = __node_328;

                }
              else if (true /*epsilon*/)
              {}
              else
                {
                  goto __catch_31;
                }
            }
            block_errors(block_errors_31);
            if (try_start_state_31)
              delete try_start_state_31;

            if (false) // the only way to enter here is using goto
              {
              __catch_31:
                if (try_start_state_31)
                  {
                    restore_state(try_start_state_31);
                    delete try_start_state_31;
                  }
                block_errors(block_errors_31);
                rewind(try_start_token_31);

              }

            (*yynode)->suffix_type = primary_suffix::type_pointer_member_access;
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

  bool parser::parse_property_declaration(property_declaration_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers, type_ast *type, type_name_safe_ast *property_name)
  {
    *yynode = create<property_declaration_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    (*yynode)->attributes = attributes;
    if (attributes && attributes->start_token < (*yynode)->start_token)
      (*yynode)->start_token = attributes->start_token;

    (*yynode)->modifiers = modifiers;
    if (modifiers && modifiers->start_token < (*yynode)->start_token)
      (*yynode)->start_token = modifiers->start_token;

    (*yynode)->type = type;
    if (type && type->start_token < (*yynode)->start_token)
      (*yynode)->start_token = type->start_token;

    (*yynode)->property_name = property_name;
    if (property_name && property_name->start_token < (*yynode)->start_token)
      (*yynode)->start_token = property_name->start_token;

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

        accessor_declarations_ast *__node_329 = 0;
        if (!parse_accessor_declarations(&__node_329))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_accessor_declarations, "accessor_declarations");
              }
            return false;
          }
        (*yynode)->accessor_declarations = __node_329;

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

  bool parser::parse_qualified_identifier(qualified_identifier_ast **yynode)
  {
    *yynode = create<qualified_identifier_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_IDENTIFIER)
      {
        identifier_ast *__node_330 = 0;
        if (!parse_identifier(&__node_330))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_identifier, "identifier");
              }
            return false;
          }
        (*yynode)->name_sequence = snoc((*yynode)->name_sequence, __node_330, memory_pool);

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

            identifier_ast *__node_331 = 0;
            if (!parse_identifier(&__node_331))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                  }
                return false;
              }
            (*yynode)->name_sequence = snoc((*yynode)->name_sequence, __node_331, memory_pool);

          }
      }
    else
      {
        return false;
      }

    (*yynode)->end_token = token_stream->index() - 1;

    return true;
  }

  bool parser::parse_rank_specifier(rank_specifier_ast **yynode)
  {
    *yynode = create<rank_specifier_ast>();

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

        (*yynode)->dimension_seperator_count = 0;
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

            (*yynode)->dimension_seperator_count++;
          }
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

  bool parser::parse_relational_expression(relational_expression_ast **yynode)
  {
    *yynode = create<relational_expression_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BASE
        || yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_CHECKED
        || yytoken == Token_DECIMAL
        || yytoken == Token_DEFAULT
        || yytoken == Token_DELEGATE
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_OBJECT
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_SIZEOF
        || yytoken == Token_STRING
        || yytoken == Token_THIS
        || yytoken == Token_TYPEOF
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_UNCHECKED
        || yytoken == Token_USHORT
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_LPAREN
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_STAR
        || yytoken == Token_BIT_AND
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_REAL_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        shift_expression_ast *__node_332 = 0;
        if (!parse_shift_expression(&__node_332))
          {
            if (!yy_block_errors)
              {
                yy_expected_symbol(ast_node::Kind_shift_expression, "shift_expression");
              }
            return false;
          }
        (*yynode)->expression = __node_332;

        while (yytoken == Token_AS
               || yytoken == Token_IS
               || yytoken == Token_LESS_THAN
               || yytoken == Token_LESS_EQUAL
               || yytoken == Token_GREATER_THAN
               || yytoken == Token_GREATER_EQUAL)
          {
            relational_expression_rest_ast *__node_333 = 0;
            if (!parse_relational_expression_rest(&__node_333))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_relational_expression_rest, "relational_expression_rest");
                  }
                return false;
              }
            (*yynode)->additional_expression_sequence = snoc((*yynode)->additional_expression_sequence, __node_333, memory_pool);

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

    if (yytoken == Token_AS
        || yytoken == Token_IS
        || yytoken == Token_LESS_THAN
        || yytoken == Token_LESS_EQUAL
        || yytoken == Token_GREATER_THAN
        || yytoken == Token_GREATER_EQUAL)
      {
        if (yytoken == Token_LESS_THAN
            || yytoken == Token_LESS_EQUAL
            || yytoken == Token_GREATER_THAN
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
            shift_expression_ast *__node_334 = 0;
            if (!parse_shift_expression(&__node_334))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_shift_expression, "shift_expression");
                  }
                return false;
              }
            (*yynode)->expression = __node_334;

          }
        else if (yytoken == Token_AS
                 || yytoken == Token_IS)
          {
            if (yytoken == Token_IS)
              {
                if (yytoken != Token_IS)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_IS, "is");
                      }
                    return false;
                  }
                yylex();

                (*yynode)->relational_operator = relational_expression_rest::op_is;
              }
            else if (yytoken == Token_AS)
              {
                if (yytoken != Token_AS)
                  {
                    if (!yy_block_errors)
                      {
                        yy_expected_token(yytoken, Token_AS, "as");
                      }
                    return false;
                  }
                yylex();

                (*yynode)->relational_operator = relational_expression_rest::op_as;
              }
            else
              {
                return false;
              }
            type_ast *__node_335 = 0;
            if (!parse_type(&__node_335))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_type, "type");
                  }
                return false;
              }
            (*yynode)->type = __node_335;

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

  bool parser::parse_resource_acquisition(resource_acquisition_ast **yynode)
  {
    *yynode = create<resource_acquisition_ast>();

    (*yynode)->start_token = token_stream->index() - 1;

    if (yytoken == Token_BASE
        || yytoken == Token_BOOL
        || yytoken == Token_BYTE
        || yytoken == Token_CHAR
        || yytoken == Token_CHECKED
        || yytoken == Token_DECIMAL
        || yytoken == Token_DEFAULT
        || yytoken == Token_DELEGATE
        || yytoken == Token_DOUBLE
        || yytoken == Token_FLOAT
        || yytoken == Token_INT
        || yytoken == Token_LONG
        || yytoken == Token_NEW
        || yytoken == Token_OBJECT
        || yytoken == Token_SBYTE
        || yytoken == Token_SHORT
        || yytoken == Token_SIZEOF
        || yytoken == Token_STRING
        || yytoken == Token_THIS
        || yytoken == Token_TYPEOF
        || yytoken == Token_UINT
        || yytoken == Token_ULONG
        || yytoken == Token_UNCHECKED
        || yytoken == Token_USHORT
        || yytoken == Token_VOID
        || yytoken == Token_ADD
        || yytoken == Token_ALIAS
        || yytoken == Token_GET
        || yytoken == Token_GLOBAL
        || yytoken == Token_PARTIAL
        || yytoken == Token_REMOVE
        || yytoken == Token_SET
        || yytoken == Token_VALUE
        || yytoken == Token_WHERE
        || yytoken == Token_YIELD
        || yytoken == Token_ASSEMBLY
        || yytoken == Token_LPAREN
        || yytoken == Token_BANG
        || yytoken == Token_TILDE
        || yytoken == Token_INCREMENT
        || yytoken == Token_DECREMENT
        || yytoken == Token_PLUS
        || yytoken == Token_MINUS
        || yytoken == Token_STAR
        || yytoken == Token_BIT_AND
        || yytoken == Token_TRUE
        || yytoken == Token_FALSE
        || yytoken == Token_NULL
        || yytoken == Token_INTEGER_LITERAL
        || yytoken == Token_REAL_LITERAL
        || yytoken == Token_CHARACTER_LITERAL
        || yytoken == Token_STRING_LITERAL
        || yytoken == Token_IDENTIFIER)
      {
        bool block_errors_32 = block_errors(true);
        std::size_t try_start_token_32 = token_stream->index() - 1;
        parser_state *try_start_state_32 = copy_current_state();
        {
          local_variable_declaration_ast *__node_336 = 0;
          if (!parse_local_variable_declaration(&__node_336))
            {
              goto __catch_32;
            }
          (*yynode)->local_variable_declaration = __node_336;

        }
        block_errors(block_errors_32);
        if (try_start_state_32)
          delete try_start_state_32;

        if (false) // the only way to enter here is using goto
          {
          __catch_32:
            if (try_start_state_32)
              {
                restore_state(try_start_state_32);
                delete try_start_state_32;
              }
            block_errors(block_errors_32);
            rewind(try_start_token_32);

            expression_ast *__node_337 = 0;
            if (!parse_expression(&__node_337))
              {
                if (!yy_block_errors)
                  {
                    yy_expected_symbol(ast_node::Kind_expression, "expression");
                  }
                return false;
              }
            (*yynode)->expression = __node_337;

          }

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

                                 if (yytoken == Token_BASE
                                     || yytoken == Token_BOOL
                                     || yytoken == Token_BYTE
                                     || yytoken == Token_CHAR
                                     || yytoken == Token_CHECKED
                                     || yytoken == Token_DECIMAL
                                     || yytoken == Token_DEFAULT
                                     || yytoken == Token_DELEGATE
                                     || yytoken == Token_DOUBLE
                                     || yytoken == Token_FLOAT
                                     || yytoken == Token_INT
                                     || yytoken == Token_LONG
                                     || yytoken == Token_NEW
                                     || yytoken == Token_OBJECT
                                     || yytoken == Token_SBYTE
                                     || yytoken == Token_SHORT
                                     || yytoken == Token_SIZEOF
                                     || yytoken == Token_STRING
                                     || yytoken == Token_THIS
                                     || yytoken == Token_TYPEOF
                                     || yytoken == Token_UINT
                                     || yytoken == Token_ULONG
                                     || yytoken == Token_UNCHECKED
                                     || yytoken == Token_USHORT
                                     || yytoken == Token_ADD
                                     || yytoken == Token_ALIAS
                                     || yytoken == Token_GET
                                     || yytoken == Token_GLOBAL
                                     || yytoken == Token_PARTIAL
                                     || yytoken == Token_REMOVE
                                     || yytoken == Token_SET
                                     || yytoken == Token_VALUE
                                     || yytoken == Token_WHERE
                                     || yytoken == Token_YIELD
                                     || yytoken == Token_ASSEMBLY
                                     || yytoken == Token_LPAREN
                                     || yytoken == Token_BANG
                                     || yytoken == Token_TILDE
                                     || yytoken == Token_INCREMENT
                                     || yytoken == Token_DECREMENT
                                     || yytoken == Token_PLUS
                                     || yytoken == Token_MINUS
                                     || yytoken == Token_STAR
                                     || yytoken == Token_BIT_AND
                                     || yytoken == Token_TRUE
                                     || yytoken == Token_FALSE
                                     || yytoken == Token_NULL
                                     || yytoken == Token_INTEGER_LITERAL
                                     || yytoken == Token_REAL_LITERAL
                                     || yytoken == Token_CHARACTER_LITERAL
                                     || yytoken == Token_STRING_LITERAL
                                     || yytoken == Token_IDENTIFIER)
                                   {
                                     expression_ast *__node_338 = 0;
                                     if (!parse_expression(&__node_338))
                                       {
                                         if (!yy_block_errors)
                                           {
                                             yy_expected_symbol(ast_node::Kind_expression, "expression");
                                           }
                                         return false;
                                       }
                                     (*yynode)->return_expression = __node_338;

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

                           bool parser::parse_return_type(return_type_ast **yynode)
                                                    {
                                                      *yynode = create<return_type_ast>();

                                                      (*yynode)->start_token = token_stream->index() - 1;

                                                      if (yytoken == Token_BOOL
                                                          || yytoken == Token_BYTE
                                                          || yytoken == Token_CHAR
                                                          || yytoken == Token_DECIMAL
                                                          || yytoken == Token_DOUBLE
                                                          || yytoken == Token_FLOAT
                                                          || yytoken == Token_INT
                                                          || yytoken == Token_LONG
                                                          || yytoken == Token_OBJECT
                                                          || yytoken == Token_SBYTE
                                                          || yytoken == Token_SHORT
                                                          || yytoken == Token_STRING
                                                          || yytoken == Token_UINT
                                                          || yytoken == Token_ULONG
                                                          || yytoken == Token_USHORT
                                                          || yytoken == Token_VOID
                                                          || yytoken == Token_ADD
                                                          || yytoken == Token_ALIAS
                                                          || yytoken == Token_GET
                                                          || yytoken == Token_GLOBAL
                                                          || yytoken == Token_PARTIAL
                                                          || yytoken == Token_REMOVE
                                                          || yytoken == Token_SET
                                                          || yytoken == Token_VALUE
                                                          || yytoken == Token_WHERE
                                                          || yytoken == Token_YIELD
                                                          || yytoken == Token_ASSEMBLY
                                                          || yytoken == Token_IDENTIFIER)
                                                        {
                                                          if ((yytoken == Token_VOID) && ( LA(2).kind != Token_STAR ))
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

                                                              (*yynode)->type = return_type::type_void;
                                                            }
                                                          else if (yytoken == Token_BOOL
                                                                   || yytoken == Token_BYTE
                                                                   || yytoken == Token_CHAR
                                                                   || yytoken == Token_DECIMAL
                                                                   || yytoken == Token_DOUBLE
                                                                   || yytoken == Token_FLOAT
                                                                   || yytoken == Token_INT
                                                                   || yytoken == Token_LONG
                                                                   || yytoken == Token_OBJECT
                                                                   || yytoken == Token_SBYTE
                                                                   || yytoken == Token_SHORT
                                                                   || yytoken == Token_STRING
                                                                   || yytoken == Token_UINT
                                                                   || yytoken == Token_ULONG
                                                                   || yytoken == Token_USHORT
                                                                   || yytoken == Token_VOID
                                                                   || yytoken == Token_ADD
                                                                   || yytoken == Token_ALIAS
                                                                   || yytoken == Token_GET
                                                                   || yytoken == Token_GLOBAL
                                                                   || yytoken == Token_PARTIAL
                                                                   || yytoken == Token_REMOVE
                                                                   || yytoken == Token_SET
                                                                   || yytoken == Token_VALUE
                                                                   || yytoken == Token_WHERE
                                                                   || yytoken == Token_YIELD
                                                                   || yytoken == Token_ASSEMBLY
                                                                   || yytoken == Token_IDENTIFIER)
                                                            {
                                                              type_ast *__node_339 = 0;
                                                              if (!parse_type(&__node_339))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_type, "type");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->regular_type = __node_339;

                                                              (*yynode)->type = return_type::type_regular;
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

                                                    bool parser::parse_secondary_constraints(secondary_constraints_ast **yynode)
                                                    {
                                                      *yynode = create<secondary_constraints_ast>();

                                                      (*yynode)->start_token = token_stream->index() - 1;

                                                      if (yytoken == Token_ADD
                                                          || yytoken == Token_ALIAS
                                                          || yytoken == Token_GET
                                                          || yytoken == Token_GLOBAL
                                                          || yytoken == Token_PARTIAL
                                                          || yytoken == Token_REMOVE
                                                          || yytoken == Token_SET
                                                          || yytoken == Token_VALUE
                                                          || yytoken == Token_WHERE
                                                          || yytoken == Token_YIELD
                                                          || yytoken == Token_ASSEMBLY
                                                          || yytoken == Token_IDENTIFIER)
                                                        {
                                                          type_name_ast *__node_340 = 0;
                                                          if (!parse_type_name(&__node_340))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_type_name, "type_name");
                                                                }
                                                              return false;
                                                            }
                                                          (*yynode)->interface_type_or_type_parameter_sequence = snoc((*yynode)->interface_type_or_type_parameter_sequence, __node_340, memory_pool);

                                                          while (yytoken == Token_COMMA)
                                                            {
                                                              if (LA(2).kind == Token_NEW)
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

                                                              type_name_ast *__node_341 = 0;
                                                              if (!parse_type_name(&__node_341))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_type_name, "type_name");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->interface_type_or_type_parameter_sequence = snoc((*yynode)->interface_type_or_type_parameter_sequence, __node_341, memory_pool);

                                                            }
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

                                                      if (yytoken == Token_BASE
                                                          || yytoken == Token_BOOL
                                                          || yytoken == Token_BYTE
                                                          || yytoken == Token_CHAR
                                                          || yytoken == Token_CHECKED
                                                          || yytoken == Token_DECIMAL
                                                          || yytoken == Token_DEFAULT
                                                          || yytoken == Token_DELEGATE
                                                          || yytoken == Token_DOUBLE
                                                          || yytoken == Token_FLOAT
                                                          || yytoken == Token_INT
                                                          || yytoken == Token_LONG
                                                          || yytoken == Token_NEW
                                                          || yytoken == Token_OBJECT
                                                          || yytoken == Token_SBYTE
                                                          || yytoken == Token_SHORT
                                                          || yytoken == Token_SIZEOF
                                                          || yytoken == Token_STRING
                                                          || yytoken == Token_THIS
                                                          || yytoken == Token_TYPEOF
                                                          || yytoken == Token_UINT
                                                          || yytoken == Token_ULONG
                                                          || yytoken == Token_UNCHECKED
                                                          || yytoken == Token_USHORT
                                                          || yytoken == Token_ADD
                                                          || yytoken == Token_ALIAS
                                                          || yytoken == Token_GET
                                                          || yytoken == Token_GLOBAL
                                                          || yytoken == Token_PARTIAL
                                                          || yytoken == Token_REMOVE
                                                          || yytoken == Token_SET
                                                          || yytoken == Token_VALUE
                                                          || yytoken == Token_WHERE
                                                          || yytoken == Token_YIELD
                                                          || yytoken == Token_ASSEMBLY
                                                          || yytoken == Token_LPAREN
                                                          || yytoken == Token_BANG
                                                          || yytoken == Token_TILDE
                                                          || yytoken == Token_INCREMENT
                                                          || yytoken == Token_DECREMENT
                                                          || yytoken == Token_PLUS
                                                          || yytoken == Token_MINUS
                                                          || yytoken == Token_STAR
                                                          || yytoken == Token_BIT_AND
                                                          || yytoken == Token_TRUE
                                                          || yytoken == Token_FALSE
                                                          || yytoken == Token_NULL
                                                          || yytoken == Token_INTEGER_LITERAL
                                                          || yytoken == Token_REAL_LITERAL
                                                          || yytoken == Token_CHARACTER_LITERAL
                                                          || yytoken == Token_STRING_LITERAL
                                                          || yytoken == Token_IDENTIFIER)
                                                        {
                                                          additive_expression_ast *__node_342 = 0;
                                                          if (!parse_additive_expression(&__node_342))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_additive_expression, "additive_expression");
                                                                }
                                                              return false;
                                                            }
                                                          (*yynode)->expression = __node_342;

                                                          while (yytoken == Token_LSHIFT
                                                                 || yytoken == Token_RSHIFT)
                                                            {
                                                              shift_expression_rest_ast *__node_343 = 0;
                                                              if (!parse_shift_expression_rest(&__node_343))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_shift_expression_rest, "shift_expression_rest");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->additional_expression_sequence = snoc((*yynode)->additional_expression_sequence, __node_343, memory_pool);

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
                                                          || yytoken == Token_RSHIFT)
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
                                                          else if (yytoken == Token_RSHIFT)
                                                            {
                                                              if (yytoken != Token_RSHIFT)
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_token(yytoken, Token_RSHIFT, ">>");
                                                                    }
                                                                  return false;
                                                                }
                                                              yylex();

                                                              (*yynode)->shift_operator = shift_expression_rest::op_rshift;
                                                            }
                                                          else
                                                            {
                                                              return false;
                                                            }
                                                          additive_expression_ast *__node_344 = 0;
                                                          if (!parse_additive_expression(&__node_344))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_additive_expression, "additive_expression");
                                                                }
                                                              return false;
                                                            }
                                                          (*yynode)->expression = __node_344;

                                                        }
                                                      else
                                                        {
                                                          return false;
                                                        }

                                                      (*yynode)->end_token = token_stream->index() - 1;

                                                      return true;
                                                    }

                                                    bool parser::parse_simple_name_or_member_access(simple_name_or_member_access_ast **yynode)
                                                    {
                                                      *yynode = create<simple_name_or_member_access_ast>();

                                                      (*yynode)->start_token = token_stream->index() - 1;

                                                      if (yytoken == Token_BOOL
                                                          || yytoken == Token_BYTE
                                                          || yytoken == Token_CHAR
                                                          || yytoken == Token_DECIMAL
                                                          || yytoken == Token_DOUBLE
                                                          || yytoken == Token_FLOAT
                                                          || yytoken == Token_INT
                                                          || yytoken == Token_LONG
                                                          || yytoken == Token_OBJECT
                                                          || yytoken == Token_SBYTE
                                                          || yytoken == Token_SHORT
                                                          || yytoken == Token_STRING
                                                          || yytoken == Token_UINT
                                                          || yytoken == Token_ULONG
                                                          || yytoken == Token_USHORT
                                                          || yytoken == Token_ADD
                                                          || yytoken == Token_ALIAS
                                                          || yytoken == Token_GET
                                                          || yytoken == Token_GLOBAL
                                                          || yytoken == Token_PARTIAL
                                                          || yytoken == Token_REMOVE
                                                          || yytoken == Token_SET
                                                          || yytoken == Token_VALUE
                                                          || yytoken == Token_WHERE
                                                          || yytoken == Token_YIELD
                                                          || yytoken == Token_ASSEMBLY
                                                          || yytoken == Token_IDENTIFIER)
                                                        {
                                                          if (yytoken == Token_ADD
                                                              || yytoken == Token_ALIAS
                                                              || yytoken == Token_GET
                                                              || yytoken == Token_GLOBAL
                                                              || yytoken == Token_PARTIAL
                                                              || yytoken == Token_REMOVE
                                                              || yytoken == Token_SET
                                                              || yytoken == Token_VALUE
                                                              || yytoken == Token_WHERE
                                                              || yytoken == Token_YIELD
                                                              || yytoken == Token_ASSEMBLY
                                                              || yytoken == Token_IDENTIFIER)
                                                            {
                                                              if ((yytoken == Token_ADD
                                                                   || yytoken == Token_ALIAS
                                                                   || yytoken == Token_GET
                                                                   || yytoken == Token_GLOBAL
                                                                   || yytoken == Token_PARTIAL
                                                                   || yytoken == Token_REMOVE
                                                                   || yytoken == Token_SET
                                                                   || yytoken == Token_VALUE
                                                                   || yytoken == Token_WHERE
                                                                   || yytoken == Token_YIELD
                                                                   || yytoken == Token_ASSEMBLY
                                                                   || yytoken == Token_IDENTIFIER) && ( LA(2).kind == Token_SCOPE ))
                                                                {
                                                                  identifier_ast *__node_345 = 0;
                                                                  if (!parse_identifier(&__node_345))
                                                                    {
                                                                      if (!yy_block_errors)
                                                                        {
                                                                          yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                                                                        }
                                                                      return false;
                                                                    }
                                                                  (*yynode)->qualified_alias_label = __node_345;

                                                                  if (yytoken != Token_SCOPE)
                                                                    {
                                                                      if (!yy_block_errors)
                                                                        {
                                                                          yy_expected_token(yytoken, Token_SCOPE, "::");
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
                                                              identifier_ast *__node_346 = 0;
                                                              if (!parse_identifier(&__node_346))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->member_name = __node_346;

                                                              bool block_errors_33 = block_errors(true);
                                                              std::size_t try_start_token_33 = token_stream->index() - 1;
                                                              parser_state *try_start_state_33 = copy_current_state();
                                                              {
                                                                if ((yytoken == Token_LESS_THAN) && ( compatibility_mode() >= csharp20_compatibility ))
                                                                  {
                                                                    type_arguments_ast *__node_347 = 0;
                                                                    if (!parse_type_arguments(&__node_347))
                                                                      {
                                                                        goto __catch_33;
                                                                      }
                                                                    (*yynode)->type_arguments = __node_347;

                                                                  }
                                                                else if (true /*epsilon*/)
                                                                {}
                                                                else
                                                                  {
                                                                    goto __catch_33;
                                                                  }
                                                              }
                                                              block_errors(block_errors_33);
                                                              if (try_start_state_33)
                                                                delete try_start_state_33;

                                                              if (false) // the only way to enter here is using goto
                                                                {
                                                                __catch_33:
                                                                  if (try_start_state_33)
                                                                    {
                                                                      restore_state(try_start_state_33);
                                                                      delete try_start_state_33;
                                                                    }
                                                                  block_errors(block_errors_33);
                                                                  rewind(try_start_token_33);

                                                                }

                                                            }
                                                          else if (yytoken == Token_BOOL
                                                                   || yytoken == Token_BYTE
                                                                   || yytoken == Token_CHAR
                                                                   || yytoken == Token_DECIMAL
                                                                   || yytoken == Token_DOUBLE
                                                                   || yytoken == Token_FLOAT
                                                                   || yytoken == Token_INT
                                                                   || yytoken == Token_LONG
                                                                   || yytoken == Token_OBJECT
                                                                   || yytoken == Token_SBYTE
                                                                   || yytoken == Token_SHORT
                                                                   || yytoken == Token_STRING
                                                                   || yytoken == Token_UINT
                                                                   || yytoken == Token_ULONG
                                                                   || yytoken == Token_USHORT)
                                                            {
                                                              predefined_type_ast *__node_348 = 0;
                                                              if (!parse_predefined_type(&__node_348))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_predefined_type, "predefined_type");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->predefined_type = __node_348;

                                                              if (yytoken != Token_DOT)
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_token(yytoken, Token_DOT, ".");
                                                                    }
                                                                  return false;
                                                                }
                                                              yylex();

                                                              identifier_ast *__node_349 = 0;
                                                              if (!parse_identifier(&__node_349))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->member_name = __node_349;

                                                              bool block_errors_34 = block_errors(true);
                                                              std::size_t try_start_token_34 = token_stream->index() - 1;
                                                              parser_state *try_start_state_34 = copy_current_state();
                                                              {
                                                                if ((yytoken == Token_LESS_THAN) && ( compatibility_mode() >= csharp20_compatibility ))
                                                                  {
                                                                    type_arguments_ast *__node_350 = 0;
                                                                    if (!parse_type_arguments(&__node_350))
                                                                      {
                                                                        goto __catch_34;
                                                                      }
                                                                    (*yynode)->type_arguments = __node_350;

                                                                  }
                                                                else if (true /*epsilon*/)
                                                                {}
                                                                else
                                                                  {
                                                                    goto __catch_34;
                                                                  }
                                                              }
                                                              block_errors(block_errors_34);
                                                              if (try_start_state_34)
                                                                delete try_start_state_34;

                                                              if (false) // the only way to enter here is using goto
                                                                {
                                                                __catch_34:
                                                                  if (try_start_state_34)
                                                                    {
                                                                      restore_state(try_start_state_34);
                                                                      delete try_start_state_34;
                                                                    }
                                                                  block_errors(block_errors_34);
                                                                  rewind(try_start_token_34);

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

                                                    bool parser::parse_simple_type(simple_type_ast **yynode)
                                                    {
                                                      *yynode = create<simple_type_ast>();

                                                      (*yynode)->start_token = token_stream->index() - 1;

                                                      if (yytoken == Token_BOOL
                                                          || yytoken == Token_BYTE
                                                          || yytoken == Token_CHAR
                                                          || yytoken == Token_DECIMAL
                                                          || yytoken == Token_DOUBLE
                                                          || yytoken == Token_FLOAT
                                                          || yytoken == Token_INT
                                                          || yytoken == Token_LONG
                                                          || yytoken == Token_SBYTE
                                                          || yytoken == Token_SHORT
                                                          || yytoken == Token_UINT
                                                          || yytoken == Token_ULONG
                                                          || yytoken == Token_USHORT)
                                                        {
                                                          if (yytoken == Token_BYTE
                                                              || yytoken == Token_CHAR
                                                              || yytoken == Token_DECIMAL
                                                              || yytoken == Token_DOUBLE
                                                              || yytoken == Token_FLOAT
                                                              || yytoken == Token_INT
                                                              || yytoken == Token_LONG
                                                              || yytoken == Token_SBYTE
                                                              || yytoken == Token_SHORT
                                                              || yytoken == Token_UINT
                                                              || yytoken == Token_ULONG
                                                              || yytoken == Token_USHORT)
                                                            {
                                                              numeric_type_ast *__node_351 = 0;
                                                              if (!parse_numeric_type(&__node_351))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_numeric_type, "numeric_type");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->numeric_type = __node_351;

                                                              (*yynode)->type = simple_type::type_numeric;
                                                            }
                                                          else if (yytoken == Token_BOOL)
                                                            {
                                                              if (yytoken != Token_BOOL)
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_token(yytoken, Token_BOOL, "bool");
                                                                    }
                                                                  return false;
                                                                }
                                                              yylex();

                                                              (*yynode)->type = simple_type::type_bool;
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

                                                    bool parser::parse_specific_catch_clause(specific_catch_clause_ast **yynode)
                                                    {
                                                      *yynode = create<specific_catch_clause_ast>();

                                                      (*yynode)->start_token = token_stream->index() - 1;

                                                      if (yytoken == Token_CATCH)
                                                        {
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

                                                          class_type_ast *__node_352 = 0;
                                                          if (!parse_class_type(&__node_352))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_class_type, "class_type");
                                                                }
                                                              return false;
                                                            }
                                                          (*yynode)->exception_type = __node_352;

                                                          if (yytoken == Token_ADD
                                                              || yytoken == Token_ALIAS
                                                              || yytoken == Token_GET
                                                              || yytoken == Token_GLOBAL
                                                              || yytoken == Token_PARTIAL
                                                              || yytoken == Token_REMOVE
                                                              || yytoken == Token_SET
                                                              || yytoken == Token_VALUE
                                                              || yytoken == Token_WHERE
                                                              || yytoken == Token_YIELD
                                                              || yytoken == Token_ASSEMBLY
                                                              || yytoken == Token_IDENTIFIER)
                                                            {
                                                              identifier_ast *__node_353 = 0;
                                                              if (!parse_identifier(&__node_353))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->exception_name = __node_353;

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

                                                          block_ast *__node_354 = 0;
                                                          if (!parse_block(&__node_354))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_block, "block");
                                                                }
                                                              return false;
                                                            }
                                                          (*yynode)->body = __node_354;

                                                        }
                                                      else
                                                        {
                                                          return false;
                                                        }

                                                      (*yynode)->end_token = token_stream->index() - 1;

                                                      return true;
                                                    }

                                                    bool parser::parse_stackalloc_initializer(stackalloc_initializer_ast **yynode)
                                                    {
                                                      *yynode = create<stackalloc_initializer_ast>();

                                                      (*yynode)->start_token = token_stream->index() - 1;

                                                      if (yytoken == Token_STACKALLOC)
                                                        {
                                                          if (yytoken != Token_STACKALLOC)
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_token(yytoken, Token_STACKALLOC, "stackalloc");
                                                                }
                                                              return false;
                                                            }
                                                          yylex();

                                                          unmanaged_type_ast *__node_355 = 0;
                                                          if (!parse_unmanaged_type(&__node_355))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_unmanaged_type, "unmanaged_type");
                                                                }
                                                              return false;
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

                                                          expression_ast *__node_356 = 0;
                                                          if (!parse_expression(&__node_356))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_expression, "expression");
                                                                }
                                                              return false;
                                                            }
                                                          (*yynode)->expression = __node_356;

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

                                                    bool parser::parse_statement_expression(statement_expression_ast **yynode)
                                                    {
                                                      *yynode = create<statement_expression_ast>();

                                                      (*yynode)->start_token = token_stream->index() - 1;

                                                      if (yytoken == Token_BASE
                                                          || yytoken == Token_BOOL
                                                          || yytoken == Token_BYTE
                                                          || yytoken == Token_CHAR
                                                          || yytoken == Token_CHECKED
                                                          || yytoken == Token_DECIMAL
                                                          || yytoken == Token_DEFAULT
                                                          || yytoken == Token_DELEGATE
                                                          || yytoken == Token_DOUBLE
                                                          || yytoken == Token_FLOAT
                                                          || yytoken == Token_INT
                                                          || yytoken == Token_LONG
                                                          || yytoken == Token_NEW
                                                          || yytoken == Token_OBJECT
                                                          || yytoken == Token_SBYTE
                                                          || yytoken == Token_SHORT
                                                          || yytoken == Token_SIZEOF
                                                          || yytoken == Token_STRING
                                                          || yytoken == Token_THIS
                                                          || yytoken == Token_TYPEOF
                                                          || yytoken == Token_UINT
                                                          || yytoken == Token_ULONG
                                                          || yytoken == Token_UNCHECKED
                                                          || yytoken == Token_USHORT
                                                          || yytoken == Token_ADD
                                                          || yytoken == Token_ALIAS
                                                          || yytoken == Token_GET
                                                          || yytoken == Token_GLOBAL
                                                          || yytoken == Token_PARTIAL
                                                          || yytoken == Token_REMOVE
                                                          || yytoken == Token_SET
                                                          || yytoken == Token_VALUE
                                                          || yytoken == Token_WHERE
                                                          || yytoken == Token_YIELD
                                                          || yytoken == Token_ASSEMBLY
                                                          || yytoken == Token_LPAREN
                                                          || yytoken == Token_BANG
                                                          || yytoken == Token_TILDE
                                                          || yytoken == Token_INCREMENT
                                                          || yytoken == Token_DECREMENT
                                                          || yytoken == Token_PLUS
                                                          || yytoken == Token_MINUS
                                                          || yytoken == Token_STAR
                                                          || yytoken == Token_BIT_AND
                                                          || yytoken == Token_TRUE
                                                          || yytoken == Token_FALSE
                                                          || yytoken == Token_NULL
                                                          || yytoken == Token_INTEGER_LITERAL
                                                          || yytoken == Token_REAL_LITERAL
                                                          || yytoken == Token_CHARACTER_LITERAL
                                                          || yytoken == Token_STRING_LITERAL
                                                          || yytoken == Token_IDENTIFIER)
                                                        {
                                                          expression_ast *__node_357 = 0;
                                                          if (!parse_expression(&__node_357))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_expression, "expression");
                                                                }
                                                              return false;
                                                            }
                                                          (*yynode)->expression = __node_357;

                                                        }
                                                      else
                                                        {
                                                          return false;
                                                        }

                                                      (*yynode)->end_token = token_stream->index() - 1;

                                                      return true;
                                                    }

                                                    bool parser::parse_struct_body(struct_body_ast **yynode)
                                                    {
                                                      *yynode = create<struct_body_ast>();

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
                                                                 || yytoken == Token_BOOL
                                                                 || yytoken == Token_BYTE
                                                                 || yytoken == Token_CHAR
                                                                 || yytoken == Token_CLASS
                                                                 || yytoken == Token_CONST
                                                                 || yytoken == Token_DECIMAL
                                                                 || yytoken == Token_DELEGATE
                                                                 || yytoken == Token_DOUBLE
                                                                 || yytoken == Token_ENUM
                                                                 || yytoken == Token_EVENT
                                                                 || yytoken == Token_EXPLICIT
                                                                 || yytoken == Token_EXTERN
                                                                 || yytoken == Token_FIXED
                                                                 || yytoken == Token_FLOAT
                                                                 || yytoken == Token_IMPLICIT
                                                                 || yytoken == Token_INT
                                                                 || yytoken == Token_INTERFACE
                                                                 || yytoken == Token_INTERNAL
                                                                 || yytoken == Token_LONG
                                                                 || yytoken == Token_NEW
                                                                 || yytoken == Token_OBJECT
                                                                 || yytoken == Token_OVERRIDE
                                                                 || yytoken == Token_PRIVATE
                                                                 || yytoken == Token_PROTECTED
                                                                 || yytoken == Token_PUBLIC
                                                                 || yytoken == Token_READONLY
                                                                 || yytoken == Token_SBYTE
                                                                 || yytoken == Token_SEALED
                                                                 || yytoken == Token_SHORT
                                                                 || yytoken == Token_STATIC
                                                                 || yytoken == Token_STRING
                                                                 || yytoken == Token_STRUCT
                                                                 || yytoken == Token_UINT
                                                                 || yytoken == Token_ULONG
                                                                 || yytoken == Token_UNSAFE
                                                                 || yytoken == Token_USHORT
                                                                 || yytoken == Token_VIRTUAL
                                                                 || yytoken == Token_VOID
                                                                 || yytoken == Token_VOLATILE
                                                                 || yytoken == Token_ADD
                                                                 || yytoken == Token_ALIAS
                                                                 || yytoken == Token_GET
                                                                 || yytoken == Token_GLOBAL
                                                                 || yytoken == Token_PARTIAL
                                                                 || yytoken == Token_REMOVE
                                                                 || yytoken == Token_SET
                                                                 || yytoken == Token_VALUE
                                                                 || yytoken == Token_WHERE
                                                                 || yytoken == Token_YIELD
                                                                 || yytoken == Token_ASSEMBLY
                                                                 || yytoken == Token_LBRACKET
                                                                 || yytoken == Token_IDENTIFIER)
                                                            {
                                                              std::size_t try_start_token_35 = token_stream->index() - 1;
                                                              parser_state *try_start_state_35 = copy_current_state();
                                                              {
                                                                struct_member_declaration_ast *__node_358 = 0;
                                                                if (!parse_struct_member_declaration(&__node_358))
                                                                  {
                                                                    goto __catch_35;
                                                                  }
                                                                (*yynode)->member_declaration_sequence = snoc((*yynode)->member_declaration_sequence, __node_358, memory_pool);

                                                              }
                                                              if (try_start_state_35)
                                                                delete try_start_state_35;

                                                              if (false) // the only way to enter here is using goto
                                                                {
                                                                __catch_35:
                                                                  if (try_start_state_35)
                                                                    {
                                                                      restore_state(try_start_state_35);
                                                                      delete try_start_state_35;
                                                                    }
                                                                  if (try_start_token_35 == token_stream->index() - 1)
                                                                    yylex();

                                                                  while (yytoken != Token_EOF
                                                                         && yytoken != Token_ABSTRACT
                                                                         && yytoken != Token_BOOL
                                                                         && yytoken != Token_BYTE
                                                                         && yytoken != Token_CHAR
                                                                         && yytoken != Token_CLASS
                                                                         && yytoken != Token_CONST
                                                                         && yytoken != Token_DECIMAL
                                                                         && yytoken != Token_DELEGATE
                                                                         && yytoken != Token_DOUBLE
                                                                         && yytoken != Token_ENUM
                                                                         && yytoken != Token_EVENT
                                                                         && yytoken != Token_EXPLICIT
                                                                         && yytoken != Token_EXTERN
                                                                         && yytoken != Token_FIXED
                                                                         && yytoken != Token_FLOAT
                                                                         && yytoken != Token_IMPLICIT
                                                                         && yytoken != Token_INT
                                                                         && yytoken != Token_INTERFACE
                                                                         && yytoken != Token_INTERNAL
                                                                         && yytoken != Token_LONG
                                                                         && yytoken != Token_NEW
                                                                         && yytoken != Token_OBJECT
                                                                         && yytoken != Token_OVERRIDE
                                                                         && yytoken != Token_PRIVATE
                                                                         && yytoken != Token_PROTECTED
                                                                         && yytoken != Token_PUBLIC
                                                                         && yytoken != Token_READONLY
                                                                         && yytoken != Token_SBYTE
                                                                         && yytoken != Token_SEALED
                                                                         && yytoken != Token_SHORT
                                                                         && yytoken != Token_STATIC
                                                                         && yytoken != Token_STRING
                                                                         && yytoken != Token_STRUCT
                                                                         && yytoken != Token_UINT
                                                                         && yytoken != Token_ULONG
                                                                         && yytoken != Token_UNSAFE
                                                                         && yytoken != Token_USHORT
                                                                         && yytoken != Token_VIRTUAL
                                                                         && yytoken != Token_VOID
                                                                         && yytoken != Token_VOLATILE
                                                                         && yytoken != Token_ADD
                                                                         && yytoken != Token_ALIAS
                                                                         && yytoken != Token_GET
                                                                         && yytoken != Token_GLOBAL
                                                                         && yytoken != Token_PARTIAL
                                                                         && yytoken != Token_REMOVE
                                                                         && yytoken != Token_SET
                                                                         && yytoken != Token_VALUE
                                                                         && yytoken != Token_WHERE
                                                                         && yytoken != Token_YIELD
                                                                         && yytoken != Token_ASSEMBLY
                                                                         && yytoken != Token_RBRACE
                                                                         && yytoken != Token_LBRACKET
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

                                                    bool parser::parse_struct_declaration(struct_declaration_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers, bool partial)
                                                    {
                                                      *yynode = create<struct_declaration_ast>();

                                                      (*yynode)->start_token = token_stream->index() - 1;

                                                      (*yynode)->attributes = attributes;
                                                      if (attributes && attributes->start_token < (*yynode)->start_token)
                                                        (*yynode)->start_token = attributes->start_token;

                                                      (*yynode)->modifiers = modifiers;
                                                      if (modifiers && modifiers->start_token < (*yynode)->start_token)
                                                        (*yynode)->start_token = modifiers->start_token;

                                                      (*yynode)->partial = partial;
                                                      if (yytoken == Token_STRUCT)
                                                        {
                                                          if (yytoken != Token_STRUCT)
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_token(yytoken, Token_STRUCT, "struct");
                                                                }
                                                              return false;
                                                            }
                                                          yylex();

                                                          identifier_ast *__node_359 = 0;
                                                          if (!parse_identifier(&__node_359))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                                                                }
                                                              return false;
                                                            }
                                                          (*yynode)->struct_name = __node_359;

                                                          if ((yytoken == Token_LESS_THAN) && ( compatibility_mode() >= csharp20_compatibility ))
                                                            {
                                                              std::size_t try_start_token_36 = token_stream->index() - 1;
                                                              parser_state *try_start_state_36 = copy_current_state();
                                                              {
                                                                type_parameters_ast *__node_360 = 0;
                                                                if (!parse_type_parameters(&__node_360))
                                                                  {
                                                                    goto __catch_36;
                                                                  }
                                                                (*yynode)->type_parameters = __node_360;

                                                              }
                                                              if (try_start_state_36)
                                                                delete try_start_state_36;

                                                              if (false) // the only way to enter here is using goto
                                                                {
                                                                __catch_36:
                                                                  if (try_start_state_36)
                                                                    {
                                                                      restore_state(try_start_state_36);
                                                                      delete try_start_state_36;
                                                                    }
                                                                  if (try_start_token_36 == token_stream->index() - 1)
                                                                    yylex();

                                                                  while (yytoken != Token_EOF
                                                                         && yytoken != Token_WHERE
                                                                         && yytoken != Token_LBRACE
                                                                         && yytoken != Token_COLON)
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
                                                          if (yytoken == Token_COLON)
                                                            {
                                                              struct_interfaces_ast *__node_361 = 0;
                                                              if (!parse_struct_interfaces(&__node_361))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_struct_interfaces, "struct_interfaces");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->struct_interfaces = __node_361;

                                                            }
                                                          else if (true /*epsilon*/)
                                                          {}
                                                          else
                                                            {
                                                              return false;
                                                            }
                                                          if ((yytoken == Token_WHERE) && ( compatibility_mode() >= csharp20_compatibility ))
                                                            {
                                                              do
                                                                {
                                                                  std::size_t try_start_token_37 = token_stream->index() - 1;
                                                                  parser_state *try_start_state_37 = copy_current_state();
                                                                  {
                                                                    type_parameter_constraints_clause_ast *__node_362 = 0;
                                                                    if (!parse_type_parameter_constraints_clause(&__node_362))
                                                                      {
                                                                        goto __catch_37;
                                                                      }
                                                                    (*yynode)->type_parameter_constraints_sequence = snoc((*yynode)->type_parameter_constraints_sequence, __node_362, memory_pool);

                                                                  }
                                                                  if (try_start_state_37)
                                                                    delete try_start_state_37;

                                                                  if (false) // the only way to enter here is using goto
                                                                    {
                                                                    __catch_37:
                                                                      if (try_start_state_37)
                                                                        {
                                                                          restore_state(try_start_state_37);
                                                                          delete try_start_state_37;
                                                                        }
                                                                      if (try_start_token_37 == token_stream->index() - 1)
                                                                        yylex();

                                                                      while (yytoken != Token_EOF
                                                                             && yytoken != Token_WHERE
                                                                             && yytoken != Token_LBRACE)
                                                                        {
                                                                          yylex();
                                                                        }
                                                                    }

                                                                }
                                                              while (yytoken == Token_WHERE);
                                                            }
                                                          else if (true /*epsilon*/)
                                                          {}
                                                          else
                                                            {
                                                              return false;
                                                            }
                                                          struct_body_ast *__node_363 = 0;
                                                          if (!parse_struct_body(&__node_363))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_struct_body, "struct_body");
                                                                }
                                                              return false;
                                                            }
                                                          (*yynode)->body = __node_363;

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

                                                    bool parser::parse_struct_interfaces(struct_interfaces_ast **yynode)
                                                    {
                                                      *yynode = create<struct_interfaces_ast>();

                                                      (*yynode)->start_token = token_stream->index() - 1;

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

                                                          type_name_ast *__node_364 = 0;
                                                          if (!parse_type_name(&__node_364))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_type_name, "type_name");
                                                                }
                                                              return false;
                                                            }
                                                          (*yynode)->interface_type_sequence = snoc((*yynode)->interface_type_sequence, __node_364, memory_pool);

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

                                                              type_name_ast *__node_365 = 0;
                                                              if (!parse_type_name(&__node_365))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_type_name, "type_name");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->interface_type_sequence = snoc((*yynode)->interface_type_sequence, __node_365, memory_pool);

                                                            }
                                                        }
                                                      else
                                                        {
                                                          return false;
                                                        }

                                                      (*yynode)->end_token = token_stream->index() - 1;

                                                      return true;
                                                    }

                                                    bool parser::parse_struct_member_declaration(struct_member_declaration_ast **yynode)
                                                    {
                                                      *yynode = create<struct_member_declaration_ast>();

                                                      (*yynode)->start_token = token_stream->index() - 1;

                                                      if (yytoken == Token_ABSTRACT
                                                          || yytoken == Token_BOOL
                                                          || yytoken == Token_BYTE
                                                          || yytoken == Token_CHAR
                                                          || yytoken == Token_CLASS
                                                          || yytoken == Token_CONST
                                                          || yytoken == Token_DECIMAL
                                                          || yytoken == Token_DELEGATE
                                                          || yytoken == Token_DOUBLE
                                                          || yytoken == Token_ENUM
                                                          || yytoken == Token_EVENT
                                                          || yytoken == Token_EXPLICIT
                                                          || yytoken == Token_EXTERN
                                                          || yytoken == Token_FIXED
                                                          || yytoken == Token_FLOAT
                                                          || yytoken == Token_IMPLICIT
                                                          || yytoken == Token_INT
                                                          || yytoken == Token_INTERFACE
                                                          || yytoken == Token_INTERNAL
                                                          || yytoken == Token_LONG
                                                          || yytoken == Token_NEW
                                                          || yytoken == Token_OBJECT
                                                          || yytoken == Token_OVERRIDE
                                                          || yytoken == Token_PRIVATE
                                                          || yytoken == Token_PROTECTED
                                                          || yytoken == Token_PUBLIC
                                                          || yytoken == Token_READONLY
                                                          || yytoken == Token_SBYTE
                                                          || yytoken == Token_SEALED
                                                          || yytoken == Token_SHORT
                                                          || yytoken == Token_STATIC
                                                          || yytoken == Token_STRING
                                                          || yytoken == Token_STRUCT
                                                          || yytoken == Token_UINT
                                                          || yytoken == Token_ULONG
                                                          || yytoken == Token_UNSAFE
                                                          || yytoken == Token_USHORT
                                                          || yytoken == Token_VIRTUAL
                                                          || yytoken == Token_VOID
                                                          || yytoken == Token_VOLATILE
                                                          || yytoken == Token_ADD
                                                          || yytoken == Token_ALIAS
                                                          || yytoken == Token_GET
                                                          || yytoken == Token_GLOBAL
                                                          || yytoken == Token_PARTIAL
                                                          || yytoken == Token_REMOVE
                                                          || yytoken == Token_SET
                                                          || yytoken == Token_VALUE
                                                          || yytoken == Token_WHERE
                                                          || yytoken == Token_YIELD
                                                          || yytoken == Token_ASSEMBLY
                                                          || yytoken == Token_LBRACKET
                                                          || yytoken == Token_IDENTIFIER)
                                                        {
                                                          optional_attribute_sections_ast *attributes = 0;

                                                          optional_modifiers_ast *modifiers = 0;

                                                          optional_attribute_sections_ast *__node_366 = 0;
                                                          if (!parse_optional_attribute_sections(&__node_366))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_optional_attribute_sections, "optional_attribute_sections");
                                                                }
                                                              return false;
                                                            }
                                                          attributes = __node_366;

                                                          optional_modifiers_ast *__node_367 = 0;
                                                          if (!parse_optional_modifiers(&__node_367))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_optional_modifiers, "optional_modifiers");
                                                                }
                                                              return false;
                                                            }
                                                          modifiers = __node_367;

                                                          class_or_struct_member_declaration_ast *__node_368 = 0;
                                                          if (!parse_class_or_struct_member_declaration(&__node_368, attributes, modifiers ))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_class_or_struct_member_declaration, "class_or_struct_member_declaration");
                                                                }
                                                              return false;
                                                            }
                                                          (*yynode)->declaration = __node_368;

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

                                                              constant_expression_ast *__node_369 = 0;
                                                              if (!parse_constant_expression(&__node_369))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_constant_expression, "constant_expression");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->case_expression = __node_369;

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
                                                              switch_label_ast *__node_370 = 0;
                                                              if (!parse_switch_label(&__node_370))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_switch_label, "switch_label");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->label_sequence = snoc((*yynode)->label_sequence, __node_370, memory_pool);

                                                            }
                                                          while (yytoken == Token_CASE
                                                                 || yytoken == Token_DEFAULT);
                                                          do
                                                            {
                                                              if (yytoken == Token_DEFAULT && LA(2).kind != Token_LPAREN)
                                                                {
                                                                  break;
                                                                }  // don't give in to default_value_expression
                                                              block_statement_ast *__node_371 = 0;
                                                              if (!parse_block_statement(&__node_371))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_block_statement, "block_statement");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->statement_sequence = snoc((*yynode)->statement_sequence, __node_371, memory_pool);

                                                            }
                                                          while (yytoken == Token_BASE
                                                                 || yytoken == Token_BOOL
                                                                 || yytoken == Token_BREAK
                                                                 || yytoken == Token_BYTE
                                                                 || yytoken == Token_CHAR
                                                                 || yytoken == Token_CHECKED
                                                                 || yytoken == Token_CONST
                                                                 || yytoken == Token_CONTINUE
                                                                 || yytoken == Token_DECIMAL
                                                                 || yytoken == Token_DEFAULT
                                                                 || yytoken == Token_DELEGATE
                                                                 || yytoken == Token_DO
                                                                 || yytoken == Token_DOUBLE
                                                                 || yytoken == Token_FIXED
                                                                 || yytoken == Token_FLOAT
                                                                 || yytoken == Token_FOREACH
                                                                 || yytoken == Token_FOR
                                                                 || yytoken == Token_GOTO
                                                                 || yytoken == Token_IF
                                                                 || yytoken == Token_INT
                                                                 || yytoken == Token_LOCK
                                                                 || yytoken == Token_LONG
                                                                 || yytoken == Token_NEW
                                                                 || yytoken == Token_OBJECT
                                                                 || yytoken == Token_RETURN
                                                                 || yytoken == Token_SBYTE
                                                                 || yytoken == Token_SHORT
                                                                 || yytoken == Token_SIZEOF
                                                                 || yytoken == Token_STRING
                                                                 || yytoken == Token_SWITCH
                                                                 || yytoken == Token_THIS
                                                                 || yytoken == Token_THROW
                                                                 || yytoken == Token_TRY
                                                                 || yytoken == Token_TYPEOF
                                                                 || yytoken == Token_UINT
                                                                 || yytoken == Token_ULONG
                                                                 || yytoken == Token_UNCHECKED
                                                                 || yytoken == Token_UNSAFE
                                                                 || yytoken == Token_USHORT
                                                                 || yytoken == Token_USING
                                                                 || yytoken == Token_VOID
                                                                 || yytoken == Token_WHILE
                                                                 || yytoken == Token_ADD
                                                                 || yytoken == Token_ALIAS
                                                                 || yytoken == Token_GET
                                                                 || yytoken == Token_GLOBAL
                                                                 || yytoken == Token_PARTIAL
                                                                 || yytoken == Token_REMOVE
                                                                 || yytoken == Token_SET
                                                                 || yytoken == Token_VALUE
                                                                 || yytoken == Token_WHERE
                                                                 || yytoken == Token_YIELD
                                                                 || yytoken == Token_ASSEMBLY
                                                                 || yytoken == Token_LPAREN
                                                                 || yytoken == Token_LBRACE
                                                                 || yytoken == Token_SEMICOLON
                                                                 || yytoken == Token_BANG
                                                                 || yytoken == Token_TILDE
                                                                 || yytoken == Token_INCREMENT
                                                                 || yytoken == Token_DECREMENT
                                                                 || yytoken == Token_PLUS
                                                                 || yytoken == Token_MINUS
                                                                 || yytoken == Token_STAR
                                                                 || yytoken == Token_BIT_AND
                                                                 || yytoken == Token_TRUE
                                                                 || yytoken == Token_FALSE
                                                                 || yytoken == Token_NULL
                                                                 || yytoken == Token_INTEGER_LITERAL
                                                                 || yytoken == Token_REAL_LITERAL
                                                                 || yytoken == Token_CHARACTER_LITERAL
                                                                 || yytoken == Token_STRING_LITERAL
                                                                 || yytoken == Token_IDENTIFIER);
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

                                                          expression_ast *__node_372 = 0;
                                                          if (!parse_expression(&__node_372))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_expression, "expression");
                                                                }
                                                              return false;
                                                            }
                                                          (*yynode)->switch_expression = __node_372;

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
                                                              std::size_t try_start_token_38 = token_stream->index() - 1;
                                                              parser_state *try_start_state_38 = copy_current_state();
                                                              {
                                                                switch_section_ast *__node_373 = 0;
                                                                if (!parse_switch_section(&__node_373))
                                                                  {
                                                                    goto __catch_38;
                                                                  }
                                                                (*yynode)->switch_section_sequence = snoc((*yynode)->switch_section_sequence, __node_373, memory_pool);

                                                              }
                                                              if (try_start_state_38)
                                                                delete try_start_state_38;

                                                              if (false) // the only way to enter here is using goto
                                                                {
                                                                __catch_38:
                                                                  if (try_start_state_38)
                                                                    {
                                                                      restore_state(try_start_state_38);
                                                                      delete try_start_state_38;
                                                                    }
                                                                  if (try_start_token_38 == token_stream->index() - 1)
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

                                                          if (yytoken == Token_BASE
                                                              || yytoken == Token_BOOL
                                                              || yytoken == Token_BYTE
                                                              || yytoken == Token_CHAR
                                                              || yytoken == Token_CHECKED
                                                              || yytoken == Token_DECIMAL
                                                              || yytoken == Token_DEFAULT
                                                              || yytoken == Token_DELEGATE
                                                              || yytoken == Token_DOUBLE
                                                              || yytoken == Token_FLOAT
                                                              || yytoken == Token_INT
                                                              || yytoken == Token_LONG
                                                              || yytoken == Token_NEW
                                                              || yytoken == Token_OBJECT
                                                              || yytoken == Token_SBYTE
                                                              || yytoken == Token_SHORT
                                                              || yytoken == Token_SIZEOF
                                                              || yytoken == Token_STRING
                                                              || yytoken == Token_THIS
                                                              || yytoken == Token_TYPEOF
                                                              || yytoken == Token_UINT
                                                              || yytoken == Token_ULONG
                                                              || yytoken == Token_UNCHECKED
                                                              || yytoken == Token_USHORT
                                                              || yytoken == Token_ADD
                                                              || yytoken == Token_ALIAS
                                                              || yytoken == Token_GET
                                                              || yytoken == Token_GLOBAL
                                                              || yytoken == Token_PARTIAL
                                                              || yytoken == Token_REMOVE
                                                              || yytoken == Token_SET
                                                              || yytoken == Token_VALUE
                                                              || yytoken == Token_WHERE
                                                              || yytoken == Token_YIELD
                                                              || yytoken == Token_ASSEMBLY
                                                              || yytoken == Token_LPAREN
                                                              || yytoken == Token_BANG
                                                              || yytoken == Token_TILDE
                                                              || yytoken == Token_INCREMENT
                                                              || yytoken == Token_DECREMENT
                                                              || yytoken == Token_PLUS
                                                              || yytoken == Token_MINUS
                                                              || yytoken == Token_STAR
                                                              || yytoken == Token_BIT_AND
                                                              || yytoken == Token_TRUE
                                                              || yytoken == Token_FALSE
                                                              || yytoken == Token_NULL
                                                              || yytoken == Token_INTEGER_LITERAL
                                                              || yytoken == Token_REAL_LITERAL
                                                              || yytoken == Token_CHARACTER_LITERAL
                                                              || yytoken == Token_STRING_LITERAL
                                                              || yytoken == Token_IDENTIFIER)
                                                            {
                                                              expression_ast *__node_374 = 0;
                                                              if (!parse_expression(&__node_374))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_expression, "expression");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->exception = __node_374;

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

                                                          block_ast *__node_375 = 0;
                                                          if (!parse_block(&__node_375))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_block, "block");
                                                                }
                                                              return false;
                                                            }
                                                          (*yynode)->try_body = __node_375;

                                                          if (yytoken == Token_CATCH)
                                                            {
                                                              catch_clauses_ast *__node_376 = 0;
                                                              if (!parse_catch_clauses(&__node_376))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_catch_clauses, "catch_clauses");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->catch_clauses = __node_376;

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

                                                                  block_ast *__node_377 = 0;
                                                                  if (!parse_block(&__node_377))
                                                                    {
                                                                      if (!yy_block_errors)
                                                                        {
                                                                          yy_expected_symbol(ast_node::Kind_block, "block");
                                                                        }
                                                                      return false;
                                                                    }
                                                                  (*yynode)->finally_body = __node_377;

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

                                                              block_ast *__node_378 = 0;
                                                              if (!parse_block(&__node_378))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_block, "block");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->finally_body = __node_378;

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

                                                      if (yytoken == Token_BOOL
                                                          || yytoken == Token_BYTE
                                                          || yytoken == Token_CHAR
                                                          || yytoken == Token_DECIMAL
                                                          || yytoken == Token_DOUBLE
                                                          || yytoken == Token_FLOAT
                                                          || yytoken == Token_INT
                                                          || yytoken == Token_LONG
                                                          || yytoken == Token_OBJECT
                                                          || yytoken == Token_SBYTE
                                                          || yytoken == Token_SHORT
                                                          || yytoken == Token_STRING
                                                          || yytoken == Token_UINT
                                                          || yytoken == Token_ULONG
                                                          || yytoken == Token_USHORT
                                                          || yytoken == Token_VOID
                                                          || yytoken == Token_ADD
                                                          || yytoken == Token_ALIAS
                                                          || yytoken == Token_GET
                                                          || yytoken == Token_GLOBAL
                                                          || yytoken == Token_PARTIAL
                                                          || yytoken == Token_REMOVE
                                                          || yytoken == Token_SET
                                                          || yytoken == Token_VALUE
                                                          || yytoken == Token_WHERE
                                                          || yytoken == Token_YIELD
                                                          || yytoken == Token_ASSEMBLY
                                                          || yytoken == Token_IDENTIFIER)
                                                        {
                                                          unmanaged_type_ast *__node_379 = 0;
                                                          if (!parse_unmanaged_type(&__node_379))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_unmanaged_type, "unmanaged_type");
                                                                }
                                                              return false;
                                                            }
                                                          (*yynode)->unmanaged_type = __node_379;

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
                                                          type_ast *__node_380 = 0;
                                                          if (!parse_type(&__node_380))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_type, "type");
                                                                }
                                                              return false;
                                                            }
                                                          (*yynode)->type_argument_sequence = snoc((*yynode)->type_argument_sequence, __node_380, memory_pool);

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

                                                              type_ast *__node_381 = 0;
                                                              if (!parse_type(&__node_381))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_type, "type");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->type_argument_sequence = snoc((*yynode)->type_argument_sequence, __node_381, memory_pool);

                                                            }
                                                          if (yytoken == Token_GREATER_THAN
                                                              || yytoken == Token_RSHIFT)
                                                            {
                                                              type_arguments_or_parameters_end_ast *__node_382 = 0;
                                                              if (!parse_type_arguments_or_parameters_end(&__node_382))
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
                                                          || yytoken == Token_RSHIFT)
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
                                                          else if (yytoken == Token_RSHIFT)
                                                            {
                                                              if (yytoken != Token_RSHIFT)
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_token(yytoken, Token_RSHIFT, ">>");
                                                                    }
                                                                  return false;
                                                                }
                                                              yylex();

                                                              _M_state.ltCounter -= 2;
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
                                                          || yytoken == Token_DELEGATE
                                                          || yytoken == Token_ENUM
                                                          || yytoken == Token_EXTERN
                                                          || yytoken == Token_FIXED
                                                          || yytoken == Token_INTERFACE
                                                          || yytoken == Token_INTERNAL
                                                          || yytoken == Token_NEW
                                                          || yytoken == Token_OVERRIDE
                                                          || yytoken == Token_PRIVATE
                                                          || yytoken == Token_PROTECTED
                                                          || yytoken == Token_PUBLIC
                                                          || yytoken == Token_READONLY
                                                          || yytoken == Token_SEALED
                                                          || yytoken == Token_STATIC
                                                          || yytoken == Token_STRUCT
                                                          || yytoken == Token_UNSAFE
                                                          || yytoken == Token_VIRTUAL
                                                          || yytoken == Token_VOLATILE
                                                          || yytoken == Token_PARTIAL
                                                          || yytoken == Token_LBRACKET)
                                                        {
                                                          optional_attribute_sections_ast *attributes = 0;

                                                          optional_modifiers_ast *modifiers = 0;

                                                          optional_attribute_sections_ast *__node_383 = 0;
                                                          if (!parse_optional_attribute_sections(&__node_383))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_optional_attribute_sections, "optional_attribute_sections");
                                                                }
                                                              return false;
                                                            }
                                                          attributes = __node_383;

                                                          optional_modifiers_ast *__node_384 = 0;
                                                          if (!parse_optional_modifiers(&__node_384))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_optional_modifiers, "optional_modifiers");
                                                                }
                                                              return false;
                                                            }
                                                          modifiers = __node_384;

                                                          type_declaration_rest_ast *__node_385 = 0;
                                                          if (!parse_type_declaration_rest(&__node_385, attributes, modifiers ))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_type_declaration_rest, "type_declaration_rest");
                                                                }
                                                              return false;
                                                            }
                                                          (*yynode)->rest = __node_385;

                                                        }
                                                      else
                                                        {
                                                          return false;
                                                        }

                                                      (*yynode)->end_token = token_stream->index() - 1;

                                                      return true;
                                                    }

                                                    bool parser::parse_type_declaration_rest(type_declaration_rest_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers)
                                                    {
                                                      *yynode = create<type_declaration_rest_ast>();

                                                      (*yynode)->start_token = token_stream->index() - 1;

                                                      if (yytoken == Token_CLASS
                                                          || yytoken == Token_DELEGATE
                                                          || yytoken == Token_ENUM
                                                          || yytoken == Token_INTERFACE
                                                          || yytoken == Token_STRUCT
                                                          || yytoken == Token_PARTIAL)
                                                        {
                                                          if (yytoken == Token_PARTIAL)
                                                            {
                                                              if (yytoken != Token_PARTIAL)
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_token(yytoken, Token_PARTIAL, "partial");
                                                                    }
                                                                  return false;
                                                                }
                                                              yylex();

                                                              if (yytoken == Token_CLASS)
                                                                {
                                                                  class_declaration_ast *__node_386 = 0;
                                                                  if (!parse_class_declaration(&__node_386, attributes, modifiers, true ))
                                                                    {
                                                                      if (!yy_block_errors)
                                                                        {
                                                                          yy_expected_symbol(ast_node::Kind_class_declaration, "class_declaration");
                                                                        }
                                                                      return false;
                                                                    }
                                                                  if (__node_386->start_token < (*yynode)->start_token)
                                                                    (*yynode)->start_token = __node_386->start_token;
                                                                  (*yynode)->class_declaration = __node_386;

                                                                }
                                                              else if (yytoken == Token_STRUCT)
                                                                {
                                                                  struct_declaration_ast *__node_387 = 0;
                                                                  if (!parse_struct_declaration(&__node_387, attributes, modifiers, true ))
                                                                    {
                                                                      if (!yy_block_errors)
                                                                        {
                                                                          yy_expected_symbol(ast_node::Kind_struct_declaration, "struct_declaration");
                                                                        }
                                                                      return false;
                                                                    }
                                                                  if (__node_387->start_token < (*yynode)->start_token)
                                                                    (*yynode)->start_token = __node_387->start_token;
                                                                  (*yynode)->struct_declaration = __node_387;

                                                                }
                                                              else if (yytoken == Token_INTERFACE)
                                                                {
                                                                  interface_declaration_ast *__node_388 = 0;
                                                                  if (!parse_interface_declaration(&__node_388, attributes, modifiers, true ))
                                                                    {
                                                                      if (!yy_block_errors)
                                                                        {
                                                                          yy_expected_symbol(ast_node::Kind_interface_declaration, "interface_declaration");
                                                                        }
                                                                      return false;
                                                                    }
                                                                  if (__node_388->start_token < (*yynode)->start_token)
                                                                    (*yynode)->start_token = __node_388->start_token;
                                                                  (*yynode)->interface_declaration = __node_388;

                                                                }
                                                              else
                                                                {
                                                                  return false;
                                                                }
                                                            }
                                                          else if (yytoken == Token_CLASS)
                                                            {
                                                              class_declaration_ast *__node_389 = 0;
                                                              if (!parse_class_declaration(&__node_389, attributes, modifiers, false ))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_class_declaration, "class_declaration");
                                                                    }
                                                                  return false;
                                                                }
                                                              if (__node_389->start_token < (*yynode)->start_token)
                                                                (*yynode)->start_token = __node_389->start_token;
                                                              (*yynode)->class_declaration = __node_389;

                                                            }
                                                          else if (yytoken == Token_STRUCT)
                                                            {
                                                              struct_declaration_ast *__node_390 = 0;
                                                              if (!parse_struct_declaration(&__node_390, attributes, modifiers, false ))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_struct_declaration, "struct_declaration");
                                                                    }
                                                                  return false;
                                                                }
                                                              if (__node_390->start_token < (*yynode)->start_token)
                                                                (*yynode)->start_token = __node_390->start_token;
                                                              (*yynode)->struct_declaration = __node_390;

                                                            }
                                                          else if (yytoken == Token_INTERFACE)
                                                            {
                                                              interface_declaration_ast *__node_391 = 0;
                                                              if (!parse_interface_declaration(&__node_391, attributes, modifiers, false ))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_interface_declaration, "interface_declaration");
                                                                    }
                                                                  return false;
                                                                }
                                                              if (__node_391->start_token < (*yynode)->start_token)
                                                                (*yynode)->start_token = __node_391->start_token;
                                                              (*yynode)->interface_declaration = __node_391;

                                                            }
                                                          else if (yytoken == Token_ENUM)
                                                            {
                                                              enum_declaration_ast *__node_392 = 0;
                                                              if (!parse_enum_declaration(&__node_392, attributes, modifiers ))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_enum_declaration, "enum_declaration");
                                                                    }
                                                                  return false;
                                                                }
                                                              if (__node_392->start_token < (*yynode)->start_token)
                                                                (*yynode)->start_token = __node_392->start_token;
                                                              (*yynode)->enum_declaration = __node_392;

                                                            }
                                                          else if (yytoken == Token_DELEGATE)
                                                            {
                                                              delegate_declaration_ast *__node_393 = 0;
                                                              if (!parse_delegate_declaration(&__node_393, attributes, modifiers ))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_delegate_declaration, "delegate_declaration");
                                                                    }
                                                                  return false;
                                                                }
                                                              if (__node_393->start_token < (*yynode)->start_token)
                                                                (*yynode)->start_token = __node_393->start_token;
                                                              (*yynode)->delegate_declaration = __node_393;

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

                                                    bool parser::parse_type_name(type_name_ast **yynode)
                                                    {
                                                      *yynode = create<type_name_ast>();

                                                      (*yynode)->start_token = token_stream->index() - 1;

                                                      if (yytoken == Token_ADD
                                                          || yytoken == Token_ALIAS
                                                          || yytoken == Token_GET
                                                          || yytoken == Token_GLOBAL
                                                          || yytoken == Token_PARTIAL
                                                          || yytoken == Token_REMOVE
                                                          || yytoken == Token_SET
                                                          || yytoken == Token_VALUE
                                                          || yytoken == Token_WHERE
                                                          || yytoken == Token_YIELD
                                                          || yytoken == Token_ASSEMBLY
                                                          || yytoken == Token_IDENTIFIER)
                                                        {
                                                          namespace_or_type_name_ast *__node_394 = 0;
                                                          if (!parse_namespace_or_type_name(&__node_394))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_namespace_or_type_name, "namespace_or_type_name");
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

                                                    bool parser::parse_type_name_safe(type_name_safe_ast **yynode)
                                                    {
                                                      *yynode = create<type_name_safe_ast>();

                                                      (*yynode)->start_token = token_stream->index() - 1;

                                                      if (yytoken == Token_ADD
                                                          || yytoken == Token_ALIAS
                                                          || yytoken == Token_GET
                                                          || yytoken == Token_GLOBAL
                                                          || yytoken == Token_PARTIAL
                                                          || yytoken == Token_REMOVE
                                                          || yytoken == Token_SET
                                                          || yytoken == Token_VALUE
                                                          || yytoken == Token_WHERE
                                                          || yytoken == Token_YIELD
                                                          || yytoken == Token_ASSEMBLY
                                                          || yytoken == Token_IDENTIFIER)
                                                        {
                                                          namespace_or_type_name_safe_ast *__node_395 = 0;
                                                          if (!parse_namespace_or_type_name_safe(&__node_395))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_namespace_or_type_name_safe, "namespace_or_type_name_safe");
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

                                                    bool parser::parse_type_parameter(type_parameter_ast **yynode)
                                                    {
                                                      *yynode = create<type_parameter_ast>();

                                                      (*yynode)->start_token = token_stream->index() - 1;

                                                      if (yytoken == Token_ADD
                                                          || yytoken == Token_ALIAS
                                                          || yytoken == Token_GET
                                                          || yytoken == Token_GLOBAL
                                                          || yytoken == Token_PARTIAL
                                                          || yytoken == Token_REMOVE
                                                          || yytoken == Token_SET
                                                          || yytoken == Token_VALUE
                                                          || yytoken == Token_WHERE
                                                          || yytoken == Token_YIELD
                                                          || yytoken == Token_ASSEMBLY
                                                          || yytoken == Token_LBRACKET
                                                          || yytoken == Token_IDENTIFIER)
                                                        {
                                                          optional_attribute_sections_ast *__node_396 = 0;
                                                          if (!parse_optional_attribute_sections(&__node_396))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_optional_attribute_sections, "optional_attribute_sections");
                                                                }
                                                              return false;
                                                            }
                                                          (*yynode)->attributes = __node_396;

                                                          identifier_ast *__node_397 = 0;
                                                          if (!parse_identifier(&__node_397))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                                                                }
                                                              return false;
                                                            }
                                                          (*yynode)->parameter_name = __node_397;

                                                        }
                                                      else
                                                        {
                                                          return false;
                                                        }

                                                      (*yynode)->end_token = token_stream->index() - 1;

                                                      return true;
                                                    }

                                                    bool parser::parse_type_parameter_constraints(type_parameter_constraints_ast **yynode)
                                                    {
                                                      *yynode = create<type_parameter_constraints_ast>();

                                                      (*yynode)->start_token = token_stream->index() - 1;

                                                      if (yytoken == Token_CLASS
                                                          || yytoken == Token_NEW
                                                          || yytoken == Token_OBJECT
                                                          || yytoken == Token_STRING
                                                          || yytoken == Token_STRUCT
                                                          || yytoken == Token_ADD
                                                          || yytoken == Token_ALIAS
                                                          || yytoken == Token_GET
                                                          || yytoken == Token_GLOBAL
                                                          || yytoken == Token_PARTIAL
                                                          || yytoken == Token_REMOVE
                                                          || yytoken == Token_SET
                                                          || yytoken == Token_VALUE
                                                          || yytoken == Token_WHERE
                                                          || yytoken == Token_YIELD
                                                          || yytoken == Token_ASSEMBLY
                                                          || yytoken == Token_IDENTIFIER)
                                                        {
                                                          if (yytoken == Token_CLASS
                                                              || yytoken == Token_OBJECT
                                                              || yytoken == Token_STRING
                                                              || yytoken == Token_STRUCT
                                                              || yytoken == Token_ADD
                                                              || yytoken == Token_ALIAS
                                                              || yytoken == Token_GET
                                                              || yytoken == Token_GLOBAL
                                                              || yytoken == Token_PARTIAL
                                                              || yytoken == Token_REMOVE
                                                              || yytoken == Token_SET
                                                              || yytoken == Token_VALUE
                                                              || yytoken == Token_WHERE
                                                              || yytoken == Token_YIELD
                                                              || yytoken == Token_ASSEMBLY
                                                              || yytoken == Token_IDENTIFIER)
                                                            {
                                                              primary_or_secondary_constraint_ast *__node_398 = 0;
                                                              if (!parse_primary_or_secondary_constraint(&__node_398))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_primary_or_secondary_constraint, "primary_or_secondary_constraint");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->primary_or_secondary_constraint = __node_398;

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

                                                                  if (yytoken == Token_ADD
                                                                      || yytoken == Token_ALIAS
                                                                      || yytoken == Token_GET
                                                                      || yytoken == Token_GLOBAL
                                                                      || yytoken == Token_PARTIAL
                                                                      || yytoken == Token_REMOVE
                                                                      || yytoken == Token_SET
                                                                      || yytoken == Token_VALUE
                                                                      || yytoken == Token_WHERE
                                                                      || yytoken == Token_YIELD
                                                                      || yytoken == Token_ASSEMBLY
                                                                      || yytoken == Token_IDENTIFIER)
                                                                    {
                                                                      secondary_constraints_ast *__node_399 = 0;
                                                                      if (!parse_secondary_constraints(&__node_399))
                                                                        {
                                                                          if (!yy_block_errors)
                                                                            {
                                                                              yy_expected_symbol(ast_node::Kind_secondary_constraints, "secondary_constraints");
                                                                            }
                                                                          return false;
                                                                        }
                                                                      (*yynode)->secondary_constraints = __node_399;

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

                                                                          constructor_constraint_ast *__node_400 = 0;
                                                                          if (!parse_constructor_constraint(&__node_400))
                                                                            {
                                                                              if (!yy_block_errors)
                                                                                {
                                                                                  yy_expected_symbol(ast_node::Kind_constructor_constraint, "constructor_constraint");
                                                                                }
                                                                              return false;
                                                                            }
                                                                          (*yynode)->constructor_constraint = __node_400;

                                                                        }
                                                                      else if (true /*epsilon*/)
                                                                      {}
                                                                      else
                                                                        {
                                                                          return false;
                                                                        }
                                                                    }
                                                                  else if (yytoken == Token_NEW)
                                                                    {
                                                                      constructor_constraint_ast *__node_401 = 0;
                                                                      if (!parse_constructor_constraint(&__node_401))
                                                                        {
                                                                          if (!yy_block_errors)
                                                                            {
                                                                              yy_expected_symbol(ast_node::Kind_constructor_constraint, "constructor_constraint");
                                                                            }
                                                                          return false;
                                                                        }
                                                                      (*yynode)->constructor_constraint = __node_401;

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
                                                          else if (yytoken == Token_ADD
                                                                   || yytoken == Token_ALIAS
                                                                   || yytoken == Token_GET
                                                                   || yytoken == Token_GLOBAL
                                                                   || yytoken == Token_PARTIAL
                                                                   || yytoken == Token_REMOVE
                                                                   || yytoken == Token_SET
                                                                   || yytoken == Token_VALUE
                                                                   || yytoken == Token_WHERE
                                                                   || yytoken == Token_YIELD
                                                                   || yytoken == Token_ASSEMBLY
                                                                   || yytoken == Token_IDENTIFIER)
                                                            {
                                                              secondary_constraints_ast *__node_402 = 0;
                                                              if (!parse_secondary_constraints(&__node_402))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_secondary_constraints, "secondary_constraints");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->secondary_constraints = __node_402;

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

                                                                  constructor_constraint_ast *__node_403 = 0;
                                                                  if (!parse_constructor_constraint(&__node_403))
                                                                    {
                                                                      if (!yy_block_errors)
                                                                        {
                                                                          yy_expected_symbol(ast_node::Kind_constructor_constraint, "constructor_constraint");
                                                                        }
                                                                      return false;
                                                                    }
                                                                  (*yynode)->constructor_constraint = __node_403;

                                                                }
                                                              else if (true /*epsilon*/)
                                                              {}
                                                              else
                                                                {
                                                                  return false;
                                                                }
                                                            }
                                                          else if (yytoken == Token_NEW)
                                                            {
                                                              constructor_constraint_ast *__node_404 = 0;
                                                              if (!parse_constructor_constraint(&__node_404))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_constructor_constraint, "constructor_constraint");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->constructor_constraint = __node_404;

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

                                                    bool parser::parse_type_parameter_constraints_clause(type_parameter_constraints_clause_ast **yynode)
                                                    {
                                                      *yynode = create<type_parameter_constraints_clause_ast>();

                                                      (*yynode)->start_token = token_stream->index() - 1;

                                                      if (yytoken == Token_WHERE)
                                                        {
                                                          if (yytoken != Token_WHERE)
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_token(yytoken, Token_WHERE, "where");
                                                                }
                                                              return false;
                                                            }
                                                          yylex();

                                                          identifier_ast *__node_405 = 0;
                                                          if (!parse_identifier(&__node_405))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                                                                }
                                                              return false;
                                                            }
                                                          (*yynode)->type_parameter = __node_405;

                                                          if (yytoken != Token_COLON)
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_token(yytoken, Token_COLON, ":");
                                                                }
                                                              return false;
                                                            }
                                                          yylex();

                                                          type_parameter_constraints_ast *__node_406 = 0;
                                                          if (!parse_type_parameter_constraints(&__node_406))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_type_parameter_constraints, "type_parameter_constraints");
                                                                }
                                                              return false;
                                                            }
                                                          (*yynode)->constraints = __node_406;

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
                                                          type_parameter_ast *__node_407 = 0;
                                                          if (!parse_type_parameter(&__node_407))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_type_parameter, "type_parameter");
                                                                }
                                                              return false;
                                                            }
                                                          (*yynode)->type_parameter_sequence = snoc((*yynode)->type_parameter_sequence, __node_407, memory_pool);

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

                                                              type_parameter_ast *__node_408 = 0;
                                                              if (!parse_type_parameter(&__node_408))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_type_parameter, "type_parameter");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->type_parameter_sequence = snoc((*yynode)->type_parameter_sequence, __node_408, memory_pool);

                                                            }
                                                          if (yytoken == Token_GREATER_THAN
                                                              || yytoken == Token_RSHIFT)
                                                            {
                                                              type_arguments_or_parameters_end_ast *__node_409 = 0;
                                                              if (!parse_type_arguments_or_parameters_end(&__node_409))
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

                                                    bool parser::parse_typeof_expression(typeof_expression_ast **yynode)
                                                    {
                                                      *yynode = create<typeof_expression_ast>();

                                                      (*yynode)->start_token = token_stream->index() - 1;

                                                      if (yytoken == Token_TYPEOF)
                                                        {
                                                          if (yytoken != Token_TYPEOF)
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_token(yytoken, Token_TYPEOF, "typeof");
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

                                                          if ((yytoken == Token_VOID) && ( LA(2).kind == Token_RPAREN ))
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

                                                              (*yynode)->typeof_type = typeof_expression::type_void;
                                                            }
                                                          else if ((yytoken == Token_BOOL
                                                                    || yytoken == Token_BYTE
                                                                    || yytoken == Token_CHAR
                                                                    || yytoken == Token_DECIMAL
                                                                    || yytoken == Token_DOUBLE
                                                                    || yytoken == Token_FLOAT
                                                                    || yytoken == Token_INT
                                                                    || yytoken == Token_LONG
                                                                    || yytoken == Token_OBJECT
                                                                    || yytoken == Token_SBYTE
                                                                    || yytoken == Token_SHORT
                                                                    || yytoken == Token_STRING
                                                                    || yytoken == Token_UINT
                                                                    || yytoken == Token_ULONG
                                                                    || yytoken == Token_USHORT
                                                                    || yytoken == Token_VOID
                                                                    || yytoken == Token_ADD
                                                                    || yytoken == Token_ALIAS
                                                                    || yytoken == Token_GET
                                                                    || yytoken == Token_GLOBAL
                                                                    || yytoken == Token_PARTIAL
                                                                    || yytoken == Token_REMOVE
                                                                    || yytoken == Token_SET
                                                                    || yytoken == Token_VALUE
                                                                    || yytoken == Token_WHERE
                                                                    || yytoken == Token_YIELD
                                                                    || yytoken == Token_ASSEMBLY
                                                                    || yytoken == Token_IDENTIFIER) && ( compatibility_mode() >= csharp20_compatibility ))
                                                            {
                                                              bool block_errors_39 = block_errors(true);
                                                              std::size_t try_start_token_39 = token_stream->index() - 1;
                                                              parser_state *try_start_state_39 = copy_current_state();
                                                              {
                                                                unbound_type_name_ast *__node_410 = 0;
                                                                if (!parse_unbound_type_name(&__node_410))
                                                                  {
                                                                    goto __catch_39;
                                                                  }
                                                                (*yynode)->unbound_type_name = __node_410;

                                                                (*yynode)->typeof_type = typeof_expression::type_unbound_type_name;
                                                              }
                                                              block_errors(block_errors_39);
                                                              if (try_start_state_39)
                                                                delete try_start_state_39;

                                                              if (false) // the only way to enter here is using goto
                                                                {
                                                                __catch_39:
                                                                  if (try_start_state_39)
                                                                    {
                                                                      restore_state(try_start_state_39);
                                                                      delete try_start_state_39;
                                                                    }
                                                                  block_errors(block_errors_39);
                                                                  rewind(try_start_token_39);

                                                                  type_ast *__node_411 = 0;
                                                                  if (!parse_type(&__node_411))
                                                                    {
                                                                      if (!yy_block_errors)
                                                                        {
                                                                          yy_expected_symbol(ast_node::Kind_type, "type");
                                                                        }
                                                                      return false;
                                                                    }
                                                                  (*yynode)->other_type = __node_411;

                                                                  (*yynode)->typeof_type = typeof_expression::type_type;
                                                                }

                                                            }
                                                          else if (yytoken == Token_BOOL
                                                                   || yytoken == Token_BYTE
                                                                   || yytoken == Token_CHAR
                                                                   || yytoken == Token_DECIMAL
                                                                   || yytoken == Token_DOUBLE
                                                                   || yytoken == Token_FLOAT
                                                                   || yytoken == Token_INT
                                                                   || yytoken == Token_LONG
                                                                   || yytoken == Token_OBJECT
                                                                   || yytoken == Token_SBYTE
                                                                   || yytoken == Token_SHORT
                                                                   || yytoken == Token_STRING
                                                                   || yytoken == Token_UINT
                                                                   || yytoken == Token_ULONG
                                                                   || yytoken == Token_USHORT
                                                                   || yytoken == Token_VOID
                                                                   || yytoken == Token_ADD
                                                                   || yytoken == Token_ALIAS
                                                                   || yytoken == Token_GET
                                                                   || yytoken == Token_GLOBAL
                                                                   || yytoken == Token_PARTIAL
                                                                   || yytoken == Token_REMOVE
                                                                   || yytoken == Token_SET
                                                                   || yytoken == Token_VALUE
                                                                   || yytoken == Token_WHERE
                                                                   || yytoken == Token_YIELD
                                                                   || yytoken == Token_ASSEMBLY
                                                                   || yytoken == Token_IDENTIFIER)
                                                            {
                                                              type_ast *__node_412 = 0;
                                                              if (!parse_type(&__node_412))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_type, "type");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->other_type = __node_412;

                                                              (*yynode)->typeof_type = typeof_expression::type_type;
                                                            }
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

                                                    bool parser::parse_unary_expression(unary_expression_ast **yynode)
                                                    {
                                                      *yynode = create<unary_expression_ast>();

                                                      (*yynode)->start_token = token_stream->index() - 1;

                                                      if (yytoken == Token_BASE
                                                          || yytoken == Token_BOOL
                                                          || yytoken == Token_BYTE
                                                          || yytoken == Token_CHAR
                                                          || yytoken == Token_CHECKED
                                                          || yytoken == Token_DECIMAL
                                                          || yytoken == Token_DEFAULT
                                                          || yytoken == Token_DELEGATE
                                                          || yytoken == Token_DOUBLE
                                                          || yytoken == Token_FLOAT
                                                          || yytoken == Token_INT
                                                          || yytoken == Token_LONG
                                                          || yytoken == Token_NEW
                                                          || yytoken == Token_OBJECT
                                                          || yytoken == Token_SBYTE
                                                          || yytoken == Token_SHORT
                                                          || yytoken == Token_SIZEOF
                                                          || yytoken == Token_STRING
                                                          || yytoken == Token_THIS
                                                          || yytoken == Token_TYPEOF
                                                          || yytoken == Token_UINT
                                                          || yytoken == Token_ULONG
                                                          || yytoken == Token_UNCHECKED
                                                          || yytoken == Token_USHORT
                                                          || yytoken == Token_ADD
                                                          || yytoken == Token_ALIAS
                                                          || yytoken == Token_GET
                                                          || yytoken == Token_GLOBAL
                                                          || yytoken == Token_PARTIAL
                                                          || yytoken == Token_REMOVE
                                                          || yytoken == Token_SET
                                                          || yytoken == Token_VALUE
                                                          || yytoken == Token_WHERE
                                                          || yytoken == Token_YIELD
                                                          || yytoken == Token_ASSEMBLY
                                                          || yytoken == Token_LPAREN
                                                          || yytoken == Token_BANG
                                                          || yytoken == Token_TILDE
                                                          || yytoken == Token_INCREMENT
                                                          || yytoken == Token_DECREMENT
                                                          || yytoken == Token_PLUS
                                                          || yytoken == Token_MINUS
                                                          || yytoken == Token_STAR
                                                          || yytoken == Token_BIT_AND
                                                          || yytoken == Token_TRUE
                                                          || yytoken == Token_FALSE
                                                          || yytoken == Token_NULL
                                                          || yytoken == Token_INTEGER_LITERAL
                                                          || yytoken == Token_REAL_LITERAL
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

                                                              unary_expression_ast *__node_413 = 0;
                                                              if (!parse_unary_expression(&__node_413))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_unary_expression, "unary_expression");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->unary_expression = __node_413;

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

                                                              unary_expression_ast *__node_414 = 0;
                                                              if (!parse_unary_expression(&__node_414))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_unary_expression, "unary_expression");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->unary_expression = __node_414;

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

                                                              unary_expression_ast *__node_415 = 0;
                                                              if (!parse_unary_expression(&__node_415))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_unary_expression, "unary_expression");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->unary_expression = __node_415;

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

                                                              unary_expression_ast *__node_416 = 0;
                                                              if (!parse_unary_expression(&__node_416))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_unary_expression, "unary_expression");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->unary_expression = __node_416;

                                                              (*yynode)->rule_type = unary_expression::type_unary_plus_expression;
                                                            }
                                                          else if (yytoken == Token_TILDE)
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

                                                              unary_expression_ast *__node_417 = 0;
                                                              if (!parse_unary_expression(&__node_417))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_unary_expression, "unary_expression");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->unary_expression = __node_417;

                                                              (*yynode)->rule_type = unary_expression::type_bitwise_not_expression;
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

                                                              unary_expression_ast *__node_418 = 0;
                                                              if (!parse_unary_expression(&__node_418))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_unary_expression, "unary_expression");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->unary_expression = __node_418;

                                                              (*yynode)->rule_type = unary_expression::type_logical_not_expression;
                                                            }
                                                          else if (yytoken == Token_BASE
                                                                   || yytoken == Token_BOOL
                                                                   || yytoken == Token_BYTE
                                                                   || yytoken == Token_CHAR
                                                                   || yytoken == Token_CHECKED
                                                                   || yytoken == Token_DECIMAL
                                                                   || yytoken == Token_DEFAULT
                                                                   || yytoken == Token_DELEGATE
                                                                   || yytoken == Token_DOUBLE
                                                                   || yytoken == Token_FLOAT
                                                                   || yytoken == Token_INT
                                                                   || yytoken == Token_LONG
                                                                   || yytoken == Token_NEW
                                                                   || yytoken == Token_OBJECT
                                                                   || yytoken == Token_SBYTE
                                                                   || yytoken == Token_SHORT
                                                                   || yytoken == Token_SIZEOF
                                                                   || yytoken == Token_STRING
                                                                   || yytoken == Token_THIS
                                                                   || yytoken == Token_TYPEOF
                                                                   || yytoken == Token_UINT
                                                                   || yytoken == Token_ULONG
                                                                   || yytoken == Token_UNCHECKED
                                                                   || yytoken == Token_USHORT
                                                                   || yytoken == Token_ADD
                                                                   || yytoken == Token_ALIAS
                                                                   || yytoken == Token_GET
                                                                   || yytoken == Token_GLOBAL
                                                                   || yytoken == Token_PARTIAL
                                                                   || yytoken == Token_REMOVE
                                                                   || yytoken == Token_SET
                                                                   || yytoken == Token_VALUE
                                                                   || yytoken == Token_WHERE
                                                                   || yytoken == Token_YIELD
                                                                   || yytoken == Token_ASSEMBLY
                                                                   || yytoken == Token_LPAREN
                                                                   || yytoken == Token_TRUE
                                                                   || yytoken == Token_FALSE
                                                                   || yytoken == Token_NULL
                                                                   || yytoken == Token_INTEGER_LITERAL
                                                                   || yytoken == Token_REAL_LITERAL
                                                                   || yytoken == Token_CHARACTER_LITERAL
                                                                   || yytoken == Token_STRING_LITERAL
                                                                   || yytoken == Token_IDENTIFIER)
                                                            {
                                                              bool block_errors_40 = block_errors(true);
                                                              std::size_t try_start_token_40 = token_stream->index() - 1;
                                                              parser_state *try_start_state_40 = copy_current_state();
                                                              {
                                                                cast_expression_ast *__node_419 = 0;
                                                                if (!parse_cast_expression(&__node_419))
                                                                  {
                                                                    goto __catch_40;
                                                                  }
                                                                (*yynode)->cast_expression = __node_419;

                                                                (*yynode)->rule_type = unary_expression::type_cast_expression;
                                                              }
                                                              block_errors(block_errors_40);
                                                              if (try_start_state_40)
                                                                delete try_start_state_40;

                                                              if (false) // the only way to enter here is using goto
                                                                {
                                                                __catch_40:
                                                                  if (try_start_state_40)
                                                                    {
                                                                      restore_state(try_start_state_40);
                                                                      delete try_start_state_40;
                                                                    }
                                                                  block_errors(block_errors_40);
                                                                  rewind(try_start_token_40);

                                                                  primary_expression_ast *__node_420 = 0;
                                                                  if (!parse_primary_expression(&__node_420))
                                                                    {
                                                                      if (!yy_block_errors)
                                                                        {
                                                                          yy_expected_symbol(ast_node::Kind_primary_expression, "primary_expression");
                                                                        }
                                                                      return false;
                                                                    }
                                                                  (*yynode)->primary_expression = __node_420;

                                                                  (*yynode)->rule_type = unary_expression::type_primary_expression;
                                                                }

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

                                                              unary_expression_ast *__node_421 = 0;
                                                              if (!parse_unary_expression(&__node_421))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_unary_expression, "unary_expression");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->unary_expression = __node_421;

                                                              (*yynode)->rule_type = unary_expression::type_pointer_indirection_expression;
                                                            }
                                                          else if (yytoken == Token_BIT_AND)
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

                                                              unary_expression_ast *__node_422 = 0;
                                                              if (!parse_unary_expression(&__node_422))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_unary_expression, "unary_expression");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->unary_expression = __node_422;

                                                              (*yynode)->rule_type = unary_expression::type_addressof_expression;
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

                                                    bool parser::parse_unary_or_binary_operator_declaration(unary_or_binary_operator_declaration_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers, type_ast *return_type)
                                                    {
                                                      *yynode = create<unary_or_binary_operator_declaration_ast>();

                                                      (*yynode)->start_token = token_stream->index() - 1;

                                                      (*yynode)->attributes = attributes;
                                                      if (attributes && attributes->start_token < (*yynode)->start_token)
                                                        (*yynode)->start_token = attributes->start_token;

                                                      (*yynode)->modifiers = modifiers;
                                                      if (modifiers && modifiers->start_token < (*yynode)->start_token)
                                                        (*yynode)->start_token = modifiers->start_token;

                                                      (*yynode)->return_type = return_type;
                                                      if (return_type && return_type->start_token < (*yynode)->start_token)
                                                        (*yynode)->start_token = return_type->start_token;

                                                      if (yytoken == Token_OPERATOR)
                                                        {
                                                          overloadable_unary_only_operator_ast *unary_op = 0;

                                                          overloadable_binary_only_operator_ast *binary_op = 0;

                                                          overloadable_unary_or_binary_operator_ast *unary_or_binary_op = 0;

                                                          if (yytoken != Token_OPERATOR)
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_token(yytoken, Token_OPERATOR, "operator");
                                                                }
                                                              return false;
                                                            }
                                                          yylex();

                                                          if (yytoken == Token_BANG
                                                              || yytoken == Token_TILDE
                                                              || yytoken == Token_INCREMENT
                                                              || yytoken == Token_DECREMENT
                                                              || yytoken == Token_TRUE
                                                              || yytoken == Token_FALSE)
                                                            {
                                                              overloadable_unary_only_operator_ast *__node_423 = 0;
                                                              if (!parse_overloadable_unary_only_operator(&__node_423, &(*yynode)->overloadable_operator_type))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_overloadable_unary_only_operator, "overloadable_unary_only_operator");
                                                                    }
                                                                  return false;
                                                                }
                                                              unary_op = __node_423;

                                                              if (yytoken != Token_LPAREN)
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_token(yytoken, Token_LPAREN, "(");
                                                                    }
                                                                  return false;
                                                                }
                                                              yylex();

                                                              type_ast *__node_424 = 0;
                                                              if (!parse_type(&__node_424))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_type, "type");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->source1_type = __node_424;

                                                              identifier_ast *__node_425 = 0;
                                                              if (!parse_identifier(&__node_425))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->source1_name = __node_425;

                                                              if (yytoken != Token_RPAREN)
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_token(yytoken, Token_RPAREN, ")");
                                                                    }
                                                                  return false;
                                                                }
                                                              yylex();

                                                              (*yynode)->overloadable_operator_token = unary_op->op;
                                                              (*yynode)->unary_or_binary = overloadable_operator::type_unary;
                                                            }
                                                          else if (yytoken == Token_EQUAL
                                                                   || yytoken == Token_LESS_THAN
                                                                   || yytoken == Token_LESS_EQUAL
                                                                   || yytoken == Token_GREATER_THAN
                                                                   || yytoken == Token_GREATER_EQUAL
                                                                   || yytoken == Token_NOT_EQUAL
                                                                   || yytoken == Token_STAR
                                                                   || yytoken == Token_SLASH
                                                                   || yytoken == Token_BIT_AND
                                                                   || yytoken == Token_BIT_OR
                                                                   || yytoken == Token_BIT_XOR
                                                                   || yytoken == Token_REMAINDER
                                                                   || yytoken == Token_LSHIFT
                                                                   || yytoken == Token_RSHIFT)
                                                            {
                                                              overloadable_binary_only_operator_ast *__node_426 = 0;
                                                              if (!parse_overloadable_binary_only_operator(&__node_426, &(*yynode)->overloadable_operator_type))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_overloadable_binary_only_operator, "overloadable_binary_only_operator");
                                                                    }
                                                                  return false;
                                                                }
                                                              binary_op = __node_426;

                                                              if (yytoken != Token_LPAREN)
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_token(yytoken, Token_LPAREN, "(");
                                                                    }
                                                                  return false;
                                                                }
                                                              yylex();

                                                              type_ast *__node_427 = 0;
                                                              if (!parse_type(&__node_427))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_type, "type");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->source1_type = __node_427;

                                                              identifier_ast *__node_428 = 0;
                                                              if (!parse_identifier(&__node_428))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->source1_name = __node_428;

                                                              if (yytoken != Token_COMMA)
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_token(yytoken, Token_COMMA, ",");
                                                                    }
                                                                  return false;
                                                                }
                                                              yylex();

                                                              type_ast *__node_429 = 0;
                                                              if (!parse_type(&__node_429))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_type, "type");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->source2_type = __node_429;

                                                              identifier_ast *__node_430 = 0;
                                                              if (!parse_identifier(&__node_430))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->source2_name = __node_430;

                                                              if (yytoken != Token_RPAREN)
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_token(yytoken, Token_RPAREN, ")");
                                                                    }
                                                                  return false;
                                                                }
                                                              yylex();

                                                              (*yynode)->overloadable_operator_token = binary_op->op;
                                                              (*yynode)->unary_or_binary = overloadable_operator::type_binary;
                                                            }
                                                          else if (yytoken == Token_PLUS
                                                                   || yytoken == Token_MINUS)
                                                            {
                                                              overloadable_unary_or_binary_operator_ast *__node_431 = 0;
                                                              if (!parse_overloadable_unary_or_binary_operator(&__node_431, &(*yynode)->overloadable_operator_type))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_overloadable_unary_or_binary_operator, "overloadable_unary_or_binary_operator");
                                                                    }
                                                                  return false;
                                                                }
                                                              unary_or_binary_op = __node_431;

                                                              (*yynode)->overloadable_operator_token = unary_or_binary_op->op;
                                                              if (yytoken != Token_LPAREN)
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_token(yytoken, Token_LPAREN, "(");
                                                                    }
                                                                  return false;
                                                                }
                                                              yylex();

                                                              type_ast *__node_432 = 0;
                                                              if (!parse_type(&__node_432))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_type, "type");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->source1_type = __node_432;

                                                              identifier_ast *__node_433 = 0;
                                                              if (!parse_identifier(&__node_433))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->source1_name = __node_433;

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

                                                                  type_ast *__node_434 = 0;
                                                                  if (!parse_type(&__node_434))
                                                                    {
                                                                      if (!yy_block_errors)
                                                                        {
                                                                          yy_expected_symbol(ast_node::Kind_type, "type");
                                                                        }
                                                                      return false;
                                                                    }
                                                                  (*yynode)->source2_type = __node_434;

                                                                  identifier_ast *__node_435 = 0;
                                                                  if (!parse_identifier(&__node_435))
                                                                    {
                                                                      if (!yy_block_errors)
                                                                        {
                                                                          yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                                                                        }
                                                                      return false;
                                                                    }
                                                                  (*yynode)->source2_name = __node_435;

                                                                  (*yynode)->unary_or_binary = overloadable_operator::type_binary;
                                                                }
                                                              else if (true /*epsilon*/)
                                                                {
                                                                  (*yynode)->unary_or_binary = overloadable_operator::type_unary;
                                                                }
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
                                                          if (yytoken == Token_LBRACE)
                                                            {
                                                              block_ast *__node_436 = 0;
                                                              if (!parse_block(&__node_436))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_block, "block");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->body = __node_436;

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

                                                    bool parser::parse_unbound_type_name(unbound_type_name_ast **yynode)
                                                    {
                                                      *yynode = create<unbound_type_name_ast>();

                                                      (*yynode)->start_token = token_stream->index() - 1;

                                                      if (yytoken == Token_ADD
                                                          || yytoken == Token_ALIAS
                                                          || yytoken == Token_GET
                                                          || yytoken == Token_GLOBAL
                                                          || yytoken == Token_PARTIAL
                                                          || yytoken == Token_REMOVE
                                                          || yytoken == Token_SET
                                                          || yytoken == Token_VALUE
                                                          || yytoken == Token_WHERE
                                                          || yytoken == Token_YIELD
                                                          || yytoken == Token_ASSEMBLY
                                                          || yytoken == Token_IDENTIFIER)
                                                        {
                                                          if ((yytoken == Token_ADD
                                                               || yytoken == Token_ALIAS
                                                               || yytoken == Token_GET
                                                               || yytoken == Token_GLOBAL
                                                               || yytoken == Token_PARTIAL
                                                               || yytoken == Token_REMOVE
                                                               || yytoken == Token_SET
                                                               || yytoken == Token_VALUE
                                                               || yytoken == Token_WHERE
                                                               || yytoken == Token_YIELD
                                                               || yytoken == Token_ASSEMBLY
                                                               || yytoken == Token_IDENTIFIER) && ( LA(2).kind == Token_SCOPE ))
                                                            {
                                                              identifier_ast *__node_437 = 0;
                                                              if (!parse_identifier(&__node_437))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->qualified_alias_label = __node_437;

                                                              if (yytoken != Token_SCOPE)
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_token(yytoken, Token_SCOPE, "::");
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
                                                          unbound_type_name_part_ast *__node_438 = 0;
                                                          if (!parse_unbound_type_name_part(&__node_438))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_unbound_type_name_part, "unbound_type_name_part");
                                                                }
                                                              return false;
                                                            }
                                                          (*yynode)->name_part_sequence = snoc((*yynode)->name_part_sequence, __node_438, memory_pool);

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

                                                              unbound_type_name_part_ast *__node_439 = 0;
                                                              if (!parse_unbound_type_name_part(&__node_439))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_unbound_type_name_part, "unbound_type_name_part");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->name_part_sequence = snoc((*yynode)->name_part_sequence, __node_439, memory_pool);

                                                            }
                                                        }
                                                      else
                                                        {
                                                          return false;
                                                        }

                                                      (*yynode)->end_token = token_stream->index() - 1;

                                                      return true;
                                                    }

                                                    bool parser::parse_unbound_type_name_part(unbound_type_name_part_ast **yynode)
                                                    {
                                                      *yynode = create<unbound_type_name_part_ast>();

                                                      (*yynode)->start_token = token_stream->index() - 1;

                                                      if (yytoken == Token_ADD
                                                          || yytoken == Token_ALIAS
                                                          || yytoken == Token_GET
                                                          || yytoken == Token_GLOBAL
                                                          || yytoken == Token_PARTIAL
                                                          || yytoken == Token_REMOVE
                                                          || yytoken == Token_SET
                                                          || yytoken == Token_VALUE
                                                          || yytoken == Token_WHERE
                                                          || yytoken == Token_YIELD
                                                          || yytoken == Token_ASSEMBLY
                                                          || yytoken == Token_IDENTIFIER)
                                                        {
                                                          identifier_ast *__node_440 = 0;
                                                          if (!parse_identifier(&__node_440))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                                                                }
                                                              return false;
                                                            }
                                                          (*yynode)->identifier = __node_440;

                                                          generic_dimension_specifier_ast *__node_441 = 0;
                                                          if (!parse_generic_dimension_specifier(&__node_441))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_generic_dimension_specifier, "generic_dimension_specifier");
                                                                }
                                                              return false;
                                                            }
                                                          (*yynode)->generic_dimension_specifier = __node_441;

                                                        }
                                                      else
                                                        {
                                                          return false;
                                                        }

                                                      (*yynode)->end_token = token_stream->index() - 1;

                                                      return true;
                                                    }

                                                    bool parser::parse_unchecked_statement(unchecked_statement_ast **yynode)
                                                    {
                                                      *yynode = create<unchecked_statement_ast>();

                                                      (*yynode)->start_token = token_stream->index() - 1;

                                                      if (yytoken == Token_UNCHECKED)
                                                        {
                                                          if (yytoken != Token_UNCHECKED)
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_token(yytoken, Token_UNCHECKED, "unchecked");
                                                                }
                                                              return false;
                                                            }
                                                          yylex();

                                                          block_ast *__node_442 = 0;
                                                          if (!parse_block(&__node_442))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_block, "block");
                                                                }
                                                              return false;
                                                            }
                                                          (*yynode)->body = __node_442;

                                                        }
                                                      else
                                                        {
                                                          return false;
                                                        }

                                                      (*yynode)->end_token = token_stream->index() - 1;

                                                      return true;
                                                    }

                                                    bool parser::parse_unmanaged_type(unmanaged_type_ast **yynode)
                                                    {
                                                      *yynode = create<unmanaged_type_ast>();

                                                      (*yynode)->start_token = token_stream->index() - 1;

                                                      if (yytoken == Token_BOOL
                                                          || yytoken == Token_BYTE
                                                          || yytoken == Token_CHAR
                                                          || yytoken == Token_DECIMAL
                                                          || yytoken == Token_DOUBLE
                                                          || yytoken == Token_FLOAT
                                                          || yytoken == Token_INT
                                                          || yytoken == Token_LONG
                                                          || yytoken == Token_OBJECT
                                                          || yytoken == Token_SBYTE
                                                          || yytoken == Token_SHORT
                                                          || yytoken == Token_STRING
                                                          || yytoken == Token_UINT
                                                          || yytoken == Token_ULONG
                                                          || yytoken == Token_USHORT
                                                          || yytoken == Token_VOID
                                                          || yytoken == Token_ADD
                                                          || yytoken == Token_ALIAS
                                                          || yytoken == Token_GET
                                                          || yytoken == Token_GLOBAL
                                                          || yytoken == Token_PARTIAL
                                                          || yytoken == Token_REMOVE
                                                          || yytoken == Token_SET
                                                          || yytoken == Token_VALUE
                                                          || yytoken == Token_WHERE
                                                          || yytoken == Token_YIELD
                                                          || yytoken == Token_ASSEMBLY
                                                          || yytoken == Token_IDENTIFIER)
                                                        {
                                                          if (yytoken == Token_BOOL
                                                              || yytoken == Token_BYTE
                                                              || yytoken == Token_CHAR
                                                              || yytoken == Token_DECIMAL
                                                              || yytoken == Token_DOUBLE
                                                              || yytoken == Token_FLOAT
                                                              || yytoken == Token_INT
                                                              || yytoken == Token_LONG
                                                              || yytoken == Token_OBJECT
                                                              || yytoken == Token_SBYTE
                                                              || yytoken == Token_SHORT
                                                              || yytoken == Token_STRING
                                                              || yytoken == Token_UINT
                                                              || yytoken == Token_ULONG
                                                              || yytoken == Token_USHORT
                                                              || yytoken == Token_ADD
                                                              || yytoken == Token_ALIAS
                                                              || yytoken == Token_GET
                                                              || yytoken == Token_GLOBAL
                                                              || yytoken == Token_PARTIAL
                                                              || yytoken == Token_REMOVE
                                                              || yytoken == Token_SET
                                                              || yytoken == Token_VALUE
                                                              || yytoken == Token_WHERE
                                                              || yytoken == Token_YIELD
                                                              || yytoken == Token_ASSEMBLY
                                                              || yytoken == Token_IDENTIFIER)
                                                            {
                                                              non_array_type_ast *__node_443 = 0;
                                                              if (!parse_non_array_type(&__node_443))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_non_array_type, "non_array_type");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->regular_type = __node_443;

                                                              (*yynode)->type = pointer_type::type_regular;
                                                            }
                                                          else if (yytoken == Token_VOID)
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

                                                              if (yytoken != Token_STAR)
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_token(yytoken, Token_STAR, "*");
                                                                    }
                                                                  return false;
                                                                }
                                                              yylex();

                                                              (*yynode)->type = pointer_type::type_void_star;
                                                            }
                                                          else
                                                            {
                                                              return false;
                                                            }
                                                          while (yytoken == Token_LBRACKET
                                                                 || yytoken == Token_STAR)
                                                            {
                                                              if (yytoken == Token_LBRACKET &&
                                                                  LA(2).kind != Token_COMMA && LA(2).kind != Token_RBRACKET)
                                                                {
                                                                  break;
                                                                }
                                                              unmanaged_type_suffix_ast *__node_444 = 0;
                                                              if (!parse_unmanaged_type_suffix(&__node_444))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_unmanaged_type_suffix, "unmanaged_type_suffix");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->unmanaged_type_suffix_sequence = snoc((*yynode)->unmanaged_type_suffix_sequence, __node_444, memory_pool);

                                                            }
                                                        }
                                                      else
                                                        {
                                                          return false;
                                                        }

                                                      (*yynode)->end_token = token_stream->index() - 1;

                                                      return true;
                                                    }

                                                    bool parser::parse_unmanaged_type_suffix(unmanaged_type_suffix_ast **yynode)
                                                    {
                                                      *yynode = create<unmanaged_type_suffix_ast>();

                                                      (*yynode)->start_token = token_stream->index() - 1;

                                                      if (yytoken == Token_LBRACKET
                                                          || yytoken == Token_STAR)
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

                                                              (*yynode)->type = unmanaged_type_suffix::type_star;
                                                            }
                                                          else if (yytoken == Token_LBRACKET)
                                                            {
                                                              rank_specifier_ast *__node_445 = 0;
                                                              if (!parse_rank_specifier(&__node_445))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_rank_specifier, "rank_specifier");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->rank_specifier = __node_445;

                                                              (*yynode)->type = unmanaged_type_suffix::type_rank_specifier;
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

                                                    bool parser::parse_unsafe_statement(unsafe_statement_ast **yynode)
                                                    {
                                                      *yynode = create<unsafe_statement_ast>();

                                                      (*yynode)->start_token = token_stream->index() - 1;

                                                      if (yytoken == Token_UNSAFE)
                                                        {
                                                          if (yytoken != Token_UNSAFE)
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_token(yytoken, Token_UNSAFE, "unsafe");
                                                                }
                                                              return false;
                                                            }
                                                          yylex();

                                                          block_ast *__node_446 = 0;
                                                          if (!parse_block(&__node_446))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_block, "block");
                                                                }
                                                              return false;
                                                            }
                                                          (*yynode)->body = __node_446;

                                                        }
                                                      else
                                                        {
                                                          return false;
                                                        }

                                                      (*yynode)->end_token = token_stream->index() - 1;

                                                      return true;
                                                    }

                                                    bool parser::parse_using_directive(using_directive_ast **yynode)
                                                    {
                                                      *yynode = create<using_directive_ast>();

                                                      (*yynode)->start_token = token_stream->index() - 1;

                                                      if (yytoken == Token_USING)
                                                        {
                                                          if (yytoken != Token_USING)
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_token(yytoken, Token_USING, "using");
                                                                }
                                                              return false;
                                                            }
                                                          yylex();

                                                          if ((yytoken == Token_ADD
                                                               || yytoken == Token_ALIAS
                                                               || yytoken == Token_GET
                                                               || yytoken == Token_GLOBAL
                                                               || yytoken == Token_PARTIAL
                                                               || yytoken == Token_REMOVE
                                                               || yytoken == Token_SET
                                                               || yytoken == Token_VALUE
                                                               || yytoken == Token_WHERE
                                                               || yytoken == Token_YIELD
                                                               || yytoken == Token_ASSEMBLY
                                                               || yytoken == Token_IDENTIFIER) && ( LA(2).kind == Token_ASSIGN ))
                                                            {
                                                              identifier_ast *__node_447 = 0;
                                                              if (!parse_identifier(&__node_447))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->alias = __node_447;

                                                              if (yytoken != Token_ASSIGN)
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_token(yytoken, Token_ASSIGN, "=");
                                                                    }
                                                                  return false;
                                                                }
                                                              yylex();

                                                              namespace_or_type_name_ast *__node_448 = 0;
                                                              if (!parse_namespace_or_type_name(&__node_448))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_namespace_or_type_name, "namespace_or_type_name");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->namespace_or_type_name = __node_448;

                                                            }
                                                          else if (yytoken == Token_ADD
                                                                   || yytoken == Token_ALIAS
                                                                   || yytoken == Token_GET
                                                                   || yytoken == Token_GLOBAL
                                                                   || yytoken == Token_PARTIAL
                                                                   || yytoken == Token_REMOVE
                                                                   || yytoken == Token_SET
                                                                   || yytoken == Token_VALUE
                                                                   || yytoken == Token_WHERE
                                                                   || yytoken == Token_YIELD
                                                                   || yytoken == Token_ASSEMBLY
                                                                   || yytoken == Token_IDENTIFIER)
                                                            {
                                                              namespace_or_type_name_ast *__node_449 = 0;
                                                              if (!parse_namespace_or_type_name(&__node_449))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_namespace_or_type_name, "namespace_or_type_name");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->namespace_name = __node_449;

                                                            }
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

                                                    bool parser::parse_using_statement(using_statement_ast **yynode)
                                                    {
                                                      *yynode = create<using_statement_ast>();

                                                      (*yynode)->start_token = token_stream->index() - 1;

                                                      if (yytoken == Token_USING)
                                                        {
                                                          if (yytoken != Token_USING)
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_token(yytoken, Token_USING, "using");
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

                                                          resource_acquisition_ast *__node_450 = 0;
                                                          if (!parse_resource_acquisition(&__node_450))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_resource_acquisition, "resource_acquisition");
                                                                }
                                                              return false;
                                                            }
                                                          (*yynode)->resource_acquisition = __node_450;

                                                          if (yytoken != Token_RPAREN)
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_token(yytoken, Token_RPAREN, ")");
                                                                }
                                                              return false;
                                                            }
                                                          yylex();

                                                          embedded_statement_ast *__node_451 = 0;
                                                          if (!parse_embedded_statement(&__node_451))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_embedded_statement, "embedded_statement");
                                                                }
                                                              return false;
                                                            }
                                                          (*yynode)->body = __node_451;

                                                        }
                                                      else
                                                        {
                                                          return false;
                                                        }

                                                      (*yynode)->end_token = token_stream->index() - 1;

                                                      return true;
                                                    }

                                                    bool parser::parse_variable_declaration_data(variable_declaration_data_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers, type_ast *type, const list_node<variable_declarator_ast *> *variable_declarator_sequence)
                                                    {
                                                      *yynode = create<variable_declaration_data_ast>();

                                                      (*yynode)->start_token = token_stream->index() - 1;

                                                      (*yynode)->attributes = attributes;
                                                      if (attributes && attributes->start_token < (*yynode)->start_token)
                                                        (*yynode)->start_token = attributes->start_token;

                                                      (*yynode)->modifiers = modifiers;
                                                      if (modifiers && modifiers->start_token < (*yynode)->start_token)
                                                        (*yynode)->start_token = modifiers->start_token;

                                                      (*yynode)->type = type;
                                                      if (type && type->start_token < (*yynode)->start_token)
                                                        (*yynode)->start_token = type->start_token;

                                                      (*yynode)->variable_declarator_sequence = variable_declarator_sequence;
                                                      if (variable_declarator_sequence && variable_declarator_sequence->to_front()->element->start_token < (*yynode)->start_token)
                                                        (*yynode)->start_token = variable_declarator_sequence->to_front()->element->start_token;

                                                      if (true /*epsilon*/ || yytoken == Token_ABSTRACT
                                                          || yytoken == Token_BOOL
                                                          || yytoken == Token_BYTE
                                                          || yytoken == Token_CHAR
                                                          || yytoken == Token_CLASS
                                                          || yytoken == Token_CONST
                                                          || yytoken == Token_DECIMAL
                                                          || yytoken == Token_DELEGATE
                                                          || yytoken == Token_DOUBLE
                                                          || yytoken == Token_ENUM
                                                          || yytoken == Token_EVENT
                                                          || yytoken == Token_EXPLICIT
                                                          || yytoken == Token_EXTERN
                                                          || yytoken == Token_FIXED
                                                          || yytoken == Token_FLOAT
                                                          || yytoken == Token_IMPLICIT
                                                          || yytoken == Token_INT
                                                          || yytoken == Token_INTERFACE
                                                          || yytoken == Token_INTERNAL
                                                          || yytoken == Token_LONG
                                                          || yytoken == Token_NEW
                                                          || yytoken == Token_OBJECT
                                                          || yytoken == Token_OVERRIDE
                                                          || yytoken == Token_PRIVATE
                                                          || yytoken == Token_PROTECTED
                                                          || yytoken == Token_PUBLIC
                                                          || yytoken == Token_READONLY
                                                          || yytoken == Token_SBYTE
                                                          || yytoken == Token_SEALED
                                                          || yytoken == Token_SHORT
                                                          || yytoken == Token_STATIC
                                                          || yytoken == Token_STRING
                                                          || yytoken == Token_STRUCT
                                                          || yytoken == Token_UINT
                                                          || yytoken == Token_ULONG
                                                          || yytoken == Token_UNSAFE
                                                          || yytoken == Token_USHORT
                                                          || yytoken == Token_VIRTUAL
                                                          || yytoken == Token_VOID
                                                          || yytoken == Token_VOLATILE
                                                          || yytoken == Token_ADD
                                                          || yytoken == Token_ALIAS
                                                          || yytoken == Token_GET
                                                          || yytoken == Token_GLOBAL
                                                          || yytoken == Token_PARTIAL
                                                          || yytoken == Token_REMOVE
                                                          || yytoken == Token_SET
                                                          || yytoken == Token_VALUE
                                                          || yytoken == Token_WHERE
                                                          || yytoken == Token_YIELD
                                                          || yytoken == Token_ASSEMBLY
                                                          || yytoken == Token_RPAREN
                                                          || yytoken == Token_RBRACE
                                                          || yytoken == Token_LBRACKET
                                                          || yytoken == Token_SEMICOLON
                                                          || yytoken == Token_TILDE
                                                          || yytoken == Token_IDENTIFIER)
                                                      {}
                                                      else
                                                        {
                                                          return false;
                                                        }

                                                      (*yynode)->end_token = token_stream->index() - 1;

                                                      return true;
                                                    }

                                                    bool parser::parse_variable_declarator(variable_declarator_ast **yynode, bool fixed_size_buffer)
                                                    {
                                                      *yynode = create<variable_declarator_ast>();

                                                      (*yynode)->start_token = token_stream->index() - 1;

                                                      if (yytoken == Token_ADD
                                                          || yytoken == Token_ALIAS
                                                          || yytoken == Token_GET
                                                          || yytoken == Token_GLOBAL
                                                          || yytoken == Token_PARTIAL
                                                          || yytoken == Token_REMOVE
                                                          || yytoken == Token_SET
                                                          || yytoken == Token_VALUE
                                                          || yytoken == Token_WHERE
                                                          || yytoken == Token_YIELD
                                                          || yytoken == Token_ASSEMBLY
                                                          || yytoken == Token_IDENTIFIER)
                                                        {
                                                          identifier_ast *__node_452 = 0;
                                                          if (!parse_identifier(&__node_452))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_identifier, "identifier");
                                                                }
                                                              return false;
                                                            }
                                                          (*yynode)->variable_name = __node_452;

                                                          if ((yytoken == Token_LBRACKET) && ( fixed_size_buffer ))
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

                                                              expression_ast *__node_453 = 0;
                                                              if (!parse_expression(&__node_453))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_expression, "expression");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->array_size = __node_453;

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
                                                          else if ((yytoken == Token_ASSIGN) && ( !fixed_size_buffer ))
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

                                                              variable_initializer_ast *__node_454 = 0;
                                                              if (!parse_variable_initializer(&__node_454))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_variable_initializer, "variable_initializer");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->variable_initializer = __node_454;

                                                            }
                                                          else if ((true /*epsilon*/) && ( !fixed_size_buffer ))
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

                                                      if (yytoken == Token_BASE
                                                          || yytoken == Token_BOOL
                                                          || yytoken == Token_BYTE
                                                          || yytoken == Token_CHAR
                                                          || yytoken == Token_CHECKED
                                                          || yytoken == Token_DECIMAL
                                                          || yytoken == Token_DEFAULT
                                                          || yytoken == Token_DELEGATE
                                                          || yytoken == Token_DOUBLE
                                                          || yytoken == Token_FLOAT
                                                          || yytoken == Token_INT
                                                          || yytoken == Token_LONG
                                                          || yytoken == Token_NEW
                                                          || yytoken == Token_OBJECT
                                                          || yytoken == Token_SBYTE
                                                          || yytoken == Token_SHORT
                                                          || yytoken == Token_SIZEOF
                                                          || yytoken == Token_STACKALLOC
                                                          || yytoken == Token_STRING
                                                          || yytoken == Token_THIS
                                                          || yytoken == Token_TYPEOF
                                                          || yytoken == Token_UINT
                                                          || yytoken == Token_ULONG
                                                          || yytoken == Token_UNCHECKED
                                                          || yytoken == Token_USHORT
                                                          || yytoken == Token_ADD
                                                          || yytoken == Token_ALIAS
                                                          || yytoken == Token_GET
                                                          || yytoken == Token_GLOBAL
                                                          || yytoken == Token_PARTIAL
                                                          || yytoken == Token_REMOVE
                                                          || yytoken == Token_SET
                                                          || yytoken == Token_VALUE
                                                          || yytoken == Token_WHERE
                                                          || yytoken == Token_YIELD
                                                          || yytoken == Token_ASSEMBLY
                                                          || yytoken == Token_LPAREN
                                                          || yytoken == Token_LBRACE
                                                          || yytoken == Token_BANG
                                                          || yytoken == Token_TILDE
                                                          || yytoken == Token_INCREMENT
                                                          || yytoken == Token_DECREMENT
                                                          || yytoken == Token_PLUS
                                                          || yytoken == Token_MINUS
                                                          || yytoken == Token_STAR
                                                          || yytoken == Token_BIT_AND
                                                          || yytoken == Token_TRUE
                                                          || yytoken == Token_FALSE
                                                          || yytoken == Token_NULL
                                                          || yytoken == Token_INTEGER_LITERAL
                                                          || yytoken == Token_REAL_LITERAL
                                                          || yytoken == Token_CHARACTER_LITERAL
                                                          || yytoken == Token_STRING_LITERAL
                                                          || yytoken == Token_IDENTIFIER)
                                                        {
                                                          if (yytoken == Token_BASE
                                                              || yytoken == Token_BOOL
                                                              || yytoken == Token_BYTE
                                                              || yytoken == Token_CHAR
                                                              || yytoken == Token_CHECKED
                                                              || yytoken == Token_DECIMAL
                                                              || yytoken == Token_DEFAULT
                                                              || yytoken == Token_DELEGATE
                                                              || yytoken == Token_DOUBLE
                                                              || yytoken == Token_FLOAT
                                                              || yytoken == Token_INT
                                                              || yytoken == Token_LONG
                                                              || yytoken == Token_NEW
                                                              || yytoken == Token_OBJECT
                                                              || yytoken == Token_SBYTE
                                                              || yytoken == Token_SHORT
                                                              || yytoken == Token_SIZEOF
                                                              || yytoken == Token_STRING
                                                              || yytoken == Token_THIS
                                                              || yytoken == Token_TYPEOF
                                                              || yytoken == Token_UINT
                                                              || yytoken == Token_ULONG
                                                              || yytoken == Token_UNCHECKED
                                                              || yytoken == Token_USHORT
                                                              || yytoken == Token_ADD
                                                              || yytoken == Token_ALIAS
                                                              || yytoken == Token_GET
                                                              || yytoken == Token_GLOBAL
                                                              || yytoken == Token_PARTIAL
                                                              || yytoken == Token_REMOVE
                                                              || yytoken == Token_SET
                                                              || yytoken == Token_VALUE
                                                              || yytoken == Token_WHERE
                                                              || yytoken == Token_YIELD
                                                              || yytoken == Token_ASSEMBLY
                                                              || yytoken == Token_LPAREN
                                                              || yytoken == Token_BANG
                                                              || yytoken == Token_TILDE
                                                              || yytoken == Token_INCREMENT
                                                              || yytoken == Token_DECREMENT
                                                              || yytoken == Token_PLUS
                                                              || yytoken == Token_MINUS
                                                              || yytoken == Token_STAR
                                                              || yytoken == Token_BIT_AND
                                                              || yytoken == Token_TRUE
                                                              || yytoken == Token_FALSE
                                                              || yytoken == Token_NULL
                                                              || yytoken == Token_INTEGER_LITERAL
                                                              || yytoken == Token_REAL_LITERAL
                                                              || yytoken == Token_CHARACTER_LITERAL
                                                              || yytoken == Token_STRING_LITERAL
                                                              || yytoken == Token_IDENTIFIER)
                                                            {
                                                              expression_ast *__node_455 = 0;
                                                              if (!parse_expression(&__node_455))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_expression, "expression");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->expression = __node_455;

                                                            }
                                                          else if (yytoken == Token_LBRACE)
                                                            {
                                                              array_initializer_ast *__node_456 = 0;
                                                              if (!parse_array_initializer(&__node_456))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_array_initializer, "array_initializer");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->array_initializer = __node_456;

                                                            }
                                                          else if (yytoken == Token_STACKALLOC)
                                                            {
                                                              stackalloc_initializer_ast *__node_457 = 0;
                                                              if (!parse_stackalloc_initializer(&__node_457))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_stackalloc_initializer, "stackalloc_initializer");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->stackalloc_initializer = __node_457;

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

                                                          boolean_expression_ast *__node_458 = 0;
                                                          if (!parse_boolean_expression(&__node_458))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_boolean_expression, "boolean_expression");
                                                                }
                                                              return false;
                                                            }
                                                          (*yynode)->condition = __node_458;

                                                          if (yytoken != Token_RPAREN)
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_token(yytoken, Token_RPAREN, ")");
                                                                }
                                                              return false;
                                                            }
                                                          yylex();

                                                          embedded_statement_ast *__node_459 = 0;
                                                          if (!parse_embedded_statement(&__node_459))
                                                            {
                                                              if (!yy_block_errors)
                                                                {
                                                                  yy_expected_symbol(ast_node::Kind_embedded_statement, "embedded_statement");
                                                                }
                                                              return false;
                                                            }
                                                          (*yynode)->body = __node_459;

                                                        }
                                                      else
                                                        {
                                                          return false;
                                                        }

                                                      (*yynode)->end_token = token_stream->index() - 1;

                                                      return true;
                                                    }

                                                    bool parser::parse_yield_statement(yield_statement_ast **yynode)
                                                    {
                                                      *yynode = create<yield_statement_ast>();

                                                      (*yynode)->start_token = token_stream->index() - 1;

                                                      if (yytoken == Token_YIELD)
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

                                                              expression_ast *__node_460 = 0;
                                                              if (!parse_expression(&__node_460))
                                                                {
                                                                  if (!yy_block_errors)
                                                                    {
                                                                      yy_expected_symbol(ast_node::Kind_expression, "expression");
                                                                    }
                                                                  return false;
                                                                }
                                                              (*yynode)->return_expression = __node_460;

                                                              (*yynode)->yield_type = yield_statement::type_yield_return;
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

                                                              (*yynode)->yield_type = yield_statement::type_yield_break;
                                                            }
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

                                                    visitor::parser_fun_t visitor::_S_parser_table[] = {
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_accessor_declarations),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_accessor_modifier),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_additive_expression),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_additive_expression_rest),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_anonymous_method_expression),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_anonymous_method_parameter),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_anonymous_method_signature),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_argument),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_array_creation_expression_rest),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_array_initializer),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_array_type),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_attribute),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_attribute_arguments),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_attribute_section),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_attribute_target),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_base_access),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_bit_and_expression),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_bit_or_expression),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_bit_xor_expression),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_block),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_block_statement),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_boolean_expression),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_break_statement),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_builtin_class_type),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_cast_expression),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_catch_clauses),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_checked_statement),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_class_base),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_class_body),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_class_declaration),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_class_member_declaration),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_class_or_struct_member_declaration),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_class_type),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_compilation_unit),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_conditional_expression),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_constant_declaration),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_constant_declaration_data),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_constant_declarator),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_constant_expression),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_constructor_constraint),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_constructor_declaration),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_constructor_initializer),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_continue_statement),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_conversion_operator_declaration),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_delegate_declaration),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_do_while_statement),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_embedded_statement),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_enum_base),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_enum_body),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_enum_declaration),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_enum_member_declaration),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_equality_expression),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_equality_expression_rest),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_event_accessor_declarations),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_event_declaration),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_expression),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_extern_alias_directive),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_finalizer_declaration),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_fixed_pointer_declarator),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_fixed_statement),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_floating_point_type),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_for_control),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_for_statement),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_foreach_statement),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_formal_parameter),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_formal_parameter_list),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_general_catch_clause),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_generic_dimension_specifier),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_global_attribute_section),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_goto_statement),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_identifier),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_if_statement),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_indexer_declaration),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_integral_type),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_interface_accessors),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_interface_base),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_interface_body),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_interface_declaration),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_interface_event_declaration),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_interface_indexer_declaration),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_interface_member_declaration),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_interface_method_declaration),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_interface_property_declaration),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_keyword),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_labeled_statement),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_literal),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_local_constant_declaration),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_local_variable_declaration),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_local_variable_declaration_statement),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_lock_statement),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_logical_and_expression),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_logical_or_expression),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_managed_type),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_method_declaration),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_multiplicative_expression),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_multiplicative_expression_rest),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_named_argument),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_namespace_body),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_namespace_declaration),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_namespace_member_declaration),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_namespace_name),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_namespace_or_type_name),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_namespace_or_type_name_part),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_namespace_or_type_name_safe),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_new_expression),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_non_array_type),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_non_nullable_type),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_null_coalescing_expression),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_numeric_type),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_object_or_delegate_creation_expression_rest),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_optional_argument_list),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_optional_attribute_sections),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_optional_modifiers),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_optionally_nullable_type),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_overloadable_binary_only_operator),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_overloadable_unary_only_operator),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_overloadable_unary_or_binary_operator),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_parameter_array),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_parameter_modifier),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_pointer_type),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_positional_argument),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_predefined_type),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_primary_atom),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_primary_expression),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_primary_or_secondary_constraint),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_primary_suffix),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_property_declaration),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_qualified_identifier),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_rank_specifier),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_relational_expression),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_relational_expression_rest),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_resource_acquisition),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_return_statement),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_return_type),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_secondary_constraints),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_shift_expression),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_shift_expression_rest),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_simple_name_or_member_access),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_simple_type),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_specific_catch_clause),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_stackalloc_initializer),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_statement_expression),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_struct_body),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_struct_declaration),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_struct_interfaces),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_struct_member_declaration),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_switch_label),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_switch_section),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_switch_statement),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_throw_statement),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_try_statement),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_type),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_type_arguments),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_type_arguments_or_parameters_end),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_type_declaration),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_type_declaration_rest),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_type_name),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_type_name_safe),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_type_parameter),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_type_parameter_constraints),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_type_parameter_constraints_clause),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_type_parameters),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_typeof_expression),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_unary_expression),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_unary_or_binary_operator_declaration),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_unbound_type_name),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_unbound_type_name_part),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_unchecked_statement),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_unmanaged_type),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_unmanaged_type_suffix),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_unsafe_statement),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_using_directive),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_using_statement),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_variable_declaration_data),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_variable_declarator),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_variable_initializer),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_while_statement),
                                                          reinterpret_cast<parser_fun_t>(&visitor::visit_yield_statement)
                                                        }; // _S_parser_table[]

} // end of namespace csharp


