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

  struct funcref_ast;

  struct function_args_ast;

  struct function_scope_ast;

  struct id_or_value_ast;

  struct op_ast;

  struct project_ast;

  struct quoted_value_ast;

  struct ref_ast;

  struct scope_body_ast;

  struct stmt_ast;

  struct value_list_ast;

  struct variable_assignment_ast;

  struct varref_ast;


  struct ast_node
    {
      enum ast_node_kind_enum {
        Kind_arg_list =  1000,
        Kind_funcref =  1001,
        Kind_function_args =  1002,
        Kind_function_scope =  1003,
        Kind_id_or_value =  1004,
        Kind_op =  1005,
        Kind_project =  1006,
        Kind_quoted_value =  1007,
        Kind_ref =  1008,
        Kind_scope_body =  1009,
        Kind_stmt =  1010,
        Kind_value_list =  1011,
        Kind_variable_assignment =  1012,
        Kind_varref =  1013,
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

    };

  struct funcref_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_funcref
      };

    };

  struct function_args_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_function_args
      };

    };

  struct function_scope_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_function_scope
      };

    };

  struct id_or_value_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_id_or_value
      };

    };

  struct op_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_op
      };

    };

  struct project_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_project
      };

    };

  struct quoted_value_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_quoted_value
      };

    };

  struct ref_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_ref
      };

    };

  struct scope_body_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_scope_body
      };

    };

  struct stmt_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_stmt
      };

    };

  struct value_list_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_value_list
      };

    };

  struct variable_assignment_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_variable_assignment
      };

    };

  struct varref_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_varref
      };

    };



} // end of namespace QMake

#endif


