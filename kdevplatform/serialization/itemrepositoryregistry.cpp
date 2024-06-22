/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "itemrepositoryregistry.h"

#include <QDir>
#include <QProcessEnvironment>
#include <QCoreApplication>
#include <QDataStream>
#include <QMutexLocker>
#include <QRecursiveMutex>
#include <QStandardPaths>

#include <KLocalizedString>

#include <util/shellutils.h>

#include "abstractitemrepository.h"
#include "debug.h"

#include <mutex>
#include <set>
#include <utility>

using namespace KDevelop;

namespace {
//If KDevelop crashed this many times consecutively, clean up the repository
const int crashesBeforeCleanup = 1;

void setCrashCounter(QFile& crashesFile, int count)
{
    crashesFile.close();
    crashesFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
    QDataStream writeStream(&crashesFile);
    writeStream << count;
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

    if (dir.exists(QStringLiteral("is_writing"))) {
        qCWarning(SERIALIZATION) << "repository" << path << "was write-locked, it probably is inconsistent";
        return true;
    }

    if (!dir.exists(QStringLiteral("version_%1").arg(staticItemRepositoryVersion()))) {
        qCWarning(SERIALIZATION) << "version mismatch or no version hint; expected version:" <<
            staticItemRepositoryVersion();
        return true;
    }

    QFile crashesFile(dir.filePath(QStringLiteral("crash_counter")));
    if (crashesFile.open(QIODevice::ReadOnly)) {
        int count;
        QDataStream stream(&crashesFile);
        stream >> count;

        qCDebug(SERIALIZATION) << "current count of crashes: " << count;

        if (count >= crashesBeforeCleanup && !getenv("DONT_CLEAR_DUCHAIN_DIR")) {
            bool userAnswer = askUser(i18np("The previous session crashed.", "Session crashed %1 times in a row.",
                                            count),
                                      i18nc("tty action", "Clear cache"),
                                      i18nc("@title", "Session Crashed"),
                                      i18n("The crash may be caused by a corruption of cached data.\n\n"
                                           "Press Clear if you want KDevelop to clear the cache, otherwise press Continue if you are sure the crash has another origin."),
                                      i18nc("@action:button", "Clear Cache"),
                                      i18nc("@action:button", "Continue"));
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
class ItemRepositoryRegistryPrivate
{
public:
    bool m_shallDelete = false;
    bool m_wasShutdown = false;
    QString m_path;
    // TODO: is the order of repositories important? If not, store them in a QSet rather than std::set.
    std::set<AbstractItemRepository*> m_repositories;
    QMap<QString, QAtomicInt*> m_customCounters;
    mutable QRecursiveMutex m_mutex;

    /// @param path  A shared directory-path that the item-repositories are to be loaded from.
    /// @note        Currently the given path must reference a hidden directory, just to make sure we're
    ///              not accidentally deleting something important.
    explicit ItemRepositoryRegistryPrivate(const QString& path);

    QAtomicInt& customCounter(const QString& identity, int initialValue);
    void lockForWriting();
    void unlockForWriting();
    void deleteDataDirectory(bool recreate = true);
};

//The global item-repository registry
ItemRepositoryRegistry* ItemRepositoryRegistry::m_self = nullptr;

ItemRepositoryRegistry::ItemRepositoryRegistry(const QString& repositoryPath)
    : d_ptr(new ItemRepositoryRegistryPrivate(repositoryPath))
{
}

void ItemRepositoryRegistry::initialize(const QString& repositoryPath)
{
    if (!m_self) {
        ///We intentionally leak the registry, to prevent problems in the destruction order, where
        ///the actual repositories might get deleted later than the repository registry.
        m_self = new ItemRepositoryRegistry(repositoryPath);
    }
    m_self->d_func()->m_wasShutdown = false;
    m_self->d_func()->m_shallDelete = false;
}

ItemRepositoryRegistry* ItemRepositoryRegistry::self()
{
    Q_ASSERT(m_self);
    return m_self;
}

void ItemRepositoryRegistry::deleteRepositoryFromDisk(const QString& repositoryPath)
{
    // Now, as we have only the global item-repository registry, assume that if and only if
    // the given session is ours, its cache path is used by the said global item-repository registry.
    if (m_self && !m_self->d_func()->m_wasShutdown && m_self->d_func()->m_path == repositoryPath) {
        // remove later
        m_self->d_func()->m_shallDelete = true;
    } else {
        // Otherwise, given session is not ours.
        // remove its item-repository directory directly.
        QDir(repositoryPath).removeRecursively();
    }
}

QRecursiveMutex& ItemRepositoryRegistry::mutex()
{
    Q_D(ItemRepositoryRegistry);

    return d->m_mutex;
}

QAtomicInt& ItemRepositoryRegistryPrivate::customCounter(const QString& identity, int initialValue)
{
    auto customCounterIt = m_customCounters.find(identity);
    if (customCounterIt == m_customCounters.end()) {
        customCounterIt = m_customCounters.insert(identity, new QAtomicInt(initialValue));
    }
    return **customCounterIt;
}

QAtomicInt& ItemRepositoryRegistry::customCounter(const QString& identity, int initialValue)
{
    Q_D(ItemRepositoryRegistry);

    return d->customCounter(identity, initialValue);
}

///The global item-repository registry that is used by default
ItemRepositoryRegistry& globalItemRepositoryRegistry()
{
    return *ItemRepositoryRegistry::self();
}

void ItemRepositoryRegistry::registerRepository(AbstractItemRepository* repository)
{
    Q_D(ItemRepositoryRegistry);

    QMutexLocker lock(&d->m_mutex);
    d->m_repositories.insert(repository);
    if (!d->m_path.isEmpty()) {
        // Locking the repository is documented as the caller's responsibility.
        if (!repository->open(d->m_path)) {
            d->deleteDataDirectory();
            qCritical() << "failed to open a repository";
            abort();
        }
    }
}

QString ItemRepositoryRegistry::path() const
{
    Q_D(const ItemRepositoryRegistry);

    //We cannot lock the mutex here, since this may be called with one of the repositories locked,
    //and that may lead to a deadlock when at the same time a storing is requested
    return d->m_path;
}

void ItemRepositoryRegistryPrivate::lockForWriting()
{
    QMutexLocker lock(&m_mutex);
    //Create is_writing
    QFile f(m_path + QLatin1String("/is_writing"));
    f.open(QIODevice::WriteOnly);
    f.close();
}

void ItemRepositoryRegistry::lockForWriting()
{
    Q_D(ItemRepositoryRegistry);

    d->lockForWriting();
}

void ItemRepositoryRegistryPrivate::unlockForWriting()
{
    QMutexLocker lock(&m_mutex);
    //Delete is_writing
    QFile::remove(m_path + QLatin1String("/is_writing"));
}

void ItemRepositoryRegistry::unlockForWriting()
{
    Q_D(ItemRepositoryRegistry);

    d->unlockForWriting();
}

void ItemRepositoryRegistry::unRegisterRepository(AbstractItemRepository* repository)
{
    Q_D(ItemRepositoryRegistry);

    QMutexLocker lock(&d->m_mutex);
    Q_ASSERT(d->m_repositories.count(repository) == 1);
    d->m_repositories.erase(repository);
}

//After calling this, the data-directory may be a new one
void ItemRepositoryRegistryPrivate::deleteDataDirectory(bool recreate)
{
    QMutexLocker lock(&m_mutex);

    bool result = QDir(m_path).removeRecursively();
    Q_ASSERT(result);
    Q_UNUSED(result);
    // Just recreate the directory then; leave old path (as it is dependent on appname and session only).
    if (recreate) {
        QDir().mkpath(m_path);
    }
}

ItemRepositoryRegistryPrivate::ItemRepositoryRegistryPrivate(const QString& path)
    : m_path(path)
{
    Q_ASSERT(!path.isEmpty());

    // Check if the repository shall be cleared
    if (shouldClear(path)) {
        qCWarning(SERIALIZATION) << QStringLiteral("The data-repository at %1 has to be cleared.").arg(path);
        deleteDataDirectory(false);
    }

    QDir().mkpath(path);

    QFile f(path + QLatin1String("/Counters"));
    if (f.open(QIODevice::ReadOnly)) {
        QDataStream stream(&f);

        while (!stream.atEnd()) {
            //Read in all custom counter values
            QString counterName;
            stream >> counterName;
            int counterValue;
            stream >> counterValue;
            customCounter(counterName, 0) = counterValue;
        }
    }
}

void ItemRepositoryRegistry::store()
{
    Q_D(ItemRepositoryRegistry);

    QMutexLocker lock(&d->m_mutex);
    for (auto* repository : std::as_const(d->m_repositories)) {
        std::scoped_lock repoLock(*repository);
        repository->store();
    }

    QFile versionFile(d->m_path + QStringLiteral("/version_%1").arg(staticItemRepositoryVersion()));
    if (versionFile.open(QIODevice::WriteOnly)) {
        versionFile.close();
    } else {
        qCWarning(SERIALIZATION) << "Could not open version file for writing";
    }

    //Store all custom counter values
    QFile f(d->m_path + QLatin1String("/Counters"));
    if (f.open(QIODevice::WriteOnly)) {
        f.resize(0);
        QDataStream stream(&f);
        for (QMap<QString, QAtomicInt*>::const_iterator it = d->m_customCounters.constBegin();
             it != d->m_customCounters.constEnd();
             ++it) {
            stream << it.key();
            stream << it.value()->fetchAndAddRelaxed(0);
        }
    } else {
        qCWarning(SERIALIZATION) << "Could not open counter file for writing";
    }
}

void ItemRepositoryRegistry::printAllStatistics() const
{
    Q_D(const ItemRepositoryRegistry);

    QMutexLocker lock(&d->m_mutex);
    for (auto* repository : std::as_const(d->m_repositories)) {
        std::scoped_lock repoLock(*repository);
        qCDebug(SERIALIZATION) << "statistics in" << repository->repositoryName() << ":";
        qCDebug(SERIALIZATION) << repository->printStatistics();
    }
}

int ItemRepositoryRegistry::finalCleanup()
{
    Q_D(ItemRepositoryRegistry);

    QMutexLocker lock(&d->m_mutex);
    int changed = false;
    for (auto* repository : std::as_const(d->m_repositories)) {
        std::scoped_lock repoLock(*repository);
        int added = repository->finalCleanup();
        changed += added;
        qCDebug(SERIALIZATION) << "cleaned in" << repository->repositoryName() << ":" << added;
    }

    return changed;
}

ItemRepositoryRegistry::~ItemRepositoryRegistry()
{
    Q_D(const ItemRepositoryRegistry);

    for (auto* repository : std::as_const(d->m_repositories)) {
        std::scoped_lock repoLock(*repository);
        repository->close();
    }

    for (QAtomicInt* counter : std::as_const(d->m_customCounters)) {
        delete counter;
    }
}

void ItemRepositoryRegistry::shutdown()
{
    Q_D(ItemRepositoryRegistry);

    QMutexLocker lock(&d->m_mutex);

    // FIXME: we don't close since this can trigger crashes at shutdown
    //        since some items are still referenced, e.g. in static variables
    // NOTE: ItemRepositoryRegistryPrivate::close() used to close all contained repositories and clear d->m_path
    //       under d->m_mutex lock before its code was moved into ~ItemRepositoryRegistry().
    //       If this closing is ever uncommented, d->m_path could be cleared after its use in the code below.
//   d->close();

    if (d->m_shallDelete) {
        d->deleteDataDirectory(false);
    } else {
        QFile::remove(d->m_path + QLatin1String("/crash_counter"));
    }

    d->m_wasShutdown = true;
}
}
