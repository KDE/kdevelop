#include <qfile.h>
#include <qvbox.h>

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
#include <kdebug.h>
#include <kdialogbase.h>

#include "kdevapi.h"
#include "kdevplugin.h"
#include "kdevmakefrontend.h"
#include "kdevappfrontend.h"
#include "kdevdifffrontend.h"

#include "core.h"
#include "api.h"
#include "toplevel.h"
#include "partselectwidget.h"

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
  m_globalParts.setAutoDelete( true );
  connect( Core::getInstance(), SIGNAL(configWidget(KDialogBase*)),
           this, SLOT(slotConfigWidget(KDialogBase*)) );
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

  // Diff frontend
  emit loadingPlugin(i18n("Loading plugin: Diff frontend"));
  KDevDiffFrontend *diffFrontend = loadDefaultPart< KDevDiffFrontend >( "KDevelop/DiffFrontend" );
  if ( diffFrontend ) {
    API::getInstance()->setDiffFrontend( diffFrontend );
    integratePart( diffFrontend );
  } else {
    kdDebug( 9000 ) << "could not load Diff frontend" << endl;
  }
}


void PluginController::loadGlobalPlugins()
{
  KTrader::OfferList globalOffers = pluginServices( "Global" );
  KConfig *config = KGlobal::config();
  for (KTrader::OfferList::ConstIterator it = globalOffers.begin(); it != globalOffers.end(); ++it)
  {
    config->setGroup("Plugins");
    
    QString name = (*it)->name();
    
    // Unload it if it is marked as ignored and loaded
    if (!config->readBoolEntry( name, true)) {
      KXMLGUIClient* part = m_globalParts[name];
      if( part ) {
        removePart( part );
        m_globalParts.remove( name );
      }
      continue;
    }

    // Check if it is already loaded
    if( m_globalParts[ name ] != 0 )
      continue;
      
    assert( !( *it )->hasServiceType( "KDevelop/Part" ) );

    emit loadingPlugin(i18n("Loading plugin: %1").arg((*it)->comment()));

    KDevPlugin *plugin = loadPlugin( *it );
    if ( plugin ) {
       m_globalParts.insert( name, plugin );
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

void PluginController::slotConfigWidget( KDialogBase* dlg )
{
  QVBox *vbox = dlg->addVBoxPage(i18n("Plugins"));
  PartSelectWidget *w = new PartSelectWidget(vbox, "part selection widget");
  connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
  connect( w, SIGNAL(accepted()), this, SLOT(loadGlobalPlugins()) );
}

void PluginController::integratePart(KXMLGUIClient *part)
{
  TopLevel::getInstance()->main()->guiFactory()->addClient(part);
}

void PluginController::removePart(KXMLGUIClient *part)
{
  TopLevel::getInstance()->main()->guiFactory()->removeClient(part);
}
