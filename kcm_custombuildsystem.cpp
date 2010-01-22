/************************************************************************
 * KDevelop4 Custom Buildsystem Support                                 *
 *                                                                      *
 * Copyright 2010 Andreas Pakulat <apaku@gmx.de>                        *
 *                                                                      *
 * This program is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation; either version 3 of the License, or    *
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

#include "custombuildsystemconfig.h"
#include "kcfg_custombuildsystemconfig.h"

K_PLUGIN_FACTORY(CustomBuildSystemKCModuleFactory, registerPlugin<CustomBuildSystemKCModule>(); )
K_EXPORT_PLUGIN(CustomBuildSystemKCModuleFactory("kcm_kdevcustombuildsystem"))

CustomBuildSystemKCModule::CustomBuildSystemKCModule( QWidget* parent, const QVariantList& args )
    : ProjectKCModule<CustomBuildSystemSettings>( CustomBuildSystemKCModuleFactory::componentData(), parent, args )
{
    QVBoxLayout* layout = new QVBoxLayout( this );
    CustomBuildSystemConfig* w = new CustomBuildSystemConfig;
    layout->addWidget( w );

    addConfig( CustomBuildSystemSettings::self(), w );
    load();
}

CustomBuildSystemKCModule::~CustomBuildSystemKCModule()
{
}

void CustomBuildSystemKCModule::load()
{
    KCModule::load();
}

void CustomBuildSystemKCModule::save()
{
    KCModule::save();
}

void CustomBuildSystemKCModule::defaults()
{
    KCModule::defaults();
}

#include "kcm_custombuildsystem.moc"

