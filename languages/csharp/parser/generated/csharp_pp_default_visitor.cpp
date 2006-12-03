// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#include "csharp_pp_default_visitor.h"

namespace csharp_pp
  {

  void default_visitor::visit_pp_and_expression(pp_and_expression_ast *node)
  {
    if (node->expression_sequence)
      {
        const list_node<pp_equality_expression_ast*> *__it = node->expression_sequence->to_front(), *__end = __it;
        do
          {
            visit_node(__it->element);
            __it = __it->next;
          }
        while (__it != __end);
      }
  }

  void default_visitor::visit_pp_declaration(pp_declaration_ast *)
{}

  void default_visitor::visit_pp_diagnostic(pp_diagnostic_ast *)
  {}

  void default_visitor::visit_pp_directive(pp_directive_ast *node)
  {
    visit_node(node->declaration);
    visit_node(node->if_clause);
    visit_node(node->elif_clause);
    visit_node(node->else_clause);
    visit_node(node->endif_clause);
    visit_node(node->diagnostic);
    visit_node(node->region);
    visit_node(node->line);
    visit_node(node->pragma);
  }

  void default_visitor::visit_pp_elif_clause(pp_elif_clause_ast *node)
  {
    visit_node(node->expression);
  }

  void default_visitor::visit_pp_else_clause(pp_else_clause_ast *)
  {}

  void default_visitor::visit_pp_endif_clause(pp_endif_clause_ast *)
  {}

  void default_visitor::visit_pp_equality_expression(pp_equality_expression_ast *node)
  {
    visit_node(node->expression);
    if (node->additional_expression_sequence)
      {
        const list_node<pp_equality_expression_rest_ast*> *__it = node->additional_expression_sequence->to_front(), *__end = __it;
        do
          {
            visit_node(__it->element);
            __it = __it->next;
          }
        while (__it != __end);
      }
  }

  void default_visitor::visit_pp_equality_expression_rest(pp_equality_expression_rest_ast *node)
  {
    visit_node(node->expression);
  }

  void default_visitor::visit_pp_expression(pp_expression_ast *node)
  {
    if (node->expression_sequence)
      {
        const list_node<pp_and_expression_ast*> *__it = node->expression_sequence->to_front(), *__end = __it;
        do
          {
            visit_node(__it->element);
            __it = __it->next;
          }
        while (__it != __end);
      }
  }

  void default_visitor::visit_pp_if_clause(pp_if_clause_ast *node)
  {
    visit_node(node->expression);
  }

  void default_visitor::visit_pp_line(pp_line_ast *)
  {}

  void default_visitor::visit_pp_pragma(pp_pragma_ast *)
  {}

  void default_visitor::visit_pp_primary_expression(pp_primary_expression_ast *node)
  {
    visit_node(node->parenthesis_expression);
  }

  void default_visitor::visit_pp_region(pp_region_ast *)
  {}

  void default_visitor::visit_pp_unary_expression(pp_unary_expression_ast *node)
  {
    visit_node(node->expression);
  }


} // end of namespace csharp_pp


