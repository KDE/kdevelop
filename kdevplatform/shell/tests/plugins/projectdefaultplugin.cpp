/*
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <interfaces/iplugin.h>

#include <KPluginFactory>

class ProjectDefaultPlugin : public KDevelop::IPlugin
{
    Q_OBJECT
public:
    explicit ProjectDefaultPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList&);
};

ProjectDefaultPlugin::ProjectDefaultPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList&)
    : IPlugin(QStringLiteral("projectdefaultplugin"), parent, metaData)
{
}

K_PLUGIN_FACTORY_WITH_JSON(ProjectDefaultPluginFactory, "projectdefaultplugin.testpluginjson",
                           registerPlugin<ProjectDefaultPlugin>();)

#include "projectdefaultplugin.moc"
