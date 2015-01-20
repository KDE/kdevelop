/************************************************************************
 * KDevelop4 Custom Buildsystem Support                                 *
 *                                                                      *
 * Copyright 2010 Andreas Pakulat <apaku@gmx.de>                        *
 *                                                                      *
 * This program is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation; either version 2 or version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful, but  *
 * WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     *
 * General Public License for more details.                             *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with this program; if not, see <http://www.gnu.org/licenses/>. *
 ************************************************************************/

#include "kcm_custombuildsystem.h"

#include <KPluginFactory>
#include <KAboutData>
#include <QVBoxLayout>

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>

#include "custombuildsystemconfigwidget.h"

CustomBuildSystemKCModule::CustomBuildSystemKCModule(KDevelop::IPlugin* plugin, const KDevelop::ProjectConfigOptions& options, QWidget* parent)
    : ProjectConfigPage<CustomBuildSystemSettings>(plugin, options, parent)
{
    QVBoxLayout* layout = new QVBoxLayout( this );
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
    configWidget->saveTo(CustomBuildSystemSettings::self()->config(), project());
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
    return i18n("Custom BuildSystem");
}

QString CustomBuildSystemKCModule::fullName() const
{
    return i18n("Configure a projects custom build tool and includes/defines for the language support.");
}

QIcon CustomBuildSystemKCModule::icon() const
{
    return QIcon::fromTheme("kdevelop");
}

