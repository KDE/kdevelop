/*
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
