/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef ITEMREPOSITORYREGISTRY_H
#define ITEMREPOSITORYREGISTRY_H

#include "serializationexport.h"

#include <QScopedPointer>

class QString;
class QRecursiveMutex;
class QAtomicInt;

namespace KDevelop {
class ISession;
class AbstractItemRepository;
class ItemRepositoryRegistryPrivate;

/**
 * Manages a set of item-repositories and allows loading/storing them all at once from/to disk.
 * Does not automatically store contained repositories on destruction.
 * For the global registry, the storing is triggered from within duchain, so you don't need to care about it.
 */
class KDEVPLATFORMSERIALIZATION_EXPORT ItemRepositoryRegistry
{
public:
    /// Closes all contained repositories.
    /// @warning The current state is not stored to disk.
    /// @note Currently the destructor is never invoked.
    ~ItemRepositoryRegistry();

    /**
     * Initialize the global item-repository registry for the given @p session.
     */
    static void initialize(const QString& repositoryPath);

    /// @returns The global item-repository registry.
    static ItemRepositoryRegistry* self();

    /// Deletes the item-repository of a specified session; or, if it is currently used, marks it for deletion at exit.
    static void deleteRepositoryFromDisk(const QString& repositoryPath);

    /// Add a new repository.
    /// It will automatically be opened with the current path, if one is set.
    /// @note A caller of this function, other than @p repository's constructor, must hold @p repository's mutex lock.
    void registerRepository(AbstractItemRepository* repository);

    /// Remove a repository.
    /// @note Unregistering does not close @p repository.
    void unRegisterRepository(AbstractItemRepository* repository);

    /// @returns The path to item-repositories.
    QString path() const;

    /// Stores all repositories to disk, eventually unloading unused data to save memory.
    /// @note Should be called on a regular basis.
    void store();

    /// Indicates that the application has been closed gracefully.
    /// @note Must be called somewhere at the end of the shutdown sequence.
    void shutdown();

    /// Does a big cleanup, removing all non-persistent items in the repositories.
    /// @returns Count of bytes of data that have been removed.
    int finalCleanup();

    /// Prints the statistics of all registered item-repositories to the command line using qDebug().
    void printAllStatistics() const;

    /// Marks the directory as inconsistent, so it will be discarded
    /// on next startup if the application crashes during the write process.
    void lockForWriting();

    /// Removes the inconsistency mark set by @ref lockForWriting().
    void unlockForWriting();

    /// Returns a custom counter persistently stored as part of item-repositories in the
    /// same directory, possibly creating it.
    /// @param identity     The string used to identify a counter.
    /// @param initialValue Value to initialize a previously inexistent counter with.
    QAtomicInt& customCounter(const QString& identity, int initialValue);

    /// @returns The global item-repository mutex.
    /// @note    Can be used to protect the initialization.
    QRecursiveMutex& mutex();

private:
    explicit ItemRepositoryRegistry(const QString& repositoryPath);

    const QScopedPointer<class ItemRepositoryRegistryPrivate> d_ptr;
    Q_DECLARE_PRIVATE(ItemRepositoryRegistry)

    static ItemRepositoryRegistry* m_self;
};

/// @returns The global item-repository registry (now it is @ref ItemRepositoryRegistry::self()).
KDEVPLATFORMSERIALIZATION_EXPORT ItemRepositoryRegistry& globalItemRepositoryRegistry();
}

#endif // ITEMREPOSITORYREGISTRY_H
