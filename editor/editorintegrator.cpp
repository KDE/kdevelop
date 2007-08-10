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

#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>
#include <QStack>

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

  QStack<KTextEditor::Range*> m_currentRangeStack;
  KTextEditor::Range m_newRangeMarker;
};


//Used to access SmartRange and DocumentRange in a unique way within createRange. They have common functionality without sharing an interface
struct UnifiedRange {
  UnifiedRange( Range* range = 0 ) : m_range(range) {
  }

  void setParentRange(Range* range) {
      if(m_range->isSmartRange()) {
        Q_ASSERT( range->isSmartRange() );
        static_cast<SmartRange*>(m_range)->setParentRange( static_cast<SmartRange*>(range) );
      } else {
        Q_ASSERT( dynamic_cast<DocumentRange*>(range) );
        static_cast<DocumentRange*>(m_range)->setParentRange( static_cast<DocumentRange*>(range) );
      }
  }

  Range* parentRange() {
    if( !m_range )
      return 0;
    if(m_range->isSmartRange())
        return static_cast<SmartRange*>(m_range)->parentRange();
    else
        return static_cast<DocumentRange*>(m_range)->parentRange();
  }
    
  Range* range() const {
      return m_range;
  }

  SmartRange* smartRange() {
    if( m_range && m_range->isSmartRange() )
      return static_cast<SmartRange*>(m_range);
    return 0;
  }

  DocumentRange* documentRange() {
    if( !m_range || m_range->isSmartRange() )
      return 0;
    
    return static_cast<DocumentRange*>(m_range);
  }

  operator bool() const {
    return (bool)m_range;
  }
  
  Range* m_range;
};

//A helper to iterator through childs of SmartRange'es and DocumentRange'es in the same way.
struct UnifiedRangeChildIterator {

  //Iterates through the childs of the one parameter that is not zero
  UnifiedRangeChildIterator( UnifiedRange& parent ) : m_parent(parent) {
    if( parent.smartRange() ) {
        m_smartChildRangesEnd = parent.smartRange()->childRanges().end();
        m_smartChildRangesCurrent = parent.smartRange()->childRanges().begin();
    } else {
        m_documentChildRangesEnd = parent.documentRange()->childRanges().end();
        m_documentChildRangesCurrent = parent.documentRange()->childRanges().begin();
    }
  }

  operator bool() const {
    if( m_parent.smartRange() )
      return m_smartChildRangesCurrent != m_smartChildRangesEnd;
    else
      return m_documentChildRangesCurrent != m_documentChildRangesEnd;
  }

  UnifiedRangeChildIterator& operator ++() {
    if( m_parent.smartRange() )
      ++m_smartChildRangesCurrent;
    else
      ++m_documentChildRangesCurrent;
    return *this;
  }

  UnifiedRange unified() const {
    if( m_parent.smartRange() )
      return UnifiedRange( *m_smartChildRangesCurrent );
    else
      return UnifiedRange( *m_documentChildRangesCurrent );
  }

  Range* operator*() {
    if( m_parent.smartRange() )
      return *m_smartChildRangesCurrent;
    else
      return *m_documentChildRangesCurrent;
  }

  QList<SmartRange*>::const_iterator m_smartChildRangesEnd, m_smartChildRangesCurrent;
  QList<DocumentRange*>::const_iterator m_documentChildRangesEnd, m_documentChildRangesCurrent;

  UnifiedRange& m_parent;
};

K_GLOBAL_STATIC( EditorIntegratorStatic, s_data)

EditorIntegrator::EditorIntegrator()
: d(new EditorIntegratorPrivate)
{
  d->m_currentDocument = 0;
  d->m_smart = 0;
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

Range* EditorIntegrator::topRange( TopRangeType /*type*/)
{
  QMutexLocker lock(data()->mutex);

  if (!data()->topRanges.contains(currentUrl()))
    data()->topRanges.insert(currentUrl(), QVector<Range*>(TopRangeCount));
  
  Range* newRange = 0;
  if (currentDocument()) {
    newRange = createRange(currentDocument()->documentRange(), KTextEditor::SmartRange::ExpandLeft | KTextEditor::SmartRange::ExpandRight);
    if (SmartInterface* iface = smart()) {
      QMutexLocker lock(iface->smartMutex());
      Q_ASSERT(newRange->isSmartRange());
      iface->addHighlightToDocument( newRange->toSmartRange(), false );
    }
   } else {
     // FIXME...
     newRange = createRange(Range(0,0, INT_MAX, INT_MAX));
   }
  
  d->m_currentRangeStack << newRange;
  return d->m_currentRangeStack.top();
}

Range* EditorIntegrator::createRange( const KTextEditor::Range & range, KTextEditor::SmartRange::InsertBehaviors insertBehavior )
{
  SmartInterface* iface = smart();
  
  QMutexLocker lock(iface ? iface->smartMutex() : 0);

  UnifiedRange currentRange;

  if( !d->m_currentRangeStack.isEmpty() )
    currentRange = UnifiedRange( d->m_currentRangeStack.top() );
  
  UnifiedRange ret( iface ? (Range*)iface->newSmartRange(range, 0, insertBehavior) : (Range*)new DocumentRange(d->m_currentUrl, range) );

  if (!d->m_currentRangeStack.isEmpty()) {
    ///@todo Evaluate whether these special-cases code should go into kdelibs
    ///@todo the whole thing isn't needed at all for DocumentRange's, because those do not do any clipping. So maybe only limit this to smart-ranges.

    ///Special-case 1: The range we are creating completely contains n ranges that are slaves of currentRange

    for( UnifiedRangeChildIterator it(currentRange); it; ++it ) {
        if( (*it)->end() > range.end() )
            break;
        if( (*it)->start() < range.start() )
            continue;
        //Now the condition (*it)->start() >= range.start() && (*it)->end() <= range.end() is fulfilled(range is contained)
        it.unified().setParentRange(ret.range()); //Move the range into our one
    }

    ///Special-case 2: The range we are creating is completely contained in a child-range of the current at any deeper level. Replace currentRange with it.
    bool found = true;
    while(found) {
      found = false;
      for( UnifiedRangeChildIterator it(currentRange); it; ++it ) {
          if( range.start() < (*it)->start() )
              break;
          if( range.end() > (*it)->end() )
              continue;
          if( **it == range )
            continue; //This case should already have been handled by the test above
          //Now the condition range.start() >= (*it)->start() && range.end() <= (*it)->end()  is fulfilled(range is contained)
          currentRange = *it; //Move down to the range that contains range
      }
    }

    ///Normal case:
    ret.setParentRange( currentRange.range() );

    if( !d->m_currentRangeStack.isEmpty() )
      Q_ASSERT(d->m_currentRangeStack.top()->start() <=  range.start() && d->m_currentRangeStack.top()->end() >= range.end());

  }

  d->m_currentRangeStack << ret.range();
  return ret.range();
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
   d->m_currentRangeStack << range;
 }

Range* EditorIntegrator::currentRange( ) const
{
  if( !d->m_currentRangeStack.isEmpty() )
    return d->m_currentRangeStack.top();
  else
    return 0;
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

  //kWarning() << k_funcinfo << "Could not find top range to delete." ;
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
  if (d->m_currentRangeStack.isEmpty())
    return;

  d->m_currentRangeStack.pop();
}

ModificationRevision EditorIntegrator::modificationRevision(const KUrl& url) {
  ///@todo add a cache, use the old code from Cpp::EnvironmentManager
  ///@todo support non-local files

  QFileInfo fileInfo( url.toLocalFile() );
  
  ModificationRevision ret(fileInfo.lastModified());

  KTextEditor::Document* doc = documentForUrl(url);
  if( doc ) {
    KTextEditor::SmartInterface* smart =   dynamic_cast<KTextEditor::SmartInterface*>(doc);
    if( smart )
      ret.revision = smart->currentRevision();
  }
  
  return ret;
}

ModificationRevision::ModificationRevision( const QDateTime& modTime , int revision_ ) : modificationTime(modTime), revision(revision_) {
}

bool ModificationRevision::operator <( const ModificationRevision& rhs ) const {
  return modificationTime < rhs.modificationTime || (modificationTime == rhs.modificationTime && revision < rhs.revision);
}

bool ModificationRevision::operator ==( const ModificationRevision& rhs ) const {
  return modificationTime == rhs.modificationTime && revision == rhs.revision;
}

bool ModificationRevision::operator !=( const ModificationRevision& rhs ) const {
  return modificationTime != rhs.modificationTime && revision != rhs.revision;
}

QString ModificationRevision::toString() const {
  return QString("%1 (rev %2)").arg(modificationTime.time().toString()).arg(revision);
}

kdbgstream& operator<< (kdbgstream& s, const ModificationRevision& rev) {
  s << rev.toString();
  return s;
}

}


