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

#include "typeregister.h"

#include <debug.h>

namespace KDevelop {
AbstractType* TypeSystem::create(AbstractTypeData* data) const
{
  Q_ASSERT(data);
  if (!ensureFactoryLoaded(*data)) {
    return nullptr;
  }
  return m_factories.value(data->typeClassId)->create(data);
}

void TypeSystem::callDestructor(AbstractTypeData* data) const
{
  Q_ASSERT(data);
  if (!ensureFactoryLoaded(*data)) {
    return;
  }
  m_factories.value(data->typeClassId)->callDestructor(data);
}

uint TypeSystem::dynamicSize(const AbstractTypeData& data) const
{
  if (!ensureFactoryLoaded(data)) {
    return 0;
  }
  return m_factories.value(data.typeClassId)->dynamicSize(data);
}

uint TypeSystem::dataClassSize(const AbstractTypeData& data) const
{
  Q_ASSERT(m_dataClassSizes.contains(data.typeClassId));
  return m_dataClassSizes.value(data.typeClassId);
}

bool TypeSystem::isFactoryLoaded(const AbstractTypeData& data) const
{
  return m_factories.contains(data.typeClassId);
}

bool TypeSystem::ensureFactoryLoaded(const AbstractTypeData& data) const
{
  if (!m_factories.contains(data.typeClassId)) {
    qCWarning(LANGUAGE) << "Factory for this type not loaded:" << data.typeClassId;
    Q_ASSERT(false);
    return false;
  }
  return true;
}

void TypeSystem::copy(const AbstractTypeData& from, AbstractTypeData& to, bool constant) const
{
  //Shouldn't try to copy an unknown type
  ensureFactoryLoaded(from);
  m_factories.value(from.typeClassId)->copy(from, to, constant);
}

TypeSystem& TypeSystem::self() {
  static TypeSystem system;
  return system;
}

void TypeSystem::registerTypeClassInternal(AbstractTypeFactory* repo, uint dataClassSize, uint identity)
{
  Q_ASSERT(repo);
  Q_ASSERT(!m_factories.contains(identity));
  m_factories.insert(identity, repo);
  Q_ASSERT(!m_dataClassSizes.contains(identity));
  m_dataClassSizes.insert(identity, dataClassSize);
}

void TypeSystem::unregisterTypeClassInternal(uint identity)
{
  qCDebug(LANGUAGE) << "Unregistering type class" << identity;
  AbstractTypeFactory* repo = m_factories.take(identity);
  Q_ASSERT(repo);
  delete repo;
  int removed = m_dataClassSizes.remove(identity);
  Q_ASSERT(removed);
  Q_UNUSED(removed);
}

}
