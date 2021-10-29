/*
    SPDX-FileCopyrightText: 2009 Radu Benea <radub82@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_FILEMANAGERLISTJOB_H
#define KDEVPLATFORM_FILEMANAGERLISTJOB_H

#include <KIO/Job>
#include <QQueue>
#include <QSemaphore>

// uncomment to time import jobs
// #define TIME_IMPORT_JOB

#ifdef TIME_IMPORT_JOB
#include <QElapsedTimer>
#endif


namespace KDevelop
{
class ProjectFolderItem;
class ProjectBaseItem;

class FileManagerListJob : public KIO::Job
{
    Q_OBJECT

public:
    explicit FileManagerListJob(ProjectFolderItem* item);
    virtual ~FileManagerListJob();

    void addSubDir(ProjectFolderItem* item);
    void handleRemovedItem(ProjectBaseItem* item);

    void abort();
    void start() override;

Q_SIGNALS:
    void entries(FileManagerListJob* job, ProjectFolderItem* baseItem,
                 const KIO::UDSEntryList& entries);
    void nextJob();

private Q_SLOTS:
    void slotEntries(KIO::Job* job, const KIO::UDSEntryList& entriesIn );
    void slotResult(KJob* job) override;
    void handleResults(const KIO::UDSEntryList& entries);
    void startNextJob();

private:

    QQueue<ProjectFolderItem*> m_listQueue;
    /// current base dir
    ProjectFolderItem* m_item;
    KIO::UDSEntryList entryList;
    // kill does not delete the job instantaneously
    QAtomicInt m_aborted;
    QSemaphore m_listing;

#ifdef TIME_IMPORT_JOB
    QElapsedTimer m_timer;
    QElapsedTimer m_subTimer;
    qint64 m_subWaited = 0;
#endif
};

}

#endif // KDEVPLATFORM_FILEMANAGERLISTJOB_H
