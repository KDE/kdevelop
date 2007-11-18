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

  struct function_args_ast;

  struct item_ast;

  struct op_ast;

  struct or_op_ast;

  struct project_ast;

  struct scope_ast;

  struct scope_body_ast;

  struct stmt_ast;

  struct value_ast;

  struct value_list_ast;

  struct variable_assignment_ast;


  struct  ast_node
    {
      enum ast_node_kind_enum {
        Kind_arg_list =  1000,
        Kind_function_args =  1001,
        Kind_item =  1002,
        Kind_op =  1003,
        Kind_or_op =  1004,
        Kind_project =  1005,
        Kind_scope =  1006,
        Kind_scope_body =  1007,
        Kind_stmt =  1008,
        Kind_value =  1009,
        Kind_value_list =  1010,
        Kind_variable_assignment =  1011,
        AST_NODE_KIND_COUNT
      };

      int kind;
      std::size_t start_token;
      std::size_t end_token;
    };

  struct  arg_list_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_arg_list
      };

      const list_node<value_ast *> *args_sequence;
    };

  struct  function_args_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_function_args
      };

      arg_list_ast *args;
    };

  struct  item_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_item
      };

      std::size_t id;
      function_args_ast *func_args;
    };

  struct  op_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_op
      };

      std::size_t optoken;
    };

  struct  or_op_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_or_op
      };

      const list_node<item_ast *> *item_sequence;
    };

  struct  project_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_project
      };

      const list_node<stmt_ast *> *stmts_sequence;
    };

  struct  scope_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_scope
      };

      function_args_ast *func_args;
      scope_body_ast *scope_body;
      or_op_ast *or_op;
    };

  struct  scope_body_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_scope_body
      };

      const list_node<stmt_ast *> *stmts_sequence;
    };

  struct  stmt_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_stmt
      };

      bool isNewline;
      bool isExclam;
      std::size_t id;
      variable_assignment_ast *var;
      scope_ast *scope;
    };

  struct  value_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_value
      };

      std::size_t value;
    };

  struct  value_list_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_value_list
      };

      const list_node<value_ast *> *list_sequence;
    };

  struct  variable_assignment_ast:  public ast_node
    {
      enum
      {
        KIND =  Kind_variable_assignment
      };

      op_ast *op;
      value_list_ast *values;
    };



} // end of namespace QMake

#endif


