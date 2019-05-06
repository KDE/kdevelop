/* This file is part of KDevelop
    Copyright 2009  Radu Benea <radub82@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPLATFORM_FILEMANAGERLISTJOB_H
#define KDEVPLATFORM_FILEMANAGERLISTJOB_H

#include <KIO/Job>
#include <QQueue>

#include <mutex>

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
    std::recursive_mutex m_listing;

#ifdef TIME_IMPORT_JOB
    QElapsedTimer m_timer;
    QElapsedTimer m_subTimer;
    qint64 m_subWaited = 0;
#endif
};

}

#endif // KDEVPLATFORM_FILEMANAGERLISTJOB_H
