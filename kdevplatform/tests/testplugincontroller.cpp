/*
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testplugincontroller.h"

#include <interfaces/contextmenuextension.h>

#include <QVariant>

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
    return nullptr;
}

QVector<KPluginMetaData> TestPluginController::queryExtensionPlugins(const QString& extension,
                                                                     const QVariantMap& constraints) const
{
    Q_UNUSED(extension);
    Q_UNUSED(constraints);
    return {};
}

IPlugin* TestPluginController::loadPlugin(const QString& pluginName)
{
    Q_UNUSED(pluginName);
    return nullptr;
}

KPluginMetaData TestPluginController::pluginInfo(const IPlugin*) const
{
    return KPluginMetaData();
}

QList<ContextMenuExtension> TestPluginController::queryPluginsForContextMenuExtensions(Context* context,
                                                                                       QWidget* parent) const
{
    Q_UNUSED(context);
    Q_UNUSED(parent);
    return QList<ContextMenuExtension>();
}

bool TestPluginController::unloadPlugin(const QString& plugin)
{
    Q_UNUSED(plugin);
    return false;
}
