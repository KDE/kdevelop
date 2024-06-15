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
#include <serialization/repositorymanager.h>

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

    size_t hash() const
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

QRecursiveMutex* typeRepositoryMutex()
{
    static QRecursiveMutex mutex;
    return &mutex;
}

using TypeItemRepository = ItemRepository<AbstractTypeData, AbstractTypeDataRequest, true, QRecursiveMutex>;

template<>
class ItemRepositoryFor<AbstractType>
{
    friend struct LockedItemRepository;
    static TypeItemRepository& repo()
    {
        static RepositoryManager<TypeItemRepository, false> manager(QStringLiteral("Type Repository"),
                                                                    typeRepositoryMutex());
        return *manager.repository();
    }
};

void initTypeRepository()
{
    LockedItemRepository::initialize<AbstractType>();
}

uint TypeRepository::indexForType(const AbstractType* input)
{
    if (!input)
        return 0;

    uint i = LockedItemRepository::write<AbstractType>(
        [request = AbstractTypeDataRequest(*input)](TypeItemRepository& repo) {
            return repo.index(request);
        });
#ifdef DEBUG_TYPE_REPOSITORY
    AbstractType::Ptr t = typeForIndex(i);
    if (!t->equals(input)) {
        qCWarning(LANGUAGE) << "found type in repository does not equal source type:" << input->toString() <<
            t->toString();
        t->equals(input);
    }
#ifdef ASSERT_ON_PROBLEM
    Q_ASSERT(t->equals(input));
    Q_ASSERT(input->equals(t.data()));
#endif
#endif
    return i;
}

AbstractType::Ptr TypeRepository::typeForIndex(uint index)
{
    if (index == 0)
        return AbstractType::Ptr();

    return LockedItemRepository::read<AbstractType>([index](const TypeItemRepository& repo) {
        auto item = repo.itemFromIndex(index);
        return AbstractType::Ptr(TypeSystem::self().create(const_cast<AbstractTypeData*>(item)));
    });
}

template <typename RefCountChanger>
static void changeReferenceCount(uint index, RefCountChanger changeRefCount)
{
    if (!index)
        return;

    LockedItemRepository::write<AbstractType>([index, changeRefCount](TypeItemRepository& repo) {
        AbstractTypeData* data = repo.dynamicItemFromIndexSimple(index);
        Q_ASSERT(data);
        changeRefCount(data->refCount);
    });
}

void TypeRepository::increaseReferenceCount(uint index, ReferenceCountManager* manager)
{
    changeReferenceCount(index, [index, manager](uint& refCount) {
        if (manager)
            manager->increase(refCount, index);
        else
            ++refCount;
    });
}

void TypeRepository::decreaseReferenceCount(uint index, ReferenceCountManager* manager)
{
    changeReferenceCount(index, [index, manager](uint& refCount) {
        Q_ASSERT(refCount > 0);
        if (manager)
            manager->decrease(refCount, index);
        else
            --refCount;
    });
}
}
