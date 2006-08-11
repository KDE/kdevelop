// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#ifndef csharp_pp_AST_H_INCLUDED
#define csharp_pp_AST_H_INCLUDED

#include "kdev-pg-list.h"

#include <kdevast.h>


#include "csharp_pp_scope.h"

namespace csharp
  {
  class parser;
  class Lexer;
}

namespace csharp_pp
  {

  struct pp_and_expression_ast;
  struct pp_declaration_ast;
  struct pp_diagnostic_ast;
  struct pp_directive_ast;
  struct pp_elif_clause_ast;
  struct pp_else_clause_ast;
  struct pp_endif_clause_ast;
  struct pp_equality_expression_ast;
  struct pp_equality_expression_rest_ast;
  struct pp_expression_ast;
  struct pp_if_clause_ast;
  struct pp_line_ast;
  struct pp_pragma_ast;
  struct pp_primary_expression_ast;
  struct pp_region_ast;
  struct pp_unary_expression_ast;

  namespace pp_declaration
    {
    enum pp_declaration_type_enum {
      type_define,
      type_undef
    };
  }

  namespace pp_diagnostic
    {
    enum pp_diagnostic_type_enum {
      type_error,
      type_warning
    };
  }

  namespace pp_equality_expression_rest
    {
    enum pp_equality_operator_enum {
      op_equal,
      op_not_equal
    };
  }

  namespace pp_primary_expression
    {
    enum pp_primary_expression_type_enum {
      type_true,
      type_false,
      type_conditional_symbol,
      type_parenthesis_expression
    };
  }

  namespace pp_region
    {
    enum pp_region_type_enum {
      type_region,
      type_endregion
    };
  }


  struct ast_node: public KDevAST
    {
      enum ast_node_kind_enum {
        Kind_pp_and_expression = 1000,
        Kind_pp_declaration = 1001,
        Kind_pp_diagnostic = 1002,
        Kind_pp_directive = 1003,
        Kind_pp_elif_clause = 1004,
        Kind_pp_else_clause = 1005,
        Kind_pp_endif_clause = 1006,
        Kind_pp_equality_expression = 1007,
        Kind_pp_equality_expression_rest = 1008,
        Kind_pp_expression = 1009,
        Kind_pp_if_clause = 1010,
        Kind_pp_line = 1011,
        Kind_pp_pragma = 1012,
        Kind_pp_primary_expression = 1013,
        Kind_pp_region = 1014,
        Kind_pp_unary_expression = 1015,
        AST_NODE_KIND_COUNT
      };

      int kind;
      std::size_t start_token;
      std::size_t end_token;
    };

  struct pp_and_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_pp_and_expression
      };

      const list_node<pp_equality_expression_ast *> *expression_sequence;
    };

  struct pp_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_pp_declaration
      };

      pp_declaration::pp_declaration_type_enum type;
      std::size_t conditional_symbol;
    };

  struct pp_diagnostic_ast: public ast_node
    {
      enum
      {
        KIND = Kind_pp_diagnostic
      };

      pp_diagnostic::pp_diagnostic_type_enum type;
      std::size_t message;
    };

  struct pp_directive_ast: public ast_node
    {
      enum
      {
        KIND = Kind_pp_directive
      };

      pp_declaration_ast *declaration;
      pp_if_clause_ast *if_clause;
      pp_elif_clause_ast *elif_clause;
      pp_else_clause_ast *else_clause;
      pp_endif_clause_ast *endif_clause;
      pp_diagnostic_ast *diagnostic;
      pp_region_ast *region;
      pp_line_ast *line;
      pp_pragma_ast *pragma;
    };

  struct pp_elif_clause_ast: public ast_node
    {
      enum
      {
        KIND = Kind_pp_elif_clause
      };

      pp_expression_ast *expression;
    };

  struct pp_else_clause_ast: public ast_node
    {
      enum
      {
        KIND = Kind_pp_else_clause
      };

    };

  struct pp_endif_clause_ast: public ast_node
    {
      enum
      {
        KIND = Kind_pp_endif_clause
      };

    };

  struct pp_equality_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_pp_equality_expression
      };

      pp_unary_expression_ast *expression;
      const list_node<pp_equality_expression_rest_ast *> *additional_expression_sequence;
    };

  struct pp_equality_expression_rest_ast: public ast_node
    {
      enum
      {
        KIND = Kind_pp_equality_expression_rest
      };

      pp_equality_expression_rest::pp_equality_operator_enum equality_operator;
      pp_unary_expression_ast *expression;
    };

  struct pp_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_pp_expression
      };

      const list_node<pp_and_expression_ast *> *expression_sequence;
    };

  struct pp_if_clause_ast: public ast_node
    {
      enum
      {
        KIND = Kind_pp_if_clause
      };

      pp_expression_ast *expression;
    };

  struct pp_line_ast: public ast_node
    {
      enum
      {
        KIND = Kind_pp_line
      };

      std::size_t line_number;
      std::size_t file_name;
      std::size_t token_default;
      std::size_t identifier_or_keyword;
    };

  struct pp_pragma_ast: public ast_node
    {
      enum
      {
        KIND = Kind_pp_pragma
      };

      std::size_t pragma_text;
    };

  struct pp_primary_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_pp_primary_expression
      };

      pp_primary_expression::pp_primary_expression_type_enum type;
      std::size_t conditional_symbol;
      pp_expression_ast *parenthesis_expression;
    };

  struct pp_region_ast: public ast_node
    {
      enum
      {
        KIND = Kind_pp_region
      };

      pp_region::pp_region_type_enum type;
      std::size_t label;
    };

  struct pp_unary_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_pp_unary_expression
      };

      bool negated;
      pp_primary_expression_ast *expression;
    };



} // end of namespace csharp_pp

#endif


