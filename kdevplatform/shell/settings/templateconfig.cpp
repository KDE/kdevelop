/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-or-later
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

    const QList<KDevelop::IPlugin*> plugins = KDevelop::ICore::self()->pluginController()->allPluginsForExtension(QStringLiteral("org.kdevelop.ITemplateProvider"));
    for (KDevelop::IPlugin* plugin : plugins) {
        if (auto* provider = plugin->extension<KDevelop::ITemplateProvider>()) {
            ui->tabWidget->addTab(new TemplatePage(*provider), provider->icon(), provider->name());
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

#include "moc_templateconfig.cpp"
