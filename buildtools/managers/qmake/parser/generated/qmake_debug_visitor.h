// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#ifndef qmake_DEBUG_VISITOR_H_INCLUDED
#define qmake_DEBUG_VISITOR_H_INCLUDED

#include "qmake_default_visitor.h"

#include <iostream>
#include <fstream>

namespace QMake
  {

  class debug_visitor:  public default_visitor
    {

    public:
      virtual void visit_arg_list(arg_list_ast *node)
      {
        std::cout <<  "arg_list" <<  std::endl;
        default_visitor::visit_arg_list(node);
      }

      virtual void visit_funcref(funcref_ast *node)
      {
        std::cout <<  "funcref" <<  std::endl;
        default_visitor::visit_funcref(node);
      }

      virtual void visit_function_args(function_args_ast *node)
      {
        std::cout <<  "function_args" <<  std::endl;
        default_visitor::visit_function_args(node);
      }

      virtual void visit_function_scope(function_scope_ast *node)
      {
        std::cout <<  "function_scope" <<  std::endl;
        default_visitor::visit_function_scope(node);
      }

      virtual void visit_id_or_value(id_or_value_ast *node)
      {
        std::cout <<  "id_or_value" <<  std::endl;
        default_visitor::visit_id_or_value(node);
      }

      virtual void visit_op(op_ast *node)
      {
        std::cout <<  "op" <<  std::endl;
        default_visitor::visit_op(node);
      }

      virtual void visit_project(project_ast *node)
      {
        std::cout <<  "project" <<  std::endl;
        default_visitor::visit_project(node);
      }

      virtual void visit_quoted_value(quoted_value_ast *node)
      {
        std::cout <<  "quoted_value" <<  std::endl;
        default_visitor::visit_quoted_value(node);
      }

      virtual void visit_ref(ref_ast *node)
      {
        std::cout <<  "ref" <<  std::endl;
        default_visitor::visit_ref(node);
      }

      virtual void visit_scope_body(scope_body_ast *node)
      {
        std::cout <<  "scope_body" <<  std::endl;
        default_visitor::visit_scope_body(node);
      }

      virtual void visit_stmt(stmt_ast *node)
      {
        std::cout <<  "stmt" <<  std::endl;
        default_visitor::visit_stmt(node);
      }

      virtual void visit_value_list(value_list_ast *node)
      {
        std::cout <<  "value_list" <<  std::endl;
        default_visitor::visit_value_list(node);
      }

      virtual void visit_variable_assignment(variable_assignment_ast *node)
      {
        std::cout <<  "variable_assignment" <<  std::endl;
        default_visitor::visit_variable_assignment(node);
      }

      virtual void visit_varref(varref_ast *node)
      {
        std::cout <<  "varref" <<  std::endl;
        default_visitor::visit_varref(node);
      }

    };

} // end of namespace QMake

#endif


