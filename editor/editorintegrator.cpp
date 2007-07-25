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

#include "editorintegrator.h"
#include "editorintegratorstatic.h"

#include <limits.h>

#include <QMutex>
#include <QMutexLocker>

#include <kglobal.h>

#include <ktexteditor/document.h>
#include <ktexteditor/smartinterface.h>

#include "documentrange.h"
#include "documentrangeobject.h"

using namespace KTextEditor;

namespace KDevelop
{

class EditorIntegratorPrivate
{
public:
  KUrl m_currentUrl;
  KTextEditor::Document* m_currentDocument;
  KTextEditor::SmartInterface* m_smart;
  KTextEditor::Range* m_currentRange;
  KTextEditor::Range m_newRangeMarker;
};

K_GLOBAL_STATIC( EditorIntegratorStatic, s_data)

EditorIntegrator::EditorIntegrator()
: d(new EditorIntegratorPrivate)
{
  d->m_currentDocument = 0;
  d->m_smart = 0;
  d->m_currentRange = 0;
}

EditorIntegrator::~ EditorIntegrator()
{
}

void EditorIntegrator::addDocument( KTextEditor::Document * document )
{
  Q_ASSERT(data()->thread() == document->thread());
  QObject::connect(document, SIGNAL(completed()), data(), SLOT(documentLoaded()));
  QObject::connect(document, SIGNAL(aboutToClose(KTextEditor::Document*)), data(), SLOT(removeDocument(KTextEditor::Document*)));
  QObject::connect(document, SIGNAL(documentUrlChanged(KTextEditor::Document*)), data(), SLOT(documentUrlChanged(KTextEditor::Document*)));
}



Document * EditorIntegrator::documentForUrl(const KUrl& url)
{
  QMutexLocker lock(data()->mutex);

  if (data()->documents.contains(url))
    return data()->documents[url];

  return 0;
}

bool EditorIntegrator::documentLoaded(KTextEditor::Document* document)
{
  return data()->documents.values().contains(document);
}

SmartInterface* EditorIntegrator::smart() const
{
  return d->m_smart;
}

Cursor* EditorIntegrator::createCursor(const KTextEditor::Cursor& position)
{
  Cursor* ret = 0;

  if (SmartInterface* iface = smart()) {
    QMutexLocker lock(iface->smartMutex());
    ret = iface->newSmartCursor(position);
  }

  if (!ret)
    ret = new DocumentCursor(d->m_currentUrl, position);

  return ret;
}

Document* EditorIntegrator::currentDocument() const
{
  return d->m_currentDocument;
}

Range* EditorIntegrator::topRange( TopRangeType type )
{
  QMutexLocker lock(data()->mutex);

  if (!data()->topRanges.contains(currentUrl()))
    data()->topRanges.insert(currentUrl(), QVector<Range*>(TopRangeCount));
  
  Range* newRange = 0;
  if (currentDocument()) {
    newRange = createRange(currentDocument()->documentRange());
    if (SmartInterface* iface = smart()) {
      QMutexLocker lock(iface->smartMutex());
      Q_ASSERT(newRange->isSmartRange());
      iface->addHighlightToDocument( newRange->toSmartRange(), false );
    }
   } else {
     // FIXME...
     newRange = createRange(Range(0,0, INT_MAX, INT_MAX));
   }
  
  d->m_currentRange = newRange;
  return d->m_currentRange;
}

Range* EditorIntegrator::createRange( const KTextEditor::Range & range )
{
  Range* ret;

  if (SmartInterface* iface = smart()) {
    QMutexLocker lock(iface->smartMutex());

    if (d->m_currentRange && d->m_currentRange->isSmartRange())
      ret = iface->newSmartRange(range, d->m_currentRange->toSmartRange());
    else
      ret = iface->newSmartRange(range);

  } else {
    ret = new DocumentRange(d->m_currentUrl, range, d->m_currentRange);
  }

  d->m_currentRange = ret;
  return d->m_currentRange;
}


Range* EditorIntegrator::createRange( const KTextEditor::Cursor& start, const KTextEditor::Cursor& end )
{
  return createRange(Range(start, end));
}

Range* EditorIntegrator::createRange()
{
  return createRange(d->m_newRangeMarker);
}

void EditorIntegrator::setNewRange(const KTextEditor::Range& range)
{
  d->m_newRangeMarker = range;
}

void EditorIntegrator::setNewEnd( const KTextEditor::Cursor & position )
{
  d->m_newRangeMarker.end() = position;
}

void EditorIntegrator::setNewStart( const KTextEditor::Cursor & position )
{
  d->m_newRangeMarker.start() = position;
}

void EditorIntegrator::setCurrentRange( KTextEditor::Range* range )
{
  d->m_currentRange = range;
}

Range* EditorIntegrator::currentRange( ) const
{
  return d->m_currentRange;
}

KUrl EditorIntegrator::currentUrl() const
{
  return d->m_currentUrl;
}

void EditorIntegrator::setCurrentUrl(const KUrl& url)
{
  d->m_currentUrl = url;
  d->m_currentDocument = documentForUrl(url);
  d->m_smart = dynamic_cast<KTextEditor::SmartInterface*>(d->m_currentDocument);
}

void EditorIntegrator::releaseTopRange(KTextEditor::Range * range)
{
  QMutexLocker lock(data()->mutex);

  KUrl url = DocumentRangeObject::url(range);

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

void EditorIntegrator::releaseRange(KTextEditor::Range* range)
{
  if (range) {
    if (range->isSmartRange()) {
      if (SmartInterface* iface = dynamic_cast<SmartInterface*>(range->toSmartRange()->document())) {
        QMutexLocker lock(iface->smartMutex());
        delete range;
      } else {
        delete range;
      }
    } else {
      delete range;
    }
  }
}

KDevelop::EditorIntegratorStatic * EditorIntegrator::data()
{
  return s_data;
}

void EditorIntegrator::exitCurrentRange()
{
  if (!d->m_currentRange)
    return;

  if (d->m_currentRange->isSmartRange())
    d->m_currentRange = d->m_currentRange->toSmartRange()->parentRange();
  else
    d->m_currentRange = static_cast<DocumentRange*>(d->m_currentRange)->parentRange();
}

}

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
