/* KDevelop CMake Support
 *
 * Copyright 2013 Aleix Pol <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef CMAKEIMPORTJOB_H
#define CMAKEIMPORTJOB_H

#include <KJob>

template<class T>class QFutureWatcher;
class KUrl;
class WaitAllJobs;
class CMakeManager;
class CMakeFolderItem;
class CMakeCommitChangesJob;
struct CMakeProjectData;
namespace KDevelop
{
    class IProject;
    class ProjectFolderItem;
    class ReferencedTopDUContext;
}

class CMakeImportJob : public KJob
{
    Q_OBJECT
    public:
        CMakeImportJob(KDevelop::ProjectFolderItem* dom, CMakeManager* parent);

        virtual void start();

    private slots:
        void waitFinished(KJob* job);

    private:
        void initialize();
        CMakeCommitChangesJob* importDirectory(KDevelop::IProject* project, const KUrl& url, const KDevelop::ReferencedTopDUContext& parentTop);
        KDevelop::ReferencedTopDUContext initializeProject(CMakeFolderItem*);
        KDevelop::ReferencedTopDUContext includeScript(const QString& file, const QString& currentDir, KDevelop::ReferencedTopDUContext parent);

        KDevelop::IProject* m_project;
        KDevelop::ProjectFolderItem* m_dom;
        CMakeProjectData* m_data;
        CMakeManager* m_manager;
        WaitAllJobs* m_wjob;
        QFutureWatcher<void>* m_futureWatcher;
};

#endif // CMAKEIMPORTJOB_H
