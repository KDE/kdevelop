/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef REPOSITORYMANAGER_H
#define REPOSITORYMANAGER_H

#include "abstractitemrepository.h"
#include "itemrepositoryregistry.h"

#include <QMutexLocker>

#include <type_traits>
#include <utility>

namespace KDevelop {
/// This class helps managing the lifetime of a global item repository, and protecting the consistency.
/// Especially it helps doing thread-safe lazy repository-creation.
template <class ItemRepositoryType, bool unloadingEnabled = true, bool lazy = true>
struct RepositoryManager
    : public AbstractRepositoryManager
{
public:
    using Mutex = std::decay_t<decltype(*std::declval<ItemRepositoryType>().mutex())>;
    ///@param shareMutex Option repository from where this repository should take the thread-safety mutex
    explicit RepositoryManager(const QString& name, Mutex* mutex, int version = 1,
                               ItemRepositoryRegistry& registry = globalItemRepositoryRegistry())
        : m_name(name)
        , m_version(version)
        , m_registry(registry)
        , m_mutex(mutex)
    {
        if (!lazy) {
            createRepository();
        }
    }

    ~RepositoryManager() override = default;

    Q_DISABLE_COPY(RepositoryManager)

    ItemRepositoryType * repository() const
    {
        if (!m_repository) {
            createRepository();
        }

        return static_cast<ItemRepositoryType*>(m_repository);
    }

    inline ItemRepositoryType* operator->() const
    {
        return repository();
    }

private:
    void createRepository() const
    {
        if (!m_repository) {
            QMutexLocker lock(&m_registry.mutex());
            if (!m_repository) {
                m_repository = new ItemRepositoryType(m_name, m_mutex, &m_registry, m_version,
                                                      const_cast<RepositoryManager*>(this));
                (*this)->setUnloadingEnabled(unloadingEnabled);
            }
        }
    }

    QString m_name;
    int m_version;
    ItemRepositoryRegistry& m_registry;
    Mutex* m_mutex;
};
}

#endif // REPOSITORYMANAGER_H
