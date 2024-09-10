/*
    SPDX-FileCopyrightText: 2004, 2007 Alexander Dymo <adymo@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Matt Rogers <mattr@kde.org
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    Based on code from Kopete
    SPDX-FileCopyrightText: 2002-2003 Martijn Klingens <klingens@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "plugincontroller.h"

#include <algorithm>

#include <QElapsedTimer>
#include <QJsonArray>
#include <QJsonObject>
#include <QMap>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>

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

inline QStringList DisabledLSPLanguages() {
    return {
        QStringLiteral("c"),
        QStringLiteral("cpp"),
        QStringLiteral("python"),
        QStringLiteral("php")
    };
}

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

inline QSet<QString> stringSet(const QVariant& variant)
{
    const QStringList list = variant.toStringList();
    return QSet<QString>(list.begin(), list.end());
}

bool constraintsMatch( const KPluginMetaData& info, const QVariantMap& constraints)
{
    for (auto it = constraints.begin(); it != constraints.end(); ++it) {
        const auto property = info.rawData().value(it.key()).toVariant();

        if (!property.isValid()) {
            return false;
        } else if (property.canConvert<QStringList>()) {
            const QSet<QString> values = stringSet(property);
            const QSet<QString> expected = stringSet(it.value());
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
    {
        const int pos = dependency.indexOf(QLatin1Char('@'));
        if (pos != -1) {
            interface = dependency.left(pos);
            pluginName = dependency.mid(pos + 1);
        } else {
            interface = dependency;
        }
    }

    QString interface;
    QString pluginName;
};

QVector<QString> pluginIds(const QVector<KPluginMetaData> &plugins)
{
    QVector<QString> ids(plugins.size());
    std::transform(plugins.begin(), plugins.end(), ids.begin(), [](const KPluginMetaData &meta) {
        return meta.pluginId();
    });
    return ids;
}
}

namespace KDevelop {

class PluginControllerPrivate
{
public:
    explicit PluginControllerPrivate(Core *core)
        : core(core)
    {}

    QVector<KPluginMetaData> plugins;

    //map plugin infos to currently loaded plugins
    using InfoToPluginMap = QHash<KPluginMetaData, IPlugin*>;
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
        const auto interfaces = plugin.value(KEY_Interfaces(), QStringList());
        qCDebug(SHELL) << "checking dependencies:" << interfaces;
        for (auto it = loadedPlugins.constBegin(), end = loadedPlugins.constEnd(); it != end; ++it) {
            const KPluginMetaData& info = it.key();
            if (info.pluginId() != plugin.pluginId()) {
                const auto dependencies =
                    plugin.value(KEY_Required(), QStringList()) + plugin.value(KEY_Optional(), QStringList());
                for (const QString& dep : dependencies) {
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
        for (const KPluginMetaData& info : plugins) {
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
    void foreachEnabledPlugin(F func, const QString &extension = {}, const QVariantMap& constraints = QVariantMap(), const QString &pluginName = {}) const
    {
        const auto currentPlugins = plugins;
        for (const auto& info : currentPlugins) {
            if ((pluginName.isEmpty() || info.pluginId() == pluginName)
                && (extension.isEmpty() || info.value(KEY_Interfaces(), QStringList()).contains(extension))
                && constraintsMatch(info, constraints) && isEnabled(info)) {
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
        static const QStringList disabledPlugins = QString::fromLatin1(qgetenv("KDEV_DISABLE_PLUGINS")).split(QLatin1Char(';'));
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

    void initKTextEditorIntegration()
    {
        if (core->setupFlags() == Core::NoUi) {
            qCDebug(SHELL) << "Skipping KTextEditor integration in Core::NoUi mode";
            return;
        }

        KTextEditorIntegration::initialize();
        const auto ktePlugins =
            KPluginMetaData::findPlugins(QStringLiteral("kf6/ktexteditor"), [](const KPluginMetaData& data) {
#if KTEXTEDITOR_VERSION < QT_VERSION_CHECK(6, 8, 0)
                // This conditionally enabled code should become obsolete in a few months
                // with KDE Gear 24.08 and Kate changes being in distro packages:
                // https://commits.kde.org/kate/171d57820868db3eb711d328b2ef64c674793c3c

                const auto pluginData = data.rawData().value(QLatin1String("KPlugin")).toObject();
                const auto serviceTypes = pluginData.value(QLatin1String("ServiceTypes")).toArray();
                if (std::any_of(serviceTypes.begin(), serviceTypes.end(), [](const QJsonValue& value) {
                        return value.toString() == QLatin1String("KDevelop/Plugin");
                    })) {
                    return true;
                }
#endif
                return data.value(QStringLiteral("LoadInKDevelop"), false);
            });

        qCDebug(SHELL) << "Found" << ktePlugins.size() << " KTextEditor plugins:" << pluginIds(ktePlugins);

        plugins.reserve(plugins.size() + ktePlugins.size());
        for (const auto& info : ktePlugins) {
            auto data = info.rawData();
            // temporary workaround for Kate's ctags plugin being enabled by default
            // see https://mail.kde.org/pipermail/kwrite-devel/2019-July/004821.html
            if (info.pluginId() == QLatin1String("katectagsplugin")) {
                auto kpluginData = data[KEY_KPlugin()].toObject();
                kpluginData[KEY_EnabledByDefault()] = false;
                data[KEY_KPlugin()] = kpluginData;
            }
            // add some KDevelop specific JSON data
            data[KEY_Category()] = KEY_Global();
            data[KEY_Mode()] = KEY_Gui();
            data[KEY_Version()] = KDEVELOP_PLUGIN_VERSION;
            plugins.append({data, info.fileName()});
        }
    }

    Core* const core;
};

PluginController::PluginController(Core *core)
    : IPluginController()
    , d_ptr(new PluginControllerPrivate(core))
{
    Q_D(PluginController);

    setObjectName(QStringLiteral("PluginController"));

    d->plugins = KPluginMetaData::findPlugins(QStringLiteral("kdevplatform/" QT_STRINGIFY(KDEVELOP_PLUGIN_VERSION)));

    qCDebug(SHELL) << "Found" << d->plugins.size() << "plugins:" << pluginIds(d->plugins);
    if (d->plugins.empty()) {
        qCWarning(SHELL) << "Did not find any plugins, check your environment.";
        qCWarning(SHELL) << "  Note: QT_PLUGIN_PATH is set to:" << qgetenv("QT_PLUGIN_PATH");
    }

    d->initKTextEditorIntegration();

    d->cleanupMode = PluginControllerPrivate::Running;
    // Register the KDevelop::IPlugin* metatype so we can properly unload it
    qRegisterMetaType<KDevelop::IPlugin*>( "KDevelop::IPlugin*" );
}

PluginController::~PluginController()
{
    Q_D(PluginController);

    if ( d->cleanupMode != PluginControllerPrivate::CleanupDone ) {
        qCWarning(SHELL) << "Destructing plugin controller without going through the shutdown process!";
    }
}

KPluginMetaData PluginController::pluginInfo( const IPlugin* plugin ) const
{
    Q_D(const PluginController);

    return d->loadedPlugins.key(const_cast<IPlugin*>(plugin));
}

void PluginController::cleanup()
{
    Q_D(PluginController);

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
    Q_D(PluginController);

    QElapsedTimer timer;
    timer.start();

    QMap<QString, bool> pluginMap;
    if( ShellExtension::getInstance()->defaultPlugins().isEmpty() )
    {
        for (const KPluginMetaData& pi : std::as_const(d->plugins)) {
            QJsonValue enabledByDefaultValue = pi.rawData()[KEY_KPlugin()].toObject()[KEY_EnabledByDefault()];
            // plugins enabled until explicitly specified otherwise
            const bool enabledByDefault = (enabledByDefaultValue.isNull() || enabledByDefaultValue.toBool());
            pluginMap.insert(pi.pluginId(), enabledByDefault);
        }
    } else
    {
        // Get the default from the ShellExtension
        const auto defaultPlugins = ShellExtension::getInstance()->defaultPlugins();
        for (const QString& s : defaultPlugins) {
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
    for (const KPluginMetaData& pi : std::as_const(d->plugins)) {
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
    for (const KPluginMetaData& pi : std::as_const(d->plugins)) {
        if (isGlobalPlugin(pi)) {
            loadPluginInternal(pi.pluginId());
        }
    }

    qCDebug(SHELL) << "Done loading plugins - took:" << timer.elapsed() << "ms";
}

QList<IPlugin *> PluginController::loadedPlugins() const
{
    Q_D(const PluginController);

    return d->loadedPlugins.values();
}

bool PluginController::unloadPlugin( const QString & pluginId )
{
    Q_D(PluginController);

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
    Q_D(PluginController);

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
    Q_D(const PluginController);

    auto it = std::find_if(d->plugins.constBegin(), d->plugins.constEnd(), [&](const KPluginMetaData& info) {
        return (info.pluginId() == pluginId);
    });
    return (it != d->plugins.constEnd()) ? *it : KPluginMetaData();
}

IPlugin *PluginController::loadPluginInternal( const QString &pluginId )
{
    Q_D(PluginController);

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
                   << missingInterfaces.join(QLatin1Char(','));
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
    const auto factory = KPluginFactory::loadFactory(info);
    if (!factory) {
        qCWarning(SHELL) << "Can't load plugin" << pluginId
                         << "because a factory to load the plugin could not be obtained:" << factory.errorText;
        return nullptr;
    }

    // now create it
    auto plugin = factory.plugin->create<IPlugin>(d->core);
    if (!plugin) {
        if (auto katePlugin = factory.plugin->create<KTextEditor::Plugin>(d->core, QVariantList() << info.pluginId())) {
            configureKTextEditorPlugin(katePlugin);
            plugin = new KTextEditorIntegration::Plugin(katePlugin, d->core, info);
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
    qCDebug(SHELL) << "Successfully loaded plugin" << pluginId << "from" << info.fileName()
                   << "- took:" << timer.elapsed() << "ms";
    emit pluginLoaded( plugin );

    return plugin;
}

void PluginController::configureKTextEditorPlugin(KTextEditor::Plugin* plugin)
{
    auto const pluginName = plugin->metaObject()->className();
    qCDebug(SHELL) << "configuring KTextEditor plugin: " << pluginName;
    if (QLatin1String(pluginName) == QLatin1String("LSPClientPlugin"))
    {
        plugin->setProperty("disabledLanguages", DisabledLSPLanguages());
    }
}

IPlugin* PluginController::plugin(const QString& pluginId) const
{
    Q_D(const PluginController);

    KPluginMetaData info = infoForPluginId( pluginId );
    if ( !info.isValid() )
        return nullptr;

    return d->loadedPlugins.value( info );
}

bool PluginController::hasUnresolvedDependencies( const KPluginMetaData& info, QStringList& missing ) const
{
    Q_D(const PluginController);

    const auto requiredList = info.value(KEY_Required(), QStringList());
    QSet<QString> required(requiredList.begin(), requiredList.end());
    if (!required.isEmpty()) {
        d->foreachEnabledPlugin([&required](const KPluginMetaData& plugin) -> bool {
            const auto interfaces = plugin.value(KEY_Interfaces(), QStringList());
            for (const QString& iface : interfaces) {
                required.remove(iface);
                required.remove(iface + QLatin1Char('@') + plugin.pluginId());
            }
            return !required.isEmpty();
        });
    }
    // if we found all dependencies required should be empty now
    if (!required.isEmpty()) {
        missing = required.values();
        return false;
    }
    return true;
}

void PluginController::loadOptionalDependencies( const KPluginMetaData& info )
{
    const auto dependencies = info.value(KEY_Optional(), QStringList());
    for (const QString& dep : dependencies) {
        Dependency dependency(dep);
        if (!pluginForExtension(dependency.interface, dependency.pluginName)) {
            qCDebug(SHELL) << "Couldn't load optional dependency:" << dep << info.pluginId();
        }
    }
}

bool PluginController::loadDependencies( const KPluginMetaData& info, QString& failedDependency )
{
    const auto dependencies = info.value(KEY_Required(), QStringList());
    for (const QString& value : dependencies) {
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
    Q_D(PluginController);

    IPlugin* plugin = nullptr;
    d->foreachEnabledPlugin([this, &plugin] (const KPluginMetaData& info) -> bool {
        Q_D(PluginController);

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
    Q_D(PluginController);

    //qCDebug(SHELL) << "Finding all Plugins for Extension:" << extension << "|" << constraints;
    QList<IPlugin*> plugins;
    d->foreachEnabledPlugin([this, &plugins] (const KPluginMetaData& info) -> bool {
        Q_D(PluginController);

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
    Q_D(const PluginController);

    QVector<KPluginMetaData> plugins;
    d->foreachEnabledPlugin([&plugins] (const KPluginMetaData& info) -> bool {
        plugins << info;
        return true;
    }, extension, constraints);
    return plugins;
}

QStringList PluginController::allPluginNames() const
{
    Q_D(const PluginController);

    QStringList names;
    names.reserve(d->plugins.size());
    for (const KPluginMetaData& info : std::as_const(d->plugins)) {
        names << info.pluginId();
    }
    return names;
}

QList<ContextMenuExtension> PluginController::queryPluginsForContextMenuExtensions(KDevelop::Context* context, QWidget* parent) const
{
    Q_D(const PluginController);

    // This fixes random order of extension menu items between different runs of KDevelop.
    // Without sorting we have random reordering of "Analyze With" submenu for example:
    // 1) "Cppcheck" actions, "Vera++" actions - first run
    // 2) "Vera++" actions, "Cppcheck" actions - some other run.
    QMultiMap<QString, IPlugin*> sortedPlugins;
    for (auto it = d->loadedPlugins.constBegin(); it != d->loadedPlugins.constEnd(); ++it) {
        sortedPlugins.insert(it.key().name(), it.value());
    }

    QList<ContextMenuExtension> exts;
    exts.reserve(sortedPlugins.size());
    for (IPlugin* plugin : std::as_const(sortedPlugins)) {
        exts << plugin->contextMenuExtension(context, parent);
    }

    exts << Core::self()->debugControllerInternal()->contextMenuExtension(context, parent);
    exts << Core::self()->documentationControllerInternal()->contextMenuExtension(context, parent);
    exts << Core::self()->sourceFormatterControllerInternal()->contextMenuExtension(context, parent);
    exts << Core::self()->runControllerInternal()->contextMenuExtension(context, parent);
    exts << Core::self()->projectControllerInternal()->contextMenuExtension(context, parent);

    return exts;
}

QStringList PluginController::projectPlugins() const
{
    Q_D(const PluginController);

    QStringList names;
    for (const KPluginMetaData& info : std::as_const(d->plugins)) {
        if (info.value(KEY_Category()) == KEY_Project()) {
            names << info.pluginId();
        }
    }
    return names;
}

void PluginController::loadProjectPlugins()
{
    const auto pluginNames = projectPlugins();
    for (const QString& name : pluginNames) {
        loadPluginInternal( name );
    }
}

void PluginController::unloadProjectPlugins()
{
    const auto pluginNames = projectPlugins();
    for (const QString& name : pluginNames) {
        unloadPlugin( name );
    }
}

QVector<KPluginMetaData> PluginController::allPluginInfos() const
{
    Q_D(const PluginController);

    return d->plugins;
}

void PluginController::updateLoadedPlugins()
{
    Q_D(PluginController);

    QStringList defaultPlugins = ShellExtension::getInstance()->defaultPlugins();
    KConfigGroup grp = Core::self()->activeSession()->config()->group( KEY_Plugins() );
    for (const KPluginMetaData& info : std::as_const(d->plugins)) {
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
    Q_D(PluginController);

    KSharedConfigPtr cfg = Core::self()->activeSession()->config();
    cfg->deleteGroup( KEY_Plugins() );
    cfg->sync();
    KConfigGroup grp = cfg->group( KEY_Plugins() );
    QStringList plugins = ShellExtension::getInstance()->defaultPlugins();
    if( plugins.isEmpty() )
    {
        for (const KPluginMetaData& info : std::as_const(d->plugins)) {
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
    for (const QString& s : std::as_const(plugins)) {
        grp.writeEntry(s + KEY_Suffix_Enabled(), true);
    }
    grp.sync();
}

}

#include "moc_plugincontroller.cpp"
