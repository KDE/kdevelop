/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
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

#include "cmakepreferences.h"

#include <QWidget>
#include <QVBoxLayout>

#include <kurl.h>
#include <kgenericfactory.h>

#include "ui_cmakebuildsettings.h"
#include "config.h"

typedef KGenericFactory<CMakePreferences> CMakePreferencesFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kdevcmake_settings, CMakePreferencesFactory( "kcm_kdevcmake_settings" )  )

CMakePreferences::CMakePreferences(QWidget* parent, const QStringList& args)
: KDevCModule(CMakeSettings::self(),CMakePreferencesFactory::instance(), parent, args)
{
    QVBoxLayout* l = new QVBoxLayout( this );
    QWidget* w = new QWidget;
    m_prefsUi = new Ui::CMakeBuildSettings;
    m_prefsUi->setupUi( w );
    l->addWidget( w );

        //Is this correct?
    addConfig( CMakeSettings::self(), w );

    load();

}

CMakePreferences::~CMakePreferences()
{
}

void CMakePreferences::load()
{
    KDevCModule::load();
}

void CMakePreferences::save()
{
    KDevCModule::save();
}

void CMakePreferences::defaults()
{
    KDevCModule::defaults();
}

//kate: space-indent on; indent-width 4; replace-tabs on;
