/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
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

TypeSystem& TypeSystem::self()
{
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
