/*
    SPDX-FileCopyrightText: 2004, 2007 Alexander Dymo <adymo@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Matt Rogers <mattr@kde.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    Based on code from Kopete
    SPDX-FileCopyrightText: 2002-2003 Martijn Klingens <klingens@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_IPLUGINCONTROLLER_H
#define KDEVPLATFORM_IPLUGINCONTROLLER_H

#include <QObject>
#include <QList>
#include <QVector>
#include <QVariant>

#include <KPluginMetaData>

#include "iplugin.h"
#include "interfacesexport.h"

class QExtensionManager;

namespace KDevelop
{
class ProfileEngine;

/**
 * The KDevelop plugin controller.
 * The Plugin controller is responsible for querying, loading and unloading
 * available plugins.
 *
 * Most of the time if you want to get at a plugin you should be using
 * extensionForPlugin with the extension interface name. If you need to get at
 * the actual \c IPlugin* pointer to connect signals/slots you should use
 * \c pluginForExtension() and then the IPlugin's extension member function to get
 * at the extension interface if necessary.
 *
 * If you have the need to load a specific plugin for a given extension both
 * functions have an optional second parameter that allows one to specify the
 * name of the plugin as declared in the \c .desktop file under the
 * \c X-KDE-PluginInfo-Name property. This should be used only very seldomly in
 * real code and is mostly meant for testing and for implementation in the
 * shell as it makes the code dependent on the plugin name which may change and
 * also the actual plugin implementation so users cannot exchange one plugin
 * with another also implementing the same interface.
 *
 */
class KDEVPLATFORMINTERFACES_EXPORT IPluginController : public QObject
{

Q_OBJECT

public:
    /**
     * \brief Indicates the plugin type
     * This is used to determine how the plugin is loaded
     */
    enum PluginType {
        Global = 0, ///< Indicates that the plugin is loaded at startup
        Project ///< Indicates that the plugin is loaded with the first opened project
    };

    explicit IPluginController( QObject* parent = nullptr );

    ~IPluginController() override;

    /**
     * Get the plugin info for a loaded plugin
     */
    virtual KPluginMetaData pluginInfo(const IPlugin*) const = 0;

    /**
     * Find the KPluginMetaData structure for the given @p pluginId.
     */
    virtual KPluginMetaData infoForPluginId(const QString &pluginId) const = 0;

    /**
     * Get a list of currently loaded plugins
     */
    virtual QList<IPlugin*> loadedPlugins() const = 0;

    /**
     * @brief Unloads the plugin specified by @p plugin
     *
     * @param plugin The name of the plugin as specified by the
     * X-KDE-PluginInfo-Name key of the .desktop file for the plugin
     */
    virtual bool unloadPlugin( const QString & plugin ) = 0;

    /**
     * @brief Loads the plugin specified by @p pluginName
     *
     * @param pluginName the name of the plugin, as given in the X-KDE-PluginInfo-Name property
     * @returns a pointer to the plugin instance or 0
     */
    virtual IPlugin* loadPlugin( const QString & pluginName ) = 0;

     /**
     * Retrieve a plugin which supports the given extension interface.
     *
     * All already loaded plugins will be queried and the first one to support the extension interface
     * will be returned. Any plugin can be an extension, only the "ServiceTypes=..." entry is
     * required in .desktop file for that plugin.
     *
     * If no already-loaded plugin was found, we try to load a plugin for the given extension.
     *
     * If no plugin was found, a nullptr will be returned.
     *
     * @param extension The extension interface. Can be empty if you want to find a plugin by name or other constraint.
     * @param pluginName The name of the plugin to load if multiple plugins for the extension exist, corresponds to the X-KDE-PluginInfo-Name
     * @param constraints A map of constraints on other plugin info properties.
     * @return A KDevelop extension plugin for given service type or 0 if no plugin supports it
     */
    virtual IPlugin *pluginForExtension(const QString &extension, const QString& pluginName = {}, const QVariantMap& constraints = QVariantMap()) = 0;

     /**
     * Retrieve a list of plugins which supports the given extension interface.
     * All already loaded plugins will be queried and the first one to support the extension interface
     * will be returned. Any plugin can be an extension, only the "ServiceTypes=..." entry is
     * required in .desktop file for that plugin.
     * @param extension The extension interface
     * @param constraints A map of constraints on other plugin info properties.
     * @return A KDevelop extension plugin for given service type or 0 if no plugin supports it
     */
    virtual QList<IPlugin*> allPluginsForExtension(const QString &extension, const QVariantMap& constraints = QVariantMap()) = 0;

     /**
     * Retrieve the plugin which supports given extension interface and
     * returns a pointer to the extension interface.
     *
     * All already loaded plugins will be queried and the first one to support the extension interface
     * will be returned. Any plugin can be an extension, only "ServiceTypes=..." entry is
     * required in .desktop file for that plugin.
     * @param extension The extension interface
     * @param pluginName The name of the plugin to load if multiple plugins for the extension exist, corresponds to the X-KDE-PluginInfo-Name
     * @return Pointer to the extension interface or 0 if no plugin supports it
      */
    template<class Extension> Extension* extensionForPlugin( const QString &extension = {}, const QString &pluginName = {})
    {
        QString ext;
        if( extension.isEmpty() ) {
            ext = QString::fromUtf8(qobject_interface_iid<Extension*>());
        } else {
            ext = extension;
        }
        IPlugin *plugin = pluginForExtension(ext, pluginName);
        if (plugin) {
            return plugin->extension<Extension>();
        }
        return nullptr;
    }

    /**
     * Query for plugin information on KDevelop plugins implementing the given extension.
     *
     * The service version is checked for automatically and the only serviceType
     * searched for is "KDevelop/Plugin"
     *
     * @param extension The extension that should be implemented by the plugin, i.e. listed in X-KDevelop-Interfaces.
     * @param constraints A map of constraints on other plugin info properties.
     * @return The list of plugin offers.
     */
    virtual QVector<KPluginMetaData> queryExtensionPlugins(const QString &extension, const QVariantMap& constraints = QVariantMap()) const = 0;

    virtual QList<ContextMenuExtension> queryPluginsForContextMenuExtensions(KDevelop::Context* context, QWidget* parent) const = 0;

Q_SIGNALS:
    void loadingPlugin( const QString& );
    void pluginLoaded( KDevelop::IPlugin* );
    void unloadingPlugin( KDevelop::IPlugin* );
    /**
     * This signal is emitted whenever a plugin is unloaded.
     * @note: that you shouldn't use the pointer anymore
     * except for comparing it against against other pointers. The plugin instance can already have been completely
     * deleted when this signal is emitted.
     */
    void pluginUnloaded( KDevelop::IPlugin* );

private:
    friend class IPlugin;
};

}
#endif

