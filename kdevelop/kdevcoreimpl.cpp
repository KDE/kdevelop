
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

#include <kparts/partmanager.h>
#include <kparts/factory.h>
#include <kapplication.h>
#include <kdebug.h>
#include <dcopclient.h>

KDevCoreImpl::KDevCoreImpl( CKDevelop* pDev )
    : KDevCore(),
      m_pDevelop( pDev )
{
    kdDebug() << "KDevCoreImpl::KDevCoreImpl()" << endl;
    api = new KDevApi();
    api->core = this;
    kdDebug() << "KDevCoreImpl::KDevCoreImpl() - 1" << endl;
    api->classStore = new ClassStore();
    kdDebug() << "KDevCoreImpl::KDevCoreImpl() - 2" << endl;


    // added by daniel
    api->ccClassStore = new ClassStore( );

    api->projectDom = 0;
    api->project = 0;

    kdDebug() << "-------------> before KDevCoreIface()" << endl;
    dcopIface = new KDevCoreIface(this);
    kdDebug() << "------------> dcopIface attached" << endl;
    kapp->dcopClient()->registerAs( "kdevelop" );

    manager = new KParts::PartManager( m_pDevelop );
    kdDebug() << "-----------> parts manager created" << endl;
    connect( manager, SIGNAL(activePartChanged(KParts::Part*)),
             this, SLOT(activePartChanged(KParts::Part*)) );
    connect( manager, SIGNAL(partAdded(KParts::Part*)),
             this, SLOT(partCountChanged()));
    connect( manager, SIGNAL(partRemoved(KParts::Part*)),
             this, SLOT(partCountChanged()));

    initGlobalParts();
    kdDebug() << "---------> global parts created" << endl;

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


void KDevCoreImpl::embedWidget(QWidget*, Role, const QString&)
{
    kdDebug() << "KDevCoreImpl::embedWidget()" << endl;
}

void KDevCoreImpl::raiseWidget(QWidget*)
{
    kdDebug() << "KDevCoreImpl::raiseWidget()" << endl;
}

void KDevCoreImpl::removeWidget( QWidget*, Role )
{
    kdDebug() << "KDevCoreImpl::removeWidget()" << endl;
}

void KDevCoreImpl::fillContextMenu(QPopupMenu *popup, const Context *context)
{
    kdDebug() << "KDevCoreImpl::fillContextMenu()" << endl;
}

void KDevCoreImpl::openProject(const QString& projectFileName)
{
    kdDebug() << "KDevCoreImpl::openProject()" << endl;
}

void KDevCoreImpl::gotoFile(const KURL &url)
{
    kdDebug() << "KDevCoreImpl::gotoFile()" << endl;
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
}

void KDevCoreImpl::gotoExecutionPoint(const QString &fileName, int lineNum )
{
    kdDebug() << "KDevCoreImpl::gotoExecutionPoint()" << endl;
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
    return 0;
}

void KDevCoreImpl::initPart(KDevPart* part)
{
    kdDebug() << "KDevCoreImpl::initPart() -- NOT IMLEMENTED YET" << endl;
    //parts.append(part);
    //win->guiFactory()->addClient(part);
}

void KDevCoreImpl::removePart(KDevPart* part)
{
    kdDebug() << "KDevCoreImpl::removePart() -- NOT IMLEMENTED YET" << endl;
    //win->guiFactory()->removeClient(part);
    //parts.remove(part);
    //delete part;
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
