/* This file is part of KDevelop
    Copyright 2006-2008 Hamish Rodda <rodda@kde.org>
    Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "documentrangeobject.h"
#include "documentrangeobject_p.h"

#include <QtCore/QMutexLocker>

#include <kglobal.h>

#include <ktexteditor/smartrange.h>
#include <ktexteditor/smartinterface.h>
#include <ktexteditor/document.h>

#include "hashedstring.h"

#include "editorintegrator.h"
#include "../duchain/duchainregister.h"
#include "../duchain/duchainbase.h"

K_GLOBAL_STATIC(QMutex, s_mutex)

using namespace KTextEditor;

namespace KDevelop {

bool DocumentRangeObjectData::appendedListDynamicDefault() {
  return !DUChainBaseData::shouldCreateConstantData();
}

uint DocumentRangeObjectData::classSize() const {
  //classSize isn't needed when the inheriting class is not based on DUChainBase, so we can safely do this
  return static_cast<const DUChainBaseData*>(this)->classSize();
}

DocumentRangeObjectData::DocumentRangeObjectData(const DocumentRangeObjectData& rhs) : m_range(rhs.m_range) {
  initializeAppendedLists();
}

DocumentRangeObjectDynamicPrivate::DocumentRangeObjectDynamicPrivate() : m_smartRange(0), m_smartMutex(0), m_ownsRange(DocumentRangeObject::DontOwn) {
}

void DocumentRangeObject::aboutToWriteData() {
}

const int smartLockTimeout = 10;

struct OptionalMutexLocker {
  OptionalMutexLocker(QMutex& mutex, uint timeout) : m_mutex(mutex), m_locked(false)
  {
    m_locked = m_mutex.tryLock(timeout);
  }
  
  ~OptionalMutexLocker() {
    if(m_locked)
      m_mutex.unlock();
  }
  
  QMutex& m_mutex;
  bool m_locked;
};

SimpleRange DocumentRangeObject::syncFromSmart() const {
    if(!dd_ptr)
      return d_ptr->m_range;

    OptionalMutexLocker l(*dd_ptr->m_smartMutex, smartLockTimeout);

    if(!dd_ptr->m_smartRange)
        return d_ptr->m_range;
    
    ///@todo This sucks: If the data isn't writable, we won't get the update
    ///But we also cannot make the data writable using aboutToWriteData, because we don't have a write lock
    ///At least return the correct current range
    if(!canWriteData())
      return SimpleRange(*dd_ptr->m_smartRange);

    d_ptr->m_range = *dd_ptr->m_smartRange;
    return d_ptr->m_range;
}

bool DocumentRangeObject::canWriteData() const {
  return true;
}

void DocumentRangeObject::syncToSmart() const {
    if(!dd_ptr)
      return;
    OptionalMutexLocker l(*dd_ptr->m_smartMutex, smartLockTimeout);

    if(!dd_ptr->m_smartRange)
        return;

    dd_ptr->m_smartRange->setRange(d_ptr->m_range.textRange());
}

DocumentRangeObject::DocumentRangeObject(const SimpleRange& range)
    : d_ptr( new DocumentRangeObjectData ), dd_ptr(0), m_ownsData(true)
{
    if(range.isValid())
        d_ptr->m_range = range;
}

DocumentRangeObject::DocumentRangeObject(DocumentRangeObjectData& dd, const SimpleRange& range)
    : d_ptr( &dd ), dd_ptr(0), m_ownsData(true)
{
  Q_ASSERT(d_ptr);
    if(range.isValid())
        d_ptr->m_range = range;
}

DocumentRangeObject::DocumentRangeObject(DocumentRangeObject& useDataFrom)
    : KTextEditor::SmartRangeWatcher(), d_ptr( useDataFrom.d_ptr ), dd_ptr( useDataFrom.dd_ptr ), m_ownsData(false)
{
  Q_ASSERT(d_ptr);
}

void DocumentRangeObject::setData(DocumentRangeObjectData* data, bool /*constructorCalled*/)
{
  Q_ASSERT(data);
  if(d_ptr->m_dynamic && m_ownsData)
    delete d_ptr;
  d_ptr = data;
}

DocumentRangeObject::~ DocumentRangeObject( )
{
    if(m_ownsData)
    {
        if (dd_ptr && dd_ptr->m_smartRange) {
            QMutexLocker l(dd_ptr->m_smartMutex);
            dd_ptr->m_smartRange->removeWatcher(this);

            //If a smart-range is deleted, move it's child-ranges into it's parent.
            //That way we do not delete other DocumentRangeObject's smart-ranges, which corrupts the structure integrity.
            if (dd_ptr->m_ownsRange == DocumentRangeObject::Own)
                EditorIntegrator::releaseRange(dd_ptr->m_smartRange);
        }
      delete dd_ptr;

      if(d_ptr->m_dynamic) {
        //We only delete the data when it's dynamic, because else it is embedded in an array in the top-context.
        //The exact destructor is eventually called from within DUChainBase.
        delete d_ptr;
      }
    }
}

void DocumentRangeObject::setSmartRange(KTextEditor::SmartRange * range, RangeOwning ownsRange)
{
    // If we're being called from anything but a copy constructor, the smart lock should already be held for
    // the new range.

    if(!dd_ptr) {
      if(!range)
        return;
      else
        dd_ptr = new DocumentRangeObjectDynamicPrivate;
    }

    if (dd_ptr->m_smartRange == range)
        return;

    {
        QMutexLocker l(dd_ptr->m_smartMutex);
        if (dd_ptr->m_smartRange)
        {
            dd_ptr->m_smartRange->removeWatcher(this);

            if (dd_ptr->m_ownsRange == DocumentRangeObject::Own)
                EditorIntegrator::releaseRange(dd_ptr->m_smartRange);
        }
    }

    if (range) {
        // Smart lock must already be held.
        dd_ptr->m_smartRange = range;
        KTextEditor::SmartInterface *iface = qobject_cast<KTextEditor::SmartInterface*>( range->document() );
        Q_ASSERT(iface);
        dd_ptr->m_smartMutex = iface->smartMutex();
        dd_ptr->m_ownsRange = ownsRange;

        if(dd_ptr->m_smartRange)
            dd_ptr->m_smartRange->addWatcher(this);

    } else {
        dd_ptr->m_smartRange = 0;
        dd_ptr->m_smartMutex = 0;
        delete dd_ptr;
        dd_ptr = 0;
    }
}

void DocumentRangeObject::clearSmartRange()
{
    setSmartRange(0);
}

SimpleRange DocumentRangeObject::range( ) const
{
    return syncFromSmart();
}

void DocumentRangeObject::setRange(const SimpleRange& range)
{
    aboutToWriteData();
    d_ptr->m_range = range;
    syncToSmart();
}

SmartRange* DocumentRangeObject::smartRange() const
{
    if(dd_ptr)
      return dd_ptr->m_smartRange;
    else
      return 0;
}

bool DocumentRangeObject::contains(const SimpleCursor& cursor) const
{
    return syncFromSmart().contains(cursor);
}

void DocumentRangeObject::rangeDeleted(KTextEditor::SmartRange * range)
{
    // Already smart locked
    Q_ASSERT(dd_ptr);
    Q_ASSERT(range == dd_ptr->m_smartRange);
    if(canWriteData()) ///@todo this sux
      d_ptr->m_range = SimpleRange(*range);
    delete dd_ptr;
    dd_ptr = 0;
}

KTextEditor::SmartRange* DocumentRangeObject::takeRange()
{
    if(!dd_ptr)
      return 0;
    
    aboutToWriteData();
    
    QMutexLocker l(dd_ptr->m_smartMutex);
    KTextEditor::SmartRange* ret;

    syncFromSmart();
    
    ret = dd_ptr->m_smartRange;
    ret->removeWatcher(this);
    
    delete dd_ptr;
    dd_ptr = 0;

    return ret;
}

QMutex* DocumentRangeObject::mutex()
{
    return s_mutex;
}

}
