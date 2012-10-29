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

#include <KGlobal>

#include <project/projectmodel.h>

#include <interfaces/icore.h>

using namespace KDevelop;

TestProject::TestProject(QObject* parent)
: IProject(parent)
, m_projectFileUrl(KUrl("file:///tmp/kdev-testproject/kdev-testproject.kdev4"))
, m_folder(KUrl("file:///tmp/kdev-testproject"))
, m_root(new ProjectFolderItem(this, m_folder))
, m_projectConfiguration(KGlobal::config())
{
    ICore::self()->projectController()->projectModel()->appendRow( m_root );
}

TestProject::~TestProject()
{
    ICore::self()->projectController()->projectModel()->removeRow( m_root->row() );
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
        m_folder.clear();
        m_projectFileUrl.clear();
    }
    if (item) {
        m_root = item;
        m_folder = item->url();
        m_projectFileUrl = KUrl(m_folder, m_folder.fileName() + ".kdev4");
        ICore::self()->projectController()->projectModel()->appendRow( m_root );
    }
}

KUrl TestProject::projectFileUrl() const
{
    return m_projectFileUrl;
}

const KUrl TestProject::folder() const
{
    return m_folder;
}

bool TestProject::inProject(const KUrl& url) const
{
    return folder().isParentOf(url);
}

void TestProject::addToFileSet(const IndexedString& file)
{
    if (!m_fileSet.contains(file)) {
        m_fileSet.insert(file);
        emit fileAddedToSet(this, file);
    }
}

void TestProject::removeFromFileSet(const IndexedString& file)
{
    if (m_fileSet.remove(file)) {
        emit fileRemovedFromSet(this, file);
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

#include "testproject.moc"
