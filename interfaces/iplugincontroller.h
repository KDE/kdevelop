/* This file is part of the KDE project
Copyright 2004, 2007 Alexander Dymo <adymo@kdevelop.org>
Copyright 2006 Matt Rogers <mattr@kde.org>
Copyright 2007 Andreas Pakulat <apaku@gmx.de>

Based on code from Kopete
Copyright 2002-2003 Martijn Klingens <klingens@kde.org>

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
#ifndef KDEVPLATFORM_IPLUGINCONTROLLER_H
#define KDEVPLATFORM_IPLUGINCONTROLLER_H

#include <QtCore/QObject>
#include <QtCore/QList>

#include <kplugininfo.h>
#include <kurl.h>
#include <kservice.h>

#include "iplugin.h"
#include "interfacesexport.h"

class QExtensionManager;
class KMenu;

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
 * functions have an optional second parameter that allows to specify the name
 * of the plugin as declared in the \c .desktop file under the
 * \c X-KDE-PluginInfo-Name property. This should be used only very seldomly in
 * real code and is mostly meant for testing and for implementation in the
 * shell as it makes the code dependent on the pluginname which may change and
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

    IPluginController( QObject* parent = 0 );

    virtual ~IPluginController();

    /**
     * Get the plugin info for a loaded plugin
     */
    virtual KPluginInfo pluginInfo( const IPlugin* ) const = 0;

    /**
     * Get a list of currently loaded plugins
     */
    Q_SCRIPTABLE virtual QList<IPlugin*> loadedPlugins() const = 0;

    /**
     * @brief Unloads the plugin specified by @p plugin
     *
     * @param plugin The name of the plugin as specified by the
     * X-KDE-PluginInfo-Name key of the .desktop file for the plugin
     */
    Q_SCRIPTABLE virtual bool unloadPlugin( const QString & plugin ) = 0;

    /**
     * @brief Loads the plugin specified by @p pluginname
     *
     * @param pluginName the name of the plugin, as given in the X-KDE-PluginInfo-Name property
     * @returns a pointer to the plugin instance or 0
     */
    Q_SCRIPTABLE virtual IPlugin* loadPlugin( const QString & pluginName ) = 0;

     /**
     * Retrieve a plugin which supports the given extension interface.
     * All already loaded plugins will be queried and the first one to support the extension interface
     * will be returned. Any plugin can be an extension, only the "ServiceTypes=..." entry is
     * required in .desktop file for that plugin.
     * @param extension The extension interface
     * @param pluginname The name of the plugin to load if multiple plugins for the extension exist, corresponds to the X-KDE-PluginInfo-Name
     * @return A KDevelop extension plugin for given service type or 0 if no plugin supports it
     */
    Q_SCRIPTABLE virtual IPlugin *pluginForExtension(const QString &extension, const QString& pluginname = "" ) = 0;

     /**
     * Retrieve a list of plugins which supports the given extension interface.
     * All already loaded plugins will be queried and the first one to support the extension interface
     * will be returned. Any plugin can be an extension, only the "ServiceTypes=..." entry is
     * required in .desktop file for that plugin.
     * @param extension The extension interface
     * @return A KDevelop extension plugin for given service type or 0 if no plugin supports it
     */
    virtual QList<IPlugin*> allPluginsForExtension(const QString &extension, const QStringList &constraints = QStringList()) = 0;

     /**
     * Retrieve the plugin which supports given extension interface and
     * returns a pointer to the extension interface.
     *
     * All already loaded plugins will be queried and the first one to support the extension interface
     * will be returned. Any plugin can be an extension, only "ServiceTypes=..." entry is
     * required in .desktop file for that plugin.
     * @param extension The extension interface
     * @param pluginname The name of the plugin to load if multiple plugins for the extension exist, corresponds to the X-KDE-PluginInfo-Name
     * @return Pointer to the extension interface or 0 if no plugin supports it
      */
    template<class Extension> Extension* extensionForPlugin( const QString &extension = "", const QString &pluginname = "") {
        QString ext;
        if( extension.isEmpty() ) {
            ext = qobject_interface_iid<Extension*>();
        } else {
            ext = extension;
        }
        IPlugin *plugin = pluginForExtension(ext, pluginname);
        if (plugin) {
            return plugin->extension<Extension>();
        }
        return 0L;
    }


    /**
     * Query for a KDevelop service.
     *
     * The service version is checked for automatically
     * @param serviceType The service type to query for. Examples include:
     * "KDevelop/Plugin" or "KDevelop/SourceFormatter."
     * @param constraint A constraint for the service. Do not include plugin
     * version number - it is done automatically.
     * @return The list of plugin offers.
     */
    static KPluginInfo::List query( const QString &serviceType, const QString &constraint );

    /**
     * Query for a KDevelop plugin.
     *
     * The service version is checked for automatically and the only serviceType
     * searched for is "KDevelop/Plugin"
     * @param constraint A constraint for the service. Do not include plugin
     * version number - it is done automatically.
     * @return The list of plugin offers.
     */
    static KPluginInfo::List queryPlugins( const QString &constraint );

    static QStringList argumentsFromService( const KService::Ptr &service );

    static KPluginInfo::List queryExtensionPlugins(const QString &extension, const QStringList &constraints = QStringList());

    virtual QList<ContextMenuExtension> queryPluginsForContextMenuExtensions( KDevelop::Context* context ) const = 0;

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

