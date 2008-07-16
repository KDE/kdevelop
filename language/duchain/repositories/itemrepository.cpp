#include "itemrepository.h"

namespace KDevelop {

AbstractItemRepository::~AbstractItemRepository() {
}

ItemRepositoryRegistry::ItemRepositoryRegistry() : m_cleared(false) {
}


///The global item-repository registry that is used by default
ItemRepositoryRegistry& globalItemRepositoryRegistry() {
  static ItemRepositoryRegistry global;
  return global;
}

void ItemRepositoryRegistry::registerRepository(AbstractItemRepository* repository) {
  m_repositories << repository;
  if(!m_path.isEmpty())
    repository->open(m_path, m_cleared);
}

void ItemRepositoryRegistry::unRegisterRepository(AbstractItemRepository* repository) {
  Q_ASSERT(m_repositories.contains(repository));
  repository->close();
  m_repositories.removeAll(repository);
}

bool ItemRepositoryRegistry::open(const QString& path, bool clear) {
  m_path = path;
  m_cleared = clear;
  foreach(AbstractItemRepository* repository, m_repositories) {
    if(!repository->open(path, clear)) {
      Q_ASSERT(!clear); //We have a problem if opening a repository fails although it should be cleared
      close();
      open(path, true);
    }
  }
  return true;
}

void ItemRepositoryRegistry::close() {
  foreach(AbstractItemRepository* repository, m_repositories)
    repository->close();
  m_path = QString();
  m_cleared = false;
}

ItemRepositoryRegistry::~ItemRepositoryRegistry() {
  close();
}

}