/* This file is part of KDevelop
 *
 * Copyright (C) 2012-2013 Miquel Sabaté <mikisabate@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#include <KLocalizedString>
#include <KAboutData>
#include <KPluginFactory>

#include <ghproviderplugin.h>
#include <ghproviderwidget.h>

using namespace KDevelop;

K_PLUGIN_FACTORY(KDevGHProviderFactory, registerPlugin<gh::ProviderPlugin>();)
// K_EXPORT_PLUGIN(KDevGHProviderFactory(KAboutData("kdevghprovider","kdevghprovider", ki18n("Github Provider"), "0.1", ki18n("Import projects from Github"), KAboutData::License_GPL)))


namespace gh
{

ProviderPlugin::ProviderPlugin(QObject *parent, const QList<QVariant> &args)
    : IPlugin("kdevghprovider", parent)
{
    Q_UNUSED(args);
    KDEV_USE_EXTENSION_INTERFACE(KDevelop::IProjectProvider)
}

ProviderPlugin::~ProviderPlugin()
{
    /* There's nothing to do here! */
}

QString ProviderPlugin::name() const
{
    return i18n("Github");
}

IProjectProviderWidget * ProviderPlugin::providerWidget(QWidget *parent)
{
    return new ProviderWidget(parent);
}

} // End of namespace gh

#include "ghproviderplugin.moc"
