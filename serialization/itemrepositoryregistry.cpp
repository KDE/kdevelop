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

#include "itemrepositoryregistry.h"

#include <QtCore/QDir>
#include <QtCore/QProcessEnvironment>
#include <QtCore/QCoreApplication>
#include <QtDBus/QDBusConnection>
#include <QtCore/QDebug>
#include <QDataStream>

#include <KLocalizedString>

#include <util/fileutils.h>
#include <util/shellutils.h>

#include "abstractitemrepository.h"
#include "debug.h"

using namespace KDevelop;

namespace {

//If KDevelop crashed this many times consicutively, clean up the repository
const int crashesBeforeCleanup = 1;

void setCrashCounter(QFile& crashesFile, int count)
{
  crashesFile.close();
  crashesFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
  QDataStream writeStream(&crashesFile);
  writeStream << count;
}

QString repositoryPathForSession(const KDevelop::ISessionLock::Ptr& session)
{
  QString xdgCacheDir = QProcessEnvironment::systemEnvironment().value("XDG_CACHE_HOME", QDir::homePath() + "/.cache") + "/kdevduchain";
  QString baseDir = QProcessEnvironment::systemEnvironment().value("KDEV_DUCHAIN_DIR", xdgCacheDir);
  baseDir += QStringLiteral("/%1-%2").arg(qApp->applicationName()).arg(session->id());
  return baseDir;
}

bool shouldClear(const QString& path)
{
  QDir dir(path);

  if (!dir.exists()) {
    return false;
  }

  if (getenv("CLEAR_DUCHAIN_DIR")) {
    qCDebug(SERIALIZATION) << "clearing duchain directory because CLEAR_DUCHAIN_DIR is set";
    return true;
  }

  if (dir.exists("is_writing")) {
    qWarning() << "repository" << path << "was write-locked, it probably is inconsistent";
    return true;
  }

  if (!dir.exists(QStringLiteral("version_%1").arg(staticItemRepositoryVersion()))) {
    qWarning() << "version-hint not found, seems to be an old version";
    return true;
  }

  QFile crashesFile(dir.filePath(QStringLiteral("crash_counter")));
  if (crashesFile.open(QIODevice::ReadOnly)) {
    int count;
    QDataStream stream(&crashesFile);
    stream >> count;

    qCDebug(SERIALIZATION) << "current count of crashes: " << count;

    if (count >= crashesBeforeCleanup && !getenv("DONT_CLEAR_DUCHAIN_DIR")) {
      bool userAnswer = askUser(i18np("The previous session crashed", "Session crashed %1 times in a row", count),
                                i18nc("@action", "Clear cache"),
                                i18nc("@title", "Session crashed"),
                                i18n("The crash may be caused by a corruption of cached data.\n\n"
                                      "Press OK if you want KDevelop to clear the cache, otherwise press Cancel if you are sure the crash has another origin."));
      if (userAnswer) {
        qCDebug(SERIALIZATION) << "User chose to clean repository";
        return true;
      } else {
        setCrashCounter(crashesFile, 1);
        qCDebug(SERIALIZATION) << "User chose to reset crash counter";
      }
    } else {
      ///Increase the crash-count. It will be reset if kdevelop is shut down cleanly.
      setCrashCounter(crashesFile, ++count);
    }
  } else {
    setCrashCounter(crashesFile, 1);
  }

  return false;
}

}

namespace KDevelop {

struct ItemRepositoryRegistryPrivate {
  ItemRepositoryRegistry* m_owner;
  bool m_shallDelete;
  QString m_path;
  ISessionLock::Ptr m_sessionLock;
  QMap<AbstractItemRepository*, AbstractRepositoryManager*> m_repositories;
  QMap<QString, QAtomicInt*> m_customCounters;
  mutable QMutex m_mutex;

  ItemRepositoryRegistryPrivate(ItemRepositoryRegistry* owner)
  : m_owner(owner)
  , m_shallDelete(false)
  , m_mutex(QMutex::Recursive)
  {
  }

  void lockForWriting();
  void unlockForWriting();
  void deleteDataDirectory(const QString& path, bool recreate = true);

  /// @param path  A shared directory-path that the item-repositories are to be loaded from.
  /// @returns     Whether the repository registry has been opened successfully.
  ///              If @c false, then all registered repositories should have been deleted.
  /// @note        Currently the given path must reference a hidden directory, just to make sure we're
  ///              not accidentally deleting something important.
  bool open(const QString& path);

  /// Close all contained repositories.
  /// @warning The current state is not stored to disk.
  void close();
};

//The global item-reposity registry
ItemRepositoryRegistry* ItemRepositoryRegistry::m_self = 0;

ItemRepositoryRegistry::ItemRepositoryRegistry(const ISessionLock::Ptr& session)
: d(new ItemRepositoryRegistryPrivate(this))
{
  Q_ASSERT(session);
  d->open(repositoryPathForSession(session));
}

void ItemRepositoryRegistry::initialize(const ISessionLock::Ptr& session)
{
  if (!m_self) {
    ///We intentionally leak the registry, to prevent problems in the destruction order, where
    ///the actual repositories might get deleted later than the repository registry.
    m_self = new ItemRepositoryRegistry(session);
  }
}

ItemRepositoryRegistry* ItemRepositoryRegistry::self()
{
  Q_ASSERT(m_self);
  return m_self;
}

void ItemRepositoryRegistry::deleteRepositoryFromDisk(const ISessionLock::Ptr& session)
{
  // Now, as we have only the global item-repository registry, assume that if and only if
  // the given session is ours, its cache path is used by the said global item-repository registry.
  const QString repositoryPath = repositoryPathForSession(session);
  if(m_self && m_self->d->m_path == repositoryPath) {
    // remove later
    m_self->d->m_shallDelete = true;
  } else {
    // Otherwise, given session is not ours.
    // remove its item-repository directory directly.
    removeDirectory(repositoryPath);
  }
}

QMutex& ItemRepositoryRegistry::mutex()
{
  return d->m_mutex;
}

QAtomicInt& ItemRepositoryRegistry::getCustomCounter(const QString& identity, int initialValue)
{
  if(!d->m_customCounters.contains(identity))
    d->m_customCounters.insert(identity, new QAtomicInt(initialValue));
  return *d->m_customCounters[identity];
}

///The global item-repository registry that is used by default
ItemRepositoryRegistry& globalItemRepositoryRegistry()
{
  return *ItemRepositoryRegistry::self();
}

void ItemRepositoryRegistry::registerRepository(AbstractItemRepository* repository, AbstractRepositoryManager* manager)
{
  QMutexLocker lock(&d->m_mutex);
  d->m_repositories.insert(repository, manager);
  if(!d->m_path.isEmpty()) {
    if(!repository->open(d->m_path)) {
      d->deleteDataDirectory(d->m_path);
      qCritical() << "failed to open a repository";
      abort();
    }
  }
}

QString ItemRepositoryRegistry::path() const
{
  //We cannot lock the mutex here, since this may be called with one of the repositories locked,
  //and that may lead to a deadlock when at the same time a storing is requested
  return d->m_path;
}

void ItemRepositoryRegistryPrivate::lockForWriting()
{
  QMutexLocker lock(&m_mutex);
  //Create is_writing
  QFile f(m_path + "/is_writing");
  f.open(QIODevice::WriteOnly);
  f.close();
}

void ItemRepositoryRegistry::lockForWriting()
{
  d->lockForWriting();
}

void ItemRepositoryRegistryPrivate::unlockForWriting()
{
  QMutexLocker lock(&m_mutex);
  //Delete is_writing
  QFile::remove(m_path + "/is_writing");
}

void ItemRepositoryRegistry::unlockForWriting()
{
  d->unlockForWriting();
}

void ItemRepositoryRegistry::unRegisterRepository(AbstractItemRepository* repository)
{
  QMutexLocker lock(&d->m_mutex);
  Q_ASSERT(d->m_repositories.contains(repository));
  repository->close();
  d->m_repositories.remove(repository);
}

//After calling this, the data-directory may be a new one
void ItemRepositoryRegistryPrivate::deleteDataDirectory(const QString& path, bool recreate)
{
  QMutexLocker lock(&m_mutex);

  //lockForWriting creates a file, that prevents any other KDevelop instance from using the directory as it is.
  //Instead, the other instance will try to delete the directory as well.
  lockForWriting();

  bool result = removeDirectory(path);
  Q_ASSERT(result);
  Q_UNUSED(result);
  // Just recreate the directory then; leave old path (as it is dependent on appname and session only).
  if(recreate) {
    QDir().mkpath(path);
  }
}

bool ItemRepositoryRegistryPrivate::open(const QString& path)
{
  QMutexLocker mlock(&m_mutex);
  if(m_path == path) {
    return true;
  }

  // Check if the repository shall be cleared
  if (shouldClear(path)) {
    qWarning() << QStringLiteral("The data-repository at %1 has to be cleared.").arg(m_path);
    deleteDataDirectory(path);
  }

  QDir().mkpath(path);

  foreach(AbstractItemRepository* repository, m_repositories.keys()) {
    if(!repository->open(path)) {
      deleteDataDirectory(path);
      qCritical() << "failed to open a repository";
      abort();
    }
  }

  QFile f(path + "/Counters");
  if(f.open(QIODevice::ReadOnly)) {
    QDataStream stream(&f);

    while(!stream.atEnd()) {
      //Read in all custom counter values
      QString counterName;
      stream >> counterName;
      int counterValue;
      stream >> counterValue;
      m_owner->getCustomCounter(counterName, 0) = counterValue;
    }
  }

  m_path = path;

  return true;
}

void ItemRepositoryRegistry::store()
{
  QMutexLocker lock(&d->m_mutex);
  foreach(AbstractItemRepository* repository, d->m_repositories.keys()) {
    repository->store();
  }

  QFile versionFile(d->m_path + QStringLiteral("/version_%1").arg(staticItemRepositoryVersion()));
  if(versionFile.open(QIODevice::WriteOnly)) {
    versionFile.close();
  } else {
    qWarning() << "Could not open version file for writing";
  }

  //Store all custom counter values
  QFile f(d->m_path + "/Counters");
  if(f.open(QIODevice::WriteOnly)) {
    f.resize(0);
    QDataStream stream(&f);
    for(QMap<QString, QAtomicInt*>::const_iterator it = d->m_customCounters.constBegin();
        it != d->m_customCounters.constEnd();
        ++it) {
      stream << it.key();
      stream << it.value()->fetchAndAddRelaxed(0);
    }
  } else {
    qWarning() << "Could not open counter file for writing";
  }
}

void ItemRepositoryRegistry::printAllStatistics() const
{
  QMutexLocker lock(&d->m_mutex);
  foreach(AbstractItemRepository* repository, d->m_repositories.keys()) {
    qCDebug(SERIALIZATION) << "statistics in" << repository->repositoryName() << ":";
    qCDebug(SERIALIZATION) << repository->printStatistics();
  }
}

int ItemRepositoryRegistry::finalCleanup()
{
  QMutexLocker lock(&d->m_mutex);
  int changed = false;
  foreach(AbstractItemRepository* repository, d->m_repositories.keys()) {
    int added = repository->finalCleanup();
    changed += added;
    qCDebug(SERIALIZATION) << "cleaned in" << repository->repositoryName() << ":" << added;
  }
  return changed;
}

void ItemRepositoryRegistryPrivate::close()
{
  QMutexLocker lock(&m_mutex);

  foreach(AbstractItemRepository* repository, m_repositories.keys()) {
    repository->close();
  }

  m_path.clear();
}

ItemRepositoryRegistry::~ItemRepositoryRegistry()
{
  QMutexLocker lock(&d->m_mutex);
  d->close();
  foreach(QAtomicInt * counter, d->m_customCounters) {
    delete counter;
  }
  delete d;
}

void ItemRepositoryRegistry::shutdown()
{
  QMutexLocker lock(&d->m_mutex);
  QString path = d->m_path;

  // FIXME: we don't close since this can trigger crashes at shutdown
  //        since some items are still referenced, e.g. in static variables
//   d->close();

  if(d->m_shallDelete) {
    d->deleteDataDirectory(path, false);
  } else {
    QFile::remove(path + QLatin1String("/crash_counter"));
  }
}

}
