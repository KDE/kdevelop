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

#ifndef CSHARP_PP_HANDLER_VISITOR_H
#define CSHARP_PP_HANDLER_VISITOR_H

#include "csharp_pp.h"

namespace csharp_pp
{

class scope;


class handler_visitor : public default_visitor
{
public:
    handler_visitor( parser* pp_parser );
    ~handler_visitor();

    void set_scope( scope* current_scope );

    virtual void visit_pp_declaration(pp_declaration_ast* node);
    virtual void visit_pp_if_clause(pp_if_clause_ast* node);
    virtual void visit_pp_elif_clause(pp_elif_clause_ast* node);
    virtual void visit_pp_else_clause(pp_else_clause_ast* node);
    virtual void visit_pp_endif_clause(pp_endif_clause_ast* node);
    virtual void visit_pp_diagnostic(pp_diagnostic_ast* node);
    virtual void visit_pp_region(pp_region_ast* node);
    virtual void visit_pp_line(pp_line_ast* node);
    virtual void visit_pp_pragma(pp_pragma_ast* node);

    virtual void visit_pp_expression(pp_expression_ast* node);
    virtual void visit_pp_and_expression(pp_and_expression_ast* node);
    virtual void visit_pp_equality_expression(pp_equality_expression_ast* node);
    virtual void visit_pp_equality_expression_rest(pp_equality_expression_rest_ast* node);
    virtual void visit_pp_unary_expression(pp_unary_expression_ast* node);
    virtual void visit_pp_primary_expression(pp_primary_expression_ast* node);

private:
    parser* _M_pp_parser;
    scope* _M_scope;

    /** Temporary variable for evaluating #if and #elif conditions. */
    bool _M_expression_bool;
};

} // end of namespace csharp_pp

#endif // CSHARP_PP_HANDLER_VISITOR_H
