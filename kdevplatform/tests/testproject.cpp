/*
    SPDX-FileCopyrightText: 2010 Niko Sams <niko.sams@gmail.com>
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

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
        m_path = Path{};
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

#include "moc_testproject.cpp"
