/* This file is part of the KDE project
Copyright (C) 2004 Alexander Dymo <adymo@kdevelop.org>
Copyright     2006 Matt Rogers <mattr@kde.org

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
#ifndef KDEVPLUGINCONTROLLER_H
#define KDEVPLUGINCONTROLLER_H

#include <QObject>
#include "kdevcore.h"

#include <QHash>
#include <QList>

#include <kurl.h>
#include <kservice.h>
#include <kplugininfo.h>
#include <kservicetypetrader.h>

#include "kdevexport.h"

class KDialog;
class KXMLGUIClient;
class KDevPlugin;
class ProjectInfo;
class ProfileEngine;

/**
@file kdevplugincontroller.h
KDevelop plugin controller interface.
*/

/**
 *The KDevelop plugin controller.
* The Plugin controller is responsible for querying, loading and unloading available plugins.
*/
class KDEVPLATFORM_EXPORT KDevPluginController: public QObject
{
    
    Q_OBJECT

public:
    enum PluginType {
        Global = 0,
        Project
    };

    static KDevPluginController* self();
    virtual ~KDevPluginController();

    /**
     * Get the plugin instance based on the ID. The ID should be whatever is
     * in X-KDE-PluginInfo-Name
     */
    KDevPlugin* plugin( const QString& );

    /**
     * Get the plugin info for a loaded plugin
     */
    KPluginInfo* pluginInfo( KDevPlugin* ) const;

    /**
     * Get a list of currently loaded plugins
     */
    QList<KDevPlugin*> loadedPlugins() const;

    /**
     * Returns a uniquely specified plugin. If it isn't already loaded, it will be.
     */
    KDevPlugin * loadPlugin( const QString & _pluginId );

    /**
     * @brief Unloads the plugin specified by @p plugin
     * 
     * @param plugin The name of the plugin as specified by the 
     * X-KDE-PluginInfo-Name key of the .desktop file for the plugin
     */
    void unloadPlugin( const QString & plugin );

    /**
     * Queries for the plugin which supports given service type.
     * All already loaded plugins will be queried and the first one to support the service type
     * will be returned. Any plugin can be an extension, only "ServiceTypes=..." entry is
     * required in .desktop file for that plugin.
     * @param serviceType The service type of an extension (like "KDevelop/SourceFormatter")
     * @param constraint The constraint which is applied when quering for the service. This
     * constraint is a usual KTrader constraint statement (like "[X-KDevelop-Foo]=='MyFoo'").
     * @return A KDevelop extension plugin for given service type or 0 if no plugin supports it
     */
    template <class Extension>
    Extension *extension(const QString &serviceType, const QString &constraint = QString() )
    {
        return static_cast<Extension*>(getExtension( serviceType, constraint ) );
    }

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

    /**
     * Reimplement this function only if your shell supports plugin profiles.
     * @return The list of URLs to the profile resources (files) with given @p extension.
     * @param nameFilter Name filter for files. @see QDir::setNameFilter documentation
     * for name filters syntax.
     */
    KUrl::List profileResources( const QString &nameFilter );

    /**
     * Reimplement this function only if your shell supports plugin profiles.
     * @return The list of URLs to the resources (files) with given @p extension.
     * This list is obtained by a recursive search that process given profile
     * and all it's subprofiles.
     * @param nameFilter Name filter for files. @see QDir::setNameFilter documentation
     * for name filters syntax.
     */
    KUrl::List profileResourcesRecursive( const QString &nameFilter );

    static QStringList argumentsFromService( const KService::Ptr &service );

    QString currentProfile() const;

    void loadPlugins( PluginType offer );
    void unloadPlugins( PluginType offer );

    ProfileEngine &engine() const;

    //returns the name of an old profile that was unloaded
    QString changeProfile( const QString &newProfile );
    
    void shutdown();

Q_SIGNALS:
    void loadingPlugin( const QString& );
    void pluginLoaded( KDevPlugin* );

    /**
     * Emitted when a plugin profile was changed (reloaded, other profile opened, etc.).
     * Should work only on shells with plugin profiles support.
     */
    void profileChanged();

private Q_SLOTS:
    ///A plugin has been destroyed. Cleanup our data structures
    void pluginDestroyed( QObject* );

    ///A plugin is ready to unload. Unload it
    void pluginReadyForUnload( KDevPlugin* );

    ///Our timeout timer has expired
    void shutdownTimeout();

    void shutdownDone();

private:

    /**
     * @internal
     *
     * The internal method for loading plugins.
     * Called by @ref loadPlugin directly or through the queue for async plugin
     * loading.
     */
    KDevPlugin* loadPluginInternal( const QString &pluginId );

    /**
     * @internal
     *
     * Find the KPluginInfo structure by key. Reduces some code duplication.
     *
     * Returns a null pointer when no plugin info is found.
     */
    KPluginInfo * infoForPluginId( const QString &pluginId ) const;

    /**
     * @internal
     * 
     * Used for the extension template function
     */
    KDevPlugin* getExtension( const QString&, const QString& );

private:
    class Private;
    Private* d;

    KDevPluginController();
    static KDevPluginController *s_self;
};

#endif

//kate: auto-insert-doxygen on;
