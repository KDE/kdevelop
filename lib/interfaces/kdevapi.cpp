#include <kdebug.h>

#include "kdevapi.h"
#include "kdevversioncontrol.h"

class KDevApiPrivate
{
public:

  KDevApiPrivate()
    : m_projectDom(0), m_project(0), m_makeFrontend(0),
      m_appFrontend(0), m_languageSupport(0), m_versionControl(0),
      m_diffFrontend(0), m_createFile(0), m_sourceFormatter(0)
  {}

  QDomDocument *m_projectDom;
  KDevProject  *m_project;
  KDevMakeFrontend *m_makeFrontend;
  KDevAppFrontend *m_appFrontend;
  KDevLanguageSupport *m_languageSupport;
  KDevVersionControl *m_versionControl;
  KDevDiffFrontend *m_diffFrontend;
  KDevCreateFile *m_createFile;
  KDevSourceFormatter *m_sourceFormatter;

  KDevApi::VersionControlMap m_registeredVcs;
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

void KDevApi::registerVersionControl( KDevVersionControl *vcs )
{
	d->m_registeredVcs.insert( vcs->uid(), vcs );
}

void KDevApi::unregisterVersionControl( KDevVersionControl *vcs )
{
	d->m_registeredVcs.remove( vcs->uid() );
}


QStringList KDevApi::registeredVersionControls() const
{
	QStringList foundVersionControls;

		// We query for all vcs plugins for KDevelop
	const KDevApi::VersionControlMap &availableVcs = d->m_registeredVcs;

	kdDebug( 9000 ) << "  ** Starting examining services ..." << endl;

	for(KDevApi::VersionControlMap::const_iterator it( availableVcs.begin() ); it != availableVcs.end(); ++it)
	{
		KDevVersionControl *vcs = (*it);
		foundVersionControls.append( vcs->uid() );
		kdDebug( 9000 ) << "  =====> Found VCS: " << vcs->uid() << endl;
	}

	return foundVersionControls;
}

KDevVersionControl *KDevApi::versionControlByName( const QString &uid ) const
{
	return d->m_registeredVcs[ uid ];
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


void KDevApi::setCreateFile(KDevCreateFile *createFile) {
  d->m_createFile = createFile;
}

KDevCreateFile *KDevApi::createFile() {
  return d->m_createFile;
}

KDevSourceFormatter *KDevApi::sourceFormatter()
{
  return d->m_sourceFormatter;
}

void KDevApi::setSourceFormatter(KDevSourceFormatter *sourceFormatter)
{
  d->m_sourceFormatter = sourceFormatter;
}

#include "kdevapi.moc"

