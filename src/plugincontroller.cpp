#include <qfile.h>

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
#include <kdevcreatefile.h>
#include <kdevplugininfo.h>
#include <kaction.h>
#include <kxmlguifactory.h>

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

/*  m_defaultProfile = QLatin1String( "FullIDE" );
  m_defaultProfilePath = kapp->dirs()->localkdedir() + "/" +
			 KStandardDirs::kde_default( "data" ) +
			 QLatin1String("/kdevelop/profiles/FullIDE");*/

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

void PluginController::loadCorePlugins()
{
  KTrader::OfferList coreOffers = m_engine.offers(m_profile, ProfileEngine::Core);
  loadPlugins( coreOffers );
}

void PluginController::loadGlobalPlugins( const QStringList & ignorePlugins )
{
  KTrader::OfferList globalOffers = m_engine.offers(m_profile, ProfileEngine::Global);
  loadPlugins( globalOffers, ignorePlugins );
}

void PluginController::loadProjectPlugins( const QStringList & ignorePlugins )
{
	KTrader::OfferList projectOffers = m_engine.offers(m_profile, ProfileEngine::Project);
	loadPlugins( projectOffers, ignorePlugins );
}

void PluginController::loadPlugins( KTrader::OfferList offers, const QStringList & ignorePlugins )
{
  for (KTrader::OfferList::ConstIterator it = offers.begin(); it != offers.end(); ++it)
  {
    QString name = (*it)->desktopEntryName();

    // Check if it is already loaded or shouldn't be
    if( m_parts.value(name) != 0 || ignorePlugins.contains(name) )
      continue;

    emit loadingPlugin(i18n("Loading: %1").arg((*it)->genericName()));

    KDevPlugin *plugin = loadPlugin( *it );
    if ( plugin )
    {
        m_parts[name] = plugin;
        integratePart( plugin );
    }
  }
}

void PluginController::unloadPlugins()
{
  QHash<QString, KDevPlugin *>::iterator it = m_parts.begin();
  while (it != m_parts.end()) {
    KDevPlugin* part = it.value();
    removePart( part );
    m_parts.remove( it.key() );
    delete part;
    it = m_parts.erase(it);
  }
}

void PluginController::unloadProjectPlugins( )
{
	KTrader::OfferList offers = m_engine.offers(m_profile, ProfileEngine::Project);
	for (KTrader::OfferList::ConstIterator it = offers.begin(); it != offers.end(); ++it)
	{
		QString name = (*it)->desktopEntryName();

		if ( KDevPlugin * plugin = m_parts.value(name) )
		{
			removeAndForgetPart( name, plugin );
			delete plugin;
		}
	}
}

void PluginController::unloadPlugins( QStringList const & unloadParts )
{
	QStringList::ConstIterator it = unloadParts.begin();
	while ( it != unloadParts.end() )
	{
		KDevPlugin* part = m_parts.value(*it);
		if( part )
		{
			removePart( part );
			m_parts.remove( *it );
			delete part;
		}
		++it;
	}
}

KDevPlugin *PluginController::loadPlugin( const KService::Ptr &service )
{
    int err = 0;
    KDevPlugin * pl = KParts::ComponentFactory
        ::createInstanceFromService<KDevPlugin>( service, API::getInstance(), service->name().toAscii().constData(),
                                                 argumentsFromService( service ), &err );
//    kDebug() << "ERR: " << err << endl;
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
    //FIXME: adymo: i disabled this because plugin configuration should be project-wide
    // in profile-enabled shell
/*  QVBox *vbox = dlg->addVBoxPage( i18n("Plugins"), i18n("Plugins"), BarIcon( "kdf", KIcon::SizeMedium ) );
  PartSelectWidget *w = new PartSelectWidget(vbox, "part selection widget");
  connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
  connect( w, SIGNAL(accepted()), this, SLOT(loadGlobalPlugins()) );*/
}

void PluginController::integratePart(KXMLGUIClient *part)
{
    if ( ! part ) return;

    TopLevel::getInstance()->main()->guiFactory()->addClient(part);
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
    kDebug() << "removing: " << name << endl;
    m_parts.remove(name);
    removePart(part);
}

const QList<KDevPlugin *> PluginController::loadedPlugins()
{
    return m_parts.values();
}

KDevPlugin * PluginController::extension( const QString & serviceType, const QString & constraint )
{
    KTrader::OfferList offers = KDevPluginController::query(serviceType, constraint);
    for (KTrader::OfferList::const_iterator it = offers.constBegin(); it != offers.constEnd(); ++it)
    {
        KDevPlugin *ext = m_parts.value((*it)->desktopEntryName());
        if (ext) return ext;
    }
    return 0;
}

KDevPlugin * PluginController::loadPlugin( const QString & serviceType, const QString & constraint )
{
	KTrader::OfferList offers = KDevPluginController::query( serviceType, constraint );
	if ( !offers.size() == 1 ) return 0;

	KTrader::OfferList::const_iterator it = offers.constBegin();
	QString name = (*it)->desktopEntryName();

	KDevPlugin * plugin = 0;
	if ( plugin = m_parts.value(name) )
	{
		return plugin;
	}

	if ( plugin = loadPlugin( *it ) )
	{
		m_parts.insert( name, plugin );
		integratePart( plugin );
	}

	return plugin;
}

void PluginController::unloadPlugin( const QString & plugin )
{
	QStringList pluginList;
	pluginList << plugin;
	unloadPlugins( pluginList );
}

KUrl::List PluginController::profileResources(const QString &nameFilter)
{
    return m_engine.resources(currentProfile(), nameFilter);
}

KUrl::List PluginController::profileResourcesRecursive(const QString &nameFilter)
{
    return m_engine.resourcesRecursive(currentProfile(), nameFilter);
}

QString PluginController::changeProfile(const QString &newProfile)
{
    kDebug() << "CHANGING PROFILE: from " << currentProfile() << " to " << newProfile << endl;
    QStringList unload;
    KTrader::OfferList coreLoad;
    KTrader::OfferList globalLoad;
    m_engine.diffProfiles(ProfileEngine::Core, currentProfile(), newProfile, unload, coreLoad);
    m_engine.diffProfiles(ProfileEngine::Global, currentProfile(), newProfile, unload, globalLoad);

    QString oldProfile = m_profile;
    m_profile = newProfile;

    unloadPlugins(unload);
    loadPlugins( coreLoad );
    loadPlugins( globalLoad );

    return oldProfile;
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

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
