/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef ABSTRACTITEMREPOSITORY_H
#define ABSTRACTITEMREPOSITORY_H

#include "serializationexport.h"

#include <QtGlobal>

class QString;

namespace KDevelop {
/// Returns a version-number that is used to reset the item-repository after incompatible layout changes.
KDEVPLATFORMSERIALIZATION_EXPORT uint staticItemRepositoryVersion();

/// The interface class for an item-repository object.
class KDEVPLATFORMSERIALIZATION_EXPORT AbstractItemRepository
{
public:
    virtual ~AbstractItemRepository();
    /// @param path A shared directory-name that the item-repository is to be loaded from.
    /// @returns    Whether the repository has been opened successfully.
    virtual bool open(const QString& path) = 0;
    virtual void close(bool doStore = false) = 0;
    /// Stores the repository contents to disk, eventually unloading unused data to save memory.
    virtual void store() = 0;
    /// Does a big cleanup, removing all non-persistent items in the repositories.
    /// @returns Count of bytes of data that have been removed.
    virtual int finalCleanup() = 0;
    virtual QString repositoryName() const = 0;
    virtual QString printStatistics() const = 0;

    /// lock the repository
    virtual void lock() = 0;
    /// unlock the repository
    virtual void unlock() = 0;
};
}

#endif // ABSTRACTITEMREPOSITORY_H
