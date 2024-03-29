/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "modificationrevisionset.h"
#include <debug.h>

#include <serialization/itemrepository.h>
#include <serialization/indexedstring.h>
#include <util/setrepository.h>

//When uncommented, the reason for needed updates is printed
// #define DEBUG_NEEDSUPDATE

namespace KDevelop {
QRecursiveMutex* modificationRevisionSetMutex()
{
    static QRecursiveMutex mutex;
    return &mutex;
}

struct FileModificationPair
{
    KDevelop::IndexedString file;
    KDevelop::ModificationRevision revision;

    FileModificationPair()
    {
    }

    FileModificationPair(const KDevelop::IndexedString& _file, KDevelop::ModificationRevision _revision)
        : file(_file)
        , revision(_revision)
    {
    }

    FileModificationPair& operator=(const FileModificationPair& rhs) = delete;

    unsigned int hash() const
    {
        return ((file.hash() + revision.modificationTime) * 17 + revision.revision) * 73;
    }

    unsigned short int itemSize() const
    {
        return sizeof(FileModificationPair);
    }

    bool operator==(const FileModificationPair& rhs) const
    {
        return file == rhs.file && revision == rhs.revision;
    }
};

struct FileModificationPairRequest
{
    FileModificationPairRequest(const FileModificationPair& data) : m_data(data)
    {
    }

    const FileModificationPair& m_data;

    enum {
        AverageSize = sizeof(FileModificationPair)
    };

    unsigned int hash() const
    {
        return m_data.hash();
    }

    uint itemSize() const
    {
        return m_data.itemSize();
    }

    void createItem(FileModificationPair* item) const
    {
        new (item)  FileModificationPair(m_data);
    }

    bool equals(const FileModificationPair* item) const
    {
        return *item == m_data;
    }

    static void destroy(FileModificationPair* item, KDevelop::AbstractItemRepository&)
    {
        item->~FileModificationPair();
    }

    static bool persistent(const FileModificationPair* /*item*/)
    {
        return true; //Reference-counting is done implicitly using the set-repository
    }
};

using FileModificationPairRepository
    = KDevelop::ItemRepository<FileModificationPair, FileModificationPairRequest, true, QRecursiveMutex>;

static FileModificationPairRepository& fileModificationPairRepository()
{
    static FileModificationPairRepository rep(QStringLiteral("file modification repository"),
                                              modificationRevisionSetMutex());
    return rep;
}

void initModificationRevisionSetRepository()
{
    fileModificationPairRepository();
}

QHash<uint, std::pair<QDateTime, bool>> needsUpdateCache;

void ModificationRevisionSet::clearCache()
{
    QMutexLocker lock(modificationRevisionSetMutex());
    ///@todo More intelligent clearing. We actually need to watch the directory for changes, and if there are changes, clear the cache.
    needsUpdateCache.clear();
}

struct FileModificationSetRepository
    : public Utils::BasicSetRepository
{
    FileModificationSetRepository()
        : Utils::BasicSetRepository(QStringLiteral("file modification sets"), modificationRevisionSetMutex(),
                                    &globalItemRepositoryRegistry(), true)
    {
    }
    void itemRemovedFromSets(uint index) override;
};

struct FileModificationSetRepositoryRepresenter
{
    static FileModificationSetRepository& repository()
    {
        static FileModificationSetRepository fileModificationSetRepository;
        return fileModificationSetRepository;
    }
};

ModificationRevisionSet::ModificationRevisionSet(unsigned int index) : m_index(index)
{
}

uint ModificationRevisionSet::size() const
{
    Utils::Set set = Utils::Set(m_index, &FileModificationSetRepositoryRepresenter::repository());
    return set.count();
}

void ModificationRevisionSet::clear()
{
    QMutexLocker lock(modificationRevisionSetMutex());

    if (m_index) {
        Utils::Set oldModificationTimes = Utils::Set(m_index, &FileModificationSetRepositoryRepresenter::repository());
        oldModificationTimes.staticUnref();
        m_index = 0;
    }
}

void ModificationRevisionSet::addModificationRevision(const IndexedString& url,
                                                      const KDevelop::ModificationRevision& revision)
{
    QMutexLocker lock(modificationRevisionSetMutex());

    if (m_index == 0) {
        Utils::Set set = FileModificationSetRepositoryRepresenter::repository().createSet(
            fileModificationPairRepository().index(FileModificationPair(url, revision)));
        set.staticRef();
        m_index = set.setIndex();
    } else {
        Utils::Set oldModificationTimes = Utils::Set(m_index, &FileModificationSetRepositoryRepresenter::repository());
        Utils::Set newModificationTimes = oldModificationTimes;

        Utils::Set tempSet = FileModificationSetRepositoryRepresenter::repository().createSet(
            fileModificationPairRepository().index(FileModificationPair(url, revision)));
        tempSet.staticRef();

        newModificationTimes += tempSet;
        newModificationTimes.staticRef();
        oldModificationTimes.staticUnref();
        tempSet.staticUnref();

        m_index = newModificationTimes.setIndex();
    }
}

bool ModificationRevisionSet::removeModificationRevision(const IndexedString& url,
                                                         const KDevelop::ModificationRevision& revision)
{
    QMutexLocker lock(modificationRevisionSetMutex());

    if (!m_index)
        return false;

    Utils::Set oldModificationTimes = Utils::Set(m_index, &FileModificationSetRepositoryRepresenter::repository());
    Utils::Set newModificationTimes = oldModificationTimes;

    Utils::Set tempSet = FileModificationSetRepositoryRepresenter::repository().createSet(
        fileModificationPairRepository().index(FileModificationPair(url, revision)));
    tempSet.staticRef();

    newModificationTimes -= tempSet;
    newModificationTimes.staticRef();
    oldModificationTimes.staticUnref();
    tempSet.staticUnref();

    m_index = newModificationTimes.setIndex();
    return m_index != oldModificationTimes.setIndex();
}

// const QMap<IndexedString, KDevelop::ModificationRevision> ModificationRevisionSet::allModificationTimes() const {
//   QMap<IndexedString, KDevelop::ModificationRevision> ret;
//   Utils::Set::Iterator it = m_allModificationTimes.iterator();
//   while(it) {
//     const FileModificationPair* data = fileModificationPairRepository().itemFromIndex(*it);
//     ret[data->file] = data->revision;
//     ++it;
//   }
//   return ret;
// }

using ModificationRevisionSetNode = Utils::VirtualSetNode<uint, Utils::IdentityConversion<uint>,
    FileModificationSetRepositoryRepresenter>;
// static bool (const Utils::SetNodeData* node) {
//   ModificationRevisionSetNode
//   if(!node)
//     return false;
// }

static bool nodeNeedsUpdate(uint index)
{
    QMutexLocker lock(modificationRevisionSetMutex());

    if (!index)
        return false;

    const auto currentTime = QDateTime::currentDateTimeUtc();

    auto cached = needsUpdateCache.constFind(index);
    if (cached != needsUpdateCache.constEnd()) {
        if ((*cached).first.secsTo(currentTime) < cacheModificationTimesForSeconds) {
            return cached->second;
        }
    }

    bool result = false;

    const Utils::SetNodeData* nodeData = FileModificationSetRepositoryRepresenter::repository().nodeFromIndex(index);
    if (nodeData->contiguous()) {
        //Do  the actual checking
        for (unsigned int a = nodeData->start(); a < nodeData->end(); ++a) {
            const FileModificationPair* data = fileModificationPairRepository().itemFromIndex(a);
            ModificationRevision revision = KDevelop::ModificationRevision::revisionForFile(data->file);
            if (revision != data->revision) {
                result = true;
                break;
            }
        }
    } else {
        result = nodeNeedsUpdate(nodeData->leftNode()) || nodeNeedsUpdate(nodeData->rightNode());
    }

    needsUpdateCache.insert(index, std::make_pair(currentTime, result));

    return result;
}

QString ModificationRevisionSet::toString() const
{
    QMutexLocker lock(modificationRevisionSetMutex());
    Utils::Set set(m_index, &FileModificationSetRepositoryRepresenter::repository());
    Utils::Set::Iterator it = set.iterator();
    QStringList revisions;
    while (it) {
        const FileModificationPair* data = fileModificationPairRepository().itemFromIndex(*it);
        revisions.append(data->file.str() + QLatin1Char(':') + data->revision.toString());
        ++it;
    }

    QString ret = QLatin1Char('[') + revisions.join(QLatin1String(", ")) + QLatin1Char(']');
    return ret;
}

bool ModificationRevisionSet::needsUpdate() const
{
    QMutexLocker lock(modificationRevisionSetMutex());

  #ifdef DEBUG_NEEDSUPDATE
    Utils::Set set(m_index, &FileModificationSetRepositoryRepresenter::repository());
    Utils::Set::Iterator it = set.iterator();
    while (it) {
        const FileModificationPair* data = fileModificationPairRepository().itemFromIndex(*it);
        ModificationRevision revision = KDevelop::ModificationRevision::revisionForFile(data->file);
        if (revision != data->revision) {
            qCDebug(LANGUAGE) << "dependency" << data->file.str() << "has changed, stored stamp:" << data->revision <<
                "new time:" << revision;
            return true;
        }
        ++it;
    }
    return false;
  #else
    return nodeNeedsUpdate(m_index);
  #endif
}

ModificationRevisionSet& ModificationRevisionSet::operator+=(const ModificationRevisionSet& rhs)
{
    QMutexLocker lock(modificationRevisionSetMutex());

    Utils::Set oldModificationTimes = Utils::Set(m_index, &FileModificationSetRepositoryRepresenter::repository());
    Utils::Set otherModificationTimes =
        Utils::Set(rhs.m_index, &FileModificationSetRepositoryRepresenter::repository());

    Utils::Set newModificationTimes = oldModificationTimes;

    newModificationTimes += otherModificationTimes;
    newModificationTimes.staticRef();
    oldModificationTimes.staticUnref();

    m_index = newModificationTimes.setIndex();

    return *this;
}

ModificationRevisionSet& ModificationRevisionSet::operator-=(const ModificationRevisionSet& rhs)
{
    QMutexLocker lock(modificationRevisionSetMutex());

    Utils::Set oldModificationTimes = Utils::Set(m_index, &FileModificationSetRepositoryRepresenter::repository());
    Utils::Set otherModificationTimes =
        Utils::Set(rhs.m_index, &FileModificationSetRepositoryRepresenter::repository());

    Utils::Set newModificationTimes = oldModificationTimes;

    newModificationTimes -= otherModificationTimes;
    newModificationTimes.staticRef();
    oldModificationTimes.staticUnref();

    m_index = newModificationTimes.setIndex();

    return *this;
}

void FileModificationSetRepository::itemRemovedFromSets(uint index)
{
    fileModificationPairRepository().deleteItem(index);
    needsUpdateCache.remove(index);
}
}
