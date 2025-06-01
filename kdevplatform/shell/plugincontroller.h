/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2004, 2007 Alexander Dymo <adymo@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Matt Rogers <mattr@kde.org

    Based on code from Kopete
    SPDX-FileCopyrightText: 2002-2003 Martijn Klingens <klingens@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGINCONTROLLER_H
#define KDEVPLATFORM_PLUGINCONTROLLER_H

#include <interfaces/iplugincontroller.h>

#include "shellexport.h"


namespace KDevelop
{
class Core;
class CorePrivate;
class IPlugin;
class PluginControllerPrivate;
/**
 * The KDevelop plugin controller.
 * The Plugin controller is responsible for querying, loading and unloading
 * available plugins.
 */
class KDEVPLATFORMSHELL_EXPORT PluginController: public IPluginController
{
    Q_OBJECT
friend class Core;
friend class CorePrivate;

public:

    explicit PluginController(Core *core);

    ~PluginController() override;

    /**
     * Get the plugin instance based on the ID. The ID should be whatever is
     * in X-KDE-PluginInfo-Name
     */
    IPlugin* plugin(const QString& pluginId) const;

    /**
     * Get the plugin info for a loaded plugin
     */
    KPluginMetaData pluginInfo( const IPlugin* ) const override;

    /**
     * Find the KPluginMetaData structure for the given @p pluginId.
     */
    KPluginMetaData infoForPluginId(const QString &pluginId) const override;

    /**
     * Get a list of currently loaded plugins
     */
    QList<IPlugin*> loadedPlugins() const override;

    /**
     * Returns a uniquely specified plugin. If it isn't already loaded, it will be.
     * @param pluginName the name of the plugin, as given in the X-KDE-PluginInfo-Name property
     * @returns a pointer to the plugin instance or 0
     */
    IPlugin * loadPlugin( const QString & pluginName ) override;

    /**
     * @brief Unloads the plugin specified by @p plugin
     *
     * @param plugin The name of the plugin as specified by the
     * X-KDE-PluginInfo-Name key of the .desktop file for the plugin
     */
    bool unloadPlugin( const QString & plugin ) override;

    enum PluginDeletion {
        Now,
        Later
    };

    /**
     * retrieve all plugin infos
     */
    QVector<KPluginMetaData> allPluginInfos() const;

    /**
     * loads not-yet-loaded plugins and unloads plugins
     * depending on the configuration in the session\
     */
    void updateLoadedPlugins();


    /**
     * Queries for the plugin which supports given extension interface.
     *
     * All already loaded plugins will be queried and the first one to support the extension interface
     * will be returned. Any plugin can be an extension, only "ServiceTypes=..." entry is
     * required in .desktop file for that plugin.
     *
     * @param extension The extension interface
     * @param pluginName The name of the plugin to load if multiple plugins for the extension exist, corresponds to the X-KDE-PluginInfo-Name
     * @return A KDevelop extension plugin for given service type or 0 if no plugin supports it
     */
    IPlugin *pluginForExtension(const QString &extension, const QString &pluginName = {}, const QVariantMap& constraints = QVariantMap()) override;

    QList<IPlugin*> allPluginsForExtension(const QString &extension, const QVariantMap& constraints = QVariantMap()) override;

    QStringList allPluginNames() const;

    QVector<KPluginMetaData> queryExtensionPlugins(const QString& extension, const QVariantMap& constraints = QVariantMap()) const override;

    QList<ContextMenuExtension> queryPluginsForContextMenuExtensions(KDevelop::Context* context, QWidget* parent) const override;

    QStringList projectPlugins() const;

    void loadProjectPlugins();
    void unloadProjectPlugins();

    void resetToDefaults();

private:
    /**
     * Directly unload the given \a plugin, either deleting it now or \a deletion.
     *
     * \param plugin plugin to unload
     * \param deletion if true, delete the plugin later, if false, delete it now.
     */
    bool unloadPlugin(IPlugin* plugin, PluginDeletion deletion);


    /**
     * @internal
     *
     * The internal method for loading plugins.
     * Called by @ref loadPlugin directly or through the queue for async plugin
     * loading.
     */
    IPlugin* loadPluginInternal( const QString &pluginId );

    /**
     * Check whether the plugin identified by @p info has unresolved dependencies.
     *
     * Assume a plugin depends on the interfaces Foo and Bar. Then, all available enabled
     * plugins are queried to check whether any fulfills the interfaces. If any of the
     * interfaces is not found, then it is inserted into the returned list. Otherwise,
     * the returned list is empty, which indicates that all dependencies can be fulfilled.
     *
     * @return the list of unresolved dependencies
     */
    [[nodiscard]] QStringList unresolvedDependencies(const KPluginMetaData& info) const;

    bool loadDependencies(const KPluginMetaData&, QString& failedPlugin);
    void loadOptionalDependencies(const KPluginMetaData& info);

    void cleanup();
    virtual void initialize();

private:
    const QScopedPointer<class PluginControllerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(PluginController)
};

}
#endif


