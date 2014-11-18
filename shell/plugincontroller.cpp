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
#include <QApplication>
#include <QMap>
#include <QtCore/QDebug>

#include <kservice.h>
#include <kservicetypetrader.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <KLocalizedString>
#include <kxmlguiwindow.h>
#include <assert.h>
#include <kxmlguifactory.h>
#include <kpluginloader.h>
#include <kpluginmetadata.h>
#include <KConfigGroup>

#include <interfaces/contextmenuextension.h>
#include <interfaces/iplugin.h>
#include <interfaces/isession.h>
#include <interfaces/idebugcontroller.h>
#include <interfaces/idocumentationcontroller.h>
#include <interfaces/ipluginversion.h>

#include "mainwindow.h"
#include "core.h"
#include "shellextension.h"
#include "runcontroller.h"
#include "debugcontroller.h"
#include "documentationcontroller.h"
#include "sourceformattercontroller.h"
#include "projectcontroller.h"
#include "debug.h"

namespace {

// TODO kf5: use QStringLiteral
const QString KEY_Plugins = "Plugins";

const QString KEY_LoadMode = "X-KDevelop-LoadMode";
const QString KEY_Category = "X-KDevelop-Category";
const QString KEY_Mode = "X-KDevelop-Mode";
const QString KEY_Version = "X-KDevelop-Version";
const QString KEY_Interfaces = "X-KDevelop-Interfaces";
const QString KEY_Required = "X-KDevelop-IRequired";
const QString KEY_Optional = "X-KDevelop-IOptional";

const QString KEY_Global = "Global";
const QString KEY_Project = "Project";
const QString KEY_Gui = "GUI";
const QString KEY_AlwaysOn = "AlwaysOn";
const QString KEY_UserSelectable = "UserSelectable";

bool isUserSelectable( const KPluginInfo& info )
{
    QString loadMode = info.property( KEY_LoadMode ).toString();
    return loadMode.isEmpty() || loadMode == KEY_UserSelectable;
}

bool isGlobalPlugin( const KPluginInfo& info )
{
    return info.property( KEY_Category ).toString() == KEY_Global;
}

bool hasMandatoryProperties( const KPluginInfo& info )
{
    QVariant mode = info.property( KEY_Mode );
    QVariant version = info.property( KEY_Version );

    return mode.isValid() && mode.canConvert( QVariant::String )
           && version.isValid() && version.canConvert( QVariant::String );
}

bool constraintsMatch( const KPluginInfo& info, const QVariantMap& constraints)
{
    for (auto it = constraints.begin(); it != constraints.end(); ++it) {
        const auto property = info.property(it.key());

        if (!property.isValid()) {
            return false;
        } else if (property.canConvert<QStringList>()) {
            QSet<QString> values = property.toStringList().toSet();
            QSet<QString> expected = it.value().toStringList().toSet();
            if (!values.contains(expected)) {
                return false;
            }
        } else if (it.value() != property) {
            return false;
        }
    }
    return true;
}

struct Dependency
{
    Dependency(const QString &dependency)
        : interface(dependency)
    {
        if (dependency.contains('@')) {
            const auto list = dependency.split('@', QString::SkipEmptyParts);
            if (list.size() == 2) {
                interface = list.at(0);
                pluginName = list.at(1);
            }
        }
    }

    QString interface;
    QString pluginName;
};

}

namespace KDevelop {

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
        qCDebug(SHELL) << "checking can unload for:" << plugin.name() << plugin.property(KEY_LoadMode);
        if( plugin.property( KEY_LoadMode ).toString() == KEY_AlwaysOn )
        {
            return false;
        }
        QStringList interfaces = plugin.property( KEY_Interfaces ).toStringList();
        qCDebug(SHELL) << "checking dependencies:" << interfaces;
        foreach( const KPluginInfo& info, loadedPlugins.keys() )
        {
            if( info.pluginName() != plugin.pluginName() )
            {
                QStringList dependencies = info.property( KEY_Required ).toStringList();
                dependencies += info.property( KEY_Optional ).toStringList();
                foreach( const QString& dep, dependencies )
                {
                    Dependency dependency(dep);
                    if (!dependency.pluginName.isEmpty() && dependency.pluginName != plugin.pluginName()) {
                        continue;
                    }
                    if (interfaces.contains(dependency.interface) && !canUnload(info)) {
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

    /**
     * Iterate over all cached plugin infos, and call the functor for every enabled plugin.
     *
     * If an extension and/or pluginName is given, the functor will only be called for
     * those plugins matching this information.
     *
     * The functor should return false when the iteration can be stopped, and true if it
     * should be continued.
     */
    template<typename F>
    void foreachEnabledPlugin(F func, const QString &extension = {}, const QVariantMap& constraints = {}, const QString &pluginName = {})
    {
        foreach (const auto& info, plugins) {
            if ((pluginName.isEmpty() || info.pluginName() == pluginName)
                && (extension.isEmpty() || info.property(KEY_Interfaces).toStringList().contains(extension))
                && constraintsMatch(info, constraints)
                && isEnabled(info))
            {
                if (!func(info)) {
                    break;
                }
            }
        }
    }

    bool isEnabled(const KPluginInfo& info) const
    {
        static const QStringList disabledPlugins = QString(qgetenv("KDEV_DISABLE_PLUGINS")).split(';');
        if (disabledPlugins.contains(info.pluginName())) {
            return false;
        }

        if (!isGlobalPlugin( info ) || !isUserSelectable( info )) {
            return true;
        }

        KConfigGroup grp = Core::self()->activeSession()->config()->group( KEY_Plugins );
        bool isEnabled = grp.readEntry( info.pluginName()+"Enabled", ShellExtension::getInstance()->defaultPlugins().isEmpty() || ShellExtension::getInstance()->defaultPlugins().contains( info.pluginName() ) );
        //qCDebug(SHELL) << "read config:" << isEnabled << "is global plugin:" << isGlobalPlugin( info ) << "default:" << ShellExtension::getInstance()->defaultPlugins().isEmpty()  << ShellExtension::getInstance()->defaultPlugins().contains( info.pluginName() );
        return isEnabled;
    }

    Core *core;
};

PluginController::PluginController(Core *core)
    : IPluginController(), d(new PluginControllerPrivate)
{
    setObjectName("PluginController");
    d->core = core;

    auto newPlugins = KPluginLoader::findPlugins("kdevplatform/" QT_STRINGIFY(KDEVELOP_PLUGIN_VERSION), [](const KPluginMetaData& meta) {
        if (meta.serviceTypes().contains(QStringLiteral("KDevelop/Plugin"))) {
            return true;
        } else {
            qWarning() << "Plugin" << meta.fileName() << "is installed into the kdevplatform plugin directory, but does not have"
                " \"KDevelop/Plugin\" set as the service type. This plugin will not be loaded.";
            return false;
        }
    });
    qCDebug(SHELL) << "Found" << newPlugins.size() << " plugins using the new search method.";
    d->plugins = KPluginInfo::fromMetaData(newPlugins);

    //qCDebug(SHELL) << "Fetching plugin info which matches:" << QString( "[X-KDevelop-Version] == %1" ).arg(KDEVELOP_PLUGIN_VERSION);
    KPluginInfo::List oldStylePlugins = KPluginInfo::fromServices( KServiceTypeTrader::self()->query( QStringLiteral( "KDevelop/Plugin" ),
        QString( "[X-KDevelop-Version] == %1" ).arg(KDEVELOP_PLUGIN_VERSION) ) );
    qCDebug(SHELL) << "Found" << oldStylePlugins.size() << " plugins using the old search method.";
    if (!oldStylePlugins.isEmpty()) {
        foreach (const KPluginInfo& info, oldStylePlugins) {
            qWarning() << "Plugin" << info.pluginName() << "still uses the old .desktop file based metadata."
                " It must be ported to JSON metadata or it will no longer work with future kdevplatform versions.";
        }
    }
    d->plugins.append(oldStylePlugins);

    d->cleanupMode = PluginControllerPrivate::Running;
    // Register the KDevelop::IPlugin* metatype so we can properly unload it
    qRegisterMetaType<KDevelop::IPlugin*>( "KDevelop::IPlugin*" );
}

PluginController::~PluginController()
{
    if ( d->cleanupMode != PluginControllerPrivate::CleanupDone ) {
        qCWarning(SHELL) << "Destructing plugin controller without going through the shutdown process! Backtrace is: " << endl;
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
        //qCDebug(SHELL) << "called when not running. state =" << d->cleanupMode;
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
    return d->isEnabled(info);
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

    KConfigGroup grp = Core::self()->activeSession()->config()->group( KEY_Plugins );
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
    qCDebug(SHELL) << "Unloading plugin:" << pluginId << "?" << thePlugin << canUnload;
    if( thePlugin && canUnload )
    {
        return unloadPlugin(thePlugin, Later);
    }
    return (canUnload && thePlugin);
}

bool PluginController::unloadPlugin(IPlugin* plugin, PluginDeletion deletion)
{
    qCDebug(SHELL) << "unloading plugin:" << plugin << pluginInfo( plugin ).name();

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
        qCWarning(SHELL) << "Unable to find a plugin named '" << pluginId << "'!" ;
        return 0L;
    }

    if ( IPlugin* plugin = d->loadedPlugins.value( info ) )
        return plugin;

    if( !isEnabled( info ) )
    {
        // Do not load disabled plugins
        qWarning() << "Not loading plugin named" << pluginId << "because its been disabled!";
        return 0;
    }

    if( !hasMandatoryProperties( info ) ) {
        qWarning() << "Unable to load plugin named " << pluginId << "! Doesn't have all mandatory properties set";
        return 0;
    }

    if( info.property(KEY_Mode) == KEY_Gui
        && Core::self()->setupFlags() == Core::NoUi )
    {
        qCDebug(SHELL) << "Not loading plugin named" << pluginId << "- Running in No-Ui mode, but the plugin says it needs a GUI";
        return 0;
    }

    qCDebug(SHELL) << "Attempting to load" << pluginId << "- name:" << info.name();

    emit loadingPlugin( info.pluginName() );
    IPlugin *plugin = 0;
    QStringList missingInterfaces;
    if ( checkForDependencies( info, missingInterfaces ) ) {

        if (!missingInterfaces.isEmpty()) {
            qCDebug(SHELL) << "Missing dependencies:" << missingInterfaces;
        }

        QString failedDependency;
        if( !loadDependencies( info, failedDependency ) ) {
            qWarning() << "Could not load a required dependency:" << failedDependency;
            return 0;
        }
        loadOptionalDependencies( info );

        KPluginLoader loader(info.service() ? *info.service() : KPluginName(info.libraryPath()));
        auto factory = loader.factory();
        if (factory) {
            plugin = factory->create<IPlugin>(d->core);
        } else {
            qWarning() << "Could not obtain factory to load plugin" << pluginId << loader.errorString();
        }
    }

    if ( plugin ) {
        if ( plugin->hasError() ) {
            qWarning() << i18n("Plugin '%1' could not be loaded correctly and was disabled.\nReason: %2.", info.name(), plugin->errorDescription());
            info.setPluginEnabled(false);
            info.save(Core::self()->activeSession()->config()->group(KEY_Plugins));
            unloadPlugin(pluginId);
            return 0;
        }
        d->loadedPlugins.insert( info, plugin );
        info.setPluginEnabled( true );

        qCDebug(SHELL) << "Successfully loaded plugin '" << pluginId << "'";
        emit pluginLoaded( plugin );
    } else {
        if( !missingInterfaces.isEmpty() ) {
            qWarning() << "Can't load plugin '" << pluginId
                    << "' some of its required dependencies could not be fulfilled:" << endl
                    << missingInterfaces.join(",") << endl;
        } else {
            qWarning() << "Loading plugin '" << pluginId << "' failed.";
        }
    }

    return plugin;
}


IPlugin* PluginController::plugin( const QString& pluginId )
{
    KPluginInfo info = infoForPluginId( pluginId );
    if ( !info.isValid() )
        return 0L;

    return d->loadedPlugins.value( info );
}

bool PluginController::checkForDependencies( const KPluginInfo& info, QStringList& missing ) const
{
    QVariant prop = info.property( KEY_Required );
    if( prop.canConvert<QStringList>() ) {
        QSet<QString> required = prop.toStringList().toSet();
        if (!required.isEmpty()) {
            d->foreachEnabledPlugin([&required] (const KPluginInfo& plugin) -> bool {
                foreach(const QString& iface, plugin.property( KEY_Interfaces ).toStringList()) {
                    required.remove(iface);
                    required.remove(iface + '@' + plugin.pluginName());
                }
                return !required.isEmpty();
            });
        }
        if (!required.isEmpty()) {
            missing = required.toList();
            return false;
        }
    }
    return true;
}

void PluginController::loadOptionalDependencies( const KPluginInfo& info )
{
    QVariant prop = info.property( KEY_Optional );
    if( prop.canConvert<QStringList>() ) {
        foreach( const QString &dep, prop.toStringList() ) {
            Dependency dependency(dep);
            if (!pluginForExtension(dependency.interface, dependency.pluginName)) {
                qCDebug(SHELL) << "Couldn't load optional dependency:" << dep << info.pluginName();
            }
        }
    }
}

bool PluginController::loadDependencies( const KPluginInfo& info, QString& failedDependency )
{
    QVariant prop = info.property( KEY_Required );
    if( prop.canConvert<QStringList>() ) {
        foreach( const QString &value, prop.toStringList() ) {
            Dependency dependency(value);
            if (!pluginForExtension(dependency.interface, dependency.pluginName)) {
                failedDependency = value;
                return false;
            }
        }
    }
    return true;
}

IPlugin *PluginController::pluginForExtension(const QString &extension, const QString &pluginName, const QVariantMap& constraints)
{
    IPlugin* plugin = nullptr;
    d->foreachEnabledPlugin([this, &plugin] (const KPluginInfo& info) -> bool {
        plugin = d->loadedPlugins.value( info );
        if( !plugin ) {
            plugin = loadPluginInternal( info.pluginName() );
        }
        return !plugin;
    }, extension, constraints, pluginName);

    return plugin;
}

QList<IPlugin*> PluginController::allPluginsForExtension(const QString &extension, const QVariantMap& constraints)
{
    //qCDebug(SHELL) << "Finding all Plugins for Extension:" << extension << "|" << constraints;
    QList<IPlugin*> plugins;
    d->foreachEnabledPlugin([this, &plugins] (const KPluginInfo& info) -> bool {
        IPlugin* plugin = d->loadedPlugins.value( info );
        if( !plugin) {
            plugin = loadPluginInternal( info.pluginName() );
        }
        if (plugin)
            plugins << plugin;
        return true;
    }, extension, constraints);
    return plugins;
}

KPluginInfo::List PluginController::queryExtensionPlugins(const QString& extension, const QVariantMap& constraints) const
{
    KPluginInfo::List plugins;
    d->foreachEnabledPlugin([&plugins] (const KPluginInfo& info) -> bool {
        plugins << info;
        return true;
    }, extension, constraints);
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
    for( auto it=d->loadedPlugins.constBegin(), itEnd = d->loadedPlugins.constEnd(); it!=itEnd; ++it )
    {
        IPlugin* plug = it.value();
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
        if( info.property(KEY_Category).toString() == KEY_Project )
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
    KConfigGroup grp = Core::self()->activeSession()->config()->group( KEY_Plugins );
    foreach( const KPluginInfo& info, d->plugins )
    {
        if( isGlobalPlugin( info ) )
        {
            bool enabled = grp.readEntry( info.pluginName()+"Enabled", ( defaultPlugins.isEmpty() || defaultPlugins.contains( info.pluginName() ) ) ) || !isUserSelectable( info );
            bool loaded = d->loadedPlugins.contains( info );
            if( loaded && !enabled )
            {
                qCDebug(SHELL) << "unloading" << info.pluginName();
                if( !unloadPlugin( info.pluginName() ) )
                {
                    grp.writeEntry( info.pluginName()+"Enabled", false );
                }
            } else if( !loaded && enabled )
            {
                loadPluginInternal( info.pluginName() );
            }
        }
    }
}

void PluginController::resetToDefaults()
{
    KSharedConfigPtr cfg = Core::self()->activeSession()->config();
    cfg->deleteGroup( KEY_Plugins );
    cfg->sync();
    KConfigGroup grp = cfg->group( KEY_Plugins );
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

