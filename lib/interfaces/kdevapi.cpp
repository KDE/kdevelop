/* This file is part of the KDE project
   Copyright (C) 2000-2001 Bernd Gehrmann <bernd@kdevelop.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <kdebug.h>

#include "kdevapi.h"
#include "kdevversioncontrol.h"
#include "kdevcoderepository.h"


///////////////////////////////////////////////////////////////////////////////
// Private types
///////////////////////////////////////////////////////////////////////////////

typedef QMap<QString,KDevVersionControl*> VersionControlMap;

///////////////////////////////////////////////////////////////////////////////
// class KDevApiPrivate
///////////////////////////////////////////////////////////////////////////////

class KDevApiPrivate
{
public:

  KDevApiPrivate()
    : m_projectDom(0), m_project(0), m_makeFrontend(0),
      m_appFrontend(0), m_languageSupport(0), m_versionControl(0),
      m_diffFrontend(0), m_createFile(0), m_codeRepository(0)
  {}

  QDomDocument *m_projectDom;
  KDevProject  *m_project;
  KDevMakeFrontend *m_makeFrontend;
  KDevAppFrontend *m_appFrontend;
  KDevLanguageSupport *m_languageSupport;
  KDevVersionControl *m_versionControl;
  KDevDiffFrontend *m_diffFrontend;
  KDevCreateFile *m_createFile;
  VersionControlMap m_registeredVcs;
  KDevCodeRepository* m_codeRepository;
};

///////////////////////////////////////////////////////////////////////////////
// class KDevApi
///////////////////////////////////////////////////////////////////////////////

KDevApi::KDevApi()
{
  d = new KDevApiPrivate;
  d->m_codeRepository = new KDevCodeRepository();
}

///////////////////////////////////////////////////////////////////////////////

KDevApi::~KDevApi()
{
  delete d->m_codeRepository;
  delete d;
}

///////////////////////////////////////////////////////////////////////////////

KDevProject *KDevApi::project() const
{
  return d->m_project;
}

///////////////////////////////////////////////////////////////////////////////

void KDevApi::setProject(KDevProject *project)
{
  d->m_project = project;
}

///////////////////////////////////////////////////////////////////////////////

KDevMakeFrontend *KDevApi::makeFrontend() const
{
  return d->m_makeFrontend;
}

///////////////////////////////////////////////////////////////////////////////

void KDevApi::setMakeFrontend(KDevMakeFrontend *makeFrontend)
{
  d->m_makeFrontend = makeFrontend;
}

///////////////////////////////////////////////////////////////////////////////

KDevAppFrontend *KDevApi::appFrontend() const
{
  return d->m_appFrontend;
}

///////////////////////////////////////////////////////////////////////////////

void KDevApi::setAppFrontend(KDevAppFrontend *appFrontend)
{
  d->m_appFrontend = appFrontend;
}

///////////////////////////////////////////////////////////////////////////////

KDevLanguageSupport *KDevApi::languageSupport() const
{
  return d->m_languageSupport;
}

///////////////////////////////////////////////////////////////////////////////

void KDevApi::setLanguageSupport(KDevLanguageSupport *languageSupport)
{
  d->m_languageSupport = languageSupport;
}

///////////////////////////////////////////////////////////////////////////////

KDevVersionControl *KDevApi::versionControl() const
{
    return d->m_versionControl;
}

///////////////////////////////////////////////////////////////////////////////

void KDevApi::setVersionControl( KDevVersionControl *vcs )
{
    d->m_versionControl = vcs;
}

///////////////////////////////////////////////////////////////////////////////

void KDevApi::registerVersionControl( KDevVersionControl *vcs )
{
    d->m_registeredVcs.insert( vcs->uid(), vcs );
}

///////////////////////////////////////////////////////////////////////////////

void KDevApi::unregisterVersionControl( KDevVersionControl *vcs )
{
    if( vcs == d->m_versionControl )
    	setVersionControl( 0 );
    d->m_registeredVcs.remove( vcs->uid() );
}

///////////////////////////////////////////////////////////////////////////////

QStringList KDevApi::registeredVersionControls() const
{
    QStringList foundVersionControls;

        // We query for all vcs plugins for KDevelop
    const VersionControlMap &availableVcs = d->m_registeredVcs;

    kdDebug( 9000 ) << "  ** Starting examining services ..." << endl;

    for(VersionControlMap::const_iterator it( availableVcs.begin() ); it != availableVcs.end(); ++it)
    {
        KDevVersionControl *vcs = (*it);
        foundVersionControls.append( vcs->uid() );
        kdDebug( 9000 ) << "  =====> Found VCS: " << vcs->uid() << endl;
    }

    return foundVersionControls;
}

///////////////////////////////////////////////////////////////////////////////

KDevVersionControl *KDevApi::versionControlByName( const QString &uid ) const
{
    return d->m_registeredVcs[ uid ];
}

///////////////////////////////////////////////////////////////////////////////

KDevDiffFrontend *KDevApi::diffFrontend() const
{
  return d->m_diffFrontend;
}

///////////////////////////////////////////////////////////////////////////////

void KDevApi::setDiffFrontend(KDevDiffFrontend *diffFrontend)
{
  d->m_diffFrontend = diffFrontend;
}

///////////////////////////////////////////////////////////////////////////////

QDomDocument *KDevApi::projectDom() const
{
  return d->m_projectDom;
}

///////////////////////////////////////////////////////////////////////////////

void KDevApi::setProjectDom(QDomDocument *dom)
{
  d->m_projectDom = dom;
}

///////////////////////////////////////////////////////////////////////////////

void KDevApi::setCreateFile(KDevCreateFile *createFile)
{
  d->m_createFile = createFile;
}

///////////////////////////////////////////////////////////////////////////////

KDevCreateFile *KDevApi::createFile() const
{
  return d->m_createFile;
}

///////////////////////////////////////////////////////////////////////////////

KDevCodeRepository * KDevApi::codeRepository( ) const
{
  return d->m_codeRepository;
}

#include "kdevapi.moc"

