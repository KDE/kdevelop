/* This file is part of KDevelop

   Copyright 2018 Anton Anikin <anton@anikin.xyz>
   Copyright 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

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

#include "projectconfigpage.h"

// plugin
#include "checksdb.h"
#include "plugin.h"
#include "globalsettings.h"
#include "projectsettings.h"
#include "checksetselectionmanager.h"
// KDevPlatform
#include <interfaces/iproject.h>
// KF
#include <KLocalizedString>
#include <KShell>

namespace Clazy
{

ProjectConfigPage::ProjectConfigPage(Plugin* plugin, KDevelop::IProject* project,
                                     CheckSetSelectionManager* checkSetSelectionManager,
                                     QWidget* parent)
    : ConfigPage(plugin, nullptr, parent)
    , m_checkSetSelections(checkSetSelectionManager->checkSetSelections())
    , m_defaultCheckSetSelectionId(checkSetSelectionManager->defaultCheckSetSelectionId())
{
    Q_ASSERT(plugin);

    m_settings = new ProjectSettings;
    m_settings->setSharedConfig(project->projectConfiguration());
    m_settings->load();
    setConfigSkeleton(m_settings);

    m_ui.setupUi(this);

    if (plugin->checksDB()->isValid()) {
        m_ui.dbError->setVisible(false);
    } else {
        m_ui.dbError->setText(plugin->checksDB()->error());
        m_ui.dbError->setVisible(true);

        m_ui.tabWidget->setVisible(false);
        m_ui.commandLineWidget->setVisible(false);
        return;
    }

    configSkeleton()->setSharedConfig(project->projectConfiguration());
    configSkeleton()->load();

    // =============================================================================================

    m_ui.kcfg_checkSetSelection->setCheckSetSelections(m_checkSetSelections, m_defaultCheckSetSelectionId);
    m_ui.checks->setChecksDb(plugin->checksDB());
    connect(m_ui.checks, &ChecksWidget::checksChanged,
            this, &ProjectConfigPage::updateCommandLine);

    connect(m_ui.kcfg_checkSetSelection, &CheckSetSelectionComboBox::selectionChanged,
            this, &ProjectConfigPage::onSelectionChanged);
    connect(m_ui.checks, &ChecksWidget::checksChanged,
            this, &ProjectConfigPage::onChecksChanged);

    // =============================================================================================

    QCheckBox* const commandLineCheckBoxes[] = {
        m_ui.kcfg_onlyQt,
        m_ui.kcfg_qtDeveloper,
        m_ui.kcfg_qt4Compat,
        m_ui.kcfg_visitImplicitCode,
        m_ui.kcfg_ignoreIncludedFiles,
        m_ui.kcfg_enableAllFixits,
        m_ui.kcfg_noInplaceFixits,
    };
    for (auto* checkBox : commandLineCheckBoxes) {
        connect(checkBox, &QCheckBox::stateChanged,
                this, &ProjectConfigPage::updateCommandLine);
    }
    QLineEdit* const commandLineLineEdits[] = {
        m_ui.kcfg_headerFilter,
        m_ui.kcfg_extraAppend,
        m_ui.kcfg_extraPrepend,
        m_ui.kcfg_extraClazy,
    };
    for (auto* lineEdit : commandLineLineEdits) {
        lineEdit->setPlaceholderText(lineEdit->toolTip());
        connect(lineEdit, &QLineEdit::textChanged,
                this, &ProjectConfigPage::updateCommandLine);
    }

    updateCommandLine();
}

ProjectConfigPage::~ProjectConfigPage() = default;

QIcon ProjectConfigPage::icon() const
{
    return QIcon::fromTheme(QStringLiteral("clazy"));
}

QString ProjectConfigPage::name() const
{
    return i18n("Clazy");
}

void ProjectConfigPage::updateCommandLine()
{
    QStringList arguments;

    arguments << GlobalSettings::executablePath().toLocalFile();

    const auto checks = m_ui.checks->checks();
    if (!checks.isEmpty()) {
        arguments << QLatin1String("-checks=") + checks;
    }

    if (m_ui.kcfg_onlyQt->isChecked()) {
        arguments << QStringLiteral("-only-qt");
    }

    if (m_ui.kcfg_qtDeveloper->isChecked()) {
        arguments << QStringLiteral("-qt-developer");
    }

    if (m_ui.kcfg_qt4Compat->isChecked()) {
        arguments << QStringLiteral("-qt4-compat");
    }

    if (m_ui.kcfg_visitImplicitCode->isChecked()) {
        arguments << QStringLiteral("-visit-implicit-code");
    }

    if (m_ui.kcfg_ignoreIncludedFiles->isChecked()) {
        arguments << QStringLiteral("-ignore-included-files");
    }

    const auto headerFilter = m_ui.kcfg_headerFilter->text();
    if (!headerFilter.isEmpty()) {
        arguments << QLatin1String("-header-filter=") + headerFilter;
    }

    if (m_ui.kcfg_enableAllFixits->isChecked()) {
        arguments << QStringLiteral("-enable-all-fixits");
    }

    if (m_ui.kcfg_noInplaceFixits->isChecked()) {
        arguments << QStringLiteral("-no-inplace-fixits");
    }

    const auto extraAppend = m_ui.kcfg_extraAppend->text();
    if (!extraAppend.isEmpty()) {
        arguments << QLatin1String("-extra-arg=") + extraAppend;
    }

    const auto extraPrepend = m_ui.kcfg_extraPrepend->text();
    if (!extraPrepend.isEmpty()) {
        arguments << QLatin1String("-extra-arg-before1") + extraPrepend;
    }

    const auto extraClazy = m_ui.kcfg_extraClazy->text();
    if (!extraClazy.isEmpty()) {
        arguments << KShell::splitArgs(extraClazy);
    }

    arguments << QStringLiteral("-p <build directory>");

    m_ui.commandLineWidget->setText(arguments.join(QLatin1Char(' ')));
}

void ProjectConfigPage::defaults()
{
    ConfigPage::defaults();
    onSelectionChanged(m_ui.kcfg_checkSetSelection->selection());
}

void ProjectConfigPage::reset()
{
    ConfigPage::reset();

    onSelectionChanged(m_ui.kcfg_checkSetSelection->selection());
}

void ProjectConfigPage::apply()
{
    ConfigPage::apply();
}

void ProjectConfigPage::onSelectionChanged(const QString& selectionId)
{
    QString checks;
    bool editable = false;
    if (selectionId.isEmpty()) {
        checks = m_ui.kcfg_checks->checks();
        editable = true;
    } else {
        const  QString effectiveSelectionId = (selectionId == QLatin1String("Default")) ? m_defaultCheckSetSelectionId : selectionId;
        for (auto& selection : m_checkSetSelections) {
            if (selection.id() == effectiveSelectionId) {
                checks = selection.selectionAsString();
                break;
            }
        }
    }

    m_ui.checks->setEditable(editable);
    m_ui.checks->setChecks(checks);
}

void ProjectConfigPage::onChecksChanged(const QString& checks)
{
    const bool isCustomSelected =  m_ui.kcfg_checkSetSelection->selection().isEmpty();
    if (!isCustomSelected) {
        return;
    }

    m_ui.kcfg_checks->setChecks(checks);
}

}
