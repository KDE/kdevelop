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

  QStack<KTextEditor::SmartRange*> m_currentRangeStack;
  KTextEditor::Range m_newRangeMarker;
  template<class RangeType>
  SmartRange* createRange( const KTextEditor::Range & range, KTextEditor::SmartRange::InsertBehaviors insertBehavior );
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
  delete d;
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

SmartCursor* EditorIntegrator::createCursor(const KTextEditor::Cursor& position)
{
  if (SmartInterface* iface = smart()) {
    QMutexLocker lock(iface->smartMutex());
    return iface->newSmartCursor(position);
  }else{
      return 0;
  }
}

Document* EditorIntegrator::currentDocument() const
{
  return d->m_currentDocument;
}

SmartRange* EditorIntegrator::topRange( TopRangeType /*type*/)
{
  QMutexLocker lock(data()->mutex);

  if(!smart())
      return 0;
  
  Q_ASSERT(d->m_currentRangeStack.isEmpty());
  
  SmartRange* newRange = 0;
  if (currentDocument()) {
    newRange = createRange(currentDocument()->documentRange(), KTextEditor::SmartRange::ExpandLeft | KTextEditor::SmartRange::ExpandRight);
    Q_ASSERT(!dynamic_cast<KTextEditor::SmartRange*>(newRange) || static_cast<KTextEditor::SmartRange*>(newRange)->parentRange() == 0 || static_cast<KTextEditor::SmartRange*>(newRange)->childRanges().count() == 0);
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
SmartRange* EditorIntegratorPrivate::createRange<SmartRange>( const KTextEditor::Range & range, KTextEditor::SmartRange::InsertBehaviors insertBehavior )
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

SmartRange* EditorIntegrator::createRange( const KTextEditor::Range & range, KTextEditor::SmartRange::InsertBehaviors insertBehavior )
{
  if(!smart())
      return 0;
  SmartInterface* iface = smart();
  
  QMutexLocker lock(iface ? iface->smartMutex() : 0);

  return d->createRange<SmartRange>(range, insertBehavior);
}


SmartRange* EditorIntegrator::createRange( const KTextEditor::Cursor& start, const KTextEditor::Cursor& end )
{
  return createRange(Range(start, end));
}

SmartRange* EditorIntegrator::createRange()
{
  return createRange(d->m_newRangeMarker);
}

void EditorIntegrator::setNewRange(const KTextEditor::SmartRange& range)
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

 void EditorIntegrator::setCurrentRange( KTextEditor::SmartRange* range )
{
   if(!range)
     return;
   d->m_currentRangeStack << range;
 }

SmartRange* EditorIntegrator::currentRange( ) const
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

void EditorIntegrator::releaseTopRange(KTextEditor::SmartRange * range)
{
  QMutexLocker lock(data()->mutex);

  if (range->isSmartRange())
    range->toSmartRange()->removeWatcher(data());

  delete range;
  //kWarning() << "Could not find top range to delete." ;
}


void EditorIntegrator::releaseRange(KTextEditor::SmartRange* range)
{
  SmartInterface* iface = dynamic_cast<SmartInterface*>(range->toSmartRange()->document());
  QMutexLocker lock(iface ? iface->smartMutex() : 0);
  
  if( range->parentRange() )
  {
    SmartRange* oldParent = range->parentRange();
    range->setParentRange(0);
    QList<SmartRange*> childRanges = range->childRanges();
    foreach( SmartRange* range, childRanges )
      range->setParentRange(oldParent);
  }
  delete range;
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
