/* This file is part of the KDE project
Copyright 2004, 2007 Alexander Dymo <adymo@kdevelop.org>
Copyright 2006 Matt Rogers <mattr@kde.org
Copyright 2007 Andreas Pakulat <apaku@gmx.de>

Based on code from Kopete
Copyright (c) 2002-2003 Martijn Klingens <klingens@kde.org>

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
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/
#include "plugincontroller.h"

#include <QtCore/QFile>
#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QAction>

#include <kcmdlineargs.h>
#include <klibloader.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <klocale.h>
#include <kxmlguiwindow.h>
#include <assert.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kstandarddirs.h>
#include <kaction.h>
#include <kxmlguifactory.h>
#include <kmenu.h>

#include <interfaces/contextmenuextension.h>
#include <interfaces/iplugin.h>
#include <interfaces/isession.h>
#include <interfaces/idebugcontroller.h>
#include <interfaces/idocumentationcontroller.h>
#include <interfaces/ipluginversion.h>

//#include <kross/krossplugin.h>

#include "mainwindow.h"
#include "core.h"
#include "shellextension.h"
#include "runcontroller.h"
#include "debugcontroller.h"
#include "documentationcontroller.h"
#include "sourceformattercontroller.h"
#include "projectcontroller.h"

namespace KDevelop
{

static const QString pluginControllerGrp("Plugins");

bool isUserSelectable( const KPluginInfo& info )
{
    QString loadMode = info.property( "X-KDevelop-LoadMode").toString();
    return loadMode.isEmpty() || loadMode == "UserSelectable";
}

bool isGlobalPlugin( const KPluginInfo& info )
{
    return info.property( "X-KDevelop-Category" ).toString() == "Global";
}

bool hasMandatoryProperties( const KPluginInfo& info )
{
    QVariant mode = info.property( "X-KDevelop-Mode" );
    QVariant version = info.property( "X-KDevelop-Version" );

    return mode.isValid() && mode.canConvert( QVariant::String )
           && version.isValid() && version.canConvert( QVariant::String );
}


class PluginControllerPrivate
{
public:
    QList<KPluginInfo> plugins;

    //map plugin infos to currently loaded plugins
    typedef QMap<KPluginInfo, IPlugin*> InfoToPluginMap;
    InfoToPluginMap loadedPlugins;

    // The plugin manager's mode. The mode is StartingUp until loadAllPlugins()
    // has finished loading the plugins, after which it is set to Running.
    // ShuttingDown and DoneShutdown are used during shutdown by the
    // async unloading of plugins.
    enum CleanupMode
    {
        Running     /**< the plugin manager is running */,
        CleaningUp  /**< the plugin manager is cleaning up for shutdown */,
        CleanupDone /**< the plugin manager has finished cleaning up */
    };
    CleanupMode cleanupMode;

    bool canUnload( const KPluginInfo& plugin )
    {
        kDebug() << "checking can unload for:" << plugin.name() << plugin.property("X-KDevelop-LoadMode");
        if( plugin.property( "X-KDevelop-LoadMode" ).toString() == "AlwaysOn" )
        {
            return false;
        }
        QStringList interfaces = plugin.property( "X-KDevelop-Interfaces" ).toStringList();
        kDebug() << "checking dependencies:" << interfaces;
        foreach( const KPluginInfo& info, loadedPlugins.keys() )
        {
            if( info.pluginName() != plugin.pluginName() )
            {
                QStringList dependencies = info.property( "X-KDevelop-IRequired" ).toStringList();
                dependencies += info.property( "X-KDevelop-IOptional" ).toStringList();
                foreach( const QString& dep, dependencies )
                {
                    if( interfaces.contains( dep ) && !canUnload( info ) )
                    {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    KPluginInfo infoForId( const QString& id ) const
    {
        foreach( const KPluginInfo& info, plugins )
        {
            if( info.pluginName() == id )
            {
                return info;
            }
        }
        return KPluginInfo();
    }

    Core *core;
};

PluginController::PluginController(Core *core)
    : IPluginController(), d(new PluginControllerPrivate)
{
    setObjectName("PluginController");
    d->core = core;
    //kDebug() << "Fetching plugin info which matches:" << QString( "[X-KDevelop-Version] == %1" ).arg(KDEVELOP_PLUGIN_VERSION);
    d->plugins = KPluginInfo::fromServices( KServiceTypeTrader::self()->query( QLatin1String( "KDevelop/Plugin" ),
        QString( "[X-KDevelop-Version] == %1" ).arg(KDEVELOP_PLUGIN_VERSION) ) );
    d->cleanupMode = PluginControllerPrivate::Running;
    // Register the KDevelop::IPlugin* metatype so we can properly unload it
    qRegisterMetaType<KDevelop::IPlugin*>( "KDevelop::IPlugin*" );
}

PluginController::~PluginController()
{
    if ( d->cleanupMode != PluginControllerPrivate::CleanupDone ) {
        kWarning(9501) << "Destructing plugin controller without going through the shutdown process! Backtrace is: "
                       << endl << kBacktrace() << endl;
    }

    delete d;
}

KPluginInfo PluginController::pluginInfo( const IPlugin* plugin ) const
{
    return d->loadedPlugins.key(const_cast<IPlugin*>(plugin));
}

void PluginController::cleanup()
{
    if(d->cleanupMode != PluginControllerPrivate::Running)
    {
        //kDebug() << "called when not running. state =" << d->cleanupMode;
        return;
    }

    d->cleanupMode = PluginControllerPrivate::CleaningUp;

    // Ask all plugins to unload
    while ( !d->loadedPlugins.isEmpty() )
    {
        //Let the plugin do some stuff before unloading
        unloadPlugin(d->loadedPlugins.begin().value(), Now);
    }

    d->cleanupMode = PluginControllerPrivate::CleanupDone;
}

IPlugin* PluginController::loadPlugin( const QString& pluginName )
{
    return loadPluginInternal( pluginName );
}

bool PluginController::isEnabled( const KPluginInfo& info )
{
    KConfigGroup grp = Core::self()->activeSession()->config()->group( pluginControllerGrp );
    bool isEnabled = grp.readEntry( info.pluginName()+"Enabled", ShellExtension::getInstance()->defaultPlugins().isEmpty() || ShellExtension::getInstance()->defaultPlugins().contains( info.pluginName() ) );
    //kDebug() << "read config:" << isEnabled << "is global plugin:" << isGlobalPlugin( info ) << "default:" << ShellExtension::getInstance()->defaultPlugins().isEmpty()  << ShellExtension::getInstance()->defaultPlugins().contains( info.pluginName() );
    return !isGlobalPlugin( info ) || !isUserSelectable( info ) || isEnabled;
}

void PluginController::initialize()
{
    QMap<QString, bool> pluginMap;
    if( ShellExtension::getInstance()->defaultPlugins().isEmpty() )
    {
        foreach( const KPluginInfo& pi, d->plugins )
        {
            pluginMap.insert( pi.pluginName(), true );
        }
    } else
    {
        // Get the default from the ShellExtension
        foreach( const QString& s, ShellExtension::getInstance()->defaultPlugins() )
        {
            pluginMap.insert( s, true );
        }
    }

    KConfigGroup grp = Core::self()->activeSession()->config()->group( pluginControllerGrp );
    QMap<QString, QString> entries = grp.entryMap();

    QMap<QString, QString>::Iterator it;
    for ( it = entries.begin(); it != entries.end(); ++it )
    {
        QString key = it.key();
        if ( key.endsWith( QLatin1String( "Enabled" ) ) )
        {
            QString pluginid = key.left( key.length() - 7 );
            bool defValue;
            QMap<QString, bool>::const_iterator entry = pluginMap.constFind( pluginid );
            if( entry != pluginMap.constEnd() )
            {
                defValue = entry.value();
            } else {
                defValue = false;
            }
            pluginMap.insert( key.left(key.length() - 7), grp.readEntry(key,defValue) );
        }
    }

    foreach( const KPluginInfo& pi, d->plugins )
    {
        if( isGlobalPlugin( pi ) )
        {
            QMap<QString, bool>::const_iterator it = pluginMap.constFind( pi.pluginName() );
            if( it != pluginMap.constEnd() && ( it.value() || !isUserSelectable( pi ) ) )
            {
                // Plugin is mentioned in pluginmap and the value is true, so try to load it
                loadPluginInternal( pi.pluginName() );
                if( !grp.hasKey( pi.pluginName() + "Enabled" ) )
                {
                    if( isUserSelectable( pi ) )
                    {
                        // If plugin isn't listed yet, add it with true now
                        grp.writeEntry( pi.pluginName()+"Enabled", true );
                    }
                } else if( grp.hasKey( pi.pluginName() + "Disabled" ) && !isUserSelectable( pi ) )
                {
                    // Remove now-obsolete entries
                    grp.deleteEntry( pi.pluginName() + "Disabled" );
                }
            }
        }
    }
    // Synchronize so we're writing out to the file.
    grp.sync();
}

QList<IPlugin *> PluginController::loadedPlugins() const
{
    return d->loadedPlugins.values();
}

bool PluginController::unloadPlugin( const QString & pluginId )
{
    IPlugin *thePlugin = plugin( pluginId );
    bool canUnload = d->canUnload( d->infoForId( pluginId ) );
    kDebug() << "Unloading plugin:" << pluginId << "?" << thePlugin << canUnload;
    if( thePlugin && canUnload )
    {
        return unloadPlugin(thePlugin, Later);
    }
    return (canUnload && thePlugin);
}

bool PluginController::unloadPlugin(IPlugin* plugin, PluginDeletion deletion)
{
    kDebug() << "unloading plugin:" << plugin << pluginInfo( plugin ).name();

    emit unloadingPlugin(plugin);
    plugin->unload();
    emit pluginUnloaded(plugin);

    //Remove the plugin from our list of plugins so we create a new
    //instance when we're asked for it again.
    //This is important to do right here, not later when the plugin really
    //vanishes. For example project re-opening might try to reload the plugin
    //and then would get the "old" pointer which will be deleted in the next
    //event loop run and thus causing crashes.
    for ( PluginControllerPrivate::InfoToPluginMap::Iterator it = d->loadedPlugins.begin();
            it != d->loadedPlugins.end(); ++it )
    {
        if ( it.value() == plugin )
        {
            d->loadedPlugins.erase( it );
            break;
        }
    }

    if (deletion == Later)
        plugin->deleteLater();
    else
        delete plugin;
    return true;
}

KPluginInfo PluginController::infoForPluginId( const QString &pluginId ) const
{
    QList<KPluginInfo>::ConstIterator it;
    for ( it = d->plugins.constBegin(); it != d->plugins.constEnd(); ++it )
    {
        if ( it->pluginName() == pluginId )
            return *it;
    }

    return KPluginInfo();
}

IPlugin *PluginController::loadPluginInternal( const QString &pluginId )
{
    KPluginInfo info = infoForPluginId( pluginId );
    if ( !info.isValid() )
    {
        kWarning(9501) << "Unable to find a plugin named '" << pluginId << "'!" ;
        return 0L;
    }

    if ( d->loadedPlugins.contains( info ) )
        return d->loadedPlugins[ info ];

    if( !isEnabled( info ) )
    {
        // Do not load disabled plugins
        kWarning() << "Not loading plugin named" << pluginId << "because its been disabled!";
        return 0;
    }

    if( !hasMandatoryProperties( info ) ) {
        kWarning() << "Unable to load plugin named " << pluginId << "! Doesn't have all mandatory properties set";
        return 0;
    }

    if( info.property("X-KDevelop-Mode") == "GUI"
        && Core::self()->setupFlags() == Core::NoUi )
    {
        kDebug() << "Unable to load plugin named" << pluginId << ". Running in No-Ui mode, but the plugin says it needs a GUI";
        return 0;
    }

    bool isKrossPlugin = false;
    //TODO: Re-Enable after generating new kross bindings for project model
//     QString krossScriptFile;
//     if( info.property("X-KDevelop-PluginType").toString() == "Kross" )
//     {
//         isKrossPlugin = true;
//         krossScriptFile = KStandardDirs::locate( "appdata", info.service()->library(), KComponentData("kdevkrossplugins"));
//         if( krossScriptFile.isEmpty() || !QFileInfo( krossScriptFile ).exists() || !QFileInfo( krossScriptFile ).isReadable() )
//         {
//             kWarning() << "Unable to load kross plugin" << pluginId << ". Script file" << krossScriptFile << "not found or not readable";
//             return 0;
//         }
//     }

    kDebug() << "Attempting to load '" << pluginId << "'";
    emit loadingPlugin( info.pluginName() );
    QString str_error;
    IPlugin *plugin = 0;
    QStringList missingInterfaces;
    kDebug() << "Checking... " << info.name();
    if ( checkForDependencies( info, missingInterfaces ) )
    {
        QVariant prop = info.property( "X-KDevelop-PluginType" );
        kDebug() << "Checked... starting to load:" << info.name() << "type:" << prop;

        QString failedPlugin;
        if( !loadDependencies( info, failedPlugin ) )
        {
            kWarning() << "Could not load a required dependency:" << failedPlugin;
            return 0;
        }
        loadOptionalDependencies( info );

        if( isKrossPlugin )
        {
    //TODO: Re-Enable after generating new kross bindings for project model
            // Kross is special, we create always the same "plugin" which hooks up
            // the script and makes the connection between C++ and script side
//             kDebug() << "it is a kross plugin!!";
//             // Workaround for KAboutData constructor needing a KLocalizedString and
//             // KLocalized string storing the char* for later usage
//             QString tmp = info.name();
//             int len = tmp.toUtf8().size();
//             char* name = new char[len+1];
//             memcpy( name, tmp.toUtf8().data(), len );
//             name[len] = '\0';
//             tmp = info.comment();
//             len = tmp.toUtf8().size();
//             char* comment = new char[len+1];
//             memcpy( comment, tmp.toUtf8().data(), len );
//             comment[len] = '\0';
//             // Create the kross plugin instance from the desktop file data.
//             plugin = new KrossPlugin( krossScriptFile, KAboutData( info.pluginName().toUtf8(), info.pluginName().toUtf8(),
//                               ki18n( name ), info.version().toUtf8(), ki18n( comment ), KAboutLicense::byKeyword( info.license() ).key() ), d->core );
        }
        else
        {
            plugin = KServiceTypeTrader::createInstanceFromQuery<IPlugin>( QLatin1String( "KDevelop/Plugin" ),
                    QString::fromLatin1( "[X-KDE-PluginInfo-Name]=='%1'" ).arg( pluginId ), d->core, QVariantList(), &str_error );
        }
    }

    if ( plugin )
    {
        if ( plugin->hasError() ) {
            KMessageBox::error(0, i18n("Plugin '%1' could not be loaded correctly and was disabled.\nReason: %2.", info.name(), plugin->errorDescription()));
            info.setPluginEnabled(false);
            info.save(Core::self()->activeSession()->config()->group(pluginControllerGrp));
            unloadPlugin(pluginId);
            return 0;
        }
        d->loadedPlugins.insert( info, plugin );
        info.setPluginEnabled( true );

        kDebug() << "Successfully loaded plugin '" << pluginId << "'";
        emit pluginLoaded( plugin );
    }
    else
    {
        if( str_error.isEmpty() && !missingInterfaces.isEmpty() )
        {
            kWarning() << "Can't load plugin '" << pluginId
                    << "' some of its required dependencies could not be fulfilled:" << endl
                    << missingInterfaces.join(",") << endl;
        }
        else
        {
            kWarning() << "Loading plugin '" << pluginId
                << "' failed, KServiceTypeTrader reported error: '" << endl <<
                str_error << "'";
        }
    }

    return plugin;
}


IPlugin* PluginController::plugin( const QString& pluginId )
{
    KPluginInfo info = infoForPluginId( pluginId );
    if ( !info.isValid() )
        return 0L;

    if ( d->loadedPlugins.contains( info ) )
        return d->loadedPlugins[ info ];
    else
        return 0L;
}

///@todo plugin load operation should be O(n)
bool PluginController::checkForDependencies( const KPluginInfo& info, QStringList& missing ) const
{
    QVariant prop = info.property( "X-KDevelop-IRequired" );
    bool result = true;
    if( prop.canConvert<QStringList>() )
    {
        QStringList deps = prop.toStringList();
        foreach( const QString &iface, deps )
        {
            KPluginInfo::List l = queryPlugins( QString("'%1' in [X-KDevelop-Interfaces]").arg(iface) );
            if( l.isEmpty() )
            {
                result = false;
                missing << iface;
            }
        }
    }
    return result;
}

void PluginController::loadOptionalDependencies( const KPluginInfo& info )
{
    QVariant prop = info.property( "X-KDevelop-IOptional" );
    if( prop.canConvert<QStringList>() )
    {
        QStringList deps = prop.toStringList();
        foreach( const QString &iface, deps )
        {
            KPluginInfo info = queryPlugins( QString("'%1' in [X-KDevelop-Interfaces]").arg(iface) ).first();
            if( !loadPluginInternal( info.pluginName() ) )
	    {
		    kDebug() << "Couldn't load optional dependecy:" << iface << info.pluginName();
	    }
        }
    }
}

bool PluginController::loadDependencies( const KPluginInfo& info, QString& failedPlugin )
{
    QVariant prop = info.property( "X-KDevelop-IRequired" );
    QStringList loadedPlugins;
    if( prop.canConvert<QStringList>() )
    {
        QStringList deps = prop.toStringList();
        foreach( const QString &iface, deps )
        {
            KPluginInfo info = queryPlugins( QString("'%1' in [X-KDevelop-Interfaces]").arg(iface) ).first();
            if( !loadPluginInternal( info.pluginName() ) )
            {
                foreach( const QString& name, loadedPlugins )
                {
                    unloadPlugin( name );
                }
                failedPlugin = info.pluginName();
                return false;
            }
            loadedPlugins << info.pluginName();
        }
    }
    return true;
}

IPlugin* PluginController::pluginForExtension( const QString& extension, const QString& pluginname)
{
    //kDebug() << "Loading Plugin ("<< pluginname << ") for Extension:" << extension;
    QStringList constraints;
    if (!pluginname.isEmpty())
        constraints << QString("[X-KDE-PluginInfo-Name]=='%1'").arg( pluginname );

    return pluginForExtension(extension, constraints);
}

IPlugin *PluginController::pluginForExtension(const QString &extension, const QStringList &constraints)
{
    //kDebug() << "Finding Plugin for Extension:" << extension << "|" << constraints;
    KPluginInfo::List infos = queryExtensionPlugins(extension, constraints);

    if( infos.isEmpty() )
        return 0;
    if( d->loadedPlugins.contains( infos.first() ) )
        return d->loadedPlugins[ infos.first() ];
    else
        return loadPluginInternal( infos.first().pluginName() );
}

QList<IPlugin*> PluginController::allPluginsForExtension(const QString &extension, const QStringList &constraints)
{
    //kDebug() << "Finding all Plugins for Extension:" << extension << "|" << constraints;
    KPluginInfo::List infos = queryExtensionPlugins(extension, constraints);
    QList<IPlugin*> plugins;
    foreach (const KPluginInfo &info, infos)
    {
        if( !isEnabled(info) )
            continue;

        IPlugin* plugin;
        if( d->loadedPlugins.contains( info ) )
            plugin = d->loadedPlugins[ info ];
        else
            plugin = loadPluginInternal( info.pluginName() );

        if (plugin)
            plugins << plugin;
        else
            kWarning(9501) << "Null plugin retrieved! Loading error?" << extension << constraints;
    }
    return plugins;
}

QStringList PluginController::allPluginNames()
{
    QStringList names;
    Q_FOREACH( const KPluginInfo& info , d->plugins )
    {
        names << info.pluginName();
    }
    return names;
}

QList<ContextMenuExtension> PluginController::queryPluginsForContextMenuExtensions( KDevelop::Context* context ) const
{
    QList<ContextMenuExtension> exts;
    Q_FOREACH( const KPluginInfo& info, d->loadedPlugins.keys() )
    {
        IPlugin* plug = d->loadedPlugins[info];
        exts << plug->contextMenuExtension( context );
    }
    exts << Core::self()->debugControllerInternal()->contextMenuExtension( context );
    exts << Core::self()->documentationControllerInternal()->contextMenuExtension( context );
    exts << Core::self()->sourceFormatterControllerInternal()->contextMenuExtension( context );
    exts << Core::self()->runControllerInternal()->contextMenuExtension( context );
    exts << Core::self()->projectControllerInternal()->contextMenuExtension( context );
    return exts;
}

QStringList PluginController::projectPlugins()
{
    QStringList names;
    Q_FOREACH( const KPluginInfo& info , d->plugins )
    {
        if( info.property("X-KDevelop-Category").toString() == "Project" )
            names << info.pluginName();
    }
    return names;
}

void PluginController::loadProjectPlugins()
{
    Q_FOREACH( const QString& name, projectPlugins() )
    {
        loadPluginInternal( name );
    }
}

void PluginController::unloadProjectPlugins()
{
    Q_FOREACH( const QString& name, projectPlugins() )
    {
        unloadPlugin( name );
    }
}

QList<KPluginInfo> PluginController::allPluginInfos() const
{
    return d->plugins;
}

void PluginController::updateLoadedPlugins()
{
    QStringList defaultPlugins = ShellExtension::getInstance()->defaultPlugins();
    KConfigGroup grp = Core::self()->activeSession()->config()->group( pluginControllerGrp );
    foreach( const KPluginInfo& info, d->plugins )
    {
        if( isGlobalPlugin( info ) )
        {
            bool enabled = grp.readEntry( info.pluginName()+"Enabled", ( defaultPlugins.isEmpty() || defaultPlugins.contains( info.pluginName() ) ) ) || !isUserSelectable( info );
            if( d->loadedPlugins.contains( info ) && !enabled )
            {
                kDebug() << "unloading" << info.pluginName();
                if( !unloadPlugin( info.pluginName() ) )
                {
                    grp.writeEntry( info.pluginName()+"Enabled", false );
                }
            } else if( !d->loadedPlugins.contains( info ) && enabled )
            {
                loadPluginInternal( info.pluginName() );
            }
        }
    }
}

void PluginController::resetToDefaults()
{
    KSharedConfig::Ptr cfg = Core::self()->activeSession()->config();
    cfg->deleteGroup( pluginControllerGrp );
    cfg->sync();
    KConfigGroup grp = cfg->group( pluginControllerGrp );
    QStringList plugins = ShellExtension::getInstance()->defaultPlugins();
    if( plugins.isEmpty() )
    {
        foreach( const KPluginInfo& info, d->plugins )
        {
            plugins << info.pluginName();
        }
    }
    foreach( const QString& s, plugins )
    {
        grp.writeEntry( s+"Enabled", true );
    }
    grp.sync();
}

}
#include "plugincontroller.moc"

