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

#include "ast.h"
#include "kdevdocumentrange.h"
#include "kdevdocumentrangeobject.h"

using namespace KTextEditor;

QHash<TokenStream*, Lexer*> CppEditorIntegrator::s_parsedSources;

CppEditorIntegrator::CppEditorIntegrator( TokenStream * tokenStream )
  : m_lexer(s_parsedSources[tokenStream])
  , m_tokenStream(tokenStream)
{
}

void CppEditorIntegrator::addParsedSource( Lexer * lexer, TokenStream * tokenStream )
{
  s_parsedSources.insert(tokenStream, lexer);
}

KDevDocumentCursor CppEditorIntegrator::findPosition( std::size_t token, Edge edge ) const
{
  const Token& t = m_tokenStream->token(token);
  return findPosition(t, edge);
}

KDevDocumentCursor CppEditorIntegrator::findPosition( const Token & token, Edge edge ) const
{
  int line, column;
  QString fileName;

  m_lexer->positionAt((edge == BackEdge) ? token.position + token.size : token.position,
                       &line, &column, &fileName);

  if (fileName.isEmpty())
    // FIXME assumption wrong? Best to fix in the parser I think, always return a filename.
    return KDevDocumentCursor(m_currentUrl, Cursor(line, column));
  else
    return KDevDocumentCursor(KUrl(fileName), Cursor(line, column));
}

Range* CppEditorIntegrator::createRange( AST * node, RangeEdge edge )
{
  Q_UNUSED(edge);
  return createRange(findPosition(node->start_token, FrontEdge), findPosition(node->end_token, FrontEdge));
}

Range* CppEditorIntegrator::createRange( const Token & token )
{
  return createRange(findPosition(token, FrontEdge), findPosition(token, FrontEdge));
}

Cursor * CppEditorIntegrator::createCursor( std::size_t token, Edge edge )
{
  const Token& t = m_tokenStream->token(token);
  return createCursor(findPosition(t, edge));
}

// kate: indent-width 2;
