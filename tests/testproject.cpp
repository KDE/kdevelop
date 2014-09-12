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

#include "testproject.h"

#include <QDebug>

#include <project/projectmodel.h>

#include <interfaces/icore.h>

using namespace KDevelop;

TestProject::TestProject(const Path& path, QObject* parent)
: IProject(parent)
, m_root(0)
, m_projectConfiguration(KSharedConfig::openConfig())
{
    m_path = path.isValid() ? path : Path("/tmp/kdev-testproject/");
    m_root = new ProjectFolderItem(this, m_path);
    ICore::self()->projectController()->projectModel()->appendRow( m_root );
}

void TestProject::setPath(const Path& path)
{
    m_path = path;
    if (m_root) {
        m_root->setPath(path);
    }
}

void TestProject::setProjectUrl(const KUrl& url)
{
    setPath(Path(url));
}

TestProject::~TestProject()
{
    if (m_root) {
        delete m_root;
    }
}

ProjectFolderItem* TestProject::projectItem() const
{
    return m_root;
}

void TestProject::setProjectItem(ProjectFolderItem* item)
{
    if (m_root) {
        ICore::self()->projectController()->projectModel()->removeRow( m_root->row() );
        m_root = 0;
        m_path.clear();
    }
    if (item) {
        m_root = item;
        m_path = item->path();
        ICore::self()->projectController()->projectModel()->appendRow( m_root );
    }
}

KUrl TestProject::projectFileUrl() const
{
    return projectFile().toUrl();
}

Path TestProject::projectFile() const
{
    return Path(m_path, m_path.lastPathSegment() + ".kdev4");
}

const KUrl TestProject::folder() const
{
    KUrl url = m_path.toUrl();
    url.adjustPath(KUrl::AddTrailingSlash);
    return url;
}

Path TestProject::path() const
{
    return m_path;
}

bool TestProject::inProject(const IndexedString& path) const
{
    return m_path.isParentOf(Path(path.str()));
}

void findFileItems(ProjectBaseItem* root, QList<ProjectFileItem*>& items)
{
    foreach(ProjectBaseItem* item, root->children()) {
        if (item->file()) {
            items << item->file();
        }
        if (item->rowCount()) {
            findFileItems(item, items);
        }
    }
}

QList< ProjectFileItem* > TestProject::files() const
{
    QList<ProjectFileItem*> ret;
    findFileItems(m_root, ret);
    return ret;
}

void TestProject::addToFileSet(ProjectFileItem* file)
{
    if (!m_fileSet.contains(file->indexedPath())) {
        m_fileSet.insert(file->indexedPath());
        emit fileAddedToSet(file);
    }
}

void TestProject::removeFromFileSet(ProjectFileItem* file)
{
    if (m_fileSet.remove(file->indexedPath())) {
        emit fileRemovedFromSet(file);
    }
}

void TestProjectController::addProject(IProject* p)
{
    emit projectAboutToBeOpened(p);
    p->setParent(this);
    m_projects << p;
    emit projectOpened(p);
}

void TestProjectController::clearProjects()
{
    foreach(IProject* p, m_projects) {
        closeProject(p);
    }
}

void TestProjectController::closeProject(IProject* p)
{
    emit projectClosing(p);
    delete p;
    m_projects.removeOne(p);
    emit projectClosed(p);
}

void KDevelop::TestProjectController::takeProject(KDevelop::IProject* p)
{
    emit projectClosing(p);
    m_projects.removeOne(p);
    emit projectClosed(p);
}

void TestProjectController::initialize()
{
}

