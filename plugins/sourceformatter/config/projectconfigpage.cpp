/*
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
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

#include "moc_projectconfigpage.cpp"
