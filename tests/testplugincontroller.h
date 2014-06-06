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

#include "kdevplatformtestsexport.h"

#include <shell/plugincontroller.h>
#include <interfaces/contextmenuextension.h>

namespace KDevelop {

/**
 * Dummy controller that disables plugins.
 */
class KDEVPLATFORMTESTS_EXPORT TestPluginController : public PluginController
{
    Q_OBJECT

public:
    TestPluginController(KDevelop::Core* core);
    virtual QList< KDevelop::IPlugin* > allPluginsForExtension(const QString& extension,
                                                               const QVariantMap& constraints = {}) override;
    virtual QList< KDevelop::IPlugin* > loadedPlugins() const;
    virtual KDevelop::IPlugin* pluginForExtension(const QString& extension, const QString& pluginName = {}, const QVariantMap& constraints = {}) override;
    virtual KDevelop::IPlugin* loadPlugin(const QString& pluginName);
    virtual KPluginInfo pluginInfo(const KDevelop::IPlugin* ) const;
    virtual QList< KDevelop::ContextMenuExtension > queryPluginsForContextMenuExtensions(KDevelop::Context* context)const ;
    virtual KPluginInfo::List queryExtensionPlugins(const QString& extension, const QVariantMap& constraints = {}) const override;
    virtual bool unloadPlugin(const QString& plugin);
    virtual void initialize();
};

}

#endif // KDEVPLATFORM_TESTPLUGINCONTROLLER_H
