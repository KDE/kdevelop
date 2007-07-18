// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#ifndef qmake_AST_H_INCLUDED
#define qmake_AST_H_INCLUDED

#include <kdev-pg-list.h>



#include <QtCore/QString>

namespace QMake
  {

  class Lexer;
}


namespace QMake
  {

  struct arg_list_ast;

  struct argument_ast;

  struct funcref_ast;

  struct function_args_ast;

  struct function_scope_ast;

  struct id_or_value_ast;

  struct op_ast;

  struct project_ast;

  struct quote_value_ast;

  struct quoted_value_ast;

  struct ref_ast;

  struct scope_body_ast;

  struct stmt_ast;

  struct value_ast;

  struct value_list_ast;

  struct variable_assignment_ast;

  struct varref_ast;


  struct ast_node
    {
      enum ast_node_kind_enum {
        Kind_arg_list =  1000,
        Kind_argument =  1001,
        Kind_funcref =  1002,
        Kind_function_args =  1003,
        Kind_function_scope =  1004,
        Kind_id_or_value =  1005,
        Kind_op =  1006,
        Kind_project =  1007,
        Kind_quote_value =  1008,
        Kind_quoted_value =  1009,
        Kind_ref =  1010,
        Kind_scope_body =  1011,
        Kind_stmt =  1012,
        Kind_value =  1013,
        Kind_value_list =  1014,
        Kind_variable_assignment =  1015,
        Kind_varref =  1016,
        AST_NODE_KIND_COUNT
      };

      int kind;
      std::size_t start_token;
      std::size_t end_token;
    };

  struct arg_list_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_arg_list
      };

      const list_node<argument_ast *> *args_sequence;
    };

  struct argument_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_argument
      };

      id_or_value_ast *value_str;
      quoted_value_ast *quoted_val;
      ref_ast *ref;
    };

  struct funcref_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_funcref
      };

      std::size_t id;
      function_args_ast *args;
    };

  struct function_args_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_function_args
      };

      arg_list_ast *args;
    };

  struct function_scope_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_function_scope
      };

      function_args_ast *args;
      scope_body_ast *scopebody;
    };

  struct id_or_value_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_id_or_value
      };

      std::size_t val;
    };

  struct op_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_op
      };

      std::size_t optoken;
    };

  struct project_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_project
      };

      const list_node<stmt_ast *> *stmts_sequence;
    };

  struct quote_value_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_quote_value
      };

      id_or_value_ast *value_str;
      ref_ast *ref;
      std::size_t token;
    };

  struct quoted_value_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_quoted_value
      };

      quote_value_ast *value;
    };

  struct ref_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_ref
      };

      varref_ast *varref;
      funcref_ast *funcref;
      std::size_t idref;
    };

  struct scope_body_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_scope_body
      };

      const list_node<stmt_ast *> *stmts_sequence;
    };

  struct stmt_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_stmt
      };

      std::size_t id;
      variable_assignment_ast *var;
      function_scope_ast *func;
      scope_body_ast *scope;
    };

  struct value_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_value
      };

      id_or_value_ast *value_str;
      quoted_value_ast *quote_val;
      ref_ast *ref;
    };

  struct value_list_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_value_list
      };

      const list_node<value_ast *> *list_sequence;
    };

  struct variable_assignment_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_variable_assignment
      };

      op_ast *op;
      value_list_ast *values;
    };

  struct varref_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_varref
      };

      std::size_t id;
    };



} // end of namespace QMake

#endif


