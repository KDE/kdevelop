#include <qfile.h>


#include <kapp.h>
#include <klibloader.h>
#include <kservice.h>
#include <ktrader.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <kparts/componentfactory.h>

#include "kdevapi.h"
#include "kdevfactory.h"
#include "kdevplugin.h"
#include "kdevmakefrontend.h"
#include "kdevappfrontend.h"


#include "core.h"
#include "api.h"
#include "toplevel.h"


#include "plugincontroller.h"


PluginController *PluginController::s_instance = 0;


PluginController *PluginController::getInstance()
{
  if (!s_instance)
    s_instance = new PluginController();
  return s_instance;
}


PluginController::PluginController()
{
  s_instance = this;

  loadDefaultParts();
  loadGlobalPlugins();
}


PluginController::~PluginController()
{
}


void PluginController::loadDefaultParts()
{
  KService *service;
  KDevPart *part;

  // Make frontend
  KTrader::OfferList makeFrontendOffers = KTrader::self()->query(QString::fromLatin1("KDevelop/MakeFrontend"), QString::null);
  if (makeFrontendOffers.isEmpty())
    return;
  service = *makeFrontendOffers.begin();
  part = loadPlugin(service, "KDevMakeFrontend", Core::getInstance());
  if (part)
  {
    API::getInstance()->setMakeFrontend(static_cast<KDevMakeFrontend*>(part));
    integratePart(part);
  }

  // App frontend
  KTrader::OfferList appFrontendOffers = KTrader::self()->query(QString::fromLatin1("KDevelop/AppFrontend"), QString::null);
  if (appFrontendOffers.isEmpty())
    return;
  service = *appFrontendOffers.begin();
  part = loadPlugin(service, "KDevAppFrontend", Core::getInstance());
  if (part)
  {
    API::getInstance()->setAppFrontend(static_cast<KDevAppFrontend*>(part));
    integratePart(part);
  }
}


void PluginController::loadGlobalPlugins()
{

  KTrader::OfferList globalOffers = pluginServices( "Global" );
  KConfig *config = KGlobal::config();
  for (KTrader::OfferList::ConstIterator it = globalOffers.begin(); it != globalOffers.end(); ++it)
  {
    config->setGroup("Plugins");
    if (!config->readBoolEntry((*it)->name(), true))
       continue;

    if ( ( *it )->hasServiceType( "KDevelop/Part" ) ) {
      KDevPart *part = loadPlugin(*it, "KDevPart", Core::getInstance());
      if (!part)
        return;

      integratePart(part);
    } else {
        QStringList args = argumentsFromService( *it );

        KDevPlugin *plugin = KParts::ComponentFactory
	    ::createInstanceFromService<KDevPlugin>( *it, API::getInstance(), 0,
                                                     args );
        if ( plugin )
            integratePart( plugin );
    }
  }
}


KDevPart *PluginController::loadPlugin(const KService::Ptr &service, const char *className, QObject *parent)
{
  QStringList args = argumentsFromService( service );;

  kdDebug(9000) << "Loading service " << service->name() << endl;
  KLibFactory *factory = KLibLoader::self()->factory(QFile::encodeName(service->library()));

  if (!factory || !factory->inherits("KDevFactory"))
  {
    if (!factory)
    {
      QString errorMessage = KLibLoader::self()->lastErrorMessage();
      KMessageBox::error(0, i18n("There was an error loading the module %1.\n"
                                 "The diagnostics is:\n%2").arg(service->name()).arg(errorMessage));
      return 0;
    }

    kdDebug(9000) << "Does not have a KDevFactory" << endl;
    return 0;
  }

  KDevPart *part = static_cast<KDevFactory*>(factory)->createPart(API::getInstance(), parent, args);

  if (!part->inherits(className))
  {
    kdDebug(9000) << "Part does not inherit " << className << endl;
    return 0;
  }

  return part;
}

KService::List PluginController::pluginServices( const QString &scope )
{
    QString constraint;

    if ( !scope.isEmpty() )
	constraint = QString::fromLatin1( "[X-KDevelop-Scope] == '%1'" ).arg( scope );
    return KTrader::self()->query( QString::fromLatin1( "KDevelop/Plugin" ), 
	                           constraint );
}

void PluginController::integratePart(KXMLGUIClient *part)
{
  TopLevel::getInstance()->main()->guiFactory()->addClient(part);
}

QStringList PluginController::argumentsFromService( const KService::Ptr &service )
{
    QStringList args;
    QVariant prop = service->property( "X-KDevelop-Args" );
    if ( prop.isValid() )
        args = QStringList::split( " ", prop.toString() );
    return args;
}
