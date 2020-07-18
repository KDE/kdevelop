/* This file is part of KDevelop

   Copyright 2018 Anton Anikin <anton@anikin.xyz>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "globalconfigpage.h"
#include "ui_globalconfigpage.h"

#include "checksdb.h"
#include "globalsettings.h"
#include "checksetselectionmanager.h"

namespace Clazy
{

GlobalConfigPage::GlobalConfigPage(CheckSetSelectionManager* checkSetSelectionManager,
                                   const QSharedPointer<const ChecksDB>& db,
                                   KDevelop::IPlugin* plugin, QWidget* parent)
    : ConfigPage(plugin, GlobalSettings::self(), parent)
    , m_checkSetSelectionManager(checkSetSelectionManager)
{
    ui = new Ui::GlobalConfigPage();
    ui->setupUi(this);
    ui->checksets->setCheckSetSelectionManager(checkSetSelectionManager, db);

    auto checkPaths = [&]() {
        ChecksDB db(ui->kcfg_docsPath->url());
        ui->checksInfoLabel->setText(i18np("1 check detected", "%1 checks detected", db.checks().size()));

        JobGlobalParameters params(ui->kcfg_executablePath->url(), ui->kcfg_docsPath->url());
        if (!params.isValid()) {
            ui->errorWidget->setText(params.error());
            ui->errorWidget->setVisible(true);
            return;
        }

        if (!db.isValid()) {
            ui->errorWidget->setText(db.error());
            ui->errorWidget->setVisible(true);
            return;
        }

        ui->errorWidget->setVisible(false);
    };

    connect(ui->kcfg_executablePath, &KUrlRequester::textChanged, this, checkPaths);
    connect(ui->kcfg_docsPath, &KUrlRequester::textChanged, this, checkPaths);

    checkPaths();
    ui->kcfg_executablePath->setPlaceholderText(ui->kcfg_executablePath->toolTip());
    ui->kcfg_docsPath->setPlaceholderText(ui->kcfg_docsPath->toolTip());

    auto checkJobs = [&]() {
        const bool jobsEnabled = ui->kcfg_parallelJobsEnabled->checkState() == Qt::Checked;
        const bool autoEnabled = ui->kcfg_parallelJobsAutoCount->checkState() == Qt::Checked;

        ui->kcfg_parallelJobsAutoCount->setEnabled(jobsEnabled);

        ui->kcfg_parallelJobsFixedCount->setEnabled(jobsEnabled && !autoEnabled);
        ui->parallelJobsFixedCountLabel->setEnabled(jobsEnabled && !autoEnabled);
    };

    connect(ui->kcfg_parallelJobsEnabled, &QCheckBox::stateChanged, this, checkJobs);
    connect(ui->kcfg_parallelJobsAutoCount, &QCheckBox::stateChanged, this, checkJobs);
    connect(ui->checksets, &CheckSetManageWidget::changed,
            this, &GlobalConfigPage::changed);

    checkJobs();
}

KDevelop::ConfigPage::ConfigPageType GlobalConfigPage::configPageType() const
{
    return KDevelop::ConfigPage::AnalyzerConfigPage;
}

QString GlobalConfigPage::name() const
{
    return i18nc("@title:tab", "Clazy");
}

QString GlobalConfigPage::fullName() const
{
    return i18nc("@title:tab", "Configure Clazy Settings");
}

QIcon GlobalConfigPage::icon() const
{
    return QIcon::fromTheme(QStringLiteral("clazy"));
}

void GlobalConfigPage::apply()
{
    ConfigPage::apply();
    ui->checksets->store();
}

void GlobalConfigPage::defaults()
{
    ConfigPage::defaults();
    ui->checksets->reload();
}

void GlobalConfigPage::reset()
{
    ConfigPage::reset();
    ui->checksets->reload();
}

}
