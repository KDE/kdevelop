/*
    SPDX-FileCopyrightText: 2009 Radu Benea <radub82@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "filemanagerlistjob.h"

#include <interfaces/iproject.h>
#include <project/projectmodel.h>

#include "path.h"
#include "debug.h"

#include <KIO/ListJob>

#include <QtConcurrentRun>
#include <QDir>

using namespace KDevelop;

namespace {
bool isChildItem(ProjectBaseItem* parent, ProjectBaseItem* child)
{
    do {
        if (child == parent) {
            return true;
        }
        child = child->parent();
    } while(child);
    return false;
}
}

FileManagerListJob::FileManagerListJob(ProjectFolderItem* item)
    : m_item(item)
{
    setCapabilities(Killable);

    qRegisterMetaType<KIO::UDSEntryList>("KIO::UDSEntryList");
    qRegisterMetaType<KJob*>();

    /* the following line is not an error in judgment, apparently starting a
     * listJob while the previous one hasn't self-destructed takes a lot of time,
     * so we give the job a chance to selfdestruct first */
    connect( this, &FileManagerListJob::nextJob, this, &FileManagerListJob::startNextJob, Qt::QueuedConnection );

    addSubDir(item);

#ifdef TIME_IMPORT_JOB
    m_timer.start();
#endif
}

FileManagerListJob::~FileManagerListJob()
{
    doKill();
    m_localFolderFuture.waitForFinished();
}

void FileManagerListJob::addSubDir( ProjectFolderItem* item )
{
    Q_ASSERT(!m_listQueue.contains(item));
    Q_ASSERT(!m_item || m_item == item || m_item->path().isDirectParentOf(item->path()));

    m_listQueue.enqueue(item);
}

void FileManagerListJob::handleRemovedItem(ProjectBaseItem* item)
{
    // NOTE: the item could be (partially) destroyed already, thus it's not save
    // to call e.g. item->folder to cast the base item to a folder item...
    auto *folder = reinterpret_cast<ProjectFolderItem*>(item);
    m_listQueue.removeAll(folder);

    if (isChildItem(item, m_item)) {
        kill();
    }
}

void FileManagerListJob::remoteFolderSubjobEntriesFound(KJob* job, const KIO::UDSEntryList& foundEntries)
{
    Q_UNUSED(job);
    entryList.append(foundEntries);
}

void FileManagerListJob::startNextJob()
{
    if (m_listQueue.empty() || isCanceled()) {
        return;
    }

#ifdef TIME_IMPORT_JOB
    m_subTimer.start();
#endif

    m_item = m_listQueue.dequeue();
    if (m_item->path().isLocalFile()) {
        // optimized version for local projects using QDir directly
        m_localFolderFuture = QtConcurrent::run([this] (const Path& path) {
            if (isCanceled()) {
                return;
            }
            QDir dir(path.toLocalFile());
            const auto entries = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries | QDir::Hidden);
            if (isCanceled()) {
                return;
            }
            KIO::UDSEntryList results;
            std::transform(entries.begin(), entries.end(), std::back_inserter(results), [] (const QFileInfo& info) -> KIO::UDSEntry {
                KIO::UDSEntry entry;
                entry.fastInsert(KIO::UDSEntry::UDS_NAME, info.fileName());
                if (info.isDir()) {
                    entry.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, QT_STAT_DIR);
                }
                if (info.isSymLink()) {
                    entry.fastInsert(KIO::UDSEntry::UDS_LINK_DEST, info.symLinkTarget());
                }
                return entry;
            });
            QMetaObject::invokeMethod(this, "handleResults", Q_ARG(KIO::UDSEntryList, results));
        }, m_item->path());
    } else {
        KIO::ListJob* job = KIO::listDir( m_item->path().toUrl(), KIO::HideProgressInfo );
        job->addMetaData(QStringLiteral("details"), QStringLiteral("0"));
        connect(job, &KIO::ListJob::entries, this, &FileManagerListJob::remoteFolderSubjobEntriesFound);
        connect(job, &KJob::finished, this, &FileManagerListJob::remoteFolderSubjobFinished);

        m_remoteFolderSubjob = job;
    }
}

void FileManagerListJob::remoteFolderSubjobFinished(KJob* job)
{
    if( job && job->error() ) {
        qCDebug(FILEMANAGER) << "error in list job:" << job->error() << job->errorString();
    }

    Q_ASSERT(m_remoteFolderSubjob == job);
    m_remoteFolderSubjob = nullptr;

    handleResults(entryList);
    entryList.clear();
}

void FileManagerListJob::handleResults(const KIO::UDSEntryList& entriesIn)
{
    if (isCanceled()) {
        return;
    }

#ifdef TIME_IMPORT_JOB
    {
        auto waited = m_subTimer.elapsed();
        m_subWaited += waited;
        qCDebug(PROJECT) << "TIME FOR SUB JOB:" << waited << m_subWaited;
    }
#endif

    emit entries(this, m_item, entriesIn);

    if( m_listQueue.isEmpty() ) {
        emitResult();

#ifdef TIME_IMPORT_JOB
        qCDebug(PROJECT) << "TIME FOR LISTJOB:" << m_timer.elapsed();
#endif
    } else {
        emit nextJob();
    }
}

void FileManagerListJob::start()
{
    startNextJob();
}

bool FileManagerListJob::doKill()
{
    m_canceled.store(true, std::memory_order_relaxed);
    if (m_remoteFolderSubjob) {
        m_remoteFolderSubjob->kill();
    }
    return true;
}

bool FileManagerListJob::isCanceled() const
{
    return m_canceled.load(std::memory_order_relaxed);
}

#include "moc_filemanagerlistjob.cpp"
