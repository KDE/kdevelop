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
#include "bgpreferences.h"

#include <QVBoxLayout>

#include <kgenericfactory.h>

#include "core.h"
#include "ilanguagecontroller.h"
#include "backgroundparser.h"

#include "bgconfig.h"

#include "ui_bgsettings.h"

namespace KDevelop
{

K_PLUGIN_FACTORY(BGPreferencesFactory, registerPlugin<BGPreferences>();)
K_EXPORT_PLUGIN(BGPreferencesFactory("kcm_kdev_bgsettings"))


BGPreferences::BGPreferences( QWidget *parent, const QVariantList &args )
 : KCModule( BGPreferencesFactory::componentData(), parent, args )
{

    QVBoxLayout * l = new QVBoxLayout( this );
    QWidget* w = new QWidget;
    preferencesDialog = new Ui::BGSettings;
    preferencesDialog->setupUi( w );

    preferencesDialog->kcfg_delay->setRange( 0, 5000, true );
    preferencesDialog->kcfg_threads->setRange( 1, 32, true );

    l->addWidget( w );

    addConfig( BGSettings::self(), w );

    load();
}

BGPreferences::~BGPreferences( )
{
    delete preferencesDialog;
}

void BGPreferences::save()
{
    KCModule::save();

    if ( preferencesDialog->kcfg_enable->isChecked() )
        Core::self()->languageController()->backgroundParser()->resume();
    else
        Core::self()->languageController()->backgroundParser()->suspend();

    Core::self()->languageController()->backgroundParser()->setDelay( preferencesDialog->kcfg_delay->value() );
    Core::self()->languageController()->backgroundParser()->setThreadCount( preferencesDialog->kcfg_threads->value() );

}

}

#include "bgpreferences.moc"

