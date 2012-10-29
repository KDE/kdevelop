#include "testproject.h"
#include <project/projectmodel.h>

using namespace KDevelop;

TestProject::TestProject(QObject* parent)
: IProject(parent)
, m_projectFileUrl(KUrl("file:///tmp/kdev-testproject/kdev-testproject.kdev4"))
, m_folder(KUrl("file:///tmp/kdev-testproject"))
, m_root(new ProjectFolderItem(this, m_folder))
, m_projectConfiguration(KGlobal::config())
{
}

ProjectFolderItem* TestProject::projectItem() const
{
    return m_root;
}

void TestProject::setProjectItem(ProjectFolderItem* item)
{
    Q_ASSERT(item);
    delete m_root;
    m_root = item;
    m_folder = item->url();
    m_projectFileUrl = KUrl(m_folder, m_folder.fileName() + ".kdev4");
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
