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
#include "documentrange.h"
#include "rangeobject.h"

using namespace KTextEditor;

QHash<TokenStream*, Lexer*> EditorIntegrator::s_parsedSources;
QHash<KUrl, Document*> EditorIntegrator::s_documents;
QHash<KUrl, QVector<Range*> > EditorIntegrator::s_topRanges;

EditorIntegrator::EditorIntegrator( TokenStream * tokenStream )
  : m_lexer(s_parsedSources[tokenStream])
  , m_tokenStream(tokenStream)
  , m_currentDocument(0)
  , m_currentRange(0)
{
}

void EditorIntegrator::addDocument( Document * document )
{
  s_documents.insert(document->url(), document);
}

void EditorIntegrator::addParsedSource( Lexer * lexer, TokenStream * tokenStream )
{
  s_parsedSources.insert(tokenStream, lexer);
}

SmartInterface* EditorIntegrator::smart(KTextEditor::Document* document) const
{
  return dynamic_cast<SmartInterface*>(document ? document : currentDocument());
}

Cursor* EditorIntegrator::createCursor(const DocumentCursor& position)
{
  Cursor* ret = 0;

  if (position.document() == m_currentUrl) {
    if (SmartInterface* iface = smart())
      ret = iface->newSmartCursor(position);
  } else {
    KTextEditor::Document* document = s_documents[position.document()];
    if (SmartInterface* iface = smart(document))
      ret = iface->newSmartCursor(position);
  }

  if (!ret)
    ret = new DocumentCursor(position);

  return ret;
}

DocumentCursor EditorIntegrator::findPosition( std::size_t token, Edge edge ) const
{
  const Token& t = m_tokenStream->token(token);
  return findPosition(t, edge);
}

DocumentCursor EditorIntegrator::findPosition( const Token & token, Edge edge ) const
{
  int line, column;
  QString fileName;

  m_lexer->positionAt((edge == BackEdge) ? token.position + token.size : token.position,
                       &line, &column, &fileName);

  if (fileName.isEmpty())
    // FIXME assumption wrong? Best to fix in the parser I think, always return a filename.
    return DocumentCursor(m_currentUrl, Cursor(line, column));
  else
    return DocumentCursor(KUrl(fileName), Cursor(line, column));
}

Document* EditorIntegrator::currentDocument() const
{
  return m_currentDocument;
}

Range* EditorIntegrator::topRange( TopRangeType type )
{
  if (!s_topRanges.contains(currentUrl()))
    s_topRanges.insert(currentUrl(), QVector<Range*>(TopRangeCount));

  if (!s_topRanges[currentUrl()][type])
    if (currentDocument()) {
      Range* newRange = s_topRanges[currentUrl()][type] = createRange(currentDocument()->documentRange());
      if (SmartInterface* iface = smart()) {
        Q_ASSERT(newRange->isSmartRange());
        iface->addHighlightToDocument( newRange->toSmartRange(), false );
      }

    } else {
      // FIXME...
      s_topRanges[currentUrl()][type] = createRange(Range(0,0, INT_MAX, INT_MAX));
    }

  return s_topRanges[currentUrl()][type];
}

Range* EditorIntegrator::createRange( const Range & range, KTextEditor::Document* document )
{
  Range* ret;

  if (SmartInterface* iface = smart(document))
    if (m_currentRange && m_currentRange->isSmartRange())
      ret = iface->newSmartRange(range, static_cast<SmartRange*>(m_currentRange));
    else
      ret = iface->newSmartRange(range);
  else
    ret = new DocumentRange(m_currentUrl, range);

  m_currentRange = ret;
  return m_currentRange;
}


Range* EditorIntegrator::createRange( const DocumentCursor& start, const DocumentCursor& end )
{
  if (start.document() != end.document()) {
    kWarning() << k_funcinfo << "Start: " << start << ", End: " << end << ", documents " << start.document() << " != " << end.document() << endl;
    // FIXME difficult problem
    createRange(Range(start, start), documentForUrl(start.document()));
  }

  return createRange(Range(start, end), documentForUrl(start.document()));
}

Range* EditorIntegrator::createRange()
{
  return createRange(m_newRangeMarker);
}

Range* EditorIntegrator::createRange( AST * node, RangeEdge edge )
{
  Q_UNUSED(edge);
  return createRange(findPosition(node->start_token, FrontEdge), findPosition(node->end_token, FrontEdge));
}

Range* EditorIntegrator::createRange( const Token & token )
{
  return createRange(findPosition(token, FrontEdge), findPosition(token, BackEdge));
}

void EditorIntegrator::setNewRange(const Range& range)
{
  m_newRangeMarker = range;
}

void EditorIntegrator::setNewEnd( const Cursor & position )
{
  m_newRangeMarker.end() = position;
}

void EditorIntegrator::setNewStart( const Cursor & position )
{
  m_newRangeMarker.start() = position;
}

Cursor * EditorIntegrator::createCursor( std::size_t token, Edge edge )
{
  const Token& t = m_tokenStream->token(token);
  return createCursor(findPosition(t, edge));
}

Document * EditorIntegrator::documentForUrl(const KUrl& url)
{
  foreach (Document* d, s_documents)
    if (d->url() == url)
      return d;

  return 0;
}

void EditorIntegrator::removeTextSource( Document * document )
{
  s_documents.remove(document->url());
  s_topRanges.remove(document->url());
}

void EditorIntegrator::setCurrentRange( Range* range )
{
  m_currentRange = range;
}

Range* EditorIntegrator::currentRange( ) const
{
  return m_currentRange;
}

const KUrl& EditorIntegrator::currentUrl() const
{
  return m_currentUrl;
}

void EditorIntegrator::setCurrentUrl(const KUrl& url)
{
  m_currentUrl = url;
  m_currentDocument = documentForUrl(url);
}

void EditorIntegrator::deleteTopRange(KTextEditor::Range * range)
{
  KUrl url = RangeObject::url(range);

  if (s_topRanges.contains(url)) {
    QVector<Range*>& ranges = s_topRanges[url];
    for (int i = 0; i < ranges.count(); ++i) {
      if (range == ranges[i]) {
        delete range;
        ranges[i] = 0;
        return;
      }
    }
  }

  kWarning() << k_funcinfo << "Could not find top range to delete." << endl;
}

#include "editorintegrator.moc"

// kate: indent-width 2;
