// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#ifndef csharp_pp_DEFAULT_VISITOR_H_INCLUDED
#define csharp_pp_DEFAULT_VISITOR_H_INCLUDED

#include "csharp_pp_visitor.h"

namespace csharp_pp
  {

  class default_visitor: public visitor
    {
    public:
      virtual void visit_pp_and_expression(pp_and_expression_ast *node);
      virtual void visit_pp_declaration(pp_declaration_ast *node);
      virtual void visit_pp_diagnostic(pp_diagnostic_ast *node);
      virtual void visit_pp_directive(pp_directive_ast *node);
      virtual void visit_pp_elif_clause(pp_elif_clause_ast *node);
      virtual void visit_pp_else_clause(pp_else_clause_ast *node);
      virtual void visit_pp_endif_clause(pp_endif_clause_ast *node);
      virtual void visit_pp_equality_expression(pp_equality_expression_ast *node);
      virtual void visit_pp_equality_expression_rest(pp_equality_expression_rest_ast *node);
      virtual void visit_pp_expression(pp_expression_ast *node);
      virtual void visit_pp_if_clause(pp_if_clause_ast *node);
      virtual void visit_pp_line(pp_line_ast *node);
      virtual void visit_pp_pragma(pp_pragma_ast *node);
      virtual void visit_pp_primary_expression(pp_primary_expression_ast *node);
      virtual void visit_pp_region(pp_region_ast *node);
      virtual void visit_pp_unary_expression(pp_unary_expression_ast *node);
    };

} // end of namespace csharp_pp

#endif


