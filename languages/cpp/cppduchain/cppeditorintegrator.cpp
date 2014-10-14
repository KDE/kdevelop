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

#include <language/editor/documentrange.h>
#include <serialization/indexedstring.h>

#include "ast.h"
#include "parsesession.h"
#include "debug.h"
#include <rpp/chartools.h>

using namespace KDevelop;

CppEditorIntegrator::CppEditorIntegrator( ParseSession* session )
  : m_session(session)
{
}

CursorInRevision CppEditorIntegrator::findPosition( std::size_t token, Edge edge ) const
{
  if(token == 0) {
    qCDebug(CPPDUCHAIN) << "Searching position of invalid token";
    return CursorInRevision();
    }

  const Token& t = m_session->token_stream->token(token);
  return findPosition(t, edge);
}

CursorInRevision CppEditorIntegrator::findPosition( const Token & token, Edge edge ) const
{
  QPair<rpp::Anchor, uint> a = m_session->positionAndSpaceAt(token.position);
  rpp::Anchor position = a.first;
  if(edge == BackEdge) {
    if(position.collapsed)
      return position;
    else {
      uint length = m_session->token_stream->symbolLength(token);
      if(a.second && length > a.second)
        length = a.second;
      //We have to check the following anchor in the location-table to make sure we don't make the range longer than possible
      return position + CursorInRevision(0, length);
    }
  } else
    return position;
}

RangeInRevision CppEditorIntegrator::findRange( AST * node, RangeEdge edge )
{
  Q_UNUSED(edge);
  return RangeInRevision(findPosition(node->start_token, FrontEdge), findPosition(node->end_token - 1, BackEdge));
}

RangeInRevision CppEditorIntegrator::findRangeForContext( size_t start_token, size_t end_token )
{
  if(start_token == 0 || end_token == 0) {
    qCDebug(CPPDUCHAIN) << "Searching position of invalid token";
    return RangeInRevision();
  }
  const Token& tStart = m_session->token_stream->token(start_token);
  const Token& tEnd = m_session->token_stream->token(end_token-1);

  rpp::Anchor start = m_session->positionAt(tStart.position, true);
  rpp::Anchor end = m_session->positionAt(tEnd.position, true);
  if(!end.collapsed)
    end.column += m_session->token_stream->symbolLength(tEnd); //We want the back edge

  if(start.macroExpansion.isValid() && start.macroExpansion == end.macroExpansion)
    return RangeInRevision(start.macroExpansion, start.macroExpansion);
  else
    return RangeInRevision(start, end);
}

RangeInRevision CppEditorIntegrator::findRange( size_t start_token, size_t end_token )
{
  return RangeInRevision(findPosition(start_token, FrontEdge), findPosition(end_token - 1, BackEdge));
}

RangeInRevision CppEditorIntegrator::findRange( size_t token )
{
  return RangeInRevision(findPosition(token, FrontEdge), findPosition(token, BackEdge));
}

RangeInRevision CppEditorIntegrator::findRange(AST* from, AST* to)
{
  return RangeInRevision(findPosition(from->start_token, FrontEdge), findPosition(to->end_token - 1, BackEdge));
}

RangeInRevision CppEditorIntegrator::findRange( const Token & token )
{
  return RangeInRevision(findPosition(token, FrontEdge), findPosition(token, BackEdge));
}

QString CppEditorIntegrator::tokenToString(std::size_t token) const
{
  return m_session->token_stream->symbolString(token);
}

QString CppEditorIntegrator::tokensToStrings(std::size_t start, std::size_t end) const
{
  QString ret;
  for(std::size_t a = start; a < end; ++a) {
    ret += tokenToString(a) + " ";
  }
  return ret;
}

QByteArray CppEditorIntegrator::tokenToByteArray(std::size_t token) const
{
  return m_session->token_stream->symbolByteArray(token);
}

QByteArray CppEditorIntegrator::tokensToByteArray(std::size_t start, std::size_t end) const
{
  QByteArray ret;
  for(std::size_t a = start; a < end; ++a) {
    ret += tokenToByteArray(a) + " ";
  }
  return ret;
}

ParseSession * CppEditorIntegrator::parseSession() const
{
  return m_session;
}

