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

#include "editorintegrator.h"

#include <ktexteditor/document.h>
#include <ktexteditor/smartrange.h>
#include <ktexteditor/smartinterface.h>

#include "ast.h"

QHash<TokenStream*, Lexer*> EditorIntegrator::s_parsedSources;
QList<KTextEditor::Document*> EditorIntegrator::s_documents;
QHash<KTextEditor::Document*, QVector<KTextEditor::SmartRange*> > EditorIntegrator::s_topRanges;

EditorIntegrator::EditorIntegrator( TokenStream * tokenStream )
  : m_lexer(s_parsedSources[tokenStream])
  , m_tokenStream(tokenStream)
  , m_currentDocument(0)
  , m_currentRange(0)
{
}

void EditorIntegrator::addDocument( KTextEditor::Document * document )
{
  s_documents.append(document);
}

void EditorIntegrator::addParsedSource( Lexer * lexer, TokenStream * tokenStream )
{
  s_parsedSources.insert(tokenStream, lexer);
}

void EditorIntegrator::setCurrentDocument( KTextEditor::Document * document )
{
  m_currentDocument = document;
}

KTextEditor::SmartInterface* EditorIntegrator::smart() const
{
  return dynamic_cast<KTextEditor::SmartInterface*>(currentDocument());
}

KTextEditor::SmartCursor* EditorIntegrator::createCursor(const KTextEditor::Cursor& position)
{
  if (KTextEditor::SmartInterface* iface = smart())
    return iface->newSmartCursor(position);

  return 0;
}

KTextEditor::Cursor EditorIntegrator::findPosition( std::size_t token, Edge edge, QString * file ) const
{
  const Token& t = m_tokenStream->token(token);
  return findPosition(t, edge, file);
}

KTextEditor::Cursor EditorIntegrator::findPosition( const Token & token, Edge edge, QString * file ) const
{
  int line, column;
  QString fileName;

  m_lexer->positionAt((edge == BackEdge) ? token.position + token.size : token.position,
                       &line, &column, &fileName);

  if (file) {
    if (fileName.isEmpty())
      // FIXME assumption wrong? Best to fix in the parser I think, always return a filename.
      fileName = currentDocument()->url().toString();//m_currentFile;

    *file = fileName;
  }

  return KTextEditor::Cursor(line, column);
}

KTextEditor::Document* EditorIntegrator::currentDocument() const
{
  return m_currentDocument;
}

KTextEditor::SmartRange * EditorIntegrator::topRange( TopRangeType type )
{
  if (s_topRanges.contains(currentDocument()))
    return s_topRanges[currentDocument()][type];

  return 0;
}

KTextEditor::SmartRange * EditorIntegrator::createRange( const KTextEditor::Range & range )
{
  if (KTextEditor::SmartInterface* iface = smart()) {
    m_currentRange = iface->newSmartRange(range, m_currentRange);
    return m_currentRange;
  }

  return 0;
}


KTextEditor::SmartRange * EditorIntegrator::createRange( const KTextEditor::Cursor & start, const KTextEditor::Cursor & end )
{
  return createRange(KTextEditor::Range(start, end));
}

KTextEditor::SmartRange * EditorIntegrator::createRange()
{
  return createRange(m_newRangeMarker);
}

KTextEditor::SmartRange * EditorIntegrator::createRange( AST * node, RangeEdge edge )
{
  return createRange(findPosition(node->start_token, edge == OuterEdge ? FrontEdge : BackEdge), findPosition(node->end_token, edge == InnerEdge ? FrontEdge : BackEdge));
}

KTextEditor::SmartRange * EditorIntegrator::createRange( const Token & token )
{
  return createRange(findPosition(token, FrontEdge), findPosition(token, BackEdge));
}

void EditorIntegrator::setNewRange(const KTextEditor::Range& range)
{
  m_newRangeMarker = range;
}

void EditorIntegrator::setNewEnd( const KTextEditor::Cursor & position )
{
  m_newRangeMarker.end() = position;
}

void EditorIntegrator::setNewStart( const KTextEditor::Cursor & position )
{
  m_newRangeMarker.start() = position;
}

KTextEditor::SmartCursor * EditorIntegrator::createCursor( std::size_t token, Edge edge )
{
  const Token& t = m_tokenStream->token(token);
  return createCursor(findPosition(t, edge));
}


void EditorIntegrator::removeTextSource( KTextEditor::Document * document )
{
  s_documents.removeAll(document);
  s_topRanges.remove(document);
}

void EditorIntegrator::setCurrentRange( KTextEditor::SmartRange * range )
{
  m_currentRange = range;
  m_currentDocument = range->document();
}

KTextEditor::SmartRange * EditorIntegrator::currentRange( ) const
{
  return m_currentRange;
}

#include "editorintegrator.moc"

// kate: indent-width 2;
