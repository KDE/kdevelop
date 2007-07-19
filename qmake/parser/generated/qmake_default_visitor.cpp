// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#include "qmake_default_visitor.h"

namespace QMake
  {

  void default_visitor::visit_arg_list(arg_list_ast *node)
  {
    if  (node->args_sequence)
      {
        const list_node<argument_ast*> *__it =  node->args_sequence->to_front(),  *__end =  __it;

        do
          {
            visit_node(__it->element);
            __it =  __it->next;
          }

        while  (__it !=  __end);
      }
  }

  void default_visitor::visit_argument(argument_ast *node)
  {
    visit_node(node->value_str);
    visit_node(node->quoted_val);
    visit_node(node->ref);
  }

  void default_visitor::visit_func_var_ref(func_var_ref_ast *node)
  {
    visit_node(node->args);
  }

  void default_visitor::visit_function_args(function_args_ast *node)
  {
    visit_node(node->args);
  }

  void default_visitor::visit_function_scope(function_scope_ast *node)
  {
    visit_node(node->args);
    visit_node(node->scopebody);
  }

  void default_visitor::visit_id_or_value(id_or_value_ast *)
  {}

  void default_visitor::visit_op(op_ast *)
  {}

  void default_visitor::visit_project(project_ast *node)
  {
    if  (node->stmts_sequence)
      {
        const list_node<stmt_ast*> *__it =  node->stmts_sequence->to_front(),  *__end =  __it;

        do
          {
            visit_node(__it->element);
            __it =  __it->next;
          }

        while  (__it !=  __end);
      }
  }

  void default_visitor::visit_quote_value(quote_value_ast *node)
  {
    visit_node(node->value_str);
    visit_node(node->ref);
  }

  void default_visitor::visit_quoted_value(quoted_value_ast *node)
  {
    visit_node(node->value);
  }

  void default_visitor::visit_ref(ref_ast *node)
  {
    visit_node(node->func_var_ref);
    visit_node(node->varref);
  }

  void default_visitor::visit_scope_body(scope_body_ast *node)
  {
    if  (node->stmts_sequence)
      {
        const list_node<stmt_ast*> *__it =  node->stmts_sequence->to_front(),  *__end =  __it;

        do
          {
            visit_node(__it->element);
            __it =  __it->next;
          }

        while  (__it !=  __end);
      }
  }

  void default_visitor::visit_stmt(stmt_ast *node)
  {
    visit_node(node->var);
    visit_node(node->func);
    visit_node(node->scope);
  }

  void default_visitor::visit_value(value_ast *node)
  {
    visit_node(node->value_str);
    visit_node(node->quote_val);
    visit_node(node->ref);
  }

  void default_visitor::visit_value_list(value_list_ast *node)
  {
    if  (node->list_sequence)
      {
        const list_node<value_ast*> *__it =  node->list_sequence->to_front(),  *__end =  __it;

        do
          {
            visit_node(__it->element);
            __it =  __it->next;
          }

        while  (__it !=  __end);
      }
  }

  void default_visitor::visit_variable_assignment(variable_assignment_ast *node)
  {
    visit_node(node->op);
    visit_node(node->values);
  }

  void default_visitor::visit_varref(varref_ast *)
  {}


} // end of namespace QMake


