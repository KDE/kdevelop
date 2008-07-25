/* This  is part of KDevelop
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

#include "duchainbase.h"

#include <QMutexLocker>
#include <QThreadStorage>

#include "duchainpointer.h"
#include "indexedstring.h"
#include "topducontext.h"
#include "duchainregister.h"

namespace KDevelop
{
REGISTER_DUCHAIN_ITEM(DUChainBase);

uint DUChainBaseData::classSize() const {
  return DUChainItemSystem::self().dataClassSize(*this);
}

void DUChainBase::aboutToSave() {
}

DUChainBase::DUChainBase(const SimpleRange& range)
  : KDevelop::DocumentRangeObject(*new DUChainBaseData, range), m_ptr( 0L )
{
  d_func_dynamic()->setClassId(this);
}

DUChainBase::DUChainBase( DUChainBaseData & dd, const SimpleRange& range )
  : KDevelop::DocumentRangeObject( dd, range ), m_ptr( 0 )
{
}

DUChainBase::DUChainBase( DUChainBaseData & dd )
  : KDevelop::DocumentRangeObject( dd ), m_ptr( 0 )
{
}

DUChainBase::DUChainBase( DUChainBase& rhs )
  : KDevelop::DocumentRangeObject( rhs ), m_ptr( 0 )
{
}

IndexedString DUChainBase::url() const
{
  TopDUContext* top = topContext();
  if(top)
    return top->url();
  else
    return IndexedString();
}


DUChainBase::~DUChainBase()
{
  if(d_func()->m_dynamic) {
    //We only delete the data when it's dynamic, because else it is embedded in an array in the top-context.
    KDevelop::DUChainItemSystem::self().callDestructor(d_func_dynamic());
  }
  
  if (m_ptr)
    m_ptr->m_base = 0;
}

TopDUContext* DUChainBase::topContext() const
{
  return 0;
}

const KSharedPtr<DUChainPointerData>& DUChainBase::weakPointer() const
{
  QMutexLocker lock(mutex());

  if (!m_ptr) {
    m_ptr = new DUChainPointerData(const_cast<DUChainBase*>(this));
    m_ptr->m_base = const_cast<DUChainBase*>(this);
  }

  return m_ptr;
}

void DUChainBase::rebuildDynamicData(DUContext* parent, uint ownIndex)
{
}

void DUChainBase::makeDynamic() {
  if(!d_func()->m_dynamic) {
    //We don't delete the previous data, because it's embedded in the top-context when it isn't dynamic.
    d_ptr = DUChainItemSystem::self().cloneData(*d_func());
    Q_ASSERT(d_func()->m_dynamic);
  }
}

QThreadStorage<char*> shouldCreateConstantDataStorage;

bool DUChainBaseData::shouldCreateConstantData() {
  return shouldCreateConstantDataStorage.hasLocalData();
}

void DUChainBaseData::setShouldCreateConstantData(bool should) {
  if(should == shouldCreateConstantData())
    return;
  if(should)
    shouldCreateConstantDataStorage.setLocalData(new char);
  else
    shouldCreateConstantDataStorage.setLocalData(0);
}
}

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on

