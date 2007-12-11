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
#include "hashedstring.h"

using namespace KTextEditor;

namespace KDevelop
{

class EditorIntegratorPrivate
{
public:
  HashedString m_currentUrl;
  KTextEditor::Document* m_currentDocument;
  KTextEditor::SmartInterface* m_smart;

  QStack<KTextEditor::Range*> m_currentRangeStack;
  KTextEditor::Range m_newRangeMarker;
  template<class RangeType>
  Range* createRange( const KTextEditor::Range & range, KTextEditor::SmartRange::InsertBehaviors insertBehavior );
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

Document * EditorIntegrator::documentForUrl(const HashedString& url)
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

template<>
Range* EditorIntegratorPrivate::createRange<SmartRange>( const KTextEditor::Range & range, KTextEditor::SmartRange::InsertBehaviors insertBehavior )
{
  SmartInterface* iface = m_smart;
  
  QMutexLocker lock(iface ? iface->smartMutex() : 0);

  SmartRange* currentRange;

  if( !m_currentRangeStack.isEmpty() ) {
    currentRange = dynamic_cast<SmartRange*>(m_currentRangeStack.top());
    Q_ASSERT(currentRange);
  }

  SmartRange* ret = m_smart->newSmartRange(range, 0, insertBehavior);

  if (!m_currentRangeStack.isEmpty()) {

    ///Special-case 2: The range we are creating is completely contained in a child-range of the current at any deeper level. Replace currentRange with it.
    bool found = true;
    while(found) {
      found = false;
      foreach( SmartRange* other, currentRange->childRanges() ) {
      //for( QList<SmartRange*>::const_iterator it = currentRange->childRanges().begin(); it != currentRange->childRanges().end(); ++it ) {
          if( range.start() < (other)->start() )
              break;
          if( range.end() > (other)->end() )
              continue;
          if( *other == range )
            continue;
          //Now the condition range.start() >= (other)->start() && range.end() <= (other)->end()  is fulfilled(range is contained)
          currentRange = other; //Move down to the range that contains range
          found = true;
      }
    }

    ///Special-case 1: The range we are creating completely contains n ranges that are slaves of currentRange
    QList<SmartRange*> importList;
    
    foreach( SmartRange* other, currentRange->childRanges() ) {
    //for( QList<SmartRange*>::const_iterator it = currentRange->childRanges().begin(); it != currentRange->childRanges().end(); ++it ) {
        if( (other)->end() > range.end() )
            break;
        if( (other)->start() < range.start() )
            continue;
        //Now the condition (other)->start() >= range.start() && (other)->end() <= range.end() is fulfilled(range is contained)

        importList << other; //Delay the setParent because else the list we iterate over gets corrupted
    }

    for( QList<SmartRange*>::const_iterator it = importList.begin(); it != importList.end(); ++it ) {
      Q_ASSERT((*it)->parentRange()); //We must never import a top-range, top-ranges must stay top-ranges
      (*it)->setParentRange(ret);
    }

    ///Normal case:
    ret->setParentRange( currentRange );
  }

  m_currentRangeStack << ret;
  return ret;
}


template<>
Range* EditorIntegratorPrivate::createRange<DocumentRange>( const KTextEditor::Range & range, KTextEditor::SmartRange::InsertBehaviors /*insertBehavior*/ )
{
  DocumentRange* currentRange = 0;

  if( !m_currentRangeStack.isEmpty() ) {
    currentRange = dynamic_cast<DocumentRange*>(m_currentRangeStack.top());
    Q_ASSERT(currentRange);
  }

  DocumentRange* ret = new DocumentRange(m_currentUrl, range);

  if (currentRange)
    ret->setParentRange( currentRange );

  m_currentRangeStack << ret;
  return ret;
}

Range* EditorIntegrator::createRange( const KTextEditor::Range & range, KTextEditor::SmartRange::InsertBehaviors insertBehavior )
{
  SmartInterface* iface = smart();
  
  QMutexLocker lock(iface ? iface->smartMutex() : 0);

  if( iface && ( d->m_currentRangeStack.isEmpty() || d->m_currentRangeStack.top()->isSmartRange() ) )
    return d->createRange<SmartRange>(range, insertBehavior);
  else {
    if( !d->m_currentRangeStack.isEmpty() && d->m_currentRangeStack.top()->isSmartRange() ) {
      //This should never happen
      kDebug() << "EditorIntegrator: WARNING: Creating a document-range as slave of a smart-range";
    }
    return d->createRange<DocumentRange>(range, insertBehavior);
  }
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

HashedString EditorIntegrator::currentUrl() const
{
  return d->m_currentUrl;
}

void EditorIntegrator::setCurrentUrl(const HashedString& url)
{
  d->m_currentUrl = url;
  d->m_currentDocument = documentForUrl(url);
  d->m_smart = dynamic_cast<KTextEditor::SmartInterface*>(d->m_currentDocument);
}

void EditorIntegrator::releaseTopRange(KTextEditor::Range * range)
{
  QMutexLocker lock(data()->mutex);

  HashedString url = DocumentRangeObject::url(range);

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

  //kWarning() << "Could not find top range to delete." ;
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

ModificationRevision EditorIntegrator::modificationRevision(const HashedString& url) {
  ///@todo add a cache, use the old code from Cpp::EnvironmentManager
  ///@todo support non-local files

  
  QString localFile = url.str();
  if( localFile.startsWith("file://") )
    localFile = localFile.mid(7); //This is much faster then first constructing KUrl
  else
    localFile = KUrl(url.str()).toLocalFile();

  QFileInfo fileInfo( localFile );
  
  
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

QObject * EditorIntegrator::notifier()
{
    return data();
}

}
