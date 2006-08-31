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
#include "kdeveditorintegrator_p.h"

#include <limits.h>

#include <QMutex>
#include <QMutexLocker>

#include <ktexteditor/document.h>
#include <ktexteditor/smartrange.h>
#include <ktexteditor/smartinterface.h>

#include "kdevast.h"
#include "kdevdocumentrange.h"
#include "kdevdocumentrangeobject.h"

#ifndef DUCHAINTEST
#include "kdevproject.h"
#include "kdevcore.h"
#include "kdevpersistenthash.h"
#endif

using namespace KTextEditor;

KDevEditorIntegratorPrivate* KDevEditorIntegrator::s_data = 0;

KDevEditorIntegrator::KDevEditorIntegrator()
  : m_currentDocument(0)
  , m_smart(0)
  , m_currentRange(0)
{
}

KDevEditorIntegrator::~ KDevEditorIntegrator()
{
  if (m_smart) {
    // Unlock the smart interface, if one exists
    m_smart->smartMutex()->unlock();
  }
}

KDevEditorIntegratorPrivate::KDevEditorIntegratorPrivate()
  : mutex(new QMutex)
{
}

KDevEditorIntegratorPrivate::~KDevEditorIntegratorPrivate()
{
  kDebug() << k_funcinfo << endl;
  QHashIterator<KUrl, QVector<KTextEditor::Range*> > it = topRanges;
  while (it.hasNext()) {
    it.next();
    foreach (KTextEditor::Range* range, it.value())
      if (range && range->isSmartRange())
        range->toSmartRange()->removeWatcher(this);
  }

  delete mutex;
}

void KDevEditorIntegrator::addDocument( Document * document )
{
  Q_ASSERT(data()->thread() == document->thread());
  QObject::connect(document, SIGNAL(completed()), data(), SLOT(documentLoaded()));
  QObject::connect(document, SIGNAL(aboutToClose(KTextEditor::Document*)), data(), SLOT(removeDocument(KTextEditor::Document*)));
  QObject::connect(document, SIGNAL(documentUrlChanged(KTextEditor::Document*)), data(), SLOT(documentUrlChanged(KTextEditor::Document*)));
}

void KDevEditorIntegratorPrivate::documentLoaded()
{
  Document* doc = qobject_cast<Document*>(sender());
  if (!doc) {
    kWarning() << k_funcinfo << "Unexpected non-document sender called this slot!" << endl;
    return;
  }

  {
    QMutexLocker lock(mutex);

    documents.insert(doc->url(), doc);
  }

#ifndef DUCHAINTEST
  if (KDevProject* project = KDevCore::activeProject())
    if (KDevPersistentHash* hash = project->persistentHash())
      if (KDevAST* ast = hash->retrieveAST(doc->url()))
        ast->documentLoaded(doc->url());
#endif
}

void KDevEditorIntegratorPrivate::documentUrlChanged(Document* document)
{
  QMutexLocker lock(mutex);

  QMutableHashIterator<KUrl, Document*>  it = documents;
  while (it.hasNext()) {
    it.next();
    if (it.value() == document) {
      if (topRanges.contains(it.key())) {
        kDebug() << k_funcinfo << "Document URL change - found corresponding document" << endl;
        topRanges.insert(document->url(), topRanges.take(it.key()));
      }

      it.remove();
      documents.insert(document->url(), document);
      // TODO trigger reparsing??
      return;
    }
  }

  //kWarning() << k_funcinfo << "Document URL change - couldn't find corresponding document!" << endl;
}

Document * KDevEditorIntegrator::documentForUrl(const KUrl& url)
{
  QMutexLocker lock(data()->mutex);

  if (data()->documents.contains(url))
    return data()->documents[url];

  return 0;
}

bool KDevEditorIntegrator::documentLoaded(KTextEditor::Document* document)
{
  return data()->documents.values().contains(document);
}

void KDevEditorIntegratorPrivate::removeDocument( Document* document )
{
  QMutexLocker lock(mutex);

  // TODO save smart stuff to non-smart cursors and ranges

  documents.remove(document->url());

  foreach (KTextEditor::Range* range, topRanges[document->url()])
    if (range && range->isSmartRange())
      range->toSmartRange()->removeWatcher(this);

  topRanges.remove(document->url());
}

SmartInterface* KDevEditorIntegrator::smart() const
{
  return m_smart;
}

Cursor* KDevEditorIntegrator::createCursor(const Cursor& position)
{
  Cursor* ret = 0;

  if (SmartInterface* iface = smart())
    ret = iface->newSmartCursor(position);

  if (!ret)
    ret = new KDevDocumentCursor(m_currentUrl, position);

  return ret;
}

Document* KDevEditorIntegrator::currentDocument() const
{
  return m_currentDocument;
}

Range* KDevEditorIntegrator::topRange( TopRangeType type )
{
  QMutexLocker lock(data()->mutex);

  if (!data()->topRanges.contains(currentUrl()))
    data()->topRanges.insert(currentUrl(), QVector<Range*>(TopRangeCount));

  // FIXME temporary until we get conversion working
  if (data()->topRanges[currentUrl()][type] && !data()->topRanges[currentUrl()][type]->isSmartRange() && smart()) {
    //delete data()->topRanges[currentUrl()][type];
    data()->topRanges[currentUrl()][type] = 0L;
  }

  if (!data()->topRanges[currentUrl()][type])
    if (currentDocument()) {
      Range* newRange = data()->topRanges[currentUrl()][type] = createRange(currentDocument()->documentRange());
      if (SmartInterface* iface = smart()) {
        Q_ASSERT(newRange->isSmartRange());
        iface->addHighlightToDocument( newRange->toSmartRange(), false );
        newRange->toSmartRange()->addWatcher(data());
      }

    } else {
      // FIXME...
      data()->topRanges[currentUrl()][type] = createRange(Range(0,0, INT_MAX, INT_MAX));
    }

  m_currentRange = data()->topRanges[currentUrl()][type];
  return m_currentRange;
}

void KDevEditorIntegratorPrivate::rangeDeleted(KTextEditor::SmartRange * range)
{
  QMutexLocker lock(mutex);

  QMutableHashIterator<KUrl, QVector<KTextEditor::Range*> > it = topRanges;
  while (it.hasNext()) {
    it.next();
    //kDebug() << k_funcinfo << "Searching for " << range << ", potentials " << it.value().toList() << endl;
    int index = it.value().indexOf(range);
    if (index != -1) {
      it.value()[index] = 0;
      return;
    }
  }

  // Should have found the top level range by now
  kWarning() << k_funcinfo << "Could not find record of top level range " << range << "!" << endl;
}

Range* KDevEditorIntegrator::createRange( const Range & range )
{
  Range* ret;

  if (SmartInterface* iface = smart())
    if (m_currentRange && m_currentRange->isSmartRange())
      ret = iface->newSmartRange(range, m_currentRange->toSmartRange());
    else
      ret = iface->newSmartRange(range);
  else
    ret = new KDevDocumentRange(m_currentUrl, range, m_currentRange);

  m_currentRange = ret;
  return m_currentRange;
}


Range* KDevEditorIntegrator::createRange( const KTextEditor::Cursor& start, const KTextEditor::Cursor& end )
{
  return createRange(Range(start, end));
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
  if (m_smart) {
    // Unlock the smart interface, if one exists
    m_smart->smartMutex()->unlock();
  }

  m_currentUrl = url;
  m_currentDocument = documentForUrl(url);
  m_smart = dynamic_cast<KTextEditor::SmartInterface*>(m_currentDocument);

  if (m_smart) {
    // Lock the smart interface, if one exists
    m_smart->smartMutex()->lock();
  }
}

void KDevEditorIntegrator::releaseTopRange(KTextEditor::Range * range)
{
  QMutexLocker lock(data()->mutex);

  KUrl url = KDevDocumentRangeObject::url(range);

  if (range->isSmartRange())
    range->toSmartRange()->removeWatcher(data());

  if (data()->topRanges.contains(url)) {
    QVector<Range*>& ranges = data()->topRanges[url];
    int index = ranges.indexOf(range);
    if (index != -1) {
      ranges[index] = 0;
      return;
    }
  }

  //kWarning() << k_funcinfo << "Could not find top range to delete." << endl;
}

KDevEditorIntegratorPrivate * KDevEditorIntegrator::data()
{
  if (!s_data)
    s_data = new KDevEditorIntegratorPrivate();

  return s_data;
}

void KDevEditorIntegrator::exitCurrentRange()
{
  if (!m_currentRange)
    return;

  if (m_currentRange->isSmartRange())
    m_currentRange = m_currentRange->toSmartRange()->parentRange();
  else
    m_currentRange = static_cast<KDevDocumentRange*>(m_currentRange)->parentRange();
}

void KDevEditorIntegrator::initialise()
{
  data();
}

#include "kdeveditorintegrator_p.moc"

// kate: indent-width 2;
