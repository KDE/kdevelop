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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "editmodelbuilder.h"

#include <ktexteditor/smartrange.h>
#include <ktexteditor/document.h>
#include <ktexteditor/smartinterface.h>

#include "coolhighlights.h"

#include <QVector>

using namespace KTextEditor;

QVector<int> _G_newLineLocations;

EditModelBuilder::EditModelBuilder(KTextEditor::SmartRange* topRange, const cool::token_stream_type& token_stream, QHash<KTextEditor::SmartRange*, int>& recoveryPoints, const KTextEditor::Cursor& offset, bool monochrome)
  : m_topRange(topRange)
  , m_currentRange(topRange)
  , m_tokenStream(token_stream)
  , m_depth(0)
  , m_monochrome(monochrome)
  , m_recoveryPoints(recoveryPoints)
  , m_offset(offset)
{
}

EditModelBuilder::~EditModelBuilder()
{
}

void EditModelBuilder::visit_node( cool_ast_node * node )
{
  if (node && m_monochrome) {
    m_currentRange = newRange(node->start_token, node->end_token);
    m_currentRange->setAttribute(CoolHighlights::depthHighlight(m_depth++));
    m_recoveryPoints.insert(m_currentRange, node->kind);
  }

  cool_default_visitor::visit_node( node );

  if (node && m_monochrome) {
    m_currentRange = m_currentRange->parentRange();
    --m_depth;
    //kDebug() << k_funcinfo << node->start_token << ", " << node->start_token << " translates to " << tokenToPosition(node->start_token) << ", " << tokenToPosition(node->end_token) << endl;
  }
}

void EditModelBuilder::visit_class( class_ast * ast )
{
  if (!m_monochrome) {
    m_currentRange = newRange(ast->start_token, ast->end_token);
    m_currentRange->setAttribute(CoolHighlights::classHighlight());
  }

  newRange(ast->type)->setAttribute(CoolHighlights::typeHighlight());
  newRange(ast->base_type)->setAttribute(CoolHighlights::typeHighlight());

  cool_default_visitor::visit_class(ast);

  if (!m_monochrome)
    m_currentRange = m_currentRange->parentRange();
}

void EditModelBuilder::visit_block_expression( block_expression_ast * ast )
{
  if (!m_monochrome) {
    m_currentRange = newRange(ast->start_token, ast->end_token);
    m_currentRange->setAttribute(CoolHighlights::nextHighlight());
  }

  cool_default_visitor::visit_block_expression(ast);

  if (!m_monochrome)
    m_currentRange = m_currentRange->parentRange();
}

void EditModelBuilder::visit_case_condition( case_condition_ast * ast )
{
  newRange(ast->type)->setAttribute(CoolHighlights::typeHighlight());

  cool_default_visitor::visit_case_condition(ast);
}

void EditModelBuilder::visit_case_expression( case_expression_ast * ast )
{
  cool_default_visitor::visit_case_expression(ast);
}

void EditModelBuilder::visit_expression( expression_ast * ast )
{
  //m_currentRange = newRange(ast->start_token, ast->end_token);
  //m_currentRange->setAttribute(CoolHighlights::nextHighlight());

  cool_default_visitor::visit_expression(ast);

  //m_currentRange = m_currentRange->parentRange();
}

void EditModelBuilder::visit_feature( feature_ast * ast )
{
  if (!m_monochrome) {
    m_currentRange = newRange(ast->start_token, ast->end_token);
    m_currentRange->setAttribute(CoolHighlights::methodHighlight());
  }

  SmartRange* name = newRange(ast->name);
  name->setAttribute(CoolHighlights::nameHighlight());

  cool_default_visitor::visit_feature(ast);

  SmartRange* type = newRange(ast->type);
  type->setAttribute(CoolHighlights::typeHighlight());

  kDebug() << k_funcinfo << name->text() << " type " << type->text() << endl;

  //debugOutput(m_currentRange);

  if (!m_monochrome)
    m_currentRange = m_currentRange->parentRange();
}

void EditModelBuilder::visit_formal( formal_ast * ast )
{
  if (!m_monochrome) {
    m_currentRange = newRange(ast->start_token, ast->end_token, true, false);
    m_currentRange->setAttribute(CoolHighlights::variableDefinitionHighlight());
  }

  SmartRange* name = newRange(ast->name);
  name->setAttribute(CoolHighlights::variableHighlight());

  SmartRange* type = newRange(ast->type);
  type->setAttribute(CoolHighlights::typeHighlight());

  cool_default_visitor::visit_formal(ast);

  if (!m_monochrome)
    m_currentRange = m_currentRange->parentRange();
}

void EditModelBuilder::visit_if_expression( if_expression_ast * ast )
{
  cool_default_visitor::visit_if_expression(ast);
}

void EditModelBuilder::visit_let_declaration( let_declaration_ast * ast )
{
  cool_default_visitor::visit_let_declaration(ast);

  newRange(ast->type)->setAttribute(CoolHighlights::typeHighlight());
}

void EditModelBuilder::visit_let_expression( let_expression_ast * ast )
{
  cool_default_visitor::visit_let_expression(ast);
}

void EditModelBuilder::visit_multiplicative_expression( multiplicative_expression_ast * ast )
{
  cool_default_visitor::visit_multiplicative_expression(ast);
}

void EditModelBuilder::visit_postfix_expression( postfix_expression_ast * ast )
{
  cool_default_visitor::visit_postfix_expression(ast);
}

void EditModelBuilder::visit_primary_expression( primary_expression_ast * ast )
{
  cool_default_visitor::visit_primary_expression(ast);

  if (ast->variable) {
    SmartRange* variable = newRange(ast->variable);
    variable->setAttribute(CoolHighlights::variableHighlight());
  }

  if (ast->new_type)
    newRange(ast->new_type)->setAttribute(CoolHighlights::typeHighlight());
}

void EditModelBuilder::visit_relational_expression( relational_expression_ast * ast )
{
  //m_currentRange = newRange(ast->start_token, ast->end_token);
  //m_currentRange->setAttribute(CoolHighlights::nextHighlight());

  cool_default_visitor::visit_relational_expression(ast);

  //m_currentRange = m_currentRange->parentRange();
}

SmartRange * EditModelBuilder::newRange( std::size_t start_token, std::size_t end_token, bool includeStartToken, bool includeEndToken )
{
  return smart()->newSmartRange(tokenToPosition(start_token, !includeStartToken), tokenToPosition(end_token, includeEndToken), m_currentRange);
}

Cursor EditModelBuilder::tokenToPosition( std::size_t token, bool end )
{
  const kdev_pg_token_stream::token_type& actualToken = m_tokenStream.token(token);

  int len = end ? actualToken.end : actualToken.begin;

  if (len == 0)
    return Cursor();

  int i = 0;
  for (; i < _G_newLineLocations.count() - 1; ++i)
    if (len >= _G_newLineLocations[i] && len <= _G_newLineLocations[i+1])
        return Cursor(i, len - _G_newLineLocations[i]);

  return Cursor(i, len - _G_newLineLocations.last()) + m_offset;
}

SmartInterface * EditModelBuilder::smart( ) const
{
  return dynamic_cast<SmartInterface*>(m_topRange->document());
}

KTextEditor::SmartRange * EditModelBuilder::newRange( std::size_t token )
{
  return smart()->newSmartRange(tokenToPosition(token), tokenToPosition(token, true), m_currentRange);
}

void EditModelBuilder::debugOutput( KTextEditor::SmartRange * range, int depth )
{
  kDebug() << QString(depth, QChar('\\')) << range << " " << *range << " attrib " << range->attribute() << endl;
  foreach (SmartRange* child, range->childRanges())
    debugOutput(child, depth+1);
}

