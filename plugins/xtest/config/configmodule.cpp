/*
 * This file is part of KDevelop
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "configmodule.h"

#include <KPluginFactory>
#include <KPluginInfo>
#include <KPluginLoader>
#include <KDebug>
#include <QVBoxLayout>

#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include "../itestframework.h"
#include "configwidget.h"

using KDevelop::ICore;
using KDevelop::IPlugin;
using KDevelop::IPluginController;
using Veritas::ConfigModule;
using Veritas::ITestFramework;

K_PLUGIN_FACTORY(VeritasConfigFactory, registerPlugin<Veritas::ConfigModule>(); )
K_EXPORT_PLUGIN(VeritasConfigFactory("kcm_kdevveritas_config"))

ConfigModule::ConfigModule(QWidget* parent, const QVariantList& args)
    : ProjectKCModule<VeritasConfig>(VeritasConfigFactory::componentData(), parent, args)
{
    QVBoxLayout* l = new QVBoxLayout(this);
    m_widget = new ConfigWidget;
    l->addWidget(m_widget);

    ICore* core = ICore::self();
    IPluginController* pc = core->pluginController();
    QList<IPlugin*> plugs = pc->allPluginsForExtension("org.kdevelop.ITestFramework");
    foreach(IPlugin* xframe, plugs) {
        KPluginInfo i = pc->pluginInfo(xframe);
        kDebug() << i.service()->genericName();
    }

    addConfig(VeritasConfig::self(), this);
    load();
}

ConfigModule::~ConfigModule()
{
}

#include "configmodule.moc"
