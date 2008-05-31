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
using namespace KDevelop;

CppEditorIntegrator::CppEditorIntegrator( ParseSession* session )
  : m_session(session)
{
}

SimpleCursor CppEditorIntegrator::findPosition( std::size_t token, Edge edge ) const
{
  if(token == 0) {
    kDebug() << "Searching position of invalid token";
    return SimpleCursor();
    }
  
  const Token& t = m_session->token_stream->token(token);
  return findPosition(t, edge);
}

SimpleCursor CppEditorIntegrator::findPosition( const Token & token, Edge edge ) const
{
  rpp::Anchor position = m_session->positionAt(token.position);
  if(edge == BackEdge) {
    if(position.collapsed)
      return position;
    else
      return position + SimpleCursor(0, token.size);
  } else
    return position;
}

SimpleRange CppEditorIntegrator::findRange( AST * node, RangeEdge edge )
{
  Q_UNUSED(edge);
  return SimpleRange(findPosition(node->start_token, FrontEdge), findPosition(node->end_token - 1, BackEdge));
}

SimpleRange CppEditorIntegrator::findRangeForContext( size_t start_token, size_t end_token )
{
  if(start_token == 0 || end_token == 0) {
    kDebug() << "Searching position of invalid token";
    return SimpleRange();
  }
  const Token& tStart = m_session->token_stream->token(start_token);
  const Token& tEnd = m_session->token_stream->token(end_token-1);

  rpp::Anchor start = m_session->positionAt(tStart.position, true);
  rpp::Anchor end = m_session->positionAt(tEnd.position, true);
  if(!end.collapsed)
    end.column += tEnd.size; //We want the back edge
  
  if(start.macroExpansion.isValid() && start.macroExpansion == end.macroExpansion)
    return SimpleRange(start.macroExpansion, start.macroExpansion);
  else
    return SimpleRange(start, end);
}

SimpleRange CppEditorIntegrator::findRange( size_t start_token, size_t end_token )
{
  return SimpleRange(findPosition(start_token, FrontEdge), findPosition(end_token - 1, BackEdge));
}

SimpleRange CppEditorIntegrator::findRange(AST* from, AST* to)
{
  return SimpleRange(findPosition(from->start_token, FrontEdge), findPosition(to->end_token - 1, BackEdge));
}

SimpleRange CppEditorIntegrator::findRange( const Token & token )
{
  return SimpleRange(findPosition(token, FrontEdge), findPosition(token, BackEdge));
}

QString CppEditorIntegrator::tokenToString(std::size_t token) const
{
  return m_session->token_stream->token(token).symbol();
}

QString CppEditorIntegrator::tokensToStrings(std::size_t start, std::size_t end) const
{
  QString ret;
  for(std::size_t a = start; a < end; ++a) {
    ret += tokenToString(a) + " ";
  }
  return ret;
}

ParseSession * CppEditorIntegrator::parseSession() const
{
  return m_session;
}

