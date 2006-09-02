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
#include "kdevbgpreferences.h"

#include <QVBoxLayout>

#include <kgenericfactory.h>

#include "kdevcore.h"
#include "kdevbackgroundparser.h"

#include "kdevbgconfig.h"

#include "ui_bgsettings.h"

typedef KGenericFactory<KDevBGPreferences> KDevBGPreferencesFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kdevbg_settings, KDevBGPreferencesFactory( "kcm_kdevbg_settings" ) )

KDevBGPreferences::KDevBGPreferences( QWidget *parent, const QStringList &args )
        : KDevCModule( KDevBGSettings::self(),
                       KDevBGPreferencesFactory::instance(), parent, args )
{

    QVBoxLayout * l = new QVBoxLayout( this );
    QWidget* w = new QWidget;
    preferencesDialog = new Ui::BGSettings;
    preferencesDialog->setupUi( w );

    preferencesDialog->kcfg_delay->setRange( 0, 5000, true );
    preferencesDialog->kcfg_threads->setRange( 1, 32, true );

    l->addWidget( w );

    addConfig( KDevBGSettings::self(), w );

    load();
}

KDevBGPreferences::~KDevBGPreferences( )
{
    delete preferencesDialog;
}

void KDevBGPreferences::save()
{
    KDevCModule::save();

    if ( preferencesDialog->kcfg_enable->isChecked() )
        KDevCore::backgroundParser()->resume();
    else
        KDevCore::backgroundParser()->suspend();

    KDevCore::backgroundParser()->setDelay( preferencesDialog->kcfg_delay->value() );
    KDevCore::backgroundParser()->setThreads( preferencesDialog->kcfg_threads->value() );
}

#include "kdevbgpreferences.moc"

