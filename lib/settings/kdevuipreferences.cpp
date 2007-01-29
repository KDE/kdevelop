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
#include "kdevuipreferences.h"

#include <QVBoxLayout>

#include <kgenericfactory.h>

#include "kdevcore.h"
#include "kdevmainwindow.h"

#include "kdevuiconfig.h"

#include "ui_uisettings.h"

namespace Koncrete
{

typedef KGenericFactory<UIPreferences> UIPreferencesFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kdevui_settings, UIPreferencesFactory( "kcm_kdevui_settings" ) )

UIPreferences::UIPreferences( QWidget *parent, const QStringList &args )
        : ConfigModule( UISettings::self(),
                       UIPreferencesFactory::componentData(), parent, args )
{

    QVBoxLayout * l = new QVBoxLayout( this );
    QWidget* w = new QWidget;
    preferencesDialog = new Ui::UISettings;
    preferencesDialog->setupUi( w );
    l->addWidget( w );

    addConfig( UISettings::self(), w );

    load();
}

UIPreferences::~UIPreferences( )
{
    delete preferencesDialog;
}

void UIPreferences::save()
{
    ConfigModule::save();

    if ( UISettings::self()->docked() )
        Core::mainWindow()->setUIMode( MainWindow::DockedMode );
    if ( UISettings::self()->toplevel() )
        Core::mainWindow()->setUIMode( MainWindow::TopLevelMode );
}

}
#include "kdevuipreferences.moc"

