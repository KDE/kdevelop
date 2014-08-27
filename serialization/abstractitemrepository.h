/*
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

#ifndef ABSTRACTITEMREPOSITORY_H
#define ABSTRACTITEMREPOSITORY_H

#include <QtCore/QString>
#include <QtCore/QMutex>

#include "serializationexport.h"

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
};

/// Internal helper class that wraps around a repository object and manages its lifetime.
class KDEVPLATFORMSERIALIZATION_EXPORT AbstractRepositoryManager
{
  public:
    AbstractRepositoryManager();
    virtual ~AbstractRepositoryManager();

    void deleteRepository();

    virtual QMutex* repositoryMutex() const = 0;

  protected:
    mutable AbstractItemRepository* m_repository;
};

}

#endif // ABSTRACTITEMREPOSITORY_H
