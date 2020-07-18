/*
 * This file is part of KDevelop
 * Copyright (C) 2017  Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "projectconfigpage.h"
#include "ui_projectconfigpage.h"

#include <shell/core.h>
#include <shell/sourceformattercontroller.h>

#include "projectconfig.h"

#include <interfaces/iproject.h>

namespace SourceFormatter
{

namespace Strings {
QString SourceFormatter() { return QStringLiteral("SourceFormatter"); }
}


ProjectConfigPage::ProjectConfigPage(KDevelop::IPlugin* plugin, KDevelop::IProject* project, QWidget* parent)
    : ConfigPage(plugin, new ProjectConfig, parent)
    , ui(new Ui::ProjectConfigPage)
{
    configSkeleton()->setSharedConfig(project->projectConfiguration());
    configSkeleton()->load();

    ui->setupUi(this);

    connect(ui->kcfg_useDefault, &QAbstractButton::toggled,
            this, &ProjectConfigPage::disableCustomSettings);
    connect(ui->formatterSelectionEdit, &KDevelop::SourceFormatterSelectionEdit::changed,
            this, &ProjectConfigPage::changed);
}

ProjectConfigPage::~ProjectConfigPage() = default;

QString ProjectConfigPage::name() const
{
    return i18nc("@title:tab", "Source Formatter");
}

QString ProjectConfigPage::fullName() const
{
    return i18nc("@title:tab", "Configure Source Formatter");
}

QIcon ProjectConfigPage::icon() const
{
    return QIcon::fromTheme(QStringLiteral("text-field"));
}

void ProjectConfigPage::disableCustomSettings(bool checked)
{
    ui->generalGroupBox->setEnabled(! checked);
    ui->formattinStylesGroupBox->setEnabled(! checked);
}

void ProjectConfigPage::reset()
{
    auto config = configSkeleton()->sharedConfig()->group(Strings::SourceFormatter());
    ui->formatterSelectionEdit->loadSettings(config);
    ConfigPage::reset();
}

void ProjectConfigPage::apply()
{
    auto config = configSkeleton()->sharedConfig()->group(Strings::SourceFormatter());
    ui->formatterSelectionEdit->saveSettings(config);

    ConfigPage::apply();

    // TODO: fix exception of accessing internal API, find a better way to notify about settings change
    KDevelop::Core::self()->sourceFormatterControllerInternal()->settingsChanged();
}

}
