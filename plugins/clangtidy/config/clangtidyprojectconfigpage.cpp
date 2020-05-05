/*
 * This file is part of KDevelop
 *
 * Copyright 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
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

#include "config/clangtidyprojectconfigpage.h"

// plugin
#include "checksetselectionmanager.h"
#include <debug.h>
#include <checkset.h>
#include <clangtidyprojectconfig.h>
// KDevPlatform
#include <interfaces/iproject.h>

namespace ClangTidy
{

ProjectConfigPage::ProjectConfigPage(KDevelop::IPlugin* plugin,
                                     KDevelop::IProject* project,
                                     CheckSetSelectionManager* checkSetSelectionManager,
                                     const CheckSet* checkSet,
                                     QWidget* parent)
    : KDevelop::ConfigPage(plugin, nullptr, parent)
    , m_project(project)
    , m_checkSetSelections(checkSetSelectionManager->checkSetSelections())
    , m_defaultCheckSetSelectionId(checkSetSelectionManager->defaultCheckSetSelectionId())
{
    m_settings = new ClangTidyProjectSettings;
    m_settings->setSharedConfig(project->projectConfiguration());
    m_settings->load();
    setConfigSkeleton(m_settings);

    m_ui.setupUi(this);

    m_ui.kcfg_checkSetSelection->setCheckSetSelections(m_checkSetSelections, m_defaultCheckSetSelectionId);
    m_ui.enabledChecks->setCheckSet(checkSet);

    connect(m_ui.kcfg_checkSetSelection, &CheckSetSelectionComboBox::selectionChanged,
            this, &ProjectConfigPage::onSelectionChanged);
    connect(m_ui.enabledChecks, &CheckSelection::checksChanged,
            this, &ProjectConfigPage::onChecksChanged);
}

ProjectConfigPage::~ProjectConfigPage() = default;

KDevelop::ConfigPage::ConfigPageType ProjectConfigPage::configPageType() const
{
    return ConfigPage::AnalyzerConfigPage;
}

QString ProjectConfigPage::name() const
{
    return i18n("Clang-Tidy");
}

QIcon ProjectConfigPage::icon() const
{
    return QIcon::fromTheme(QStringLiteral("dialog-ok"));
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
        checks = m_ui.kcfg_enabledChecks->checks();
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

    m_ui.enabledChecks->setEditable(editable);
    m_ui.enabledChecks->setChecks(checks);
}

void ProjectConfigPage::onChecksChanged(const QString& checks)
{
    const bool isCustomSelected =  m_ui.kcfg_checkSetSelection->selection().isEmpty();
    if (!isCustomSelected) {
        return;
    }

    m_ui.kcfg_enabledChecks->setChecks(checks);
}

} // namespace ClangTidy
