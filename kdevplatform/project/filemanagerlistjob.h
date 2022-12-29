/*
    SPDX-FileCopyrightText: 2009 Radu Benea <radub82@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_FILEMANAGERLISTJOB_H
#define KDEVPLATFORM_FILEMANAGERLISTJOB_H

#include <KIO/UDSEntry>
#include <KJob>

#include <QQueue>
#include <QSemaphore>

#include <atomic>

// uncomment to time import jobs
// #define TIME_IMPORT_JOB

#ifdef TIME_IMPORT_JOB
#include <QElapsedTimer>
#endif


namespace KDevelop
{
class ProjectFolderItem;
class ProjectBaseItem;

class FileManagerListJob : public KJob
{
    Q_OBJECT

public:
    explicit FileManagerListJob(ProjectFolderItem* item);
    virtual ~FileManagerListJob();

    void addSubDir(ProjectFolderItem* item);
    void handleRemovedItem(ProjectBaseItem* item);

    void start() override;

Q_SIGNALS:
    void entries(FileManagerListJob* job, ProjectFolderItem* baseItem,
                 const KIO::UDSEntryList& entries);
    void nextJob();

protected:
    bool doKill() override;

private Q_SLOTS:
    void remoteFolderSubjobEntriesFound(KJob* job, const KIO::UDSEntryList& foundEntries);
    void remoteFolderSubjobFinished(KJob* job);
    void handleResults(const KIO::UDSEntryList& entries);
    void startNextJob();

private:
    bool isCanceled() const;

    QQueue<ProjectFolderItem*> m_listQueue;
    /// current base dir
    ProjectFolderItem* m_item;

    std::atomic<bool> m_canceled = false;

    // This data is used when the currently processed folder is remote.
    KJob* m_remoteFolderSubjob = nullptr;
    KIO::UDSEntryList entryList;

    // This data is used when the currently processed folder is local.
    QSemaphore m_listing;

#ifdef TIME_IMPORT_JOB
    QElapsedTimer m_timer;
    QElapsedTimer m_subTimer;
    qint64 m_subWaited = 0;
#endif
};

}

#endif // KDEVPLATFORM_FILEMANAGERLISTJOB_H
