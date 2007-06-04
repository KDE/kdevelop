/* KDevelop
 *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>
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

#include "uipreferences.h"

#include <QVBoxLayout>

#include <kgenericfactory.h>

#include "ui_uiconfig.h"
#include "uiconfig.h"

typedef KGenericFactory<UiPreferences> UiPreferencesFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kdev_uisettings, UiPreferencesFactory( "kcm_kdev_uisettings" )  )


UiPreferences::UiPreferences(QWidget* parent, const QStringList& args )
    : KCModule( UiPreferencesFactory::componentData(), parent, args )
{
    QVBoxLayout* l = new QVBoxLayout( this );
    QWidget* w = new QWidget(parent);
    m_uiconfigUi = new Ui::UiConfig();
    m_uiconfigUi->setupUi( w );
    l->addWidget( w );

    addConfig( UiConfig::self(), w );
    load();
}

UiPreferences::~UiPreferences()
{}


//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
