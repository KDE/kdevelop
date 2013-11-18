/* This file is part of KDevelop
  Copyright 2013 Christoph Thielecke <crissi99@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, o (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <KDebug>
#include <KPluginFactory>

#include "genericconfigpage.h"
#include "plugin.h"

#include "ui_genericconfig.h"

K_PLUGIN_FACTORY(CppcheckPreferencesFactory, registerPlugin<cppcheck::GenericConfigPage>();)
K_EXPORT_PLUGIN(CppcheckPreferencesFactory("kcm_kdev_cppcheck"));


namespace cppcheck
{



GenericConfigPage::GenericConfigPage(QWidget* parent, const QVariantList& args)
    : KCModule(CppcheckPreferencesFactory::componentData(), parent, args)
{

    ui = new Ui::GenericConfig();
    ui->setupUi(this);
    connect(ui->SavePushButton , SIGNAL(clicked()), this, SLOT(save()));
}

GenericConfigPage::~GenericConfigPage(void)
{
    disconnect(ui->SavePushButton , SIGNAL(clicked()), this, SLOT(save()));
    delete ui;
}

KIcon GenericConfigPage::icon() const
{
    return KIcon("fork");
}

void GenericConfigPage::load()
{
    kDebug() << "load config";
    bool wasBlocked = signalsBlocked();
    blockSignals(true);
    KConfig config("kdevcppcheckrc");
    KConfigGroup cfg = config.group("cppcheck");
    ui->cppcheckExecutable->setUrl(cfg.readEntry("CppcheckExecutable", KUrl("/usr/bin/cppcheck")));
    ui->cppcheckParameters->setText(cfg.readEntry("cppcheckParameters", QString("")));
    ui->styleCheckBox->setChecked(cfg.readEntry("AdditionalCheckStyle", false));
    ui->performanceCheckBox->setChecked(cfg.readEntry("AdditionalCheckPerformance", false));
    ui->portabilityCheckBox->setChecked(cfg.readEntry("AdditionalCheckPortability", false));
    ui->informationCheckBox->setChecked(cfg.readEntry("AdditionalCheckInformation", false));
    ui->unusedFunctionCheckBox->setChecked(cfg.readEntry("AdditionalCheckUnusedInclude", false));
    ui->missingIncludeCheckBox->setChecked(cfg.readEntry("AdditionalCheckMissingInclude", false));

    blockSignals(wasBlocked);
}

void GenericConfigPage::save()
{
    kDebug() << "save config";
    KConfig config("kdevcppcheckrc");
    KConfigGroup cfg = config.group("cppcheck");
    cfg.writeEntry("CppcheckExecutable", ui->cppcheckExecutable->url());
    cfg.writeEntry("cppcheckParameters", ui->cppcheckParameters->text());
    cfg.writeEntry("AdditionalCheckStyle", ui->styleCheckBox->isChecked());
    cfg.writeEntry("AdditionalCheckPerformance", ui->performanceCheckBox->isChecked());
    cfg.writeEntry("AdditionalCheckPortability", ui->portabilityCheckBox->isChecked());
    cfg.writeEntry("AdditionalCheckInformation", ui->informationCheckBox->isChecked());
    cfg.writeEntry("AdditionalCheckUnusedInclude", ui->unusedFunctionCheckBox->isChecked());
    cfg.writeEntry("AdditionalCheckMissingInclude", ui->missingIncludeCheckBox->isChecked());

}

QString GenericConfigPage::title() const
{
    return i18n("Global settings");
}

}
