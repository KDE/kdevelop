/* This file is part of KDevelop
    Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "duchainregister.h"
#include "duchainbase.h"

namespace KDevelop {
DUChainBase* DUChainItemSystem::create(DUChainBaseData* data) const {
  if(uint(m_factories.size()) <= data->classId || m_factories[data->classId] == 0)
    return 0;
  return m_factories[data->classId]->create(data);
}

DUChainBaseData* DUChainItemSystem::cloneData(const DUChainBaseData& data) const {
  if(uint(m_factories.size()) <= data.classId || m_factories[data.classId] == 0) {
    Q_ASSERT(0); //Or we'll crash later
    return 0;
  }
  return m_factories[data.classId]->cloneData(data);
}

void DUChainItemSystem::callDestructor(DUChainBaseData* data) const {
  if(uint(m_factories.size()) <= data->classId || m_factories[data->classId] == 0)
    return;
  return m_factories[data->classId]->callDestructor(data);
}

void DUChainItemSystem::freeDynamicData(KDevelop::DUChainBaseData* data) const {
  if(uint(m_factories.size()) <= data->classId || m_factories[data->classId] == 0)
    return;
  return m_factories[data->classId]->freeDynamicData(data);

}

uint DUChainItemSystem::dynamicSize(const DUChainBaseData& data) const {
  if(uint(m_factories.size()) <= data.classId || m_factories[data.classId] == 0)
    return 0;
  return m_factories[data.classId]->dynamicSize(data);
}

uint DUChainItemSystem::dataClassSize(const DUChainBaseData& data) const {
  if(uint(m_dataClassSizes.size()) <= data.classId || m_dataClassSizes[data.classId] == 0)
    return 0;
  return m_dataClassSizes[data.classId];
}


void DUChainItemSystem::copy(const DUChainBaseData& from, DUChainBaseData& to, bool constant) const {
  if(uint(m_factories.size()) <= from.classId || m_factories[from.classId] == 0) {
    Q_ASSERT(0); //Shouldn't try to copy an unknown type
    return;
  }
  return m_factories[from.classId]->copy(from, to, constant);
}

DUChainItemSystem& DUChainItemSystem::self() {
  static DUChainItemSystem system;
  return system;
}
}
