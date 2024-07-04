/*
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <interfaces/iplugin.h>

#include <KPluginFactory>

class GlobalNonDefaultPlugin : public KDevelop::IPlugin
{
    Q_OBJECT
public:
    explicit GlobalNonDefaultPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList&);
};

GlobalNonDefaultPlugin::GlobalNonDefaultPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList&)
    : IPlugin(QStringLiteral("globalnondefaultplugin"), parent, metaData)
{
}

K_PLUGIN_FACTORY_WITH_JSON(GlobalNonDefaultPluginFactory, "globalnondefaultplugin.testpluginjson",
                           registerPlugin<GlobalNonDefaultPlugin>();)

#include "globalnondefaultplugin.moc"
