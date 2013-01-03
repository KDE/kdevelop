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

#include "ninjabuilderpreferences.h"

#include <QVBoxLayout>

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <util/environmentgrouplist.h>

#include "ui_ninjaconfig.h"
#include "ninjabuilderconfig.h"

K_PLUGIN_FACTORY(NinjaBuilderPreferencesFactory, registerPlugin<NinjaBuilderPreferences>(); )
K_EXPORT_PLUGIN(NinjaBuilderPreferencesFactory("kcm_kdev_ninjabuilder"))

NinjaBuilderPreferences::NinjaBuilderPreferences(QWidget* parent, const QVariantList& args)
    : ProjectKCModule<NinjaBuilderSettings>( NinjaBuilderPreferencesFactory::componentData(),
                parent, args)
{
    QVBoxLayout* l = new QVBoxLayout( this );
    QWidget* w = new QWidget;
    m_prefsUi = new Ui::NinjaConfig;
    m_prefsUi->setupUi( w );
    l->addWidget( w );

    m_prefsUi->configureEnvironment->setSelectionWidget( m_prefsUi->kcfg_environmentProfile );

    addConfig( NinjaBuilderSettings::self(), w );
}

NinjaBuilderPreferences::~NinjaBuilderPreferences()
{
    delete m_prefsUi;
}

#include "ninjabuilderpreferences.moc"

