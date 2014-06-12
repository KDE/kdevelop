/*
 * This file is part of KDevelop
 *
 * Copyright 2012 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "testplugincontroller.h"

using namespace KDevelop;

TestPluginController::TestPluginController(Core* core) : PluginController(core)
{

}

void TestPluginController::initialize()
{

}

QList<IPlugin*> TestPluginController::allPluginsForExtension(const QString& extension,
                                                             const QVariantMap& constraints)
{
    Q_UNUSED(extension);
    Q_UNUSED(constraints);
    return QList<IPlugin*>();
}

QList<IPlugin*> TestPluginController::loadedPlugins() const
{
    return QList<IPlugin*>();
}

IPlugin* TestPluginController::pluginForExtension(const QString& extension, const QString& pluginname,
                                                  const QVariantMap& constraints)
{
    Q_UNUSED(extension);
    Q_UNUSED(pluginname);
    Q_UNUSED(constraints);
    return 0;
}

KPluginInfo::List TestPluginController::queryExtensionPlugins(const QString& extension, const QVariantMap& constraints) const
{
    Q_UNUSED(extension);
    Q_UNUSED(constraints);
    return {};
}

IPlugin* TestPluginController::loadPlugin(const QString& pluginName)
{
    Q_UNUSED(pluginName);
    return 0;
}

KPluginInfo TestPluginController::pluginInfo(const IPlugin*) const
{
    return KPluginInfo();
}

QList< ContextMenuExtension > TestPluginController::queryPluginsForContextMenuExtensions(Context* context) const
{
    Q_UNUSED(context);
    return QList< ContextMenuExtension >();
}

bool TestPluginController::unloadPlugin(const QString& plugin)
{
    Q_UNUSED(plugin);
    return false;
}
