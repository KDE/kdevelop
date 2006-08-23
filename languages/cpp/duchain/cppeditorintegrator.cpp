/* This file is part of KDevelop
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

#include "cppeditorintegrator.h"

#include <ktexteditor/document.h>
#include <ktexteditor/smartrange.h>
#include <ktexteditor/smartinterface.h>

#include "kdevdocumentrange.h"
#include "kdevdocumentrangeobject.h"

#include "ast.h"
#include "parsesession.h"

using namespace KTextEditor;

CppEditorIntegrator::CppEditorIntegrator( ParseSession* session )
  : m_session(session)
{
}

Cursor CppEditorIntegrator::findPosition( std::size_t token, Edge edge ) const
{
  const Token& t = m_session->token_stream->token(token);
  return findPosition(t, edge);
}

Cursor CppEditorIntegrator::findPosition( const Token & token, Edge edge ) const
{
  int line, column;
  QString fileName;

  m_session->positionAt((edge == BackEdge) ? token.position + token.size : token.position,
                       &line, &column, &fileName);

  return Cursor(line, column);
}

Range* CppEditorIntegrator::createRange( AST * node, RangeEdge edge )
{
  Q_UNUSED(edge);
  return createRange(Range(findPosition(node->start_token, FrontEdge), findPosition(node->end_token, FrontEdge)));
}

KTextEditor::Range* CppEditorIntegrator::createRange(AST* from, AST* to)
{
  return createRange(Range(findPosition(from->start_token, FrontEdge), findPosition(to->end_token, FrontEdge)));
}

Range* CppEditorIntegrator::createRange( const Token & token )
{
  return createRange(Range(findPosition(token, FrontEdge), findPosition(token, BackEdge)));
}

Cursor * CppEditorIntegrator::createCursor( std::size_t token, Edge edge )
{
  const Token& t = m_session->token_stream->token(token);
  return createCursor(findPosition(t, edge));
}

QString CppEditorIntegrator::tokenToString(std::size_t token) const
{
  return m_session->token_stream->symbol(token)->as_string();
}

// kate: indent-width 2;
