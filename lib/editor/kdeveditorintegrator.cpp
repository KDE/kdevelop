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

#include "kdeveditorintegrator.h"

#include <limits.h>

#include <QHash>

#include <ktexteditor/document.h>
#include <ktexteditor/smartrange.h>
#include <ktexteditor/smartinterface.h>

#include "kdevast.h"
#include "kdevdocumentrange.h"
#include "kdevdocumentrangeobject.h"

using namespace KTextEditor;

QHash<KUrl, Document*> KDevEditorIntegrator::s_documents;
QHash<KUrl, QVector<Range*> > KDevEditorIntegrator::s_topRanges;

KDevEditorIntegrator::KDevEditorIntegrator()
  : m_currentDocument(0)
  , m_currentRange(0)
{
}

void KDevEditorIntegrator::addDocument( Document * document )
{
  s_documents.insert(document->url(), document);
}

SmartInterface* KDevEditorIntegrator::smart(KTextEditor::Document* document) const
{
  return dynamic_cast<SmartInterface*>(document ? document : currentDocument());
}

Cursor* KDevEditorIntegrator::createCursor(const KDevDocumentCursor& position)
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
    ret = new KDevDocumentCursor(position);

  return ret;
}

Document* KDevEditorIntegrator::currentDocument() const
{
  return m_currentDocument;
}

Range* KDevEditorIntegrator::topRange( TopRangeType type )
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

Range* KDevEditorIntegrator::createRange( const Range & range, KTextEditor::Document* document )
{
  Range* ret;

  if (SmartInterface* iface = smart(document))
    if (m_currentRange && m_currentRange->isSmartRange())
      ret = iface->newSmartRange(range, static_cast<SmartRange*>(m_currentRange));
    else
      ret = iface->newSmartRange(range);
  else
    ret = new KDevDocumentRange(m_currentUrl, range);

  m_currentRange = ret;
  return m_currentRange;
}


Range* KDevEditorIntegrator::createRange( const KDevDocumentCursor& start, const KDevDocumentCursor& end )
{
  if (start.document() != end.document()) {
    kWarning() << k_funcinfo << "Start: " << start << ", End: " << end << ", documents " << start.document() << " != " << end.document() << endl;
    // FIXME difficult problem
    createRange(Range(start, start), documentForUrl(start.document()));
  }

  return createRange(Range(start, end), documentForUrl(start.document()));
}

Range* KDevEditorIntegrator::createRange()
{
  return createRange(m_newRangeMarker);
}

void KDevEditorIntegrator::setNewRange(const Range& range)
{
  m_newRangeMarker = range;
}

void KDevEditorIntegrator::setNewEnd( const Cursor & position )
{
  m_newRangeMarker.end() = position;
}

void KDevEditorIntegrator::setNewStart( const Cursor & position )
{
  m_newRangeMarker.start() = position;
}

Document * KDevEditorIntegrator::documentForUrl(const KUrl& url)
{
  foreach (Document* d, s_documents)
    if (d->url() == url)
      return d;

  return 0;
}

void KDevEditorIntegrator::removeDocument( Document * document )
{
  s_documents.remove(document->url());
  s_topRanges.remove(document->url());
}

void KDevEditorIntegrator::setCurrentRange( Range* range )
{
  m_currentRange = range;
}

Range* KDevEditorIntegrator::currentRange( ) const
{
  return m_currentRange;
}

const KUrl& KDevEditorIntegrator::currentUrl() const
{
  return m_currentUrl;
}

void KDevEditorIntegrator::setCurrentUrl(const KUrl& url)
{
  m_currentUrl = url;
  m_currentDocument = documentForUrl(url);
}

void KDevEditorIntegrator::deleteTopRange(KTextEditor::Range * range)
{
  KUrl url = KDevDocumentRangeObject::url(range);

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

#include "kdeveditorintegrator.moc"

// kate: indent-width 2;
