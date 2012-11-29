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

#include "itemrepository.h"

#include <QDataStream>
#include <QUuid>
#include <QApplication>
#include <QTextStream>
#include <QProcessEnvironment>

#include <KStandardDirs>
#include <KComponentData>
#include <KLockFile>
#include <KMessageBox>
#include <KLocale>

#include <interfaces/icore.h>
#include <interfaces/isession.h>
#include <util/fileutils.h>

#include "../duchain.h"

namespace KDevelop {

//If KDevelop crashed this many times consicutively, clean up the repository
const int crashesBeforeCleanup = 2;

//The global item-reposity registry
ItemRepositoryRegistry* ItemRepositoryRegistry::m_self = 0;

uint staticItemRepositoryVersion() {
  //Increase this to reset incompatible item-repositories
  return 72;
}

AbstractItemRepository::~AbstractItemRepository() {
}


ItemRepositoryRegistry::ItemRepositoryRegistry() : m_mutex(QMutex::Recursive) {
  Q_ASSERT( ICore::self() );
  Q_ASSERT( ICore::self()->activeSession() );
  QString repositoryPath = repositoryPathForSession( ICore::self()->activeSession()->id() );
  open( repositoryPath, false );
}

ItemRepositoryRegistry* ItemRepositoryRegistry::self()
{
  if( !m_self ) {
    ///We intentionally leak the registry, to prevent problems in the destruction order, where
    ///the actual repositories might get deleted later than the repository registry.
    m_self = new ItemRepositoryRegistry;
  }
  return m_self;
}

QString ItemRepositoryRegistry::repositoryPathForSession(const QUuid& uuid) {
  QString xdgCacheDir = QProcessEnvironment::systemEnvironment().value( "XDG_CACHE_HOME", QDir::homePath() + "/.cache" ) + "/kdevduchain";
  QString baseDir = QProcessEnvironment::systemEnvironment().value( "KDEV_DUCHAIN_DIR", xdgCacheDir );
  baseDir += QString( "/%1-%2" ).arg( qAppName() ).arg( uuid.toString() );
  KStandardDirs::makeDir( baseDir );
  return baseDir;
}

QMutex& ItemRepositoryRegistry::mutex() {
  return m_mutex;
}

QAtomicInt& ItemRepositoryRegistry::getCustomCounter(const QString& identity, int initialValue) {
  if(!m_customCounters.contains(identity))
    m_customCounters.insert(identity, new QAtomicInt(initialValue));
  return *m_customCounters[identity];
}

///The global item-repository registry that is used by default
ItemRepositoryRegistry& globalItemRepositoryRegistry() {
  return *ItemRepositoryRegistry::self();
}

void ItemRepositoryRegistry::registerRepository(AbstractItemRepository* repository, AbstractRepositoryManager* manager) {
  QMutexLocker lock(&m_mutex);
  m_repositories.insert(repository, manager);
  if(!m_path.isEmpty()) {
    if(!repository->open(m_path)) {
      deleteDataDirectory();
      kError() << "failed to open a repository";
      abort();
    }
  }
}

QString ItemRepositoryRegistry::path() const {
  //We cannot lock the mutex here, since this may be called with one of the repositories locked,
  //and that may lead to a deadlock when at the same time a storing is requested
  return m_path;
}

void ItemRepositoryRegistry::lockForWriting() {
  QMutexLocker lock(&m_mutex);
  //Create is_writing
  QFile f(m_path + "/is_writing");
  f.open(QIODevice::WriteOnly);
  f.close();
}

void ItemRepositoryRegistry::unlockForWriting() {
  QMutexLocker lock(&m_mutex);
  //Delete is_writing
  QFile::remove(m_path + "/is_writing");
}

void ItemRepositoryRegistry::unRegisterRepository(AbstractItemRepository* repository) {
  QMutexLocker lock(&m_mutex);
  Q_ASSERT(m_repositories.contains(repository));
  repository->close();
  m_repositories.remove(repository);
}

//After calling this, the data-directory may be a new one
void ItemRepositoryRegistry::deleteDataDirectory() {
  QMutexLocker lock(&m_mutex);

  //lockForWriting creates a file, that prevents any other KDevelop instance from using the directory as it is.
  //Instead, the other instance will try to delete the directory as well.
  lockForWriting();

  bool result = removeDirectory(m_path);
  Q_ASSERT(result);
  Q_UNUSED(result);
  // Just recreate the directory then; leave old path (as it is dependent on appname and session only).
  KStandardDirs::makeDir(m_path);
}

void setCrashCounter(QFile& crashesFile, int count) {
  crashesFile.close();
  crashesFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
  QDataStream writeStream(&crashesFile);
  writeStream << count;
}

bool ItemRepositoryRegistry::open(const QString& path, bool clear) {
  QMutexLocker mlock(&m_mutex);
  if(m_path == path && !clear) {
    return true;
  }
  m_path = path;

  if(QFile::exists(m_path + "/is_writing")) {
    kWarning() << "repository" << m_path << "was write-locked, it probably is inconsistent";
    clear = true;
  }

  QDir pathDir(m_path);
  pathDir.setFilter(QDir::Files);

  // If there is no files in the repository, it has been just cleared
  if(!pathDir.count()) {
    if(!QFile::exists( m_path + QString("/version_%1").arg(staticItemRepositoryVersion()) )) {
      kWarning() << "version-hint not found, seems to be an old version";
      clear = true;
    } else if(getenv("CLEAR_DUCHAIN_DIR")) {
      kWarning() << "clearing duchain directory because CLEAR_DUCHAIN_DIR is set";
      clear = true;
    }
  }

  QFile crashesFile(m_path + QString("/crash_counter"));
  if(crashesFile.open(QIODevice::ReadOnly)) {
    int count;
    QDataStream stream(&crashesFile);
    stream >> count;

    kDebug() << "current count of crashes: " << count;

    if(count >= crashesBeforeCleanup && !getenv("DONT_CLEAR_DUCHAIN_DIR"))
    {
      int userAnswer = 0;
      ///NOTE: we don't want to crash our beloved tools when run in no-gui mode
      ///NOTE 2: create a better, reusable version of the below for other tools
      if (QApplication::type() == QApplication::Tty) {
        // no ui-mode e.g. for duchainify and other tools
        QTextStream out(stdout);
        out << i18np("Session crashed %1 time in a row", "Session crashed %1 times in a row", count) << endl;
        out << endl;
        QTextStream in(stdin);
        QString input;
        while(true) {
          out << i18n("Clear cache: [Y/n] ") << flush;
          input = in.readLine().trimmed();
          if (input.toLower() == "y" || input.isEmpty()) {
            userAnswer = KMessageBox::Yes;
            break;
          } else if (input.toLower() == "n") {
            userAnswer = KMessageBox::No;
            break;
          }
        }
      } else {
        userAnswer = KMessageBox::questionYesNo(0,
                                                i18np("The Session crashed once.", "The Session crashed %1 times in a row.", count) + "\n\n" + i18n("The crash may be caused by a corruption of cached data.\n\nPress OK if you want KDevelop to clear the cache, otherwise press Cancel if you are sure the crash has another origin."),
                                                i18n("Session crashed"),
                                                KStandardGuiItem::ok(),
                                                KStandardGuiItem::cancel());
      }
      if (userAnswer == KMessageBox::Yes) {
        clear = true;
        kDebug() << "User chose to clean repository";
      } else {
        setCrashCounter(crashesFile, 1);
        kDebug() << "User chose to reset crash counter";
      }
    } else {
      ///Increase the crash-count. It will be reset if kdevelop is shut down cleanly.
      setCrashCounter(crashesFile, ++count);
    }
  } else {
    setCrashCounter(crashesFile, 1);
  }

  if(clear) {
    kWarning() << QString("The data-repository at %1 has to be cleared.").arg(m_path);
//     KMessageBox::information( 0, i18n("The data-repository at %1 has to be cleared. Either the disk format has changed, or KDevelop crashed while writing the repository.", m_path ) );
#ifdef Q_OS_WIN
    /// on Windows a file can't be deleted unless the last file handle gets closed
    /// deleteDataDirectory would enter a never ending loop
    crashesFile.close();
#endif
    deleteDataDirectory();
  }

  foreach(AbstractItemRepository* repository, m_repositories.keys()) {
    if(!repository->open(path)) {
      deleteDataDirectory();
      kError() << "failed to open a repository";
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
      if(m_customCounters.contains(counterName))
        *m_customCounters[counterName] = counterValue;
      else
        getCustomCounter(counterName, 0) = counterValue;
    }
  }else{
//     kDebug() << "Could not open counter file";
  }
  
  return true;
}

void ItemRepositoryRegistry::store() {
  QMutexLocker lock(&m_mutex);
  foreach(AbstractItemRepository* repository, m_repositories.keys())
    repository->store();

  QFile versionFile(m_path + QString("/version_%1").arg(staticItemRepositoryVersion()));
  if(versionFile.open(QIODevice::WriteOnly)) {
    versionFile.close();
  }else{
    kWarning() << "Could not open version file for writing";
  }
  
  //Store all custom counter values
  QFile f(m_path + "/Counters");
  if(f.open(QIODevice::WriteOnly)) {
    f.resize(0);
    QDataStream stream(&f);
    for(QMap<QString, QAtomicInt*>::const_iterator it = m_customCounters.constBegin(); it != m_customCounters.constEnd(); ++it) {
      stream << it.key();
      stream << it.value()->fetchAndAddRelaxed(0);
    }
  }else{
    kWarning() << "Could not open counter file for writing";
  }
}

void ItemRepositoryRegistry::printAllStatistics() const {
  QMutexLocker lock(&m_mutex);
  foreach(AbstractItemRepository* repository, m_repositories.keys()) {
    kDebug() << "statistics in" << repository->repositoryName() << ":";
    kDebug() << repository->printStatistics();
  }
}

int ItemRepositoryRegistry::finalCleanup() {
  QMutexLocker lock(&m_mutex);
  int changed = false;
  foreach(AbstractItemRepository* repository, m_repositories.keys()) {
    int added = repository->finalCleanup();
    changed += added;
    kDebug() << "cleaned in" << repository->repositoryName() << ":" << added;
  }
  
  return changed;
}


void ItemRepositoryRegistry::close() {

  QMutexLocker lock(&m_mutex);
    
  foreach(AbstractItemRepository* repository, m_repositories.keys())
    repository->close();
  
  m_path.clear();
}

ItemRepositoryRegistry::~ItemRepositoryRegistry() {
  close();
  QMutexLocker lock(&m_mutex);
  foreach(QAtomicInt* counter, m_customCounters)
    delete counter;
}

void ItemRepositoryRegistry::shutdown() {
  QFile::remove(m_path + QString("/crash_counter"));
}

}
