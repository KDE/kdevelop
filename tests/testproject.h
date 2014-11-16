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

#include "kdevplatformtestsexport.h"
#include <util/path.h>

namespace KDevelop {

/**
 * Dummy Project than can be used for Unit Tests.
 *
 * Currently only FileSet methods are implemented.
 */
class KDEVPLATFORMTESTS_EXPORT TestProject : public IProject
{
    Q_OBJECT
public:
    /**
     * @p url Path to project directory.
     */
    TestProject(const Path& url = Path(), QObject* parent = 0);
    virtual ~TestProject();
    IProjectFileManager* projectFileManager() const { return 0; }
    IBuildSystemManager* buildSystemManager() const { return 0; }
    IPlugin* managerPlugin() const { return 0; }
    IPlugin* versionControlPlugin() const { return 0; }
    ProjectFolderItem* projectItem() const;
    void setProjectItem(ProjectFolderItem* item);
    int fileCount() const { return 0; }
    ProjectFileItem* fileAt( int ) const { return 0; }
    QList<ProjectFileItem*> files() const;
    virtual QList< ProjectBaseItem* > itemsForPath(const IndexedString&) const { return QList< ProjectBaseItem* >(); }
    virtual QList< ProjectBaseItem* > itemsForUrl(const QUrl&) const { return QList< ProjectBaseItem* >(); }
    virtual QList< ProjectFileItem* > filesForPath(const IndexedString&) const { return QList<ProjectFileItem*>(); }
    KDEVPLATFORMTESTS_DEPRECATED QList<ProjectFileItem*> filesForUrl( const QUrl& ) const { return QList<ProjectFileItem*>(); }
    virtual QList< ProjectFolderItem* > foldersForPath(const IndexedString&) const { return QList<ProjectFolderItem*>(); }
    KDEVPLATFORMTESTS_DEPRECATED QList<ProjectFolderItem*> foldersForUrl( const QUrl& ) const { return QList<ProjectFolderItem*>(); }
    void reloadModel() { }
    Path projectFile() const;
    KDEVPLATFORMTESTS_DEPRECATED QUrl projectFileUrl() const;
    KSharedConfigPtr projectConfiguration() const { return m_projectConfiguration; }
    void addToFileSet( ProjectFileItem* file);
    void removeFromFileSet( ProjectFileItem* file);
    QSet<IndexedString> fileSet() const { return m_fileSet; }
    bool isReady() const { return true; }

    KDEVPLATFORMTESTS_DEPRECATED void setProjectUrl(const QUrl &url);
    void setPath(const Path& path);

    KDEVPLATFORMTESTS_DEPRECATED const QUrl folder() const;
    Path path() const;
    QString name() const { return "Test Project"; }
    virtual bool inProject(const IndexedString& path) const;
    virtual void setReloadJob(KJob* ) {}
private:
    QSet<IndexedString> m_fileSet;
    Path m_path;
    ProjectFolderItem* m_root;
    KSharedConfigPtr m_projectConfiguration;
};

/**
 * ProjectController that can clear open projects. Useful in Unit Tests.
 */
class KDEVPLATFORMTESTS_EXPORT TestProjectController : public ProjectController
{
    Q_OBJECT
public:
    TestProjectController(Core* core) : ProjectController(core) {}
    IProject* projectAt( int i ) const { return m_projects.at(i); }
    int projectCount() const { return m_projects.count(); }
    QList<IProject*> projects() const { return m_projects; }
public:
    void addProject(IProject* p);
    void takeProject(IProject* p);
    void clearProjects();
    virtual void closeProject(IProject* p);

    virtual void initialize();
private:
    QList<IProject*> m_projects;
};

}
#endif
