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
#include <debug.h>
#include <checkset.h>
#include <clangtidyprojectconfig.h>
// KDevPlatform
#include <interfaces/iproject.h>

namespace ClangTidy
{

ProjectConfigPage::ProjectConfigPage(KDevelop::IPlugin* plugin,
                                     KDevelop::IProject* project, const CheckSet* checkSet,
                                     QWidget* parent)
    : KDevelop::ConfigPage(plugin, nullptr, parent)
    , m_project(project)
{
    auto* config = new ClangTidyProjectSettings;
    config->setSharedConfig(project->projectConfiguration());
    config->load();
    setConfigSkeleton(config);

    m_ui.setupUi(this);

    m_ui.kcfg_enabledChecks->setCheckSet(checkSet);
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

} // namespace ClangTidy
