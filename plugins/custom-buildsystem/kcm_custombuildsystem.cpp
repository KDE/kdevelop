/*
    SPDX-FileCopyrightText: 2010 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later
*/

#include "kcm_custombuildsystem.h"

#include <QVBoxLayout>

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>

#include "custombuildsystemconfigwidget.h"

CustomBuildSystemKCModule::CustomBuildSystemKCModule(KDevelop::IPlugin* plugin, const KDevelop::ProjectConfigOptions& options, QWidget* parent)
    : ProjectConfigPage<CustomBuildSystemSettings>(plugin, options, parent)
{
    auto* layout = new QVBoxLayout( this );
    layout->setContentsMargins(0, 0, 0, 0);
    configWidget = new CustomBuildSystemConfigWidget( this );
    connect(configWidget, &CustomBuildSystemConfigWidget::changed, this, &ConfigPage::changed);
    layout->addWidget( configWidget );
}

CustomBuildSystemKCModule::~CustomBuildSystemKCModule()
{
}

void CustomBuildSystemKCModule::reset()
{
    ProjectConfigPage::reset();
    configWidget->loadFrom(CustomBuildSystemSettings::self()->config());
}

void CustomBuildSystemKCModule::apply()
{
    configWidget->saveTo(CustomBuildSystemSettings::self()->config());
    ProjectConfigPage::apply();
    if (KDevelop::IProjectController::parseAllProjectSources()) {
        KDevelop::ICore::self()->projectController()->reparseProject(project());
    }
}

void CustomBuildSystemKCModule::defaults()
{
    ProjectConfigPage::defaults();
    configWidget->loadDefaults();
}

QString CustomBuildSystemKCModule::name() const
{
    return i18nc("@title:tab", "Custom Build System");
}

QString CustomBuildSystemKCModule::fullName() const
{
    return i18n("Configure a projects custom build tool and includes/defines for the language support.");
}

QIcon CustomBuildSystemKCModule::icon() const
{
    return QIcon::fromTheme(QStringLiteral("kdevelop"));
}

#include "moc_kcm_custombuildsystem.cpp"
