/* This file is part of the KDE libraries
   Copyright (C) 2005 Hamish Rodda <rodda@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef EDITMODELBUILDER_H
#define EDITMODELBUILDER_H

#include "cool.h"

namespace KTextEditor {
  class SmartRange;
}

class EditModelBuilder : public cool_default_visitor
{
  public:
    EditModelBuilder(KTextEditor::SmartRange* m_topRange);
    virtual ~EditModelBuilder();

    /*virtual void visit_node(cool_ast_node *node);
    virtual void visit_additive_expression(additive_expression_ast *ast);
    virtual void visit_block_expression(block_expression_ast *ast);
    virtual void visit_case_condition(case_condition_ast *ast);
    virtual void visit_case_expression(case_expression_ast *ast);*/
    virtual void visit_class(class_ast *ast);
    /*virtual void visit_expression(expression_ast *ast);
    virtual void visit_feature(feature_ast *ast);
    virtual void visit_formal(formal_ast *ast);
    virtual void visit_if_expression(if_expression_ast *ast);
    virtual void visit_let_declaration(let_declaration_ast *ast);
    virtual void visit_let_expression(let_expression_ast *ast);
    virtual void visit_multiplicative_expression(multiplicative_expression_ast *ast);
    virtual void visit_postfix_expression(postfix_expression_ast *ast);
    virtual void visit_primary_expression(primary_expression_ast *ast);
    virtual void visit_program(program_ast *ast);
    virtual void visit_relational_expression(relational_expression_ast *ast);
    virtual void visit_unary_expression(unary_expression_ast *ast);
    virtual void visit_while_expression(while_expression_ast *ast);*/

  private:
    KTextEditor::SmartRange* newRange(std::size_t start_token, std::size_t end_token);

    KTextEditor::SmartRange* m_topRange;
    KTextEditor::SmartRange* m_currentRange;
};

#endif
