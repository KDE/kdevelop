/***************************************************************************
 *   Copyright 2010 Niko Sams <niko.sams@gmail.com>                        *
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

#include <KGlobal>

#include "kdevplatformtestsexport.h"
#include "../interfaces/iproject.h"
#include "../shell/projectcontroller.h"
#include "../language/duchain/indexedstring.h"

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
    TestProject(QObject* parent = 0);
    IProjectFileManager* projectFileManager() const { return 0; }
    IBuildSystemManager* buildSystemManager() const { return 0; }
    IPlugin* managerPlugin() const { return 0; }
    IPlugin* versionControlPlugin() const { return 0; }
    ProjectFolderItem* projectItem() const { return 0; }
    int fileCount() const { return 0; }
    ProjectFileItem* fileAt( int ) const { return 0; }
    QList<ProjectFileItem*> files() const { return QList<ProjectFileItem*>(); }
    QList<ProjectFileItem*> filesForUrl( const KUrl& ) const { return QList<ProjectFileItem*>(); }
    QList<ProjectFolderItem*> foldersForUrl( const KUrl& ) const { return QList<ProjectFolderItem*>(); }
    void reloadModel() { }
    KUrl projectFileUrl() const { return m_projectFileUrl; }
    KSharedConfig::Ptr projectConfiguration() const { return m_projectConfiguration; }
    void addToFileSet( const IndexedString& file);
    void removeFromFileSet( const IndexedString& file);
    QSet<IndexedString> fileSet() const { return m_fileSet; }
    bool isReady() const { return true; }
    virtual QList< ProjectBaseItem* > itemsForUrl(const KUrl&) const { return QList< ProjectBaseItem* >(); }

    void set_projectFileUrl(const KUrl& url);
public Q_SLOTS:
    const KUrl folder() const;
    QString name() const { return "Test Project"; }
    KUrl relativeUrl(const KUrl& ) const { return KUrl(); }
    bool inProject(const KUrl &) const;
private:
    QSet<IndexedString> m_fileSet;
    KUrl m_projectFileUrl;
    KSharedConfig::Ptr m_projectConfiguration;
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

private:
    QList<IProject*> m_projects;
};

}
#endif
