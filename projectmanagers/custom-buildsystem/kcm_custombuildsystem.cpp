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
#include <QVBoxLayout>

#include "custombuildsystemconfigwidget.h"
#include "kcfg_custombuildsystemconfig.h"

K_PLUGIN_FACTORY(CustomBuildSystemKCModuleFactory, registerPlugin<CustomBuildSystemKCModule>(); )
K_EXPORT_PLUGIN(CustomBuildSystemKCModuleFactory("kcm_kdevcustombuildsystem", "kdevcustombuildsystem"))

CustomBuildSystemKCModule::CustomBuildSystemKCModule( QWidget* parent, const QVariantList& args )
    : ProjectKCModule<CustomBuildSystemSettings>( CustomBuildSystemKCModuleFactory::componentData(), parent, args )
{
    QVBoxLayout* layout = new QVBoxLayout( this );
    configWidget = new CustomBuildSystemConfigWidget( this );
    connect( configWidget, SIGNAL(changed()), SLOT(dataChanged()) );
    layout->addWidget( configWidget );

    addConfig( CustomBuildSystemSettings::self(), configWidget );
}

void CustomBuildSystemKCModule::dataChanged()
{
    emit changed(true);
}


CustomBuildSystemKCModule::~CustomBuildSystemKCModule()
{
}

void CustomBuildSystemKCModule::load()
{
    KCModule::load();
    configWidget->loadFrom( CustomBuildSystemSettings::self()->config() );
}

void CustomBuildSystemKCModule::save()
{
    configWidget->saveTo( CustomBuildSystemSettings::self()->config(), project() );
    KCModule::save();

    if ( KDevelop::IProjectController::parseAllProjectSources()) {
        KDevelop::ICore::self()->projectController()->reparseProject(project());
    }
}

void CustomBuildSystemKCModule::defaults()
{
    KCModule::defaults();
    configWidget->loadDefaults();
}

#include "kcm_custombuildsystem.moc"

