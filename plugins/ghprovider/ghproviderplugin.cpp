/*
    SPDX-FileCopyrightText: 2012-2013 Miquel Sabaté <mikisabate@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <ghproviderplugin.h>

#include <debug.h>
#include <ghproviderwidget.h>

#include <KLocalizedString>
#include <KPluginFactory>

using namespace KDevelop;

K_PLUGIN_FACTORY_WITH_JSON(KDevGHProviderFactory, "kdevghprovider.json", registerPlugin<gh::ProviderPlugin>();)

namespace gh
{

ProviderPlugin::ProviderPlugin(QObject* parent, const KPluginMetaData& metaData, const QList<QVariant>& args)
    : IPlugin(QStringLiteral("kdevghprovider"), parent, metaData)
{
    Q_UNUSED(args);
}

ProviderPlugin::~ProviderPlugin()
{
    /* There's nothing to do here! */
}

QString ProviderPlugin::name() const
{
    return i18n("GitHub");
}

IProjectProviderWidget * ProviderPlugin::providerWidget(QWidget *parent)
{
    return new ProviderWidget(parent);
}

} // End of namespace gh

#include "ghproviderplugin.moc"
#include "moc_ghproviderplugin.cpp"
