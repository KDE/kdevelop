/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "duchainregister.h"
#include "duchainbase.h"

#include <QDebug>

#define ENSURE_VALID_CLASSID(id) \
    qFatal("Invalid class id: %i", id);

namespace KDevelop {
DUChainItemSystem::~DUChainItemSystem()
{
    qDeleteAll(m_factories);
}

DUChainBase* DUChainItemSystem::create(DUChainBaseData* data) const
{
    if (uint(m_factories.size()) <= data->classId || m_factories[data->classId] == nullptr)
        return nullptr;
    return m_factories[data->classId]->create(data);
}

DUChainBaseData* DUChainItemSystem::cloneData(const DUChainBaseData& data) const
{
    if (uint(m_factories.size()) <= data.classId || m_factories[data.classId] == nullptr) {
        ENSURE_VALID_CLASSID(data.classId)
        return nullptr;
    }
    return m_factories[data.classId]->cloneData(data);
}

void DUChainItemSystem::callDestructor(DUChainBaseData* data) const
{
    if (uint(m_factories.size()) <= data->classId || m_factories[data->classId] == nullptr)
        return;
    m_factories[data->classId]->callDestructor(data);
}

void DUChainItemSystem::freeDynamicData(KDevelop::DUChainBaseData* data) const
{
    if (uint(m_factories.size()) <= data->classId || m_factories[data->classId] == nullptr)
        return;
    m_factories[data->classId]->freeDynamicData(data);
}

void DUChainItemSystem::deleteDynamicData(DUChainBaseData* data) const
{
    if (uint(m_factories.size()) <= data->classId || m_factories[data->classId] == nullptr)
        return;
    m_factories[data->classId]->deleteDynamicData(data);
}

uint DUChainItemSystem::dynamicSize(const DUChainBaseData& data) const
{
    if (uint(m_factories.size()) <= data.classId || m_factories[data.classId] == nullptr)
        return 0;
    return m_factories[data.classId]->dynamicSize(data);
}

uint DUChainItemSystem::dataClassSize(const DUChainBaseData& data) const
{
    if (uint(m_dataClassSizes.size()) <= data.classId || m_dataClassSizes[data.classId] == 0)
        return 0;
    return m_dataClassSizes[data.classId];
}

void DUChainItemSystem::copy(const DUChainBaseData& from, DUChainBaseData& to, bool constant) const
{
    if (uint(m_factories.size()) <= from.classId || m_factories[from.classId] == nullptr) {
        ENSURE_VALID_CLASSID(from.classId)
        return;
    }
    m_factories[from.classId]->copy(from, to, constant);
}

DUChainItemSystem& DUChainItemSystem::self()
{
    static DUChainItemSystem system;
    return system;
}
}
