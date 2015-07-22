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

#include <KPluginFactory>

#include "debug.h"
#include "genericconfigpage.h"
#include "plugin.h"

#include "ui_genericconfig.h"

#include <interfaces/iproject.h>

#include <KSharedConfig>
#include <KConfigGroup>

namespace cppcheck
{

GenericConfigPage::GenericConfigPage(KDevelop::IProject *project, QWidget* parent)
    : ConfigPage(nullptr, nullptr, parent)
    , m_project(project)
{

    ui = new Ui::GenericConfig();
    ui->setupUi(this);
}
GenericConfigPage::~GenericConfigPage(void)
{
    delete ui;
}

QString GenericConfigPage::name() const
{
    return QStringLiteral("cppcheck");
}

void GenericConfigPage::apply()
{
    KSharedConfigPtr ptr = m_project->projectConfiguration();
    KConfigGroup group = ptr->group("CppCheck");

    group.writeEntry("cppcheckParameters", ui->cppcheckParameters->text());
    group.writeEntry("AdditionalCheckStyle", ui->styleCheckBox->isChecked());
    group.writeEntry("AdditionalCheckPerformance", ui->performanceCheckBox->isChecked());
    group.writeEntry("AdditionalCheckPortability", ui->portabilityCheckBox->isChecked());
    group.writeEntry("AdditionalCheckInformation", ui->informationCheckBox->isChecked());
    group.writeEntry("AdditionalCheckUnusedFunction", ui->unusedFunctionCheckBox->isChecked());
    group.writeEntry("AdditionalCheckMissingInclude", ui->missingIncludeCheckBox->isChecked());
}

void GenericConfigPage::defaults()
{
    reset();
}

void GenericConfigPage::reset()
{
    KSharedConfigPtr ptr = m_project->projectConfiguration();
    KConfigGroup group = ptr->group("CppCheck");
    if (!group.isValid())
        return;

    bool wasBlocked = signalsBlocked();
    blockSignals(true);

    ui->cppcheckParameters->setText(group.readEntry("cppcheckParameters", QString("")));
    ui->styleCheckBox->setChecked(group.readEntry("AdditionalCheckStyle", false));
    ui->performanceCheckBox->setChecked(group.readEntry("AdditionalCheckPerformance", false));
    ui->portabilityCheckBox->setChecked(group.readEntry("AdditionalCheckPortability", false));
    ui->informationCheckBox->setChecked(group.readEntry("AdditionalCheckInformation", false));
    ui->unusedFunctionCheckBox->setChecked(group.readEntry("AdditionalCheckUnusedFunction", false));
    ui->missingIncludeCheckBox->setChecked(group.readEntry("AdditionalCheckMissingInclude", false));

    blockSignals(wasBlocked);
}

}

#include "genericconfigpage.moc"
