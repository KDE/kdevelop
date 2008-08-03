

#ifndef TESTSTUBS_PLUGINCONTROLLER_STUB_H
#define TESTSTUBS_PLUGINCONTROLLER_STUB_H



#include "interfaces/iplugincontroller.h"
#include "interfaces/contextmenuextension.h"
#include "shell/profileengine.h" // yaikes, needed because no interface + plugincontroller returns references.

namespace TestStubs
{

class PluginController : public KDevelop::IPluginController
{
public:
//     enum PluginType {
//         Global = 0,
//         Project
//     };

    PluginController( QObject* parent = 0 ) : KDevelop::IPluginController(parent), m_pluginForExtension(0) {} ;
    virtual ~PluginController() {}

    KDevelop::IPlugin* m_pluginForExtension;

    virtual KPluginInfo pluginInfo( KDevelop::IPlugin* ) const { Q_ASSERT(0); return KPluginInfo(); }
    virtual QList<KDevelop::IPlugin*> loadedPlugins() const { Q_ASSERT(0); return QList<KDevelop::IPlugin*>(); }
    virtual void unloadPlugin( const QString & plugin ) {Q_ASSERT(0); }
    virtual KDevelop::IPlugin* loadPlugin( const QString & pluginName ) { Q_ASSERT(0); return 0; }

    virtual KDevelop::IPlugin *pluginForExtension(const QString &extension, const QString& pluginname = "" ) { return m_pluginForExtension; }
    virtual QList<KDevelop::IPlugin*> allPluginsForExtension(const QString &extension, const QStringList &constraints) { Q_ASSERT(0); return QList<KDevelop::IPlugin*>(); }

//     static KPluginInfo::List query( const QString &serviceType, const QString &constraint );
//     static KPluginInfo::List queryPlugins( const QString &constraint );
//     static QStringList argumentsFromService( const KService::Ptr &service );

    virtual QString currentProfile() const { Q_ASSERT(0); return ""; }
    virtual void loadPlugins( PluginType offer ) { Q_ASSERT(0); return; }
    virtual void unloadPlugins( PluginType offer ) { Q_ASSERT(0); return; }
    KDevelop::ProfileEngine* m_engine;
    virtual KDevelop::ProfileEngine &engine() const { Q_ASSERT(0); return *m_engine; }
    virtual QString changeProfile( const QString &newProfile ) { Q_ASSERT(0); return ""; }
    virtual QExtensionManager* extensionManager() { Q_ASSERT(0); return 0; }
    virtual QList<KDevelop::ContextMenuExtension> queryPluginsForContextMenuExtensions( KDevelop::Context* context ) const { Q_ASSERT(0); return QList<KDevelop::ContextMenuExtension>(); } 

// Q_SIGNALS:
//     void loadingPlugin( const QString& );
//     void pluginLoaded( KDevelop::KDevelop::IPlugin* );
//     void pluginUnloaded( KDevelop::KDevelop::IPlugin* );
//     void profileChanged();
};

}

#endif
