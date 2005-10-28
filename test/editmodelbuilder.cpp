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

#include "editmodelbuilder.h"

#include <ktexteditor/smartrange.h>
#include <ktexteditor/document.h>
#include <ktexteditor/smartinterface.h>

#include "coolhighlights.h"

#include <QVector>

using namespace KTextEditor;

QVector<int> _G_tokenLocations;

EditModelBuilder::EditModelBuilder(KTextEditor::SmartRange* topRange)
  : m_topRange(topRange)
  , m_currentRange(topRange)
{
}

EditModelBuilder::~EditModelBuilder()
{
}

void EditModelBuilder::visit_node( cool_ast_node * node )
{
  cool_default_visitor::visit_node( node );

  //kdDebug() << k_funcinfo << tokenToPosition(node->start_token) << " " << tokenToPosition(node->end_token) << endl;
}

void EditModelBuilder::visit_class( class_ast * ast )
{
  m_currentRange = newRange(ast->start_token, ast->end_token);
  m_currentRange->setAttribute(CoolHighlights::classHighlight());

  cool_default_visitor::visit_class(ast);

  m_currentRange = m_currentRange->parentRange();
}

SmartRange * EditModelBuilder::newRange( std::size_t start_token, std::size_t end_token )
{
  return dynamic_cast<SmartInterface*>(m_topRange->document())->newSmartRange(tokenToPosition(start_token), tokenToPosition(end_token), m_topRange);
}

Cursor EditModelBuilder::tokenToPosition( std::size_t token )
{
  if (token == 0)
    return Cursor();

  for (int i = 0; i < _G_tokenLocations.count(); ++i)
    if (token > _G_tokenLocations[i])
        return Cursor(i, token - _G_tokenLocations[i]);

  return Cursor::invalid();
}
