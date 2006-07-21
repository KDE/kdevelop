/* KDevEnv Settings
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
#include "kdevenvpreferences.h"

#include <qspinbox.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qradiobutton.h> 
//Added by qt3to4:
#include <QVBoxLayout>

#include <klocale.h>
#include <kgenericfactory.h>

#include "kdevenvwidget.h"

typedef KGenericFactory<KDevEnvPreferences> PreferencesFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kdevenv_settings, PreferencesFactory( "kcm_kdevenv_settings" ) )

KDevEnvPreferences::KDevEnvPreferences( QWidget *parent, const QStringList &args )
        : KDevCModule( PreferencesFactory::instance(), parent, args )
{
    QVBoxLayout * l = new QVBoxLayout( this );
    QWidget* w = new QWidget;
    preferencesDialog = new KDevEnvWidget( w );
    l->addWidget( w );

    connect( preferencesDialog, SIGNAL( changed( bool ) ),
             this, SLOT( settingsChanged( bool ) ) );

    load();
}

KDevEnvPreferences::~KDevEnvPreferences( )
{
    delete preferencesDialog;
}

void KDevEnvPreferences::save()
{
    preferencesDialog->saveSettings();
    KDevCModule::save();
}

void KDevEnvPreferences::load()
{
    preferencesDialog->loadSettings();
    KDevCModule::load();
}

void KDevEnvPreferences::defaults()
{
    preferencesDialog->defaults();
    KDevCModule::defaults();
}

void KDevEnvPreferences::settingsChanged( bool changed )
{
    unmanagedWidgetChangeState( changed );
}

#include "kdevenvpreferences.moc"

