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
#include <cmaketypes.h>
#include <project/path.h>

namespace KDevelop {
    class IProject;
    class ProjectTargetItem;
    class ProjectFolderItem;
}
class CMakeManager;
class CMakeFolderItem;
struct CMakeProjectData;

struct ProcessedTarget
{
    Target target;
    QStringList includes;
    QStringList defines;
    QString outputName;
    KDevelop::Path location;
};
Q_DECLARE_TYPEINFO(ProcessedTarget, Q_MOVABLE_TYPE);

class CMakeCommitChangesJob : public KJob
{
Q_OBJECT
public:
    explicit CMakeCommitChangesJob(const KDevelop::Path& url, CMakeManager* manager, KDevelop::IProject* project);

    KDevelop::Path::List addProjectData(const CMakeProjectData& data);
    void setFindParentItem(bool find);
    virtual void start();

public slots:
    void reloadFiles();
    void folderAvailable(KDevelop::ProjectFolderItem* item);

signals:
    void folderCreated(KDevelop::ProjectFolderItem* item);

private slots:
    void makeChanges();

private:
    void reloadFiles(KDevelop::ProjectFolderItem* item);
    void setTargetFiles(KDevelop::ProjectTargetItem* target, const KDevelop::Path::List& files);

    KDevelop::Path m_path;
    QVector<Subdirectory> m_subdirectories;
    QVector<ProcessedTarget> m_targets;
    QVector<Test> m_tests;
    KDevelop::IProject* m_project;
    CMakeManager* m_manager;

    QStringList m_directories;
    CMakeDefinitions m_definitions;
    bool m_projectDataAdded;
    KDevelop::ProjectFolderItem* m_parentItem;
    bool m_waiting;
    bool m_reloadFiles;
    bool m_findParent;
};

#endif // CMAKECOMMITCHANGESJOB_H
