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
#include <QtCore/QQueue>

// uncomment to time imort jobs
// #define TIME_IMPORT_JOB

#ifdef TIME_IMPORT_JOB
#include <QElapsedTimer>
#endif

namespace KDevelop
{
    class ProjectFolderItem;

class FileManagerListJob : public KIO::Job
{
    Q_OBJECT

public:
    FileManagerListJob(ProjectFolderItem* item, const bool forceRecursion);
    ProjectFolderItem* item() const;

    void addSubDir(ProjectFolderItem* item);
    void removeSubDir(ProjectFolderItem* item);

    void abort();

signals:
    void entries(FileManagerListJob* job, ProjectFolderItem* baseItem,
                 const KIO::UDSEntryList& entries, const bool forceRecursion);
    void nextJob();

private slots:
    void slotEntries(KIO::Job* job, const KIO::UDSEntryList& entriesIn );
    void slotResult(KJob* job);
    void startNextJob();

private:
    QQueue<ProjectFolderItem*> m_listQueue;
    /// current base dir
    ProjectFolderItem* m_item;
    KIO::UDSEntryList entryList;
    const bool m_forceRecursion;
    // kill does not delete the job instantaniously
    bool m_aborted;

#ifdef TIME_IMPORT_JOB
    QElapsedTimer m_timer;
#endif
};

}

#endif // KDEVPLATFORM_FILEMANAGERLISTJOB_H
