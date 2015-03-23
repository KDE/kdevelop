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

#include <QElapsedTimer>
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
#include <KPluginInfo> // TODO: remove once we no longer support old style plugins
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

const QString KEY_Plugins = QStringLiteral("Plugins");
const QString KEY_Suffix_Enabled = QStringLiteral("Enabled");

const QString KEY_LoadMode = QStringLiteral("X-KDevelop-LoadMode");
const QString KEY_Category = QStringLiteral("X-KDevelop-Category");
const QString KEY_Mode = QStringLiteral("X-KDevelop-Mode");
const QString KEY_Version = QStringLiteral("X-KDevelop-Version");
const QString KEY_Interfaces = QStringLiteral("X-KDevelop-Interfaces");
const QString KEY_Required = QStringLiteral("X-KDevelop-IRequired");
const QString KEY_Optional = QStringLiteral("X-KDevelop-IOptional");

const QString KEY_Global = QStringLiteral("Global");
const QString KEY_Project = QStringLiteral("Project");
const QString KEY_Gui = QStringLiteral("GUI");
const QString KEY_AlwaysOn = QStringLiteral("AlwaysOn");
const QString KEY_UserSelectable = QStringLiteral("UserSelectable");


bool isUserSelectable( const KPluginMetaData& info )
{
    QString loadMode = info.value(KEY_LoadMode);
    return loadMode.isEmpty() || loadMode == KEY_UserSelectable;
}

bool isGlobalPlugin( const KPluginMetaData& info )
{
    return info.value(KEY_Category) == KEY_Global;
}

bool hasMandatoryProperties( const KPluginMetaData& info )
{
    QString mode = info.value(KEY_Mode);
    if (mode.isEmpty()) {
        return false;
    }

    // when the plugin is installed into the versioned plugin path, it's good to go
    if (info.fileName().contains(QLatin1String("/kdevplatform/" QT_STRINGIFY(KDEVELOP_PLUGIN_VERSION) "/"))) {
        return true;
    }

    // the version property is only required when the plugin is not installed into the right directory
    QVariant version = info.rawData().value(KEY_Version).toVariant();
    if (version.isValid() && version.value<int>() == KDEVELOP_PLUGIN_VERSION) {
        return true;
    }

    return false;
}

bool constraintsMatch( const KPluginMetaData& info, const QVariantMap& constraints)
{
    for (auto it = constraints.begin(); it != constraints.end(); ++it) {
        const auto property = info.rawData().value(it.key()).toVariant();

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
    QVector<KPluginMetaData> plugins;

    //map plugin infos to currently loaded plugins
    typedef QHash<KPluginMetaData, IPlugin*> InfoToPluginMap;
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

    bool canUnload(const KPluginMetaData& plugin)
    {
        qCDebug(SHELL) << "checking can unload for:" << plugin.name() << plugin.value(KEY_LoadMode);
        if (plugin.value(KEY_LoadMode) == KEY_AlwaysOn) {
            return false;
        }
        const QStringList interfaces = KPluginMetaData::readStringList(plugin.rawData(), KEY_Interfaces);
        qCDebug(SHELL) << "checking dependencies:" << interfaces;
        foreach (const KPluginMetaData& info, loadedPlugins.keys()) {
            if (info.pluginId() != plugin.pluginId()) {
                QStringList dependencies = KPluginMetaData::readStringList(plugin.rawData(), KEY_Required);
                dependencies += KPluginMetaData::readStringList(plugin.rawData(), KEY_Optional);
                foreach (const QString& dep, dependencies) {
                    Dependency dependency(dep);
                    if (!dependency.pluginName.isEmpty() && dependency.pluginName != plugin.pluginId()) {
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

    KPluginMetaData infoForId( const QString& id ) const
    {
        foreach (const KPluginMetaData& info, plugins) {
            if (info.pluginId() == id) {
                return info;
            }
        }
        return KPluginMetaData();
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
    void foreachEnabledPlugin(F func, const QString &extension = {}, const QVariantMap& constraints = QVariantMap(), const QString &pluginName = {})
    {
        foreach (const auto& info, plugins) {
            if ((pluginName.isEmpty() || info.pluginId() == pluginName)
                && (extension.isEmpty() || KPluginMetaData::readStringList(info.rawData(), KEY_Interfaces).contains(extension))
                && constraintsMatch(info, constraints)
                && isEnabled(info))
            {
                if (!func(info)) {
                    break;
                }
            }
        }
    }

    bool isEnabled(const KPluginMetaData& info) const
    {
        static const QStringList disabledPlugins = QString::fromLatin1(qgetenv("KDEV_DISABLE_PLUGINS")).split(';');
        if (disabledPlugins.contains(info.pluginId())) {
            return false;
        }

        if (!isGlobalPlugin( info ) || !isUserSelectable( info )) {
            return true;
        }

        KConfigGroup grp = Core::self()->activeSession()->config()->group( KEY_Plugins );
        const bool isDefaultPlugin = ShellExtension::getInstance()->defaultPlugins().isEmpty() || ShellExtension::getInstance()->defaultPlugins().contains(info.pluginId());
        bool isEnabled = grp.readEntry(info.pluginId() + KEY_Suffix_Enabled, isDefaultPlugin);
        //qCDebug(SHELL) << "read config:" << isEnabled << "is global plugin:" << isGlobalPlugin( info ) << "default:" << ShellExtension::getInstance()->defaultPlugins().isEmpty()  << ShellExtension::getInstance()->defaultPlugins().contains( info.pluginId() );
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
    d->plugins = newPlugins;

    //qCDebug(SHELL) << "Fetching plugin info which matches:" << QStringLiteral( "[X-KDevelop-Version] == %1" ).arg(KDEVELOP_PLUGIN_VERSION);
    const KPluginInfo::List oldStylePlugins = KPluginInfo::fromServices( KServiceTypeTrader::self()->query( QStringLiteral( "KDevelop/Plugin" ),
        QStringLiteral( "[X-KDevelop-Version] == %1" ).arg(KDEVELOP_PLUGIN_VERSION) ) );
    qCDebug(SHELL) << "Found" << oldStylePlugins.size() << " plugins using the old search method.";
    if (!oldStylePlugins.isEmpty()) {
        foreach (const KPluginInfo& info, oldStylePlugins) {
            qWarning() << "Plugin" << info.pluginName() << "still uses the old .desktop file based metadata."
                " It must be ported to JSON metadata or it will no longer work with future kdevplatform versions.";
            d->plugins.append(info.toMetaData());
        }
    }

    d->cleanupMode = PluginControllerPrivate::Running;
    // Register the KDevelop::IPlugin* metatype so we can properly unload it
    qRegisterMetaType<KDevelop::IPlugin*>( "KDevelop::IPlugin*" );
}

PluginController::~PluginController()
{
    if ( d->cleanupMode != PluginControllerPrivate::CleanupDone ) {
        qCWarning(SHELL) << "Destructing plugin controller without going through the shutdown process!";
    }

    delete d;
}

KPluginMetaData PluginController::pluginInfo( const IPlugin* plugin ) const
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

bool PluginController::isEnabled( const KPluginMetaData& info )
{
    return d->isEnabled(info);
}

void PluginController::initialize()
{
    QMap<QString, bool> pluginMap;
    if( ShellExtension::getInstance()->defaultPlugins().isEmpty() )
    {
        foreach( const KPluginMetaData& pi, d->plugins )
        {
            pluginMap.insert( pi.pluginId(), true );
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
        if (key.endsWith(KEY_Suffix_Enabled)) {
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

    foreach( const KPluginMetaData& pi, d->plugins )
    {
        if( isGlobalPlugin( pi ) )
        {
            QMap<QString, bool>::const_iterator it = pluginMap.constFind( pi.pluginId() );
            if( it != pluginMap.constEnd() && ( it.value() || !isUserSelectable( pi ) ) )
            {
                // Plugin is mentioned in pluginmap and the value is true, so try to load it
                loadPluginInternal( pi.pluginId() );
                if(!grp.hasKey(pi.pluginId() + KEY_Suffix_Enabled)) {
                    if( isUserSelectable( pi ) )
                    {
                        // If plugin isn't listed yet, add it with true now
                        grp.writeEntry(pi.pluginId() + KEY_Suffix_Enabled, true);
                    }
                } else if( grp.hasKey( pi.pluginId() + "Disabled" ) && !isUserSelectable( pi ) )
                {
                    // Remove now-obsolete entries
                    grp.deleteEntry( pi.pluginId() + "Disabled" );
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

KPluginMetaData PluginController::infoForPluginId( const QString &pluginId ) const
{
    foreach (const KPluginMetaData& info, d->plugins) {
        if (info.pluginId() == pluginId) {
            return info;
        }
    }

    return KPluginMetaData();
}

IPlugin *PluginController::loadPluginInternal( const QString &pluginId )
{
    QElapsedTimer timer;
    timer.start();

    KPluginMetaData info = infoForPluginId( pluginId );
    if ( !info.isValid() ) {
        qCWarning(SHELL) << "Unable to find a plugin named '" << pluginId << "'!" ;
        return nullptr;
    }

    if ( IPlugin* plugin = d->loadedPlugins.value( info ) ) {
        return plugin;
    }

    if ( !isEnabled( info ) ) {
        // Do not load disabled plugins
        qWarning() << "Not loading plugin named" << pluginId << "because it has been disabled!";
        return nullptr;
    }

    if ( !hasMandatoryProperties( info ) ) {
        qWarning() << "Unable to load plugin named" << pluginId << "because not all mandatory properties are set.";
        return nullptr;
    }

    if ( info.value(KEY_Mode) == KEY_Gui && Core::self()->setupFlags() == Core::NoUi ) {
        qCDebug(SHELL) << "Not loading plugin named" << pluginId
                       << "- Running in No-Ui mode, but the plugin says it needs a GUI";
        return nullptr;
    }

    qCDebug(SHELL) << "Attempting to load" << pluginId << "- name:" << info.name();

    emit loadingPlugin( info.pluginId() );

    // first, ensure all dependencies are available and not disabled
    // this is unrelated to whether they are loaded already or not.
    // when we depend on e.g. A and B, but B cannot be found, then we
    // do not want to load A first and then fail on B and leave A loaded.
    // this would happen if we'd skip this step here and directly loadDependencies.
    QStringList missingInterfaces;
    if ( !hasUnresolvedDependencies( info, missingInterfaces ) ) {
        qWarning() << "Can't load plugin" << pluginId
                   << "some of its required dependencies could not be fulfilled:"
                   << missingInterfaces.join(",");
        return nullptr;
    }

    // now ensure all dependencies are loaded
    QString failedDependency;
    if( !loadDependencies( info, failedDependency ) ) {
        qWarning() << "Can't load plugin" << pluginId
                   << "because a required dependency could not be loaded:" << failedDependency;
        return nullptr;
    }

    // same for optional dependencies, but don't error out if anything fails
    loadOptionalDependencies( info );

    // now we can finally load the plugin itself
    KPluginLoader loader(info.fileName());
    auto factory = loader.factory();
    if (!factory) {
        qWarning() << "Can't load plugin" << pluginId
                   << "because a factory to load the plugin could not be obtained:" << loader.errorString();
        return nullptr;
    }

    // now create it
    auto plugin = factory->create<IPlugin>(d->core);
    if (!plugin) {
        qWarning() << "Creating plugin" << pluginId << "failed.";
        return nullptr;
    }

    KConfigGroup group = Core::self()->activeSession()->config()->group(KEY_Plugins);
    // runtime errors such as missing executables on the system or such get checked now
    if (plugin->hasError()) {
        qWarning() << "Could not load plugin" << pluginId << ", it reported the error:" << plugin->errorDescription()
                    << "Disabling the plugin now.";
        group.writeEntry(info.pluginId() + KEY_Suffix_Enabled, false); // do the same as KPluginInfo did
        group.sync();
        unloadPlugin(pluginId);
        return nullptr;
    }

    // yay, it all worked - the plugin is loaded
    d->loadedPlugins.insert(info, plugin);
    group.writeEntry(info.pluginId() + KEY_Suffix_Enabled, true); // do the same as KPluginInfo did
    group.sync();
    qCDebug(SHELL) << "Successfully loaded plugin" << pluginId << "from" << loader.fileName() << "- took:" << timer.elapsed() << "ms";
    emit pluginLoaded( plugin );

    return plugin;
}


IPlugin* PluginController::plugin( const QString& pluginId )
{
    KPluginMetaData info = infoForPluginId( pluginId );
    if ( !info.isValid() )
        return 0L;

    return d->loadedPlugins.value( info );
}

bool PluginController::hasUnresolvedDependencies( const KPluginMetaData& info, QStringList& missing ) const
{
    QSet<QString> required = KPluginMetaData::readStringList(info.rawData(), KEY_Required).toSet();
    if (!required.isEmpty()) {
        d->foreachEnabledPlugin([&required] (const KPluginMetaData& plugin) -> bool {
            foreach (const QString& iface, KPluginMetaData::readStringList(plugin.rawData(), KEY_Interfaces)) {
                required.remove(iface);
                required.remove(iface + '@' + plugin.pluginId());
            }
            return !required.isEmpty();
        });
    }
    // if we found all dependencies required should be empty now
    if (!required.isEmpty()) {
        missing = required.toList();
        return false;
    }
    return true;
}

void PluginController::loadOptionalDependencies( const KPluginMetaData& info )
{
   const QStringList dependencies = KPluginMetaData::readStringList(info.rawData(), KEY_Optional);
   foreach (const QString& dep, dependencies) {
        Dependency dependency(dep);
        if (!pluginForExtension(dependency.interface, dependency.pluginName)) {
            qCDebug(SHELL) << "Couldn't load optional dependency:" << dep << info.pluginId();
        }
    }
}

bool PluginController::loadDependencies( const KPluginMetaData& info, QString& failedDependency )
{
   const QStringList dependencies = KPluginMetaData::readStringList(info.rawData(), KEY_Optional);
   foreach (const QString& value, dependencies) {
        Dependency dependency(value);
        if (!pluginForExtension(dependency.interface, dependency.pluginName)) {
            failedDependency = value;
            return false;
        }
    }
    return true;
}

IPlugin *PluginController::pluginForExtension(const QString &extension, const QString &pluginName, const QVariantMap& constraints)
{
    IPlugin* plugin = nullptr;
    d->foreachEnabledPlugin([this, &plugin] (const KPluginMetaData& info) -> bool {
        plugin = d->loadedPlugins.value( info );
        if( !plugin ) {
            plugin = loadPluginInternal( info.pluginId() );
        }
        return !plugin;
    }, extension, constraints, pluginName);

    return plugin;
}

QList<IPlugin*> PluginController::allPluginsForExtension(const QString &extension, const QVariantMap& constraints)
{
    //qCDebug(SHELL) << "Finding all Plugins for Extension:" << extension << "|" << constraints;
    QList<IPlugin*> plugins;
    d->foreachEnabledPlugin([this, &plugins] (const KPluginMetaData& info) -> bool {
        IPlugin* plugin = d->loadedPlugins.value( info );
        if( !plugin) {
            plugin = loadPluginInternal( info.pluginId() );
        }
        if (plugin)
            plugins << plugin;
        return true;
    }, extension, constraints);
    return plugins;
}

QVector<KPluginMetaData> PluginController::queryExtensionPlugins(const QString& extension, const QVariantMap& constraints) const
{
    QVector<KPluginMetaData> plugins;
    d->foreachEnabledPlugin([&plugins] (const KPluginMetaData& info) -> bool {
        plugins << info;
        return true;
    }, extension, constraints);
    return plugins;
}

QStringList PluginController::allPluginNames()
{
    QStringList names;
    Q_FOREACH( const KPluginMetaData& info , d->plugins )
    {
        names << info.pluginId();
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
    foreach (const KPluginMetaData& info, d->plugins) {
        if (info.value(KEY_Category) == KEY_Project) {
            names << info.pluginId();
        }
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

QVector<KPluginMetaData> PluginController::allPluginInfos() const
{
    return d->plugins;
}

void PluginController::updateLoadedPlugins()
{
    QStringList defaultPlugins = ShellExtension::getInstance()->defaultPlugins();
    KConfigGroup grp = Core::self()->activeSession()->config()->group( KEY_Plugins );
    foreach( const KPluginMetaData& info, d->plugins )
    {
        if( isGlobalPlugin( info ) )
        {
            bool enabled = grp.readEntry(info.pluginId() + KEY_Suffix_Enabled, ( defaultPlugins.isEmpty() || defaultPlugins.contains( info.pluginId() ) ) ) || !isUserSelectable( info );
            bool loaded = d->loadedPlugins.contains( info );
            if( loaded && !enabled )
            {
                qCDebug(SHELL) << "unloading" << info.pluginId();
                if( !unloadPlugin( info.pluginId() ) )
                {
                    grp.writeEntry( info.pluginId() + KEY_Suffix_Enabled, false );
                }
            } else if( !loaded && enabled )
            {
                loadPluginInternal( info.pluginId() );
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
        foreach( const KPluginMetaData& info, d->plugins )
        {
            plugins << info.pluginId();
        }
    }
    foreach( const QString& s, plugins )
    {
        grp.writeEntry(s + KEY_Suffix_Enabled, true);
    }
    grp.sync();
}

}

