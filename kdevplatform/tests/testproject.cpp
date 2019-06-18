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

#include <project/projectmodel.h>
#include <interfaces/icore.h>

#include <QDir>

using namespace KDevelop;

TestProject::TestProject(const Path& path, QObject* parent)
    : IProject(parent)
    , m_projectConfiguration(KSharedConfig::openConfig())
{
    m_path = path.isValid() ? path : Path(QDir::tempPath() + QLatin1String("/kdev-testproject/"));
    m_root = new ProjectFolderItem(this, m_path);
    ICore::self()->projectController()->projectModel()->appendRow(m_root);
}

void TestProject::setPath(const Path& path)
{
    m_path = path;
    if (m_root) {
        m_root->setPath(path);
    }
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
        ICore::self()->projectController()->projectModel()->removeRow(m_root->row());
        m_root = nullptr;
        m_path.clear();
    }
    if (item) {
        m_root = item;
        m_path = item->path();
        ICore::self()->projectController()->projectModel()->appendRow(m_root);
    }
}

Path TestProject::projectFile() const
{
    return Path(m_path, m_path.lastPathSegment() + QLatin1String(".kdev4"));
}

Path TestProject::path() const
{
    return m_path;
}

bool TestProject::inProject(const IndexedString& path) const
{
    return m_path.isParentOf(Path(path.str()));
}

void findFileItems(ProjectBaseItem* root, QList<ProjectFileItem*>& items, const Path& path = {})
{
    const auto children = root->children();
    for (ProjectBaseItem* item : children) {
        if (item->file() && (path.isEmpty() || item->path() == path)) {
            items << item->file();
        }
        if (item->rowCount()) {
            findFileItems(item, items, path);
        }
    }
}

QList<ProjectFileItem*> TestProject::files() const
{
    QList<ProjectFileItem*> ret;
    findFileItems(m_root, ret);
    return ret;
}

QList<ProjectFileItem*> TestProject::filesForPath(const IndexedString& path) const
{
    QList<ProjectFileItem*> ret;
    findFileItems(m_root, ret, Path(path.toUrl()));
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

void TestProjectController::initialize()
{
}
