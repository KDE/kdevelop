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
#include <QMap>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KPluginLoader>

#include <interfaces/contextmenuextension.h>
#include <interfaces/iplugin.h>
#include <interfaces/isession.h>
#include <interfaces/idebugcontroller.h>
#include <interfaces/idocumentationcontroller.h>
#include <interfaces/ipluginversion.h>

#include "core.h"
#include "shellextension.h"
#include "runcontroller.h"
#include "debugcontroller.h"
#include "documentationcontroller.h"
#include "sourceformattercontroller.h"
#include "projectcontroller.h"
#include "ktexteditorpluginintegration.h"
#include "debug.h"

namespace {

inline QString KEY_Plugins() { return QStringLiteral("Plugins"); }
inline QString KEY_Suffix_Enabled() { return QStringLiteral("Enabled"); }

inline QString KEY_LoadMode() { return QStringLiteral("X-KDevelop-LoadMode"); }
inline QString KEY_Category() { return QStringLiteral("X-KDevelop-Category"); }
inline QString KEY_Mode() { return QStringLiteral("X-KDevelop-Mode"); }
inline QString KEY_Version() { return QStringLiteral("X-KDevelop-Version"); }
inline QString KEY_Interfaces() { return QStringLiteral("X-KDevelop-Interfaces"); }
inline QString KEY_Required() { return QStringLiteral("X-KDevelop-IRequired"); }
inline QString KEY_Optional() { return QStringLiteral("X-KDevelop-IOptional"); }
inline QString KEY_KPlugin() { return QStringLiteral("KPlugin"); }
inline QString KEY_EnabledByDefault() { return QStringLiteral("EnabledByDefault"); }

inline QString KEY_Global() { return QStringLiteral("Global"); }
inline QString KEY_Project() { return QStringLiteral("Project"); }
inline QString KEY_Gui() { return QStringLiteral("GUI"); }
inline QString KEY_AlwaysOn() { return QStringLiteral("AlwaysOn"); }
inline QString KEY_UserSelectable() { return QStringLiteral("UserSelectable"); }

bool isUserSelectable( const KPluginMetaData& info )
{
    QString loadMode = info.value(KEY_LoadMode());
    return loadMode.isEmpty() || loadMode == KEY_UserSelectable();
}

bool isGlobalPlugin( const KPluginMetaData& info )
{
    return info.value(KEY_Category()) == KEY_Global();
}

bool hasMandatoryProperties( const KPluginMetaData& info )
{
    QString mode = info.value(KEY_Mode());
    if (mode.isEmpty()) {
        return false;
    }

    // when the plugin is installed into the versioned plugin path, it's good to go
    if (info.fileName().contains(QLatin1String("/kdevplatform/" QT_STRINGIFY(KDEVELOP_PLUGIN_VERSION) "/"))) {
        return true;
    }

    // the version property is only required when the plugin is not installed into the right directory
    QVariant version = info.rawData().value(KEY_Version()).toVariant();
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
    explicit Dependency(const QString &dependency)
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
        qCDebug(SHELL) << "checking can unload for:" << plugin.name() << plugin.value(KEY_LoadMode());
        if (plugin.value(KEY_LoadMode()) == KEY_AlwaysOn()) {
            return false;
        }
        const QStringList interfaces = KPluginMetaData::readStringList(plugin.rawData(), KEY_Interfaces());
        qCDebug(SHELL) << "checking dependencies:" << interfaces;
        foreach (const KPluginMetaData& info, loadedPlugins.keys()) {
            if (info.pluginId() != plugin.pluginId()) {
                QStringList dependencies = KPluginMetaData::readStringList(plugin.rawData(), KEY_Required());
                dependencies += KPluginMetaData::readStringList(plugin.rawData(), KEY_Optional());
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
                && (extension.isEmpty() || KPluginMetaData::readStringList(info.rawData(), KEY_Interfaces()).contains(extension))
                && constraintsMatch(info, constraints)
                && isEnabled(info))
            {
                if (!func(info)) {
                    break;
                }
            }
        }
    }

    enum EnableState {
        DisabledByEnv,
        DisabledBySetting,
        DisabledByUnknown,

        FirstEnabledState,
        EnabledBySetting = FirstEnabledState,
        AlwaysEnabled
    };

    /**
     * Estimate enabled state of a plugin
     */
    EnableState enabledState(const KPluginMetaData& info) const
    {
        // first check black listing from environment
        static const QStringList disabledPlugins = QString::fromLatin1(qgetenv("KDEV_DISABLE_PLUGINS")).split(';');
        if (disabledPlugins.contains(info.pluginId())) {
            return DisabledByEnv;
        }

        if (!isUserSelectable( info ))
            return AlwaysEnabled;

        // read stored user preference
        const KConfigGroup grp = Core::self()->activeSession()->config()->group( KEY_Plugins() );
        const QString pluginEnabledKey = info.pluginId() + KEY_Suffix_Enabled();
        if (grp.hasKey(pluginEnabledKey)) {
            return grp.readEntry(pluginEnabledKey, true) ? EnabledBySetting : DisabledBySetting;
        }

        // should not happen
        return DisabledByUnknown;
    }

    /**
     * Decide whether a plugin is enabled
     */
    bool isEnabled(const KPluginMetaData& info) const
    {
        return (enabledState(info) >= FirstEnabledState);
    }

    Core *core;
};

PluginController::PluginController(Core *core)
    : IPluginController(), d(new PluginControllerPrivate)
{
    setObjectName(QStringLiteral("PluginController"));
    d->core = core;

    QSet<QString> foundPlugins;
    auto newPlugins = KPluginLoader::findPlugins(QStringLiteral("kdevplatform/" QT_STRINGIFY(KDEVELOP_PLUGIN_VERSION)), [&](const KPluginMetaData& meta) {
        if (meta.serviceTypes().contains(QStringLiteral("KDevelop/Plugin"))) {
            foundPlugins.insert(meta.pluginId());
            return true;
        } else {
            qCWarning(SHELL) << "Plugin" << meta.fileName() << "is installed into the kdevplatform plugin directory, but does not have"
                " \"KDevelop/Plugin\" set as the service type. This plugin will not be loaded.";
            return false;
        }
    });

    qCDebug(SHELL) << "Found" << newPlugins.size() << "plugins:" << foundPlugins;
    if (newPlugins.isEmpty()) {
        qCWarning(SHELL) << "Did not find any plugins, check your environment.";
        qCWarning(SHELL) << "  Note: QT_PLUGIN_PATH is set to:" << qgetenv("QT_PLUGIN_PATH");
    }

    d->plugins = newPlugins;

    KTextEditorIntegration::initialize();
    const QVector<KPluginMetaData> ktePlugins = KPluginLoader::findPlugins(QStringLiteral("ktexteditor"), [](const KPluginMetaData & md) {
        return md.serviceTypes().contains(QStringLiteral("KTextEditor/Plugin"))
            && md.serviceTypes().contains(QStringLiteral("KDevelop/Plugin"));
    });

    foundPlugins.clear();
    std::for_each(ktePlugins.cbegin(), ktePlugins.cend(), [&foundPlugins](const KPluginMetaData& data) {
        foundPlugins << data.pluginId();
    });
    qCDebug(SHELL) << "Found" << ktePlugins.size() << " KTextEditor plugins:" << foundPlugins;

    foreach (const auto& info, ktePlugins) {
        auto data = info.rawData();
        // add some KDevelop specific JSON data
        data[KEY_Category()] = KEY_Global();
        data[KEY_Mode()] = KEY_Gui();
        data[KEY_Version()] = KDEVELOP_PLUGIN_VERSION;
        d->plugins.append({data, info.fileName(), info.metaDataFileName()});
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

void PluginController::initialize()
{
    QElapsedTimer timer;
    timer.start();

    QMap<QString, bool> pluginMap;
    if( ShellExtension::getInstance()->defaultPlugins().isEmpty() )
    {
        foreach( const KPluginMetaData& pi, d->plugins )
        {
            QJsonValue enabledByDefaultValue = pi.rawData()[KEY_KPlugin()].toObject()[KEY_EnabledByDefault()];
            // plugins enabled until explicitly specified otherwise
            const bool enabledByDefault = (enabledByDefaultValue.isNull() || enabledByDefaultValue.toBool());
            pluginMap.insert(pi.pluginId(), enabledByDefault);
        }
    } else
    {
        // Get the default from the ShellExtension
        foreach( const QString& s, ShellExtension::getInstance()->defaultPlugins() )
        {
            pluginMap.insert( s, true );
        }
    }

    KConfigGroup grp = Core::self()->activeSession()->config()->group( KEY_Plugins() );
    QMap<QString, QString> entries = grp.entryMap();

    QMap<QString, QString>::Iterator it;
    for ( it = entries.begin(); it != entries.end(); ++it )
    {
        const QString key = it.key();
        if (key.endsWith(KEY_Suffix_Enabled())) {
            const QString pluginid = key.left(key.length() - 7);
            const bool defValue = pluginMap.value( pluginid, false );
            const bool enabled = grp.readEntry(key, defValue);
            pluginMap.insert( pluginid, enabled );
        }
    }

    // store current known set of enabled plugins
    foreach (const KPluginMetaData& pi, d->plugins) {
        if (isUserSelectable(pi)) {
            auto it = pluginMap.constFind(pi.pluginId());
            if (it != pluginMap.constEnd() && (it.value())) {
                grp.writeEntry(pi.pluginId() + KEY_Suffix_Enabled(), true);
            }
        } else {
            // Backward compat: Remove any now-obsolete entries
            grp.deleteEntry(pi.pluginId() + QLatin1String("Disabled"));
        }
    }
    // Synchronize so we're writing out to the file.
    grp.sync();

    // load global plugins
    foreach (const KPluginMetaData& pi, d->plugins) {
        if (isGlobalPlugin(pi)) {
            loadPluginInternal(pi.pluginId());
        }
    }

    qCDebug(SHELL) << "Done loading plugins - took:" << timer.elapsed() << "ms";
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

    const auto enabledState = d->enabledState(info);
    if (enabledState < PluginControllerPrivate::FirstEnabledState) {
        // Do not load disabled plugins
        qCDebug(SHELL) << "Not loading plugin named" << pluginId << (
            (enabledState == PluginControllerPrivate::DisabledByEnv) ?
                "because disabled by KDEV_DISABLE_PLUGINS." :
            (enabledState == PluginControllerPrivate::DisabledBySetting) ?
                "because disabled by setting." :
            /* else, should not happen */
                "because disabled for unknown reason.");
        return nullptr;
    }

    if ( !hasMandatoryProperties( info ) ) {
        qCWarning(SHELL) << "Unable to load plugin named" << pluginId << "because not all mandatory properties are set.";
        return nullptr;
    }

    if ( info.value(KEY_Mode()) == KEY_Gui() && Core::self()->setupFlags() == Core::NoUi ) {
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
        qCWarning(SHELL) << "Can't load plugin" << pluginId
                   << "some of its required dependencies could not be fulfilled:"
                   << missingInterfaces.join(QStringLiteral(","));
        return nullptr;
    }

    // now ensure all dependencies are loaded
    QString failedDependency;
    if( !loadDependencies( info, failedDependency ) ) {
        qCWarning(SHELL) << "Can't load plugin" << pluginId
                   << "because a required dependency could not be loaded:" << failedDependency;
        return nullptr;
    }

    // same for optional dependencies, but don't error out if anything fails
    loadOptionalDependencies( info );

    // now we can finally load the plugin itself
    KPluginLoader loader(info.fileName());
    auto factory = loader.factory();
    if (!factory) {
        qCWarning(SHELL) << "Can't load plugin" << pluginId
                   << "because a factory to load the plugin could not be obtained:" << loader.errorString();
        return nullptr;
    }

    // now create it
    auto plugin = factory->create<IPlugin>(d->core);
    if (!plugin) {
        if (auto katePlugin = factory->create<KTextEditor::Plugin>(d->core, QVariantList() << info.pluginId())) {
            plugin = new KTextEditorIntegration::Plugin(katePlugin, d->core);
        } else {
            qCWarning(SHELL) << "Creating plugin" << pluginId << "failed.";
            return nullptr;
        }
    }

    KConfigGroup group = Core::self()->activeSession()->config()->group(KEY_Plugins());
    // runtime errors such as missing executables on the system or such get checked now
    if (plugin->hasError()) {
        qCWarning(SHELL) << "Could not load plugin" << pluginId << ", it reported the error:" << plugin->errorDescription()
                    << "Disabling the plugin now.";
        group.writeEntry(info.pluginId() + KEY_Suffix_Enabled(), false); // do the same as KPluginInfo did
        group.sync();
        unloadPlugin(pluginId);
        return nullptr;
    }

    // yay, it all worked - the plugin is loaded
    d->loadedPlugins.insert(info, plugin);
    group.writeEntry(info.pluginId() + KEY_Suffix_Enabled(), true); // do the same as KPluginInfo did
    group.sync();
    qCDebug(SHELL) << "Successfully loaded plugin" << pluginId << "from" << loader.fileName() << "- took:" << timer.elapsed() << "ms";
    emit pluginLoaded( plugin );

    return plugin;
}


IPlugin* PluginController::plugin( const QString& pluginId )
{
    KPluginMetaData info = infoForPluginId( pluginId );
    if ( !info.isValid() )
        return nullptr;

    return d->loadedPlugins.value( info );
}

bool PluginController::hasUnresolvedDependencies( const KPluginMetaData& info, QStringList& missing ) const
{
    QSet<QString> required = KPluginMetaData::readStringList(info.rawData(), KEY_Required()).toSet();
    if (!required.isEmpty()) {
        d->foreachEnabledPlugin([&required] (const KPluginMetaData& plugin) -> bool {
            foreach (const QString& iface, KPluginMetaData::readStringList(plugin.rawData(), KEY_Interfaces())) {
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
   const QStringList dependencies = KPluginMetaData::readStringList(info.rawData(), KEY_Optional());
   foreach (const QString& dep, dependencies) {
        Dependency dependency(dep);
        if (!pluginForExtension(dependency.interface, dependency.pluginName)) {
            qCDebug(SHELL) << "Couldn't load optional dependency:" << dep << info.pluginId();
        }
    }
}

bool PluginController::loadDependencies( const KPluginMetaData& info, QString& failedDependency )
{
   const QStringList dependencies = KPluginMetaData::readStringList(info.rawData(), KEY_Required());
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
        if (plugin && !plugins.contains(plugin)) {
            plugins << plugin;
        }
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

QList<ContextMenuExtension> PluginController::queryPluginsForContextMenuExtensions(KDevelop::Context* context, QWidget* parent) const
{
    // This fixes random order of extension menu items between different runs of KDevelop.
    // Without sorting we have random reordering of "Analyze With" submenu for example:
    // 1) "Cppcheck" actions, "Vera++" actions - first run
    // 2) "Vera++" actions, "Cppcheck" actions - some other run.
    QMultiMap<QString, IPlugin*> sortedPlugins;
    for (auto it = d->loadedPlugins.constBegin(); it != d->loadedPlugins.constEnd(); ++it) {
        sortedPlugins.insert(it.key().name(), it.value());
    }

    QList<ContextMenuExtension> exts;
    foreach (IPlugin* plugin, sortedPlugins) {
        exts << plugin->contextMenuExtension(context, parent);
    }

    exts << Core::self()->debugControllerInternal()->contextMenuExtension(context, parent);
    exts << Core::self()->documentationControllerInternal()->contextMenuExtension(context, parent);
    exts << Core::self()->sourceFormatterControllerInternal()->contextMenuExtension(context, parent);
    exts << Core::self()->runControllerInternal()->contextMenuExtension(context, parent);
    exts << Core::self()->projectControllerInternal()->contextMenuExtension(context, parent);

    return exts;
}

QStringList PluginController::projectPlugins()
{
    QStringList names;
    foreach (const KPluginMetaData& info, d->plugins) {
        if (info.value(KEY_Category()) == KEY_Project()) {
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
    KConfigGroup grp = Core::self()->activeSession()->config()->group( KEY_Plugins() );
    foreach( const KPluginMetaData& info, d->plugins )
    {
        if( isGlobalPlugin( info ) )
        {
            bool enabled = grp.readEntry(info.pluginId() + KEY_Suffix_Enabled(), ( defaultPlugins.isEmpty() || defaultPlugins.contains( info.pluginId() ) ) ) || !isUserSelectable( info );
            bool loaded = d->loadedPlugins.contains( info );
            if( loaded && !enabled )
            {
                qCDebug(SHELL) << "unloading" << info.pluginId();
                if( !unloadPlugin( info.pluginId() ) )
                {
                    grp.writeEntry( info.pluginId() + KEY_Suffix_Enabled(), false );
                }
            } else if( !loaded && enabled )
            {
                loadPluginInternal( info.pluginId() );
            }
        }
        // TODO: what about project plugins? what about dependency plugins?
    }
}

void PluginController::resetToDefaults()
{
    KSharedConfigPtr cfg = Core::self()->activeSession()->config();
    cfg->deleteGroup( KEY_Plugins() );
    cfg->sync();
    KConfigGroup grp = cfg->group( KEY_Plugins() );
    QStringList plugins = ShellExtension::getInstance()->defaultPlugins();
    if( plugins.isEmpty() )
    {
        foreach( const KPluginMetaData& info, d->plugins )
        {
            if (!isUserSelectable(info)) {
                continue;
            }

            QJsonValue enabledByDefault = info.rawData()[KEY_KPlugin()].toObject()[KEY_EnabledByDefault()];
            // plugins enabled until explicitly specified otherwise
            if (enabledByDefault.isNull() || enabledByDefault.toBool()) {
                plugins << info.pluginId();
            }
        }
    }
    foreach( const QString& s, plugins )
    {
        grp.writeEntry(s + KEY_Suffix_Enabled(), true);
    }
    grp.sync();
}

}

