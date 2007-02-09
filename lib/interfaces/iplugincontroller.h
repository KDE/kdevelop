/* This file is part of the KDE project
Copyright (C) 2004, 2007 Alexander Dymo <adymo@kdevelop.org>
Copyright (C) 2006 Matt Rogers <mattr@kde.org>
Copyright (C) 2007 Andreas Pakulat <apaku@gmx.de>

Based on code from Kopete
Copyright (c) 2002-2003 by Martijn Klingens <klingens@kde.org>

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
#ifndef IPLUGINCONTROLLER_H
#define IPLUGINCONTROLLER_H

#include <QObject>

#include <kplugininfo.h>
#include <kurl.h>
#include <kservice.h>

#include "kdevexport.h"

class QExtensionManager;

namespace KDevelop
{
class IPlugin;
class ProfileEngine;

/**
 * The KDevelop plugin controller.
 * The Plugin controller is responsible for querying, loading and unloading
 * available plugins.
 */
class KDEVPLATFORM_EXPORT IPluginController : public QObject
{

Q_OBJECT

public:
    enum PluginType {
        Global = 0,
        Project
    };

    IPluginController( QObject* parent = 0 );

    virtual ~IPluginController();

    /**
     * Get the plugin info for a loaded plugin
     */
    virtual KPluginInfo* pluginInfo( IPlugin* ) const = 0;

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
    virtual void unloadPlugin( const QString & plugin ) = 0;

    /**
     * @brief Loads the plugin specified by @p pluginname
     *
     * @param pluginName the name of the plugin, as given in the X-KDE-PluginInfo-Name property
     * @returns a pointer to the plugin instance or 0
     */
    virtual IPlugin* loadPlugin( const QString & pluginName ) = 0;

    /**
     * Queries for the plugin which supports given extension interface.
     * All already loaded plugins will be queried and the first one to support the extension interface
     * will be returned. Any plugin can be an extension, only "ServiceTypes=..." entry is
     * required in .desktop file for that plugin.
     * @param extension The extension interface
     * @return A KDevelop extension plugin for given service type or 0 if no plugin supports it, returns the first found plugin
     */
     virtual IPlugin *pluginForExtension(const QString &extension ) = 0;

     /**
     * Queries for the plugin which supports given extension interface.
     * All already loaded plugins will be queried and the first one to support the extension interface
     * will be returned. Any plugin can be an extension, only "ServiceTypes=..." entry is
     * required in .desktop file for that plugin.
     * @param extension The extension interface
     * @param pluginname The name of the plugin to load if multiple plugins for the extension exist, corresponds to the X-KDE-PluginInfo-Name
     * @return A KDevelop extension plugin for given service type or 0 if no plugin supports it
     */
    virtual IPlugin *pluginForExtension(const QString &extension, const QString& pluginname ) = 0;

    /**
     * Queries KDevelop services. Version is checked automatically
     * by adding proper X-KDevelop-Version=N statement into the query.
     * @param serviceType The service type to query, for example "KDevelop/Plugin" or
     * "KDevelop/SourceFormatter."
     * @param constraint A constraint for the service. Do not include plugin version number - it
     * is done automatically.
     * @return The list of plugin offers.
     */
    static KPluginInfo::List query( const QString &serviceType, const QString &constraint );

    /**
     * Queries KDevelop plugins. Works like KDevPluginController::query
     * with serviceType set to "KDevelop/Plugin".
     * @param constraint A constraint for the service. Do not include plugin version number - it
     * is done automatically.
     * @return The list of plugin offers.
     */
    static KPluginInfo::List queryPlugins( const QString &constraint );

    static QStringList argumentsFromService( const KService::Ptr &service );

    virtual QString currentProfile() const = 0;

    virtual void loadPlugins( PluginType offer ) = 0;
    virtual void unloadPlugins( PluginType offer ) = 0;

    virtual ProfileEngine &engine() const = 0;

    //returns the name of an old profile that was unloaded
    virtual QString changeProfile( const QString &newProfile ) = 0;

    virtual QExtensionManager* extensionManager() = 0;

Q_SIGNALS:
    void loadingPlugin( const QString& );
    void pluginLoaded( IPlugin* );

    /**
     * Emitted when a plugin profile was changed (reloaded, other profile opened, etc.).
     * Should work only on shells with plugin profiles support.
     */
    void profileChanged();

};

}
#endif

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
