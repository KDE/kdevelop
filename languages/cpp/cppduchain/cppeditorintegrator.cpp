/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>

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

#include <documentrange.h>
#include <documentrangeobject.h>

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
  return m_session->positionAt((edge == BackEdge) ? token.position + token.size : token.position);
}

Range CppEditorIntegrator::findRange( AST * node, RangeEdge edge )
{
  Q_UNUSED(edge);
  return Range(findPosition(node->start_token, FrontEdge), findPosition(node->end_token - 1, BackEdge));
}

Range CppEditorIntegrator::findRange( size_t start_token, size_t end_token )
{
  return Range(findPosition(start_token, FrontEdge), findPosition(end_token - 1, BackEdge));
}

Range CppEditorIntegrator::findRange(AST* from, AST* to)
{
  return Range(findPosition(from->start_token, FrontEdge), findPosition(to->end_token - 1, BackEdge));
}

Range CppEditorIntegrator::findRange( const Token & token )
{
  return Range(findPosition(token, FrontEdge), findPosition(token, BackEdge));
}

QString CppEditorIntegrator::tokenToString(std::size_t token) const
{
  return m_session->token_stream->token(token).symbol();
}

ParseSession * CppEditorIntegrator::parseSession() const
{
  return m_session;
}

