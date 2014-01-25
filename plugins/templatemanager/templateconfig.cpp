/*
 * This file is part of KDevelop
 * Copyright 2012 Miha Čančula <miha@noughmad.eu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
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

#include "templateconfig.h"
#include "templatepage.h"
#include "ui_templateconfig.h"

#include <interfaces/iplugin.h>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/itemplateprovider.h>

#include <KPluginFactory>
#include <KCoreAddons/KAboutData>
#include <KLocalizedString>

K_PLUGIN_FACTORY(TemplateConfigFactory, registerPlugin<TemplateConfig>();)
// K_EXPORT_PLUGIN(TemplateConfigFactory("kdevtemplates_config"))

TemplateConfig::TemplateConfig(QWidget* parent, const QVariantList& args)
    : KCModule(new KAboutData("kdevtemplates_config", "kdevplatform_templates", i18n("Template Provider")
        , QString()), parent, args)
{
    ui = new Ui::TemplateConfig;
    ui->setupUi(this);

    foreach (KDevelop::IPlugin* plugin, KDevelop::ICore::self()->pluginController()->allPluginsForExtension("org.kdevelop.ITemplateProvider"))
    {
        if (KDevelop::ITemplateProvider* provider = plugin->extension<KDevelop::ITemplateProvider>())
        {
            ui->ktabwidget->addTab(new TemplatePage(provider), provider->icon(), provider->name());
        }
    }
}

TemplateConfig::~TemplateConfig()
{
    delete ui;
}

#include "templateconfig.moc"
