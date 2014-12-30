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

#include <KLocalizedString>

TemplateConfig::TemplateConfig(QWidget* parent)
    : ConfigPage(nullptr, nullptr, parent)
{
    ui = new Ui::TemplateConfig;
    ui->setupUi(this);

    foreach (KDevelop::IPlugin* plugin, KDevelop::ICore::self()->pluginController()->allPluginsForExtension("org.kdevelop.ITemplateProvider")) {
        if (KDevelop::ITemplateProvider* provider = plugin->extension<KDevelop::ITemplateProvider>()) {
            ui->tabWidget->addTab(new TemplatePage(provider), provider->icon(), provider->name());
        }
    }
}

TemplateConfig::~TemplateConfig()
{
    delete ui;
}

QString TemplateConfig::name() const
{
    return i18n("Templates");
}

QString TemplateConfig::fullName() const
{
    return i18n("Configure Templates");
}

QIcon TemplateConfig::icon() const
{
    return QIcon::fromTheme(QStringLiteral("project-development-new-template"));
}

void TemplateConfig::apply()
{
}

void TemplateConfig::defaults()
{
}

void TemplateConfig::reset()
{
}
