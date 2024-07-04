/*
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <interfaces/iplugin.h>

#include <KPluginFactory>

class GlobalDefaultPlugin : public KDevelop::IPlugin
{
    Q_OBJECT
public:
    explicit GlobalDefaultPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList&);
};

GlobalDefaultPlugin::GlobalDefaultPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList&)
    : IPlugin(QStringLiteral("globaldefaultplugin"), parent, metaData)
{
}

K_PLUGIN_FACTORY_WITH_JSON(GlobalDefaultPluginFactory, "globaldefaultplugin.testpluginjson",
                           registerPlugin<GlobalDefaultPlugin>();)

#include "globaldefaultplugin.moc"
