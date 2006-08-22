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

typedef KGenericFactory<KDevProjectPreferences> KDevProjectPreferencesFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kdevproject_settings, KDevProjectPreferencesFactory( "kcm_kdevproject_settings" ) )

KDevProjectPreferences::KDevProjectPreferences( QWidget *parent, const QStringList &args )
        : KDevCModule( KDevProjectSettings::self(),
                       KDevProjectPreferencesFactory::instance(), parent, args )
{

    QVBoxLayout * l = new QVBoxLayout( this );
    QWidget* w = new QWidget;
    preferencesDialog = new Ui::ProjectSettings;
    preferencesDialog->setupUi( w );
    l->addWidget( w );

    addConfig( KDevProjectSettings::self(), w );

    load();
}

KDevProjectPreferences::~KDevProjectPreferences( )
{
    delete preferencesDialog;
}

void KDevProjectPreferences::save()
{
    KDevCModule::save();
}

void KDevProjectPreferences::load()
{
    KDevCModule::load();
}

void KDevProjectPreferences::slotSettingsChanged()
{
    emit changed( true );
}

void KDevProjectPreferences::defaults()
{}

#include "kdevprojectpreferences.moc"

