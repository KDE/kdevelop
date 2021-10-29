/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef REPOSITORYMANAGER_H
#define REPOSITORYMANAGER_H

#include "abstractitemrepository.h"
#include "itemrepositoryregistry.h"

namespace KDevelop {
/// This class helps managing the lifetime of a global item repository, and protecting the consistency.
/// Especially it helps doing thread-safe lazy repository-creation.
template <class ItemRepositoryType, bool unloadingEnabled = true, bool lazy = true>
struct RepositoryManager
    : public AbstractRepositoryManager
{
public:
    ///@param shareMutex Option repository from where this repository should take the thread-safety mutex
    explicit RepositoryManager(const QString& name,
                               int version = 1,
                               AbstractRepositoryManager*(*shareMutex)() = nullptr,
                               ItemRepositoryRegistry& registry = globalItemRepositoryRegistry()) :
        m_name(name)
        , m_version(version)
        , m_registry(registry)
        , m_shareMutex(shareMutex)
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

    QMutex* repositoryMutex() const override
    {
        return (*this)->mutex();
    }

private:
    void createRepository() const
    {
        if (!m_repository) {
            QMutexLocker lock(&m_registry.mutex());
            if (!m_repository) {
                m_repository =
                    new ItemRepositoryType(m_name, &m_registry, m_version, const_cast<RepositoryManager*>(this));
                if (m_shareMutex) {
                    (*this)->setMutex(m_shareMutex()->repositoryMutex());
                }
                (*this)->setUnloadingEnabled(unloadingEnabled);
            }
        }
    }

    QString m_name;
    int m_version;
    ItemRepositoryRegistry& m_registry;
    AbstractRepositoryManager* (* m_shareMutex)();
};
}

#endif // REPOSITORYMANAGER_H
