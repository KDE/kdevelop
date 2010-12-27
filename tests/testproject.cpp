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

#include "testproject.moc"
