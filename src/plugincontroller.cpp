#include <qfile.h>
#include <qvbox.h>

#include <kcmdlineargs.h>
#include <kapplication.h>
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
#include <kcmdlineargs.h>
#include <kstandarddirs.h>
#include <kstatusbar.h>
#include <kiconloader.h>

#include <kdevapi.h>
#include <kdevplugin.h>
#include <kdevmakefrontend.h>
#include <kdevappfrontend.h>
#include <kdevdifffrontend.h>
#include <kdevsourceformatter.h>
#include <kdevcreatefile.h>
#include <kdevplugininfo.h>
#include <kaction.h>

#include <profileengine.h>

#include "core.h"
#include "api.h"
#include "toplevel.h"
#include "projectmanager.h"
#include "partselectwidget.h"

#include "plugincontroller.h"

#include "shellextension.h"

// a separate method in this anonymous namespace to avoid having it all
// inline in plugincontroller.h
namespace
{
  template <class ComponentType>
  ComponentType *loadDefaultPart( const QString &serviceType )
  {
    KTrader::OfferList offers = KTrader::self()->query(serviceType, QString("[X-KDevelop-Version] == %1").arg(KDEVELOP_PLUGIN_VERSION));
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
}

PluginController *PluginController::s_instance = 0;


PluginController *PluginController::getInstance()
{
  if (!s_instance)
    s_instance = new PluginController();
  return s_instance;
}


PluginController::PluginController()
  : KDevPluginController()
{
  connect( Core::getInstance(), SIGNAL(configWidget(KDialogBase*)),
           this, SLOT(slotConfigWidget(KDialogBase*)) );
  
/*  m_defaultProfile = QString::fromLatin1( "FullIDE" );
  m_defaultProfilePath = kapp->dirs()->localkdedir() + "/" + 
			 KStandardDirs::kde_default( "data" ) + 
			 QString::fromLatin1("/kdevelop/profiles/FullIDE");*/
    
    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
    if( args->isSet("profile") ){
        m_profile = QString::fromLocal8Bit( args->getOption("profile") );
    } else {
        m_profile = ShellExtension::getInstance()->defaultProfile();
    }

}


void PluginController::loadInitialPlugins()
{    
    loadCorePlugins();    
    loadGlobalPlugins();
}


PluginController::~PluginController()
{
  unloadPlugins();
}



// a Core plugin is implicitly global, so it makes
// sense to put them in the global plugin container
void PluginController::loadCorePlugins()
{
  KTrader::OfferList coreOffers = m_engine.offers(m_profile, ProfileEngine::Core);
  for (KTrader::OfferList::ConstIterator it = coreOffers.begin(); it != coreOffers.end(); ++it)
  {
    QString name = (*it)->name();

    // Check if it is already loaded
    if( m_parts[ name ] != 0 )
      continue;

    assert( !( *it )->hasServiceType( "KDevelop/Part" ) );

    emit loadingPlugin(i18n("Loading: %1").arg((*it)->genericName()));

    KDevPlugin *plugin = loadPlugin( *it );
    if ( plugin )
    {
        m_parts.insert( name, plugin );
        integratePart( plugin );
    }
  }
}

void PluginController::loadGlobalPlugins()
{
  KTrader::OfferList globalOffers = m_engine.offers(m_profile, ProfileEngine::Global);
//  KConfig config( m_profilePath );
  for (KTrader::OfferList::ConstIterator it = globalOffers.begin(); it != globalOffers.end(); ++it)
  {
//    config.setGroup( "Plugins" );

    QString name = (*it)->name();
	
/*    // Unload it if is marked as ignored and loaded
    if (!config.readBoolEntry( name, true)) {
      KDevPlugin* part = m_parts[name];
      if( part ) {
        removePart( part );
        m_parts.remove( name );
        part->deleteLater();
      }
      continue;
    }*/
    
    // Check if it is already loaded
    if( m_parts[ name ] != 0 )
      continue;

    assert( !( *it )->hasServiceType( "KDevelop/Part" ) );

    emit loadingPlugin(i18n("Loading: %1").arg((*it)->genericName()));

    KDevPlugin *plugin = loadPlugin( *it );
    if ( plugin ) {
        m_parts.insert( name, plugin );
        integratePart( plugin );
    }
  }
}

//void PluginController::unloadGlobalPlugins()
void PluginController::unloadPlugins()
{
//  for( QDictIterator<KDevPlugin> it( m_globalParts ); !it.isEmpty(); )
  for( QDictIterator<KDevPlugin> it( m_parts ); !it.isEmpty(); )
  {
    KDevPlugin* part = it.current();
    removePart( part );
    m_parts.remove( it.currentKey() );
    delete part;
  }
}

void PluginController::loadLocalParts( ProjectInfo * projectInfo, QStringList const & loadPlugins, QStringList const & ignorePlugins  )
{
	KTrader::OfferList localOffers = m_engine.offers(m_profile, ProfileEngine::Project);
	for (KTrader::OfferList::ConstIterator it = localOffers.begin(); it != localOffers.end(); ++it)
	{
		QString name = (*it)->name();
		TopLevel::getInstance()->statusBar()->message( i18n("Loading: %1").arg( (*it)->genericName() ) );
		
		kdDebug(9000) << "-----------------------------> load part " << name << endl;
		
		// Check if it is already loaded or should be ignored
		if( m_parts[ name ] != 0 || ignorePlugins.contains( name ) )
			continue;
	
		if( loadPlugins.contains( name ) || checkNewService( projectInfo, *it ) )
		{
			KDevPlugin *part = loadPlugin( *it );
			if ( !part ) continue;
		
			integratePart( part );
			m_parts.insert( name, part );
		}
	}
}

void PluginController::unloadPlugins( QStringList const & unloadParts )
{
	QStringList::ConstIterator it = unloadParts.begin();
	while ( it != unloadParts.end() )
	{
		KDevPlugin* part = m_parts[ *it ];
		if( part ) 
		{
			removePart( part );
			m_parts.remove( *it );
			delete part;
		}
		++it;
	}
}

bool PluginController::checkNewService( ProjectInfo * projectInfo, const KService::Ptr &service )
{
  QVariant var = service->property("X-KDevelop-ProgrammingLanguages");
  QStringList langlist = var.asStringList();

  // empty means it supports all languages
  if( !langlist.isEmpty() && !langlist.contains(projectInfo->m_activeLanguage) ) {
    projectInfo->m_ignoreParts << service->name();
    return false;
  }

  // the language is ok, now check if the keywords match
  QStringList serviceKeywords = service->keywords();
  for ( QStringList::Iterator is = serviceKeywords.begin();
        is != serviceKeywords.end(); ++is )
  {
    if ( !projectInfo->m_keywords.contains(*is) ) {
      // no match
      kdDebug(9000) << "ignoreParts because Keyword does not match: " << service->name() << endl;
      projectInfo->m_ignoreParts << service->name();
      return false;
    }
  }

  projectInfo->m_loadParts << service->name();
  return true;
}

/*KService::List PluginController::pluginServices( const QString &scope )
{
    QString constraint = QString::fromLatin1("[X-KDevelop-Version] == %1").arg(KDEVELOP_PLUGIN_VERSION);

    if ( !scope.isEmpty() )
	constraint += QString::fromLatin1( " and [X-KDevelop-Scope] == '%1'").arg( scope );
    if (TopLevel::mode == TopLevel::AssistantMode)
        constraint += QString::fromLatin1( " and [X-KDevelop-Mode] == 'AssistantMode'");
    return KTrader::self()->query( QString::fromLatin1( "KDevelop/Plugin" ),
	                           constraint );
}*/

KDevPlugin *PluginController::loadPlugin( const KService::Ptr &service )
{
    int err = 0;
    KDevPlugin * pl = KParts::ComponentFactory
        ::createInstanceFromService<KDevPlugin>( service, API::getInstance(), 0,
                                                 argumentsFromService( service ), &err );
//    kdDebug() << "ERR: " << err << endl;
    return pl;
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
  QVBox *vbox = dlg->addVBoxPage( i18n("Plugins"), i18n("Plugins"), BarIcon( "kdf", KIcon::SizeMedium ) );
  PartSelectWidget *w = new PartSelectWidget(vbox, "part selection widget");
  connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
  connect( w, SIGNAL(accepted()), this, SLOT(loadGlobalPlugins()) );
}

void PluginController::integratePart(KXMLGUIClient *part)
{
    if ( ! part ) return;

    TopLevel::getInstance()->main()->guiFactory()->addClient(part);

    connect( part->actionCollection(), SIGNAL( actionStatusText( const QString & ) ),
        TopLevel::getInstance()->main()->actionCollection(), SIGNAL( actionStatusText( const QString & ) ) );
}

void PluginController::integrateAndRememberPart(const QString &name, KDevPlugin *part)
{
    m_parts.insert(name, part);
    integratePart(part);
}

void PluginController::removePart(KXMLGUIClient *part)
{
  TopLevel::getInstance()->main()->guiFactory()->removeClient(part);
}

void PluginController::removeAndForgetPart(const QString &name, KDevPlugin *part)
{
    kdDebug() << "removing: " << name << endl;
    m_parts.remove(name);
    removePart(part);
}

const QValueList<KDevPlugin*> PluginController::loadedPlugins()
{
	QValueList<KDevPlugin*> plugins;
	QDictIterator<KDevPlugin> itt(m_parts);
	while( itt.current() )
	{
		plugins.append( itt.current() );
		++itt;
	}
	return plugins;
}

KDevPlugin * PluginController::extension( const QString & serviceType, const QString & constraint )
{
    KTrader::OfferList offers = KDevPluginController::query(serviceType, constraint);
    for (KTrader::OfferList::const_iterator it = offers.constBegin(); it != offers.end(); ++it)
    {
        KDevPlugin *ext = m_parts[(*it)->name()];
        if (ext) return ext;
    }
    return 0;
}

KURL::List PluginController::profileResources(const QString &nameFilter)
{
    return m_engine.resources(currentProfile(), nameFilter);
}

KURL::List PluginController::profileResourcesRecursive(const QString &nameFilter)
{
    return m_engine.resourcesRecursive(currentProfile(), nameFilter);
}


/*
KDevPlugin * PluginController::getPlugin( const KService::Ptr & service )
{
	KDevPlugin * plugin = m_parts[ (*it)->name() ];
	if ( !plugin )
	{
		KDevPlugin * plugin = loadPlugin( *it );
		if ( plugin )
		{
			integratePart( plugin );
			m_parts.insert( (*it)->name(), plugin );
		}
	}
	return plugin;
}
*/
#include "plugincontroller.moc"

