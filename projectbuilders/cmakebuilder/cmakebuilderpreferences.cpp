/* KDevelop CCMake Support
 *
 * Copyright 2012 Aleix Pol Gonzalez <aleixpol@kde.org>
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

#include "cmakebuilderpreferences.h"

#include <QVBoxLayout>

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include "ui_cmakebuildersettings.h"
#include "cmakebuilderconfig.h"

K_PLUGIN_FACTORY(CMakeBuilderPreferencesFactory, registerPlugin<CMakeBuilderPreferences>(); )
K_EXPORT_PLUGIN(CMakeBuilderPreferencesFactory("kcm_kdev_cmakebuilder"))

CMakeBuilderPreferences::CMakeBuilderPreferences(QWidget* parent, const QVariantList& args)
    : KCModule( CMakeBuilderPreferencesFactory::componentData(), parent, args)
{
    QVBoxLayout* l = new QVBoxLayout( this );
    QWidget* w = new QWidget;
    m_prefsUi = new Ui::CMakeBuilderConfig;
    m_prefsUi->setupUi( w );
    l->addWidget( w );

    m_prefsUi->kcfg_generator->addItem("Unix Makefiles");
    m_prefsUi->kcfg_generator->addItem("Ninja");
    addConfig( CMakeBuilderSettings::self(), w );
}

CMakeBuilderPreferences::~CMakeBuilderPreferences()
{
    delete m_prefsUi;
}
