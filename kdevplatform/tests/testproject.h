/***************************************************************************
 *   Copyright 2010 Niko Sams <niko.sams@gmail.com>                        *
 *   Copyright 2012 Milian Wolff <mail@milianw.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_TEST_PROJECT_H
#define KDEVPLATFORM_TEST_PROJECT_H

#include <QSet>

#include <interfaces/iproject.h>

#include <serialization/indexedstring.h>
#include <shell/projectcontroller.h>

#include "testsexport.h"
#include <util/path.h>

namespace KDevelop {
/**
 * Dummy Project than can be used for Unit Tests.
 *
 * Currently only FileSet methods are implemented.
 */
class KDEVPLATFORMTESTS_EXPORT TestProject
    : public IProject
{
    Q_OBJECT

public:
    /**
     * @p url Path to project directory.
     */
    explicit TestProject(const Path& url = Path(), QObject* parent = nullptr);
    ~TestProject() override;
    IProjectFileManager* projectFileManager() const override { return nullptr; }
    IBuildSystemManager* buildSystemManager() const override { return nullptr; }
    IPlugin* managerPlugin() const override { return nullptr; }
    IPlugin* versionControlPlugin() const override { return nullptr; }
    ProjectFolderItem* projectItem() const override;
    void setProjectItem(ProjectFolderItem* item);
    int fileCount() const { return 0; }
    ProjectFileItem* fileAt(int) const { return nullptr; }
    QList<ProjectFileItem*> files() const;
    QList<ProjectBaseItem*> itemsForPath(const IndexedString&) const override { return QList<ProjectBaseItem*>(); }
    QList<ProjectFileItem*> filesForPath(const IndexedString&) const override;
    QList<ProjectFolderItem*> foldersForPath(const IndexedString&) const override
    {
        return QList<ProjectFolderItem*>();
    }
    void reloadModel() override { }
    void close() override {}
    Path projectFile() const override;
    KSharedConfigPtr projectConfiguration() const override { return m_projectConfiguration; }
    void addToFileSet(ProjectFileItem* file) override;
    void removeFromFileSet(ProjectFileItem* file) override;
    QSet<IndexedString> fileSet() const override { return m_fileSet; }
    bool isReady() const override { return true; }

    void setPath(const Path& path);

    Path path() const override;
    QString name() const override { return QStringLiteral("Test Project"); }
    bool inProject(const IndexedString& path) const override;
    void setReloadJob(KJob*) override {}

private:
    QSet<IndexedString> m_fileSet;
    Path m_path;
    ProjectFolderItem* m_root = nullptr;
    KSharedConfigPtr m_projectConfiguration;
};

/**
 * ProjectController that can clear open projects. Useful in Unit Tests.
 */
class KDEVPLATFORMTESTS_EXPORT TestProjectController
    : public ProjectController
{
    Q_OBJECT

public:
    explicit TestProjectController(Core* core) : ProjectController(core) {}

public:
    using ProjectController::addProject;
    using ProjectController::takeProject;

    void initialize() override;
};
}
#endif
