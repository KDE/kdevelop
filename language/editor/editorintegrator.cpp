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
#include <ktexteditor/rangefeedback.h>

#include "documentrangeobject.h"
#include "hashedstring.h"

using namespace KTextEditor;

namespace KDevelop
{

class EditorIntegratorPrivate
{
public:
  ~EditorIntegratorPrivate()
  {
    while (!m_currentRangeStack.isEmpty())
      exitCurrentRange();
  }

  IndexedString m_currentUrl;

  bool m_useSmart;
  /// The following two pointers may only be accessed with the editor integrator static mutex held
  KTextEditor::Document* m_currentDocument;
  KTextEditor::SmartInterface* m_smart;

  template<class RangeType>
  SmartRange* createRange( const LockedSmartInterface& iface, const KTextEditor::Range & range, KTextEditor::SmartRange::InsertBehaviors insertBehavior );

  // Integrity protected by the smart lock
  void enterCurrentRange(KTextEditor::SmartRange* range);
  void exitCurrentRange();
  QStack<KTextEditor::SmartRange*> m_currentRangeStack;
};

K_GLOBAL_STATIC( EditorIntegratorStatic, s_data)

EditorIntegrator::EditorIntegrator()
: d(new EditorIntegratorPrivate)
{
  d->m_currentDocument = 0;
  d->m_smart = 0;
  d->m_useSmart = true;
}

EditorIntegrator::~ EditorIntegrator()
{
  {
    LockedSmartInterface locked(smart());

    QMutexLocker lock(data()->mutex);

    if (d->m_currentDocument)
      data()->editorIntegrators.remove(d->m_currentDocument, this);
  }

  delete d;
}

void EditorIntegrator::insertLoadedDocument(KTextEditor::Document* document) {
    data()->insertLoadedDocument(document);
}

void EditorIntegrator::addDocument( KTextEditor::Document * document )
{
  Q_ASSERT(data()->thread() == document->thread());
  QObject::connect(document, SIGNAL(aboutToClose(KTextEditor::Document*)), data(), SLOT(removeDocument(KTextEditor::Document*)));
  QObject::connect(document, SIGNAL(aboutToReload(KTextEditor::Document*)), data(), SLOT(reloadDocument(KTextEditor::Document*)));
  QObject::connect(document, SIGNAL(documentUrlChanged(KTextEditor::Document*)), data(), SLOT(documentUrlChanged(KTextEditor::Document*)));

  data()->insertLoadedDocument(document);
}

void EditorIntegrator::removeDocument( KTextEditor::Document * document )
{
  QObject::disconnect(document, SIGNAL(aboutToClose(KTextEditor::Document*)), data(), SLOT(removeDocument(KTextEditor::Document*)));
  QObject::disconnect(document, SIGNAL(aboutToReload(KTextEditor::Document*)), data(), SLOT(reloadDocument(KTextEditor::Document*)));
  QObject::disconnect(document, SIGNAL(documentUrlChanged(KTextEditor::Document*)), data(), SLOT(documentUrlChanged(KTextEditor::Document*)));

  data()->removeDocument(document);
}

Document * EditorIntegrator::documentForUrl(const HashedString& url)
{
  return documentForUrl(IndexedString(url.str()));
}

Document * EditorIntegrator::documentForUrl(const IndexedString& url)
{
  QMutexLocker lock(data()->mutex);

  if (data()->documents.contains(url))
    return data()->documents[url];

  return 0;
}

LockedSmartInterface EditorIntegrator::smart() const
{
  SmartInterface* iface = 0;
  KTextEditor::Document* doc = 0;
  {
    ///@todo This opens a very short window where the document could eventually be deleted
    ///      But it prevents a much more probable deadlock.
    QMutexLocker lock(data()->mutex);
    iface = d->m_smart;
    doc = d->m_currentDocument;
  }
  
  return LockedSmartInterface(iface, doc);
}

LockedSmartInterface EditorIntegrator::smart(const KUrl& url)
{
  SmartInterface* iface = 0;
  KTextEditor::Document* doc = 0;

  IndexedString indexedUrl(url.pathOrUrl());
  if (data()->documents.contains(indexedUrl)) {
    KTextEditor::Document* doc = data()->documents[indexedUrl];
    return LockedSmartInterface(dynamic_cast<KTextEditor::SmartInterface*>(doc), doc);
  }

  return LockedSmartInterface(iface, doc);
}

SmartCursor* EditorIntegrator::createCursor(const LockedSmartInterface& iface, const KTextEditor::Cursor& position)
{
  if (!iface)
    return 0;

  return iface->newSmartCursor(position);
}

template<>
SmartRange* EditorIntegratorPrivate::createRange<SmartRange>( const LockedSmartInterface& iface, const KTextEditor::Range & range, KTextEditor::SmartRange::InsertBehaviors insertBehavior )
{
  SmartRange* ret = iface->newSmartRange(range, 0, insertBehavior);
  Q_ASSERT(ret->insertBehavior() == insertBehavior);

  KTextEditor::Cursor rangeStart = ret->start();
  KTextEditor::Cursor rangeEnd = ret->end();

  if (!m_currentRangeStack.isEmpty()) {

    SmartRange* currentRange = dynamic_cast<SmartRange*>(m_currentRangeStack.top());
    Q_ASSERT(currentRange);
  #ifdef HACK_RANGE_CONSISTENCY

    QList<SmartRange*> importList;

    bool found = true;
    while(found && currentRange->childRanges().size()) {
      found = false;


      //We use a binary search hear to find contained ranges and containing ranges in one run
      int count = currentRange->childRanges().size();
      int leftBound = 0; //Left bound of the search, inclusive
      int rightBound = count; //Right bound of the search, exclusive
      int current = count/2;
      const QList<KTextEditor::SmartRange*>& list(currentRange->childRanges());

      //Find child ranges that are contained in range, or a child range that range is contained in, and build a usable structure
      ///@todo eventually change kate so neighbours don't move each other around, then we won't need all this hacking, and will get a better result

      while(true) {
        if(list[current]->end() <= rangeStart) {
            //Our range is behind list[current], increase current, move to the point between current and rightBound
            leftBound = current+1;
            int nextCurrent =  current + (rightBound-current)/2;
            if(nextCurrent == current)
                ++nextCurrent;
            if(nextCurrent >= rightBound)
                break; //Found nothing
            current = nextCurrent;
        }else if(list[current]->start() >= rangeEnd) {
            //Our range is before list[current], decrease current, move to point between current and leftBound
            rightBound = current-1;
            int nextCurrent =  current - (current-leftBound)/2;
            if(nextCurrent == current)
                --nextCurrent;
            if(nextCurrent < leftBound)
                break; //Found nothing
            current = nextCurrent;
        }else{
            //We're contained in the range
            //rangeEnd > rangeStart, rangeStart < currentEnd, rangeEnd > currentStart

            //We can have 3 possible cases now:
            //- range is contained in list[current]
            //- list[current] is contained in range
            //- list[current] and range intersect in a not fixable way

            if(rangeStart >= list[current]->start() && rangeEnd <= list[current]->start()) {
                found  = true;
                currentRange = list[current];
                importList.clear();
            } else {
                //Range contains list[current], or intersects list[current]-> Find all neighbours of list[current] that are also contained
                for(int a = current-1; a >= 0; --a) {
                    if(list[a]->start() >= rangeStart && list[a]->end() <= rangeEnd)
                        importList << list[a]; //The range is contained, add it to the import list
                    else
                        break;
                }
                if(rangeStart <= list[current]->start() && rangeEnd >= list[current]->end())
                    importList << list[current]; //The current item is contained in range

                for(int a = current+1; a < count; ++a) {
                    if(list[a]->start() >= rangeStart && list[a]->end() <= rangeEnd)
                        importList << list[a]; //The range is contained, add it to the import list
                    else
                        break;
                }
                ///If importList is empty now, we definitely have an intersection problem
            }

            break;
        }
      }
    }

    for( QList<SmartRange*>::const_iterator it = importList.begin(); it != importList.end(); ++it ) {
      Q_ASSERT((*it)->parentRange()); //We must never import a top-range, top-ranges must stay top-ranges
      Q_ASSERT(ret->contains(**it));
      (*it)->setParentRange(ret);
    }
    #endif

    ///Normal case:
    ret->setParentRange( currentRange );
  }

  Q_ASSERT(ret->end() == rangeEnd && ret->start() == rangeStart);
  enterCurrentRange(ret);
  return ret;
}

SmartRange* EditorIntegrator::topRange(const LockedSmartInterface& iface, TopRangeType /*type*/)
{
  if(!iface)
      return 0;

  Q_ASSERT(d->m_currentRangeStack.isEmpty());

  SmartRange* newRange = d->createRange<SmartRange>(iface, iface.currentDocument()->documentRange(), KTextEditor::SmartRange::ExpandLeft | KTextEditor::SmartRange::ExpandRight);
  Q_ASSERT(!dynamic_cast<KTextEditor::SmartRange*>(newRange) || static_cast<KTextEditor::SmartRange*>(newRange)->parentRange() == 0 || static_cast<KTextEditor::SmartRange*>(newRange)->childRanges().count() == 0);
  Q_ASSERT(newRange->isSmartRange());
  iface->addHighlightToDocument( newRange->toSmartRange(), false );

  Q_ASSERT(d->m_currentRangeStack.count() == 1);
  return d->m_currentRangeStack.top();
}

SmartRange* EditorIntegrator::createRange(const LockedSmartInterface& iface, const KTextEditor::Range & range, KTextEditor::SmartRange::InsertBehaviors insertBehavior )
{
  if (!iface)
    return 0;

  return d->createRange<SmartRange>(iface, range, insertBehavior);
}


SmartRange* EditorIntegrator::createRange(const LockedSmartInterface& iface, const KTextEditor::Cursor& start, const KTextEditor::Cursor& end )
{
  return createRange(iface, Range(start, end));
}

 void EditorIntegrator::setCurrentRange(const LockedSmartInterface& iface, KTextEditor::SmartRange* range)
{
  Q_UNUSED(iface);

   if(!range)
     return;

   d->enterCurrentRange( range );
 }

SmartRange* EditorIntegrator::currentRange(const LockedSmartInterface& iface) const
{
  Q_UNUSED(iface);

  if( !d->m_currentRangeStack.isEmpty() )
    return d->m_currentRangeStack.top();
  else
    return 0;
}

IndexedString EditorIntegrator::currentUrl() const
{
  return d->m_currentUrl;
}

void EditorIntegrator::setCurrentUrl(const IndexedString& url, bool useSmart)
{
  QMutexLocker lock(data()->mutex);
  d->m_useSmart = useSmart;

  if (d->m_currentDocument) {
    data()->editorIntegrators.remove(d->m_currentDocument, this);
  }

  d->m_currentUrl = url;

  IndexedString indexedUrl(url.str());

  if (useSmart && data()->documents.contains(indexedUrl)) {
    d->m_currentDocument = data()->documents[indexedUrl];
    data()->editorIntegrators.insert(d->m_currentDocument, this);

    d->m_smart = dynamic_cast<KTextEditor::SmartInterface*>(d->m_currentDocument);
  }else{
    d->m_smart = 0;
  }
}

void EditorIntegrator::clearCurrentDocument()
{
  // The editor integrator static mutex is locked, because this is only called with it locked
  d->m_currentDocument = 0;
  d->m_smart = 0;
  d->m_currentRangeStack.clear();
}

void EditorIntegrator::releaseTopRange(KTextEditor::SmartRange * range)
{
  delete range;
}

void EditorIntegrator::releaseRange(KTextEditor::SmartRange* range)
{
  // Smart lock must already be held
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

void EditorIntegrator::exitCurrentRange(const LockedSmartInterface& iface)
{
  Q_UNUSED(iface);

  if (d->m_currentRangeStack.isEmpty())
    return;

  d->exitCurrentRange();
}

void EditorIntegratorPrivate::exitCurrentRange()
{
  m_currentRangeStack.pop();
}

void EditorIntegratorPrivate::enterCurrentRange(KTextEditor::SmartRange* range)
{
  m_currentRangeStack.push(range);
}

int EditorIntegrator::rangeStackSize(const LockedSmartInterface& iface) const
{
    Q_UNUSED(iface);
    return d->m_currentRangeStack.size();
}

QObject * EditorIntegrator::notifier()
{
    return data();
}

void EditorIntegrator::adjustRangeTo(const LockedSmartInterface& iface, const SimpleRange& fromRange)
{
  if (!iface)
    return;

  if (currentRange(iface)) {
    currentRange(iface)->setRange(iface->translateFromRevision(fromRange.textRange()));
  }
}

SimpleRange EditorIntegrator::translate(const LockedSmartInterface& iface, const SimpleRange& fromRange) const
{
  if (iface)
    return iface->translateFromRevision(fromRange.textRange());

  return fromRange;
}

class LockedSmartInterfacePrivate
{
  public:
    KTextEditor::SmartInterface* iface;
    KTextEditor::Document* doc;
    int ref;
};

LockedSmartInterface::LockedSmartInterface(KTextEditor::SmartInterface* iface, KTextEditor::Document* doc)
  : d(new LockedSmartInterfacePrivate)
{
  d->iface = iface;
  d->doc = doc;
  d->ref = 1;

  if (d->iface)
    d->iface->smartMutex()->lock();
}

LockedSmartInterface::LockedSmartInterface(const LockedSmartInterface& lock)
  : d(lock.d)
{
  ++d->ref;
}

LockedSmartInterface::~LockedSmartInterface()
{
  --d->ref;
  if (!d->ref) {
    unlock();
    delete d;
  }
}

void LockedSmartInterface::unlock() const
{
  if (d->iface) {
    d->iface->smartMutex()->unlock();
    d->iface = 0;
    d->doc = 0;
  }
}

KTextEditor::Document* LockedSmartInterface::currentDocument() const
{
  return d->doc;
}

KTextEditor::SmartInterface* LockedSmartInterface::operator->() const
{
  return d->iface;
}

LockedSmartInterface::operator bool() const
{
  return d->iface;
}

}
