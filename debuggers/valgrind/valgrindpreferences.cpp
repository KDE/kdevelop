/* KDevelop Valgrind Support
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

#include "valgrindpreferences.h"

#include <QVBoxLayout>

#include <kgenericfactory.h>

#include "valgrindconfig.h"

#include "ui_valgrindsettings.h"

K_PLUGIN_FACTORY(ValgrindPreferencesFactory, registerPlugin<ValgrindPreferences>();)
K_EXPORT_PLUGIN(ValgrindPreferencesFactory("kcm_kdev_valgrindsettings"))

ValgrindPreferences::ValgrindPreferences( QWidget *parent, const QVariantList &args )
  : KCModule( ValgrindPreferencesFactory::componentData(), parent, args )
{
    QVBoxLayout * l = new QVBoxLayout( this );
    QWidget* w = new QWidget;
    settings = new Ui::ValgrindSettings;
    settings->setupUi( w );

    l->addWidget( w );

    addConfig( ValgrindSettings::self(), w );

    load();
}

ValgrindPreferences::~ValgrindPreferences( )
{
    delete settings;
}

#include "valgrindpreferences.moc"

