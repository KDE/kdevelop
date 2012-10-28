#include "testproject.h"

KDevelop::TestProject::TestProject(QObject* parent):
    IProject(parent),
    m_projectConfiguration(KGlobal::config())
{

}

void KDevelop::TestProject::set_projectFileUrl(const KUrl& url) {
    m_projectFileUrl = url;
}

const KUrl KDevelop::TestProject::folder() const
{
    KUrl ret = m_projectFileUrl.upUrl();
    ret.adjustPath(KUrl::AddTrailingSlash);
    return ret;
}

bool KDevelop::TestProject::inProject(const KUrl& url) const
{
    return folder().isParentOf(url);
}

void KDevelop::TestProject::addToFileSet(const KDevelop::IndexedString& file)
{
    if (!m_fileSet.contains(file)) {
        m_fileSet.insert(file);
        emit fileAddedToSet(this, file);
    }
}

void KDevelop::TestProject::removeFromFileSet(const KDevelop::IndexedString& file)
{
    if (m_fileSet.remove(file)) {
        emit fileRemovedFromSet(this, file);
    }
}

void KDevelop::TestProjectController::addProject(KDevelop::IProject* p)
{
    emit projectAboutToBeOpened(p);
    p->setParent(this);
    m_projects << p;
    emit projectOpened(p);
}

void KDevelop::TestProjectController::clearProjects()
{
    foreach(KDevelop::IProject* p, m_projects) {
        closeProject(p);
    }
}

void KDevelop::TestProjectController::closeProject(KDevelop::IProject* p)
{
    emit projectClosing(p);
    delete p;
    emit projectClosed(p);
    m_projects.removeOne(p);
}

void KDevelop::TestProjectController::takeProject(KDevelop::IProject* p)
{
    m_projects.removeOne(p);
}

#include "testproject.moc"
