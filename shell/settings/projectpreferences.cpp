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
#include "projectpreferences.h"

#include <QVBoxLayout>

#include <kaboutdata.h>
#include <KPluginFactory>

#include "projectconfig.h"
#include "../core.h"
#include "../projectcontroller.h"

#include "ui_projectsettings.h"

namespace KDevelop
{

K_PLUGIN_FACTORY_WITH_JSON(ProjectPreferencesFactory, "kcm_kdev_projectsettings.json", registerPlugin<ProjectPreferences>();)

ProjectPreferences::ProjectPreferences( QWidget *parent, const QVariantList &args )
    : KCModule( KAboutData::pluginData("kcm_kdev_projectsettings"), parent, args )
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
    KCModule::save();

    Core::self()->projectControllerInternal()->loadSettings(false);
}

void ProjectPreferences::slotSettingsChanged()
{
    emit changed( true );
}

}
#include "projectpreferences.moc"

