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

#ifndef GENERICMANAGERLISTJOB_H
#define GENERICMANAGERLISTJOB_H

#include <KIO/Job>
#include <QtCore/QQueue>

namespace KDevelop
{
    class IProject;
    class ProjectFolderItem;
}

class GenericManagerListJob : public KIO::Job
{
    Q_OBJECT

public:
    GenericManagerListJob(KDevelop::ProjectFolderItem* item);

    KUrl url();
    KDevelop::IProject* project();

signals:
    void entries(KDevelop::IProject* project, const KUrl& url, const KIO::UDSEntryList& entries );
    void nextJob();

private slots:
    void slotEntries(KIO::Job* job, const KIO::UDSEntryList& entriesIn );
    void slotResult(KJob* job);
    void addSubDir(KDevelop::ProjectFolderItem* item);
    void startNextJob();

private:
    QQueue<KDevelop::ProjectFolderItem*> m_listQueue;
    KUrl m_topUrl;
    KDevelop::IProject* m_project;
};

#endif // GENERICMANAGERLISTJOB_H
