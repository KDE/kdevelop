#include "kdevapi.h"


class KDevApiPrivate
{
public:

  KDevApiPrivate() 
    : m_projectDom(0), m_project(0), m_makeFrontend(0),
      m_appFrontend(0), m_languageSupport(0), m_versionControl(0),
      m_diffFrontend(0)
  {}

  QDomDocument *m_projectDom;
  KDevProject  *m_project;
  KDevMakeFrontend *m_makeFrontend;
  KDevAppFrontend *m_appFrontend;
  KDevLanguageSupport *m_languageSupport;
  KDevVersionControl *m_versionControl;
  KDevDiffFrontend *m_diffFrontend;

};


KDevApi::KDevApi()
{
  d = new KDevApiPrivate;
}


KDevApi::~KDevApi()
{
  delete d;
}


KDevProject *KDevApi::project()
{
  return d->m_project;
}


void KDevApi::setProject(KDevProject *project)
{
  d->m_project = project;
}


KDevMakeFrontend *KDevApi::makeFrontend()
{
  return d->m_makeFrontend;
}


void KDevApi::setMakeFrontend(KDevMakeFrontend *makeFrontend)
{
  d->m_makeFrontend = makeFrontend;
}


KDevAppFrontend *KDevApi::appFrontend()
{
  return d->m_appFrontend;
}


void KDevApi::setAppFrontend(KDevAppFrontend *appFrontend)
{
  d->m_appFrontend = appFrontend;
}


KDevLanguageSupport *KDevApi::languageSupport()
{
  return d->m_languageSupport;
}


void KDevApi::setLanguageSupport(KDevLanguageSupport *languageSupport)
{
  d->m_languageSupport = languageSupport;
}


KDevVersionControl *KDevApi::versionControl()
{
  return d->m_versionControl;
}


void KDevApi::setVersionControl(KDevVersionControl *versionControl)
{
  d->m_versionControl = versionControl;
}

KDevDiffFrontend *KDevApi::diffFrontend()
{
  return d->m_diffFrontend;
}

void KDevApi::setDiffFrontend(KDevDiffFrontend *diffFrontend)
{
  d->m_diffFrontend = diffFrontend;
}

QDomDocument *KDevApi::projectDom() 
{
  return d->m_projectDom;
}


void KDevApi::setProjectDom(QDomDocument *dom)
{
  d->m_projectDom = dom;
}

#include "kdevapi.moc"

