/***************************************************************************
 *   Copyright (C) 2006 by Jakob Petsovits                                 *
 *   jpetso@gmx.at                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "csharp_pp_handler_visitor.h"
#include "decoder.h"

#include <string>


namespace csharp_pp
{

handler_visitor::handler_visitor(parser* pp_parser)
 : default_visitor(), _M_pp_parser(pp_parser), _M_scope(0)
{
}

handler_visitor::~handler_visitor()
{
}

void handler_visitor::set_scope(scope* current_scope)
{
  _M_scope = current_scope;
}



void handler_visitor::visit_pp_declaration(pp_declaration_ast* node)
{
  if (_M_scope == 0 || _M_pp_parser == 0)
    return;
  else if (_M_scope->is_active() == false)
    return; // don't do anything if the current section is #if'd out.

  decoder d(_M_pp_parser->token_stream);
  std::string symbol_name = d.decode_id(node->conditional_symbol);

  if (node->type == pp_declaration::type_define)
    _M_scope->csharp_parser()->pp_define_symbol(symbol_name);
  else if (node->type == pp_declaration::type_undef)
    _M_scope->csharp_parser()->pp_undefine_symbol(symbol_name);
}

void handler_visitor::visit_pp_if_clause(pp_if_clause_ast* node)
{
  if (_M_scope == 0 || _M_pp_parser == 0)
    return;

  _M_scope->push_scope(scope::type_if, &_M_scope);

  if (_M_scope->is_active() == false)
    return;
  else if (_M_scope->is_waiting_for_active_section())
    {
      visit_node(node->expression);
      _M_scope->set_activated(_M_expression_bool);
    }
  else
    _M_scope->set_activated(false);
}

void handler_visitor::visit_pp_elif_clause(pp_elif_clause_ast* node)
{
  if (_M_scope == 0 || _M_pp_parser == 0)
    return;

  if (_M_scope->type() != scope::type_if)
    {
      _M_scope->csharp_parser()->report_problem( ::csharp::parser::error,
        "#elif is unexpected here");
    }

  if (_M_scope->is_waiting_for_active_section())
    {
      visit_node(node->expression);
      _M_scope->set_activated(_M_expression_bool);
    }
  else
    _M_scope->set_activated(false);
}

void handler_visitor::visit_pp_else_clause(pp_else_clause_ast* node)
{
  if (_M_scope == 0 || _M_pp_parser == 0)
    return;

  if (_M_scope->type() != scope::type_if)
    {
      _M_scope->csharp_parser()->report_problem( ::csharp::parser::error,
        "#else is unexpected here");
    }

  if (_M_scope->is_waiting_for_active_section())
    _M_scope->set_activated(true);
  else
    _M_scope->set_activated(false);
}

void handler_visitor::visit_pp_endif_clause(pp_endif_clause_ast* node)
{
  if (_M_scope == 0 || _M_pp_parser == 0)
    return;

  bool successful = _M_scope->pop_scope( scope::type_if, &_M_scope );
  if (!successful && _M_scope->csharp_parser() != 0)
    {
      _M_scope->csharp_parser()->report_problem( ::csharp::parser::error,
        "#endif is unexpected here");
    }
}

void handler_visitor::visit_pp_diagnostic(pp_diagnostic_ast* node)
{
  if (_M_scope == 0 || _M_pp_parser == 0)
    return;

  if (!_M_scope->csharp_parser() != 0)
    {
      ::csharp::parser::problem_type diagnostic_type;
      if (node->type == pp_diagnostic::type_error)
        diagnostic_type = ::csharp::parser::error;
      else if (node->type == pp_diagnostic::type_warning)
        diagnostic_type = ::csharp::parser::warning;

      if (node->message)
        {
          decoder d(_M_pp_parser->token_stream);
          std::string message = d.decode_id(node->message);
          _M_scope->csharp_parser()->pp_diagnostic( diagnostic_type, message );
        }
      else
          _M_scope->csharp_parser()->pp_diagnostic( diagnostic_type );
    }
}

void handler_visitor::visit_pp_region(pp_region_ast* node)
{
  if (_M_scope == 0 || _M_pp_parser == 0)
    return;

  if (node->type == pp_region::type_region)
    {
      _M_scope->push_scope( scope::type_region, &_M_scope );
    }
  if (node->type == pp_region::type_endregion)
    {
      bool successful = _M_scope->pop_scope( scope::type_region, &_M_scope );
      if (!successful && _M_scope->csharp_parser() != 0)
        {
          _M_scope->csharp_parser()->report_problem( ::csharp::parser::error,
            "#endregion is unexpected here");
        }
    }
}

void handler_visitor::visit_pp_line(pp_line_ast* node)
{
  default_visitor::visit_pp_line(node);
}

void handler_visitor::visit_pp_pragma(pp_pragma_ast* node)
{
  default_visitor::visit_pp_pragma(node);
}


void handler_visitor::visit_pp_expression(pp_expression_ast* node)
{
  bool result = false;
  const list_node<pp_and_expression_ast*> *__it =
    node->expression_sequence->to_front(), *__end = __it;

  do
    {
      visit_node(__it->element);
      __it = __it->next;

      result = result || _M_expression_bool;
      // pp_expression is actually pp_or_expression
    }
  while (__it != __end && result == false);

  _M_expression_bool = result;
}

void handler_visitor::visit_pp_and_expression(pp_and_expression_ast* node)
{
  bool result = true;
  const list_node<pp_equality_expression_ast*> *__it =
    node->expression_sequence->to_front(), *__end = __it;

  do
    {
      visit_node(__it->element);
      __it = __it->next;

      result = result && _M_expression_bool;
    }
  while (__it != __end && result == true);

  _M_expression_bool = result;
}

void handler_visitor::visit_pp_equality_expression(pp_equality_expression_ast* node)
{
  default_visitor::visit_pp_equality_expression(node);
  // the child visit_*() methods assign the right value to _M_expression_bool
}

void handler_visitor::visit_pp_equality_expression_rest(pp_equality_expression_rest_ast* node)
{
  bool previous = _M_expression_bool;
  visit_node(node->expression);
  if (node->equality_operator == pp_equality_expression_rest::op_equal)
    _M_expression_bool = (previous == _M_expression_bool);
  else // op_not_equal
    _M_expression_bool = (previous != _M_expression_bool);
}

void handler_visitor::visit_pp_unary_expression(pp_unary_expression_ast* node)
{
  default_visitor::visit_pp_unary_expression(node);

  if (node->negated == true)
    _M_expression_bool = !_M_expression_bool;
}

void handler_visitor::visit_pp_primary_expression(pp_primary_expression_ast* node)
{
  switch (node->type)
  {
  case pp_primary_expression::type_true:
    _M_expression_bool = true;
    break;
  case pp_primary_expression::type_false:
    _M_expression_bool = false;
    break;
  case pp_primary_expression::type_conditional_symbol:
    {
      decoder d(_M_pp_parser->token_stream);
      _M_expression_bool = _M_scope->csharp_parser()->pp_is_symbol_defined(
        d.decode_id(node->conditional_symbol)
      );
    }
    break;
  case pp_primary_expression::type_parenthesis_expression:
    default_visitor::visit_node(node->parenthesis_expression);
    break;
  }
}

} // end of namespace csharp_pp
