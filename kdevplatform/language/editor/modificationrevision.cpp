/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "modificationrevision.h"

#include <QString>
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>

#include <serialization/indexedstring.h>
#include "modificationrevisionset.h"

/// @todo Listen to filesystem changes (together with the project manager)
/// and call fileModificationCache().clear(...) when a file has changed

const int KDevelop::cacheModificationTimesForSeconds = 30;

using namespace KDevelop;
namespace
{
struct FileModificationCache
{
    QDateTime m_readTime;
    QDateTime m_modificationTime;
};
}
Q_DECLARE_TYPEINFO(FileModificationCache, Q_MOVABLE_TYPE);

namespace
{
using FileModificationMap = QHash<IndexedString, FileModificationCache>;
using OpenDocumentRevisionsMap = QHash<IndexedString, int>;

// data protected by the mutex in the StaticCacheData below
struct CacheData {
    FileModificationMap fileModificationCache;
    OpenDocumentRevisionsMap openRevisionsCache;

    QDateTime fileModificationTimeCached(const IndexedString& fileName)
    {
        const auto currentTime = QDateTime::currentDateTimeUtc();

        auto it = fileModificationCache.constFind(fileName);
        if (it != fileModificationCache.constEnd()) {
            /// Use the cache for X seconds
            if (it.value().m_readTime.secsTo(currentTime) < cacheModificationTimesForSeconds) {
                return it.value().m_modificationTime;
            }
        }

        QFileInfo fileInfo(fileName.str());
        FileModificationCache data = { currentTime, fileInfo.lastModified() };
        fileModificationCache.insert(fileName, data);
        return data.m_modificationTime;
    }

    ModificationRevision revisionForFile(const IndexedString& url)
    {
        ModificationRevision ret(fileModificationTimeCached(url));

        OpenDocumentRevisionsMap::const_iterator it = openRevisionsCache.constFind(url);
        if (it != openRevisionsCache.constEnd()) {
            ret.revision = it.value();
        }

        return ret;
    }
};

// protects CacheData with a mutex and only allows thread safe access
class StaticCacheData
{
public:
    template <typename Op>
    auto op(Op&& op)
    {
        QMutexLocker lock(&m_mutex);
        return op(m_cacheData);
    }

private:
    QMutex m_mutex;
    CacheData m_cacheData;
};

StaticCacheData& cacheData()
{
    static StaticCacheData cacheData;
    return cacheData;
}
}

void ModificationRevision::clearModificationCache(const IndexedString& fileName)
{
    ///@todo Make the cache management more clever (don't clear the whole)
    ModificationRevisionSet::clearCache();

    cacheData().op([&fileName](CacheData& data) { data.fileModificationCache.remove(fileName); });
}

ModificationRevision ModificationRevision::revisionForFile(const IndexedString& url)
{
    return cacheData().op([&url](CacheData& data) { return data.revisionForFile(url); });
}

void ModificationRevision::clearEditorRevisionForFile(const IndexedString& url)
{
    ModificationRevisionSet::clearCache(); ///@todo Make the cache management more clever (don't clear the whole)

    return cacheData().op([&url](CacheData& data) { data.openRevisionsCache.remove(url); });
}

void ModificationRevision::setEditorRevisionForFile(const IndexedString& url, int revision)
{
    ModificationRevisionSet::clearCache(); ///@todo Make the cache management more clever (don't clear the whole)

    return cacheData().op([&url, revision](CacheData& data) {
        data.openRevisionsCache.insert(url, revision);
        Q_ASSERT(data.revisionForFile(url).revision == revision);
    });
}

ModificationRevision::ModificationRevision(const QDateTime& modTime, int revision_)
    : modificationTime(modTime.toSecsSinceEpoch())
    , revision(revision_)
{
}

bool ModificationRevision::operator <(const ModificationRevision& rhs) const
{
    return modificationTime < rhs.modificationTime ||
           (modificationTime == rhs.modificationTime && revision < rhs.revision);
}

bool ModificationRevision::operator ==(const ModificationRevision& rhs) const
{
    return modificationTime == rhs.modificationTime && revision == rhs.revision;
}

bool ModificationRevision::operator !=(const ModificationRevision& rhs) const
{
    return modificationTime != rhs.modificationTime || revision != rhs.revision;
}

QString ModificationRevision::toString() const
{
    return QStringLiteral("%1 (rev %2)")
        .arg(QDateTime::fromSecsSinceEpoch(modificationTime, QTimeZone::LocalTime).time().toString())
        .arg(revision);
}
