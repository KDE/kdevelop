/* $Id$
 *
 *  Copyright (C) 2002 Roberto Raggi (raggi@cli.di.unipi.it)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */

#include "kdevcoreimpl.h"
#include "partloader.h"
#include "classstore.h"
#include "kdevapi.h"
#include "kdevmakefrontend.h"
#include "kdevappfrontend.h"
#include "kdevproject.h"
#include "kdevlanguagesupport.h"
#include "KDevCoreIface.h"
#include "ckdevelop.h"

#include <qstatusbar.h>

#include <kparts/partmanager.h>
#include <kparts/factory.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kstatusbar.h>
#include <dcopclient.h>

KDevCoreImpl::KDevCoreImpl( CKDevelop* pDev )
    : KDevCore(),
      m_pDevelop( pDev )
{
    api = new KDevApi();
    api->core = this;
    api->classStore = new ClassStore();

    // added by daniel
    api->ccClassStore = new ClassStore( );

    api->projectDom = 0;
    api->project = 0;

    dcopIface = new KDevCoreIface(this);
    kapp->dcopClient()->registerAs( "kdevelop" );

    manager = new KParts::PartManager( m_pDevelop );
    connect( manager, SIGNAL(activePartChanged(KParts::Part*)),
             this, SLOT(activePartChanged(KParts::Part*)) );
    connect( manager, SIGNAL(partAdded(KParts::Part*)),
             this, SLOT(partCountChanged()));
    connect( manager, SIGNAL(partRemoved(KParts::Part*)),
             this, SLOT(partCountChanged()));

    initGlobalParts();

    emit coreInitialized();
}

KDevCoreImpl::~KDevCoreImpl()
{
    delete( dcopIface );
    PartLoader::cleanup();

}

void KDevCoreImpl::initGlobalParts()
{
    KService *service;
    KDevPart *part;

    // Make frontend
    KTrader::OfferList makeFrontendOffers =
        KTrader::self()->query(QString::fromLatin1("KDevelop/MakeFrontend"), QString::null);
    if (makeFrontendOffers.isEmpty())
        return;
    service = *makeFrontendOffers.begin();
    part = PartLoader::loadService(service, "KDevMakeFrontend", api, this);
    if (!part)
        return;
    initPart(api->makeFrontend = static_cast<KDevMakeFrontend*>(part));

    // App frontend
    KTrader::OfferList appFrontendOffers =
        KTrader::self()->query(QString::fromLatin1("KDevelop/AppFrontend"), QString::null);
    if (appFrontendOffers.isEmpty())
        return;
    service = *appFrontendOffers.begin();
    part = PartLoader::loadService(service, "KDevAppFrontend", api, this);
    if (!part)
        return;
    initPart(api->appFrontend = static_cast<KDevAppFrontend*>(part));

    // Global parts
    KTrader::OfferList globalOffers
        = KTrader::self()->query(QString::fromLatin1("KDevelop/Part"),
                                 QString::fromLatin1("[X-KDevelop-Scope] == 'Global'"));
    KConfig *config = KGlobal::config();
    for (KTrader::OfferList::ConstIterator it = globalOffers.begin(); it != globalOffers.end(); ++it) {
        config->setGroup("Plugins");
        if (!config->readBoolEntry((*it)->name(), true)) {
            kdDebug(9000) << "Not loading " << (*it)->name() << endl;
            continue;
        }
        part = PartLoader::loadService(*it, "KDevPart", api, this);
        if (!part)
            return;
        initPart(part);
        globalParts.append(part);
    }
}


void KDevCoreImpl::embedWidget( QWidget* w, Role r,
                                const QString& description )
{
    kdDebug() << "KDevCoreImpl::embedWidget()" << endl;
    m_pDevelop->embedToolWidget( w, r, description );
}

void KDevCoreImpl::raiseWidget( QWidget* w )
{
    kdDebug() << "KDevCoreImpl::raiseWidget()" << endl;
    w->show();
}

void KDevCoreImpl::removeWidget( QWidget* w, Role r )
{
    kdDebug() << "KDevCoreImpl::removeWidget()" << endl;
    m_pDevelop->removeToolWidget( w, r );
}

void KDevCoreImpl::fillContextMenu(QPopupMenu *popup, const Context *context)
{
    kdDebug() << "KDevCoreImpl::fillContextMenu()" << endl;
}

void KDevCoreImpl::openProject(const QString& projectFileName)
{
    kdDebug() << "KDevCoreImpl::openProject()" << endl;
    m_pDevelop->slotOpenProject( KURL(projectFileName) );
}

void KDevCoreImpl::gotoFile( const KURL &url )
{
    kdDebug() << "KDevCoreImpl::gotoFile()" << endl;
    m_pDevelop->switchToFile( url.path() );
}

void KDevCoreImpl::gotoDocumentationFile(const KURL& url,
                                         Embedding embed )
{
    kdDebug() << "KDevCoreImpl::gotoDocumentationFile()" << endl;
}

void KDevCoreImpl::gotoSourceFile(const KURL& url, int lineNum,
                                  Embedding embed )
{
    kdDebug() << "KDevCoreImpl::gotoSourceFile()" << endl;
    m_pDevelop->switchToFile( url.path(), lineNum );
}

void KDevCoreImpl::gotoExecutionPoint(const QString &fileName, int lineNum )
{
    kdDebug() << "KDevCoreImpl::gotoExecutionPoint()" << endl;
    m_pDevelop->switchToFile( fileName, lineNum );
}

void KDevCoreImpl::saveAllFiles()
{
    kdDebug() << "KDevCoreImpl::saveAllFiles()" << endl;
}

void KDevCoreImpl::revertAllFiles()
{
    kdDebug() << "KDevCoreImpl::revertAllFiles()" << endl;
}

void KDevCoreImpl::setBreakpoint(const QString &fileName, int lineNum,
                                 int id, bool enabled, bool pending)
{
    kdDebug() << "KDevCoreImpl::setBreakpoint()" << endl;
}

void KDevCoreImpl::running(KDevPart *which, bool runs)
{
    kdDebug() << "KDevPart::running()" << endl;
}

void KDevCoreImpl::message(const QString &str)
{
    kdDebug() << "KDevCoreImpl::message()" << endl;
    m_pDevelop->statusBar()->message( str, 2000 );
}

KParts::PartManager * KDevCoreImpl::partManager() const
{
    kdDebug() << "KDevCoreImpl::partManager()" << endl;
    return manager;
}

KEditor::Editor * KDevCoreImpl::editor()
{
    kdDebug() << "KDevCoreImpl::editor()" << endl;
    return 0;
}

QStatusBar * KDevCoreImpl::statusBar() const
{
    kdDebug() << "KDevCoreImpl::statusBar()" << endl;
    return m_pDevelop->statusBar();
}

void KDevCoreImpl::initPart(KDevPart* part)
{
    parts.append( part );
    m_pDevelop->guiFactory()->addClient(part);
}

void KDevCoreImpl::removePart(KDevPart* part)
{
    m_pDevelop->guiFactory()->removeClient(part);
    parts.remove(part);
    delete( part );
}

void KDevCoreImpl::removeGlobalParts()
{
    kdDebug() << "KDevCoreImpl::removeGlobalParts()" << endl;
    QPtrListIterator<KDevPart> it( globalParts );
    for (; it.current(); ++it) {
        kdDebug(9000) << "Still have part " << it.current()->name() << endl;
    }

    while (!globalParts.isEmpty()) {
        removePart( globalParts.first() );
        globalParts.removeFirst();
    }
}
