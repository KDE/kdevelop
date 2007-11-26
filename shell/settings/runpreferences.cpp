/* KDevelop Run Settings
*
* Copyright 2006  Matt Rogers <mattr@kde.org>
* Copyright 2007  Hamish Rodda <rodda@kde.org>
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

#include "runpreferences.h"

#include <QVBoxLayout>

#include <kgenericfactory.h>

#include "core.h"
#include "ilanguagecontroller.h"

#include "environmentgrouplist.h"

#include "runconfig.h"

#include "ui_runsettings.h"

namespace KDevelop
{

K_PLUGIN_FACTORY(RunPreferencesFactory, registerPlugin<RunPreferences>();)
K_EXPORT_PLUGIN(RunPreferencesFactory("kcm_kdev_runsettings"))

RunPreferences::RunPreferences( QWidget *parent, const QVariantList &args )
 : KCModule( RunPreferencesFactory::componentData(), parent, args )
{
    QVBoxLayout * l = new QVBoxLayout( this );
    QWidget* w = new QWidget;
    preferencesDialog = new Ui::RunSettings;
    preferencesDialog->setupUi( w );

    KDevelop::EnvironmentGroupList env( RunSettings::self()->config() );
    preferencesDialog->kcfg_environment->addItems( env.groups() );

    l->addWidget( w );

    addConfig( RunSettings::self(), w );

    load();
}

RunPreferences::~RunPreferences( )
{
    delete preferencesDialog;
}

void RunPreferences::save()
{
    KCModule::save();
}

}

#include "runpreferences.moc"
