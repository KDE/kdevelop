/*
 * This file is part of KDevelop
 *
 * Copyright 2016 Carlos Nihelton <carlosnsoliveira@gmail.com>
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

#include "config/perprojectconfigpage.h"
#include "ui_perprojectconfig.h"

#include <checkset.h>

#include <interfaces/iproject.h>

#include <QItemSelectionModel>
#include <QSortFilterProxyModel>
#include <QStringListModel>

namespace ClangTidy
{

PerProjectConfigPage::PerProjectConfigPage(KDevelop::IProject* project, const CheckSet* checkSet,
                                           QWidget* parent)
    : KDevelop::ConfigPage(nullptr, nullptr, parent)
    , m_project(project)
    , m_checkSet(checkSet)
{
    ui = new Ui::PerProjectConfig();
    ui->setupUi(this);

    m_availableChecksModel = new QStringListModel();
    m_availableChecksModel->setStringList(m_checkSet->all());
    QSortFilterProxyModel* checksFilterProxyModel = new QSortFilterProxyModel(this);
    ui->checkFilterEdit->setProxy(checksFilterProxyModel);
    checksFilterProxyModel->setSourceModel(m_availableChecksModel);
    ui->checkListView->setModel(checksFilterProxyModel);

    m_selectedItemModel = new QItemSelectionModel(m_availableChecksModel);
    ui->checkListView->setSelectionModel(m_selectedItemModel);

    m_config = m_project->projectConfiguration()->group("ClangTidy");
}

PerProjectConfigPage::~PerProjectConfigPage(void)
{
    delete ui;
}

QIcon PerProjectConfigPage::icon() const
{
    return QIcon::fromTheme(QStringLiteral("dialog-ok"));
}

KDevelop::ConfigPage::ConfigPageType PerProjectConfigPage::configPageType() const
{
    return ConfigPage::AnalyzerConfigPage;
}

QString PerProjectConfigPage::name() const
{
    return i18n("Clang-Tidy");
}

void PerProjectConfigPage::updateEnabledChecks(const QStringList& enabledCheckList)
{
    for (int i = 0; i < m_availableChecksModel->rowCount(); ++i) {
        QModelIndex index = m_availableChecksModel->index(i, 0);
        if (index.isValid()) {
            const bool isEnabled = enabledCheckList.contains(index.data().toString());
            m_selectedItemModel->select(index, isEnabled ? QItemSelectionModel::Select : QItemSelectionModel::Deselect);
        }
    }
}

void PerProjectConfigPage::apply()
{
    // TODO: discover a way to set the project folders where user header files
    // might exist into this option.
    // Right now it only works with manual entry.
    m_config.writeEntry(ConfigGroup::HeaderFilter, ui->headerFilterText->text());
    m_config.writeEntry(ConfigGroup::AdditionalParameters, ui->clangtidyParameters->text());
    m_config.enableEntry(ConfigGroup::CheckSystemHeaders, ui->sysHeadersCheckBox->isChecked());
    m_config.enableEntry(ConfigGroup::UseConfigFile, !ui->overrideConfigFileCheckBox->isChecked());
    m_config.enableEntry(ConfigGroup::DumpConfig, ui->dumpCheckBox->isChecked());

    QStringList activeChecks;
    for (int i = 0; i < m_availableChecksModel->rowCount(); ++i) {
        QModelIndex index = m_availableChecksModel->index(i, 0);
        if (index.isValid()) {
            const bool isSelected = m_selectedItemModel->isSelected(index);
            if (isSelected) {
                const auto check = index.data().toString();
                activeChecks.append(check);
            }
        }
    }
    m_config.writeEntry(ConfigGroup::EnabledChecks, activeChecks.join(','));
}

void PerProjectConfigPage::defaults()
{
    // TODO: discover a way to set the project folders where user header files
    // might exist into this option.
    // Right now it only works with manual entry.

    ui->headerFilterText->setText(QString());

    ui->clangtidyParameters->setText(QString());

    ui->sysHeadersCheckBox->setChecked(false);

    ui->overrideConfigFileCheckBox->setChecked(true);
    ui->checkListGroupBox->setEnabled(true);

    ui->dumpCheckBox->setChecked(true);
    ui->checkListGroupBox->setEnabled(true);

    updateEnabledChecks(m_checkSet->defaults());
}

void PerProjectConfigPage::reset()
{
    ui->headerFilterText->setText(m_config.readEntry(ConfigGroup::HeaderFilter).remove("--header-filter="));
    ui->clangtidyParameters->setText(m_config.readEntry(ConfigGroup::AdditionalParameters));
    ui->sysHeadersCheckBox->setChecked(!m_config.readEntry(ConfigGroup::CheckSystemHeaders).isEmpty());
    ui->overrideConfigFileCheckBox->setChecked(m_config.readEntry(ConfigGroup::UseConfigFile).isEmpty());
    ui->checkListGroupBox->setEnabled(m_config.readEntry(ConfigGroup::UseConfigFile).isEmpty());
    ui->dumpCheckBox->setChecked(!m_config.readEntry(ConfigGroup::DumpConfig).isEmpty());

    auto enabledChecks = m_config.readEntry(ConfigGroup::EnabledChecks).split(QLatin1Char(','));
    if (enabledChecks.isEmpty()) {
        enabledChecks = m_checkSet->defaults();
    }
    updateEnabledChecks(enabledChecks);
}

} // namespace ClangTidy
