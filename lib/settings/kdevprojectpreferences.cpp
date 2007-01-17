/* KDevelop Project Settings
*
* Copyright 2006  Matt Rogers <mattr@kde.org>
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
#include "kdevprojectpreferences.h"

#include <QVBoxLayout>

#include <kgenericfactory.h>

#include "kdevprojectconfig.h"

#include "ui_projectsettings.h"

namespace Koncrete
{

typedef KGenericFactory<ProjectPreferences> ProjectPreferencesFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kdevproject_settings, ProjectPreferencesFactory( "kcm_kdevproject_settings" ) )

ProjectPreferences::ProjectPreferences( QWidget *parent, const QStringList &args )
        : ConfigModule( ProjectSettings::self(),
                       ProjectPreferencesFactory::instance(), parent, args )
{

    QVBoxLayout * l = new QVBoxLayout( this );
    QWidget* w = new QWidget;
    preferencesDialog = new Ui::ProjectSettings;
    preferencesDialog->setupUi( w );
    l->addWidget( w );

    addConfig( ProjectSettings::self(), w );

    load();
}

ProjectPreferences::~ProjectPreferences( )
{
    delete preferencesDialog;
}

void ProjectPreferences::save()
{
    ConfigModule::save();
}

void ProjectPreferences::load()
{
    ConfigModule::load();
}

void ProjectPreferences::slotSettingsChanged()
{
    emit changed( true );
}

void ProjectPreferences::defaults()
{
}

}
#include "kdevprojectpreferences.moc"

