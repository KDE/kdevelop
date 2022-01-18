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

#include <KTextEditor/Document>

/// @todo Listen to filesystem changes (together with the project manager)
/// and call fileModificationCache().clear(...) when a file has changed

using namespace KDevelop;

const int KDevelop::cacheModificationTimesForSeconds = 30;

QMutex* fileModificationTimeCacheMutex()
{
    static QMutex mutex;
    return &mutex;
}

struct FileModificationCache
{
    QDateTime m_readTime;
    QDateTime m_modificationTime;
};
Q_DECLARE_TYPEINFO(FileModificationCache, Q_MOVABLE_TYPE);

using FileModificationMap = QHash<KDevelop::IndexedString, FileModificationCache>;

FileModificationMap& fileModificationCache()
{
    static FileModificationMap cache;
    return cache;
}

using OpenDocumentRevisionsMap = QHash<KDevelop::IndexedString, int>;

OpenDocumentRevisionsMap& openDocumentsRevisionMap()
{
    static OpenDocumentRevisionsMap map;
    return map;
}

QDateTime fileModificationTimeCached(const IndexedString& fileName)
{
    const auto currentTime = QDateTime::currentDateTimeUtc();

    auto it = fileModificationCache().constFind(fileName);
    if (it != fileModificationCache().constEnd()) {
        ///Use the cache for X seconds
        if (it.value().m_readTime.secsTo(currentTime) < cacheModificationTimesForSeconds) {
            return it.value().m_modificationTime;
        }
    }

    QFileInfo fileInfo(fileName.str());
    FileModificationCache data = {currentTime, fileInfo.lastModified()};
    fileModificationCache().insert(fileName, data);
    return data.m_modificationTime;
}

void ModificationRevision::clearModificationCache(const IndexedString& fileName)
{
    ///@todo Make the cache management more clever (don't clear the whole)
    ModificationRevisionSet::clearCache();

    QMutexLocker lock(fileModificationTimeCacheMutex());

    fileModificationCache().remove(fileName);
}

static ModificationRevision revisionForFile_locked(const IndexedString& url, const QMutexLocker& lock)
{
    Q_ASSERT(lock.mutex() == fileModificationTimeCacheMutex());

    ModificationRevision ret(fileModificationTimeCached(url));

    OpenDocumentRevisionsMap::const_iterator it = openDocumentsRevisionMap().constFind(url);
    if (it != openDocumentsRevisionMap().constEnd()) {
        ret.revision = it.value();
    }

    return ret;
}

ModificationRevision ModificationRevision::revisionForFile(const IndexedString& url)
{
    QMutexLocker lock(fileModificationTimeCacheMutex());
    return revisionForFile_locked(url, lock);
}

void ModificationRevision::clearEditorRevisionForFile(const KDevelop::IndexedString& url)
{
    ModificationRevisionSet::clearCache(); ///@todo Make the cache management more clever (don't clear the whole)

    QMutexLocker lock(fileModificationTimeCacheMutex());
    openDocumentsRevisionMap().remove(url);
}

void ModificationRevision::setEditorRevisionForFile(const KDevelop::IndexedString& url, int revision)
{
    ModificationRevisionSet::clearCache(); ///@todo Make the cache management more clever (don't clear the whole)

    QMutexLocker lock(fileModificationTimeCacheMutex());
    openDocumentsRevisionMap().insert(url, revision);
    Q_ASSERT(revisionForFile_locked(url, lock).revision == revision);
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
    return QStringLiteral("%1 (rev %2)").arg(QDateTime::fromSecsSinceEpoch(modificationTime, Qt::LocalTime).time().toString()).arg(revision);
}
