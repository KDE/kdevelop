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

#ifndef FILEMANAGERLISTJOB_H
#define FILEMANAGERLISTJOB_H

#include <KIO/Job>
#include <QtCore/QQueue>

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
};

}

#endif // FILEMANAGERLISTJOB_H
