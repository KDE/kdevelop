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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef KDEVPLUGINCONTROLLER_H
#define KDEVPLUGINCONTROLLER_H

#include <ktrader.h>

/**
@file kdevplugincontroller.h
KDevelop plugin controller interface.
*/

class KDevPlugin;

/**
The base class for KDevelop plugin controller.
Plugin controller is responsible for quering, loading and unloading available plugins.
*/
class KDevPluginController
{
public:
    
    /**Unloads plugins specified by @p list.
    @param list The list of plugin names to unload. plugin name corresponds
    to the "Name" property in plugin .desktop file.*/
    virtual void unloadPlugins(QStringList const &list) = 0;
    
    /**@return The list of currently loaded plugins.*/
    virtual const QValueList<KDevPlugin*> loadedPlugins() = 0;
    
    /**Queries for the plugin which supports given service type.
    All already loaded plugins will be queried and the first one to support the service type
    will be returned. Any plugin can be an extension, only "ServiceTypes=..." entry is
    required in .desktop file for that plugin.
    @param serviceType a service type of an extension (like "KDevelop/SourceFormatter")
    @return a KDevelop extension plugin for given service type or 0 if no plugin supports it*/
    virtual KDevPlugin *extension(const QString &serviceType, const QString &constraint = "") = 0;
    
    /**Queries KDevelop services. Version is checked automatically
    by adding proper X-KDevelop-Version=N statement into the query.
    @param serviceType The service type to query, for example "KDevelop/Plugin" or
    "KDevelop/SourceFormatter."
    @param constraint A constraint for the service. Do not include plugin version number - it
    is done automatically.
    @return The list of plugin offers.*/
    static KTrader::OfferList query(const QString &serviceType, const QString &constraint);
    
    /**Queries KDevelop plugins. Works like
    KDevPluginController::query with serviceType set to "KDevelop/Plugin".
    @param constraint A constraint for the service. Do not include plugin version number - it
    is done automatically.
    @return The list of plugin offers.*/
    static KTrader::OfferList queryPlugins(const QString &constraint);
    
protected:
    /**Constructor.*/
    KDevPluginController();

};

#endif
