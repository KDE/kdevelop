/* This file is part of the KDE project
Copyright (C) 2007 Andreas Pakulat <apaku@gmx.de>
Copyright (C) 2004, 2007 Alexander Dymo <adymo@kdevelop.org>
Copyright (C) 2006 Matt Rogers <mattr@kde.org

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

#include "iplugincontroller.h"

#include <QHash>
#include <QList>

#include <kurl.h>
#include <kservice.h>
#include <kplugininfo.h>
#include <kservicetypetrader.h>

#include "kdevexport.h"

class KDialog;
class KXMLGUIClient;

namespace Koncrete
{
class Core;
class IPlugin;
class ProjectInfo;
class ProfileEngine;
class PluginControllerPrivate;
/**
 * The KDevelop plugin controller.
 * The Plugin controller is responsible for querying, loading and unloading
 * available plugins.
 */
class KDEVPLATFORM_EXPORT PluginController: public IPluginController
{

    Q_OBJECT
friend class Core;

public:

    PluginController(Core *core);

    virtual ~PluginController();

    /**
     * Get the plugin instance based on the ID. The ID should be whatever is
     * in X-KDE-PluginInfo-Name
     */
    IPlugin* plugin( const QString& );

    /**
     * Get the plugin info for a loaded plugin
     */
    KPluginInfo* pluginInfo( IPlugin* ) const;

    /**
     * Get a list of currently loaded plugins
     */
    QList<IPlugin*> loadedPlugins() const;

    /**
     * Returns a uniquely specified plugin. If it isn't already loaded, it will be.
     * @param pluginName the name of the plugin, as given in the X-KDE-PluginInfo-Name property
     * @returns a pointer to the plugin instance or 0
     */
    IPlugin * loadPlugin( const QString & pluginName );

    /**
     * @brief Unloads the plugin specified by @p plugin
     *
     * @param plugin The name of the plugin as specified by the
     * X-KDE-PluginInfo-Name key of the .desktop file for the plugin
     */
    void unloadPlugin( const QString & plugin );

    /**
     * Queries for the plugin which supports given extension interface.
     * All already loaded plugins will be queried and the first one to support the extension interface
     * will be returned. Any plugin can be an extension, only "ServiceTypes=..." entry is
     * required in .desktop file for that plugin.
     * @param extension The extension interface
     * @param pluginname The name of the plugin to load if multiple plugins for the extension exist, corresponds to the X-KDE-PluginInfo-Name
     * @return A KDevelop extension plugin for given service type or 0 if no plugin supports it
     */
    IPlugin *pluginForExtension(const QString &extension );
    IPlugin *pluginForExtension(const QString &extension, const QString& pluginname );

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

    QString currentProfile() const;

    void loadPlugins( PluginType offer );
    void unloadPlugins( PluginType offer );

    ProfileEngine &engine() const;

    //returns the name of an old profile that was unloaded
    QString changeProfile( const QString &newProfile );

    QExtensionManager* extensionManager();

private Q_SLOTS:
    ///A plugin has been destroyed. Cleanup our data structures
    void pluginDestroyed( QObject* );

    ///A plugin is ready to unload. Unload it
    void pluginReadyForUnload( IPlugin* );

    ///Our timeout timer has expired
    void cleanupTimeout();

    void cleanupDone();

private:

    /**
     * @internal
     *
     * The internal method for loading plugins.
     * Called by @ref loadPlugin directly or through the queue for async plugin
     * loading.
     */
    IPlugin* loadPluginInternal( const QString &pluginId );

    /**
     * @internal
     *
     * Find the KPluginInfo structure by key. Reduces some code duplication.
     *
     * Returns a null pointer when no plugin info is found.
     */
    KPluginInfo * infoForPluginId( const QString &pluginId ) const;

    bool checkForDependecies( KPluginInfo* info, QStringList& missing ) const;

    void loadDependecies( KPluginInfo* );

    void cleanup();

private:
    struct PluginControllerPrivate* const d;
    QExtensionManager* m_manager;
};

}
#endif


// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
