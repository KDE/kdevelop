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

#ifndef KDEVPLATFORM_TESTPLUGINCONTROLLER_H
#define KDEVPLATFORM_TESTPLUGINCONTROLLER_H

#include "testsexport.h"

#include <shell/plugincontroller.h>

namespace KDevelop {
/**
 * Dummy controller that disables plugins.
 */
class KDEVPLATFORMTESTS_EXPORT TestPluginController
    : public PluginController
{
    Q_OBJECT

public:
    explicit TestPluginController(KDevelop::Core* core);
    QList<KDevelop::IPlugin*> allPluginsForExtension(const QString& extension,
                                                     const QVariantMap& constraints = QVariantMap()) override;
    QList<KDevelop::IPlugin*> loadedPlugins() const override;
    KDevelop::IPlugin* pluginForExtension(const QString& extension, const QString& pluginName = {},
                                          const QVariantMap& constraints = QVariantMap()) override;
    KDevelop::IPlugin* loadPlugin(const QString& pluginName) override;
    KPluginMetaData pluginInfo(const KDevelop::IPlugin*) const override;
    QList<KDevelop::ContextMenuExtension> queryPluginsForContextMenuExtensions(KDevelop::Context* context,
                                                                               QWidget* parent) const override;
    QVector<KPluginMetaData> queryExtensionPlugins(const QString& extension,
                                                   const QVariantMap& constraints = QVariantMap()) const override;
    bool unloadPlugin(const QString& plugin) override;
    void initialize() override;
};
}

#endif // KDEVPLATFORM_TESTPLUGINCONTROLLER_H
