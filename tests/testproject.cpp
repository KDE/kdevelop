#include "testproject.h"

KDevelop::TestProject::TestProject(QObject* parent):
    IProject(parent),
    m_projectConfiguration(KGlobal::config())
{

}

void KDevelop::TestProject::set_projectFileUrl(const KUrl& url) {
    m_projectFileUrl = url;
}

#include "testproject.moc"
