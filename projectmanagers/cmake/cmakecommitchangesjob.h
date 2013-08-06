/* KDevelop CMake Support
 *
 * Copyright 2013 Aleix Pol Gonzalez <aleixpol@kde.org>
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

#ifndef CMAKECOMMITCHANGESJOB_H
#define CMAKECOMMITCHANGESJOB_H

#include <KJob>
#include <KUrl>
#include <cmaketypes.h>

namespace KDevelop {
    class IProject;
    class ProjectTargetItem;
    class ProjectFolderItem;
}
class CMakeManager;
class CMakeFolderItem;
class CMakeProjectData;

struct ProcessedTarget
{
    Target target;
    QStringList includes;
    QStringList defines;
};

class CMakeCommitChangesJob : public KJob
{
Q_OBJECT
public:
    explicit CMakeCommitChangesJob(const KUrl& url, CMakeManager* manager, KDevelop::IProject* parent);

    KUrl::List addProjectData(const CMakeProjectData* data);
    virtual void start();

private slots:
    void makeChanges();

private:
    void reloadFiles(KDevelop::ProjectFolderItem* item);
    void setTargetFiles(KDevelop::ProjectTargetItem* target, const KUrl::List& files);

    KUrl m_url;
    QList<Subdirectory> m_subdirectories;
    QList<ProcessedTarget> m_targets;
    QList<Test> m_tests;
    KDevelop::IProject* m_project;
    CMakeManager* m_manager;

    QStringList m_directories;
    QStringList m_definitions;

};

#endif // CMAKECOMMITCHANGESJOB_H
