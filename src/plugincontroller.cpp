#include <qfile.h>


#include <kapp.h>
#include <klibloader.h>
#include <kservice.h>
#include <ktrader.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <kparts/componentfactory.h>
#include <assert.h>

#include "kdevapi.h"
#include "kdevplugin.h"
#include "kdevmakefrontend.h"
#include "kdevappfrontend.h"


#include "core.h"
#include "api.h"
#include "toplevel.h"


#include "plugincontroller.h"
#include "plugincontroller.moc"

// a separate method in this anonymous namespace to avoid having it all
// inline in plugincontroller.h
namespace
{
  template <class ComponentType>
  ComponentType *loadDefaultPart( const QString &serviceType )
  {
    KTrader::OfferList offers = KTrader::self()->query( serviceType, QString::null );
    KTrader::OfferList::ConstIterator serviceIt = offers.begin();
    for ( ; serviceIt != offers.end(); ++serviceIt ) {
      KService::Ptr service = *serviceIt;

      ComponentType *part = KParts::ComponentFactory
        ::createInstanceFromService< ComponentType >( service, API::getInstance(), 0,
                                                      PluginController::argumentsFromService( service ) );
      
      if ( part )
        return part;
    }
    return 0;
  }
};

PluginController *PluginController::s_instance = 0;


PluginController *PluginController::getInstance()
{
  if (!s_instance)
    s_instance = new PluginController();
  return s_instance;
}


PluginController::PluginController()
  : QObject()
{
  s_instance = this;
}


void PluginController::loadInitialPlugins()
{
  loadDefaultParts();
  loadGlobalPlugins();
}


PluginController::~PluginController()
{
}


void PluginController::loadDefaultParts()
{
  // Make frontend
  emit loadingPlugin(i18n("Loading plugin: Make frontend"));
  KDevMakeFrontend *makeFrontend = loadDefaultPart< KDevMakeFrontend >( "KDevelop/MakeFrontend" );
  if ( makeFrontend ) {
    API::getInstance()->setMakeFrontend( makeFrontend );
    integratePart( makeFrontend );
  }

  // App frontend
  emit loadingPlugin(i18n("Loading plugin: Application frontend"));
  KDevAppFrontend *appFrontend = loadDefaultPart< KDevAppFrontend >( "KDevelop/AppFrontend" );
  if ( appFrontend ) {
    API::getInstance()->setAppFrontend( appFrontend );
    integratePart( appFrontend );
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
      assert( false );
    } else {

	emit loadingPlugin(i18n("Loading plugin: %1").arg((*it)->comment()));

        QStringList args = argumentsFromService( *it );

        KDevPlugin *plugin = KParts::ComponentFactory
	        ::createInstanceFromService<KDevPlugin>( *it, API::getInstance(), 0,
                                                     args );
        if ( plugin )
            integratePart( plugin );
    }
  }
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

KDevPlugin *PluginController::loadPlugin( const KService::Ptr &service )
{
    return KParts::ComponentFactory
        ::createInstanceFromService<KDevPlugin>( service, API::getInstance(), 0,
                                                 argumentsFromService( service ) );
}

QStringList PluginController::argumentsFromService( const KService::Ptr &service )
{
    QStringList args;
    if ( !service ) 
        // service is a reference to a pointer, so a check whether it is 0 is still required
        return args;
    QVariant prop = service->property( "X-KDevelop-Args" );
    if ( prop.isValid() )
        args = QStringList::split( " ", prop.toString() );
    return args;
}
