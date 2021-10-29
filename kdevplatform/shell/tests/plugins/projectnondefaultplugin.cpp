/*
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <interfaces/iplugin.h>

#include <KPluginFactory>

class ProjectNonDefaultPlugin : public KDevelop::IPlugin
{
    Q_OBJECT
public:
    explicit ProjectNonDefaultPlugin(QObject* parent, const QVariantList&);
};

ProjectNonDefaultPlugin::ProjectNonDefaultPlugin(QObject* parent, const QVariantList&)
    : IPlugin(QStringLiteral("projectnondefaultplugin"), parent)
{
}

K_PLUGIN_FACTORY_WITH_JSON(ProjectNonDefaultPluginFactory, "projectnondefaultplugin.testpluginjson",
                           registerPlugin<ProjectNonDefaultPlugin>();)

#include "projectnondefaultplugin.moc"
