/*
    SPDX-FileCopyrightText: 2016 Carlos Nihelton <carlosnsoliveira@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "clangtidypreferences.h"

// plugin
#include "clangtidyconfig.h"
#include "ui_clangtidypreferences.h"
#include "checksetselectionmanager.h"

using KDevelop::IPlugin;
using KDevelop::ConfigPage;
using namespace ClangTidy;

ClangTidyPreferences::ClangTidyPreferences(CheckSetSelectionManager* checkSetSelectionManager,
                                           const CheckSet* checkSet,
                                           IPlugin* plugin, QWidget* parent)
    : ConfigPage(plugin, ClangTidySettings::self(), parent)
{
    ui = new Ui::ClangTidyPreferences();
    ui->setupUi(this);
    ui->checksets->setCheckSetSelectionManager(checkSetSelectionManager, checkSet);

    connect(ui->kcfg_parallelJobsEnabled, &QCheckBox::toggled,
            this, &ClangTidyPreferences::updateJobCountEnabledState);
    connect(ui->kcfg_parallelJobsAutoCount, &QCheckBox::toggled,
            this, &ClangTidyPreferences::updateJobCountEnabledState);
    connect(ui->checksets, &CheckSetManageWidget::changed,
            this, &ClangTidyPreferences::changed);

    updateJobCountEnabledState();
}

ClangTidyPreferences::~ClangTidyPreferences()
{
    delete ui;
}

void ClangTidyPreferences::updateJobCountEnabledState()
{
    const bool jobsEnabled = ui->kcfg_parallelJobsEnabled->isChecked();
    const bool autoEnabled = ui->kcfg_parallelJobsAutoCount->isChecked();
    const bool manualJobsEnabled = (jobsEnabled && !autoEnabled);

    ui->kcfg_parallelJobsAutoCount->setEnabled(jobsEnabled);

    ui->kcfg_parallelJobsFixedCount->setEnabled(manualJobsEnabled);
    ui->parallelJobsFixedCountLabel->setEnabled(manualJobsEnabled);
}

ConfigPage::ConfigPageType ClangTidyPreferences::configPageType() const
{
    return ConfigPage::AnalyzerConfigPage;
}

QString ClangTidyPreferences::name() const
{
    return i18nc("@title:tab", "Clang-Tidy");
}

QString ClangTidyPreferences::fullName() const
{
    return i18nc("@title:tab", "Configure Clang-Tidy Settings");
}

QIcon ClangTidyPreferences::icon() const
{
    return QIcon::fromTheme(QStringLiteral("dialog-ok"));
}

void ClangTidyPreferences::apply()
{
    ConfigPage::apply();
    ui->checksets->store();
}

void ClangTidyPreferences::defaults()
{
    ConfigPage::defaults();
    ui->checksets->reload();
}

void ClangTidyPreferences::reset()
{
    ConfigPage::reset();
    ui->checksets->reload();
}

#include "moc_clangtidypreferences.cpp"
