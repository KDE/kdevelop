/* This file is part of the KDE project
   Copyright (C) 2004 Alexander Dymo <adymo@kdevelop.org>

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
#include <qlist.h>

#include <kurl.h>
#include "kdevexport.h"
#include <kservicetypetrader.h>

/**
@file kdevplugincontroller.h
KDevelop plugin controller interface.
*/

class KDevPlugin;

/**
The base class for KDevelop plugin controller.
Plugin controller is responsible for quering, loading and unloading available plugins.
*/
class KDEVINTERFACES_EXPORT KDevPluginController: public QObject
{
    Q_OBJECT
public:

    /**
     * Returns a uniquely specified plugin. If it isn't already loaded, it will be.
     * Use with caution! See extension for parameter details.
     */
    virtual KDevPlugin * loadPlugin( const QString & serviceType, const QString & constraint ) = 0;

    /**
     * Unloads the plugin specified by @p plugin
     * @param plugin The plugin desktopEntryName of the plugin to unload
     */
    virtual void unloadPlugin( const QString & plugin ) = 0;

    /**Unloads plugins specified by @p list.
    @param list The list of plugin names to unload. plugin name corresponds
    to the "Name" property in plugin .desktop file.*/
    virtual void unloadPlugins(QStringList const &list) = 0;

    /**@return The list of currently loaded plugins.*/
    virtual const QList<KDevPlugin*> loadedPlugins() = 0;

    /**Queries for the plugin which supports given service type.
    All already loaded plugins will be queried and the first one to support the service type
    will be returned. Any plugin can be an extension, only "ServiceTypes=..." entry is
    required in .desktop file for that plugin.
    @param serviceType The service type of an extension (like "KDevelop/SourceFormatter")
    @param constraint The constraint which is applied when quering for the service. This
    constraint is a usual KTrader constraint statement (like "[X-KDevelop-Foo]=='MyFoo'").
    @return A KDevelop extension plugin for given service type or 0 if no plugin supports it*/
    virtual KDevPlugin *extension(const QString &serviceType, const QString &constraint = "") = 0;

    /**Queries KDevelop services. Version is checked automatically
    by adding proper X-KDevelop-Version=N statement into the query.
    @param serviceType The service type to query, for example "KDevelop/Plugin" or
    "KDevelop/SourceFormatter."
    @param constraint A constraint for the service. Do not include plugin version number - it
    is done automatically.
    @return The list of plugin offers.*/
    static KService::List query(const QString &serviceType, const QString &constraint);

    /**Queries KDevelop plugins. Works like
    KDevPluginController::query with serviceType set to "KDevelop/Plugin".
    @param constraint A constraint for the service. Do not include plugin version number - it
    is done automatically.
    @return The list of plugin offers.*/
    static KService::List queryPlugins(const QString &constraint);

    /**Reimplement this function only if your shell supports plugin profiles.
    @return The list of URLs to the profile resources (files) with given @p extension.
    @param nameFilter Name filter for files. @see QDir::setNameFilter documentation
    for name filters syntax.*/
    virtual KUrl::List profileResources(const QString &nameFilter);

    /**Reimplement this function only if your shell supports plugin profiles.
    @return The list of URLs to the resources (files) with given @p extension.
    This list is obtained by a recursive search that process given profile
    and all it's subprofiles.
    @param nameFilter Name filter for files. @see QDir::setNameFilter documentation
    for name filters syntax.*/
    virtual KUrl::List profileResourcesRecursive(const QString &nameFilter);

signals:
    /**Emitted when a plugin profile was changed (reloaded, other profile opened, etc.).
    Should work only on shells with plugin profiles support.*/
    void profileChanged();

protected:
    /**Constructor.*/
    KDevPluginController();

};

#endif
