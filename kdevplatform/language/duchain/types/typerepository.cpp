/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "typerepository.h"

#include <QMutex>
#include <QMutexLocker>

#include <debug.h>
#include "../types/typesystemdata.h"
#include "../types/typeregister.h"
#include <serialization/referencecounting.h>
#include <serialization/itemrepository.h>

#define DEBUG_TYPE_REPOSITORY
#define ASSERT_ON_PROBLEM

namespace KDevelop  {
class AbstractTypeDataRequest
{
public:
    AbstractTypeDataRequest(const AbstractType& type) : m_item(type)
    {
    }

    enum {
        AverageSize = sizeof(AbstractTypeData) + 12
    };

    unsigned int hash() const
    {
        return m_item.hash();
    }

    uint itemSize() const
    {
        return TypeSystem::self().dynamicSize(*m_item.d_ptr);
    }

    void createItem(AbstractTypeData* item) const
    {
        TypeSystem::self().copy(*m_item.d_ptr, *item, true);
        Q_ASSERT(!item->m_dynamic);
#ifdef DEBUG_TYPE_REPOSITORY
        AbstractType::Ptr otherType(TypeSystem::self().create(const_cast<AbstractTypeData*>(item)));
        if (!otherType->equals(&m_item)) {
            //For debugging, so one can trace what happened
            qCWarning(LANGUAGE) << "created type in repository does not equal source type:" << m_item.toString() <<
                otherType->toString();
            TypeSystem::self().copy(*m_item.d_ptr, *item, true);
            otherType->equals(&m_item);
        }
#ifdef ASSERT_ON_PROBLEM
        Q_ASSERT(otherType->equals(&m_item));
#endif
#endif
        item->inRepository = true;
    }

    static void destroy(AbstractTypeData* item, KDevelop::AbstractItemRepository&)
    {
        TypeSystem::self().callDestructor(item);
    }

    static bool persistent(const AbstractTypeData* item)
    {
        // Don't try to delete release items for which the factory is not loaded, as that will lead to a crash/assertion later
        return ( bool )item->refCount || !TypeSystem::self().isFactoryLoaded(*item);
    }

    bool equals(const AbstractTypeData* item) const
    {
        if (!TypeSystem::self().isFactoryLoaded(*item))
            return false;

        AbstractType::Ptr otherType(TypeSystem::self().create(const_cast<AbstractTypeData*>(item)));
        Q_ASSERT(otherType);
        return m_item.equals(otherType.data());
    }

    const AbstractType& m_item;
};

QMutex* typeRepositoryMutex()
{
    static auto mutex = QMutex(QMutex::Recursive);
    return &mutex;
}

// The object is created in a function, to prevent initialization-order issues
static RepositoryManager<ItemRepository<AbstractTypeData, AbstractTypeDataRequest>, false>& typeRepository()
{
    static RepositoryManager<ItemRepository<AbstractTypeData, AbstractTypeDataRequest>, false> repository(
        QStringLiteral("Type Repository"), typeRepositoryMutex());
    return repository;
}

void initTypeRepository()
{
    typeRepository();
}

uint TypeRepository::indexForType(const AbstractType::Ptr& input)
{
    if (!input)
        return 0;

    uint i = typeRepository()->index(AbstractTypeDataRequest(*input));
#ifdef DEBUG_TYPE_REPOSITORY
    AbstractType::Ptr t = typeForIndex(i);
    if (!t->equals(input.data())) {
        qCWarning(LANGUAGE) << "found type in repository does not equal source type:" << input->toString() <<
            t->toString();
        t->equals(input.data());
    }
#ifdef ASSERT_ON_PROBLEM
    Q_ASSERT(t->equals(input.data()));
    Q_ASSERT(input->equals(t.data()));
#endif
#endif
    return i;
}

AbstractType::Ptr TypeRepository::typeForIndex(uint index)
{
    if (index == 0)
        return AbstractType::Ptr();

    return AbstractType::Ptr(TypeSystem::self().create(const_cast<AbstractTypeData*>(typeRepository()->itemFromIndex(
                                                                                         index))));
}

void TypeRepository::increaseReferenceCount(uint index, ReferenceCountManager* manager)
{
    if (!index)
        return;
    QMutexLocker lock(typeRepository()->mutex());
    AbstractTypeData* data = typeRepository()->dynamicItemFromIndexSimple(index);
    Q_ASSERT(data);
    if (manager)
        manager->increase(data->refCount, index);
    else
        ++data->refCount;
}

void TypeRepository::decreaseReferenceCount(uint index, ReferenceCountManager* manager)
{
    if (!index)
        return;
    QMutexLocker lock(typeRepository()->mutex());
    AbstractTypeData* data = typeRepository()->dynamicItemFromIndexSimple(index);
    Q_ASSERT(data);
    Q_ASSERT(data->refCount > 0);
    if (manager)
        manager->decrease(data->refCount, index);
    else
        --data->refCount;
}
}
