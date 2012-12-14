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

#ifndef REPOSITORYMANAGER_H
#define REPOSITORYMANAGER_H

#include "abstractitemrepository.h"
#include "itemrepositoryregistry.h"

namespace KDevelop {

/// This class helps managing the lifetime of a global item repository, and protecting the consistency.
/// Especially it helps doing thread-safe lazy repository-creation.
template<class ItemRepositoryType, bool unloadingEnabled = true, bool lazy = true>
struct RepositoryManager : public AbstractRepositoryManager {
public:
  ///@param shareMutex Option repository from where this repository should take the thread-safety mutex
  RepositoryManager(QString name,
                    int version = 1,
                    AbstractRepositoryManager*(*shareMutex)() = 0,
                    ItemRepositoryRegistry& registry = globalItemRepositoryRegistry()) :
  m_name(name),
  m_version(version),
  m_registry(registry),
  m_shareMutex(shareMutex) {
    if(!lazy) {
      createRepository();
    }
  }

  ~RepositoryManager() {
  }

  inline ItemRepositoryType* operator->() const {
    if(!m_repository) {
      createRepository();
    }

    return static_cast<ItemRepositoryType*>(m_repository);
  }

  QMutex* repositoryMutex() const {
    return (*this)->mutex();
  }

private:
  void createRepository() const {
    if(!m_repository) {
      QMutexLocker lock(&m_registry.mutex());
      if(!m_repository) {
        m_repository = new ItemRepositoryType(m_name, &m_registry, m_version, const_cast<RepositoryManager*>(this));
        if(m_shareMutex) {
          (*this)->setMutex(m_shareMutex()->repositoryMutex());
        }
        (*this)->setUnloadingEnabled(unloadingEnabled);
      }
    }
  }

  QString m_name;
  int m_version;
  ItemRepositoryRegistry& m_registry;
  AbstractRepositoryManager* (*m_shareMutex)();
};

}

#endif // REPOSITORYMANAGER_H
