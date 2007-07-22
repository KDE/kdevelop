// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#include "qmake_default_visitor.h"

namespace QMake
  {

  void default_visitor::visit_arg_list(arg_list_ast *node)
  {
    if  (node->args_sequence)
      {
        const list_node<value_ast*> *__it =  node->args_sequence->to_front(),  *__end =  __it;

        do
          {
            visit_node(__it->element);
            __it =  __it->next;
          }

        while  (__it !=  __end);
      }
  }

  void default_visitor::visit_function_args(function_args_ast *node)
  {
    visit_node(node->args);
  }

  void default_visitor::visit_item(item_ast *node)
  {
    visit_node(node->func_args);
  }

  void default_visitor::visit_op(op_ast *)
  {}

  void default_visitor::visit_or_op(or_op_ast *node)
  {
    if  (node->item_sequence)
      {
        const list_node<item_ast*> *__it =  node->item_sequence->to_front(),  *__end =  __it;

        do
          {
            visit_node(__it->element);
            __it =  __it->next;
          }

        while  (__it !=  __end);
      }
  }

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

  void default_visitor::visit_scope(scope_ast *node)
  {
    visit_node(node->func_args);
    visit_node(node->scope_body);
    visit_node(node->or_op);
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
    visit_node(node->scope);
  }

  void default_visitor::visit_value(value_ast *)
  {}

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


} // end of namespace QMake


